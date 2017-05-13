//-------------------------------------------------------------------
//
//  File:        RexAdvResTktValidator.cpp
//  Created:     February 02, 2008
//  Authors:     Artur Krezel
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "RexPricing/RexAdvResTktValidator.h"

#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "RexPricing/GenericRexMapper.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <utility>


namespace tse
{
static Logger
logger("atseintl.RexPricing.RexAdvResTktValidator");
const std::string RexAdvResTktValidator::NOT_APPLY = "";
const std::vector<uint16_t>
RexAdvResTktValidator::PU_CATEGORIES(1, RuleConst::ADVANCE_RESERVATION_RULE);

RexAdvResTktValidator::RexAdvResTktValidator(
    RexPricingTrx& trx,
    Itin& newItin,
    FarePath& newFarePath,
    FarePath& excFarePath,
    RepriceSolutionValidator::AdvResOverrideCache& advResOverrideCache,
    Diag689Collector*& dc,
    const GenericRexMapper& grm)
  : _trx(trx),
    _dc(dc),
    _dc305(nullptr),
    _newItin(newItin),
    _newFarePath(newFarePath),
    _excFarePath(excFarePath),
    _allFCsFBAndCarrierUnchanged(false),
    _isTicketResvIndTheSameOnJourney(false),
    _isSimultaneousCheckNeeded(false),
    _isPriorOfDepartureCheckNeeded(false),
    _simultaneousCheckValid(NOT_PROCESSED),
    _priorOfDepartureCheckValid(NOT_PROCESSED),
    _advResOverrideCache(advResOverrideCache),
    _emptyBytes93to106(true),
    _mapper(grm),
    _ignoreTktAfterResRestBasedOnPermData(false),
    _ignoreTktBeforeDeptRestBasedOnPermData(false),
    _filterPermutationIndex(0)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic305)
  {
    DCFactory* factory = DCFactory::instance();
    _dc305 = factory->create(_trx);
    if (_dc305 != nullptr)
    {
      _dc305->enable(Diagnostic305);

      if (!_dc305->isActive())
      {
        _dc305 = nullptr;
      }
      else
      {
        std::string diagPermIndex = _trx.diagnostic().diagParamMapItem("ID");
        _filterPermutationIndex = std::atoi(diagPermIndex.c_str());
        if (_filterPermutationIndex < 0)
          _filterPermutationIndex = 0;
      }
    }
  }
}

RexAdvResTktValidator::~RexAdvResTktValidator()
{
  if (_dc305 != nullptr)
  {
    _dc305->flushMsg();
    _dc305 = nullptr;
  }
}

void
RexAdvResTktValidator::initialize()
{
  for (const ProcessTagInfo* pti : _permutation->processTags())
  {
    _allPTIs.push_back(pti);
    _emptyBytes93to106 &= emptyBytes93to106(*(pti->reissueSequence()));
  }

  _puCategories.push_back(RuleConst::ADVANCE_RESERVATION_RULE);

  _allFCsFBAndCarrierUnchanged = areAllFCsWithNoChangesToFBAndCarrier();

  setTicketResvIndTheSameOnJourney();
  setTicketResvInd(SIMULTANEOUS, _isSimultaneousCheckNeeded);
  setTicketResvInd(PRIOR_TO_DEPARTURE, _isPriorOfDepartureCheckNeeded);

  updateOldFM2OldFUMapping();

  if (isJourneyScope())
  {
    if (findMostRestrictiveFromDateForJourney())
      _fromDate = _trx.currentTicketingDT();
    else
      _fromDate = (_trx.applyReissueExchange() && !_trx.previousExchangeDT().isEmptyDate())
                      ? _trx.previousExchangeDT()
                      : _trx.originalTktIssueDT();
  }
}

void
RexAdvResTktValidator::updateOldFM2OldFUMapping()
{
  std::vector<PricingUnit*>::const_iterator puIter = _excFarePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd = _excFarePath.pricingUnit().end();
  std::vector<FareUsage*>::const_iterator fuIter;
  std::vector<FareUsage*>::const_iterator fuIterEnd;

  for (; puIter != puIterEnd; ++puIter)
  {
    fuIter = (*puIter)->fareUsage().begin();
    fuIterEnd = (*puIter)->fareUsage().end();
    for (; fuIter != fuIterEnd; ++fuIter)
    {
      _excFM2ExcPuAndFuMapping.insert(FM2PUFU::value_type((*fuIter)->paxTypeFare()->fareMarket(),
                                                          std::make_pair(*puIter, *fuIter)));
    }
  }
}

bool
RexAdvResTktValidator::isJourneyScope() const
{
  return (_isTicketResvIndTheSameOnJourney || !_allFCsFBAndCarrierUnchanged) && !_emptyBytes93to106;
}

bool
RexAdvResTktValidator::isPUWhollyUnchanged(const PricingUnit& pu)
{
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    if ((*fuIter)->paxTypeFare()->fareMarket()->isChanged())
      return false;
  }
  return true;
}

class FlagResetter
{
  bool& _flag;
  bool _origFlag;

public:
  FlagResetter(bool& flag) : _flag(flag), _origFlag(flag) {}
  ~FlagResetter() { _flag = _origFlag; }
};

bool
RexAdvResTktValidator::validate(const ProcessTagPermutation& permutation)
{
  _permutation = &permutation;
  diag305Header();

  if (_trx.getOptions()->AdvancePurchaseOption() == 'N')
    return advPurchaseValidation();

  initialize();

  std::vector<PricingUnit*>::const_iterator puIter = _newFarePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd = _newFarePath.pricingUnit().end();

  FlagResetter(_newFarePath.forceCat5Validation());
  _newFarePath.forceCat5Validation() = true;

  for (; puIter != puIterEnd; ++puIter)
  {
    if (!validate(**puIter))
      return false;
  }
  return true;
}

bool
RexAdvResTktValidator::validate(PricingUnit& pu)
{
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    FareUsage& fu = **fuIter;
    if (ignoreCat5(fu))
      continue;

    if (!validate(fu, pu))
      return false;
  }
  return true;
}

bool
RexAdvResTktValidator::ignoreCat5(const FareUsage& fu)
{
  if (fu.categoryIgnoredForKeepFare().find(5) != fu.categoryIgnoredForKeepFare().end())
  {
    if (_dc305 && !_trx.diagnostic().filterRulePhase(RuleItem::FarePathFactoryPhase) &&
        (!_filterPermutationIndex || _filterPermutationIndex == _permutation->number()))
    {
      const FareMarket& fareMarket = *fu.paxTypeFare()->fareMarket();
      *_dc305 << "FARE USAGE: " << fareMarket.origin()->loc() << "-"
              << fareMarket.destination()->loc() << "\n"
              << "ADV TKT/RES VALIDATION: IGNORED\n";
    }

    return true;
  }

  return false;
}

bool
RexAdvResTktValidator::validateAdvRes(AdvResOverride& advResOverride,
                                      FareUsage& fu,
                                      PricingUnit& pu)
{
  advResOverride.pricingUnit() = &pu;
  advResOverride.fareUsage() = &fu;
  bool isOptionN = (advResOverride.fromDate() != DateTime::emptyDate() &&
                    advResOverride.toDate() != DateTime::emptyDate());
  if (isOptionN || advResOverride.ignoreTktAfterResRestriction() == true ||
      advResOverride.ignoreTktDeforeDeptRestriction() == true ||
      advResOverride.reissueSequence() != nullptr)
    pu.volChangesAdvResOverride() = &advResOverride;
  else
    pu.volChangesAdvResOverride() = nullptr;

  if (!isOptionN)
  {
    RepriceSolutionValidator::AdvResOverrideCache::const_iterator cacheIter =
        _advResOverrideCache.find(advResOverride);
    if (cacheIter != _advResOverrideCache.end())
    {
      diag305CacheResult(fu, pu, advResOverride, cacheIter->second);
      pu.volChangesAdvResOverride() = nullptr;

      return cacheIter->second;
    }
  }

  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(DynamicValidation,
                                                                        _puCategories);
  bool result = ruleController.validate(_trx, _newFarePath, pu, fu);

  if (!isOptionN)
    _advResOverrideCache.insert(
        RepriceSolutionValidator::AdvResOverrideCache::value_type(advResOverride, result));
  pu.volChangesAdvResOverride() = nullptr;

  return result;
}

void
RexAdvResTktValidator::diag305Header()
{
  if (_dc305 && !_trx.diagnostic().filterRulePhase(RuleItem::FarePathFactoryPhase) &&
      (!_filterPermutationIndex || _filterPermutationIndex == _permutation->number()))
  {
    *_dc305 << "PERMUTATION " << _permutation->number() << "\n";
    *_dc305 << _newFarePath;
    _dc305->lineSkip(0);
    _dc305->flushMsg();
  }
}

void
RexAdvResTktValidator::diag305CacheResult(const FareUsage& fu,
                                          const PricingUnit& pu,
                                          const AdvResOverride& advResOverride,
                                          const bool result)
{
  if (_dc305)
  {
    *_dc305 << "RESULT REUSED  FARE USAGE: " << fu.paxTypeFare()->fareMarket()->origin()->loc()
            << "-" << fu.paxTypeFare()->fareMarket()->destination()->loc() << "  "
            << "PU: " << pu.travelSeg().front()->origAirport();
    if (pu.turnAroundPoint())
    {
      *_dc305 << "-" << pu.turnAroundPoint()->origAirport();
    }
    *_dc305 << "-" << pu.travelSeg().back()->destAirport() << "\n";

    if (advResOverride.reissueSequence())
    {
      *_dc305 << " REC T988: " << advResOverride.reissueSequence()->overridingWhenExists()->itemNo()
              << "\n";
    }
    *_dc305 << " ADVANCE RESTKT: " << (result ? "PASS" : "FAIL") << "\n";
    _dc305->flushMsg();
  }
}

bool
RexAdvResTktValidator::checkAfterResAndBeforeDept(const ReissueSequenceW& rs,
                                                  AdvResOverride& advResOverride,
                                                  FareUsage& fu,
                                                  PricingUnit& pu,
                                                  bool& isProcessed) // bytes 93-105
{
  if (!rs.orig())
    return true;

  bool isAfterResSpecified = false;
  bool isBeforeDeptSpecified = false;

  const Indicator& ticketResvInd = rs.ticketResvInd();

  isAfterResSpecified = (rs.reissuePeriod() != NOT_APPLY && rs.reissueUnit() != NOT_APPLY);
  isBeforeDeptSpecified = (rs.departure() != VAL_NOT_APPLY && rs.departureUnit() != IND_NOT_APPLY);

  if ((ticketResvInd != SIMULTANEOUS && isAfterResSpecified) || isBeforeDeptSpecified)
  {
    if (ticketResvInd == SIMULTANEOUS)
    {
      if (!checkSimultaneous())
        return false;
      advResOverride.ignoreTktAfterResRestriction() = true;
    }
    else if (isAfterResSpecified)
    {
      advResOverride.reissueSequence() = const_cast<ReissueSequenceW*>(&rs);
    }
    if (!isBeforeDeptSpecified)
    {
      if (ticketResvInd == PRIOR_TO_DEPARTURE)
      {
        if (!checkPriorOfDeparture())
          return false;
        advResOverride.ignoreTktDeforeDeptRestriction() = true;
      }
      else if (_trx.getOptions()->AdvancePurchaseOption() == 'N')
      {
        prepareCat31PCGOptionN(advResOverride, fu, pu);
      }
    }
    else
    {
      advResOverride.reissueSequence() = const_cast<ReissueSequenceW*>(&rs);
    }

    if (!validateAdvRes(advResOverride, fu, pu))
    {
      if (_dc && _dc->filterPassed())
      {
        *_dc << "ADV TKT/RES VALIDATION: FAILED\n"
             << "  FAILED OVERRIDED CAT 5 VALIDATION\n";
      }
      return false;
    }
    isProcessed = true;
  }
  return true;
}

bool
RexAdvResTktValidator::validate(FareUsage& fu, PricingUnit& pu)
{
  AdvResOverride advResOverride;
  bool isPUUnchanged = isPUWhollyUnchanged(pu);

  if (!isPUUnchanged)
  {
    const std::vector<const ProcessTagInfo*>& mappedFBAndCarrier =
        findFCsWithNoChangesToFBAndCarrier(fu.paxTypeFare()->fareMarket());
    std::vector<const ProcessTagInfo*>::const_iterator ptiIter = mappedFBAndCarrier.begin();
    const std::vector<const ProcessTagInfo*>::const_iterator ptiIterEnd = mappedFBAndCarrier.end();

    bool isProcessed = false;

    for (; ptiIter != ptiIterEnd; ++ptiIter)
    {
      if (!checkAfterResAndBeforeDept(
              *(*ptiIter)->reissueSequence(), advResOverride, fu, pu, isProcessed))
        return false;
    }
    if (isProcessed)
      return true;

    if (_isSimultaneousCheckNeeded)
    {
      if (!checkSimultaneous())
        return false;
      advResOverride.ignoreTktAfterResRestriction() = true;
    }
    if (_isPriorOfDepartureCheckNeeded)
    {
      if (!checkPriorOfDeparture())
        return false;
      advResOverride.ignoreTktDeforeDeptRestriction() = true;
    }
    else if (_trx.getOptions()->AdvancePurchaseOption() == 'N')
    {
      prepareCat31PCGOptionN(advResOverride, fu, pu);
    }
  }
  else if (_trx.getOptions()->AdvancePurchaseOption() == 'N')
  {
    prepareCat31PCGOptionN(advResOverride, fu, pu);
  }

  if (isPUUnchanged || !_isSimultaneousCheckNeeded || !_isPriorOfDepartureCheckNeeded)
  {
    if (!validateAdvRes(advResOverride, fu, pu))
    {
      if (_dc && _dc->filterPassed())
      {
        *_dc << "ADV TKT/RES VALIDATION: FAILED\n"
             << "  FAILED CAT 5 VALIDATION - " << fu.paxTypeFare()->fareClass() << "\n";
      }

      return false;
    }
  }

  return true;
}

struct RexAdvResTktValidator::SameFareBreaksAndCarrier
{
  SameFareBreaksAndCarrier(const FareMarket* fm) : _repriceFM(fm) {}

  bool operator()(const ProcessTagInfo* pti) const
  {
    const FareMarket* fm = pti->fareMarket();
    return fm->boardMultiCity() == _repriceFM->boardMultiCity() &&
           fm->offMultiCity() == _repriceFM->offMultiCity() &&
           fm->governingCarrier() == _repriceFM->governingCarrier();
  }
  const FareMarket* _repriceFM;
};

const std::vector<const ProcessTagInfo*>&
RexAdvResTktValidator::findFCsWithNoChangesToFBAndCarrier(const FareMarket* fm)
{
  static std::vector<const ProcessTagInfo*> emptyPTIs;

  FareMarket2ProcessTagInfos::const_iterator fbcCache =
      _unchangedFareBreaksAndCarrierCache.find(fm);
  if (fbcCache != _unchangedFareBreaksAndCarrierCache.end())
  {
    return fbcCache->second;
  }
  else
  {
    std::vector<const ProcessTagInfo*> newPTIs;
    std::vector<const ProcessTagInfo*>::const_iterator ptiIter = _allPTIs.begin();
    const std::vector<const ProcessTagInfo*>::const_iterator ptiIterEnd = _allPTIs.end();
    while ((ptiIter = std::find_if(ptiIter, ptiIterEnd, SameFareBreaksAndCarrier(fm))) !=
           ptiIterEnd)
      newPTIs.push_back(*ptiIter++);
    if (newPTIs.size() > 0)
      return _unchangedFareBreaksAndCarrierCache.insert(FareMarket2ProcessTagInfos::value_type(
                                                            fm, newPTIs)).first->second;
  }

  return emptyPTIs;
}

bool
RexAdvResTktValidator::areAllFCsWithNoChangesToFBAndCarrier()
{
  std::vector<PricingUnit*>::const_iterator puIter = _newFarePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIterEnd = _newFarePath.pricingUnit().end();

  std::vector<FareUsage*>::const_iterator fuIter;
  std::vector<FareUsage*>::const_iterator fuIterEnd;

  for (; puIter != puIterEnd; ++puIter)
  {
    fuIter = (*puIter)->fareUsage().begin();
    fuIterEnd = (*puIter)->fareUsage().end();

    for (; fuIter != fuIterEnd; ++fuIter)
    {
      const std::vector<const ProcessTagInfo*>& mappedFBAndCarrier =
          findFCsWithNoChangesToFBAndCarrier((*fuIter)->paxTypeFare()->fareMarket());
      if (mappedFBAndCarrier.size() == 0)
        return false;
    }
  }
  return true;
}

struct RexAdvResTktValidator::CompareTicketResvInd
    : public std::unary_function<ProcessTagInfo*, bool>
{
  CompareTicketResvInd(const Indicator& ticketResvInd) : _ticketResvInd(ticketResvInd) {}

  bool operator()(ProcessTagInfo* pti) const
  {
    return pti->reissueSequence()->orig()
               ? (pti->reissueSequence()->ticketResvInd() == _ticketResvInd)
               : false;
  }

private:
  const Indicator& _ticketResvInd;
};

void
RexAdvResTktValidator::setTicketResvIndTheSameOnJourney()
{
  const ProcessTagInfo* firstPTIWithT988 = _permutation->firstWithT988();
  if (!firstPTIWithT988)
    return;

  std::vector<ProcessTagInfo*>::const_iterator ptiIter = _permutation->processTags().begin();
  const std::vector<ProcessTagInfo*>::const_iterator ptiIterEnd = _permutation->processTags().end();

  const Indicator& ticketResvInd = firstPTIWithT988->reissueSequence()->ticketResvInd();
  ++ptiIter;
  ptiIter = find_if(ptiIter, ptiIterEnd, std::not1(CompareTicketResvInd(ticketResvInd)));
  if (ptiIter == ptiIterEnd)
    _isTicketResvIndTheSameOnJourney = true;
}

void
RexAdvResTktValidator::setTicketResvInd(const Indicator ticketResvInd, bool& ticketResvIndNeeded)
{
  const ProcessTagInfo* firstPTIWithT988 = _permutation->firstWithT988();
  if (!firstPTIWithT988)
    return;

  if (_isTicketResvIndTheSameOnJourney == true &&
      firstPTIWithT988->reissueSequence()->ticketResvInd() == ticketResvInd)
  {
    ticketResvIndNeeded = true;
    return;
  }
  const std::vector<ProcessTagInfo*>::const_iterator ptiIterEnd = _permutation->processTags().end();

  std::vector<ProcessTagInfo*>::const_iterator pos =
      find_if(_permutation->processTags().begin(), ptiIterEnd, CompareTicketResvInd(ticketResvInd));
  if (pos != ptiIterEnd)
    ticketResvIndNeeded = true;
}

bool
RexAdvResTktValidator::checkSimultaneous()
{
  if (_simultaneousCheckValid == NOT_PROCESSED)
  {
    DateTime latestTktTime;

    const OnlyChangedSegments& changedSegVec =
        for_each(_newItin.travelSeg().begin(), _newItin.travelSeg().end(), OnlyChangedSegments());
    OnlyChangedSegments::const_iterator latestSegIter =
        max_element(changedSegVec.begin(), changedSegVec.end(), CompareBookingDate());
    if (latestSegIter == changedSegVec.end())
    {
      _simultaneousCheckValid = NOT_VALID;
      return false;
    }
    const DateTime& latestBookingDateForChangedSegments = (*latestSegIter)->bookingDT();

    if (_newItin.geoTravelType() == GeoTravelType::Domestic || _newItin.geoTravelType() == GeoTravelType::Transborder ||
        (_newItin.geoTravelType() == GeoTravelType::ForeignDomestic &&
         _newItin.travelSeg().front()->origin()->nation() == CANADA))
    {
      latestTktTime = latestBookingDateForChangedSegments.addSeconds(1800); // 30 minutes
    }
    else
    {
      latestTktTime = DateTime(latestBookingDateForChangedSegments.date(), 23, 59, 0);
    }
    _simultaneousCheckValid = (_trx.currentTicketingDT() <= latestTktTime) ? VALID : NOT_VALID;

    if (_dc && _dc->filterPassed() && _simultaneousCheckValid == NOT_VALID)
    {
      *_dc << "ADV TKT/RES VALIDATION: FAILED\n";
      *_dc << "  FAILED AFTER RES - SIMULTANEOUS VALIDATION\n"
           << "  REISSUE DATE: " << _trx.currentTicketingDT().toIsoExtendedString() << "\n"
           << "  LATEST BOOKING DATE: " << latestBookingDateForChangedSegments.toIsoExtendedString()
           << "\n";
      *_dc << "  LATEST TKT DATE: " << latestTktTime.toIsoExtendedString() << "\n";
      *_dc << "  REISSUE DATE IS NOT ON OR BEFORE LATEST TKT DATE\n";
    }
  }
  return _simultaneousCheckValid == VALID ? true : false;
}

bool
RexAdvResTktValidator::checkPriorOfDeparture()
{
  if (_priorOfDepartureCheckValid == NOT_PROCESSED)
  {
    const OnlyChangedSegments& changedSegVec =
        for_each(_newItin.travelSeg().begin(), _newItin.travelSeg().end(), OnlyChangedSegments());
    OnlyChangedSegments::const_iterator earliestSegIter =
        min_element(changedSegVec.begin(), changedSegVec.end(), CompareDepartureDate());
    if (earliestSegIter == changedSegVec.end())
    {
      _priorOfDepartureCheckValid = NOT_VALID;
      return false;
    }
    const DateTime& earliestDepartureDateForChangedSegments = (*earliestSegIter)->departureDT();

    _priorOfDepartureCheckValid =
        (_trx.currentTicketingDT() < earliestDepartureDateForChangedSegments) ? VALID : NOT_VALID;

    if (_dc && _dc->filterPassed() && _priorOfDepartureCheckValid == NOT_VALID)
    {
      *_dc << "ADV TKT/RES VALIDATION: FAILED\n";
      *_dc << "  FAILED BEFORE DEPT - PRIOR DEPARTURE VALIDATION\n"
           << "  REISSUE DATE: " << _trx.currentTicketingDT().toIsoExtendedString() << "\n"
           << "  EARLIEST DEPT DATE: "
           << earliestDepartureDateForChangedSegments.toIsoExtendedString() << "\n";
      *_dc << "  REISSUE DATE IS NOT BEFORE REISSUE DATE\n";
    }
  }
  return _priorOfDepartureCheckValid == VALID ? true : false;
}

void
RexAdvResTktValidator::prepareCat31PCGOptionN(AdvResOverride& advResOverride,
                                              FareUsage& fu,
                                              PricingUnit& pu)
{
  if (!isJourneyScope())
  {
    if (findMostRestrictiveFromDateForPU(pu))
      advResOverride.fromDate() = _trx.currentTicketingDT();
    else
      advResOverride.fromDate() =
          (_trx.applyReissueExchange() && !_trx.previousExchangeDT().isEmptyDate())
              ? _trx.previousExchangeDT()
              : _trx.originalTktIssueDT();
    advResOverride.toDate() = findMostRestrictiveToDateForPU(pu, fu);
  }
  else
  {
    advResOverride.fromDate() = _fromDate;
    advResOverride.toDate() = findMostRestrictiveToDateForJourney(pu, fu);
  }
}

bool
RexAdvResTktValidator::isOutboundChanged(const PricingUnit& pu) const
{
  const std::vector<TravelSeg*> tsVec =
      pu.fareUsage().front()->paxTypeFare()->fareMarket()->travelSeg();
  return std::any_of(tsVec.cbegin(),
                     tsVec.cend(),
                     [](const TravelSeg* ts)
                     {
    return (ts->changeStatus() == TravelSeg::CHANGED ||
            ts->changeStatus() == TravelSeg::INVENTORYCHANGED);
  });
}

bool
RexAdvResTktValidator::isMostRestrictiveFromDate(const ProcessTagInfo& pti,
                                                 const PricingUnit& pu,
                                                 const FareUsage& fu,
                                                 const FareUsage* newFu) const
{
  switch (pti.reissueSequence()->fromAdvResInd())
  {
  case FROM_ADVRES_OUTBOUND:
  {
    if (isOutboundChanged(pu))
      return isUnflown(pti.reissueSequence()->toAdvResInd(), pu, fu);
  }
  break;
  case FROM_ADVRES_TYPEOFFARE:
  {
    if (newFu ? newFu->paxTypeFare()->retrievalFlag() == FareMarket::RetrievCurrent
              : _permutation->fareTypeSelection(pti.fareCompInfo()->fareMarket()->changeStatus()) ==
                    CURRENT)
    {
      return isUnflown(pti.reissueSequence()->toAdvResInd(), pu, newFu ? *newFu : fu);
    }
  }
  break;
  case FROM_ADVRES_NEWTKTDATE:
  {
    return isUnflown(pti.reissueSequence()->toAdvResInd(), pu, fu);
  }
  break;
  }
  return false;
}

bool
RexAdvResTktValidator::findMostRestrictiveFromDateForJourney() const
{
  for (const ProcessTagInfo* pti : _permutation->processTags())
  {
    if (!pti->reissueSequence()->orig())
      continue;

    FM2PUFU::const_iterator mapIter =
        _excFM2ExcPuAndFuMapping.find(pti->paxTypeFare()->fareMarket());
    if (mapIter != _excFM2ExcPuAndFuMapping.end() &&
        isMostRestrictiveFromDate(*pti, *mapIter->second.first, *mapIter->second.second))
      return true;
  }
  return false;
}

bool
RexAdvResTktValidator::findMostRestrictiveFromDateForPU(const PricingUnit& pu)
{
  if (!pu.travelSeg().front()->unflown())
  {
    return false;
  }

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const FareUsage& fu = **fuIter;
    const std::vector<const ProcessTagInfo*>& mappedFBAndCarrier =
        findFCsWithNoChangesToFBAndCarrier(fu.paxTypeFare()->fareMarket());
    if (mappedFBAndCarrier.size() == 0)
      return false;
    std::vector<const ProcessTagInfo*>::const_iterator ptiIter = mappedFBAndCarrier.begin();
    const std::vector<const ProcessTagInfo*>::const_iterator ptiIterEnd = mappedFBAndCarrier.end();
    for (; ptiIter != ptiIterEnd; ++ptiIter)
    {
      const ProcessTagInfo& pti = **ptiIter;

      if (!pti.reissueSequence()->orig())
        continue;

      FM2PUFU::const_iterator mapIter =
          _excFM2ExcPuAndFuMapping.find(pti.paxTypeFare()->fareMarket());
      if (mapIter != _excFM2ExcPuAndFuMapping.end())
      {
        if (isMostRestrictiveFromDate(pti, *mapIter->second.first, *mapIter->second.second, &fu))
          return true;
      }
    }
  }
  return false;
}

const DateTime&
RexAdvResTktValidator::findMostRestrictiveToDateForJourney(const PricingUnit& pu,
                                                           const FareUsage& fu)
{
  bool usePU = false;
  std::vector<ProcessTagInfo*>::const_iterator ptiIter = _permutation->processTags().begin();
  const std::vector<ProcessTagInfo*>::const_iterator ptiIterEnd = _permutation->processTags().end();

  for (; ptiIter != ptiIterEnd; ++ptiIter)
  {
    if (!(**ptiIter).reissueSequence()->orig())
      continue;

    Indicator toAdvResInd = (*ptiIter)->reissueSequence()->toAdvResInd();

    if (toAdvResInd == TO_ADVRES_JOURNEY)
    {
      return _trx.itin().front()->travelSeg().front()->departureDT();
    }
    else if (toAdvResInd == TO_ADVRES_PRICINGUNIT)
      usePU = true;
  }
  if (usePU)
    return pu.travelSeg().front()->departureDT();
  return fu.travelSeg().front()->departureDT();
}

namespace
{
// this blocks bug that appear after Cat31AdvResDatesFix
bool
notAllFromOValues(const ProcessTagPermutation& permutation)
{
  std::vector<ProcessTagInfo*>::const_iterator ptiIter = permutation.processTags().begin();
  const std::vector<ProcessTagInfo*>::const_iterator ptiIterEnd = permutation.processTags().end();

  for (; ptiIter != ptiIterEnd; ++ptiIter)
  {
    const ProcessTagInfo& pti = **ptiIter;

    if (!pti.reissueSequence()->orig() || pti.reissueSequence()->fromAdvResInd() != 'O')
      return true;
  }

  return false;
}

class ByteValue : public std::unary_function<uint16_t, bool>
{
  Indicator _value;
  const std::vector<ProcessTagInfo*>& _ptis;

public:
  ByteValue(Indicator value, const std::vector<ProcessTagInfo*>& ptis) : _value(value), _ptis(ptis)
  {
  }

  bool operator()(uint16_t fcNo) const
  {
    return fcNo && _ptis[fcNo - 1]->reissueSequence()->orig() &&
           _ptis[fcNo - 1]->reissueSequence()->ticketResvInd() == _value;
  }
};
}

const DateTime&
RexAdvResTktValidator::findMostRestrictiveToDateForPU(const PricingUnit& pu, const FareUsage& fu)
{
  bool usePU = false;
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const std::vector<const ProcessTagInfo*>& mappedFBAndCarrier =
        findFCsWithNoChangesToFBAndCarrier((*fuIter)->paxTypeFare()->fareMarket());
    if (mappedFBAndCarrier.size() == 0)
    {
      if (notAllFromOValues(*_permutation))
        return DateTime::emptyDate();
      else
        return findMostRestrictiveToDateForJourney(pu, fu);
    }
    std::vector<const ProcessTagInfo*>::const_iterator ptiIter = mappedFBAndCarrier.begin();
    const std::vector<const ProcessTagInfo*>::const_iterator ptiIterEnd = mappedFBAndCarrier.end();
    for (; ptiIter != ptiIterEnd; ++ptiIter)
    {
      if (!(**ptiIter).reissueSequence()->orig())
        continue;

      Indicator toAdvResInd = (*ptiIter)->reissueSequence()->toAdvResInd();
      if (toAdvResInd == TO_ADVRES_JOURNEY)
      {
        return _trx.itin().front()->travelSeg().front()->departureDT();
      }
      else if (toAdvResInd == TO_ADVRES_PRICINGUNIT)
        usePU = true;
    }
  }
  if (usePU)
    return pu.travelSeg().front()->departureDT();
  return fu.travelSeg().front()->departureDT();
}

bool
RexAdvResTktValidator::isUnflown(const Indicator portion,
                                 const PricingUnit& pu,
                                 const FareUsage& fu) const
{
  switch (portion)
  {
  case TO_ADVRES_JOURNEY:
  {
    return _trx.itin().front()->travelSeg().front()->unflown();
  }
  case TO_ADVRES_PRICINGUNIT:
  {
    return pu.travelSeg().front()->unflown();
  }
  default: // TO_ADVRES_FARECOMPONENT:
  {
    return fu.travelSeg().front()->unflown();
  }
  }
}

bool
RexAdvResTktValidator::emptyBytes93to106(const ReissueSequenceW& rsw)
{
  return !rsw.orig() || (rsw.reissuePeriod().empty() && rsw.reissueUnit().empty() &&
                         rsw.departure() == 0 && rsw.departureUnit() == IND_NOT_APPLY);
}

// *** 2012 new advance purchase application ***

void
RexAdvResTktValidator::permutationIndependentSetUp()
{
  determinePermIndepFromDate();
  permIndepFromDateForNotMapped();
}

void
RexAdvResTktValidator::determinePermIndepFromDate()
{
  _permIndepFromDateAndLoc = _trx.getLocAndDateForAdjustment();
}

void
RexAdvResTktValidator::permIndepFromDateForNotMapped()
{
  for (PricingUnit* pu : _newFarePath.pricingUnit())
  {
    bool current = false;
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->retrievalFlag() == FareMarket::RetrievCurrent)
      {
        current = true;
        break;
      }
    }
    _fromDateForNotMapped.insert(std::make_pair(
        pu, (current ? &_trx.currentTicketingDT() : _permIndepFromDateAndLoc.first)));
  }
}

bool
RexAdvResTktValidator::advPurchaseValidation()
{
  FlagResetter(_newFarePath.forceCat5Validation());
  _newFarePath.forceCat5Validation() = true;

  if (_dc && _dc->filterPassed())
  {
    *_dc << "VALIDATING ADVANCE PURCHASE\n";
    if (_dc->printCat5Info())
      *_dc << _mapper.domesticDiag().str();
  }

  setIgnorePermutationBased();

  // Advanced Ticketing bytes handled only for mapped FCs
  // seldom used, don't really matter
  if (!checkAdvancedTicketing())
    return false;

  std::vector<uint16_t>::size_type ptfIdx = 0;

  for (PricingUnit* pu : _newFarePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      if (diagCat5Info())
      {
        *_dc << "NEW";
        _dc->printNarrowPtf(*fu->paxTypeFare());
      }

      if (ignoreCat5(*fu))
      {
        if (diagCat5Info())
          *_dc << "ADV TKT/RES VALIDATION: IGNORED\n";

        continue;
      }

      if (!validateAdvPurchase(*fu, *pu, ptfIdx++))
        return false;
    }
  }
  return true;
}

bool
RexAdvResTktValidator::validateAdvPurchase(FareUsage& fu,
                                           PricingUnit& pu,
                                           std::vector<uint16_t>::size_type ptfIdx)
{
  uint16_t fcNumber = _mapper.getQuickAccessMap()[ptfIdx];
  AdvResOverride advResOverride(pu);

  // below line is constant in solution scope!
  if (fcNumber)
  {
    const ProcessTagInfo& pti = *_permutation->processTags()[fcNumber - 1];

    if (fcNumber != pti.fareCompNumber())
      LOG4CXX_ERROR(logger,
                    "CAT 31 ADV PUR MAPPING ERROR !!! : " << fcNumber << " TO"
                                                          << pti.fareCompNumber());

    if (!pti.reissueSequence()->orig())
    {
      if (diagCat5Info())
        *_dc << " NO T988 - TREAT AS NOT MATCHED - DATES OVERRIDE:\n";

      determineUnmappedOverrideData(advResOverride, pu, fu);
    }
    else
    {
      if (diagCat5Info())
        *_dc << " DATA OVERRIDE:\n";

      // T988 present process
      advResOverride.reissueSequence() = const_cast<ReissueSequenceW*>(pti.reissueSequence());
      determineFromDate(advResOverride, pti, pu, fu);
      determineToDate(advResOverride, pti, pu, fu);
      determineIgnoreTktAfterRes(advResOverride, pti, pu, fu);
      determineIgnoreTktBeforeDeparture(advResOverride, pti, pu, fu);
    }
  }
  else
  {
    if (diagCat5Info())
      *_dc << "NOT MATCHED - DATES OVERRIDE:\n";

    determineUnmappedOverrideData(advResOverride, pu, fu);
  }

  if (diagCat5Info())
    _dc->printAdvResOverrideData(advResOverride);

  if (_filterPermutationIndex && _filterPermutationIndex != _permutation->number())
    advResOverride.setDiag305OFF();

  //    pu.volChangesAdvResOverride() = &advResOverride;
  return performRuleControllerValidation(fu, pu);
}

bool
RexAdvResTktValidator::performRuleControllerValidation(FareUsage& fu, PricingUnit& pu)
{
  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(DynamicValidation,
                                                                        PU_CATEGORIES);
  bool result = ruleController.validate(_trx, _newFarePath, pu, fu);
  if (!result && _dc && _dc->filterPassed())
  {
    *_dc << "  FAILED ADV TKT/RES VALIDATION - " << fu.paxTypeFare()->fareClass() << "\n";
  }
  return result;
}

static DateTime
adjustDateToCommonLoc(const PricingTrx& trx, const DateTime& date, const Loc& loc)
{
  short utcOffset = 0;
  DateTime localTime = DateTime::localTime();
  LocUtil::getUtcOffsetDifference(loc,
                                  *AdvanceResTkt::getCommonReferenceLoc(trx),
                                  utcOffset,
                                  trx.dataHandle(),
                                  localTime,
                                  localTime);
  return date.subtractSeconds(utcOffset * 60);
}

void
RexAdvResTktValidator::determineFromDate(AdvResOverride& advResOverride,
                                         const ProcessTagInfo& pti,
                                         const PricingUnit& pu,
                                         const FareUsage& fu) const
{
  std::pair<const DateTime*, const Loc*> fromDateAndLoc;

  switch (pti.reissueSequence()->fromAdvResInd())
  {
  case FROM_ADVRES_OUTBOUND: // P
  {
    if (isOutboundChanged(pu))
    {
      fromDateAndLoc = std::make_pair(&_trx.currentTicketingDT(), _trx.currentSaleLoc());
    }
    else
    {
      advResOverride.ignoreTktAfterResRestriction() = true;
      fromDateAndLoc = _permIndepFromDateAndLoc;
    }
  }
  break;

  case FROM_ADVRES_TYPEOFFARE: // C
  {
    if (fu.paxTypeFare()->retrievalFlag() == FareMarket::RetrievCurrent)
    {
      fromDateAndLoc = std::make_pair(&_trx.currentTicketingDT(), _trx.currentSaleLoc());
    }
    else
    {
      advResOverride.ignoreTktAfterResRestriction() = true;
      fromDateAndLoc = _permIndepFromDateAndLoc;
    }
  }
  break;

  case FROM_ADVRES_NEWTKTDATE: //' '
  {
    fromDateAndLoc = std::make_pair(&_trx.currentTicketingDT(), _trx.currentSaleLoc());
  }
  break;

  case FROM_ADVRES_ORIGIN: // 0
  {
    fromDateAndLoc = _permIndepFromDateAndLoc;
  }
  break;

  default:
  {
    LOG4CXX_ERROR(logger,
                  "T988:" << pti.reissueSequence()->itemNo()
                          << " DATA ERROR - WRONG ADV RES IND VALUE");
  }
  }

  if (fromDateAndLoc.second)
    advResOverride.fromDate() =
        adjustDateToCommonLoc(_trx, *fromDateAndLoc.first, *fromDateAndLoc.second);
  else
    advResOverride.fromDate() = *fromDateAndLoc.first;
}

static std::pair<DateTime, const Loc*>
getDepartureDateAndLoc(const PricingTrx& trx, const std::vector<TravelSeg*>& tvlSegs)
{
  DateTime date = tvlSegs.front()->departureDT();
  const Loc* loc = trx.dataHandle().getLoc(tvlSegs.front()->origAirport(), DateTime::localTime());
  return std::make_pair(date, loc);
}

void
RexAdvResTktValidator::determineToDate(AdvResOverride& advResOverride,
                                       const ProcessTagInfo& pti,
                                       const PricingUnit& pu,
                                       const FareUsage& fu) const
{
  std::pair<DateTime, const Loc*> toDateAndLoc;

  switch (pti.reissueSequence()->toAdvResInd())
  {
  case TO_ADVRES_JOURNEY: // J
  {
    toDateAndLoc = getDepartureDateAndLoc(_trx, _trx.itin().front()->travelSeg());
  }
  break;

  case TO_ADVRES_PRICINGUNIT: // ' '
  {
    toDateAndLoc = getDepartureDateAndLoc(_trx, pu.travelSeg());
  }
  break;

  case TO_ADVRES_FARECOMPONENT: // F
  {
    toDateAndLoc = getDepartureDateAndLoc(_trx, fu.travelSeg());
  }
  break;

  default:
  {
    LOG4CXX_ERROR(logger,
                  "T988:" << pti.reissueSequence()->itemNo()
                          << " DATA ERROR - WRONG TO DATE IND VALUE");
  }
  }

  if (toDateAndLoc.second)
    advResOverride.toDate() = adjustDateToCommonLoc(_trx, toDateAndLoc.first, *toDateAndLoc.second);
  else
    advResOverride.toDate() = toDateAndLoc.first;
}

void
RexAdvResTktValidator::determineUnmappedOverrideData(AdvResOverride& advResOverride,
                                                     const PricingUnit& pu,
                                                     const FareUsage& fu) const
{
  advResOverride.toDate() = pu.travelSeg().front()->departureDT();
  advResOverride.fromDate() = *_fromDateForNotMapped.at(&pu);
}

template <typename Criterium>
bool
RexAdvResTktValidator::findFromMappedFCs(Criterium criterium)
{
  return std::find_if(_mapper.getQuickAccessMap().begin(),
                      _mapper.getQuickAccessMap().end(),
                      criterium) != _mapper.getQuickAccessMap().end();
}

void
RexAdvResTktValidator::setIgnorePermutationBased()
{
  if ((_ignoreTktAfterResRestBasedOnPermData =
           findFromMappedFCs(ByteValue(SIMULTANEOUS, _permutation->processTags()))) &&
      diagCat5Info())
    *_dc << " ONE OF EXC FC HAS SIMULTANEOUS FILLED\n";

  if ((_ignoreTktBeforeDeptRestBasedOnPermData =
           findFromMappedFCs(ByteValue(PRIOR_TO_DEPARTURE, _permutation->processTags()))) &&
      diagCat5Info())
    *_dc << " ONE OF EXC FC HAS PRIOR TO DEPARTURE FILLED\n";
}

void
RexAdvResTktValidator::determineIgnoreTktAfterRes(AdvResOverride& advResOverride,
                                                  const ProcessTagInfo& pti,
                                                  const PricingUnit& pu,
                                                  const FareUsage& fu)
{
  if (!advResOverride.ignoreTktAfterResRestriction() && _ignoreTktAfterResRestBasedOnPermData)
  {
    advResOverride.ignoreTktAfterResRestriction() = true;
  }
}

void
RexAdvResTktValidator::determineIgnoreTktBeforeDeparture(AdvResOverride& advResOverride,
                                                         const ProcessTagInfo& pti,
                                                         const PricingUnit& pu,
                                                         const FareUsage& fu)
{
  if (_ignoreTktBeforeDeptRestBasedOnPermData)
    advResOverride.ignoreTktDeforeDeptRestriction() = true;
}

bool
RexAdvResTktValidator::checkAdvancedTicketing()
{
  if (_ignoreTktAfterResRestBasedOnPermData)
  {
    if (!checkSimultaneous())
    {
      if (diagCat5Info())
        *_dc << "  SIMULTANEOUS FAILED\n";

      return false;
    }
  }

  if (_ignoreTktBeforeDeptRestBasedOnPermData)
  {
    if (!checkPriorOfDeparture())
    {
      if (diagCat5Info())
        *_dc << "  PRIOR OF DEPARTURE FAILED\n";

      return false;
    }
  }

  return true;
}

} // namespace tse
