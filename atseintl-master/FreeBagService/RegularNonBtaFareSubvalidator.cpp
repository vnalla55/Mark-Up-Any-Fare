//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "FreeBagService/RegularNonBtaFareSubvalidator.h"

#include "Common/Assert.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "Rules/RuleConst.h"
#include "Util/IteratorRange.h"

namespace tse
{
namespace
{
const ServiceRuleTariffInd RULE_TARIFF_IND_PUBLIC = "PUB";
const ServiceRuleTariffInd RULE_TARIFF_IND_PRIVATE = "PRI";

bool
validateFare(ServiceRuleTariffInd ruleTariffInd, const PaxTypeFare& ptf)
{
  const bool isFarePublic = ptf.tcrTariffCat() != RuleConst::PRIVATE_TARIFF;

  return isFarePublic ? (ruleTariffInd == RULE_TARIFF_IND_PUBLIC)
                      : (ruleTariffInd == RULE_TARIFF_IND_PRIVATE);
}
}

void
RegularNonBtaFareSubvalidator::onFaresUpdated(const FarePath* fp)
{
  _fp = fp;
  _fares.clear();
  _allFaresOnBtKnown = true;
}

StatusS7Validation
RegularNonBtaFareSubvalidator::validate(const OptionalServicesInfo& s7, OCFees& ocFees)
{
  if (_fares.empty())
    prepareFareInfo();

  if (!checkRuleTariffInd(s7))
    return FAIL_S7_RULE_TARIFF_IND;

  if (!checkTourCode(s7))
    return FAIL_S7_TOURCODE;

  return PASS_S7;
}

// The method for revalidating items that had been conditionally passed by
// SoftPassNonBtaSubvalidator. Note that it should work correctly even if
// not all fare-related information are available and return SOFT_PASS_S7
// in such a case.

StatusS7Validation
RegularNonBtaFareSubvalidator::revalidate(const OptionalServicesInfo& s7)
{
  bool conditionalPass = false;

  if (!s7.ruleTariffInd().empty())
  {
    if (_fares.empty())
      prepareFareInfo();
    if (!_allFaresOnBtKnown)
      conditionalPass = true;
    if (!checkRuleTariffInd(s7))
      return FAIL_S7_RULE_TARIFF_IND;
  }

  if (!s7.tourCode().empty())
  {
    if (!_fp)
      conditionalPass = true;
    else if (!checkTourCode(s7))
      return FAIL_S7_TOURCODE;
  }

  return conditionalPass ? SOFT_PASS_S7 : PASS_S7;
}

void
RegularNonBtaFareSubvalidator::prepareFareInfo()
{
  for (TravelSeg* ts : makeIteratorRange(_bt.getTravelSegBegin(), _bt.getTravelSegEnd()))
  {
    if (!ts->isAir())
      continue;
    const auto t2ssI = _ts2ss.find(ts);
    if (t2ssI != _ts2ss.end())
      _fares.insert(t2ssI->second.second);
    else
      _allFaresOnBtKnown = false;
  }
}

inline bool
RegularNonBtaFareSubvalidator::checkRuleTariffInd(const OptionalServicesInfo& s7) const
{
  if (s7.ruleTariffInd().empty())
    return true;

  return std::all_of(_fares.begin(),
                     _fares.end(),
                     [&](PaxTypeFare* ptf)
                     { return validateFare(s7.ruleTariffInd(), *ptf); });
}

inline bool
RegularNonBtaFareSubvalidator::checkTourCode(const OptionalServicesInfo& s7) const
{
  TSE_ASSERT(_fp);

  if (s7.tourCode().empty())
    return true;

  // check cat 35 tour code
  const CollectedNegFareData* const negFareData = _fp->collectedNegFareData();
  if (negFareData && negFareData->indicatorCat35() && !negFareData->tourCode().empty())
    return s7.tourCode() == negFareData->tourCode();

  // check cat 27 tour code
  if (!_fp->cat27TourCode().empty())
    return s7.tourCode() == _fp->cat27TourCode();

  // check input tour code
  const std::string& inputTourCode = _bt._trx->getRequest()->getTourCode();
  return !inputTourCode.empty() && s7.tourCode() == inputTourCode;
}
}
