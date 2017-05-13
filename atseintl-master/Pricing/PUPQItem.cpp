//
// Copyright Sabre 2012-02-15
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
#include "Pricing/PUPQItem.h"

#include "DataModel/FareUsage.h"

namespace tse
{

bool
PUPQItem::cloneFareUsage(PricingTrx& trx, FareUsagePool& fuPool)
{
  std::vector<FareUsage*>::iterator it = _pricingUnit->fareUsage().begin();
  const std::vector<FareUsage*>::iterator itEnd = _pricingUnit->fareUsage().end();
  for (; it != itEnd; ++it)
  {
    FareUsage* fareU = fuPool.construct();
    if (UNLIKELY(fareU == nullptr))
      return false;

    *fareU = *(*it); // using operator=() of FareUsage
    *it = fareU;
  }
  return true;
}

bool
PUPQItem::GreaterFare::
operator()(const PUPQItem* lhs, const PUPQItem* rhs) const
{
  const PriorityStatus& lps = lhs->priorityStatus();
  const PriorityStatus& rps = rhs->priorityStatus();

  const int lmileage = lhs->_pricingUnit->mileage();
  const int rmileage = rhs->_pricingUnit->mileage();
  if (UNLIKELY(((lmileage != 0) || (rmileage != 0)) && (lmileage != rmileage)))
  {
    return lmileage > rmileage;
  }

  MoneyAmount diff =
      lhs->_pricingUnit->getTotalPuNucAmount() - rhs->_pricingUnit->getTotalPuNucAmount();
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

  // price is equal
  if (lps.fareByRulePriority() != rps.fareByRulePriority())
  {
    return lps.fareByRulePriority() > rps.fareByRulePriority();
  }

  if (lps.paxTypeFarePriority() != rps.paxTypeFarePriority())
  {
    return lps.paxTypeFarePriority() > rps.paxTypeFarePriority();
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
PUPQItem::LowerFare::
operator()(const PUPQItem* lhs, const PUPQItem* rhs) const
{
  const PriorityStatus& lps = lhs->priorityStatus();
  const PriorityStatus& rps = rhs->priorityStatus();

  const int lmileage = lhs->_pricingUnit->mileage();
  const int rmileage = rhs->_pricingUnit->mileage();
  if (((lmileage != 0) || (rmileage != 0)) && (lmileage != rmileage))
  {
    return lmileage < rmileage;
  }

  MoneyAmount diff =
      rhs->_pricingUnit->getTotalPuNucAmount() - lhs->_pricingUnit->getTotalPuNucAmount();
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

  if (_cabinComparator.shouldCompareCabins() &&
      !_cabinComparator.areCabinsEqual(leftCabin, rightCabin))
  {
    return _cabinComparator.isLessCabin(leftCabin, rightCabin);
  }

  // price is equal
  if (lps.fareByRulePriority() != rps.fareByRulePriority())
  {
    return lps.fareByRulePriority() > rps.fareByRulePriority();
  }

  if (lps.paxTypeFarePriority() != rps.paxTypeFarePriority())
  {
    return lps.paxTypeFarePriority() > rps.paxTypeFarePriority();
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
PUPQItem::updateCabinPriority(const CabinType& newCabin)
{
  if (!newCabin.isValidCabin())
    return;

  if (_cabinPriority.isUndefinedClass())
    _cabinPriority = newCabin;
  else if (_cabinPriority < newCabin)
    _cabinPriority = newCabin;
}
}
