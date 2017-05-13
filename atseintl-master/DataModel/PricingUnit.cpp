//-------------------------------------------------------------------
//
//  File:        PricingUnit.cpp
//  Created:
//  Authors:     Kavya Katam
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/PricingUnit.h"

#include "DataModel/FareUsage.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleConst.h"


#include <algorithm>
#include <numeric>

namespace tse
{
PricingUnit::PricingUnit()
  : _earliestTktDT(DateTime::openDate()), _latestTktDT(DateTime::openDate())
{
  // these vectors are generally grown to a smallish size. We want
  // this to happen efficiently, so we reserve a small number of items
  // in the vectors to make sure not too many reallocations occur.
  _fareUsages.reserve(8);
  _travelSeg.reserve(8);
} // lint !e1401

bool
PricingUnit::hasMultipleCurrency() const
{
  CurrencyCode puCurr("");
  return hasMultipleCurrency(puCurr);
}

bool
PricingUnit::hasMultipleCurrency(CurrencyCode& paxTypeFarecurr) const
{
  std::vector<FareUsage*>::const_iterator it = _fareUsages.begin();
  std::vector<FareUsage*>::const_iterator itEnd = _fareUsages.end();

  for (; it != itEnd; ++it)
  {
    if (LIKELY(!(*it)->paxTypeFare()->isDummyFare()))
      break;
  }

  if (it == itEnd)
    return false;

  if (!paxTypeFarecurr.empty())
  {
    if (paxTypeFarecurr != (*it)->paxTypeFare()->currency())
      return true;
  }
  else
    paxTypeFarecurr = (*it)->paxTypeFare()->currency();
  // Apart from checking currencies between paxtypefares, we will also check if the fare has
  // an add on and if that currency is different from paxtypefare currency
  if ((*it)->paxTypeFare()->isConstructed())
  {
    const ConstructedFareInfo* cfi = (*it)->paxTypeFare()->fare()->constructedFareInfo();
    if (cfi != nullptr)
    {
      if ((!cfi->origAddonCurrency().empty() && paxTypeFarecurr != cfi->origAddonCurrency()) ||
          (!cfi->destAddonCurrency().empty() && paxTypeFarecurr != cfi->destAddonCurrency()))
        return true;
    }
  }
  ++it;

  for (; it != itEnd; ++it)
  {
    if ((*it)->paxTypeFare()->isDummyFare())
      continue;

    if (paxTypeFarecurr != (*it)->paxTypeFare()->currency())
      return true;

    if ((*it)->paxTypeFare()->isConstructed())
    {
      const ConstructedFareInfo* cfi = (*it)->paxTypeFare()->fare()->constructedFareInfo();
      if (cfi != nullptr)
      {
        if ((!cfi->origAddonCurrency().empty() && paxTypeFarecurr != cfi->origAddonCurrency()) ||
            (!cfi->destAddonCurrency().empty() && paxTypeFarecurr != cfi->destAddonCurrency()))
          return true;
      }
    }
  }
  return false;
}

// Get only first PaxtypeFare Currency
CurrencyCode
PricingUnit::getFirstPaxTypeFareCurrency() const
{
  for (const FareUsage* fu : fareUsage())
  {
    if (fu->paxTypeFare()->isDummyFare())
      continue;
    return fu->paxTypeFare()->currency();
  }
  return CurrencyCode("");
}

bool
PricingUnit::isCmdPricing()
{
  if (_puIsCmdPricing != CP_UNKNOWN)
  {
    if (LIKELY(_puIsCmdPricing == NO_CMD_PRICING))
      return false;
    else
      return true;
  }

  // see if there is any fare market requiring cmd pricing
  if (UNLIKELY(_fareUsages.empty()))
  {
    // not initialized yet, return false and keep UNKNOWN
    return false;
  }
  for (const FareUsage* fu : fareUsage())
  {
    if (UNLIKELY(fu->paxTypeFare()->isCmdPricing()))
    {
      _puIsCmdPricing = IS_CMD_PRICING;
      return true;
    }
  }
  _puIsCmdPricing = NO_CMD_PRICING;
  return false;
}

bool
PricingUnit::setCmdPrcFailedFlag(const unsigned int category, const bool isFailed /* = true*/)
{
  if (category < (PaxTypeFare::PTFF_Max_Numbered + 1))
  {
    return _cpFailedStatus.set(
        (PaxTypeFare::CmdPricingFailedState)(((uint32_t)0x00000001) << (category - 1)), isFailed);
  }
  else if (category == 35)
  {
    return _cpFailedStatus.set(PaxTypeFare::PTFF_Cat35, isFailed);
  }
  return false;
}

bool
PricingUnit::isMirrorImage() const
{
  if (puType() != Type::ROUNDTRIP)
    return false;

  const Fare& fare1 = *_fareUsages[0]->paxTypeFare()->fare();
  const Fare& fare2 = *_fareUsages[1]->paxTypeFare()->fare();

  if (_geoTravelType == GeoTravelType::International)
  {
    return (fare1.carrier() == fare2.carrier() && fare1.fareClass() == fare2.fareClass() &&
            fare1.ruleNumber() == fare2.ruleNumber() &&
            fare1.tcrRuleTariff() == fare2.tcrRuleTariff() && fare1.owrt() == fare2.owrt());
  }
  else
  {
    return (fare1.carrier() == fare2.carrier() && fare1.fareClass() == fare2.fareClass() &&
            fare1.ruleNumber() == fare2.ruleNumber() &&
            fare1.tcrRuleTariff() == fare2.tcrRuleTariff() && fare1.carrier() != CarrierCode("*J"));
  }
}

PricingUnit*
PricingUnit::clone(DataHandle& dataHandle) const
{
  PricingUnit* res = dataHandle.create<PricingUnit>();
  *res = *this;
  return res;
}

bool
PricingUnit::isTravelSegPartOfPricingUnit(const TravelSeg* tvlSeg) const
{
  return std::any_of(_fareUsages.cbegin(),
                     _fareUsages.cend(),
                     [tvlSeg](const FareUsage* fareUsage)
                     { return fareUsage->hasTravelSeg(tvlSeg); });
}

const FareUsage*
PricingUnit::getFareUsageWithFirstTravelSeg(const TravelSeg* travelSeg) const
{
  auto const& fUVec = _fareUsages;
  auto fareUsageIt = std::find_if(fUVec.cbegin(),
                                  fUVec.cend(),
                                  [travelSeg](const FareUsage* fareUsage)
                                  {
    TSE_ASSERT(!fareUsage->travelSeg().empty());
    return (travelSeg == fareUsage->travelSeg().front());
  });
  if (fareUsageIt != fUVec.cend())
    return *fareUsageIt;

  return nullptr;
}

MoneyAmount
PricingUnit::getDynamicPriceDeviationForLeg(int16_t legId) const
{
  auto op = [legId](MoneyAmount sum, const FareUsage* fu) -> MoneyAmount
  { return sum += (fu->travelSeg().front()->legId() == legId) ? -fu->getDiscAmount() : 0; };

  return std::accumulate(_fareUsages.cbegin(), _fareUsages.cend(), MoneyAmount(0), op);
}

bool
PricingUnit::isRebookedClassesStatus()
{
  for (FareUsage* fu : fareUsage())
  {
    PaxTypeFare& paxTypeFare = *fu->paxTypeFare();
    if (paxTypeFare.bkgCodeTypeForRex() == PaxTypeFare::BKSS_NOT_YET_PROCESSED)
      paxTypeFare.bkgCodeTypeForRex() = paxTypeFare.getRebookedClassesStatus();

    if (paxTypeFare.bkgCodeTypeForRex() == PaxTypeFare::BKSS_REBOOKED)
      return true;
  }
  return false;
}

bool
PricingUnit::isAnyFareUsageAcrossTurnaroundPoint() const
{
  return std::any_of(_fareUsages.cbegin(),
                     _fareUsages.cend(),
                     [](const FareUsage* fu)
                     { return fu->isAcrossTurnaroundPoint(); });
}

void
PricingUnit::copyBkgStatusForEachFareUsageFromPaxTypeFare()
{
  for (FareUsage* fu : fareUsage())
    fu->copyBkgStatusFromPaxTypeFare();
}

bool
PricingUnit::needRecalculateCat12() const
{
  return std::any_of(_fareUsages.cbegin(),
                     _fareUsages.cend(),
                     [](const FareUsage* fu)
                     { return fu->needRecalculateCat12(); });
}

void
PricingUnit::reuseSurchargeData() const
{
  for (const auto fu : _fareUsages)
    fu->reuseSurchargeData();
}

bool
PricingUnit::isEqualAmountComponents(const PricingUnit& rhs) const
{
  return _fareUsages.size() == rhs._fareUsages.size() &&
         std::equal(_fareUsages.cbegin(),
                    _fareUsages.cend(),
                    rhs._fareUsages.cbegin(),
                    [](const FareUsage* const lhs, const FareUsage* const rhs)
                    { return lhs->isEqualAmountComponents(*rhs); });
}

bool
PricingUnit::isADDatePassValidation(const DatePair& altDatePair) const
{
  return std::all_of(_fareUsages.cbegin(),
                     _fareUsages.cend(),
                     [&altDatePair](const FareUsage* const fu)
                     { return fu->isADDatePassValidation(altDatePair); });
}

bool
PricingUnit::isPUWithSameFares(const PricingUnit& rhs) const
{
  return (_fareUsages.size() == rhs._fareUsages.size()) &&
         std::equal(_fareUsages.cbegin(),
                    _fareUsages.cend(),
                    rhs._fareUsages.cbegin(),
                    [](const FareUsage* const lhs, const FareUsage* const rhs)
                    { return lhs->isSamePaxTypeFare(*rhs); });
}

void
PricingUnit::clearReusedFareUsage()
{
  for (const auto fareU : _fareUsages)
    fareU->clearReusedFareUsage();
}

bool
PricingUnit::areAllFaresNormal() const
{
  return std::all_of(_fareUsages.cbegin(),
                     _fareUsages.cend(),
                     [](const FareUsage* const fu)
                     { return fu->isPaxTypeFareNormal(); });
}

void
PricingUnit::accumulateBaggageLowerBound(MoneyAmount lb)
{
  _baggageLowerBound += lb;
  _totalPuNUCAmount += lb;
}

void
PricingUnit::rollbackBaggageLowerBound()
{
  _totalPuNUCAmount -= _baggageLowerBound;
  _baggageLowerBound = 0;
}

std::ostream&
operator<<(std::ostream& os, PricingUnit::Type puType)
{
  return os << static_cast<uint16_t>(puType);
}

} // tse namespace
