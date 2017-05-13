/*---------------------------------------------------------------------------
 *  File:    ShoppingPQ.cpp
 *  Created: Jan 20, 2005
 *  Authors: David White
 *
 *  Copyright Sabre 2005
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Pricing/ShoppingPQ.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/MultiDimensionalPQ.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Swapper.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/BitmapOpOrderer.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Pricing/FareUsageMatrixMap.h"
#include "Pricing/FltOnlyCombinationIterator.h"
#include "Pricing/MultiAirportAgent.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/Shopping/IBF/IbfData.h"
#include "Pricing/Shopping/IBF/V2IbfManager.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/ShoppingFarePathFactory.h"
#include "Pricing/ShoppingPaxFarePathFactory.h"
#include "Rules/AccompaniedTravel.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <deque>
#include <numeric>

namespace tse
{
static Logger
logger("atseintl.PricingOrchestrator.ShoppingPQ");

FALLBACK_DECL(fallbackFareSelction2016);
FALLBACK_DECL(reworkTrxAborter);

namespace
{
ConfigurableValue<uint32_t>
fltCombMaxOnline("SHOPPING_OPT", "MAX_ONLINE_FLTS_COMBOS");
ConfigurableValue<uint64_t>
minimumFamilySizeOnline("PRICING_SVC", "MIN_FAMILY_SIZE");
ConfigurableValue<uint32_t>
fltCombMaxInterline("SHOPPING_OPT", "MAX_INTERLINE_FLTS_COMBOS");
ConfigurableValue<uint64_t>
minimumFamilySizeInterline("PRICING_SVC", "MIN_FAMILY_SIZE_INTERLINE");
ConfigurableValue<uint32_t>
interlineDiversityPercent("SHOPPING_OPT", "INTERLINE_DIVERSITY_PERCENT");
ConfigurableValue<uint32_t>
mustPriceInterlineSnowManPercent("SHOPPING_OPT", "MUST_PRICE_INTERLINE_SNOWMAN_PERCENT");
ConfigurableValue<uint32_t>
interlineDiversityPercentBFM("SHOPPING_OPT", "INTERLINE_DIVERSITY_PERCENT_BFM");
ConfigurableValue<uint32_t>
farePathForRuleValMax("SHOPPING_OPT", "MAX_FAREPATH_FOR_RULE_VALIDATION");
ConfigurableValue<uint32_t>
farePathForRuleValWithFltMax("SHOPPING_OPT", "MAX_FAREPATH_FOR_RULE_VALIDATION_WITH_FLT");
ConfigurableValue<uint32_t>
maxFlightsForRuleValidationCfg("SHOPPING_OPT", "MAX_FLIGHTS_RULE_VALIDATION");
ConfigurableValue<uint32_t>
altDateDiversityDivider("SHOPPING_OPT", "ALT_DATE_DIVERSITY_DIVIDER");
ConfigurableValue<uint32_t>
altDateItinPriceJumpFactor("PRICING_SVC", "ALT_DATE_ITIN_PRICE_JUMP_FACTOR");
ConfigurableValue<uint64_t>
altDateCutoffNucThreshold("PRICING_SVC", "ALT_DATE_CUTOFF_NUC_THRESHOLD");
ConfigurableValue<uint32_t>
altDateMaxPassedBitCfg("SHOPPING_OPT", "ALT_DATE_MAX_PASSED_BIT");
ConfigurableValue<uint32_t>
maxFailedCellsToValidateCfg("SHOPPING_OPT", "MAX_FLIGHT_MATRIX_FAILED_CELLS_ALTDATE");
ConfigurableValue<uint32_t>
maxFailedCellsForFltOnlySolution("SHOPPING_OPT", "MAX_FLIGHT_MATRIX_FAILED_FLT_ONLY_SOLUTION");
ConfigurableValue<bool>
taxForISProcess("SHOPPING_OPT", "TAX_FOR_IS_PROCESS", false);
ConfigurableValue<bool>
combinedFamily("SHOPPING_OPT", "COMBINED_FAMILY", false);
ConfigurableValue<bool>
cat12ForAltDatesCfg("SHOPPING_OPT", "CAT12_FOR_ALTDATES", false);
ConfigurableValue<bool>
allowToRemoveCustomFamily("SHOPPING_OPT", "REMOVE_CUSTOM_FAMILY", false);
ConfigurableValue<uint32_t>
altDateFareAmountWeightFactor("SHOPPING_OPT", "ALT_DATE_FARE_AMOUNT_WEIGHT_FACTOR");
ConfigurableValue<double>
altDateMinimumWeightFactor("SHOPPING_OPT", "ALT_DATE_MINIMUM_WEIGHT_FACTOR");
ConfigurableValue<uint16_t>
noOfTaxCallsForDiffCnxPoints("SHOPPING_OPT", "NUMBER_OF_TAX_CALLS_FOR_DIFF_CITY");
ConfigurableValue<double>
searchBayondMaxParamsCalculateFactorCfg("SHOPPING_OPT", "MAX_AMOUNTS_FOR_SEARCH_BEYOND_FACTOR");
ConfigurableValue<uint32_t>
fltCombMaxForSB("SHOPPING_OPT", "MAX_FLIGHT_COMB_FOR_SEARCH_BEYOND");
ConfigurableValue<uint32_t>
maxAmountsForCustomSearchFactor("SHOPPING_OPT", "MAX_AMOUNTS_FOR_CUSTOM_SEARCH_FACTOR");
ConfigurableValue<uint32_t>
fltCombMaxForCustom("SHOPPING_OPT", "MAX_FLIGHT_COMB_FOR_CUSTOM_SEARCH");
// this class contains a group of TravelSegs that come from a FareUsage object.
// since iteration over FareUsage objects is not necessarily in order of where
// they appear in the journey, it is useful to use this class to better group
// them in the order in which they actually occur.
class FareUsageTravelSegs
{
public:
  FareUsageTravelSegs(uint32_t position, const std::vector<TravelSeg*>& travelSegs)
    : _position(position), _segs(&travelSegs)
  {
  }

  bool operator<(const FareUsageTravelSegs& o) const { return _position < o._position; }

  const std::vector<TravelSeg*>& segs() const { return *_segs; }

private:
  uint32_t _position;
  const std::vector<TravelSeg*>* _segs;
};

static const ItinIndex::Key INTERLINE = 0;

void
getEncodedCarriersForSOPs(const ShoppingTrx& trx, std::vector<std::vector<ItinIndex::Key>>& res)
{
  res.reserve(trx.legs().size());

  for (std::vector<ShoppingTrx::Leg>::const_iterator i = trx.legs().begin(); i != trx.legs().end();
       ++i)
  {
    if (i->stopOverLegFlag())
      continue;

    const size_t nsops = i->requestSops();
    res.push_back(std::vector<ItinIndex::Key>(nsops, INTERLINE));

    for (size_t n = 0; n != nsops; ++n)
    {
      const ShoppingTrx::SchedulingOption& sop = i->sop()[n];
      const Itin& itin = *sop.itin();
      const std::vector<TravelSeg*>& segs = itin.travelSeg();
      const CarrierCode* carrier = nullptr;

      for (const auto travelSegment : segs)
      {
        const AirSeg* const airSeg = travelSegment->toAirSeg();

        if (UNLIKELY(!airSeg))
          continue;

        if (carrier == nullptr)
        {
          carrier = &airSeg->marketingCarrierCode();
        }
        else if (*carrier != airSeg->marketingCarrierCode())
        {
          carrier = nullptr;
          break;
        }
      }

      if (carrier != nullptr)
        ShoppingUtil::createCxrKey(*carrier, res.back()[n]);
    }
  }
}

typedef std::tr1::unordered_map<std::string, MoneyAmount, boost::hash<std::string>>::const_iterator
TaxedCPointsIC;
typedef std::tr1::unordered_map<std::string, MoneyAmount, boost::hash<std::string>>::iterator
TaxedCPointsI;
typedef std::pair<std::string, MoneyAmount> TaxedCPointsPair;
typedef std::vector<TravelSeg*>::const_iterator TSegVecIC;

std::string
getCnxPoints(const TravelSegPtrVec& tvlSeg)
{
  TSegVecIC itvl1EndIt = tvlSeg.end();
  std::string output = "";
  bool firstCnxInLeg = true;

  for (TSegVecIC itvl1It = tvlSeg.begin(); itvl1It != itvl1EndIt; ++itvl1It)
  {
    if (UNLIKELY((*itvl1It)->segmentType() == Arunk))
    {
      continue; // do not check if it's ARUNK segment
    }

    // Does it enough to check only stopOver??
    // We need to check if the last segment because surprisingly for last segment we set connection
    bool stopover = (*itvl1It)->stopOver() || (itvl1It + 1) == itvl1EndIt;

    if (firstCnxInLeg && !stopover)
    {
      output += (*itvl1It)->destAirport();
      firstCnxInLeg = false;
    }

    if (stopover)
    {
      // check only first connection
      firstCnxInLeg = true;
    }
  }

  return output;
}

std::string
getSopsCnxPoints(const ShoppingTrx& trx, const std::vector<int>& sops)
{
  if (sops.empty())
    return "";

  std::vector<TravelSeg*> travelSegs;

  for (uint32_t leg = 0; leg != sops.size(); ++leg)
  {
    travelSegs.insert(travelSegs.end(),
                      trx.legs()[leg].sop()[sops[leg]].itin()->travelSeg().begin(),
                      trx.legs()[leg].sop()[sops[leg]].itin()->travelSeg().end());
  }

  return getCnxPoints(travelSegs);
}

SopIdVec
getActualSops(ShoppingPQ::SopIdWrapperVec& candidate)
{
  SopIdVec sops;
  sops.reserve(candidate.size());
  for (const ShoppingPQ::SOPWrapper& sop : candidate)
    sops.push_back(sop._sopId);
  return sops;
}
} // end anonymous namespace

ShoppingPQ::ShoppingPQ(PricingOrchestrator& po,
                       ShoppingTrx& aTrx,
                       Itin* journeyItin,
                       uint32_t noOfOptionsRequested,
                       int nEstimatedOptions,
                       const CarrierCode* carrier,
                       BitmapOpOrderer& bitmapOpOrderer,
                       const bool searchAlwaysLowToHigh,
                       bool owFaresShoppingQueue,
                       int16_t fareCombRepeatLimit,
                       bool nonStopShoppingQueue)
  : _diag(nullptr),
    _error(ErrorResponseException::NO_ERROR),
    _trx(&aTrx),
    _po(&po),
    _journeyItin(journeyItin),
    _carrier(carrier),
    _fareMarketRuleController(aTrx.fareMarketRuleController()),
    _asoRuleController(aTrx.ASOfareMarketRuleController()),
    _cat4RuleController(aTrx.cat4RuleController()),
    _validatingCarrier(*_trx),
    _vitaValidator(*_trx, _carrier),
    _fmpMatrix(aTrx, *journeyItin, _mergedFareMarketVect),
    _groupFarePathFactory(aTrx),
    _fmMerger(aTrx, *journeyItin, searchAlwaysLowToHigh, carrier),
    _flightIndependentRuleController(ShoppingItinFlightIndependentValidation),
    _ruleController(ShoppingItinBasedValidation),
    _bitmapOpOrderer(bitmapOpOrderer),
    _MultiAirportAgent(nullptr),
    _lngCnxAllowedInOnlineQ(false),
    _searchBeyondActivated(false),
    _customSolutionSearchActivated(false),
    _owFaresShoppingQueue(owFaresShoppingQueue),
    _nonStopShoppingQueue(nonStopShoppingQueue),
    _foundHighestFarePath(false),
    _doneHurryWithCond(false),
    _getMoreEstimatedSolutions(false),
    _getMoreEstimatedFlightOnlySolutions(false),
    _createMoreSolution(true),
    _taxForISProcess(false),
    _combinedFamily(false),
    _cat12ForAltDates(false),
    _allowToRemoveCustomFamily(false),
    _nEstimatedOptions(nEstimatedOptions),
    _fareCombRepeatLimit(fareCombRepeatLimit),
    _optionsPerFarePath(0),
    _fltCombMaxForSB(5),
    _farePathForRuleValMaxForSB(10),
    _farePathForRuleValWithFltMaxForSB(5),
    _fltCombMaxForCustom(5),
    _farePathForRuleValMaxForCustom(10),
    _farePathForRuleValWithFltMaxForCustom(5),
    _farePathForRuleValMax(800),
    _farePathForRuleValWithFltMax(300),
    _noOfTaxCallsForDiffCnxPoints(0),
    _maxFlightsForRuleValidation(50),
    _noOfOptionsRequested(noOfOptionsRequested),
    _farePathTried(0),
    _fltCombTried(0),
    _fltCombMax(400),
    _multiLegFltCombMax(0),
    _altDateDiversityDivider(1),
    _interlineDiversityPercent(100),
    _mustPriceInterlineSnowManPercent(0),
    _altDateItinPriceJumpFactor(100000),
    _altDateMaxPassedBit(1000),
    _maxFailedCellsToValidate(1000000),
    _maxFailedCellsForFltOnlySolution(100000),
    _altDateFareAmountWeightFactor(400),
    _altDateHighestAmountAllow(0),
    _altDateCutoffNucThreshold(100000),
    _taxAmount(0),
    _numBadEstimates(0),
    _minimumFamilySize(3),
    _altDateMinimumWeightFactor(1.5),
    _numFPAndMustPriceSnowManFltOnlyInterlineOption(0),
    _puMatrix(nullptr),
    _lastSolution(nullptr),
    _cxrCode(0)
{
  desc("PO SHOPPINGPQ");
  trx(&aTrx);

  if (shoppingTrx().pqDiversifierResult().numberInterlineOptions == 0 &&
      shoppingTrx().pqDiversifierResult().onlineCarrierList.size() == 1)
  {
    _lngCnxAllowedInOnlineQ = true;
  }

  if (aTrx.isAltDates())
  {
    setNoOfOptionsRequested(0);
    _ruleController.setPhase(ShoppingAltDateItinBasedValidation);

    for (const auto& altDatePair : aTrx.altDatePairs())
    {
      setNoOfOptionsRequested(getNoOfOptionsRequested() + altDatePair.second->numOfSolutionNeeded);
      // add the journey itin to our internal map
      _journeyItinAltDates.insert(
          std::pair<DatePair, Itin>(altDatePair.first, *altDatePair.second->journeyItin));
    }

    if (_carrier)
    {
      for (const auto& altDatePair : aTrx.altDatePairs())
      {
        ShoppingTrx::AltDateInfo* altDateInfo = nullptr;
        aTrx.dataHandle().get(altDateInfo);
        altDateInfo->numOfSolutionNeeded = altDatePair.second->numOfSolutionNeeded;
        std::pair<DatePair, ShoppingTrx::AltDateInfo*> mapItem(altDatePair.first, altDateInfo);
        _altDatePairsPQ.insert(mapItem);
      }
    }
  }

  // find the number of actual legs we are calculating
  size_t numLegs = 0;

  while (numLegs != aTrx.legs().size() && aTrx.legs()[numLegs].stopOverLegFlag() == false)
  {
    ++numLegs;
  }

  // calculate the dimensions of the flight matrix
  for (size_t n = 0; n != numLegs; ++n)
  {
    _flightMatrixDim.push_back(int(aTrx.legs()[n].requestSops()));
  }

  if (shoppingTrx().getNumOfCustomSolutions() > 0)
  {
    for (size_t n = 0; n != numLegs; ++n)
    {
      SopIdWrapperVec sopsCollection;
      ShoppingTrx::Leg& leg = shoppingTrx().legs()[n];
      const bool isCustomLeg = leg.isCustomLeg();

      for (int index(0); index < _flightMatrixDim[n]; ++index)
      {
        if (!leg.sop()[index].cabinClassValid())
          continue;
        if (!isCustomLeg)
          sopsCollection.push_back(SOPWrapper(index));
        else if (leg.sop()[index].isCustomSop())
          sopsCollection.push_back(SOPWrapper(index));
      }
      _legWrappedCustomSopsCollection.push_back(sopsCollection);
    }
  }

  // setup variables with which we can easily tell if we want to process
  // a certain cell in the matrix or not
  getEncodedCarriersForSOPs(aTrx, _sopCarriers);
  if (_carrier)
  {
    _fltCombMax = fltCombMaxOnline.getValue();
    _minimumFamilySize = minimumFamilySizeOnline.getValue();
  }
  else
  {
    _fltCombMax = fltCombMaxInterline.getValue();
    _minimumFamilySize = minimumFamilySizeInterline.getValue();
    _interlineDiversityPercent = interlineDiversityPercent.getValue();
    _mustPriceInterlineSnowManPercent = mustPriceInterlineSnowManPercent.getValue();
    if (shoppingTrx().getRequestedNumOfEstimatedSolutions() > 0)
    {
      _interlineDiversityPercent = interlineDiversityPercentBFM.getValue();
    }
  }

  if (_trx->legs().size() >= 5)
    _multiLegFltCombMax = 10 * _fltCombMax;

  _farePathForRuleValMax = farePathForRuleValMax.getValue();
  _farePathForRuleValWithFltMax = farePathForRuleValWithFltMax.getValue();

  if (_carrier)
  {
    ShoppingUtil::createCxrKey(*_carrier, _cxrCode);
  }

  // get maximum number of sops to send to rule validation

  _maxFlightsForRuleValidation = maxFlightsForRuleValidationCfg.getValue();
  _altDateDiversityDivider = altDateDiversityDivider.getValue();
  _altDateItinPriceJumpFactor = altDateItinPriceJumpFactor.getValue();
  _altDateCutoffNucThreshold = altDateCutoffNucThreshold.getValue();
  _altDateMaxPassedBit = altDateMaxPassedBitCfg.getValue();
  _maxFailedCellsToValidate = maxFailedCellsToValidateCfg.getValue();
  _maxFailedCellsForFltOnlySolution = maxFailedCellsForFltOnlySolution.getValue();
  _taxForISProcess = taxForISProcess.getValue();
  _combinedFamily = combinedFamily.getValue();
  _cat12ForAltDates = cat12ForAltDatesCfg.getValue();
  _allowToRemoveCustomFamily = allowToRemoveCustomFamily.getValue();
  _altDateFareAmountWeightFactor = altDateFareAmountWeightFactor.getValue();
  _altDateMinimumWeightFactor = altDateMinimumWeightFactor.getValue();
  //* Settings for IS SB enhancement

  _noOfTaxCallsForDiffCnxPoints = noOfTaxCallsForDiffCnxPoints.getValue();
  double searchBayondMaxParamsCalculateFactor = searchBayondMaxParamsCalculateFactorCfg.getValue();

  double searchBeyondfactor = searchBayondMaxParamsCalculateFactor / RuleConst::HUNDRED_PERCENTS;
  _farePathForRuleValMaxForSB =
      static_cast<uint32_t>(ceil(_farePathForRuleValWithFltMax * searchBeyondfactor));
  _farePathForRuleValWithFltMaxForSB =
      static_cast<uint32_t>(ceil(_farePathForRuleValWithFltMax * searchBeyondfactor));

  _fltCombMaxForSB = fltCombMaxForSB.getValue();

  double customSearchMaxParamsCalculateFactor = maxAmountsForCustomSearchFactor.getValue();

  double customSearchFactor = customSearchMaxParamsCalculateFactor / RuleConst::HUNDRED_PERCENTS;
  _farePathForRuleValMaxForCustom =
      static_cast<uint32_t>(ceil(_farePathForRuleValMax * customSearchFactor));
  _farePathForRuleValWithFltMaxForCustom =
      static_cast<uint32_t>(ceil(_farePathForRuleValWithFltMax * customSearchFactor));
  _fltCombMaxForCustom = fltCombMaxForCustom.getValue();
  //*~ Settings for IS tax enhancement
} // lint !e1401

bool
ShoppingPQ::checkHurryCondOld()
{
  bool mustHurry = fallback::reworkTrxAborter(_trx) ? checkTrxMustHurryWithCond(shoppingTrx())
                                                    : shoppingTrx().checkTrxMustHurryWithCond();
  return (((((_farePathTried > _farePathForRuleValWithFltMax) && (_fltCombTried > _fltCombMax)) ||
            (_farePathTried > _farePathForRuleValMax)) &&
           mustHurry) ||
          checkHurryCondForSearchBeyond() || checkHurryCondForCustomSearch());
}

bool
ShoppingPQ::checkHurryCond()
{
  if (!_multiLegFltCombMax)
    return checkHurryCondOld();

  bool hurryForManyLegs = (_fltCombTried > _multiLegFltCombMax);
  bool mustHurry = fallback::reworkTrxAborter(_trx) ? checkTrxMustHurryWithCond(shoppingTrx())
                                                    : shoppingTrx().checkTrxMustHurryWithCond();

  return ((((((_farePathTried > _farePathForRuleValWithFltMax) && (_fltCombTried > _fltCombMax)) ||
             (_farePathTried > _farePathForRuleValMax)) ||
            hurryForManyLegs) &&
           mustHurry) ||
          checkHurryCondForSearchBeyond() || checkHurryCondForCustomSearch());
}

bool
ShoppingPQ::getAdditionalSolutions(uint32_t noptions)
{
  const size_t noOffltMatrixSolution = getFlightMatrix().size();

  if (_lastSolution == nullptr && _getMoreEstimatedFlightOnlySolutions == false)
  {
    return false;
  }

  setNoOfOptionsRequested(noptions + getFlightMatrix().size());

  if (_getMoreEstimatedFlightOnlySolutions)
  {
    generateEstimatedFlightOnlySolutions();
  }
  else
  {
    checkNumberOfSolutions();
    processSolution(_lastSolution);

    try
    {
      getSolutions();
    }
    catch (ErrorResponseException& ere)
    {
      // a timeout isn't considered an error. It just means we didn't have
      // enough time to finish this queue.
      if (ere.code() == ErrorResponseException::REQUEST_TIMEOUT)
      {
        return false;
      }

      LOG4CXX_ERROR(logger, "error when executing shopping PQ: " << ere.message());
      _error = ere;
    }
    catch (std::bad_alloc&)
    {
      LOG4CXX_ERROR(logger, "memory exhaustion when executing shopping PQ");
      _error =
          ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "Ran out of memory");
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger, "unknown error when executing shopping PQ");
      _error = ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                      "Unknown error in shopping PQ");
    }
  }

  if (noOffltMatrixSolution < getFlightMatrix().size())
  {
    return true;
  }

  return false;
}

void
ShoppingPQ::checkNumberOfSolutions()
{
  const uint32_t FlightMatrixCells =
      std::accumulate(_flightMatrixDim.begin(), _flightMatrixDim.end(), 1, std::multiplies<int>());

  if (getNoOfOptionsRequested() > FlightMatrixCells)
  {
    setNoOfOptionsRequested(FlightMatrixCells);
  }
}

void
ShoppingPQ::performTask()
{
  try
  {
    // const MallocContext allocatorContext;
    generateQueue();

    bool mustHurry = false;
    if (fallback::reworkTrxAborter(&shoppingTrx()))
      mustHurry = checkTrxMustHurry(shoppingTrx());
    else
      mustHurry = shoppingTrx().checkTrxMustHurry();

    if (mustHurry)
    {
      return;
    }

    getSolutions();
  }
  catch (ErrorResponseException& ere)
  {
    // too many combos isn't considered an error.
    if (ere.code() == ErrorResponseException::TOO_MANY_COMBOS)
    {
      return;
    }

    // a timeout isn't considered an error. It just means we didn't have
    // enough time to finish this queue.
    if (ere.code() == ErrorResponseException::REQUEST_TIMEOUT)
    {
      return;
    }

    LOG4CXX_ERROR(logger, "error when executing shopping PQ: " << ere.message());
    _error = ere;
  }
  catch (std::bad_alloc&)
  {
    LOG4CXX_ERROR(logger, "memory exhaustion when executing shopping PQ");
    _error = ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "Ran out of memory");
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "unknown error when executing shopping PQ");
    _error = ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                    "Unknown error in shopping PQ");
  }
}

void
ShoppingPQ::generateQueue()
{
  ShoppingTrx& aTrx = *_trx;

  bool mustHurry = false;
  if (fallback::reworkTrxAborter(&aTrx))
    mustHurry = checkTrxMustHurry(aTrx);
  else
    mustHurry = aTrx.checkTrxMustHurry();

  if (mustHurry)
  {
    return;
  }

  TSELatencyData metrics(aTrx, "PO SHOP PQ GENERATE QUEUE");

  // Instantiate all the buckets of PricingUnitFactory
  // One bucket per PaxType
  if (!_po->createPricingUnitFactoryBucket(aTrx, _puFactoryBucketVect))
  {
    throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                 "Could not create pricing units");
  }

  //----- Merged FareMarket of multiple Gov Cxr into one  -----
  _fmMerger.buildAllMergedFareMarket(_mergedFareMarketVect);
  LOG4CXX_INFO(logger, " _mergedFareMarketVect size = " << _mergedFareMarketVect.size());
  //----- Create FareMarketPath Matrix  -----
  _fmpMatrix.buildAllFareMarketPath();
  PricingOrchestrator::getRec2Cat10(*_trx, _mergedFareMarketVect);
  //----- Create PUPath Matrix -----
  aTrx.dataHandle().get(_puMatrix);
  _puMatrix->trx() = &aTrx;
  _puMatrix->itin() = _journeyItin;
  _puMatrix->useCxrPreference() = !isInterline();
  _puMatrix->avoidStaticObjectPool() = _po->getFactoriesConfig().avoidStaticObjectPool();
  _puMatrix->config() = &Global::config();
  _puMatrix->buildAllPUPath(_fmpMatrix, _puFactoryBucketVect);
  LOG4CXX_INFO(logger,
               "Num of PaxType, _puFactoryBucketVect size = " << aTrx.paxType().size() << ","
                                                              << _puFactoryBucketVect.size());
  //-------------- Init PricingUnit Factory -------------------------
  _po->initPricingUnitFactory(
      aTrx, _puFactoryBucketVect, diag(), isOwFaresShoppingQueue(), isNonStopShoppingQueue());
  diag().flushMsg();
  //------------------ Price the Itin -----------------
  checkNumberOfSolutions();

  // Create FarePathFactory, one for each PaxType

  ShoppingPaxFarePathFactoryCreator shpPaxfpfCreater(shoppingTrx(), *this, _bitmapOpOrderer);

  if (!_po->createPaxFarePathFactory(
          aTrx, *_puMatrix, _puFactoryBucketVect, _paxFarePathFactoryBucket, shpPaxfpfCreater))
  {
    throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                 "Could not create fare paths");
  }
  if (fallback::reworkTrxAborter(&aTrx))
    mustHurry = checkTrxMustHurry(aTrx);
  else
    mustHurry = aTrx.checkTrxMustHurry();

  if (mustHurry)
  {
    return;
  }

  // Init FarePathFactory -- Multi-Threaded
  _po->initPaxFarePathFactory(aTrx, _paxFarePathFactoryBucket, diag(), !isOwFaresShoppingQueue());
  // Create/Init GroupFarePathFactory
  _groupFarePathFactory.usePooling() = false;
  _groupFarePathFactory.setPaxFarePathFactoryBucket(_paxFarePathFactoryBucket);
  aTrx.dataHandle().get(_groupFarePathFactory.accompaniedTravel());

  if (!_groupFarePathFactory.initGroupFarePathPQ())
  {
    throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                 "Could not init group fare path PQ");
  }
}

void
ShoppingPQ::getSolutions()
{
  const TSELatencyData metrics(shoppingTrx(), "PO SHOPPING PQ GET SOLUTIONS");

  while ((needsMoreSolutions(getNoOfOptionsRequested())) &&
         (fallback::reworkTrxAborter(_trx) ? !checkTrxMustHurry(shoppingTrx())
                                           : !shoppingTrx().checkTrxMustHurry()))
  {
    _lastSolution = generateSolution(_altDateHighestAmountAllow);

    if ((_lastSolution == nullptr) || _foundHighestFarePath)
    {
      break;
    }

    if (UNLIKELY(
            (_altDateHighestAmountAllow != 0) &&
            (_altDateHighestAmountAllow <
             _taxAmount +
                 (*(_lastSolution->groupFPPQItem().begin()))->farePath()->getTotalNUCAmount())))
    {
      _foundHighestFarePath = true;
    }

    if (UNLIKELY(_customSolutionSearchActivated))
    {
      // We need to make sure if last solution is a custom solution before processing it.
      if (!ShoppingUtil::isCustomSolutionGfp(shoppingTrx(), _lastSolution))
      {
        continue;
      }
    }

    processSolution(_lastSolution);
  }

  if (shoppingTrx().diagnostic().diagnosticType() == Diagnostic910)
  {
    Diag910Collector* const collector =
        dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(shoppingTrx()));
    TSE_ASSERT(collector != nullptr);
    Diag910Collector& stream = *collector;
    std::string queueType;

    if (_carrier)
    {
      queueType = *_carrier;
    }
    else if (isOwFaresShoppingQueue())
    {
      queueType = "OW Fares";
    }
    else if (isNonStopShoppingQueue())
    {
      queueType = "Non-stop";
    }
    else
    {
      queueType = "Interline";
    }

    stream << "Results for queue '" << queueType << "':\n";

    if (shoppingTrx().diagnostic().diagParamMapItem("DD") == "MATRIX" ||
        shoppingTrx().diagnostic().diagParamMapItem("DD") == "FLIGHTS")
    {
      stream << "(" << getNoOfOptionsRequested() << " results requested)\n";
    }

    bool showOwFareKeyDetails =
        isOwFaresShoppingQueue() && (shoppingTrx().diagnostic().diagParamMapItem("DD") == "FPKEY");
    stream.printHeader(showOwFareKeyDetails);
    stream.printFarePaths(
        getFlightMatrix(), getEstimateMatrix(), shoppingTrx(), showOwFareKeyDetails);
    stream.flushMsg();
  }

  if (_searchBeyondActivated)
    return;

  generateEstimatedSolutions();
}

namespace
{
// lint --e{578}
class FareHasQualifierForRule
{
public:
  FareHasQualifierForRule(const PaxTypeFare& fare) : _fare(&fare) {}

  bool operator()(uint32_t cat) const { return _fare->qualifyFltAppRuleDataMap().count(cat) != 0; }

private:
  const PaxTypeFare* _fare;
};
}

GroupFarePath*
ShoppingPQ::generateSolution(const MoneyAmount lastAmount)
{
  const TSELatencyData metrics(shoppingTrx(), "PO SHOP PQ GENERATE SOLUTION");

  if (checkHurryCond())
  {
    return nullptr;
  }

  ShoppingTrx& trx = shoppingTrx();
  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  if (_puMatrix->puPathMatrix().empty())
  {
    return nullptr;
  }

  if (_lastSolution != nullptr)
  {
    _groupFarePathFactory.buildNextGroupFarePathSet(false, *_lastSolution, diag());
    diag().flushMsg();
  }

  GroupFarePath* res = _groupFarePathFactory.getGroupFarePath(diag(), lastAmount);

  if (res)
    res->setSourcePqName(getSourceName());

  diag().flushMsg();
  return res;
}

bool
ShoppingPQ::preValidateFarePath(FarePath& farePath)
{
  for (auto pricingUnit : farePath.pricingUnit())
  {
    std::vector<uint16_t> cats(_flightIndependentRuleController.categorySequence());
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      const PaxTypeFare& fare = *fareUsage->paxTypeFare();
      cats.erase(std::remove_if(cats.begin(), cats.end(), FareHasQualifierForRule(fare)),
                 cats.end());
    }

    if (UNLIKELY(cats.empty() == false))
    {
      _flightIndependentRuleController.categorySequence().swap(cats);
      const bool res =
          _flightIndependentRuleController.validate(shoppingTrx(), farePath, *pricingUnit);
      _flightIndependentRuleController.categorySequence().swap(cats);

      if (res == false)
      {
        return false;
      }
    }
  }

  return true;
}

void
ShoppingPQ::farePathValidationResult(FarePath& farePath,
                                     const char* result,
                                     const char* resultFVO,
                                     int fareIndex)
{
  const bool showDiag910Details = shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
                                  shoppingTrx().diagnostic().diagParamMapItem("DD") == "DETAILS";

  if (LIKELY(showDiag910Details == false))
  {
    return;
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);
  std::string queueType;

  if (_carrier)
  {
    queueType = *_carrier;
  }
  else if (isNonStopShoppingQueue())
  {
    queueType = "Non-stop";
  }
  else
  {
    queueType = "Interline";
  }

  // diag << "Queue '" << queueType << "' fare path validation for pax type '" <<
  // farePath.paxType()->paxType() << "':\n";
  diag << "Queue '" << queueType << "' fare path validation for pax type '"
       << farePath.paxType()->paxType() << "  " << resultFVO << "':\n";
  Diag910Collector* const diag910 = dynamic_cast<Diag910Collector*>(&diag.collector());
  TSE_ASSERT(diag910 != nullptr);
  diag910->printFarePath(farePath, shoppingTrx());

  if (fareIndex == 0)
    *diag910 << " (" << result << ")\n";
  else
    *diag910 << " (" << result << std::setw(3) << "  FareIndex=" << fareIndex << ")\n";

  if (farePath.plusUpFlag())
  {
    *diag910 << "PLUS UP ADDED NEW AMOUNT " << farePath.getTotalNUCAmount() << "\n";
  }
}

namespace
{
void
setFareUsageTravelSegs(const std::vector<int>& sops, FarePath& farePath, const ShoppingTrx& trx)
{
  for (const auto pricingUnit : farePath.pricingUnit())
  {
    for (const auto fareUsage : (*pricingUnit).fareUsage())
    {
      FareMarket& fMarket = *(*fareUsage).paxTypeFare()->fareMarket();
      const std::vector<TravelSeg*>& fmTravelSegs = fMarket.travelSeg();
      TSE_ASSERT(fmTravelSegs.empty() == false); // lint !e666
      uint32_t startIndex = 0, endIndex = 0, adoptedIndex = 0;
      const bool res = ShoppingUtil::getLegTravelSegIndices(
          trx, fmTravelSegs, startIndex, endIndex, adoptedIndex);

      if (!res)
      {
        throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                     "ShoppingUtil::getLegTravelSegIndices returned false");
      }

      fareUsage->travelSeg().clear();

      for (uint32_t index = startIndex; index != endIndex; ++index)
      {
        const ShoppingTrx::Leg& leg = trx.legs()[index];
        const int sop = sops[index];

        if (sop >= int(leg.sop().size()))
        {
          throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                       "sop >= int(leg.sop().size()");
        }

        std::vector<TravelSeg*>& fuTravelSegs = fareUsage->travelSeg();
        fuTravelSegs.insert(fuTravelSegs.end(),
                            leg.sop()[sop].itin()->travelSeg().begin(),
                            leg.sop()[sop].itin()->travelSeg().end());
      }
    }
  }
}
}

void
ShoppingPQ::processSolution(GroupFarePath* groupFPath, bool extraFarePath)
{
  const TSELatencyData metrics(shoppingTrx(), "PO SHOP PQ PROCESS SOLUTION");
  const PricingUnit* pu =
      (*(*(groupFPath->groupFPPQItem().begin()))->farePath()->pricingUnit().begin());
  const FareMarket* fMarket = (*(pu->fareUsage().begin()))->paxTypeFare()->fareMarket();
  const CarrierCode& govCxr = fMarket->governingCarrier();
  const bool showDiag910Details = shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
                                  shoppingTrx().diagnostic().diagParamMapItem("DD") == "DETAILS";
  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (UNLIKELY(showDiag910Details))
  {
    std::string queueType;

    if (_carrier)
    {
      queueType = *_carrier;
    }
    else if (isOwFaresShoppingQueue())
    {
      queueType = "OW Fares";
    }
    else if (isNonStopShoppingQueue())
    {
      queueType = "Non-stop";
    }
    else
    {
      queueType = "Interline";
    }

    Diag910Collector* const stream = dynamic_cast<Diag910Collector*>(&diag.collector());
    TSE_ASSERT(stream != nullptr);
    *stream << "RESULT for queue '" << queueType << "':\n";
    stream->printGroupFarePath(*groupFPath, shoppingTrx());
  }

  std::vector<const DatePair*> dates;

  if (UNLIKELY(doneDatePairCxrWithFarePath(govCxr, dates)))
  {
    return;
  }

  for (const auto datePair : dates)
  {
    std::vector<FarePathFlightsInfo*> info;
    info.reserve(groupFPath->groupFPPQItem().size());

    for (std::vector<FPPQItem*>::iterator it = groupFPath->groupFPPQItem().begin();
         it != groupFPath->groupFPPQItem().end();
         ++it)
    {
      const std::size_t index = it - groupFPath->groupFPPQItem().begin();
      TSE_ASSERT(index < _paxFarePathFactoryBucket.size());
      TSE_ASSERT(_paxFarePathFactoryBucket[index] != nullptr);
      ShoppingPaxFarePathFactory* const factory =
          dynamic_cast<ShoppingPaxFarePathFactory*>(_paxFarePathFactoryBucket[index]);
      TSE_ASSERT(factory != nullptr);
      info.push_back(&factory->getFarePathFlightsInfo(*(*it), datePair));
    }

    _optionsPerFarePath = 0;

    if (doneOptionPerFarePath(govCxr, datePair))
    {
      continue;
    }

    processFarePathFlightsInfo(groupFPath, info, extraFarePath);
  }
}

bool
ShoppingPQ::doneDatePairCxrWithFarePath(const CarrierCode& govCxr,
                                        std::vector<const DatePair*>& dates)
{
  bool done = true;

  if (!shoppingTrx().isAltDates())
  {
    dates.push_back(nullptr);
  }
  else
  {
    for (const auto& altDatePairPQ : _altDatePairsPQ)
    {
      if (altDatePairPQ.second->numOfSolutionNeeded > 0)
      {
        std::map<DatePair, AltDateOptionInfo>::iterator datePairCxrI;
        datePairCxrI = _cxrOptionPerDatesMap.find(altDatePairPQ.first);

        if (datePairCxrI != _cxrOptionPerDatesMap.end())
        {
          if (govCxr != datePairCxrI->second.cxrCode)
          {
            done = false;
          }
        }
        else
        {
          done = false;
        }

        dates.push_back(&altDatePairPQ.first);
      }
    }

    if (done && (_altDateDiversityDivider != 1))
    {
      return true;
    }
  }

  return false;
}
bool
ShoppingPQ::doneOptionPerFarePath(const CarrierCode& govCxr, const DatePair* d)
{
  if ((d != nullptr) && (_altDateDiversityDivider != 1))
  {
    ShoppingTrx::AltDatePairs::iterator itor = shoppingTrx().altDatePairs().find(*d);
    std::map<DatePair, AltDateOptionInfo>::iterator datePairCxrI = _cxrOptionPerDatesMap.find(*d);

    if ((datePairCxrI != _cxrOptionPerDatesMap.end()) && (govCxr == datePairCxrI->second.cxrCode))
    {
      return true;
    }

    if ((datePairCxrI == _cxrOptionPerDatesMap.end()) &&
        (itor != shoppingTrx().altDatePairs().end()))
    {
      // now it diverse half of the option for different carrier
      _optionsPerFarePath = getFlightMatrix().size() +
                            (itor->second->numOfSolutionNeeded) / _altDateDiversityDivider + 1;
    }
  }

  return false;
}

void
ShoppingPQ::processFarePathFlightsInfo(GroupFarePath* groupFPath,
                                       std::vector<FarePathFlightsInfo*>& info,
                                       bool extraFarePath)
{
  // if this is the cheapest fare path to produce results, and it produces
  // as many results as requested, then produce up to 3 additional results
  // from this fare path, to make sure we get enough results in all cases.
  const size_t MaxAdditionalOptionsCheapestFarePath = 3;
  size_t stopat = shoppingTrx().isAltDates() ? 1000000 : getNoOfOptionsRequested();

  if (getFlightMatrix().empty() && !shoppingTrx().isAltDates() &&
      (shoppingTrx().getRequestedNumOfEstimatedSolutions() <= 0))
  {
    stopat += MaxAdditionalOptionsCheapestFarePath;
  }

  // % of options using fare paths
  double useFarePath = stopat * (_interlineDiversityPercent / RuleConst::HUNDRED_PERCENTS);
  size_t newStopat = std::max<uint32_t>(1, static_cast<uint32_t>(ceil(useFarePath)));
  bool interlineDiver = false;
  size_t mustPriceInterlineSnowmanOption = 0;

  if (stopat > useFarePath)
  {
    mustPriceInterlineSnowmanOption =
        static_cast<uint32_t>(ceil((stopat - useFarePath) * (_mustPriceInterlineSnowManPercent /
                                                             RuleConst::HUNDRED_PERCENTS)));
  }

  _numFPAndMustPriceSnowManFltOnlyInterlineOption = newStopat + mustPriceInterlineSnowmanOption;

  const bool skipMillionNuc = (shoppingTrx().getRequest()->isParityBrandsPath() ||
                               _trx->diversity().isExchangeForAirlines());

  if (!_carrier && !shoppingTrx().isAltDates() && !isOwFaresShoppingQueue() && !skipMillionNuc &&
      !isNonStopShoppingQueue() && (_interlineDiversityPercent != RuleConst::HUNDRED_PERCENTS))
  {
    if ((!getFlightMatrix().empty()) && (getFlightMatrix().size() > newStopat))
    {
      if (!_customSolutionSearchActivated)
      {
        GroupFarePath* gfp = shoppingTrx().dataHandle().create<GroupFarePath>();
        gfp->setSourcePqName(getSourceName());
        gfp->setTotalNUCAmount(1000000);
        generateSolutionsWithNoFares(gfp);
      }
    }
    else
    {
      stopat = newStopat;
      interlineDiver = true;
    }
  }

  if (_optionsPerFarePath > 0)
  {
    stopat = _optionsPerFarePath;
  }

  if (UNLIKELY(isNonStopShoppingQueue()))
  {
    stopat = getNoOfOptionsRequested();
    interlineDiver = false;
  }

  TSE_ASSERT(info.empty() == false);
  const std::set<std::vector<int>>& sopSet = info.front()->getPassedSops();

  const bool showDiag910Details = shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
                                  shoppingTrx().diagnostic().diagParamMapItem("DD") == "DETAILS";
  DiagManager diag(shoppingTrx(), Diagnostic910);
  std::vector<int> sopsBuf;
  // Reset failed flight count when try new fare path
  _fltCombTried = 0;
  _doneHurryWithCond = false;
  // allocate upto MAX_PAX_COUNT (4) passenger types 10 legs
  uint failedPsgrFareCnt[MAX_PAX_COUNT][10] = {{0}};
  int psgrIndex = 0;
  int failedFareIndex = 0;

  if (extraFarePath)
  {
    stopat = getFlightMatrix().size() + 1;
  }

  std::set<std::vector<int>>::const_iterator i = sopSet.begin();

  if (_searchBeyondActivated || _customSolutionSearchActivated)
  {
    // No need to add extra options to FlightMatrix folowing standard logic
    // let lookForMoreSolutionsBeyond method to drive the process
    stopat = getFlightMatrix().size();

    // In case we are analyzing already processed GFP (1st phase of thee SB logic)
    // then there is no need to check existing sops in the set because they are
    // already processed - added to flight matrix or dropped for some reason
    if (extraFarePath)
      i = sopSet.end();
  }

  for (; needsMoreSolutions(stopat) && // lint !e574
             (!checkProcessDoneWithCond()) &&
             (i != sopSet.end() || info.front()->findNewPassedSops(_trx, &sopsBuf));
       i != sopSet.end() ? ++i : i)
  {
    if (checkHurryCond())
    {
      setProcessDoneWithCond();
      break;
    }

    const std::vector<int>& sops = i != sopSet.end() ? *i : sopsBuf;

    if (getFlightMatrix().count(sops) != 0)
    {
      continue;
    }

    if (_getMoreEstimatedSolutions && getEstimateMatrix().count(sops) != 0)
    {
      continue;
    }

    if (UNLIKELY(_MultiAirportAgent))
      if (!_MultiAirportAgent->isAirportSolution(sops))
        continue;

    if (UNLIKELY(showDiag910Details))
    {
      printSopCombination(diag.collector(), sops);
    }

    psgrIndex = 0;
    bool passed = true;

    for (std::vector<FarePathFlightsInfo*>::iterator j = info.begin() + 1; j != info.end(); ++j)
    {
      const TSELatencyData metrics(shoppingTrx(), "PO SHOP PQ CHECK MULTI");

      if (LIKELY((*j)->sopsPass(sops) == false))
      {
        if (UNLIKELY(showDiag910Details))
        {
          const std::size_t index = j - info.begin();
          diag << " Invalid for '"
               << groupFPath->groupFPPQItem()[index]->farePath()->paxType()->paxType() << "'";
        }

        failedFareIndex = 0;
        const DatePair* datesPtr = nullptr;
        DatePair datePair;

        if (shoppingTrx().isAltDates())
        {
          datePair = (ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), sops));
          datesPtr = &datePair;
        }

        psgrIndex = 0;

        for (std::vector<FarePathFlightsInfo*>::iterator j = info.begin() + 1; j != info.end();
             ++j, ++psgrIndex)
        {
          FarePath* farePath = groupFPath->groupFPPQItem()[j - info.begin()]->farePath();
          failedFareIndex = 0;
          uint puIdx = 0;

          for (const auto pricingUnit : farePath->pricingUnit())
          {
            for (const auto fareUsage : pricingUnit->fareUsage())
            {
              if (UNLIKELY(fareUsage->isFailedFound()))
              {
                ++failedPsgrFareCnt[psgrIndex][failedFareIndex];
                fareUsage->resetFailedFound();

                if (_maxFailedCellsToValidate < failedPsgrFareCnt[psgrIndex][failedFareIndex])
                {
                  PaxTypeFare* ptf = fareUsage->paxTypeFare();
                  (*j)->saveFailedFare(puIdx, ptf, datesPtr);
                }
              }
              ++failedFareIndex;
            }
            ++puIdx;
          }
        }

        passed = false;
        break;
      }
    }

    if (UNLIKELY(showDiag910Details))
    {
      diag << "\n";
    }

    if (passed == false)
    {
      continue;
    }

    const ShoppingTrx::FlightMatrix::value_type item(sops, duplicateGroupFarePath(groupFPath));

    for (const auto fppqItem : item.second->groupFPPQItem())
    {
      setFareUsageTravelSegs(sops, *fppqItem->farePath(), shoppingTrx());
    }

    const PricingUnit* pu =
        (*(*(groupFPath->groupFPPQItem().begin()))->farePath()->pricingUnit().begin());
    const FareMarket* fMarket = (*(pu->fareUsage().begin()))->paxTypeFare()->fareMarket();
    AltDateOptionInfo optInfo;
    optInfo.cxrCode = fMarket->governingCarrier();
    optInfo.lowestAmt = (*(groupFPath->groupFPPQItem().begin()))->farePath()->getTotalNUCAmount();
    _cxrOptionPerDatesMap.insert(std::pair<DatePair, AltDateOptionInfo>(
        ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), sops), optInfo));
    std::string key = getSopsCnxPoints(shoppingTrx(), sops);

    if (_searchBeyondActivated)
    {
      LOG4CXX_DEBUG(logger, "Process Flights Info for SearchBeyond logic for carrier " + *_carrier);

      // We need to be sure that this is not the solutions with cnx points we already analyzed and
      // not the solutions
      // generated for SB logic lately (during last call for more sops)
      if (_cPoints.count(key) != 0)
      {
        if (showDiag910Details)
        {
          diag << "  Skipped due to Search Beyond logic\n";
        }

        // Solution with this connection point already processed
        // Skip it as we do not want to process solution with the same cnx points twice
        // for SearchBeyond logic
        continue;
      }
    }

    if (_customSolutionSearchActivated)
    {
      if (needMoreCustomSolutions())
      {
        if (_carrier)
        {
          LOG4CXX_DEBUG(logger,
                        "Process Flights Info for Custom Solution logic for carrier " + *_carrier);
        }
        else
        {
          LOG4CXX_DEBUG(logger,
                        "Process Flights Info for Custom Solution logic for INTERLINE carrier");
        }

        // add only if it's a custom solution
        if (!ShoppingUtil::isCustomSolution(shoppingTrx(), item.first))
        {
          if (showDiag910Details)
          {
            diag << "  Skipped due to Custom Solution logic\n";
          }

          continue;
        }
        else
        {
          if (showDiag910Details)
          {
            diag << "  Additional Custom Solution candidate\n";
          }
        }
      }
      else
      {
        // We don't need anymore custom solution
        return;
      }
    }

    if (_foundHighestFarePath)
    {
      const ShoppingTrx::FlightMatrix::value_type itemNoFP(sops, nullptr);
      addToFlightMatrix(itemNoFP);
    }
    else
    {
      FarePath& farePath = *(*(groupFPath->groupFPPQItem().begin()))->farePath();
      farePath.itin()->setEstimatedTax(_taxAmount);

      if (addToFlightMatrix(item, groupFPath) && (_cPoints.count(key) == 0))
        _cPoints.insert(key);
    }

    // Reset failed count when found a good flight options
    _fltCombTried = 0;
    _farePathTried = 0;

    if (shoppingTrx().isAltDates())
    {
      if (_altDateHighestAmountAllow == 0)
      {
        CurrencyCode farePathCurrency =
            (*(groupFPath->groupFPPQItem().begin()))->farePath()->itin()->calculationCurrency();

        if ((farePathCurrency != NUC) && (farePathCurrency != USD))
        {
          CurrencyConversionFacade ccFacade;
          Money fareCurrency(optInfo.lowestAmt, farePathCurrency);
          Money nuc(NUC);

          if (ccFacade.convert(nuc, fareCurrency, shoppingTrx()))
          {
            optInfo.lowestAmt = nuc.value();
          }
        }

        if (optInfo.lowestAmt > _altDateCutoffNucThreshold)
        {
          _altDateHighestAmountAllow = (optInfo.lowestAmt + 1) * _altDateItinPriceJumpFactor;
          _altDateHighestAmountAllow += _taxAmount;
        }
      }

      const DatePair& dates = ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), sops);
      MoneyAmount adjustedFactor = optInfo.lowestAmt / _altDateFareAmountWeightFactor;
      MoneyAmount newPriceJumpFactor = ppo()->altDateCutOffAmount() - adjustedFactor;
      MoneyAmount newPriceJumpFactorSnowMan = ppo()->altDateCutOffSnowmanAmount() - adjustedFactor;

      if (newPriceJumpFactor < _altDateMinimumWeightFactor)
      {
        newPriceJumpFactor = _altDateMinimumWeightFactor;
      }

      if (newPriceJumpFactorSnowMan < _altDateMinimumWeightFactor)
      {
        newPriceJumpFactorSnowMan = _altDateMinimumWeightFactor;
      }

      // This block of code will be guarded for read/write by multi-thread
      {
        boost::lock_guard<boost::mutex> guard(shoppingTrx().altDateLowestAmountMutex());
        ShoppingTrx::AltDateLowestAmount::iterator itr =
            shoppingTrx().altDateLowestAmount().find(dates);

        if (itr != shoppingTrx().altDateLowestAmount().end())
        {
          MoneyAmount calculatedAmount = (newPriceJumpFactor * optInfo.lowestAmt) + _taxAmount;

          if (itr->second->lowestOptionAmount > calculatedAmount)
          {
            itr->second->lowestOptionAmount = calculatedAmount;
          }

          MoneyAmount calculatedAmountForSnowman =
              (newPriceJumpFactorSnowMan * optInfo.lowestAmt) + _taxAmount;

          if (itr->second->lowestOptionAmountForSnowman > calculatedAmountForSnowman)
          {
            itr->second->lowestOptionAmountForSnowman = calculatedAmountForSnowman;
          }
        }
      }
      ShoppingTrx::AltDatePairs::iterator itor = _altDatePairsPQ.find(dates);

      if (itor != _altDatePairsPQ.end())
      {
        itor->second->numOfSolutionNeeded--;

        if (itor->second->numOfSolutionNeeded == 0)
        {
          break;
        }
      }
    }

    if (_MultiAirportAgent)
    {
      findEstimatedSops(item.first, item.second);
    }

    if (_getMoreEstimatedSolutions && !_searchBeyondActivated && !_customSolutionSearchActivated &&
        !_MultiAirportAgent)
    {
      const size_t oldEstimates = getEstimateMatrix().size();
      findEstimatedSops(item.first, item.second);
      const size_t estimatesFound = getEstimateMatrix().size() - oldEstimates;

      if ((estimatesFound < _minimumFamilySize) && (false == directFlightSolution(item.first)))
      {
        if (_allowToRemoveCustomFamily ||
            _customFamilyHeads.find(item.first) == _customFamilyHeads.end())
        {
          _badEstimates.insert(item.first);
          _numBadEstimates += 1 + estimatesFound;
        }
      }
    }
  }

  if ((interlineDiver) && (getFlightMatrix().size() == stopat) && (!_customSolutionSearchActivated))
  {
    GroupFarePath* gfp = shoppingTrx().dataHandle().create<GroupFarePath>();
    gfp->setSourcePqName(getSourceName());
    gfp->setTotalNUCAmount(1000000);
    generateSolutionsWithNoFares(gfp);
  }
}

bool
ShoppingPQ::directFlightSolution(const std::vector<int>& sops)
{
  for (uint32_t leg = 0; leg != sops.size(); ++leg)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];

    if (sop.itin()->travelSeg().size() > 1)
    {
      return false;
    }
  }

  return true;
}

bool
ShoppingPQ::sopsInterline(const std::vector<int>& flightMatrixPos) const
{
  ItinIndex::Key cxr = INTERLINE;

  for (uint32_t n = 0; n != flightMatrixPos.size(); ++n)
  {
    const ItinIndex::Key newCxr = _sopCarriers[n][flightMatrixPos[n]];

    if (newCxr == INTERLINE || (cxr != INTERLINE && newCxr != cxr))
    {
      return true;
    }
    else
    {
      cxr = newCxr;
    }
  }

  return false;
}
CarrierCode*
ShoppingPQ::sopsOnline(const std::vector<int>& flightMatrixPos)
{
  CarrierCode* carrier = &(shoppingTrx().legs()[0].sop()[flightMatrixPos[0]].governingCarrier());

  for (uint32_t n = 1; n != flightMatrixPos.size(); ++n)
  {
    if (*carrier != shoppingTrx().legs()[n].sop()[flightMatrixPos[n]].governingCarrier())
    {
      return nullptr; // interline option
    }
  }

  return carrier;
}

bool
ShoppingPQ::validateNonStopSops(const std::vector<int>& sops)
{
  if (!ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), sops, _trx->legs()))
    return false;

  if (_trx->onlineSolutionsOnly())
  {
    if (_trx->noDiversity() && sopsInterline(sops))
      return false;
    if (!_trx->noDiversity() && !sopsOnline(sops))
      return false;
  }

  if (_trx->interlineSolutionsOnly() && !sopsInterline(sops))
    return false;

  if (isSopinTheMatrix(sops, nullptr)) // interline queue
    return false;

  return validateInterlineTicketCarrierAgreement(sops);
}

bool
ShoppingPQ::validateInterlineTicketCarrierAgreement(const std::vector<int>& sops)
{
  if (LIKELY(_trx->isValidatingCxrGsaApplicable()))
    return _validatingCarrier.processSops(*_trx, sops);

  if ((shoppingTrx().getRequest()->processVITAData() == false) ||
      (shoppingTrx().getOptions()->validateTicketingAgreement() == false))
  {
    return true;
  }

  bool result = _vitaValidator(sops);
  return result;
}

bool
ShoppingPQ::isValidMatrixCell(const std::vector<int>& flightMatrixPos)
{
  if (getFlightMatrix().count(flightMatrixPos) != 0 ||
      getEstimateMatrix().count(flightMatrixPos) != 0)
  {
    return false;
  }

  if (isInterline())
  {
    CarrierCode* carrier = sopsOnline(flightMatrixPos);
    if (carrier && isSopinTheMatrix(flightMatrixPos, carrier))
      return false;
  }

  if (UNLIKELY(shoppingTrx().excTrxType() == PricingTrx::EXC_IS_TRX))
  {
    if (!ShoppingUtil::checkForcedConnection(flightMatrixPos, shoppingTrx()))
    {
      return false;
    }
  }

  if (UNLIKELY(shoppingTrx().getTrxType() == PricingTrx::IS_TRX &&
               shoppingTrx().getRequest()->cxrOverride() != BLANK_CODE))
  {
    if (!ShoppingUtil::checkOverridedSegment(flightMatrixPos, shoppingTrx()))
    {
      return false;
    }
  }

  if (UNLIKELY(skipSpecialSolution(flightMatrixPos)))
    return false;

  // if this is alternate dates, check that we still want
  // options for this date.
  if (shoppingTrx().isAltDates())
  {
    const DatePair& dates = ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), flightMatrixPos);
    ShoppingTrx::AltDatePairs::const_iterator itor = shoppingTrx().altDatePairs().find(dates);

    if (itor == shoppingTrx().altDatePairs().end() || itor->second->numOfSolutionNeeded == 0)
    {
      return false;
    }
  }

  if (UNLIKELY(isNonStopShoppingQueue()))
  {
    return validateNonStopSops(flightMatrixPos);
  }

  if (UNLIKELY(shoppingTrx().noDiversity() && shoppingTrx().onlineSolutionsOnly()))
  {
    // this is a single merged queue, but interline solutions
    // are not allowed, so check that
    if (sopsInterline(flightMatrixPos))
    {
      return false;
    }
  }

  if (UNLIKELY(shoppingTrx().noDiversity() && !shoppingTrx().interlineSolutionsOnly()))
  {
    // this is a single merged queue, and all cells are
    // processed unconditionally
    if (!ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), flightMatrixPos, _trx->legs()))
      return false;
  }
  else
  {
    if (!ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), flightMatrixPos, _trx->legs()))
      return false;

    if (isInterline())
    {
      // In non-stop queue we accept online solutions too
      if (!sopsInterline(flightMatrixPos))
      {
        if (shoppingTrx().isLngCnxProcessingEnabled())
        {
          if (!hasLngCnxSop(flightMatrixPos))
            return false;
        }
        else
        {
          // Do not allow to discard any online option
          // if LngCnxProcessing is disabled and
          // we deal with an IBF transaction
          if (LIKELY(!shoppingTrx().getRequest()->isParityBrandsPath() &&
                     !shoppingTrx().diversity().isExchangeForAirlines()))
          {
            return false;
          }
        }
      }
    }
    else
    {
      // in an online PQ, we process interline flight option also
      if (shoppingTrx().isLngCnxProcessingEnabled())
      {
        if (!_lngCnxAllowedInOnlineQ && hasLngCnxSop(flightMatrixPos))
          return false;
      }

      for (uint32_t n = 0; n != flightMatrixPos.size(); ++n)
      {
        if (shoppingTrx().legs()[n].sop()[flightMatrixPos[n]].governingCarrier() != *_carrier)
        {
          return false;
        }
      }
      if (shoppingTrx().diversity().hasDCL() && shoppingTrx().diversity().isOCO())
      {
        auto& onlineV2Map = getOnlineV2Map();
        const auto& DCLMap = shoppingTrx().diversity().getDCLMap();
        const auto& mapEnd = onlineV2Map.end();
        const auto& mapDCLEnd = DCLMap.end();

        bool isCarrierInDCLMap = false;
        bool stillSearching = false;
        size_t optionFoundSoFar = 0;
        size_t optionRequest = 0;

        const auto& carrierPairInDCL = DCLMap.find(*_carrier);
        if (carrierPairInDCL != mapDCLEnd)
        {     
          optionRequest = carrierPairInDCL->second;
          if (onlineV2Map.find(*_carrier) == onlineV2Map.end())
          onlineV2Map[*_carrier] = 0;
        }

        const auto& carrierPair = onlineV2Map.find(*_carrier);
        if (carrierPair != mapEnd)
        {
          optionFoundSoFar = carrierPair->second;

          isCarrierInDCLMap = true;
          stillSearching = optionFoundSoFar < optionRequest;
        }

        if (isCarrierInDCLMap && stillSearching &&
           (!ShoppingUtil::isOnlineFlightSolution(shoppingTrx(), flightMatrixPos)))
          return false;
        else
        { 
          ++optionFoundSoFar;
          setOnlineCarrierListV2(*_carrier, optionFoundSoFar);
        }
      }
    }
  }

  // check Validate Ticketing Agreement if VTI indicator is set in the request
  return validateInterlineTicketCarrierAgreement(flightMatrixPos);
}

void
ShoppingPQ::propagateError()
{
  if (_error.code() != ErrorResponseException::NO_ERROR)
  {
    throw _error;
  }
}

DiagCollector&
ShoppingPQ::diag()
{
  if (_diag == nullptr)
  {
    DCFactory* factory = DCFactory::instance();
    _diag = factory->create(shoppingTrx());
    _diag->enable(Diagnostic603, Diagnostic601, Diagnostic605, Diagnostic606, Diagnostic661);
    _diag->printHeader();
  }

  return *_diag;
}

void
ShoppingPQ::printSopCombination(DiagCollector& diag, const std::vector<int>& sops)
{
  Diag910Collector* dc910 = dynamic_cast<Diag910Collector*>(&diag);
  TSE_ASSERT(dc910 != nullptr);
  (*dc910) << dc910->sopsToStr(*_trx, sops, false);
}

bool
ShoppingPQ::isFarePathValid(FarePath& farePath, const std::vector<int>& sopIndices)
{
  // Interline queue option will need to check whethere
  // the option is already exist in the online queue option
  if (isInterline())
  {
    CarrierCode* cxr = sameCxrFarePath(farePath);

    if (UNLIKELY(cxr && isSopinTheMatrix(sopIndices, cxr)))
    {
      return false;
    }
  }
  // online Queue option will need to check whether the option
  // is already exist in the interline queue or not. This need
  // to be done after the 1st loop of all online queue when it
  // is not in multi thread mode
  else if (!shoppingTrx().isOnlineShoppingPQProcessing() && sopsInterline(sopIndices))
  {
    if (isSopinTheMatrix(sopIndices, nullptr))
    {
      return false;
    }
  }

  // first see if we have already validated this sop/fare path
  // combination, and it's in our cache
  FarePath* baseFarePath = getBaseFarePath(&farePath);
  const SopFarePathPair cacheKey(sopIndices, baseFarePath);
  std::map<SopFarePathPair, bool>::const_iterator cacheItor = _validationResultCache.find(cacheKey);

  if (cacheItor != _validationResultCache.end())
  {
    return cacheItor->second;
  }

  bool& cacheResult = _validationResultCache[cacheKey];
  cacheResult = false;

  // Alternate Date highest amount allow
  if (UNLIKELY(_foundHighestFarePath))
  {
    return true;
  }

  // Keep consecutive count for how many flight combination tried and keep failing
  ++_fltCombTried;
  // a vector which will contain one item for each FareUsage. We will
  // collect the travel segs from the FareUsages and put it in this vector,
  // and then when we are done we will sort it in the order the travel segs
  // actually occur in the journey, rather than the order in which FareUsage
  // objects appear, and store it in the Itin.
  std::vector<FareUsageTravelSegs> travelSegPath;
  std::deque<std::vector<TravelSeg*>> backupTravelSegs;
  typedef Swapper<std::vector<TravelSeg*>> TravelSegSwapper;
  std::deque<TravelSegSwapper> travelSegSwappers;
  std::map<FareMarket*, std::vector<TravelSeg*>> fareMarketSegsMap;

  // swap the travel segments around to prepare for rule validation
  for (const auto pricingUnit : farePath.pricingUnit())
  {
    backupTravelSegs.push_back(std::vector<TravelSeg*>());
    travelSegSwappers.push_back(
        TravelSegSwapper(backupTravelSegs.back(), pricingUnit->travelSeg()));

    for (const auto fareUsage : (*pricingUnit).fareUsage())
    {
      FareMarket& fMarket = *(*fareUsage).paxTypeFare()->fareMarket();
      const std::vector<TravelSeg*>& fmTravelSegs = fMarket.travelSeg();
      TSE_ASSERT(fmTravelSegs.empty() == false); // lint !e666
      uint32_t startIndex = 0, endIndex = 0, adoptedIndex = 0;
      const bool res = ShoppingUtil::getLegTravelSegIndices(
          shoppingTrx(), fmTravelSegs, startIndex, endIndex, adoptedIndex);

      if (UNLIKELY(!res))
      {
        throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                     "ShoppingUtil::getLegTravelSegIndices returned false");
      }

      std::vector<TravelSeg*>& travelSegs = fareMarketSegsMap[&fMarket];
      // FareUsage will be modified permanently
      fareUsage->travelSeg().clear();
      const bool updateGD = (startIndex + 1 == endIndex);

      for (uint32_t index = startIndex; index != endIndex; ++index)
      {
        const ShoppingTrx::Leg& leg = shoppingTrx().legs()[index];
        const int sop = sopIndices[index];

        if (UNLIKELY(sop >= int(leg.sop().size())))
        {
          throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR,
                                       "sop >= leg.sop().size()");
        }

        if (updateGD)
        {
          fMarket.setGlobalDirection(leg.sop()[sop].globalDirection());

}

        travelSegs.insert(travelSegs.end(),
                          leg.sop()[sop].itin()->travelSeg().begin(),
                          leg.sop()[sop].itin()->travelSeg().end());
        std::vector<TravelSeg*>& fuTravelSegs = fareUsage->travelSeg();
        fuTravelSegs.insert(fuTravelSegs.end(),
                            leg.sop()[sop].itin()->travelSeg().begin(),
                            leg.sop()[sop].itin()->travelSeg().end());
        std::vector<TravelSeg*>& puTravelSegs = pricingUnit->travelSeg();
        puTravelSegs.insert(puTravelSegs.end(),
                            leg.sop()[sop].itin()->travelSeg().begin(),
                            leg.sop()[sop].itin()->travelSeg().end());
      }

      TSE_ASSERT(travelSegs.empty() == false); // lint !e666
      travelSegPath.push_back(FareUsageTravelSegs(startIndex, fareUsage->travelSeg()));
    }
  }

  if (UNLIKELY(shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
               shoppingTrx().diagnostic().diagParamMapItem("DD") == "IGNOREREVAL"))
  {
    cacheResult = true;
    return true;
  }

  // make a copy of the itin, since we have to modify it, and we
  // don't want to interfere with any other threads.
  Itin* const farePathItin = shoppingTrx().dataHandle().create<Itin>();
  *farePathItin = *farePath.itin();
  farePath.itin() = farePathItin;
  std::vector<TravelSeg*> backupItinTravelSegs;
  const TravelSegSwapper farePathItinSwapper(farePath.itin()->travelSeg(), backupItinTravelSegs);
  // sort the travel segments so now they are in the order they actually
  // occur in, rather than the order the fare usages appear in, and
  // populate the itin with them.
  std::sort(travelSegPath.begin(), travelSegPath.end());

  for (std::vector<FareUsageTravelSegs>::const_iterator i = travelSegPath.begin();
       i != travelSegPath.end();
       ++i)
  {
    farePath.itin()->travelSeg().insert(
        farePath.itin()->travelSeg().end(), i->segs().begin(), i->segs().end());
  }

  TravelSegAnalysis tvlSegAnalysis;
  Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(farePathItin->travelSeg());
  ItinUtil::setGeoTravelType(tvlSegAnalysis, tvlBoundary, *farePathItin);
  _validatingCarrier.update(*farePathItin, false, &sopIndices);

  // --- Rule validation per PU of the FarePath ----
  TSELatencyData metrics(shoppingTrx(), "PO RULE VALIDATE");
  if (UNLIKELY(shoppingTrx().diagnostic().diagnosticType() == Diagnostic555) &&
      !shoppingTrx().diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN))
  {
    DCFactory* factory = DCFactory::instance();
    // lint -e{578}
    DiagCollector& diag = *(factory->create(shoppingTrx()));
    diag.enable(Diagnostic555);
    diag << "****************************************************************" << std::endl;
    diag << "           PRICING UNIT/FARE USAGE RULE VALIDATION DIAGNOSTICS" << std::endl;
    diag << " " << std::endl;
    diag << "FARE PATH " << farePath;
    diag << " " << std::endl;
    diag.flushMsg();
  }

  bool record8SameCxrFound = false;
  std::deque<TravelSegSwapper> fmTravelSegSwappers;

  for (const auto pricingUnit : farePath.pricingUnit())
  {
    // make a list of the fare markets that are going to be used
    // to validate this pricing unit
    std::vector<FareMarket*> fareMarkets;

    for (const auto fareUsage : (*pricingUnit).fareUsage())
    {
      FareMarket& fMarket = *(*fareUsage).paxTypeFare()->fareMarket();
      fareMarkets.push_back(&fMarket);
    }

    // get rid of any duplicate fare markets
    std::sort(fareMarkets.begin(), fareMarkets.end());
    fareMarkets.erase(std::unique(fareMarkets.begin(), fareMarkets.end()), fareMarkets.end());

    for (const auto faremarket : fareMarkets)
    {
      // now swap the travel segs with what we've previously determined
      // to be its intended travel segs
      const std::map<FareMarket*, std::vector<TravelSeg*>>::iterator itor =
          fareMarketSegsMap.find(faremarket);
      TSE_ASSERT(itor != fareMarketSegsMap.end());
      fmTravelSegSwappers.push_back(TravelSegSwapper(itor->second, faremarket->travelSeg()));
    }

    std::set<int> gdSet;
    bool globalDirectionSwapped = false;
    int position = 0;

    // validate Record 8 same carrier/Table 986
    for (const auto fareUsage : (*pricingUnit).fareUsage())
    {
      const PaxTypeFare& fare = *fareUsage->paxTypeFare();
      FareMarket& fMarket = *(*fareUsage).paxTypeFare()->fareMarket();
      GlobalDirection globalDirection = fMarket.getGlobalDirection();

      if (globalDirection == GlobalDirection::ZZ)
      {
        globalDirectionSwapped = true;
        GlobalDirectionFinderV2Adapter::getGlobalDirection(
            _trx, fMarket.travelDate(), fMarket.travelSeg(), globalDirection);
        if (LIKELY(globalDirection != GlobalDirection::ZZ))
        {
          fMarket.setGlobalDirection(globalDirection);
          gdSet.insert(position);
        }
      }
      position++;

      if (fare.isFareByRule())
      {
        const FareByRuleApp& fbrApp = fare.fbrApp();

        if (UNLIKELY(fbrApp.sameCarrier() != BLANK))
        {
          if (!record8SameCxrFound)
          {
            record8SameCxrFound = true;

            if (!RuleUtil::useSameCarrier(farePath.itin()->travelSeg()))
            {
              return false;
            }
          }
        }
        else if (UNLIKELY(fbrApp.carrierFltTblItemNo() != 0))
        {
          if (!RuleUtil::useCxrInCxrFltTbl(farePath.itin()->travelSeg(),
                                           fbrApp.vendor(),
                                           fbrApp.carrierFltTblItemNo(),
                                           shoppingTrx().ticketingDate()))
          {
            return false;
          }
        }
      }

      // check cat35 validating carrier
      const PaxTypeFareRuleData* ptfRd = fare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);

      if (ptfRd != nullptr)
      {
        if (checkValidatingCarrierForCat35(*ptfRd, farePath) == false)
        {
          return false;
        }
      }
    }

    if (UNLIKELY(shoppingTrx().isAltDates() &&
                 !shoppingTrx().getOptions()->isEnableCalendarForInterlines() &&
                 checkIfSimilarOption(sopIndices)))
    {
      return false;
    }

    // clear total transfer for each pu
    (*pricingUnit).totalTransfers() = 0;
    (*pricingUnit).mostRestrictiveMaxTransfer() = -1;

    // validate Rule
    bool rcValidate = _ruleController.validate(shoppingTrx(), farePath, *pricingUnit);

    if (globalDirectionSwapped && gdSet.size() > 0) // swap back to original value
    {
      uint16_t i = 0;
      for (const auto fareUsage : (*pricingUnit).fareUsage())
      {
        for (std::set<int>::iterator it = gdSet.begin(); it != gdSet.end(); ++it)
        {
          if (*it == i)
          {
            FareMarket& fMarket = *(*fareUsage).paxTypeFare()->fareMarket();
            fMarket.setGlobalDirection(GlobalDirection::ZZ);
          }
        }
        i++;
      }
    }

    if (!rcValidate)
    {
      return false;
    }
  }

  if (UNLIKELY((_taxForISProcess) && (shoppingTrx().isAltDates()) && (_taxAmount < EPSILON)))
  {
    Itin& curItin = *(farePath.itin());

    if (curItin.validatingCarrier().empty())
    {
      _validatingCarrier.update(farePath);
    }

    _taxAmount = getTax(curItin, &farePath);
  }

  if (UNLIKELY(shoppingTrx().diagnostic().diagnosticType() == Diagnostic555) &&
      !shoppingTrx().diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN))
  {
    DCFactory* factory = DCFactory::instance();
    // lint -e{578}
    DiagCollector& diag = *(factory->create(shoppingTrx()));
    diag.enable(Diagnostic555);
    diag << "\nPASSED RULE-REVALIDATION";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag.setf(std::ios::fixed, std::ios::floatfield);
    diag.precision(2);
    diag << std::setw(8);
    diag << " FARE PATH NEW AMOUNT: " << farePath.getTotalNUCAmount() << '\n';
    diag.flushMsg();
  }

  cacheResult = true;
  return true;
}

MoneyAmount
ShoppingPQ::getTax(Itin& curItin, FarePath* farePath)
{
  MoneyAmount taxAmt = 1; // default to 1 to indicate the tax process once run for this Q

  if (!curItin.farePath().empty())
  {
    return taxAmt;
  }

  curItin.farePath().push_back(farePath);

  if (curItin.getTaxResponses().empty())
  {
    if (farePath->baseFareCurrency().empty() || farePath->calculationCurrency().empty())
    {
      farePath->baseFareCurrency() = curItin.originationCurrency();
      farePath->calculationCurrency() = curItin.calculationCurrency();
    }

    TaxMap::TaxFactoryMap taxFactoryMap;
    TaxMap::buildTaxFactoryMap(_trx->dataHandle(), taxFactoryMap);
    TaxItinerary taxItinerary;
    taxItinerary.initialize(*_trx, curItin, taxFactoryMap);
    taxItinerary.accumulator();
  }

  curItin.farePath().clear();

  if (curItin.getTaxResponses().empty())
  {
    return taxAmt;
  }

  const TaxResponse* taxResponse = curItin.getTaxResponses().front();

  if (taxResponse == nullptr || taxResponse->taxItemVector().empty())
  {
    return taxAmt;
  }

  CurrencyCode taxCurrencyCode;
  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << "GET TAX AMOUNT FOR QUEUE - ";

    if (_carrier)
    {
      diag << *_carrier;
    }
    else
    {
      diag << "INTERLINE";
    }

    diag << " \n";
  }

  for (const auto taxItem : taxResponse->taxItemVector())
  {
    if (taxItem->taxAmount() != 0)
    {
      taxCurrencyCode = taxItem->paymentCurrency(); // assume all taxes have the same currency
      taxAmt += taxItem->taxAmount();
    }

    if (diag.isActive())
    {
      diag << "TAX CODE " << taxItem->taxCode();
      diag << "  " << taxItem->taxAmount() << " \n";
    }
  }

  CurrencyCode farePathCurrency = curItin.calculationCurrency();

  if (!taxCurrencyCode.empty() && farePathCurrency != taxCurrencyCode)
  {
    CurrencyConversionFacade ccFacade;
    Money fareCurrency(farePathCurrency);
    Money taxCurrency(taxAmt, taxCurrencyCode);

    if (ccFacade.convert(fareCurrency, taxCurrency, *_trx))
    {
      taxAmt = fareCurrency.value();
    }
  }

  // --- process cat12 - surcharges ---
  if (_cat12ForAltDates)
  {
    std::vector<uint16_t> categories;
    categories.push_back(RuleConst::SURCHARGE_RULE); // category 12
    RuleControllerWithChancelor<PricingUnitRuleController> rc(FPRuleValidation, categories);
    bool valRes = true;

    for (const auto pricingUnit : farePath->pricingUnit())
    {
      valRes = rc.validate(*_trx, *farePath, *pricingUnit);

      if (!valRes)
      {
        break;
      }
    }

    if (valRes)
    {
      RuleUtil::getSurcharges(*_trx, *farePath);
    }
  }

  return taxAmt;
}

namespace
{
class DirectConnectionPointsCounter
{
public:
  DirectConnectionPointsCounter(const ShoppingTrx& trx) : _trx(&trx) {}

  int operator()(const std::vector<int>& sops) const;

private:
  const ShoppingTrx* _trx;
};

int
DirectConnectionPointsCounter::
operator()(const std::vector<int>& sops) const
{
  int res = 0;
  int sz(static_cast<int>(sops.size()));

  for (int i = 0; i < sz; ++i)
  {
    res += sops[i];
  }

  return res;
}
}

void
ShoppingPQ::generateSolutionsWithNoFares(GroupFarePath* gfp, bool directFltOnly)
{
  if (shoppingTrx().isAltDates() || isOwFaresShoppingQueue())
  {
    return;
  }

  if (_customSolutionSearchActivated)
  {
    generateCustomSolutionsWithNoFares(gfp);
    return;
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << "Generating flight-only solutions for queue ";

    if (_carrier)
    {
      diag << *_carrier;
    }
    else if (isOwFaresShoppingQueue())
    {
      diag << "OW Fares";
    }
    else
    {
      diag << "Interline";
    }

    diag << ":\n";
  }

  std::map<std::string, uint16_t> cxrCntMap;
  std::string tempCxrFirstLeg;
  std::string tempCxrSecondLeg;
  uint32_t leg = 0;

  if ((!_carrier) && (_interlineDiversityPercent != RuleConst::HUNDRED_PERCENTS) &&
      (_trx->legs().size() == 2))
  {
    const ShoppingTrx::FlightMatrix& matrix = _trx->flightMatrix();

    for (const auto& flightMatrixElem : matrix)
    {
      tempCxrFirstLeg = "";
      const std::vector<int>& cell = flightMatrixElem.first;
      std::vector<int>::const_iterator j = cell.begin();

      for (leg = 0; j == cell.begin(); ++j, ++leg)
      {
        const Itin* itin = _trx->legs()[leg].sop()[size_t(*j)].itin();
        TSE_ASSERT(itin);

        if (itin->travelSeg().size() < 1)
        {
          break;
        }

        for (const auto travelSeg : itin->travelSeg())
        {
          const AirSeg* const airSeg = travelSeg->toAirSeg();

          if (airSeg == nullptr)
          {
            continue;
          }

          tempCxrFirstLeg = tempCxrFirstLeg + airSeg->marketingCarrierCode();
        }

        std::map<std::string, uint16_t>::iterator cxrI = cxrCntMap.find(tempCxrFirstLeg);

        if (cxrI != cxrCntMap.end())
        {
          ++cxrI->second;
        }
        else
        {
          cxrCntMap.insert(std::make_pair(tempCxrFirstLeg, 1));
        }
      }
    }
  }

  std::vector<std::vector<int>> sopsSaved;
  uint32_t failedMatrixCnt = 0;
  std::vector<int> sops;
  FltOnlyCombinationIterator iter(*_trx, _flightMatrixDim);
  uint64_t trialCnt = 0, abortCheckInterval = TrxUtil::abortCheckInterval(*_trx);

  while ((lookForMoreSolutions(getNoOfOptionsRequested()) || lookForMoreCustomSolutions()) &&
         iter.next(sops))
  {
    if (UNLIKELY(++trialCnt == abortCheckInterval))
    {
      try
      {
        if (fallback::reworkTrxAborter(_trx))
          checkTrxAborted(*_trx);
        else
          _trx->checkTrxAborted();
      }
      catch (ErrorResponseException& ex)
      {
        return;
      }
      trialCnt = 0;
    }

    ++failedMatrixCnt;

    if (failedMatrixCnt > _maxFailedCellsForFltOnlySolution)
    {
      break;
    }

    if (directFltOnly)
    {
      uint16_t numberOfDirect = 0;

      for (leg = 0; leg != sops.size(); ++leg)
      {
        const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];

        if (sop.itin()->travelSeg().size() == 1)
        {
          ++numberOfDirect;
        }
      }

      if (numberOfDirect == 0)
      {
        return;
      }

      if (numberOfDirect < sops.size())
      {
        continue;
      }
    }

    if (UNLIKELY(isSolutionProducedByInterlineQueue(sops)))
      continue;

    if (isValidMatrixCell(sops) && ShoppingUtil::isCabinClassValid(*_trx, sops) &&
        isValidCxrRestriction(sops))
    {
      failedMatrixCnt = 0;

      if ((!_carrier) && (_interlineDiversityPercent != RuleConst::HUNDRED_PERCENTS) &&
          (_trx->legs().size() == 2))
      {
        tempCxrFirstLeg = "";
        tempCxrSecondLeg = "";

        for (leg = 0; leg != sops.size(); ++leg)
        {
          const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];
          const std::vector<TravelSeg*>& segs = sop.itin()->travelSeg();

          for (const auto travelSeg : segs)
          {
            const AirSeg* const airSeg = travelSeg->toAirSeg();

            if (UNLIKELY(airSeg == nullptr))
            {
              continue;
            }

            if (leg == 0)
              tempCxrFirstLeg = tempCxrFirstLeg + airSeg->marketingCarrierCode();
            else
              tempCxrSecondLeg = airSeg->marketingCarrierCode() + tempCxrSecondLeg;
          }
        }

        std::map<std::string, uint16_t>::iterator cxrI = cxrCntMap.find(tempCxrFirstLeg);

        if (cxrI != cxrCntMap.end())
        {
          ++cxrI->second;

          if (cxrI->second < 100)
          {
            continue;
          }
        }
        else
        {
          // if it's not mirror image - snow man trip then skip it
          if (tempCxrFirstLeg != tempCxrSecondLeg)
          {
            if (sopsSaved.size() < getNoOfOptionsRequested())
            {
              sopsSaved.push_back(sops);
            }

            continue;
          }

          cxrCntMap.insert(std::make_pair(tempCxrFirstLeg, 1));
        }
      }

      if (!_carrier && getFlightMatrix().size() < _numFPAndMustPriceSnowManFltOnlyInterlineOption)
      {
        const ShoppingTrx::FlightMatrix::value_type item(sops, nullptr);
        addToFlightMatrix(item);
      }
      else
      {
        const ShoppingTrx::FlightMatrix::value_type item(sops, gfp);
        addToFlightMatrix(item);
      }

      if (diag.isActive())
      {
        diag << "  (";

        for (size_t n = 0; n != sops.size(); ++n)
        {
          if (n != 0)
          {
            diag << ", ";
          }

          diag << ShoppingUtil::findSopId(shoppingTrx(), n, sops[n]);
        }

        diag << ")\n";
      }
    }
  } // end of loop

  if ((getNoOfOptionsRequested() > getFlightMatrix().size()) && (sopsSaved.size() > 0))
  {
    size_t i = getNoOfOptionsRequested() - getFlightMatrix().size();

    if (sopsSaved.size() < i)
    {
      i = sopsSaved.size();
    }

    for (size_t j = 0; j < i; j++)
    {
      const ShoppingTrx::FlightMatrix::value_type item(sopsSaved[j], gfp);
      addToFlightMatrix(item);
    }
  }
}

bool
ShoppingPQ::generateSnowManSolutionsWithNoFares(GroupFarePath* gfp)
{
  if (shoppingTrx().isAltDates() || !_carrier)
  {
    return false;
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << "Generating flight-only snow-man solutions for queue ";

    if (_carrier)
    {
      diag << *_carrier;
    }
    else if (isOwFaresShoppingQueue())
    {
      diag << "OW Fares";
    }
    else
    {
      diag << "Interline";
    }

    diag << ":\n";
  }

  std::string tempAirportFirstLeg;
  std::string tempAirportSecondLeg;
  std::string tempCxrFirstLeg;
  std::string tempCxrSecondLeg;
  uint32_t leg = 0;
  std::vector<std::vector<int>> sopsSaved;
  uint32_t failedMatrixCnt = 0;

  for (MatrixRatingIterator<DirectConnectionPointsCounter> itor(
           _flightMatrixDim, DirectConnectionPointsCounter(*_trx));
       !itor.atEnd() && lookForMoreSolutions(getNoOfOptionsRequested());
       itor.next())
  {
    const std::vector<int> sops(itor.value());
    ++failedMatrixCnt;

    if (failedMatrixCnt > _maxFailedCellsForFltOnlySolution)
    {
      break;
    }

    bool carrierMatch = true;
    bool flightOnlySolution = true;
    bool needDirectFlt = false;

    for (leg = 0; leg != sops.size(); ++leg)
    {
      const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];

      if ((sop.itin()->travelSeg().size() != 2) ||
          !ShoppingUtil::isOnlineConnectionFlight(
              sops[leg], leg, _carrier, _trx, needDirectFlt, flightOnlySolution))
      {
        carrierMatch = false;
        break;
      }
    }

    if (!carrierMatch)
    {
      continue;
    }

    if (isSolutionProducedByInterlineQueue(sops))
      continue;

    if (isValidMatrixCell(sops) && ShoppingUtil::isCabinClassValid(*_trx, sops) &&
        isValidCxrRestriction(sops))
    {
      failedMatrixCnt = 0;

      if (_trx->legs().size() == 2)
      {
        tempAirportFirstLeg = "";
        tempAirportSecondLeg = "";
        tempCxrFirstLeg = "";
        tempCxrSecondLeg = "";

        for (leg = 0; leg != sops.size(); ++leg)
        {
          const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];
          const std::vector<TravelSeg*>& segs = sop.itin()->travelSeg();

          for (const auto travelSeg : segs)
          {
            const AirSeg* const airSeg = travelSeg->toAirSeg();

            if (UNLIKELY(airSeg == nullptr))
            {
              continue;
            }

            if (leg == 0)
            {
              tempAirportFirstLeg =
                  tempAirportFirstLeg + travelSeg->boardMultiCity() + travelSeg->offMultiCity();
              tempCxrFirstLeg = tempCxrFirstLeg + airSeg->marketingCarrierCode();
            }
            else
            {
              tempAirportSecondLeg =
                  travelSeg->offMultiCity() + travelSeg->boardMultiCity() + tempAirportSecondLeg;
              tempCxrSecondLeg = airSeg->marketingCarrierCode() + tempCxrSecondLeg;
            }
          }
        }

        // if it's snow man trip then save and done
        if (!tempAirportFirstLeg.empty() && (tempCxrFirstLeg == tempCxrSecondLeg) &&
            (tempAirportFirstLeg == tempAirportSecondLeg))
        {
          const ShoppingTrx::FlightMatrix::value_type item(sops, gfp);
          bool addedToFlightMatrix = addToFlightMatrix(item);

          if (addedToFlightMatrix && diag.isActive())
          {
            diag << "  (";

            for (size_t n = 0; n != sops.size(); ++n)
            {
              if (n != 0)
              {
                diag << ", ";
              }

              diag << ShoppingUtil::findSopId(shoppingTrx(), n, sops[n]);
            }

            diag << ")\n";
          }

          return true; // Even if failed to add - then no more work with this GFP
        }
      }
    }
  }

  return false;
}

void
ShoppingPQ::generateCNXSolutionsWithNoFares(GroupFarePath* gfp,
                                            const bool needDirectFlt,
                                            const bool sameCxrReq)
{
  if (shoppingTrx().isAltDates())
  {
    return;
  }

  if (!sameCxrReq && generateSnowManSolutionsWithNoFares(gfp))
  {
    return;
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << "Generating flight-only solutions for queue ";
    diag << *_carrier;
    diag << ":\n";
  }

  std::vector<int> sopVec;
  bool validSopFound = true;
  uint32_t legIndex = 0;

  for (std::vector<ShoppingTrx::Leg>::const_iterator legIt = _trx->legs().begin();
       legIt != _trx->legs().end() && validSopFound;
       ++legIt, ++legIndex)
  {
    if (legIt->stopOverLegFlag())
    {
      continue;
    }

    validSopFound = false;
    const std::vector<ShoppingTrx::SchedulingOption>& sops = legIt->sop();
    bool flightOnlySolution = false;

    for (uint32_t sopIndex = 0; sopIndex < sops.size(); ++sopIndex)
    {
      if (sops[sopIndex].cabinClassValid() &&
          ShoppingUtil::isOnlineConnectionFlight(
              sopIndex, legIndex, _carrier, _trx, needDirectFlt, flightOnlySolution))
      {
        sopVec.push_back(sopIndex);

        // check minimum connection time if it's not the first leg
        if (!legIndex || (legIndex && ShoppingUtil::checkMinConnectionTime(
                                          _trx->getOptions(), sopVec, _trx->legs())))
        {
          validSopFound = true;
          break; // got a valid sop then go to next leg
        }

        sopVec.pop_back();
      }
    }
  }

  if (!validSopFound)
    return; // no valid sops

  if (isSolutionProducedByInterlineQueue(sopVec))
    return;

  if (!validateInterlineTicketCarrierAgreement(sopVec))
  {
    return;
  }

  if (shoppingTrx().excTrxType() == PricingTrx::EXC_IS_TRX)
  {
    if (!ShoppingUtil::checkForcedConnection(sopVec, shoppingTrx()))
    {
      return;
    }
  }

  if (shoppingTrx().getTrxType() == PricingTrx::IS_TRX &&
      shoppingTrx().getRequest()->cxrOverride() != BLANK_CODE)
  {
    if (!ShoppingUtil::checkOverridedSegment(sopVec, shoppingTrx()))
    {
      return;
    }
  }

  const ShoppingTrx::FlightMatrix::value_type item(sopVec, gfp);
  bool addedToFlightMatrix = addToFlightMatrix(item);

  if (addedToFlightMatrix && diag.isActive())
  {
    diag << "  (";

    for (size_t n = 0; n != sopVec.size(); ++n)
    {
      if (n != 0)
      {
        diag << ", ";
      }

      diag << ShoppingUtil::findSopId(shoppingTrx(), n, sopVec[n]);
    }

    diag << ")\n";
  }

  return; // only need 1 solution
}

bool
ShoppingPQ::isValidCxrRestriction(const std::vector<int>& sops) const
{
  if (!isInterline())
    return true;

  for (uint32_t leg = 0; leg != sops.size(); ++leg)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx->legs()[leg].sop()[sops[leg]];

    if (UNLIKELY(sop.combineSameCxr() == true))
    {
      return false;
    }
  }

  return true;
}

bool
ShoppingPQ::skipSpecialSolution(const std::vector<int>& sops)
{
  if (UNLIKELY(shoppingTrx().getNumOfCustomSolutions() &&
               !shoppingTrx().isLngCnxProcessingEnabled() &&
               !shoppingTrx().isOnlineShoppingPQProcessing()))
  {
    bool isCustom = ShoppingUtil::isCustomSolution(shoppingTrx(), sops);

    if (_customSolutionSearchActivated && !isCustom)
      return true;

    if (isCustom && !needMoreCustomSolutions())
      return true;
  }

  return false;
}

Itin*
ShoppingPQ::getJourneyItinAltDates(const DatePair& d) const
{
  const JourneyItinAltDatesMap::iterator i =
      _journeyItinAltDates.find(ShoppingAltDateUtil::dateOnly(d));

  if (i != _journeyItinAltDates.end())
  {
    return &i->second;
  }
  else
  {
    return nullptr;
  }
}

namespace
{
typedef std::pair<std::vector<int>, GroupFarePath*> ISSolution;
bool
compareSolutions(const ISSolution& sol1, const ISSolution& sol2)
{
  if (sol1.second == nullptr)
  {
    return false;
  }

  if (sol2.second == nullptr)
  {
    return true;
  }

  return sol1.second->getTotalNUCAmount() < sol2.second->getTotalNUCAmount();
}
}

void
ShoppingPQ::generateEstimatedSolutions()
{
  std::vector<ISSolution> sol(getFlightMatrix().begin(), getFlightMatrix().end());
  std::sort(sol.begin(), sol.end(), compareSolutions);

  for (std::vector<ISSolution>::iterator i = sol.begin();
       i != sol.end() &&
           static_cast<int>(getFlightMatrix().size() + getEstimateMatrix().size() -
                            _numBadEstimates) < _nEstimatedOptions;
       ++i)
  {
    if (i->second == nullptr)
    {
      continue;
    }

    if (UNLIKELY(_customSolutionSearchActivated))
    {
      if (!ShoppingUtil::isCustomSolutionGfp(shoppingTrx(), i->second))
      {
        continue;
      }
    }

    findEstimatedSops(i->first, i->second);
  }
}

void
ShoppingPQ::findEstimatedSops(const std::vector<int>& flightMatrixPos, GroupFarePath* path)
{
  if (flightMatrixPos.size() == 2)
  {
    _outboundSopSet.clear();
    _inboundSopSet.clear();
    _outboundSopSet.insert(flightMatrixPos[0]);
    _inboundSopSet.insert(flightMatrixPos[1]);
  }

  path = duplicateGroupFarePath(path);

  if (path->groupFPPQItem().empty())
  {
    return;
  }

  const DatePair* dates = nullptr;
  FarePath* firstPath = path->groupFPPQItem().front()->farePath();
  FareUsageMatrixMap fuMap(&shoppingTrx(), _journeyItin, firstPath, &_bitmapOpOrderer, dates, this);
  std::vector<FareUsageMatrixMap> multiPaxTypeFUMaps;

  for (std::vector<FPPQItem*>::iterator i = path->groupFPPQItem().begin() + 1;
       i != path->groupFPPQItem().end();
       ++i)
  {
    FarePath* farePath = (*i)->farePath();
    multiPaxTypeFUMaps.push_back(
        FareUsageMatrixMap(&shoppingTrx(), _journeyItin, farePath, &_bitmapOpOrderer, dates, this));
  }

  SOPCombinationIterator iter(shoppingTrx(), fuMap);
  std::vector<int> sops;

  while ((_MultiAirportAgent ||
          (static_cast<int>(getFlightMatrix().size() + getEstimateMatrix().size() -
                            _numBadEstimates) < _nEstimatedOptions)) &&
         iter.next(sops))
  {
    bool similar = true;
    TSE_ASSERT(flightMatrixPos.size() == sops.size());

    for (unsigned int leg = 0; leg != sops.size(); ++leg)
    {
      TSE_ASSERT(leg < shoppingTrx().legs().size());

      const ShoppingTrx::SchedulingOption& sop1 = _trx->legs()[leg].sop()[flightMatrixPos[leg]];
      const ShoppingTrx::SchedulingOption& sop2 = _trx->legs()[leg].sop()[sops[leg]];
      bool bothHighTPMAreFalse = (!sop1.isHighTPM() && !sop2.isHighTPM());

      if (!fallback::fallbackFareSelction2016(_trx))
      {
        if (!bothHighTPMAreFalse && !sopsHighTPMSimilarityCheck(sop1, sop2))
        {
          similar = false;
          break;
        }
      }

      if (fallback::fallbackFareSelction2016(_trx) || bothHighTPMAreFalse)
      {
        if (ShoppingUtil::schedulingOptionsSimilar(
                shoppingTrx(),
                shoppingTrx().legs()[leg].sop()[flightMatrixPos[leg]],
                shoppingTrx().legs()[leg].sop()[sops[leg]]) == false)
        {
          similar = false;
          break;
        }
      }
    }

    if (similar == false)
    {
      continue;
    }

    if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(sops))
      if (!checkNumOfLngCnx())
        continue;

    bool passMultiPax = true;

    for (const auto& fareUsageMatrixMap : multiPaxTypeFUMaps)
    {
      if (fareUsageMatrixMap.hasSops(sops) == false)
      {
        passMultiPax = false;
        break;
      }
    }

    if (passMultiPax == false || getFlightMatrix().count(sops) || getEstimateMatrix().count(sops) ||
        !isValidMatrixCell(sops))
    {
      continue;
    }

    bool valid = true;

    for (auto fppqItem : path->groupFPPQItem())
    {
      FarePath& farePath = *fppqItem->farePath();

      if (isFarePathValid(farePath, sops) == false)
      {
        valid = false;
        break;
      }
    }

    if (valid)
    {
      generateEstimateMatrixItem(flightMatrixPos, path, sops);

      if (_combinedFamily && sops.size() == 2)
      {
        generateCombinedEstimateMatrix(flightMatrixPos, path, sops);
      }
    }
  }
}

void
ShoppingPQ::generateEstimateMatrixItem(const std::vector<int>& flightMatrixPos,
                                       GroupFarePath* path,
                                       const std::vector<int>& sops)
{
  GroupFarePath* groupFarePath = duplicateGroupFarePath(path);

  for (auto fppqItem : groupFarePath->groupFPPQItem())
  {
    setFareUsageTravelSegs(sops, *fppqItem->farePath(), shoppingTrx());
  }

  ShoppingTrx::EstimatedSolution estimate(flightMatrixPos, groupFarePath);

  bool hasLngCnxAdded = false;
  if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(sops))
  {
    if (!checkNumOfLngCnx())
      return;

    shoppingTrx().pqDiversifierResult().incrementCurrentLngCnxOptionCount();
    hasLngCnxAdded = true;
  }

  if (processCustomSolutionStats(sops, hasLngCnxAdded))
  {
    estimateMatrix()[sops] = estimate;
    if (_trx->getNumOfCustomSolutions() && !_allowToRemoveCustomFamily)
    {
      if (ShoppingUtil::isCustomSolution(*_trx, sops) &&
          _customFamilyHeads.find(estimate.first) == _customFamilyHeads.end())
      {
        _customFamilyHeads.insert(estimate.first);
      }
    }
  }
}

void
ShoppingPQ::generateCombinedEstimateMatrix(const std::vector<int>& flightMatrixPos,
                                           GroupFarePath* path,
                                           const std::vector<int>& sops)
{
  // find new estimate sop in outboundSopSet
  std::set<int>::iterator sopIt = _outboundSopSet.find(sops[0]);

  if (sopIt == _outboundSopSet.end())
  {
    for (sopIt = _inboundSopSet.begin(); sopIt != _inboundSopSet.end(); sopIt++)
    {
      if (*sopIt == sops[1])
      {
        continue;
      }

      std::vector<int> estimateSop;
      estimateSop.push_back(sops[0]);
      estimateSop.push_back(*sopIt);

      if (getFlightMatrix().count(estimateSop) || getEstimateMatrix().count(estimateSop) ||
          !ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), estimateSop, _trx->legs()))
        continue;

      if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(estimateSop))
        if (!checkNumOfLngCnx())
          continue;

      generateEstimateMatrixItem(flightMatrixPos, path, estimateSop);
    }

    _outboundSopSet.insert(sops[0]);
  }

  // find new estimate sop in inboundSopSet
  sopIt = _inboundSopSet.find(sops[1]);

  if (sopIt == _inboundSopSet.end())
  {
    for (sopIt = _outboundSopSet.begin(); sopIt != _outboundSopSet.end(); sopIt++)
    {
      if (*sopIt == sops[0])
      {
        continue;
      }

      std::vector<int> estimateSop;
      estimateSop.push_back(*sopIt);
      estimateSop.push_back(sops[1]);

      if (getFlightMatrix().count(estimateSop) || getEstimateMatrix().count(estimateSop) ||
          !ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), estimateSop, _trx->legs()))
        continue;

      if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(estimateSop))
        if (!checkNumOfLngCnx())
          continue;

      generateEstimateMatrixItem(flightMatrixPos, path, estimateSop);
    }

    _inboundSopSet.insert(sops[1]);
  }
}

void
ShoppingPQ::removeBadEstimates()
{
  const bool showDiag910Details =
      shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
      shoppingTrx().diagnostic().diagParamMapItem("SHOWREMOVEDSOPS") == "Y";
  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (showDiag910Details && !_badEstimates.empty())
    diag << "Removed SOPs for " << (_carrier ? *_carrier : "Interline") << " queue:\n";

  for (std::set<std::vector<int>>::const_iterator i = _badEstimates.begin();
       i != _badEstimates.end();
       ++i)
  {
    _flightMatrix.erase(*i);

    if (showDiag910Details)
    {
      diag << "\t";
      printSopCombination(diag.collector(), *i);
      diag << "\n";
    }

    ShoppingTrx::EstimateMatrix::iterator j = estimateMatrix().begin();

    while (j != estimateMatrix().end())
    {
      if (j->second.first == *i)
      {
        if (showDiag910Details)
        {
          diag << "\t";
          printSopCombination(diag.collector(), j->first);
          diag << "\n";
        }

        estimateMatrix().erase(j++);
      }
      else
      {
        ++j;
      }
    }
  }
}

void
ShoppingPQ::removeHighAmountAltDates()
{
  if (!shoppingTrx().isAltDates() || !_taxForISProcess)
  {
    return;
  }

  // find the lowest amount
  MoneyAmount lowestAmount = 10000000;
  MoneyAmount lowestAmountForSnowman = 10000000;

  for (auto& altDateLowestAmount : shoppingTrx().altDateLowestAmount())
  {
    if (altDateLowestAmount.second->lowestOptionAmount < lowestAmount)
    {
      lowestAmount = altDateLowestAmount.second->lowestOptionAmount;
    }

    if (altDateLowestAmount.second->lowestOptionAmountForSnowman < lowestAmountForSnowman)
    {
      lowestAmountForSnowman = altDateLowestAmount.second->lowestOptionAmountForSnowman;
    }
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << " \n";
    diag << "REMOVE TOO HIGH AMOUNT FOR ALT-DATE OPTIONS FOR QUEUE - ";

    if (_carrier)
    {
      diag << *_carrier;
    }
    else
    {
      diag << "INTERLINE";
    }

    diag << " \n";
    diag << " LOWEST AMOUNT ACROSS ALL DATES IS " << lowestAmount << " \n";
    diag << " LOWEST SNOWMAN AMOUNT ACROSS ALL DATES IS " << lowestAmountForSnowman << " \n";
    diag << " TAX AMOUNT FOR THIS QUEUE IS " << _taxAmount << " \n";
    diag << " USING PRICE JUMP FACTOR OF " << _altDateItinPriceJumpFactor
         << " AND USING PRICE OVER " << ppo()->altDateCutOffAmount() * RuleConst::HUNDRED_PERCENTS
         << " PERCENT "
         << " \n";
    diag << " AND USING PRICE FOR SNOWMAN OVER "
         << ppo()->altDateCutOffSnowmanAmount() * RuleConst::HUNDRED_PERCENTS << " PERCENT "
         << " \n";
  }

  // find the sops which amount too high
  std::set<std::vector<int>> highAmountSops;
  for (const auto& flightMatrixElement : getFlightMatrix())
  {
    // See if higher than price jump across the dates
    MoneyAmount farePathAmt = 1000000;

    if (flightMatrixElement.second == nullptr)
    {
      continue;
    }

    const MoneyAmount diff = farePathAmt - flightMatrixElement.second->getTotalNUCAmount();

    if (diff < EPSILON)
    {
      continue;
    }

    FarePath* fpPtr = (*(flightMatrixElement.second->groupFPPQItem().begin()))->farePath();

    if (fpPtr != nullptr)
    {
      farePathAmt =
          (*(flightMatrixElement.second->groupFPPQItem().begin()))->farePath()->getTotalNUCAmount();
    }

    if (farePathAmt > 999999)
    {
      continue;
    }

    const DatePair& dates =
        ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), flightMatrixElement.first);

    if ((isFlightSnowman(flightMatrixElement.first) &&
         (farePathAmt + _taxAmount) > (_altDateItinPriceJumpFactor * lowestAmountForSnowman)) ||
        (farePathAmt + _taxAmount) > (_altDateItinPriceJumpFactor * lowestAmount))
    {
      highAmountSops.insert(flightMatrixElement.first);

      if (diag.isActive())
      {
        const std::vector<int>& myCell = flightMatrixElement.first;
        int leg = 0;

        if (isFlightSnowman(flightMatrixElement.first))
          diag << "  REMOVE PRICE JUMP SOP (SNOWMAN) ";
        else
          diag << "  REMOVE PRICE JUMP SOP ";

        for (std::vector<int>::const_iterator n = myCell.begin(); n != myCell.end(); ++n, ++leg)
        {
          diag << (n == myCell.begin() ? "" : ",")
               << ShoppingUtil::findSopId(shoppingTrx(), leg, size_t(*n));
        }

        diag << " FP AMT PLUS TAX " << farePathAmt + _taxAmount << " DATE "
             << dates.first.dateToString(DDMMM, "") << "-" << dates.second.dateToString(DDMMM, "")
             << " \n";
      }

      continue;
    }

    // See if higher than percentage within the same date pair
    ShoppingTrx::AltDateLowestAmount::iterator itr =
        shoppingTrx().altDateLowestAmount().find(dates);

    if (itr != shoppingTrx().altDateLowestAmount().end())
    {
      bool isSnowMan = isFlightSnowman(flightMatrixElement.first);

      if ((isSnowMan && (itr->second->lowestOptionAmountForSnowman) < (farePathAmt + _taxAmount)) ||
          (!isSnowMan && (itr->second->lowestOptionAmount) < (farePathAmt + _taxAmount)))
      {
        highAmountSops.insert(flightMatrixElement.first);

        if (diag.isActive())
        {
          const std::vector<int>& myCell = flightMatrixElement.first;
          int leg = 0;

          if (isSnowMan)
            diag << "  REMOVE PRICE OVER PERCENT SOP (SNOWMEN) ";
          else
            diag << "  REMOVE PRICE OVER PERCENT SOP ";

          for (std::vector<int>::const_iterator n = myCell.begin(); n != myCell.end(); ++n, ++leg)
          {
            diag << (n == myCell.begin() ? "" : ",")
                 << ShoppingUtil::findSopId(shoppingTrx(), leg, size_t(*n));
          }

          diag << " FP AMT PLUS TAX " << farePathAmt + _taxAmount << " DATE "
               << dates.first.dateToString(DDMMM, "") << "-" << dates.second.dateToString(DDMMM, "")
               << " \n";
        }
      }
    }
  }

  // now remove high amount sops from flight matrix
  for (const auto& highAmountSop : highAmountSops)
  {
    _flightMatrix.erase(highAmountSop);
  }
}

GroupFarePath*
ShoppingPQ::duplicateGroupFarePath(GroupFarePath* path)
{
  GroupFarePath* res = path->createDuplicate(shoppingTrx());
  TSE_ASSERT(res->groupFPPQItem().size() == path->groupFPPQItem().size());

  for (size_t n = 0; n != path->groupFPPQItem().size(); ++n)
  {
    _baseFarePath[getBaseFarePath(res->groupFPPQItem()[n]->farePath())] =
        path->groupFPPQItem()[n]->farePath();
  }

  return res;
}

FarePath*
ShoppingPQ::getBaseFarePath(FarePath* path)
{
  const std::map<FarePath*, FarePath*>::iterator i = _baseFarePath.find(path);

  if (i != _baseFarePath.end())
  {
    return i->second;
  }
  else
  {
    return path;
  }
}

bool
ShoppingPQ::checkValidatingCarrierForCat35(const PaxTypeFareRuleData& ptfRd, FarePath& farePath)
{
  const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ptfRd.ruleItemInfo());

  if (!negFareRest->carrier().empty())
  {
    if (UNLIKELY(farePath.itin()->validatingCarrier().empty()))
    {
      _validatingCarrier.update(farePath);
    }

    const CarrierCode valCxr = farePath.itin()->validatingCarrier();

    if ((negFareRest->tktAppl() == ' ' && negFareRest->carrier() != valCxr) ||
        (negFareRest->tktAppl() == 'X' && negFareRest->carrier() == valCxr))
    {
      return false;
    }
  }

  return true;
}

void
ShoppingPQ::generateEstimatedFlightOnlySolutions()
{
  if (shoppingTrx().isAltDates() || isOwFaresShoppingQueue())
  {
    return;
  }

  DiagManager diag(shoppingTrx(), Diagnostic910);

  if (diag.isActive())
  {
    diag << "Generating estimated flight-only solutions for queue ";

    if (_carrier)
    {
      diag << *_carrier;
    }
    else if (isOwFaresShoppingQueue())
    {
      diag << "OW Fares";
    }
    else
    {
      diag << "Interline";
    }

    diag << ":";
  }

  std::vector<std::vector<int>> sopVecforAllLegs;
  sopVecforAllLegs.resize(shoppingTrx().legs().size());
  // go through all flight matrix result.
  // create the sop list for each leg that has the same carrier and
  // connection point
  // combine them together and check for duplicate with the based solution
  int noEstimatedOption = static_cast<int>(getNoOfOptionsRequested()) < _nEstimatedOptions
                              ? getNoOfOptionsRequested()
                              : _nEstimatedOptions;

  for (ShoppingTrx::FlightMatrix::const_iterator fltResult = getFlightMatrix().begin();
       fltResult != getFlightMatrix().end() &&
           static_cast<int>(getFlightMatrix().size() + getEstimateMatrix().size()) <
               noEstimatedOption;
       ++fltResult)
  {
    if (fltResult->second != nullptr && fltResult->second->getTotalNUCAmount() != 1000000)
      continue;

    const std::vector<int>& basedSolution = fltResult->first;
    bool sameCxrAndCnxAsPreviousSolution = true;
    uint32_t legIndex = 0;

    for (std::vector<ShoppingTrx::Leg>::const_iterator legIt = _trx->legs().begin();
         legIt != _trx->legs().end();
         ++legIt, ++legIndex)
    {
      if (legIt->stopOverLegFlag())
      {
        continue;
      }

      int basedSolutionSopId = basedSolution[legIndex];

      if (!sopVecforAllLegs[legIndex].empty() &&
          isSopInTheList(basedSolutionSopId, sopVecforAllLegs[legIndex]))
      {
        continue;
      }

      sameCxrAndCnxAsPreviousSolution = false;
      sopVecforAllLegs[legIndex].clear();
      sopVecforAllLegs[legIndex].push_back(basedSolutionSopId);
      // loop sop for each leg
      const std::vector<ShoppingTrx::SchedulingOption>& sops = legIt->sop();
      int sopSize = sops.size();

      for (int sopIndex = 0; sopIndex < sopSize; ++sopIndex)
      {
        if (basedSolutionSopId == sopIndex)
        {
          continue;
        }

        if (sops[sopIndex].cabinClassValid() &&
            ShoppingUtil::isSameCxrAndCnxPointAndClassOfService(
                *_trx, sops[basedSolutionSopId], sops[sopIndex]))
        {
          if (shoppingTrx().isLngCnxProcessingEnabled() && sops[sopIndex].isLngCnxSop())
            if (!checkNumOfLngCnx())
              continue;

          sopVecforAllLegs[legIndex].push_back(sopIndex);
        }
      }
    }

    if (!sameCxrAndCnxAsPreviousSolution)
    {
      createEstimatedFlightOnlySolution(basedSolution, sopVecforAllLegs, noEstimatedOption);
    }
  }

  if (diag.isActive())
  {
    diag << " " << getEstimateMatrix().size() << "\n";
  }
}

bool
ShoppingPQ::isSopInTheList(int basedSolutionSopId, std::vector<int>& sopVecForEachLeg)
{
  for (uint32_t index = 0; index < sopVecForEachLeg.size(); ++index)
  {
    if (basedSolutionSopId == sopVecForEachLeg[index])
    {
      return true;
    }
  }

  return false;
}

void
ShoppingPQ::createEstimatedFlightOnlySolution(const std::vector<int>& basedSolution,
                                              std::vector<std::vector<int>>& sopVecforAllLegs,
                                              int32_t noEstimatedSolution)
{
  std::vector<std::vector<int>> sopVecList;
  std::vector<std::vector<int>> tempSopVecList;

  for (uint32_t legIndex = 0; legIndex < sopVecforAllLegs.size(); ++legIndex)
  {
    std::vector<int>& sopVecForEachLeg = sopVecforAllLegs[legIndex];

    if (sopVecList.empty())
    {
      for (uint32_t sopIndex = 0; sopIndex < sopVecForEachLeg.size(); ++sopIndex)
      {
        std::vector<int> oneSopVec;
        oneSopVec.push_back(sopVecForEachLeg[sopIndex]);
        sopVecList.push_back(oneSopVec);
      }
    }
    else
    {
      for (uint32_t sopVecListIndex = 0; sopVecListIndex < sopVecList.size(); ++sopVecListIndex)
      {
        for (uint32_t sopIndex = 0; sopIndex < sopVecForEachLeg.size(); ++sopIndex)
        {
          // start with the original item in the sopVecList
          std::vector<int> tempSopVec = sopVecList[sopVecListIndex];
          // add more sop for this leg
          tempSopVec.push_back(sopVecForEachLeg[sopIndex]);

          if (ShoppingUtil::checkMinConnectionTime(_trx->getOptions(), tempSopVec, _trx->legs()))
            tempSopVecList.push_back(tempSopVec);
        }
      }

      // if no item has been created for this leg, return
      if (tempSopVecList.empty())
      {
        return;
      }

      sopVecList.clear();
      sopVecList = tempSopVecList;
      tempSopVecList.clear();
    }
  }

  for (uint32_t sopVecListIndex = 0;
       sopVecListIndex < sopVecList.size() &&
           static_cast<int>(getFlightMatrix().size() + getEstimateMatrix().size()) <
               noEstimatedSolution;
       ++sopVecListIndex)
  {
    if (!isSopinTheMatrix(sopVecList[sopVecListIndex]))
    {
      GroupFarePath* gfp = shoppingTrx().dataHandle().create<GroupFarePath>();
      gfp->setSourcePqName(getSourceName());
      gfp->setTotalNUCAmount(1000000);
      ShoppingTrx::EstimatedSolution estimate(basedSolution, gfp);

      bool hasLngCnxAdded = false;
      if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(sopVecList[sopVecListIndex]))
      {
        if (!checkNumOfLngCnx())
          return;

        shoppingTrx().pqDiversifierResult().incrementCurrentLngCnxOptionCount();
        hasLngCnxAdded = true;
      }

      if (processCustomSolutionStats(sopVecList[sopVecListIndex], hasLngCnxAdded))
        _estimateMatrix[sopVecList[sopVecListIndex]] = estimate;
    }
  }
}

bool
ShoppingPQ::isSopinTheMatrix(const std::vector<int>& sopVec)
{
  ShoppingTrx::FlightMatrix::const_iterator fltItor = getFlightMatrix().find(sopVec);

  if (fltItor != getFlightMatrix().end())
  {
    return true;
  }

  ShoppingTrx::EstimateMatrix::const_iterator itor = getEstimateMatrix().find(sopVec);

  if (itor != getEstimateMatrix().end())
  {
    return true;
  }

  return false;
}

bool
ShoppingPQ::isSopinTheMatrix(const std::vector<int>& sopVec, CarrierCode* cxr)
{
  const ShoppingPQ* cxrPQ = shoppingTrx().getCxrShoppingPQ(cxr);

  if (cxrPQ == nullptr)
    return false;

  const ShoppingTrx::FlightMatrix* flightMatrix = &(cxrPQ->getFlightMatrix());

  if (UNLIKELY(flightMatrix == nullptr))
    return false;

  ShoppingTrx::FlightMatrix::const_iterator fltItor = flightMatrix->find(sopVec);

  if (fltItor != flightMatrix->end())
  {
    return true;
  }

  const ShoppingTrx::EstimateMatrix* estimateMatrix = &(cxrPQ->getEstimateMatrix());

  if (UNLIKELY(estimateMatrix == nullptr))
    return false;

  ShoppingTrx::EstimateMatrix::const_iterator itor = estimateMatrix->find(sopVec);

  if (itor != estimateMatrix->end())
  {
    return true;
  }

  return false;
}

bool
ShoppingPQ::foundOnlineOption()
{
  if (isInterline())
  {
    return true;
  }

  for (const auto& fltResult : getFlightMatrix())
  {
    const std::vector<int>& basedSolution = fltResult.first;
    uint32_t legIndex = 0;
    bool foundOnline = true;

    for (std::vector<ShoppingTrx::Leg>::const_iterator legIt = _trx->legs().begin();
         legIt != _trx->legs().end();
         ++legIt, ++legIndex)
    {
      if (legIt->stopOverLegFlag())
      {
        continue;
      }

      if (!ShoppingUtil::isOnlineConnectionFlight(
              basedSolution[legIndex], legIndex, _carrier, _trx, true))
      {
        foundOnline = false;
        break;
      }
    }

    if (foundOnline)
    {
      return true;
    }
  }

  return false;
}

void
ShoppingPQ::groupMother()
{
  if ((_trx->isAltDates()) || (_trx->getRequestedNumOfEstimatedSolutions() < 1))
  {
    return;
  }

  ShoppingTrx::FlightMatrix* groupFltMatrix = nullptr;
  _trx->dataHandle().get(groupFltMatrix);

  for (const auto& fltResult : getFlightMatrix())
  {
    if (!canBeInFamily(fltResult))
    {
      groupFltMatrix->insert(fltResult);
      continue;
    }

    bool sameFamily = true;
    ShoppingTrx::FlightMatrix::iterator grpFltMatrixI = groupFltMatrix->begin();
    ShoppingTrx::FlightMatrix::iterator grpFltMatrixEnd = groupFltMatrix->end();

    for (; grpFltMatrixI != grpFltMatrixEnd; ++grpFltMatrixI)
    {
      if (!canBeInFamily(*grpFltMatrixI))
      {
        continue;
      }

      FarePath* fltMatrixFP = fltResult.second->groupFPPQItem()[0]->farePath();
      const FarePath* baseFarePath = getBaseFarePath(fltMatrixFP);
      fltMatrixFP = grpFltMatrixI->second->groupFPPQItem()[0]->farePath();
      const FarePath* newBaseFarePath = getBaseFarePath(fltMatrixFP);

      if (baseFarePath != newBaseFarePath)
      {
        continue;
      }

      const std::vector<int>& basedSolution = fltResult.first;
      const std::vector<int>& newSolution = grpFltMatrixI->first;
      uint32_t legIndex = 0;

      for (std::vector<ShoppingTrx::Leg>::const_iterator legIt = _trx->legs().begin();
           legIt != _trx->legs().end();
           ++legIt, ++legIndex)
      {
        if (legIt->stopOverLegFlag())
        {
          continue;
        }

        const uint32_t basedSolutionSopId = basedSolution[legIndex];
        const uint32_t newSolutionSopId = newSolution[legIndex];
        const std::vector<ShoppingTrx::SchedulingOption>& sops = legIt->sop();

        if (basedSolutionSopId >= sops.size())
        {
          break;
        }

        const ShoppingTrx::SchedulingOption& sop1 = sops[basedSolutionSopId];
        const ShoppingTrx::SchedulingOption& sop2 = sops[newSolutionSopId];
        bool sameHighTPM = (sop1.isHighTPM() == sop2.isHighTPM());
        bool bothHighTPMAreTrue = (sameHighTPM && sop1.isHighTPM());
        bool bothHighTPMAreFalse = (sameHighTPM && !sop1.isHighTPM());

        if (!fallback::fallbackFareSelction2016(_trx))
        {
          if (!sameHighTPM || (bothHighTPMAreTrue && !sopsHighTPMSimilarityCheck(sop1, sop2)))
          {
            sameFamily = false;
            break;
          }
        }

        if (fallback::fallbackFareSelction2016(_trx) || bothHighTPMAreFalse)
        {
          if (ShoppingUtil::schedulingOptionsSimilar(
                  shoppingTrx(), sops[basedSolutionSopId], sops[newSolutionSopId]) == false)
          {
            sameFamily = false;
            break;
          }
        }
      }

      if (sameFamily)
      {
        break;
      }

      sameFamily = true;
    } // end for loop each groupFltMatrix

    if (sameFamily && (grpFltMatrixI != grpFltMatrixEnd))
    {
      // found similar itin here
      const std::vector<int>& oldSol = fltResult.first;

      for (auto& estimateMatrixElement : estimateMatrix())
      {
        const std::vector<int>& baseSol = estimateMatrixElement.second.first;

        if (baseSol == oldSol)
        {
          estimateMatrixElement.second.first = grpFltMatrixI->first;
        }
      }

      const ShoppingTrx::EstimatedSolution estimate(grpFltMatrixI->first, fltResult.second);

      if (shoppingTrx().isLngCnxProcessingEnabled() && hasLngCnxSop(fltResult.first))
      {
        if (checkNumOfLngCnx())
        {
          shoppingTrx().pqDiversifierResult().incrementCurrentLngCnxOptionCount();
          _estimateMatrix[fltResult.first] = estimate;
        }
      }
      else
      {
        _estimateMatrix[fltResult.first] = estimate;
      }
    }
    else
    {
      const ShoppingTrx::FlightMatrix::value_type item(fltResult.first, fltResult.second);
      groupFltMatrix->insert(item);
    }
  } // end for loop each original flight matrix

  regroupIndustryFaresFamilies(*groupFltMatrix);

  if (!groupFltMatrix->empty())
  {
    setFlightMatrix(*groupFltMatrix);
  }

  return;
}

void
ShoppingPQ::makeOptionHigherPriority()
{
  if (getFlightMatrix().size() != 1)
  {
    return;
  }

  ShoppingTrx::FlightMatrix::iterator fltResult = _flightMatrix.begin();

  if ((fltResult->second != nullptr) && ((fltResult->second->groupFPPQItem().size() == 0) ||
                                         (fltResult->second->getTotalNUCAmount() >= 1000000)))
  {
    fltResult->second->setTotalNUCAmount(0);
  }
}

bool
ShoppingPQ::foundNonStopOption()
{
  bool nonStopSop = true;

  for (auto& fltResult : getFlightMatrix())
  {
    if (fltResult.second == nullptr)
    {
      return true;
    }

    const MoneyAmount diff = 1000000 - fltResult.second->getTotalNUCAmount();

    if (diff < EPSILON)
    {
      return true;
    }

    const std::vector<int>& basedSolution = fltResult.first;
    uint32_t legIndex = 0;

    for (auto& leg : _trx->legs())
    {
      if (leg.stopOverLegFlag())
      {
        ++legIndex;
        continue;
      }

      uint32_t basedSolutionSopId = basedSolution[legIndex];
      const std::vector<ShoppingTrx::SchedulingOption>& sops = leg.sop();

      if (basedSolutionSopId >= sops.size())
      {
        return nonStopSop;
      }

      if (sops[basedSolutionSopId].itin()->travelSeg().size() > 1)
      {
        nonStopSop = false;
        break;
      }
      ++legIndex;
    }

    if (nonStopSop)
    {
      break;
    }
  }

  return nonStopSop;
}

bool
ShoppingPQ::foundFarePathOption()
{
  ShoppingTrx::FlightMatrix::const_iterator fltResultEnd = getFlightMatrix().end();

  for (const auto& fltResult : getFlightMatrix())
  {
    if ((fltResult.second == nullptr) || fltResult.second->getTotalNUCAmount() == 0)
    {
      continue;
    }

    const MoneyAmount diff = 1000000 - fltResult.second->getTotalNUCAmount();

    if (diff > EPSILON)
    {
      return true;
    }
  }

  return false;
}

CarrierCode*
ShoppingPQ::sameCxrFarePath(FarePath& farePath)
{
  CarrierCode* carrier = nullptr;

  for (const auto pricingUnit : farePath.pricingUnit())
  {
    for (const auto fareUsage : (*pricingUnit).fareUsage())
    {
      if (carrier != nullptr &&
          (*carrier != ((*fareUsage).paxTypeFare()->fareMarket()->governingCarrier())))
      {
        carrier = nullptr;
        return carrier;
      }
      else
      {
        carrier = &((*fareUsage).paxTypeFare()->fareMarket()->governingCarrier());
      }
    }
  }

  return carrier;
}

bool
ShoppingPQ::checkIfSimilarOption(const std::vector<int>& sops)
{
  const bool showDiag910Details = shoppingTrx().diagnostic().diagnosticType() == Diagnostic910 &&
                                  shoppingTrx().diagnostic().diagParamMapItem("DD") == "DETAILS";
  DiagManager diag(shoppingTrx(), Diagnostic910);
  ShoppingTrx::FlightMatrix::const_iterator flightMatrixElement = getFlightMatrix().begin();
  const DatePair& curDatePair = ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), sops);

  for (const auto& flightMatrixElement : getFlightMatrix())
  {
    const std::vector<int>& cellSops = flightMatrixElement.first;
    const DatePair& cellDatePair = ShoppingAltDateUtil::getDatePairSops(shoppingTrx(), cellSops);

    // process matrix cell only for the same dates
    if (cellDatePair.first != curDatePair.first || cellDatePair.second != curDatePair.second)
    {
      continue;
    }

    if (false == ShoppingUtil::areSopConnectionPointsDifferent(shoppingTrx(), cellSops, sops))
    {
      if (showDiag910Details)
      {
        printSopCombination(diag.collector(), sops);
        diag << " similar to ";
        printSopCombination(diag.collector(), cellSops);
        diag << " - SKIPPED\n";
      }

      return true;
    }
  }

  return false;
}

bool
ShoppingPQ::isFlightSnowman(const std::vector<int>& sops)
{
  if (_trx->legs().size() > 1)
  {
    const ShoppingTrx::SchedulingOption& sopOutbound = _trx->legs().front().sop()[sops[0]];
    const ShoppingTrx::SchedulingOption& sopInbound = _trx->legs().back().sop()[sops[1]];

    if (sopOutbound.itin()->travelSeg().size() == 2 && sopInbound.itin()->travelSeg().size() == 2)
    {
      const TravelSeg* secondSegOut = sopOutbound.itin()->travelSeg().back();
      const TravelSeg* firstSegIn = sopInbound.itin()->travelSeg().front();
      const AirSeg* secondAirSegOut = secondSegOut->toAirSeg();
      const AirSeg* firstAirSegIn = firstSegIn->toAirSeg();

      if (secondAirSegOut->carrier() == firstAirSegIn->carrier())
      {
        return true;
      }
    }
  }

  return false;
}

void
ShoppingPQ::getSortedSolutionsGFP(std::vector<GroupFarePath*>& results)
{
  results.clear();

  if (getFlightMatrix().empty())
    return;

  std::vector<ISSolution> sol(getFlightMatrix().begin(), getFlightMatrix().end());
  std::sort(sol.begin(), sol.end(), compareSolutions);

  for (std::vector<ISSolution>::iterator i = sol.begin(); i != sol.end(); ++i)
  {
    if ((*i).second == nullptr)
      continue;

    if ((*i).second->getTotalNUCAmount() >= 1000000 || (*i).second->getTotalNUCAmount() <= 0 ||
        (*i).second->groupFPPQItem().size() == 0)
      continue;

    results.push_back((*i).second);
  }
}

bool
ShoppingPQ::hasLngCnxSop(const std::vector<int>& sops) const
{
  for (uint32_t n = 0; n < sops.size(); ++n)
  {
    const ShoppingTrx::SchedulingOption& sop = shoppingTrx().legs()[n].sop()[sops[n]];

    if (sop.isLngCnxSop())
      return true;
  }

  return false;
}

bool
ShoppingPQ::checkNumOfLngCnx()
{
  if (shoppingTrx().pqDiversifierResult().currentLngCnxOptionCount() >=
      shoppingTrx().maxNumOfLngCnxSolutions())
    return false;

  return true;
}

bool
ShoppingPQ::needMoreCustomSolutions()
{
  if (!shoppingTrx().getNumOfCustomSolutions())
    return false;

  uint32_t limit = _customSolutionSearchActivated
                       ? shoppingTrx().pqDiversifierResult()._minNumCustomSolutions
                       : static_cast<uint32_t>(shoppingTrx().getNumOfCustomSolutions());

  if (shoppingTrx().pqDiversifierResult().currentCustomOptionCount() >= limit)
    return false;

  return true;
}

bool
ShoppingPQ::addToFlightMatrix(const ShoppingTrx::FlightMatrix::value_type& item,
                              const GroupFarePath* originalGfp)
{
  const GroupFarePath* gfp = originalGfp ? originalGfp : item.second;
  const bool checkRepeats = (isOwFaresShoppingQueue() && (_fareCombRepeatLimit > 0) && gfp);
  FarePathRepeatCounter::iterator it;
  FarePathRepeatCounter::key_type key;

  if (checkRepeats)
  {
    key = makeFarePathRepeatCounterKey(gfp);
    it = _farePathRepeatCounter.find(key);

    if (it != _farePathRepeatCounter.end() && it->second >= _fareCombRepeatLimit)
      return false;
  }

  if (shoppingTrx().isLngCnxProcessingEnabled())
  {
    if (hasLngCnxSop(item.first))
    {
      if (checkNumOfLngCnx())
      {
        if (!processCustomSolutionStats(item.first))
          return false;

        if (_flightMatrix.insert(item).second)
        {
          shoppingTrx().pqDiversifierResult().incrementCurrentLngCnxOptionCount();

          if (shoppingTrx().getRequest()->isParityBrandsPath() ||
              shoppingTrx().diversity().isExchangeForAirlines())
          {
            shoppingTrx().getIbfData().getV2IbfManager().newQueueSolution(item.first);
          }

          if (checkRepeats)
          {
            if (it == _farePathRepeatCounter.end())
              _farePathRepeatCounter.insert(std::make_pair(key, 1));
            else
              ++it->second;
          }
          insertCustomFamilyHead(item.first);

          return true;
        }
        else
        {
          if (shoppingTrx().getNumOfCustomSolutions() &&
              ShoppingUtil::isCustomSolution(shoppingTrx(), item.first))
          {
            // We have incremented the custom solution count before inserting the item.
            // Since the insertion failed, we need to decrement the count
            shoppingTrx().pqDiversifierResult().decrementCurrentCustomOptionCount();
          }
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }

  if (!processCustomSolutionStats(item.first))
    return false;

  _flightMatrix.insert(item);
  insertCustomFamilyHead(item.first);

  if (shoppingTrx().getRequest()->isParityBrandsPath() ||
      shoppingTrx().diversity().isExchangeForAirlines())
  {
    shoppingTrx().getIbfData().getV2IbfManager().newQueueSolution(item.first);
  }

  if (checkRepeats)
  {
    if (it == _farePathRepeatCounter.end())
    {
      _farePathRepeatCounter.insert(std::make_pair(key, 1));
    }
    else
    {
      ++it->second;
    }
  }

  return true;
}

void
ShoppingPQ::insertCustomFamilyHead(const SopIdVec& sops)
{
  if (!_allowToRemoveCustomFamily && shoppingTrx().getNumOfCustomSolutions() &&
      ShoppingUtil::isCustomSolution(shoppingTrx(), sops) &&
      _customFamilyHeads.find(sops) == _customFamilyHeads.end())
  {
    _customFamilyHeads.insert(sops);
  }
}

ShoppingPQ::FarePathRepeatCounter::key_type
ShoppingPQ::makeFarePathRepeatCounterKey(const GroupFarePath* gfp) const
{
  FarePathRepeatCounter::key_type key;

  for (size_t i = 0; i < gfp->groupFPPQItem().size(); ++i)
  {
    const FarePath* fp = gfp->groupFPPQItem()[i]->farePath();

    for (size_t j = 0; j < fp->pricingUnit().size(); ++j)
    {
      const PricingUnit* pu = fp->pricingUnit()[j];

      for (size_t k = 0; k < pu->fareUsage().size(); ++k)
      {
        const PaxTypeFare* ptf = pu->fareUsage()[k]->paxTypeFare();
        std::ostringstream os;
        os << ptf->market1() << "|" << ptf->market2() << "|" << ptf->vendor() << "|"
           << ptf->fareTariff() << "|" << ptf->carrier() << "|" << ptf->ruleNumber() << "|"
           << ptf->fareClass() << "|" << ptf->fareAmount();
        key.push_back(os.str());
      }
    }
  }

  return key;
}

std::string
ShoppingPQ::getSourceName()
{
  if (UNLIKELY(isOwFaresShoppingQueue()))
  {
    return "OWFARES";
  }
  else if (UNLIKELY(isNonStopShoppingQueue()))
  {
    return "NONSTOP";
  }
  else if (isInterline())
  {
    return "INTERLINE";
  }
  else
  {
    std::ostringstream os;
    os << "ONLINE-" << *carrier();
    return os.str();
  }
}

bool
ShoppingPQ::canBeInFamily(const ShoppingTrx::FlightMatrix::value_type& solution) const
{
  const GroupFarePath* groupFarePath = solution.second;

  if (!groupFarePath)
  {
    return false;
  }

  const MoneyAmount diff = 1000000 - groupFarePath->getTotalNUCAmount();

  if (diff < EPSILON)
  {
    return false;
  }

  if (isIndustryFareUsed(*groupFarePath))
  {
    return false;
  }

  return true;
}

bool
ShoppingPQ::isIndustryFareUsed(const GroupFarePath& groupFarePath) const
{
  bool use = false;

  for (const FPPQItem* fppqItem : groupFarePath.groupFPPQItem())
  {
    if (ShoppingUtil::isIndustryFareUsed(*fppqItem->farePath()))
    {
      use = true;
      break;
    }
  }

  return use;
}

void
ShoppingPQ::regroupIndustryFaresFamilies(ShoppingTrx::FlightMatrix& flightMatrix)
{
  for (ShoppingTrx::EstimateMatrix::iterator solutionIt = _estimateMatrix.begin();
       solutionIt != _estimateMatrix.end();
       /*no increment*/)
  {
    ShoppingTrx::EstimatedSolution& solution = solutionIt->second;
    GroupFarePath* groupFarePath = solution.second;

    if (groupFarePath && isIndustryFareUsed(*groupFarePath))
    {
      flightMatrix[solutionIt->first] = groupFarePath;
      _estimateMatrix.erase(solutionIt++);
    }
    else
    {
      ++solutionIt;
    }
  }
}

bool
ShoppingPQ::processCustomSolutionStats(const std::vector<int>& sops, bool hasLngCnxAdded)
{
  bool isAddtoMatrix = true;
  if (shoppingTrx().getNumOfCustomSolutions())
  {
    if (ShoppingUtil::isCustomSolution(shoppingTrx(), sops))
    {
      if (needMoreCustomSolutions() || hasLngCnxAdded)
      {
        shoppingTrx().pqDiversifierResult().incrementCurrentCustomOptionCount();
      }
      else
      {
        isAddtoMatrix = false;
      }
    }
    else
    {
      if (_customSolutionSearchActivated)
        isAddtoMatrix = false;
    }
  }

  return isAddtoMatrix;
}

void
ShoppingPQ::generateCustomSolutionsWithNoFares(GroupFarePath* gfp)
{
  DiagManager diag(shoppingTrx(), Diagnostic910);
  if (diag.isActive())
  {
    diag << "Generating custom flight-only solutions for queue ";
    if (_carrier)
      diag << *_carrier;
    else if (isOwFaresShoppingQueue())
      diag << "OW Fares";
    else
      diag << "Interline";
    diag << ":\n";
  }

  uint32_t failedMatrixCnt = 0;

  MultiDimensionalPQ<SOPWrapper, SopId> mpq(_legWrappedCustomSopsCollection);
  while (lookForMoreCustomSolutions())
  {
    SopIdWrapperVec candidate = mpq.next();
    if (candidate.empty())
      break;
    ++failedMatrixCnt;
    if (failedMatrixCnt > _maxFailedCellsForFltOnlySolution)
      break;

    SopIdVec sops = getActualSops(candidate);

    // Below check not needed as input container should contains only custom combinations
    // if (!ShoppingUtil::isCustomSolution(shoppingTrx(), sops))
    //  continue;

    if (isValidMatrixCell(sops) && ShoppingUtil::isCabinClassValid(*_trx, sops) &&
        isValidCxrRestriction(sops))
    {
      failedMatrixCnt = 0;
      const ShoppingTrx::FlightMatrix::value_type item(sops, gfp);
      addToFlightMatrix(item, gfp);
      if (diag.isActive())
      {
        diag << "  (";
        for (size_t n = 0; n != sops.size(); ++n)
        {
          if (n != 0)
            diag << ", ";
          diag << ShoppingUtil::findSopId(shoppingTrx(), n, sops[n]);
        }
        diag << ")\n";
      }
    }
  }
}

bool
ShoppingPQ::isSolutionProducedByInterlineQueue(const SopIdVec& sops)
{
  if (!isInterline())
  {
    // Check if a solution were not produced by interline Q already
    // This applies when producing FOS solutions
    // Getting FOS is not called multithread so no need to put a mutex
    if (isSopinTheMatrix(sops, nullptr))
      return true;
  }
  return false;
}

bool
ShoppingPQ::needsMoreSolutions(size_t wanted)
{
  if (lookForMoreSolutions(wanted))
  {
    return true;
  }

  if (lookForMoreSolutionsBeyond())
  {
    return true;
  }

  if (lookForMoreCustomSolutions())
  {
    return true;
  }

  // For Interline Branded Fares, we need more
  // solutions if not all SOPs have been represented
  // in the response.
  if ((shoppingTrx().getRequest()->isParityBrandsPath() ||
       shoppingTrx().diversity().isExchangeForAirlines()) &&
      (!shoppingTrx().getIbfData().getV2IbfManager().areAllRequirementsMet()))
  {
    return true;
  }

  return false;
}

bool
ShoppingPQ::sopsHighTPMSimilarityCheck(const ShoppingTrx::SchedulingOption& sop1,
                                       const ShoppingTrx::SchedulingOption& sop2) const
{
  if (sop1.governingCarrier() != sop2.governingCarrier())
    return false;

  const std::vector<TravelSeg*>& tvlSegVec1 = sop1.itin()->travelSeg();
  const std::vector<TravelSeg*>& tvlSegVec2 = sop2.itin()->travelSeg();

  if (tvlSegVec1.size() != tvlSegVec2.size())
    return false;

  for (size_t index = 0; index < tvlSegVec1.size(); ++index)
  {
    const AirSeg* air1 = tvlSegVec1[index]->toAirSeg();
    const AirSeg* air2 = tvlSegVec2[index]->toAirSeg();

    if ((air1 == nullptr) || (air2 == nullptr))
      return false;

    if ((air1->origAirport() != air2->origAirport()) ||
        (air1->destAirport() != air2->destAirport()) ||
        (air1->marketingCarrierCode() != air2->marketingCarrierCode()) ||
        (air1->departureDT().date() != air2->departureDT().date()))
    {
      return false;
    }
  }

  return true;
}

} // end namespace tse
