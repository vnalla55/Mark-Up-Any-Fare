//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "MinFares/MinFareFareSelection.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "MinFares/MatchRuleLevelExclTable.h"
#include "MinFares/MinimumFare.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <iomanip>

namespace tse
{
namespace
{
ConfigurableValue<bool>
minfareValDiscountRule("PRICING_SVC", "MINFARE_VALIDATE_DISCOUNT_RULE", false);
}
MinimumFareModule&
MinFareFareSelection::module()
{
  return _module;
}

bool
MinFareFareSelection::AscendingOrder::
operator()(const PaxTypeFare* paxFare1, const PaxTypeFare* paxFare2) const
{
  if (UNLIKELY(paxFare1 == nullptr || paxFare2 == nullptr))
    return false;

  const Fare* fare1 = paxFare1->fare();
  const Fare* fare2 = paxFare2->fare();

  if (UNLIKELY(fare1 == nullptr || fare2 == nullptr))
    return false;

  return fare1->nucFareAmount() < fare2->nucFareAmount();
}

const std::string MinFareFareSelection::MinFareModuleName[] = {
    "HIP", "BHC", "CTM", "COM", "DMC", "COP", "OSC", "RSC", "CPM", "OJM"};

const std::string MinFareFareSelection::FareDirStr[] = {"O", "I", "B"};

MinFareFareSelection::ApplLogicOrder::ApplLogicOrder(const PaxTypeFare* thruFare,
                                                     bool compRbd,
                                                     bool compStopOver)
  : _thruFare(thruFare), _compRbd(compRbd), _compStopOver(compStopOver)
{
}

bool
MinFareFareSelection::ApplLogicOrder::
operator()(const PaxTypeFare* fare1, const PaxTypeFare* fare2) const
{
  if (fare1 == nullptr || fare2 == nullptr)
    return false;

  if (_thruFare == nullptr)
    return fare1->nucFareAmount() < fare2->nucFareAmount();

  if (_compRbd)
  {
    bool fare1SameFareCls = (fare1->fareClass()[0] == _thruFare->fareClass()[0]);
    bool fare2SameFareCls = (fare2->fareClass()[0] == _thruFare->fareClass()[0]);
    if (fare1SameFareCls != fare2SameFareCls) // Only one fare has same 1st char as thru fare.
      return fare1SameFareCls;
  }

  if (_compStopOver)
  {
    uint16_t thruFareStopOver = _thruFare->maxStopoversPermitted();
    uint16_t fare1Diff = abs(fare1->maxStopoversPermitted() - thruFareStopOver);
    uint16_t fare2Diff = abs(fare2->maxStopoversPermitted() - thruFareStopOver);
    if (fare1Diff != fare2Diff)
      return (fare1Diff < fare2Diff);
  }

  return fare1->nucFareAmount() < fare2->nucFareAmount();
}

bool
MinFareFareSelection::isEligibleFare(const PaxTypeFare& fare,
                                     const PaxTypeFare& thruFare,
                                     MinFareFareSelection::EligibleFare eligibleFare)
{
  if (thruFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) // Tag 3
    return fare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED; // Use tag 1 or 3

  switch (eligibleFare)
  {
  case ONE_WAY: // Use tag 1 and 3
    return (fare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED);

  case HALF_ROUND_TRIP: // Use tag 1 and 2
    return (fare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED);

  default:
    return false;
  }
}

MinFareFareSelection::MinFareFareSelection(MinimumFareModule module,
                                           EligibleFare eligibleFare,
                                           FareDirectionChoice fareDirection,
                                           PricingTrx& trx,
                                           const Itin& itin,
                                           const std::vector<TravelSeg*>& travelSegs,
                                           const std::vector<PricingUnit*>& pricingUnits,
                                           const PaxType* paxType,
                                           const DateTime& travelDate,
                                           const FarePath* farePath,
                                           const PaxTypeFare* thruFare,
                                           const MinFareAppl* minFareAppl,
                                           const MinFareDefaultLogic* minFareDefaultLogic,
                                           const RepricingTrx* repricingTrx,
                                           const PaxTypeCode& actualPaxType)
  : _module(module),
    _eligibleFare(eligibleFare),
    _fareDirection(fareDirection),
    _trx(trx),
    _itin(itin),
    _travelSegs(travelSegs),
    _pricingUnits(pricingUnits),
    _paxType(paxType),
    _travelDate(travelDate),
    _farePath(farePath),
    _thruFare(thruFare),
    _minFareAppl(minFareAppl),
    _minFareDefaultLogic(minFareDefaultLogic),
    _repricingTrx(repricingTrx),
    _actualPaxType(actualPaxType),
    _govCxr(""),
    _globalDirection(GlobalDirection::XX),
    _diag(nullptr),
    _yyOverride(false),
    _exemptRuleVal(false),
    _isNetRemit(false)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic710)
  {
    bool createDiag = true;
    std::map<std::string, std::string>::const_iterator i =
        trx.diagnostic().diagParamMap().find("DD");
    std::map<std::string, std::string>::const_iterator ifm =
        trx.diagnostic().diagParamMap().find("FM");

    if ((i != trx.diagnostic().diagParamMap().end() && i->second != MinFareModuleName[module]) ||
        (ifm != trx.diagnostic().diagParamMap().end() &&
         ifm->second != _travelSegs.front()->boardMultiCity() + _travelSegs.back()->offMultiCity()))
      createDiag = false;

    if (createDiag)
    {
      DCFactory* factory = DCFactory::instance();
      _diag = factory->create(trx);
      if (_diag != nullptr)
      {
        _diag->enable(Diagnostic710);
        if (!_diag->isActive())
        {
          _diag = nullptr;
        }
      }
    }
  }

  if (!_travelSegs.empty())
  {
    _origin = _travelSegs.front()->boardMultiCity();
    _dest = _travelSegs.back()->offMultiCity();
  }

  if ((_thruFare != nullptr) && (_thruFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
    _isThruFareOw = true;
  else
    _isThruFareOw = false;

  _paxTypeStatus = PaxTypeUtil::paxTypeStatus(_paxType->paxType(), _paxType->vendorCode());
  if (_paxTypeStatus == PAX_TYPE_STATUS_UNKNOWN)
    _paxTypeStatus = PAX_TYPE_STATUS_ADULT;

  _selPaxTypeStatus = _paxTypeStatus;

  if (_farePath && (dynamic_cast<const NetRemitFarePath*>(_farePath) != nullptr))
  {
    _isNetRemit = true;
  }
}

bool
MinFareFareSelection::isCxrAllowYYFare(const CarrierCode& govCxr,
                                       const std::vector<TravelSeg*>& travelSegs)
{
  const std::vector<const IndustryPricingAppl*>& ipaList =
      _trx.dataHandle().getIndustryPricingAppl(govCxr, _globalDirection, _travelDate);
  std::vector<const IndustryPricingAppl*>::const_iterator i = ipaList.begin();
  for (; i != ipaList.end(); i++)
  {
    const IndustryPricingAppl& ipa = **i;
    if (!matchDirectionality(ipa.directionality(), ipa.loc1(), ipa.loc2(), travelSegs))
      continue;

    if (ipa.minimumFareAppl() == INCLUDE)
      return true;
    else
      return false;
  }

  return true;
}

bool
MinFareFareSelection::matchDirectionality(const Directionality& directionality,
                                          const LocKey& loc1,
                                          const LocKey& loc2,
                                          const std::vector<TravelSeg*>& tvlSegs)
{
  const Loc* setOrigin = MinimumFare::origin(tvlSegs);
  const Loc* setDest = MinimumFare::destination(tvlSegs);

  if (UNLIKELY(setOrigin == nullptr || setDest == nullptr))
    return false;

  switch (directionality)
  {
  case FROM:
    return (LocUtil::isInLoc(*setOrigin,
                             loc1.locType(),
                             loc1.loc(),
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             GeoTravelType::International,
                             EMPTY_STRING(),
                             _trx.getRequest()->ticketingDT()) &&
            LocUtil::isInLoc(*setDest,
                             loc2.locType(),
                             loc2.loc(),
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             GeoTravelType::International,
                             EMPTY_STRING(),
                             _trx.getRequest()->ticketingDT()));

  case BETWEEN:
    if (loc1.locType() == MinimumFare::BLANK)
      return true;

    if (loc2.locType() == MinimumFare::BLANK) // Means FROM/TO loc1 TO/FROM anywhere
    {
      return (LocUtil::isInLoc(*setOrigin,
                               loc1.locType(),
                               loc1.loc(),
                               Vendor::SABRE,
                               MANUAL,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               _trx.getRequest()->ticketingDT()) ||
              LocUtil::isInLoc(*setDest,
                               loc1.locType(),
                               loc1.loc(),
                               Vendor::SABRE,
                               MANUAL,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               _trx.getRequest()->ticketingDT()));
    }
    else
    {
      return MinimumFare::isBetweenLocs(
          _trx, *setOrigin, *setDest, loc1.locType(), loc1.loc(), loc2.locType(), loc2.loc());
    }

  case WITHIN:
    if (loc1.locType() == MinimumFare::BLANK)
      return true;

    return MinimumFare::isWithinLoc(_trx, tvlSegs, loc1.locType(), loc1.loc());

  default:
    return true;
  }
}

MinFareFareSelection::ValidStatus
MinFareFareSelection::validateFare(const PaxTypeFare& fare)
{
  if (UNLIKELY(_exemptRuleVal)) // Net Remit ticketed fare exempts rule validation
    return VALIDATED;

  ValidStatus status = NONE;
  std::vector<uint16_t> puCategories;
  std::vector<uint16_t> fmCategories;

  const bool validateDiscRule = minfareValDiscountRule.getValue();

  switch (_module)
  {
  case NCJ:
  {
    if (_thruFare->tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
        fare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    {
      break;
    }

    status = VALIDATED;
  }
  break;
  case HIP:
  case BHC:
  case CPM:
  {
    if (_module == HIP || _module == BHC)
    {
      if (_thruFare != nullptr && _thruFare->tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
          fare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
      {
        break;
      }
    }

    fmCategories.push_back(RuleConst::ELIGIBILITY_RULE);

    if (!fare.isDiscounted() || fare.discountInfo().category() != 19)
    {
      fmCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
    }

    fmCategories.push_back(RuleConst::FLIGHT_APPLICATION_RULE);
    fmCategories.push_back(RuleConst::MISC_FARE_TAG);

    puCategories.push_back(RuleConst::SEASONAL_RULE);
    puCategories.push_back(RuleConst::DAY_TIME_RULE);

    if (UNLIKELY(_module == CPM))
      puCategories.push_back(RuleConst::ADVANCE_RESERVATION_RULE);

    // PU scope
    if (UNLIKELY(validateDiscRule))
    {
      puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);
    }
    puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    // FM scope
    fmCategories.push_back(RuleConst::BLACKOUTS_RULE);
    fmCategories.push_back(RuleConst::STOPOVER_RULE);
    fmCategories.push_back(RuleConst::TRANSFER_RULE);


    if ((fare.isRoutingMapValid() || !fare.isRoutingProcessed() || fare.isRoutingValid() ||
         (!fare.isRoutingValid() && fare.isRoutingInvalidByMPM())) &&
        ruleValidated(fmCategories, fare) && ruleValidated(puCategories, fare, true))
      status = VALIDATED;
  }
  break;
  case COM:
  case DMC:
  {
    // PU scope
    puCategories.push_back(RuleConst::SEASONAL_RULE);
    puCategories.push_back(RuleConst::DAY_TIME_RULE);
    if (validateDiscRule)
    {
      puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);
    }
    puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    // FM scope
    fmCategories.push_back(RuleConst::ELIGIBILITY_RULE);
    fmCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
    fmCategories.push_back(RuleConst::BLACKOUTS_RULE);
    fmCategories.push_back(RuleConst::MISC_FARE_TAG);

    // We need this for inbound DMC CAT2
    // if (_module == DMC)
    //   fmCategories.push_back(RuleConst::DAY_TIME_RULE);

    if (ruleValidated(fmCategories, fare) && ruleValidated(puCategories, fare, true))
      status = VALIDATED;
  }
  break;

  case OJM:
  {
    // OJM needs only select fare with same fare basis code.
    if ((_thruFare != nullptr) && (_thruFare->fareClass() != fare.fareClass()))
      return status;

    // PU scope
    puCategories.push_back(RuleConst::SEASONAL_RULE);
    puCategories.push_back(RuleConst::DAY_TIME_RULE);
    if (validateDiscRule)
    {
      puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);
    }
    puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    // FM scope
    fmCategories.push_back(RuleConst::ELIGIBILITY_RULE);
    fmCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
    fmCategories.push_back(RuleConst::BLACKOUTS_RULE);
    fmCategories.push_back(RuleConst::MISC_FARE_TAG);

    if (ruleValidated(fmCategories, fare) && ruleValidated(puCategories, fare, true))
      status = VALIDATED;
  }
  break;

  case CTM:
  case COP:
  case RSC:
  {
    // PU scope
    puCategories.push_back(RuleConst::SEASONAL_RULE);
    puCategories.push_back(RuleConst::DAY_TIME_RULE);
    if (validateDiscRule)
    {
      puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);
    }
    puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    // FM scope
    fmCategories.push_back(RuleConst::ELIGIBILITY_RULE);

    if (!fare.isDiscounted() || fare.discountInfo().category() != 19)
    {
      fmCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
    }

    fmCategories.push_back(RuleConst::BLACKOUTS_RULE);
    fmCategories.push_back(RuleConst::MISC_FARE_TAG);

    if (ruleValidated(fmCategories, fare) && ruleValidated(puCategories, fare, true))
      status = VALIDATED;
  }
  break;
  case OSC:
  {
    // PU scope
    puCategories.push_back(RuleConst::SEASONAL_RULE);
    puCategories.push_back(RuleConst::DAY_TIME_RULE);
    if (validateDiscRule)
    {
      puCategories.push_back(RuleConst::CHILDREN_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::TOUR_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::AGENTS_DISCOUNT_RULE);
      puCategories.push_back(RuleConst::OTHER_DISCOUNT_RULE);
    }
    puCategories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    // FM scope
    fmCategories.push_back(RuleConst::ELIGIBILITY_RULE);
    fmCategories.push_back(RuleConst::SALE_RESTRICTIONS_RULE);
    fmCategories.push_back(RuleConst::BLACKOUTS_RULE);
    fmCategories.push_back(RuleConst::MISC_FARE_TAG);
    fmCategories.push_back(RuleConst::STOPOVER_RULE);
    fmCategories.push_back(RuleConst::TRANSFER_RULE);

    if (ruleValidated(fmCategories, fare) && ruleValidated(puCategories, fare, true))
      status = VALIDATED;
  }
  break;
  default:
    break;
  }
  return status;
}

bool
MinFareFareSelection::ruleValidated(const std::vector<uint16_t>& categories,
                                    const PaxTypeFare& paxTypeFare,
                                    bool puScope)
{
  if (puScope && !_pricingUnits.empty() && (_farePath != nullptr)) // PU scope rule revalidation
  {
    // Create a dummy FareUsage
    FareUsage fareUsage;
    fareUsage.paxTypeFare() = const_cast<PaxTypeFare*>(&paxTypeFare);

    PricingUnit& pu =
        *_pricingUnits.front(); // Temp, enhancement maybe needed to accept multiple PUs.

    if (UNLIKELY(_module == DMC))
    {
      if ((_fareDirection == INBOUND &&
           paxTypeFare.fareMarket()->direction() == FMDirection::OUTBOUND) ||
          (_fareDirection == OUTBOUND &&
           paxTypeFare.fareMarket()->direction() == FMDirection::INBOUND))
      {
        // reversed Fare Collection, need revered travel segments for
        // Geo table process
        pu.fareDirectionReversed() = true;
        fareUsage.fareDirectionReversed() = true;
        fareUsage.inbound() = !(_fareDirection == INBOUND);
      }
      else
        fareUsage.inbound() = (_fareDirection == INBOUND);
    }
    else
      fareUsage.inbound() = (_fareDirection == INBOUND);

    RuleControllerWithChancelor<PricingUnitRuleController> ruleController(
        DynamicValidation, categories, &_trx);

    pu.dynamicValidationPhase() = true;
    bool result = ruleController.validate(_trx, *(const_cast<FarePath*>(_farePath)), pu, fareUsage);

    pu.dynamicValidationPhase() = false;

    if (UNLIKELY(_module == DMC))
    {
      pu.fareDirectionReversed() = false;
      fareUsage.fareDirectionReversed() = false;
    }

    return result;
  }
  else
  {
    RuleControllerWithChancelor<FareMarketRuleController> ruleController(
        DynamicValidation, categories, &_trx);
    return ruleController.validate(
        _trx, const_cast<Itin&>(_itin), const_cast<PaxTypeFare&>(paxTypeFare));
  }
}

bool
MinFareFareSelection::isRightDirection(const PaxTypeFare& fare)
{
  switch (_fareDirection)
  {
  case OUTBOUND:
    return ((fare.directionality() == FROM) || fare.directionality() == tse::BOTH);

  case INBOUND:
    return (fare.directionality() == TO || fare.directionality() == tse::BOTH);

  default:
    return true;
  }
}

bool
MinFareFareSelection::isRightPaxType(const PaxTypeFare& fare)
{
  if (_paxTypeStatus != PAX_TYPE_STATUS_ADULT)
  {
    if (fare.fcasPaxType().empty()) // This is adult fare.
    {
      if ((!_actualPaxType.empty()) && (_actualPaxType == ADULT))
        return true;

      if (_selPaxTypeStatus != PAX_TYPE_STATUS_ADULT)
        return false;
    }
    else
    {
      PaxTypeStatus paxTypeStatus = PaxTypeUtil::paxTypeStatus(fare.fcasPaxType(), fare.vendor());
      if (paxTypeStatus != _selPaxTypeStatus)
        return false;
    }
  }

  return (_actualPaxType.empty() || (fare.fcasPaxType() == _actualPaxType) ||
          ((_actualPaxType == ADULT) && fare.fcasPaxType().empty()));
}

bool
MinFareFareSelection::isEligible(const PaxTypeFare& fare)
{
  if (_isThruFareOw) // Tag 3
    return fare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED; // Use tag 1 or 3

  switch (_eligibleFare)
  {
  case ONE_WAY: // Use tag 1 and 3
    return (fare.owrt() != ROUND_TRIP_MAYNOT_BE_HALVED);

  case HALF_ROUND_TRIP: // Use tag 1 and 2
    return (fare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED);

  default:
    return false;
  }
}

const PaxTypeFare*
MinFareFareSelection::selectFareByRuleLevelExcl(const PaxTypeFareSet& fares,
                                                bool selectLowest /* = true */)
{
  if (selectLowest)
  {
    PaxTypeFareSet::const_iterator i = fares.begin();
    for (; i != fares.end(); i++)
    {
      if (passRuleLevelExclusion(**i))
        return *i;
    }
  }
  else
  {
    int32_t numFares = fares.size();
    int32_t numChecked = 0;

    PaxTypeFareSet::const_iterator i = fares.end();
    for (i--; numChecked < numFares; i--, numChecked++)
    {
      if (passRuleLevelExclusion(**i))
        return *i;
    }
  }

  return nullptr;
}

bool
MinFareFareSelection::passRuleLevelExclusion(const PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  const MinFareRuleLevelExcl* ruleLevelExcl = nullptr;

  // Find the RuleLevelExcl record for the fare.

  MatchRuleLevelExclTable matchedRuleLevelExcl(
      _module, _trx, _itin, paxTypeFare, _travelDate, _thruFare ? true : false);

  if (matchedRuleLevelExcl())
    ruleLevelExcl = matchedRuleLevelExcl.matchedRuleItem();

  if (ruleLevelExcl == nullptr) // This fare is not prohibited from selection.
  {
    if (_diag != nullptr)
    {
      (*_diag) << "\nNO MATCHED RULE LEVEL EXCL:\n";
      displayFare(paxTypeFare);
      (*_diag) << " \n";
    }

    return ret;
  }

  return checkRuleLevelExclusionComparison(paxTypeFare, ruleLevelExcl, matchedRuleLevelExcl);
}

// New Code
bool
MinFareFareSelection::checkRuleLevelExclusionComparison(
    const PaxTypeFare& paxTypeFare,
    const MinFareRuleLevelExcl* ruleLevelExcl,
    MatchRuleLevelExclTable& matchedRuleLevelExcl)
{
  bool ret = true;
  bool retSameFareGroupCall = false;
  bool result = false;
  bool returnVal = false;

  if (ruleLevelExcl == nullptr) // This fare is not prohibited from selection
    return false;

  // Check "Exclusion Checks" for "Do not use for fare comp check"
  switch (_module)
  {
  case HIP:
    ret = !(ruleLevelExcl->hipFareCompAppl() == MinimumFare::YES);
    if (ret && ruleLevelExcl->hipSameGroupAppl() == MinimumFare::PERMITTED)
      retSameFareGroupCall =
          matchedRuleLevelExcl.passRuleLevelExclusionSameGroup(*ruleLevelExcl, *_thruFare, true);
    else
      result = true;
    break;
  case BHC:
    ret = !(ruleLevelExcl->backhaulFareCompAppl() == MinimumFare::YES);
    if (ret && ruleLevelExcl->backhaulSameGroupAppl() == MinimumFare::PERMITTED)
      retSameFareGroupCall =
          matchedRuleLevelExcl.passRuleLevelExclusionSameGroup(*ruleLevelExcl, *_thruFare, true);
    else
      result = true;
    break;

  case CTM:
    ret = !(ruleLevelExcl->ctmFareCompAppl() == MinimumFare::YES);
    if (ret && ruleLevelExcl->ctmSameGroupAppl() == MinimumFare::PERMITTED)
    {
      if (!_pricingUnits.empty())
      {
        PricingUnit& pu = *_pricingUnits.front();
        const std::vector<FareUsage*>& fus = pu.fareUsage();
        for (const auto fu : fus)
        {
          if (!fu)
            continue;

          const PaxTypeFare* otherThruFare = fu->paxTypeFare();
          if (!matchedRuleLevelExcl.passRuleLevelExclusionSameGroup(
                  *ruleLevelExcl, *otherThruFare, true))
          {
            retSameFareGroupCall = false;
            break;
          }
          else
            retSameFareGroupCall = true;
        }
      }
      else
        retSameFareGroupCall =
            matchedRuleLevelExcl.passRuleLevelExclusionSameGroup(*ruleLevelExcl, *_thruFare, true);
    }
    else
      result = true;
    break;
  case COM:
    ret = !(ruleLevelExcl->comFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  case DMC:
    ret = !(ruleLevelExcl->dmcFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  case COP:
    ret = !(ruleLevelExcl->copFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  case OSC:
    ret = !(ruleLevelExcl->oscFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  case RSC:
    ret = !(ruleLevelExcl->rscFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  case CPM:
    ret = !(ruleLevelExcl->cpmFareCompAppl() == MinimumFare::YES);
    if (ret)
      result = true;
    break;

  default:
    break;
  }

  if (!ret) // when FARECOMPARE is Y
  {
    if (_diag != nullptr)
      (*_diag) << "\nFAILED RULE LEVEL EXCL "
               << "SEQ# " << ruleLevelExcl->seqNo() << " - CAN NOT BE USED FOR FARE COMP";

    returnVal = ret;
  }
  else
  {
    if (!result) // FARECOMPARE is N and SAMEFAREGROUP is P
    {
      if (retSameFareGroupCall) // PASS SAMEFAREGROUP
      {
        if (_diag != nullptr)
          (*_diag) << "\nPASS RULE LEVEL EXCL SAME FARE GROUP :"
                   << "SEQ# " << ruleLevelExcl->seqNo();
      }
      else // FAIL SAMEFAREGROUP
      {
        if (_diag != nullptr)
          (*_diag) << "\nFAILED RULE LEVEL EXCL "
                   << "SEQ# " << ruleLevelExcl->seqNo() << " - NOT SAME FARE GROUP";
      }
      returnVal = retSameFareGroupCall;
    }
    else // FARECOMPARE is N and SAMEFAREGROUP is N
    {
      if (_diag != nullptr)
        (*_diag) << "\nPASSED RULE LEVEL EXCL "
                 << "SEQ# " << ruleLevelExcl->seqNo();

      returnVal = result;
    }
  }

  if (_diag != nullptr)
  {
    (*_diag) << ":" << std::endl;
    displayFare(paxTypeFare);
    (*_diag) << " \n";
  }
  return returnVal;
}

void
MinFareFareSelection::displayFareHeader()
{
  (*_diag) << " CXR V R R 2 RULE FARE    TR CT   RULE O O  FR R  S  PAX      NUC\n"
           << "       M V 5      CLS     CT TY   TRF  R I  TY U  O           AMT\n";
}

void
MinFareFareSelection::displayFare(const PaxTypeFare& paxTypeFare)
{
  DiagCollector& dc = *_diag;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(2);
  if (paxTypeFare.isDiscounted())
    dc << "D";
  else if (paxTypeFare.isFareByRule())
    dc << "B";
  else if (paxTypeFare.fare()->isIndustry())
    dc << "Y";
  else if (paxTypeFare.fare()->isConstructed())
    dc << "C";
  else
    dc << "P";

  // Cxr
  dc << std::setw(3);
  dc << paxTypeFare.carrier();

  // GI
  // std::string gd;
  // globalDirectionToStr(gd, paxTypeFare.globalDirection());
  // dc << std::setw( 3 );
  // dc << gd;

  // Vendor
  dc << std::setw(2);
  dc << ((paxTypeFare.vendor() == Vendor::ATPCO)
             ? "A"
             : ((paxTypeFare.vendor() == Vendor::SITA) ? "S" : "?"));

  // Mileage vs. Routing
  dc << std::setw(2);
  if (paxTypeFare.isRouting())
    dc << "R";
  else
    dc << "M";

  dc << std::setw(2);
  if (paxTypeFare.isRoutingValid())
    dc << "T";
  else
    dc << "F";
  dc << std::setw(2);
  if (paxTypeFare.isRoutingInvalidByMPM())
    dc << "T";
  else
    dc << "F";

  // Rule Number
  dc << std::setw(5);
  dc << paxTypeFare.ruleNumber();

  // Fare Class
  dc << std::setw(8);
  dc << paxTypeFare.fareClass();

  // Tariff Cat
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::dec);
  dc << std::setw(3);
  dc << paxTypeFare.tcrTariffCat();

  // Display Cat Type
  dc << std::setw(2);
  dc << paxTypeFare.fcaDisplayCatType();

  // Rule Tariff
  dc << std::setw(5);
  dc << paxTypeFare.tcrRuleTariff();

  // Tag 1, 2 and 3
  dc << std::setw(2);
  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    dc << "X";
  else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    dc << "R";
  else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    dc << "O";
  else
    dc << " ";

  // Directionality
  dc << std::setw(2);
  if (paxTypeFare.directionality() == FROM)
    dc << "O";
  else if (paxTypeFare.directionality() == TO)
    dc << "I";
  else
    dc << " "; // For BOTH

  // Fare Type
  dc << std::setw(4);
  dc << paxTypeFare.fcaFareType();

  // Restricted vs. Unrestricted
  const FareTypeMatrix* fareTypeMatrix =
      _trx.dataHandle().getFareTypeMatrix(paxTypeFare.fcaFareType(), _travelDate);

  if (fareTypeMatrix != nullptr)
    dc << " " << fareTypeMatrix->restrInd() << " ";

  // Max Stopover
  dc.setf(std::ios::fixed, std::ios::dec);
  dc << std::setw(2);
  if (paxTypeFare.maxStopoversPermitted() < PaxTypeFare::MAX_STOPOVERS_UNLIMITED)
  {
    dc << paxTypeFare.maxStopoversPermitted();
  }
  else
    dc << "  ";

  // PaxType
  dc << std::setw(4);
  dc << paxTypeFare.fcasPaxType();

  // NUC amount
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);
  dc << std::setw(8);
  dc << paxTypeFare.nucFareAmount();

  dc.setf(std::ios::left, std::ios::adjustfield);
}

// Check if the module is default to go thru all fare selection steps.
bool
MinFareFareSelection::isDefaultThruAll()
{
  switch (_module)
  {
  case HIP:
  case BHC:
  case COM:
  case DMC:
  case OJM:
    return true;

  case CTM:
  case CPM:
  case COP:
  case RSC:
  case OSC: // OSC does not move cabin.
  default:
    return false;
  }
}

bool
MinFareFareSelection::validateCat25Cat35(const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isFareByRule())
    return false;

  // Cat 35 fares will not be considered during the fare selection process
  if ((paxTypeFare.fcaDisplayCatType() == RuleConst::SELLING_FARE) || // 'L'
      (paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) || // 'T'
      (paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)) // 'C'
    return false;

  return true;
}

void
MinFareFareSelection::getFareSelPref()
{
  switch (_module)
  {
  case HIP:
  case BHC:
  case CTM:
    break;

  case COM:
  case DMC:
  case CPM:
  case OJM:
  case COP:
  case OSC:
  case RSC:
  default:
    return;
  }

  if (_minFareAppl == nullptr)
  {
    if (_module == CTM &&
        (!_pricingUnits.empty() && (*_pricingUnits.front()).puFareType() == PricingUnit::SP))
    {
      return;
    }

    // Only check table for HIP, BHC and CTM S
    throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA,
                                 MinimumFare::MISSING_DATA_ERROR_MSG.c_str());
  }

  if (_minFareAppl->applyDefaultLogic() == MinimumFare::YES)
  {
    // Apply default logic
    getDefaultLogic();
  }
  else
  {
    // Apply overwrite logic
    getOverrideLogic();
  }
}

bool
MinFareFareSelection::isRepriceNeeded()
{
  if (_itin.isThruFareOnly())
    return true;

  if (_module == HIP || _module == BHC)
  {
    // Reprice Fare Markets that were held back because of LimitationOnIndirectTravel
    return true;
  }

  if (!_isNetRemit)
    return false;

  bool hasAltFm = false;

  const FarePath& origFp =
      *((dynamic_cast<const NetRemitFarePath*>(_farePath))->originalFarePath());

  std::vector<PricingUnit*>::const_iterator puIter = origFp.pricingUnit().begin();
  for (; puIter != origFp.pricingUnit().end(); ++puIter)
  {
    PricingUnit& pu = **puIter;

    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
    for (; fuIter != pu.fareUsage().end(); ++fuIter)
    {
      FareUsage& fu = **fuIter;

      if ((fu.tktNetRemitFare() != nullptr) &&
          (fu.tktNetRemitFare2() == nullptr) && // Do not consider Recurring segments
          (fu.tktNetRemitFare()->fareMarket() != fu.paxTypeFare()->fareMarket()))
        hasAltFm = true;
    }
  }

  return hasAltFm;
}

PricingTrx*
MinFareFareSelection::getRepriceTrx(const std::vector<TravelSeg*>& tvlSegs)
{
  PricingTrx* retTrx = nullptr;

  try
  {
    retTrx = TrxUtil::reprice(_trx, const_cast<std::vector<TravelSeg*>&>(tvlSegs));
  }
  catch (const ErrorResponseException& ex)
  {
  }
  catch (...)
  {
  }

  return retTrx;
}

void
MinFareFareSelection::getAlternateFareMarket(const std::vector<TravelSeg*>& travelSegs,
                                             std::vector<FareMarket*>& retFareMarket)
{
  if (travelSegs.size() < 1)
    return;

  TravelSeg* startTravelSeg = travelSegs.front();
  TravelSeg* endTravelSeg = travelSegs.back();

  // Match first and last travel segments
  // Match first travel segment and destination point
  // Match origin point and last travel segment
  std::vector<FareMarket*>::const_iterator fareMarketI;

  for (fareMarketI = _trx.fareMarket().begin(); fareMarketI != _trx.fareMarket().end();
       fareMarketI++)
  {
    const std::vector<TravelSeg*>& fmTravelSegs = (*fareMarketI)->travelSeg();

    if (fmTravelSegs.front() == startTravelSeg)
    {
      if ((fmTravelSegs.back() == endTravelSeg) ||
          (fmTravelSegs.back()->offMultiCity() == endTravelSeg->offMultiCity()))
        retFareMarket.push_back(*fareMarketI);
    }
    else if (fmTravelSegs.back() == endTravelSeg)
    {
      if (fmTravelSegs.front()->boardMultiCity() == startTravelSeg->boardMultiCity())
        retFareMarket.push_back(*fareMarketI);
    }
  }
}
}
