#include "Rules/RuleItem.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "DBAccess/Tours.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag302Collector.h"
#include "Diagnostic/Diag304Collector.h"
#include "Diagnostic/Diag311Collector.h"
#include "Diagnostic/Diag316Collector.h"
#include "Diagnostic/Diag327Collector.h"
#include "Rules/AccompaniedTravel.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/AdvResOverride.h"
#include "Rules/BlackoutDates.h"
#include "Rules/DayTimeApplication.h"
#include "Rules/Eligibility.h"
#include "Rules/FareDisplayBlackoutDates.h"
#include "Rules/FDAdvanceResTkt.h"
#include "Rules/FDDayTimeApplication.h"
#include "Rules/FDEligibility.h"
#include "Rules/FDMaxStayApplication.h"
#include "Rules/FDMinStayApplication.h"
#include "Rules/FDPenalties.h"
#include "Rules/FDSalesRestrictionRule.h"
#include "Rules/FDSeasonalApplication.h"
#include "Rules/FDSurchargesRule.h"
#include "Rules/FDTravelRestrictions.h"
#include "Rules/FlightApplication.h"
#include "Rules/MaximumStayApplication.h"
#include "Rules/MinimumStayApplication.h"
#include "Rules/MiscFareTagsRule.h"
#include "Rules/Penalties.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/SeasonalApplication.h"
#include "Rules/Stopovers.h"
#include "Rules/SurchargesRule.h"
#include "Rules/TicketingEndorsement.h"
#include "Rules/ToursApplication.h"
#include "Rules/Transfers1.h"
#include "Rules/TravelRestrictions.h"
#include "Rules/UpdaterObserver.h"
#include "Rules/VoluntaryChanges.h"
#include "Rules/VoluntaryRefunds.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(apo45023ApplyCat2DefaultsInOOJPU)
static Logger
logger("atseintl.Rules.RuleItem");

static const std::string UnknownPhaseStr = "UNKNOWN";
static const std::string FarePhaseStr = "FARE VALIDATOR";
static const std::string FCOPhaseStr = "FCO FARE VALIDATOR";
static const std::string PricingUnitPhaseStr = "PRICING UNIT";
static const std::string PricingUnitFactoryPhaseStr = "PRICING UNIT FACTORY";
static const std::string FarePathFactoryPhaseStr = "FARE PATH";
static const std::string PreCombinabilityPhaseStr = "PRECOMBINABILITY";
static const std::string FareDisplayPhaseStr = "FD VAL";
static const std::string RuleDisplayPhaseStr = "RD VAL";
static const std::string DynamicValidationPhaseStr = "DYNAMIC VAL";

RuleItem::RuleItem()
  : _trx(nullptr),
    _itin(nullptr),
    _farePath(nullptr),
    _pricingUnit(nullptr),
    _fareUsage(nullptr),
    _paxTypeFare(nullptr),
    _cri(nullptr),
    _cfrItemSet(nullptr),
    _rule(nullptr),
    _ruleItemInfo(nullptr),
    _isCat15Security(nullptr),
    _phase(UnKnown),
    _skipCat15Security(false),
    _isInbound(false),
    _rcDataAccess(nullptr),
    _chancelor()
{
  _handlerMap.fill(nullptr);
  setHandlers();
}

RuleItem::RuleItem(PricingTrx& trx,
                   const CategoryRuleInfo& cri,
                   const PricingUnit* pricingUnit,
                   const PaxTypeFare& paxTypeFare,
                   const RuleItemInfo* ruleItemInfo)
  : _trx(&trx),
    _itin(nullptr),
    _farePath(nullptr),
    _pricingUnit(pricingUnit),
    _fareUsage(nullptr),
    _paxTypeFare(const_cast<PaxTypeFare*>(&paxTypeFare)),
    _cri(&cri),
    _cfrItemSet(nullptr),
    _rule(nullptr),
    _ruleItemInfo(ruleItemInfo),
    _isCat15Security(nullptr),
    _phase(UnKnown),
    _skipCat15Security(false),
    _isInbound(false),
    _rcDataAccess(nullptr),
    _chancelor()
{
  _handlerMap.fill(nullptr);
}

void
RuleItem::setHandlers()
{
  setHandler(RuleConst::ELIGIBILITY_RULE, &RuleItem::handleEligibility);
  setHandler(RuleConst::SEASONAL_RULE, &RuleItem::handleSeasonal);
  setHandler(RuleConst::BLACKOUTS_RULE, &RuleItem::handleBlackoutDates);
  setHandler(RuleConst::PENALTIES_RULE, &RuleItem::handlePenalties);
  setHandler(RuleConst::DAY_TIME_RULE, &RuleItem::handleDayTime);
  setHandler(RuleConst::SALE_RESTRICTIONS_RULE, &RuleItem::handleSalesRestrictions);
  setHandler(RuleConst::FLIGHT_APPLICATION_RULE, &RuleItem::handleFlightApplication);
  setHandler(RuleConst::MINIMUM_STAY_RULE, &RuleItem::handleMinStayRestriction);
  setHandler(RuleConst::MAXIMUM_STAY_RULE, &RuleItem::handleMaxStayRestriction);
  setHandler(RuleConst::STOPOVER_RULE, &RuleItem::handleStopovers);
  setHandler(RuleConst::TRANSFER_RULE, &RuleItem::handleTransfers);
  setHandler(RuleConst::ADVANCE_RESERVATION_RULE, &RuleItem::handleAdvResTkt);
  setHandler(RuleConst::TRAVEL_RESTRICTIONS_RULE, &RuleItem::handleTravelRestrictions);
  setHandler(RuleConst::SURCHARGE_RULE, &RuleItem::handleSurchargeRule);
  setHandler(RuleConst::ACCOMPANIED_PSG_RULE, &RuleItem::handleAccompaniedTravel);
  setHandler(RuleConst::TICKET_ENDORSMENT_RULE, &RuleItem::handleTicketingEndorsement);
  setHandler(RuleConst::MISC_FARE_TAG, &RuleItem::handleMiscFareTags);
  setHandler(RuleConst::VOLUNTARY_EXCHANGE_RULE, &RuleItem::handleVoluntaryExc);
  setHandler(RuleConst::TOURS_RULE, &RuleItem::handleTours);
  setHandler(RuleConst::VOLUNTARY_REFUNDS_RULE, &RuleItem::handleVoluntaryRefund);
}

Record3ReturnTypes
RuleItem::validate(PricingTrx& trx,
                   Itin& itin,
                   const CategoryRuleInfo& cri,
                   const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                   PaxTypeFare& paxTypeFare,
                   const CategoryRuleItemInfo* rule,
                   const RuleItemInfo* const ruleItemInfo,
                   uint32_t categoryNumber,
                   bool& isCat15Security,
                   bool skipCat15Security)
{
  _trx = &trx;
  _itin = &itin;
  _cri = &cri;
  _cfrItemSet = &cfrItemSet;
  _paxTypeFare = &paxTypeFare;
  _rule = rule;
  _ruleItemInfo = ruleItemInfo;
  _isCat15Security = &isCat15Security;
  _farePath = nullptr;
  _pricingUnit = nullptr;
  _fareUsage = nullptr;
  _skipCat15Security = skipCat15Security;
  _isInbound = false;

  ProcessingPhase phase = _phase; // save the original Phase

  setPhase(trx, categoryNumber);

  // Need to get appropriate Global Direction when FareDisplayTrx
  if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
  {
    // Save FareMarket current Global Direction
    GlobalDirection fmGlobal = _paxTypeFare->fareMarket()->getGlobalDirection();

    // Store appropriate Global Direction into the FareMarket
    _paxTypeFare->fareMarket()->setGlobalDirection(_paxTypeFare->globalDirection());

    Record3ReturnTypes retval = callHandler(categoryNumber);

    // Restore FareMarket previous Global Direction
    _paxTypeFare->fareMarket()->setGlobalDirection(fmGlobal);

    return retval;
  }
  Record3ReturnTypes retval = callHandler(categoryNumber);
  _phase = phase; // restore the original Phase
  return retval;
}

Record3ReturnTypes
RuleItem::validate(PricingTrx& trx,
                   Itin& itin,
                   const CategoryRuleInfo& cri,
                   const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                   PaxTypeFare& paxTypeFare,
                   const CategoryRuleItemInfo* rule,
                   const RuleItemInfo* const ruleItemInfo,
                   uint32_t categoryNumber,
                   bool& isCat15Security,
                   bool skipCat15Security,
                   bool isInbound)
{
  _trx = &trx;
  _itin = &itin;
  _cri = &cri;
  _cfrItemSet = &cfrItemSet;
  _paxTypeFare = &paxTypeFare;
  _rule = rule;
  _ruleItemInfo = ruleItemInfo;
  _isCat15Security = &isCat15Security;
  _farePath = nullptr;
  _pricingUnit = nullptr;
  _fareUsage = nullptr;
  _skipCat15Security = skipCat15Security;
  _isInbound = isInbound;

  ProcessingPhase phase = _phase; // save the original Phase

  setPhase(trx, categoryNumber);

  // Need to get appropriate Global Direction when FareDisplayTrx
  if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
  {
    // Save FareMarket current Global Direction
    GlobalDirection fmGlobal = _paxTypeFare->fareMarket()->getGlobalDirection();

    // Store appropriate Global Direction into the FareMarket
    _paxTypeFare->fareMarket()->setGlobalDirection(_paxTypeFare->globalDirection());

    Record3ReturnTypes retval = callHandler(categoryNumber);

    // Restore FareMarket previous Global Direction
    _paxTypeFare->fareMarket()->setGlobalDirection(fmGlobal);

    return retval;
  }

  Record3ReturnTypes retval = callHandler(categoryNumber);
  _phase = phase; // restore the original Phase
  return retval;
}

void
RuleItem::setPhase(PricingTrx& trx, uint32_t categoryNumber)
{
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (LIKELY(!fdUtil.getFareDisplayTrx(_trx, fdTrx)))
  {
    if ((_phase != FCOPhase) &&
        !((_phase == DynamicValidationPhase) && ((categoryNumber == RuleConst::STOPOVER_RULE) ||
                                                 (categoryNumber == RuleConst::TRANSFER_RULE))))
    {
      _phase = FarePhase;
    }
  }
  else
  {
    if (fdTrx->isRD() ||
        (fdTrx->getOptions() != nullptr && fdTrx->getOptions()->FBDisplay() == 'T'))
    {
      _phase = RuleDisplayPhase;
    }
    else
    {
      _phase = FareDisplayPhase;
    }
  }
}

Record3ReturnTypes
RuleItem::validate(PricingTrx& trx,
                   const CategoryRuleInfo& cri,
                   const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                   FarePath& farePath,
                   const PricingUnit& pricingUnit,
                   FareUsage& fareUsage,
                   const CategoryRuleItemInfo* rule,
                   const RuleItemInfo* const ruleItemInfo,
                   uint32_t categoryNumber,
                   bool& isCat15Security,
                   bool skipCat15Security)
{
  _trx = &trx;
  _itin = farePath.itin();
  _cri = &cri;
  _cfrItemSet = &cfrItemSet;
  _paxTypeFare = fareUsage.paxTypeFare();
  _rule = rule;
  _ruleItemInfo = ruleItemInfo;
  _isCat15Security = &isCat15Security;
  _farePath = &farePath;
  _pricingUnit = &pricingUnit;
  _fareUsage = &fareUsage;
  //_phase = PricingUnitPhase;
  _phase = FarePathFactoryPhase;
  _skipCat15Security = skipCat15Security;
  _isInbound = false;

  return callHandler(categoryNumber);
}

Record3ReturnTypes
RuleItem::validate(PricingTrx& trx,
                   const CategoryRuleInfo& cri,
                   const std::vector<CategoryRuleItemInfo>& cfrItemSet,
                   const Itin* itin,
                   const PricingUnit& pricingUnit,
                   FareUsage& fareUsage,
                   const CategoryRuleItemInfo* rule,
                   const RuleItemInfo* const ruleItemInfo,
                   uint32_t categoryNumber,
                   bool& isCat15Security,
                   bool skipCat15Security)
{
  _trx = &trx;
  _itin = const_cast<Itin*>(itin);
  _cri = &cri;
  _cfrItemSet = &cfrItemSet;
  _paxTypeFare = fareUsage.paxTypeFare();
  _rule = rule;
  _ruleItemInfo = ruleItemInfo;
  _isCat15Security = &isCat15Security;
  _farePath = nullptr;
  _pricingUnit = &pricingUnit;
  _fareUsage = &fareUsage;
  //_phase = PreCombinabilityPhase;
  _phase = PricingUnitFactoryPhase;
  _skipCat15Security = skipCat15Security;
  _isInbound = false;

  return callHandler(categoryNumber);
}

Record3ReturnTypes
RuleItem::callHandler(uint32_t categoryNumber)
{
  if (UNLIKELY(ruleIgnored(categoryNumber)))
    return PASS; // for Cat31 transaction

  Record3ReturnTypes retCode = SKIP;
  //    setHandlers();   don't do this it will cause pre-validate not to work because we change the
  // map!

  if (UNLIKELY(_ruleItemInfo == nullptr))
  {
    if (_paxTypeFare->isCmdPricing() && _paxTypeFare->validForCmdPricing(_trx->fxCnException()))
      return retCode = PASS;

    // Check for FareDisplay transaction
    if (_trx->getTrxType() == PricingTrx::FAREDISPLAY_TRX)
    {
      // Get Fare Display Info object
      FareDisplayInfo* fdInfo = _paxTypeFare->fareDisplayInfo();

      if (fdInfo)
      {
        // Update FareDisplayInfo object: Missing/Incomplete rule data
        fdInfo->setIncompleteR3Rule(categoryNumber);

        return NOTPROCESSED;
      }
    }

    return retCode;
  }

  CategoryHandler method = nullptr;

  if (LIKELY(categoryNumber <= MAX_RULE_HANDLED))
    method = _handlerMap[categoryNumber];

  if (!method)
  {
    const bool isQualifiedCategory = (_cri->categoryNumber() != categoryNumber);

    if (UNLIKELY(!isQualifiedCategory))
      return PASS;

    if (categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE)
      method = &RuleItem::handleFlightApplication;
    else if (categoryNumber == RuleConst::DAY_TIME_RULE)
      method = &RuleItem::handleDayTime;
    else
      return SOFTPASS;
  }

  return (this->*method)();
}

Record3ReturnTypes
RuleItem::handleEligibility()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  1" );

  Eligibility eligibility;
  eligibility.setChancelor(_chancelor);

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (_trx->diagnostic().diagnosticType() == Diagnostic301)
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic301);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 01 - ELIGIBILITY APPLICATION DIAGNOSTICS" << std::endl;

    if (!_paxTypeFare->isFareByRule() && _paxTypeFare->cat25Fare())
    {
      PaxTypeFare* correctPtf = _paxTypeFare->cat25Fare();
      (*diagPtr) << "BASE FARE OF CAT25 FARE - " << correctPtf->fareClass() << std::endl;
    }

    switch (_phase)
    {
    case FarePhase:
      (*diagPtr) << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case PricingUnitFactoryPhase:
      (*diagPtr) << "PHASE: PRICING UNIT FACTORY   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case FarePathFactoryPhase:
      (*diagPtr) << "PHASE: FARE PATH FACTORY   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case FareDisplayPhase:
      (*diagPtr) << "PHASE: FARE DISPLAY VALIDATOR       R3 ITEM NUMBER: "
                 << _ruleItemInfo->itemNo() << std::endl;
      break;
    default:
      // Nothing here
      break;
    }

    (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
               << _paxTypeFare->fareMarket()->destination()->loc()
               << " FC: " << _paxTypeFare->fareClass();

    std::string fareBasis = _paxTypeFare->createFareBasis(*_trx, false);
    (*diagPtr) << " FB: " << fareBasis << std::endl;

    (*diagPtr) << "R2:FARERULE   :   " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
               << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic301))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleEligibility: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::ELIGIBILITY_RULE, diagPtr, Diagnostic301);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  bool isQualifyingCat = false;

  if (_cri->categoryNumber() != _rule->itemcat())
  {
    isQualifyingCat = true;
  }
  else if (!_cfrItemSet->empty() &&
           (*_cfrItemSet)[0].relationalInd() == CategoryRuleItemInfo::ELSE &&
           !_paxTypeFare->isWebFare())
  {
    isQualifyingCat = true;
  }

  bool needBaseFare = false;

  if (isQualifyingCat && _cri->categoryNumber() == RuleConst::NEGOTIATED_RULE &&
      _paxTypeFare->isDiscounted())
  {
    needBaseFare = true;
  }

  bool isCat15Qualifying = false;

  if (isQualifyingCat && _cri->categoryNumber() == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    isCat15Qualifying = true;
  }

  PaxTypeFare* newPaxTypeFare = determinePaxTypeFare(_paxTypeFare, needBaseFare);

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    FDEligibility fdEligibility;

    retval = fdEligibility.validate(*_trx,
                                    *_itin,
                                    *_paxTypeFare,
                                    _ruleItemInfo,
                                    *_paxTypeFare->fareMarket(),
                                    isQualifyingCat,
                                    isCat15Qualifying);
  }
  break;
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  1 FC SCOPE" );

    retval = eligibility.validate(*_trx,
                                  *_itin,
                                  *newPaxTypeFare,
                                  _ruleItemInfo,
                                  *_paxTypeFare->fareMarket(),
                                  isQualifyingCat,
                                  isCat15Qualifying);
  }
  break;
  case PricingUnitFactoryPhase:
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  1 PU SCOPE" );

    retval = eligibility.validate(*_trx,
                                  _ruleItemInfo,
                                  *_pricingUnit,
                                  *_fareUsage,
                                  isQualifyingCat,
                                  _cri->categoryNumber() == RuleConst::TOURS_RULE);
  }
  break;
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  // For Discounted fare with THEN 35 IF 1, IF 1 was validated with base fare,
  // so copy matchedCorpID indicator from base fare over to Discounted fare
  if (needBaseFare)
  {
    if (UNLIKELY(newPaxTypeFare->matchedCorpID() && !_paxTypeFare->matchedCorpID()))
    {
      _paxTypeFare->setMatchedCorpID();
    }

    if (UNLIKELY((PricingTrx::MIP_TRX == _trx->getTrxType()) && (_trx->isFlexFare())))
    {
      _paxTypeFare->getMutableFlexFaresValidationStatus()->updateAttribute<flexFares::CORP_IDS>(
          newPaxTypeFare->getFlexFaresValidationStatus());

      _paxTypeFare->getMutableFlexFaresValidationStatus()->updateAttribute<flexFares::ACC_CODES>(
          newPaxTypeFare->getFlexFaresValidationStatus());
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleSeasonal()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  3" );

  if (UNLIKELY(ruleIgnored(RuleConst::SEASONAL_RULE)))
    return PASS; // for Cat31 transaction

  SeasonalApplication seasonalApplication;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic303))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic303);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 03 - SEASONS APPLICATION DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FCOPhase:
    case FarePhase:
      (*diagPtr) << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case PricingUnitFactoryPhase:
    case FarePathFactoryPhase:
      (*diagPtr) << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    case FareDisplayPhase:
      (*diagPtr) << "PHASE: FARE DISPLAY VALIDATOR   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      break;
    default:
      // Nothing here
      break;
    }

    (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
               << _paxTypeFare->fareMarket()->destination()->loc() << " "
               << _paxTypeFare->fareClass() << "     ";

    (*diagPtr) << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
               << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic303))
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::SEASONAL_RULE, diagPtr, Diagnostic303);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  seasonalApplication.setRuleDataAccess(_rcDataAccess);

  Record3ReturnTypes retval = PASS;
  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    FDSeasonalApplication fdSeasons(_rule);
    retval = PASS;

    const bool isQualifiedCategory = (_cri->categoryNumber() != RuleConst::SEASONAL_RULE);

    if (!_isInbound)
    {
      retval = fdSeasons.validate(*_trx,
                                  *_itin,
                                  *_paxTypeFare,
                                  _ruleItemInfo,
                                  *_paxTypeFare->fareMarket(),
                                  isQualifiedCategory,
                                  _isInbound);
    }
    else
    {
      FareDisplayTrx* fareDisplayTrx;
      if (FareDisplayUtil::getFareDisplayTrx(_trx, fareDisplayTrx))
      {
        if (fareDisplayTrx->getRequest()->returnDate().isValid() &&
            (_paxTypeFare->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED))
        {
          retval = fdSeasons.validate(*_trx,
                                      *_itin,
                                      *_paxTypeFare,
                                      _ruleItemInfo,
                                      *_paxTypeFare->fareMarket(),
                                      isQualifiedCategory,
                                      _isInbound);
        }
      }
    }
  }
  break;

  case FCOPhase:
  case FarePhase:
    retval = seasonalApplication.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
    break;

  case PricingUnitFactoryPhase:
    retval = seasonalApplication.validate(*_trx, _ruleItemInfo, _itin, *_pricingUnit, *_fareUsage);
    break;

  case FarePathFactoryPhase:
    retval =
        seasonalApplication.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
    break;

  case PreCombinabilityPhase:
    retval = seasonalApplication.validate(*_trx, _ruleItemInfo, *_pricingUnit, *_fareUsage);
    break;

  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleAdvResTkt()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  5" );

  AdvanceResTktWrapper advanceResTkt;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic305))
  {
    bool shouldPrint = !_trx->diagnostic().filterRulePhase(_phase);
    if (shouldPrint && RexPricingTrx::isRexTrxAndNewItin(*_trx) && _phase == FarePathFactoryPhase &&
        _pricingUnit && _pricingUnit->volChangesAdvResOverride() &&
        _pricingUnit->volChangesAdvResOverride()->diag305OFF())
      shouldPrint = false;

    if (shouldPrint)
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(*_trx);

      diagPtr->enable(Diagnostic305);
      diagEnabled = true;
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 05 - ADVANCE RESERVATION/TICKETING DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FarePhase:
      (*diagPtr) << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
                 << _paxTypeFare->fareMarket()->destination()->loc() << " "
                 << _paxTypeFare->fareClass() << "     ";
      break;

    case PricingUnitFactoryPhase:
    case FarePathFactoryPhase:
      (*diagPtr) << "PHASE: PRICING UNIT       R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      (*diagPtr) << _fareUsage->paxTypeFare()->fareClass() << " ";
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      (*diagPtr) << _fareUsage->paxTypeFare()->fareClass() << " ";
      break;
    case FareDisplayPhase:
      (*diagPtr) << "PHASE: FARE DISPLAY VALIDATOR   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl;
      (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
                 << _paxTypeFare->fareMarket()->destination()->loc() << " "
                 << _paxTypeFare->fareClass() << "     ";
      break;

    default:
      // Nothing here
      break;
    }

    if (_cri != nullptr)
    {
      (*diagPtr) << "R2:FARERULE  :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
                 << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;
    }

    diagPtr->flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic305))
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::ADVANCE_RESERVATION_RULE, diagPtr, Diagnostic305);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  advanceResTkt.initialize(*_trx,
                           *(static_cast<const AdvResTktInfo*>(_ruleItemInfo)),
                           *_paxTypeFare,
                           _pricingUnit,
                           _itin);

  advanceResTkt.setRuleDataAccess(_rcDataAccess);
  advanceResTkt.setChancelor(_chancelor);

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    const bool isQualifiedCategory =
        (_cri->categoryNumber() != RuleConst::ADVANCE_RESERVATION_RULE);

    FDAdvanceResTkt fdAdvResTkt;

    retval = fdAdvResTkt.validate(*_trx,
                                  *_itin,
                                  *_paxTypeFare,
                                  _ruleItemInfo,
                                  *_paxTypeFare->fareMarket(),
                                  isQualifiedCategory);
  }
  break;

  case FCOPhase:
  case FarePhase:
  {
    retval = advanceResTkt.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;

  case PricingUnitFactoryPhase:
  {
    FarePath fakeFP;
    fakeFP.itin() = _itin;
    fakeFP.pricingUnit().push_back(const_cast<PricingUnit*>(_pricingUnit));

    advanceResTkt.useFakeFP() = true;

    retval = advanceResTkt.validate(*_trx, _ruleItemInfo, fakeFP, *_pricingUnit, *_fareUsage);
  }
  break;
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  5 PU SCOPE" );
    retval = advanceResTkt.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;

  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    printFlexFaresRestriction(*diagPtr, ADVANCE_RESERVATION_RULE, retval);
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleFlightApplication()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  4" );

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic304))
  {
    bool activate =
        _fareUsage ? diagPtr->parseFareMarket(*_trx, *(_fareUsage->paxTypeFare()->fareMarket()))
                   : diagPtr->parseFareMarket(*_trx, *(_paxTypeFare->fareMarket()));
    if (activate)
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(*_trx);
      diagPtr->enable(Diagnostic304);
      diagEnabled = true;
    }
  }

  const bool isQualifiedCategory = (_cri->categoryNumber() != RuleConst::FLIGHT_APPLICATION_RULE);

  if (UNLIKELY(diagEnabled))
  {
    if (isQualifiedCategory)
    {
      (*diagPtr) << "QUALIFIER FOR CAT " << _cri->categoryNumber() << std::endl;
    }
    (*diagPtr) << "CATEGORY 04 - FLIGHT APPLICATION DIAGNOSTICS" << std::endl;

    (*diagPtr) << "PHASE: " << getRulePhaseString(_phase)
               << "    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;

    switch (_phase)
    {
    case FarePhase:
    case FCOPhase:
      (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
                 << _paxTypeFare->fareMarket()->destination()->loc() << " "
                 << _paxTypeFare->fareClass();
      if (_paxTypeFare->fareMarket()->direction() == FMDirection::INBOUND)
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;

    case PricingUnitFactoryPhase:
      (*diagPtr) << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass();
      if (_fareUsage->isInbound())
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;
    case FarePathFactoryPhase:
      (*diagPtr) << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass();
      if (_fareUsage->isInbound())
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass();
      if (_fareUsage->isInbound())
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;
    default:
      // Nothing here
      break;
    }

    (*diagPtr) << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
               << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  // the following code will be moving up
  // when all functions
  // in DataHandler will be done.
  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (LIKELY(_phase != FareDisplayPhase &&
                _phase != RuleDisplayPhase)) // FD and RD are not processed
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::FLIGHT_APPLICATION_RULE, diagPtr, Diagnostic304);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  const FlightAppRule* fltRule = dynamic_cast<const FlightAppRule*>(_ruleItemInfo);
  if (UNLIKELY(!fltRule))
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "FAIL - NOT FLIGHT APPLICATION RULE DATA\n";
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  FlightApplication fa;

  fa.initialize(fltRule, isQualifiedCategory, _cri->vendorCode(), _trx, _trx->ticketingDate());
  fa.setRuleDataAccess(_rcDataAccess);

  Record3ReturnTypes retval;
  switch (_phase)
  {
  /*
    case RuleDisplayPhase:
    {
    retval = PASS;
    }
    break;
  */
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  4 FC SCOPE" );
    retval = fa.process(*_paxTypeFare, *_trx);
  }
  break;
  case PricingUnitFactoryPhase:
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  4 PU SCOPE" );
    retval = fa.process(*_trx, *_pricingUnit, *_fareUsage);
  }
  break;
  case FareDisplayPhase:
  case RuleDisplayPhase:
  {
    // Fare Display does not support this rule category
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY 04 - FLIGHT APPLICATION: NOT PROCESSED BY FARE DISPLAY" << std::endl;
      diagPtr->flushMsg();
    }
    retval = NOTPROCESSED;
  }
  break;
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleBlackoutDates()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 11" );

  LOG4CXX_INFO(logger, " Entered RuleItem::handleBlackouts(&, ...)");
  DCFactory* factory = nullptr;
  Diag311Collector* diag = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic311))
  {
    factory = DCFactory::instance();
    diag = dynamic_cast<Diag311Collector*>(factory->create(*_trx));
    diag->enable(Diagnostic311);
    diagEnabled = true;
  }
  // the following code will be moving up
  // when all functions
  // in DataHandler will be done.
  if (_ruleItemInfo->overrideDateTblItemNo())
  {
    if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diag,
                                                  Diagnostic311))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleBlackouts: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::BLACKOUTS_RULE, diag, Diagnostic311);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return rtn;
      }
    }
  }

  BlackoutDates bl;
  if (LIKELY(_phase != FareDisplayPhase && _phase != RuleDisplayPhase))
  {
    bl.initialize(*_trx, dynamic_cast<const BlackoutInfo*>(_ruleItemInfo));
    bl.setRuleDataAccess(_rcDataAccess);
  }

  Record3ReturnTypes retval = PASS;
  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    FareDisplayBlackoutDates fdbl;
    fdbl.initialize(*_trx, dynamic_cast<const BlackoutInfo*>(_ruleItemInfo));

    if (!_isInbound)
    {
      retval = fdbl.process(*_paxTypeFare, *_trx, _isInbound);
    }
    else
    {
      FareDisplayTrx* fareDisplayTrx;
      if (FareDisplayUtil::getFareDisplayTrx(_trx, fareDisplayTrx))
      {
        if (fareDisplayTrx->getRequest()->returnDate().isValid() &&
            (_paxTypeFare->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED))
        {
          retval = fdbl.process(*_paxTypeFare, *_trx, _isInbound);
        }
      }
    }

    if (diag)
    {
      diag->diag311Collector(*_paxTypeFare, *_cri, fdbl, retval, _phase, *_trx, _isInbound);
      diag->flushMsg();
    }
  }
    return retval;

  case FCOPhase:
  case FarePhase:
    LOG4CXX_INFO(logger, " Entered RuleItem::handleBlackouts(FarePath, ...)");
    retval = bl.process(*_paxTypeFare, *_trx);
    break;

  case PricingUnitFactoryPhase:
  case FarePathFactoryPhase:
    LOG4CXX_INFO(logger, " Entered RuleItem::handleBlackouts(PricingUnit, ...)");
    retval = bl.process(*_trx, *_pricingUnit, *_fareUsage, *_itin);
    break;

  case PreCombinabilityPhase:
    if (UNLIKELY(diagEnabled))
      (*diag) << "CATEGORY 11 - BLACKOUT DATES\n"
              << "PRECOMBINABILITY VALIDATION PHASE NOT SUPPORTED\n";
    break;

  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  if (UNLIKELY(diag))
  {
    if (_phase == PricingUnitFactoryPhase || _phase == FarePathFactoryPhase)
      diag->diag311Collector(
          *_paxTypeFare, *_cri, bl, retval, _phase, *_trx, _isInbound, _pricingUnit);
    else
      diag->diag311Collector(*_paxTypeFare, *_cri, bl, retval, _phase, *_trx, _isInbound);

    diag->flushMsg();
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handlePenalties()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 16" );

  LOG4CXX_INFO(logger, " Entered RuleItem::handlePenalties(&, ...)");

  DCFactory* factory = nullptr;
  Diag316Collector* diag = nullptr;
  Diagnostic& trxDiag = _trx->diagnostic();
  bool diagEnabled = false;

  if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic316))
  {
    factory = DCFactory::instance();

    diag = dynamic_cast<Diag316Collector*>(factory->create(*_trx));
    diag->enable(Diagnostic316);
    diagEnabled = true;

    const PenaltyInfo* penaltyRule = dynamic_cast<const PenaltyInfo*>(_ruleItemInfo);

    if (!penaltyRule)
      return SKIP;

    if (_phase == FarePhase || _phase == FareDisplayPhase)
      diag->writeHeader(*_paxTypeFare, *_cri, *penaltyRule, *_trx, true);
    else
      diag->writeHeader(*_paxTypeFare, *_cri, *penaltyRule, *_trx, false);
  }

  Record3ReturnTypes retval = PASS;
  Penalties penalties;

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diag,
                                                  Diagnostic316))
      {
        LOG4CXX_INFO(logger, " RuleItem::handlePenalties: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::PENALTIES_RULE, diag, Diagnostic316);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return rtn;
      }
    }
  }

  penalties.setRuleDataAccess(_rcDataAccess);
  penalties.setChancelor(_chancelor);

  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    //--------------------------------------------------------------
    // Get a Fare Display Transaction from the Pricing Transaction
    //--------------------------------------------------------------
    FareDisplayUtil fdUtil;
    FareDisplayTrx* fdTrx;

    if (!fdUtil.getFareDisplayTrx(_trx, fdTrx))
    {
      retval = FAIL;
    }
    else
    {
      FareDisplayOptions* fdOptions = fdTrx->getOptions();

      //----------------------------------------------------------------
      // Check if it is a request to exclude fares with Penalty or to
      // exclude fares with any restrictions
      //----------------------------------------------------------------
      if (fdOptions &&
          (fdOptions->isExcludePenaltyFares() || fdOptions->isExcludeRestrictedFares()))
      {
        FDPenalties fdPenalties;

        retval = fdPenalties.validate(
            *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
      }
      else
      {
        // Fare Display does not support this rule category
        if (UNLIKELY(diagEnabled))
        {
          (*diag) << "CATEGORY 16 - PENALTIES: NOT PROCESSED BY FARE DISPLAY" << std::endl;
          diag->flushMsg();
        }

        retval = NOTPROCESSED;
      }
    }
  }
  break;

  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT 16 FC SCOPE" );

    retval = penalties.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;
  case PricingUnitFactoryPhase:
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT 16 PU SCOPE" );

    retval = penalties.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;
  case PreCombinabilityPhase:
    if (UNLIKELY(diagEnabled))
    {
      (*diag) << "CATEGORY 16 - PENALTIES" << std::endl
              << "PRECOMBINABILITY VALIDATION PHASE NOT SUPPORTED" << std::endl;
    }
    break;
  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  if (UNLIKELY(diag))
  {
    if (PricingTrx::MIP_TRX == _trx->getTrxType() && _trx->isFlexFare() && _chancelor)
    {
      diag->printFlexFareNoPenalties(
          _paxTypeFare->getFlexFaresValidationStatus(), retval, _chancelor->getContext());
    }
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleSalesRestrictions()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 15" );

  SalesRestrictionRuleWrapper srr;
  srr.setRuleDataAccess(_rcDataAccess);

  const SalesRestriction* sr = dynamic_cast<const SalesRestriction*>(_ruleItemInfo);

  bool isQualifiedCategory = false;

  if (_cri->categoryNumber() != RuleConst::SALE_RESTRICTIONS_RULE)
    isQualifiedCategory = true;

  Record3ReturnTypes retval;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic315))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);
    diagPtr->enable(Diagnostic315);
    diagEnabled = true;
  }

  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    if (_ruleItemInfo->overrideDateTblItemNo())
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic315))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleSalesRestriction: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }

    FDSalesRestrictionRule fdsrr;

    retval = fdsrr.validate(*_trx,
                            *_itin,
                            //                               _fareUsage,
                            _fareUsage,
                            *_paxTypeFare,
                            *_cri,
                            _rule,
                            sr,
                            isQualifiedCategory,
                            *_isCat15Security,
                            _skipCat15Security);
  }
  break;

  // Fall through to FarePathFactoryPhase
  case FarePhase:
  case PricingUnitFactoryPhase:
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT 15 PU SCOPE" );

    if (_ruleItemInfo->overrideDateTblItemNo() != 0)
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::SALE_RESTRICTIONS_RULE, diagPtr, Diagnostic315);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
    retval = srr.validate(*_trx,
                          *_itin,
                          _fareUsage,
                          *_paxTypeFare,
                          *_cri,
                          _rule,
                          sr,
                          isQualifiedCategory,
                          *_isCat15Security,
                          _skipCat15Security);

    if (UNLIKELY(!isQualifiedCategory && _paxTypeFare->validForCmdPricing(_trx->fxCnException())))
    {
      if (sr->unavailTag() == RuleApplicationBase::dataUnavailable)
      {
        // For command pricing, skip records that dataUnavailable.
        // But give warning message, unless there is a PASS not in AND relationship
        if (_pricingUnit == nullptr)
          _paxTypeFare->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE);
        else
          (const_cast<PricingUnit*>(_pricingUnit))
              ->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE);

        retval = SKIP;
      }
      else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
      {
        if (_pricingUnit == nullptr)
          _paxTypeFare->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE, false);
        else
        {
          if ((const_cast<PricingUnit*>(_pricingUnit))->fareUsage().size() == 1)
          {
            (const_cast<PricingUnit*>(_pricingUnit))
                ->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE, false);
          }
        }
      }
    }
  }
  break;
  //    case FarePathFactoryPhase:
  //        retval = srr.validate(*_trx,*_fareUsage, sr);
  //        break;

  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleDayTime()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  2" );

  LOG4CXX_INFO(logger, " Entered RuleItem::handleDayTime(&, ...)");

 

  Diag302Collector* diag = nullptr;
  DCFactory* factory = nullptr;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic302))
  {
    factory = DCFactory::instance();
    diag = dynamic_cast<Diag302Collector*>(factory->create(*_trx));
    diag->enable(Diagnostic302);
  }

  // the following code will be moving up
  // when all functions
  // in DataHandler will be done.
  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diag,
                                                  Diagnostic302))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleDayTime: skipped by table 994");
        if (UNLIKELY(diag))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::DAY_TIME_RULE, diag, Diagnostic302);

      if (rtn != PASS)
      {
        if (UNLIKELY(diag))
        {
          diag->printLine();
          diag->flushMsg();
        }
        return rtn;
      }
    }
  }

  // lint -e{578}
  std::string phase = getRulePhaseString(_phase);

  if (UNLIKELY(diag))
  {
    diag->diag302Collector(*_paxTypeFare,
                           *_pricingUnit,
                           PASS,
                           phase,
                           *_cri,
                           *dynamic_cast<const DayTimeAppInfo*>(_ruleItemInfo));

    diag->flushMsg();
  }

  DayTimeApplication dta;
  dta.initialize(dynamic_cast<const DayTimeAppInfo*>(_ruleItemInfo));
  dta.setRuleDataAccess(_rcDataAccess);

  Record3ReturnTypes retval = FAIL;
  switch (_phase)
  {
  case FarePhase:
  {
    LOG4CXX_INFO(logger, " Entered RuleItem::handleDayTime(FarePhase)");
    // TSELatencyData metrics( *_trx, "CAT  2 FC SCOPE" );
    
    //apo-45023: if the rec 2 inoutInd is set, if  the itin is an intl itin
    // and if the fc is not the first one, then this fc could form a intl ooj pu.
    // in an intl ooj pu,  ooj exception could apply where in
    // the last fc rule validation should be done as inbound not outbound. So softpass such fares.
    if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(_trx) )
    {
       if ( (_rule->inOutInd() != RuleConst::ALWAYS_APPLIES) )
       {
          //is this an intl itin
          bool intlItin =  (_itin->geoTravelType() == GeoTravelType::International);
          //is this  the first fc
          bool isFirstFC = (_paxTypeFare->fareMarket()->travelSeg().front() == _itin->firstTravelSeg());
       
          if (!isFirstFC && intlItin)
          retval = SOFTPASS;
       }
       //the fc is a softpass bcoz of intl ooj. there is no need to validate it. 
       if (retval != SOFTPASS)
          retval = dta.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
    }
    else  //fallback remove at fallback removal time. 
    retval = dta.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;
  case PricingUnitFactoryPhase:
  {
    LOG4CXX_INFO(logger, " Entered RuleItem::handleDayTime(PricingUnit)");
    // TSELatencyData metrics( *_trx, "CAT  2 PU SCOPE" );

  //APO45023:  if intl ooj and if rec 2 i/o byte is set, then set if system defaults 
  //should be applied. 
  bool applyCat2SystemAssumptions = false;
  if (!fallback::apo45023ApplyCat2DefaultsInOOJPU(_trx) )
  {
     bool oojException = false;
     if ( _pricingUnit && (_pricingUnit->puSubType() == PricingUnit::ORIG_OPENJAW) &&
          !(_pricingUnit->sameNationOJ()) )
        oojException = true;
  
     if  ( oojException && (_rule->inOutInd() == RuleConst::FARE_MARKET_OUTBOUND)  &&
           (_fareUsage->isInbound()) )
        applyCat2SystemAssumptions = true;
  }
    if (applyCat2SystemAssumptions )
      dta.setApplyCat2SystemDefaults(applyCat2SystemAssumptions);

    retval = dta.validate(*_trx, _ruleItemInfo, nullptr, *_pricingUnit, *_fareUsage);
  }
  break;
  case FarePathFactoryPhase:
  {
    LOG4CXX_INFO(logger, " Entered RuleItem::handleDayTime(PricingUnit)");
    // TSELatencyData metrics( *_trx, "CAT  2 PU SCOPE" );

    retval = dta.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;

  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    bool isQualifiedCategory = false;

    if (_rule && _rule->relationalInd() == CategoryRuleItemInfo::IF)
    {
      isQualifiedCategory = true;
    }

    FDDayTimeApplication fdDayTimeApp;
    retval = fdDayTimeApp.validate(*_trx,
                                   *_itin,
                                   *_paxTypeFare,
                                   _ruleItemInfo,
                                   *_paxTypeFare->fareMarket(),
                                   isQualifiedCategory,
                                   _isInbound);
  }
  break;

  default:
    // Whatever phase it is, I don't handle it
    // Handling is left to the reader as an exercise
    if (UNLIKELY(diag))
    {
      (*diag) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diag))
  {
    const char* result;
    switch (retval)
    {
    case FAIL:
      result = "FAIL";
      break;
    case PASS:
      result = "PASS";
      break;
    case SKIP:
      result = "SKIP";
      break;
    case SOFTPASS:
      result = "SOFTPASS";
      break;
    default:
      result = "";
    }

    *diag << "STATUS : " << result << std::endl;
    diag->flushMsg();
  }

  return retval;
}

// partial cat 15 validation
Record3ReturnTypes
RuleItem::handleSalesSecurity()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 15" );

  SalesRestrictionRuleWrapper srr;
  if (_trx && _trx->isValidatingCxrGsaApplicable())
  {
    srr.setRuleDataAccess(_rcDataAccess);
  }

  const SalesRestriction* sr = dynamic_cast<const SalesRestriction*>(_ruleItemInfo);

  // Try to do the normal cat15 validation for the prevalidation phase.
  // If there is no problem,
  //   handleSalesSecurity() function will be removed.

  // bool failedSabre = false;
  Record3ReturnTypes retval;
  bool isQualifiedCategory = false;

  if (_cri->categoryNumber() != RuleConst::SALE_RESTRICTIONS_RULE)
    isQualifiedCategory = true;

  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    if (FareDisplayUtil::isCat15TuningEnabled())
    {
      // Do whole CAT 15 during prevalidation
      FDSalesRestrictionRule fdsrr;

      retval = fdsrr.validate(*_trx,
                              *_itin,
                              _fareUsage,
                              *_paxTypeFare,
                              *_cri,
                              _rule,
                              sr,
                              isQualifiedCategory,
                              *_isCat15Security,
                              _skipCat15Security);
    }
    else
    {
      // Else just validate security during prevalidation
      bool failedSabre = false;

      if (checkSaleSecurity(srr.getSalesRestrictionRule(),
                            *_trx,
                            *_paxTypeFare,
                            *_cri,
                            _rule,
                            sr,
                            failedSabre,
                            true,
                            false))
      {
        retval = PASS;
      }
      else
      {
        retval = FAIL;
      }
    }
    return retval;
  }

  default:
    break;
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    DCFactory* factory = nullptr;
    DiagCollector* diagPtr = nullptr;
    bool diagEnabled = false;

    if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic315))
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(*_trx);

      diagPtr->enable(Diagnostic315);
      diagEnabled = true;
    }
    if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic315))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleSalesSecurity: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::SALE_RESTRICTIONS_RULE, diagPtr, Diagnostic315);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }
  retval = srr.validate(*_trx,
                        *_itin,
                        _fareUsage,
                        *_paxTypeFare,
                        *_cri,
                        _rule,
                        sr,
                        isQualifiedCategory,
                        *_isCat15Security,
                        _skipCat15Security);

  if (UNLIKELY(!isQualifiedCategory && _paxTypeFare->validForCmdPricing(_trx->fxCnException())))
  {
    if (sr->unavailTag() == RuleApplicationBase::dataUnavailable)
    {
      // For command pricing, skip records that dataUnavailable.
      // But give warning message, unless there is a PASS not in AND relationship
      if (_pricingUnit == nullptr)
        _paxTypeFare->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE);
      else
        (const_cast<PricingUnit*>(_pricingUnit))
            ->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE);

      retval = SKIP;
    }
    else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
    {
      if (_pricingUnit == nullptr)
        _paxTypeFare->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE, false);
      else
        (const_cast<PricingUnit*>(_pricingUnit))
            ->setCmdPrcFailedFlag(RuleConst::SALE_RESTRICTIONS_RULE, false);
    }
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleStopovers()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  8" );

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic308))
  {
    bool activate =
        _fareUsage ? diagPtr->parseFareMarket(*_trx, *(_fareUsage->paxTypeFare()->fareMarket()))
                   : diagPtr->parseFareMarket(*_trx, *(_paxTypeFare->fareMarket()));
    if (activate)
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(*_trx);
      diagPtr->enable(Diagnostic308);
      diagEnabled = true;
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 08 - STOPOVERS DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FarePhase:
    case DynamicValidationPhase:
      (*diagPtr) << "PHASE: " << ((_phase == DynamicValidationPhase) ? "DYNAMIC " : "")
                 << "FARE VALIDATOR   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
                 << _paxTypeFare->fareMarket()->destination()->loc() << " "
                 << _paxTypeFare->fareClass();
      if (_paxTypeFare->fareMarket()->direction() == FMDirection::INBOUND)
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;

    case FarePathFactoryPhase:
      (*diagPtr) << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass();
      if (_fareUsage->isInbound())
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass();
      if (_fareUsage->isInbound())
      {
        (*diagPtr) << " .IN.   ";
      }
      else
      {
        (*diagPtr) << " .OUT.  ";
      }
      break;
    default:
      // Nothing here
      break;
    }

    (*diagPtr) << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
               << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  // the following code will be moving up
  // when all functions
  // in DataHandler will be done.
  if (_ruleItemInfo->overrideDateTblItemNo())
  {
    if (LIKELY(_phase != FareDisplayPhase && _phase != RuleDisplayPhase))
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::STOPOVER_RULE, diagPtr, Diagnostic308);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  Record3ReturnTypes retval = PASS;

  Stopovers so;
  so.setRuleDataAccess(_rcDataAccess);
  so.setRtw(_trx->getOptions() && _trx->getOptions()->isRtw());

  switch (_phase)
  {
  case RuleDisplayPhase:
  {
    retval = PASS;
  }
  break;
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  8 FC SCOPE" );

    retval = so.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  8 PU SCOPE" );

    retval = so.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;
  case FareDisplayPhase:
  {
    // Fare Display does not support this rule category
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY 08 - STOPOVERS: NOT PROCESSED BY FARE DISPLAY" << std::endl;
      diagPtr->flushMsg();
    }
    retval = NOTPROCESSED;
  }
  break;
  case DynamicValidationPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  8 FC SCOPE" );

    retval =
        so.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket(), true);
  }
  break;
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleTransfers()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  9" );

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic309))
  {
    bool activate =
        _fareUsage ? diagPtr->parseFareMarket(*_trx, *(_fareUsage->paxTypeFare()->fareMarket()))
                   : diagPtr->parseFareMarket(*_trx, *(_paxTypeFare->fareMarket()));
    if (activate)
    {
      factory = DCFactory::instance();
      diagPtr = factory->create(*_trx);
      diagPtr->enable(Diagnostic309);
      diagEnabled = true;
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 09 - TRANSFERS DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FarePhase:
    case DynamicValidationPhase:
      (*diagPtr) << "PHASE: " << ((_phase == DynamicValidationPhase) ? "DYNAMIC " : "")
                 << "FARE VALIDATOR   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      (*diagPtr) << _paxTypeFare->fareMarket()->origin()->loc() << " "
                 << _paxTypeFare->fareMarket()->destination()->loc() << " "
                 << _paxTypeFare->fareClass() << "     ";
      break;

    case FarePathFactoryPhase:
      (*diagPtr) << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass() << "     ";
      break;
    case PreCombinabilityPhase:
      (*diagPtr) << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                 << std::endl << _fareUsage->paxTypeFare()->fareMarket()->origin()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareMarket()->destination()->loc() << " "
                 << _fareUsage->paxTypeFare()->fareClass() << "     ";
      break;
    default:
      // Nothing here
      break;
    }

    (*diagPtr) << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
               << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diagPtr->flushMsg();
  }

  // the following code will be moving up
  // when all functions
  // in DataHandler will be done.
  if (_ruleItemInfo->overrideDateTblItemNo())
  {
    if (_phase != FareDisplayPhase && _phase != RuleDisplayPhase)
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::TRANSFER_RULE, diagPtr, Diagnostic309);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  Record3ReturnTypes retval = PASS;

  Transfers1 tr;
  tr.setRuleDataAccess(_rcDataAccess);
  tr.setRtw(_trx->getOptions() && _trx->getOptions()->isRtw());

  switch (_phase)
  {
  case RuleDisplayPhase:
  {
    retval = PASS;
  }
  break;
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  9 FC SCOPE" );

    retval = tr.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  9 PU SCOPE" );
    // for directionality '3' or '4' need to validate surface sectors on PU level
    tr.needSurfValidationForFP() = _rule->directionality() == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2 ||
                                   _rule->directionality() == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1;

    retval = tr.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;
  case FareDisplayPhase:
  {
    // Fare Display does not support this rule category
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY 09 - TRANSFERS: NOT PROCESSED BY FARE DISPLAY" << std::endl;
      diagPtr->flushMsg();
    }
    retval = NOTPROCESSED;
  }
  break;
  case DynamicValidationPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  9 FC SCOPE" );
    tr.excludeSurfaceBytes() = true;
    retval =
        tr.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket(), true);
  }
  break;
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleMinStayRestriction()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  6" );

  if (UNLIKELY(ruleIgnored(RuleConst::MINIMUM_STAY_RULE)))
    return PASS; // for Cat31 transaction

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic306))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic306);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    DiagCollector& diag = *diagPtr;

    diag << "CATEGORY 06 - MIN STAY APPLICATION DIAGNOSTICS" << std::endl;

    diag << "PHASE: " << getRulePhaseString(_phase) << "    R3 ITEM NUMBER: ";

    diag << _ruleItemInfo->itemNo() << std::endl;
    diag << _paxTypeFare->fareMarket()->origin()->loc() << " "
         << _paxTypeFare->fareMarket()->destination()->loc() << " " << _paxTypeFare->fareClass()
         << "     ";
    diag << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
         << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diag.flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic306))
      {
        LOG4CXX_INFO(logger, " RuleItem::MinStayRestriction: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::MINIMUM_STAY_RULE, diagPtr, Diagnostic306);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  MinimumStayApplication minStayApplication;
  minStayApplication.setRuleDataAccess(_rcDataAccess);
  minStayApplication.setChancelor(_chancelor);

  std::unique_ptr<MinStayApplicationObserverType> minStayObserver =
      MinStayApplicationObserverType::create(
          _trx->getRequest()->isSFR() ? (ObserverType::MIN_STAY_SFR) : (ObserverType::MIN_STAY),
          _trx->dataHandle(),
          &minStayApplication);

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case FCOPhase:
  case FarePhase:
    retval = minStayApplication.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
    break;

  case PricingUnitFactoryPhase:
    retval = minStayApplication.validate(*_trx, _ruleItemInfo, *_itin, *_pricingUnit, *_fareUsage);
    break;

  case FarePathFactoryPhase:
    retval =
        minStayApplication.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);

    break;

  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    const bool isQualifiedCategory = (_cri->categoryNumber() != RuleConst::MINIMUM_STAY_RULE);

    FDMinStayApplication fdMinStayApp;

    retval = fdMinStayApp.validate(*_trx,
                                   *_itin,
                                   *_paxTypeFare,
                                   _ruleItemInfo,
                                   *_paxTypeFare->fareMarket(),
                                   isQualifiedCategory);
  }
  break;

  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE\n";
    retval = PASS;
    break;
  }

  if (_fareUsage)
    minStayObserver->updateIfNotified(*_fareUsage);

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    printFlexFaresRestriction(*diagPtr, MINIMUM_STAY_RULE, retval);
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleMaxStayRestriction()
{
  // TSELatencyData metrics( *_trx, "RULE CAT  7" );

  if (UNLIKELY(ruleIgnored(RuleConst::MAXIMUM_STAY_RULE)))
    return PASS; // for Cat31 transaction

  MaximumStayApplication maxStayApplication;
  std::unique_ptr<MaxStayApplicationObserverType> maxStayObserver =
      MaxStayApplicationObserverType::create(
          _trx->getRequest()->isSFR() ? (ObserverType::MAX_STAY_SFR) : (ObserverType::MAX_STAY),
          _trx->dataHandle(),
          &maxStayApplication);
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic307))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic307);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    DiagCollector& diag = *diagPtr;

    diag << "CATEGORY 07 - MAX STAY APPLICATION DIAGNOSTICS" << std::endl;

    diag << "PHASE: " << getRulePhaseString(_phase) << "    R3 ITEM NUMBER: ";
    diag << _ruleItemInfo->itemNo() << std::endl;
    diag << _paxTypeFare->fareMarket()->origin()->loc() << " "
         << _paxTypeFare->fareMarket()->destination()->loc() << " " << _paxTypeFare->fareClass()
         << "     ";
    diag << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
         << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diag.flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic307))
      {
        LOG4CXX_INFO(logger, " RuleItem::MaxStayRestriction: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::MAXIMUM_STAY_RULE, diagPtr, Diagnostic307);
      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  maxStayApplication.setRuleDataAccess(_rcDataAccess);
  maxStayApplication.setChancelor(_chancelor);

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case FCOPhase:
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  7 FC SCOPE" );

    retval = maxStayApplication.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;
  case PricingUnitFactoryPhase:
  {
    retval = maxStayApplication.validate(*_trx, _ruleItemInfo, *_itin, *_pricingUnit, *_fareUsage);
  }
  break;
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT  7 PU SCOPE" );

    retval =
        maxStayApplication.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    const bool isQualifiedCategory = (_cri->categoryNumber() != RuleConst::MAXIMUM_STAY_RULE);

    FDMaxStayApplication fdMaxStayApp;

    retval = fdMaxStayApp.validate(*_trx,
                                   *_itin,
                                   *_paxTypeFare,
                                   _ruleItemInfo,
                                   *_paxTypeFare->fareMarket(),
                                   isQualifiedCategory);
  }
  break;
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (_fareUsage)
    maxStayObserver->updateIfNotified(*_fareUsage);

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    printFlexFaresRestriction(*diagPtr, MAXIMUM_STAY_RULE, retval);
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleTravelRestrictions()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 14" );

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  const bool isQualifiedCategory = (_cri->categoryNumber() != RuleConst::TRAVEL_RESTRICTIONS_RULE);

  if (!isQualifiedCategory)
    _paxTypeFare->warningMap().set(WarningMap::cat14_warning, true);

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic314))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic314);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    DiagCollector& diag = *diagPtr;

    diag << "    " << std::endl << "    " << std::endl;
    diag << "CATEGORY 14 - TRAVEL RESTRICTIONS DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FarePhase:
      diag << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      diag << _paxTypeFare->fareMarket()->origin()->loc() << " "
           << _paxTypeFare->fareMarket()->destination()->loc() << " " << _paxTypeFare->fareClass()
           << "     ";
      break;

    case PricingUnitFactoryPhase:
    case FarePathFactoryPhase:
    {
      diag << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      const PaxTypeFare& ptFare = *_fareUsage->paxTypeFare();
      diag << ptFare.fareMarket()->origin()->loc() << ptFare.fareMarket()->destination()->loc()
           << " " << ptFare.fareClass() << "  " << ptFare.fareAmount() << ptFare.currency() << " ";

      const std::vector<TravelSeg*>& tvlSegs = _pricingUnit->travelSeg();
      diag << "PU " << tvlSegs.front()->origAirport() << tvlSegs.back()->destAirport() << "   ";
    }
    break;

    case PreCombinabilityPhase:
      diag << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      diag << _fareUsage->paxTypeFare()->fareClass() << "     ";
      break;

    case FareDisplayPhase:
      diag << "PHASE: FARE DISPLAY VALIDATOR   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
           << std::endl;
      diag << _paxTypeFare->fareMarket()->origin()->loc() << " "
           << _paxTypeFare->fareMarket()->destination()->loc() << " " << _paxTypeFare->fareClass()
           << "     ";
      break;

    default:
      // Nothing here
      break;
    }

    diag << "R2:FARERULE : " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
         << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diag.flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic314))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleTravelRestriction: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }
    else
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::TRAVEL_RESTRICTIONS_RULE, diagPtr, Diagnostic314);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  Record3ReturnTypes retval;

  TravelRestrictionsObserverWrapper travelRestrictions;
  travelRestrictions.setRuleDataAccess(_rcDataAccess);

  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    FDTravelRestrictions fdTravelRest;

    retval = fdTravelRest.validate(*_trx,
                                   *_itin,
                                   *_paxTypeFare,
                                   _ruleItemInfo,
                                   *_paxTypeFare->fareMarket(),
                                   isQualifiedCategory);
  }
  break;

  case FarePhase:
  {
    retval = travelRestrictions.validate(
        *_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());

    if (UNLIKELY(!isQualifiedCategory && _paxTypeFare->validForCmdPricing(_trx->fxCnException())))
    {
      if (retval == FAIL)
      {
        // usually we would not let fares failed cat14
        // to be command priced. But if we failed because
        // of unavailableFlag. We can pass and give warningmessage
        const TravelRestriction* tvlRestrictionInfo =
            dynamic_cast<const TravelRestriction*>(_ruleItemInfo);

        if (tvlRestrictionInfo &&
            tvlRestrictionInfo->unavailTag() == RuleApplicationBase::dataUnavailable)
        {
          _paxTypeFare->setCmdPrcFailedFlag(RuleConst::TRAVEL_RESTRICTIONS_RULE);
          retval = SKIP;
        }
      }
      else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
      {
        _paxTypeFare->setCmdPrcFailedFlag(RuleConst::TRAVEL_RESTRICTIONS_RULE, false);
      }
    }
  }
  break;

  case PricingUnitFactoryPhase:
    retval = travelRestrictions.validate(*_trx, _ruleItemInfo, *_itin, *_pricingUnit, *_fareUsage);
    break;

  case FarePathFactoryPhase:
    retval =
        travelRestrictions.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
    break;

  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE\n";
    retval = PASS;
    break;
  }

  if (_phase == PricingUnitFactoryPhase || _phase == FarePathFactoryPhase)
  {
    if (UNLIKELY(!isQualifiedCategory &&
                  _fareUsage->paxTypeFare()->validForCmdPricing(_trx->fxCnException())))
    {
      if (retval == FAIL)
      {
        // usually we would not let fares failed cat14
        // to be command priced. But if we failed because
        // of unavailableFlag. We can pass and give warningmessage
        const TravelRestriction* tvlRestrictionInfo =
            dynamic_cast<const TravelRestriction*>(_ruleItemInfo);

        if (tvlRestrictionInfo &&
            tvlRestrictionInfo->unavailTag() == RuleApplicationBase::dataUnavailable)
        {
          (const_cast<PricingUnit*>(_pricingUnit))
              ->setCmdPrcFailedFlag(RuleConst::TRAVEL_RESTRICTIONS_RULE);
          retval = SKIP;
        }
      }
      else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
      {
        (const_cast<PricingUnit*>(_pricingUnit))
            ->setCmdPrcFailedFlag(RuleConst::TRAVEL_RESTRICTIONS_RULE, false);
      }
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleSurchargeRule()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 12" );

  SurchargesRule srr;
  srr.setRuleDataAccess(_rcDataAccess);
  const SurchargesInfo* sr = dynamic_cast<const SurchargesInfo*>(_ruleItemInfo);

  Record3ReturnTypes retval;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic312))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);
    diagPtr->enable(Diagnostic312);
    diagEnabled = true;
  }

  switch (_phase)
  {
  case RuleDisplayPhase: // On purpose, don't add break!
  case FareDisplayPhase:
  {
    if (_ruleItemInfo->overrideDateTblItemNo())
    {
      if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                  _isInbound,
                                                  _ruleItemInfo->overrideDateTblItemNo(),
                                                  _cri->vendorCode(),
                                                  _trx->ticketingDate(),
                                                  _trx->bookingDate(),
                                                  diagPtr,
                                                  Diagnostic312))
      {
        LOG4CXX_INFO(logger, " RuleItem::handleSurchargesRules: skipped by table 994");
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return SKIP;
      }
    }

    FDSurchargesRule fdSurchargesRule;

    retval = fdSurchargesRule.process(*_trx, *_paxTypeFare, _rule, sr);
  }
  break;

  case FarePhase:
  case FarePathFactoryPhase:
  {
    if (_ruleItemInfo->overrideDateTblItemNo() != 0)
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::SURCHARGE_RULE, diagPtr, Diagnostic312);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }

    if (UNLIKELY(_phase == FarePhase))
    {
      if (TrxUtil::isFVOSurchargesEnabled())
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          *diagPtr << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo()
                   << "\n";
          diagPtr->flushMsg();
        }

        srr.initalizeCat25Ptr(_paxTypeFare->cat25Fare());
        retval = srr.validate(*_trx, *sr, *_paxTypeFare);
      }
      else
        retval = SOFTPASS;
      break;
    }

    if (UNLIKELY(diagEnabled && TrxUtil::isFVOSurchargesEnabled()))
    {
      diagPtr->printLine();
      *diagPtr << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << "\n";
      diagPtr->flushMsg();
    }

    srr.initalizeCat25Ptr(_fareUsage->cat25Fare());

    retval = srr.validate(*_trx, *_farePath, *_pricingUnit, *_fareUsage, sr);
  }
  break;

  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleAccompaniedTravel()
{
  // TSELatencyData metrics( *_trx, "RULE CAT 13" );

  AccompaniedTravel accTvl;

  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic313))
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic313);
    diagEnabled = true;
  }

  if (UNLIKELY(diagEnabled))
  {
    DiagCollector& diag = *diagPtr;

    diag << "CATEGORY 13 - ACCOMPANIED TRAVEL DIAGNOSTICS" << std::endl;

    switch (_phase)
    {
    case FarePhase:
      diag << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      diag << _paxTypeFare->fareMarket()->origin()->loc() << " "
           << _paxTypeFare->fareMarket()->destination()->loc() << " " << _paxTypeFare->fareClass()
           << "     ";
      break;

    case FarePathFactoryPhase:
      diag << "PHASE: PRICING UNIT      R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      diag << _fareUsage->paxTypeFare()->fareClass() << "     ";
      break;
    case PreCombinabilityPhase:
      diag << "PHASE: PRECOMBINABILITY  R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
      diag << _fareUsage->paxTypeFare()->fareClass() << "     ";
      break;
    default:
      // Nothing here
      break;
    }

    diag << "R2:FARERULE    :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
         << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

    diag.flushMsg();
  }

  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (_phase != FareDisplayPhase && _phase != RuleDisplayPhase)
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::ACCOMPANIED_PSG_RULE, diagPtr, Diagnostic313);

      if (rtn != PASS)
      {
        if (UNLIKELY(diagEnabled))
        {
          diagPtr->printLine();
          diagPtr->flushMsg();
        }
        return rtn;
      }
    }
  }

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case RuleDisplayPhase:
  {
    retval = PASS;
  }
  break;
  case FarePhase:
  {
    // TSELatencyData metrics( *_trx, "CAT 13 FC SCOPE" );

    retval =
        accTvl.validate(*_trx, *_itin, *_paxTypeFare, _ruleItemInfo, *_paxTypeFare->fareMarket());
  }
  break;

  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, "CAT 13 PU SCOPE" );

    retval = accTvl.validate(*_trx, _ruleItemInfo, *_farePath, *_pricingUnit, *_fareUsage);
  }
  break;

  case FareDisplayPhase:
  {
    // Fare Display does not support this rule category
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY 13 - ACCOMPANIED TRAVEL: NOT PROCESSED BY FARE DISPLAY" << std::endl;
      diagPtr->flushMsg();
    }
    retval = NOTPROCESSED;
  }
  break;

  // Re-validation with vector of FareUsage is not called here
  default:
    // Whatever phase it is, I don't handle it
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "CATEGORY DOES NOT SUPPORT VALIDATION PHASE" << std::endl;
    }
    retval = PASS;
    break;
  }

  if (UNLIKELY(diagEnabled))
  {
    diagPtr->printLine();
    diagPtr->flushMsg();
  }
  return retval;
}

Record3ReturnTypes
RuleItem::handleMiscFareTags()
{
  // TSELatencyData metrics( *_trx, MetricsUtil::RULE3CAT_23 );

  Record3ReturnTypes retval;
  switch (_phase)
  {
  case RuleDisplayPhase:
  case FareDisplayPhase:
  // Fall through to FarePhase

  // FarePhase and FarePathFactoryPhase are the same for the MiscFareTagsRule validation.
  // FarePathFactoryPhase is used when a directionality=3/4 in the record 2 string.

  case FarePhase:
    // Fall through to FarePathFactoryPhase
    {
      ////TSELatencyData metrics( *_trx, MetricsUtil::RULE3CAT_23_FC );
    }

  // lint -e{616}
  case FarePathFactoryPhase:
  {
    // TSELatencyData metrics( *_trx, MetricsUtil::RULE3CAT_23_PU );
    if (_ruleItemInfo->overrideDateTblItemNo() != 0)
    {
      DCFactory* factory = nullptr;
      DiagCollector* diagPtr = nullptr;
      bool diagEnabled = false;

      if (_trx->diagnostic().diagnosticType() == Diagnostic323)
      {
        factory = DCFactory::instance();
        diagPtr = factory->create(*_trx);

        diagPtr->enable(Diagnostic323);
        diagEnabled = true;
      }
      if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
      {
        if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                    _isInbound,
                                                    _ruleItemInfo->overrideDateTblItemNo(),
                                                    _cri->vendorCode(),
                                                    _trx->ticketingDate(),
                                                    _trx->bookingDate(),
                                                    diagPtr,
                                                    Diagnostic323))
        {
          LOG4CXX_INFO(logger, " RuleItem::handleMiscFareTags: skipped by table 994");
          if (UNLIKELY(diagEnabled))
          {
            diagPtr->printLine();
            diagPtr->flushMsg();
          }
          return SKIP;
        }
      }
      else
      {
        Record3ReturnTypes rtn =
            validateDateOverrideRuleItem(RuleConst::MISC_FARE_TAG, diagPtr, Diagnostic323);

        if (rtn != PASS)
        {
          if (UNLIKELY(diagEnabled))
          {
            diagPtr->printLine();
            diagPtr->flushMsg();
          }
          return rtn;
        }
      }
    }

    MiscFareTag* mft = const_cast<MiscFareTag*>(dynamic_cast<const MiscFareTag*>(_ruleItemInfo));
    if (LIKELY(mft))
    {
      if (UNLIKELY((_phase == RuleDisplayPhase || _phase == FareDisplayPhase) &&
                    mft->unavailtag() == RuleConst::DATA_UNAVAILABLE))
      {
        FareDisplayInfo* fdInfo = _paxTypeFare->fareDisplayInfo();

        if (fdInfo)
        {
          fdInfo->setUnavailableR3Rule(RuleConst::MISC_FARE_TAG);
          return NOTPROCESSED;
        }
      }

      MiscFareTagsRule mftr;
      retval = mftr.validate(*_trx, *_paxTypeFare, mft);

      if (UNLIKELY(_paxTypeFare->validForCmdPricing(_trx->fxCnException())))
      {
        if (mft->unavailtag() == RuleApplicationBase::dataUnavailable)
        {
          // usually we would not let fares failed cat23
          // to be command priced. But if we failed because
          // of unavailableFlag. We can pass and give warningmessage
          _paxTypeFare->setCmdPrcFailedFlag(RuleConst::MISC_FARE_TAG);
          retval = SKIP;
        }
        else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
        {
          _paxTypeFare->setCmdPrcFailedFlag(RuleConst::MISC_FARE_TAG, false);
        }
      }
    }
    else
      retval = PASS;
  }
  break;

  default:
    // Whatever phase it is, I don't handle it
    retval = PASS;
    break;
  }

  return retval;
}

// cat 23 validation for the published fares only
// (to check applicablebyte#8 (constructed) )
Record3ReturnTypes
RuleItem::handleMiscFareTagsForPubl()
{
  // TSELatencyData metrics( *_trx, MetricsUtil::RULE3CAT_23 );

  Record3ReturnTypes retval = NOTPROCESSED;
  // TSELatencyData metrics( *_trx, MetricsUtil::RULE3CAT_23_PU );
  switch (_phase)
  {
  // Rule and Fare Display now do CAT23
  case RuleDisplayPhase:
  case FareDisplayPhase:
  // Fall through to default

  default:
  {
    if (_ruleItemInfo->overrideDateTblItemNo() != 0)
    {
      DCFactory* factory = nullptr;
      DiagCollector* diagPtr = nullptr;
      bool diagEnabled = false;

      if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic323))
      {
        factory = DCFactory::instance();
        diagPtr = factory->create(*_trx);

        diagPtr->enable(Diagnostic323);
        diagEnabled = true;
      }
      if (UNLIKELY(_phase == FareDisplayPhase || _phase == RuleDisplayPhase))
      {
        if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                    _isInbound,
                                                    _ruleItemInfo->overrideDateTblItemNo(),
                                                    _cri->vendorCode(),
                                                    _trx->ticketingDate(),
                                                    _trx->bookingDate(),
                                                    diagPtr,
                                                    Diagnostic323))
        {
          LOG4CXX_INFO(logger, " RuleItem::handleMiscFareTagsForBubl: skipped by table 994");
          if (UNLIKELY(diagEnabled))
          {
            diagPtr->printLine();
            diagPtr->flushMsg();
          }
          return SKIP;
        }
      }
      else
      {
        Record3ReturnTypes rtn =
            validateDateOverrideRuleItem(RuleConst::MISC_FARE_TAG, diagPtr, Diagnostic323);

        if (rtn != PASS)
        {
          if (UNLIKELY(diagEnabled))
          {
            diagPtr->printLine();
            diagPtr->flushMsg();
          }
          return rtn;
        }
      }
    }
    MiscFareTag* mft = const_cast<MiscFareTag*>(dynamic_cast<const MiscFareTag*>(_ruleItemInfo));
    if (LIKELY(mft))
    {
      if (UNLIKELY((_phase == RuleDisplayPhase || _phase == FareDisplayPhase) &&
                    mft->unavailtag() == RuleConst::DATA_UNAVAILABLE))
      {
        FareDisplayInfo* fdInfo = _paxTypeFare->fareDisplayInfo();

        if (fdInfo)
        {
          fdInfo->setUnavailableR3Rule(RuleConst::MISC_FARE_TAG);
          return NOTPROCESSED;
        }
      }

      else
      {
        MiscFareTagsRule mftr;
        //        retval = mftr.validate(*_trx,
        //                                                                                          *_paxTypeFare,
        //                                                                                          mft);
        retval = mftr.validatePublished(*_trx, *_paxTypeFare, mft);

        if (UNLIKELY(_paxTypeFare->validForCmdPricing(_trx->fxCnException())))
        {
          if (mft->unavailtag() == RuleApplicationBase::dataUnavailable)
          {
            // usually we would not let fares failed cat23
            // to be command priced. But if we failed because
            // of unavailableFlag. We can pass and give warningmessage
            _paxTypeFare->setCmdPrcFailedFlag(RuleConst::MISC_FARE_TAG);
            retval = SKIP;
          }
          else if (retval == PASS && _rule->relationalInd() != CategoryRuleItemInfo::AND)
          {
            _paxTypeFare->setCmdPrcFailedFlag(RuleConst::MISC_FARE_TAG, false);
          }
        }
      }
    }
    else
      retval = PASS;
  }
  break;
  }

  return retval;
}

Record3ReturnTypes
RuleItem::handleTicketingEndorsement()
{
  if (_ruleItemInfo->overrideDateTblItemNo() != 0)
  {
    if (_phase != FareDisplayPhase && _phase != RuleDisplayPhase)
    {
      Record3ReturnTypes rtn =
          validateDateOverrideRuleItem(RuleConst::TICKET_ENDORSMENT_RULE, nullptr, DiagnosticNone);

      if (rtn != PASS)
      {
        return rtn;
      }
    }
  }

  if (UNLIKELY((_phase == FareDisplayPhase) || (_phase == RuleDisplayPhase)))
  {
    // Fare Display does not support this rule category
    return NOTPROCESSED;
  }

  const TicketEndorsementsInfo* endorsementInfo =
      dynamic_cast<const TicketEndorsementsInfo*>(_ruleItemInfo);

  if (UNLIKELY(!endorsementInfo))
    return PASS;

  TicketingEndorsement endorsement;

  endorsement.initialize(endorsementInfo);
  _paxTypeFare->setCategoryValid(RuleConst::TICKET_ENDORSMENT_RULE, true);
  endorsement.process(*_trx, *_fareUsage, *_paxTypeFare);

  return PASS;
}

Record3ReturnTypes
RuleItem::handleTours()
{
  Record3ReturnTypes result = PASS;

  const Tours* tours = dynamic_cast<const Tours*>(_ruleItemInfo);

  if (LIKELY(tours))
  {
    Diag327Collector* diag = nullptr;
    DCFactory* factory = DCFactory::instance();

    if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic327))
    {
      diag = dynamic_cast<Diag327Collector*>(factory->create(*_trx));

      diag->enable(Diagnostic327);
      diag->collect(_phase, *_paxTypeFare, *_cri, *tours);
    }

    if (UNLIKELY(_ruleItemInfo->overrideDateTblItemNo() != 0))
    {
      if (_phase == FareDisplayPhase || _phase == RuleDisplayPhase)
      {
        if (!RuleUtil::validateDateOverrideRuleItem(*_trx,
                                                    _isInbound,
                                                    _ruleItemInfo->overrideDateTblItemNo(),
                                                    _cri->vendorCode(),
                                                    _trx->ticketingDate(),
                                                    _trx->bookingDate(),
                                                    diag,
                                                    Diagnostic327))
        {
          LOG4CXX_INFO(logger, " RuleItem::handleTours: skipped by table 994");
          result = SKIP;
        }
      }
      else
      {
        result = validateDateOverrideRuleItem(RuleConst::TOURS_RULE, diag, Diagnostic327);
      }
    }

    if (LIKELY(result == PASS))
    {
      ToursApplication toursApp(_trx);

      switch (_phase)
      {
      case PricingUnitPhase:
        result = toursApp.validate(_pricingUnit);
        break;
      case FarePathFactoryPhase:
        result = toursApp.validate(_farePath);
        break;
      default:
        break;
      }
    }

    if (diag != nullptr)
    {
      diag->displayStatus(result);
      diag->flushMsg();
    }
  }
  // For FD trx we should return SKIP instead of PASS to be able to display all R3s in diagnostic
  if (UNLIKELY(_trx->getTrxType() == PricingTrx::FAREDISPLAY_TRX && result == PASS))
    result = SKIP;

  return result;
}

Record3ReturnTypes
RuleItem::handleVoluntaryExc()
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic331))
  {
    diagEnabled = true;

    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);

    diagPtr->enable(Diagnostic331);
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 31 - VOLUNTARY EXCHANGE";
    printDiag33XHeader(*diagPtr);
  }

  const VoluntaryChangesInfo* voluntChgInfo =
      dynamic_cast<const VoluntaryChangesInfo*>(_ruleItemInfo);

  if (!voluntChgInfo)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "NOT VALID VOLUNTARY EXCHANGE RULE\n";
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  RexPricingTrx* rexTrx = static_cast<RexPricingTrx*>(_trx);

  const Itin* itin = _farePath ? _farePath->itin() : rexTrx->itin().front();

  // Note: overrideDateTblItemNo for Cat31 is validated within VoluntaryChanges.
  VoluntaryChanges voluntaryChanges(*rexTrx, itin, _pricingUnit);
  Record3ReturnTypes rtn = SKIP;

  if (_phase != RuleDisplayPhase)
  {
    rtn = voluntaryChanges.validate(*_fareUsage, *voluntChgInfo);
  }

  if (UNLIKELY(diagEnabled))
  {
    *diagPtr << "R3 VALIDATION RESULT: " << rtn << "\n";
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  if (rtn == PASS || rtn == SOFTPASS)
  {
    // cat31 was pre-set as invalid, when any record3 PASS, we need to set
    // it as valid
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
  }
  else if (rtn == FAIL)
  {
    return SKIP; // in order not to have FAIL result after any record3
    // rule item pass, we return SKIP
  }

  return rtn;
}

Record3ReturnTypes
RuleItem::handleVoluntaryRefund()
{
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;

  bool diagEnabled = false;

  if (UNLIKELY(_trx->diagnostic().diagnosticType() == Diagnostic333))
  {
    diagEnabled = true;
    factory = DCFactory::instance();
    diagPtr = factory->create(*_trx);
    diagPtr->enable(Diagnostic333);
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "CATEGORY 33 - VOLUNTARY REFUND";
    printDiag33XHeader(*diagPtr);
    diagPtr->flushMsg();
  }

  const VoluntaryRefundsInfo* voluntaryRefundsInfo =
      dynamic_cast<const VoluntaryRefundsInfo*>(_ruleItemInfo);

  if (!voluntaryRefundsInfo)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << "NOT VALID VOLUNTARY REFUND RULE\n";
      diagPtr->flushMsg();
    }
    return FAIL;
  }

  Record3ReturnTypes rtn = SKIP;

  if (_phase != RuleDisplayPhase)
  {
    RefundPricingTrx& refundTrx = static_cast<RefundPricingTrx&>(*_trx);

    const Itin* itin = _farePath ? _farePath->itin() : refundTrx.itin().front();

    VoluntaryRefunds voluntaryRefunds(
        refundTrx, itin, _pricingUnit, *_paxTypeFare, *voluntaryRefundsInfo, diagPtr);
    rtn = voluntaryRefunds.validate();
  }

  if (UNLIKELY(diagEnabled))
  {
    *diagPtr << "R3 VALIDATION RESULT: " << rtn << "\n";
    diagPtr->printLine();
    diagPtr->flushMsg();
  }

  if (rtn == PASS || rtn == SOFTPASS)
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE, true);

  else if (rtn == FAIL)
    return SKIP;

  return rtn;
}

void
RuleItem::printDiag33XHeader(DiagCollector& diag) const
{
  const PaxTypeFare* correctPtf =
      (_trx->excTrxType() == PricingTrx::AR_EXC_TRX && _fareUsage->cat25Fare())
          ? _fareUsage->cat25Fare()
          : _paxTypeFare;

  diag << " DIAGNOSTICS\nPHASE: ";

  if (_phase == FarePhase || _phase == PricingUnitFactoryPhase)
    diag << "FARE RULE MATCHING";

  else
    diag << _phase;

  diag << "   R3 ITEM NUMBER: " << _ruleItemInfo->itemNo() << std::endl;
  diag << correctPtf->fareMarket()->origin()->loc() << " "
       << correctPtf->fareMarket()->destination()->loc() << " " << correctPtf->fareClass();

  if (_trx->getTrxType() == PricingTrx::MIP_TRX && _trx->excTrxType() == PricingTrx::AR_EXC_TRX)
    diag << " " << (static_cast<BaseExchangeTrx*>(_trx))->itinIndex() << "  ";
  else
    diag << "     ";

  if (_cri != nullptr)
    diag << "R2:FARERULE  :  " << _cri->vendorCode() << " " << _cri->tariffNumber() << " "
         << _cri->carrierCode() << " " << _cri->ruleNumber() << std::endl;

  if (!correctPtf->fareMarket()->ruleApplicationDate().isEmptyDate() ||
      _trx->excTrxType() == PricingTrx::AF_EXC_TRX)
    diag << "RULE APPLICATION DATE: "
         << correctPtf->fareMarket()->ruleApplicationDate().dateToString(YYYYMMDD, "-")
         << std::endl;

  if (_phase == PricingUnitFactoryPhase)
  {
    if (_pricingUnit->travelSeg().empty())
      diag << "PU: "
           << "EMPTY TRAVEL SEG VECTOR\n";

    else
    {
      diag << "PU: " << _pricingUnit->travelSeg().front()->origAirport();

      if (_pricingUnit->turnAroundPoint())
        diag << "-" << _pricingUnit->turnAroundPoint()->origAirport();

      diag << "-" << _pricingUnit->travelSeg().back()->destAirport() << _pricingUnit->puType()
           << '\n';
    }
  }

  diag.flushMsg();
}

Record3ReturnTypes
RuleItem::validateDateOverrideRuleItem(const uint16_t categoryNumber,
                                       DiagCollector* diag,
                                       const DiagnosticTypes& callerDiag)
{
  const DiagnosticTypes diagType = _trx->diagnostic().diagnosticType();

  DataHandle dataHandle(_trx->ticketingDate());
  dataHandle.setParentDataHandle(&_trx->dataHandle());

  bool diagEnabled = false;

  if (UNLIKELY((diag) && (callerDiag != DiagnosticNone) && (callerDiag == diagType)))
  {
    diag->enable(diagType);
    diagEnabled = true;
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << "  -: TABLE 994 -OVERRIDE DATE DATA :- " << _cri->vendorCode() << " - "
            << _ruleItemInfo->overrideDateTblItemNo() << std::endl;
  }

  const std::vector<DateOverrideRuleItem*>& dorItemList = dataHandle.getDateOverrideRuleItem(
      _cri->vendorCode(), _ruleItemInfo->overrideDateTblItemNo());

  DateTime travelDate, reservationDate;

  bool gotTravelDate = false;
  bool gotReservationDate = false;

  NoPNRPricingTrx* noPNRPricingTrx = nullptr;
  if (UNLIKELY(categoryNumber == RuleConst::TOURS_RULE))
    noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(_trx);

  std::vector<DateOverrideRuleItem*>::const_iterator it = dorItemList.begin();
  const std::vector<DateOverrideRuleItem*>::const_iterator itEnd = dorItemList.end();
  for (; it != itEnd; it++)
  {
    DateOverrideRuleItem& dorItem = **it;
    bool rtn = PASS;

    if (UNLIKELY(diagEnabled))
    {
      printDorItem(*diag, dorItem);
    }

    if (dorItem.tvlEffDate().isValid())
    {
      if (LIKELY(!gotTravelDate))
      {
        gotTravelDate = true;

        rtn = RuleUtil::getTvlDateForTbl994Validation(travelDate,
                                                      categoryNumber,
                                                      *_paxTypeFare,
                                                      _ruleItemInfo,
                                                      _pricingUnit,
                                                      noPNRPricingTrx);

        if (UNLIKELY(diagEnabled))
        {
          (*diag) << "    TVL DTE  - " << travelDate.dateToString(DDMMMYY, "");
        }

        if (!rtn)
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diag) << "  TABLE 994: SOFTPASS" << std::endl;
            diag->flushMsg();
          }
          return SOFTPASS;
        }
      }
      if (dorItem.tvlEffDate().date() > travelDate.date())
        continue;
    }

    if (dorItem.tvlDiscDate().isValid())
    {
      if (!gotTravelDate)
      {
        gotTravelDate = true;

        rtn = RuleUtil::getTvlDateForTbl994Validation(travelDate,
                                                      categoryNumber,
                                                      *_paxTypeFare,
                                                      _ruleItemInfo,
                                                      _pricingUnit,
                                                      noPNRPricingTrx);

        if (UNLIKELY(diagEnabled))
        {
          (*diag) << "    TVL DTE  - " << travelDate.dateToString(DDMMMYY, "");
        }

        if (!rtn)
        {
          if (UNLIKELY(diagEnabled))
          {
            (*diag) << "  TABLE 994: SOFTPASS" << std::endl;
            diag->flushMsg();
          }
          return SOFTPASS;
        }
      }

      if (dorItem.tvlDiscDate().date() < travelDate.date())
        continue;
    }

    if (dorItem.tktEffDate().isValid() &&
        (dorItem.tktEffDate().date() > _trx->ticketingDate().date()))
      continue;

    if (dorItem.tktDiscDate().isValid() &&
        (dorItem.tktDiscDate().date() < _trx->ticketingDate().date()))
      continue;

    if (dorItem.resEffDate().isValid())
    {
      if (!gotReservationDate)
      {
        gotReservationDate = true;

        RuleUtil::getLatestBookingDate(*_trx, reservationDate, *_paxTypeFare);

        if (UNLIKELY(diagEnabled))
        {
          (*diag) << "    BOOK DTE  - " << reservationDate.dateToString(DDMMMYY, "");
        }
      }

      if (dorItem.resEffDate().date() > reservationDate.date())
        continue;
    }

    if (dorItem.resDiscDate().isValid())
    {
      if (!gotReservationDate)
      {
        gotReservationDate = true;

        RuleUtil::getLatestBookingDate(*_trx, reservationDate, *_paxTypeFare);

        if (UNLIKELY(diagEnabled))
        {
          (*diag) << "    BOOK DTE  - " << reservationDate.dateToString(DDMMMYY, "");
        }
      }

      if (dorItem.resDiscDate().date() < reservationDate.date())
        continue;
    }

    if (UNLIKELY(diagEnabled))
    {
      (*diag) << "  TABLE 994: PASS" << std::endl;
      diag->flushMsg();
    }
    return PASS;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diag) << "  TABLE 994: NOT MATCH" << std::endl;
    diag->flushMsg();
  }
  return SKIP;
}

PaxTypeFare*
RuleItem::determinePaxTypeFare(PaxTypeFare* ptFare, bool needBaseFare) const
{
  if (needBaseFare)
  {
    PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;
    paxTypeFareRuleData = ptFare->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
    if (LIKELY(paxTypeFareRuleData))
    {
      PaxTypeFare* baseFare = paxTypeFareRuleData->baseFare();
      if (LIKELY(baseFare != nullptr))
      {
        return baseFare;
      }
    }
    return ptFare;
  }
  else
  {
    return ptFare;
  }
}

bool
RuleItem::ruleIgnored(uint32_t categoryNumber)
{
  return (_fareUsage != nullptr && _fareUsage->isKeepFare() &&
          _fareUsage->categoryIgnoredForKeepFare().find(categoryNumber) !=
              _fareUsage->categoryIgnoredForKeepFare().end());
}

const std::string&
RuleItem::getRulePhaseString(const int phase)
{
  switch (phase)
  {
  case FarePhase:
    return FarePhaseStr;
  case FCOPhase:
    return FCOPhaseStr;
  case PricingUnitPhase:
    return PricingUnitPhaseStr;
  case PricingUnitFactoryPhase:
    return PricingUnitFactoryPhaseStr;
  case FarePathFactoryPhase:
    return FarePathFactoryPhaseStr;
  case PreCombinabilityPhase:
    return PreCombinabilityPhaseStr;
  case FareDisplayPhase:
    return FareDisplayPhaseStr;
  case RuleDisplayPhase:
    return RuleDisplayPhaseStr;
  case DynamicValidationPhase:
    return DynamicValidationPhaseStr;
  default:
    return UnknownPhaseStr;
  }
}

Record3ReturnTypes
RuleItem::preliminaryT994Validation(PricingTrx& trx,
                                    const CategoryRuleInfo& cri,
                                    const PricingUnit* pricingUnit,
                                    const PaxTypeFare& paxTypeFare,
                                    const RuleItemInfo* ruleItemInfo,
                                    const uint32_t categoryNumber)
{
  RuleItem rI(trx, cri, pricingUnit, paxTypeFare, ruleItemInfo);

  return rI.validateDateOverrideRuleItem(categoryNumber, nullptr, DiagnosticNone);
}

void
RuleItem::printDorItem(DiagCollector& diag, const DateOverrideRuleItem& dorItem) const
{
  diag << "    TVL EFF DTE  - " << dorItem.tvlEffDate().dateToString(DDMMMYY, "")
       << "  TVL DISC DTE - " << dorItem.tvlDiscDate().dateToString(DDMMMYY, "") << std::endl;
  diag << "    TKT EFF DTE  - " << dorItem.tktEffDate().dateToString(DDMMMYY, "")
       << "  TKT DISC DTE - " << dorItem.tktDiscDate().dateToString(DDMMMYY, "") << std::endl;
  diag << "    RES EFF DTE  - " << dorItem.resEffDate().dateToString(DDMMMYY, "")
       << "  RES DISC DTE - " << dorItem.resDiscDate().dateToString(DDMMMYY, "") << std::endl;
}

void
RuleItem::printFlexFaresRestriction(DiagCollector& diag,
                                    const uint16_t& catNumber,
                                    Record3ReturnTypes& validationResult) const
{
  if (!((PricingTrx::MIP_TRX == _trx->getTrxType()) && (true == _trx->isFlexFare()) && _chancelor))
    return;

  std::string str;
  Record3ReturnTypes result = validationResult;
  const RuleValidationContext context = _chancelor->getContext();

  switch (catNumber)
  {
  case ADVANCE_RESERVATION_RULE:
    str = "  NO ADVANCED PURCHASE: ";
    if (RuleValidationContext::FARE_MARKET == context._contextType)
    {
      result = context._paxTypeFare->getFlexFaresValidationStatus()
                   ->getStatusForAttribute<flexFares::NO_ADVANCE_PURCHASE>();
    }
    break;

  case MINIMUM_STAY_RULE:
  case MAXIMUM_STAY_RULE:
    str = "  NO MIN/MAX STAY: ";
    if (RuleValidationContext::FARE_MARKET == context._contextType)
    {
      result = context._paxTypeFare->getFlexFaresValidationStatus()
                   ->getStatusForAttribute<flexFares::NO_MIN_MAX_STAY>();
    }
    break;

  default:
    // do nothing
    return;
  }

  diag << "FLEX FARE VALIDATION: ";
  diag << std::endl;

  // In FVO phase group IDs aren't set to chancellor
  if (_phase != FarePhase)
  {
    diag << "  GROUP ID: ";
    diag << context._groupId;
    diag << std::endl;
  }

  diag << str;

  diag << ShoppingUtil::getFlexFaresValidationStatus(result);

  diag << std::endl;
}

} // tse
