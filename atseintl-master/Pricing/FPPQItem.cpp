#include "Pricing/FPPQItem.h"

#include "Common/Assert.h"
#include "Common/FarePathCopier.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/PUPath.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FPPQItem*
FPPQItem::createDuplicate(PricingTrx& trx) const
{
  FPPQItem* res;
  trx.dataHandle().get(res);

  *res = *this;
  res->farePath() = FarePathCopier(trx.dataHandle()).getDuplicate(*farePath());

  return res;
}

bool
FPPQItem::GreaterFare::
operator()(const FPPQItem* lhs, const FPPQItem* rhs) const
{
  const PriorityStatus& lps = lhs->priorityStatus();
  const PriorityStatus& rps = rhs->priorityStatus();

  const int lmileage = lhs->_farePath->mileage();
  const int rmileage = rhs->_farePath->mileage();
  if (UNLIKELY(((lmileage != 0) || (rmileage != 0)) && (lmileage != rmileage)))
  {
    return lmileage > rmileage;
  }

  MoneyAmount diff = lhs->_farePath->getNUCAmountScore() - rhs->_farePath->getNUCAmountScore();

  if (diff > EPSILON)
  {
    return true;
  }
  else if (-diff > EPSILON)
  {
    return false;
  }

  const CabinType& leftCabin = lhs->getCabinPriority();
  const CabinType& rightCabin = rhs->getCabinPriority();

  if (UNLIKELY(_cabinComparator.shouldCompareCabins() &&
      !_cabinComparator.areCabinsEqual(leftCabin, rightCabin)))
  {
    return !_cabinComparator.isLessCabin(leftCabin, rightCabin);
  }

  if (UNLIKELY(lhs->getNumberOfStopsPriority() != rhs->getNumberOfStopsPriority()))
  {
    return lhs->getNumberOfStopsPriority() > rhs->getNumberOfStopsPriority();
  }

  // total price is equal
  if (lps.fareByRulePriority() != rps.fareByRulePriority())
  {
    return lps.fareByRulePriority() > rps.fareByRulePriority();
  }

  if (lps.paxTypeFarePriority() != rps.paxTypeFarePriority())
  {
    return lps.paxTypeFarePriority() > rps.paxTypeFarePriority();
  }

  // check price without plus ups
  MoneyAmount lhsBaseAmount =
      (lhs->_farePath->getTotalNUCAmount() - lhs->_farePath->plusUpAmount()) +
      lhs->_farePath->rexChangeFee();

  MoneyAmount rhsBaseAmount =
      (rhs->_farePath->getTotalNUCAmount() - rhs->_farePath->plusUpAmount()) +
      rhs->_farePath->rexChangeFee();

  diff = lhsBaseAmount - rhsBaseAmount;

  if (diff > EPSILON)
  {
    return true;
  }
  else if (-diff > EPSILON)
  {
    return false;
  }

  const size_t lhsPUCount = lhs->_farePath->pricingUnit().size();
  const size_t rhsPUCount = rhs->_farePath->pricingUnit().size();
  if (lhsPUCount != rhsPUCount)
  {
    return lhsPUCount > rhsPUCount;
  }

  if (lps.fareCxrTypePriority() != rps.fareCxrTypePriority())
  {
    return lps.fareCxrTypePriority() > rps.fareCxrTypePriority();
  }

  if (lps.negotiatedFarePriority() != rps.negotiatedFarePriority())
  {
    return lps.negotiatedFarePriority() > rps.negotiatedFarePriority();
  }

  // Unlike others, In this case higher the number higher
  // the priority. That's why '<' is used here
  //
  return lps.ptfRank() < rps.ptfRank();
}

bool
FPPQItem::LowerFare::
operator()(const FPPQItem* lhs, const FPPQItem* rhs) const
{
  const PriorityStatus& lps = lhs->priorityStatus();
  const PriorityStatus& rps = rhs->priorityStatus();

  const int lmileage = lhs->_farePath->mileage();
  const int rmileage = rhs->_farePath->mileage();
  if (((lmileage != 0) || (rmileage != 0)) && (lmileage != rmileage))
  {
    return lmileage < rmileage;
  }

  MoneyAmount diff = rhs->_farePath->getNUCAmountScore() - lhs->_farePath->getNUCAmountScore();

  if (diff != 0)
  {
    if (diff > EPSILON)
    {
      return true;
    }
    else if (-diff > EPSILON)
    {
      return false;
    }
  }

  const CabinType& leftCabin = lhs->getCabinPriority();
  const CabinType& rightCabin = rhs->getCabinPriority();

  if (_cabinComparator.shouldCompareCabins() &&
      !_cabinComparator.areCabinsEqual(leftCabin, rightCabin))
  {
    return _cabinComparator.isLessCabin(leftCabin, rightCabin);
  }

  if (lhs->getNumberOfStopsPriority() != rhs->getNumberOfStopsPriority())
  {
    return lhs->getNumberOfStopsPriority() > rhs->getNumberOfStopsPriority();
  }

  // total price is equal
  if (lps.fareByRulePriority() != rps.fareByRulePriority())
  {
    return lps.fareByRulePriority() > rps.fareByRulePriority();
  }

  if (lps.paxTypeFarePriority() != rps.paxTypeFarePriority())
  {
    return lps.paxTypeFarePriority() > rps.paxTypeFarePriority();
  }

  // check price without plus ups
  MoneyAmount lhsBaseAmount =
      (lhs->_farePath->getTotalNUCAmount() - lhs->_farePath->plusUpAmount()) +
      lhs->_farePath->rexChangeFee();

  MoneyAmount rhsBaseAmount =
      (rhs->_farePath->getTotalNUCAmount() - rhs->_farePath->plusUpAmount()) +
      rhs->_farePath->rexChangeFee();

  diff = rhsBaseAmount - lhsBaseAmount;

  if (diff != 0)
  {
    if (diff > EPSILON)
    {
      return true;
    }
    else if (-diff > EPSILON)
    {
      return false;
    }
  }

  const size_t lhsPUCount = lhs->_farePath->pricingUnit().size();
  const size_t rhsPUCount = rhs->_farePath->pricingUnit().size();
  if (lhsPUCount != rhsPUCount)
  {
    return lhsPUCount < rhsPUCount;
  }

  if (lps.fareCxrTypePriority() != rps.fareCxrTypePriority())
  {
    return lps.fareCxrTypePriority() > rps.fareCxrTypePriority();
  }

  if (lps.negotiatedFarePriority() != rps.negotiatedFarePriority())
  {
    return lps.negotiatedFarePriority() > rps.negotiatedFarePriority();
  }

  // Unlike others, In this case higher the number higher
  // the priority. That's why '<' is used here
  //
  return lps.ptfRank() < rps.ptfRank();
}

void
FPPQItem::updateCabinPriority(const CabinType& newCabin)
{
  if (!newCabin.isValidCabin())
    return;

  if (_cabinPriority.isUndefinedClass())
    _cabinPriority = newCabin;
  else if (_cabinPriority < newCabin)
    _cabinPriority = newCabin;
}

const uint16_t&
FPPQItem::getFlexFaresGroupId() const
{
  return puPath()->getFlexFaresGroupId();
}

bool
FPPQItem::isEqualAmountComponents(const FPPQItem& rhs) const
{
  if (_farePath && rhs.farePath() && _farePath->isEqualAmountComponents(*(rhs.farePath())))
    return fabs(_farePath->yqyrNUCAmount() - rhs.farePath()->yqyrNUCAmount()) < EPSILON;

  return false;
}

FPPQItem*
FPPQItem::clone(FarePathFactoryStorage& storage)
{
  FPPQItem* newItem = &storage.constructFPPQItem();
  *newItem = *this;
  newItem->farePathFactory() = _farePathFactory;
  newItem->farePath() = _farePath->clone(_puPath, storage);

  return newItem;
}

bool
FPPQItem::needRecalculateCat12() const
{
  TSE_ASSERT(_farePath);
  return _farePath->needRecalculateCat12();
}

void
FPPQItem::reuseSurchargeData() const
{
  _farePath->reuseSurchargeData();
}
}
