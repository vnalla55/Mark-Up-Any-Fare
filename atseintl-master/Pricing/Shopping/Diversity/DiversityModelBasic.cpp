// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/Diversity/DiversityModelBasic.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Diagnostic/Diag941Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/Diversity/DsciDiversityModelBasicAdapter.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <boost/tokenizer.hpp>

#include <cmath>
#include <iomanip>

namespace tse
{
FALLBACK_DECL(fallbackExpandCarrierContinueProcessing);
FALLBACK_DECL(fallbackExpandPreferredCxrCutoffCoeff);

namespace
{
ConfigurableValue<double>
basicModelFareCutoffCoeffForCalc("SHOPPING_OPT", "BASIC_MODEL_FARE_CUTOFF_CORRECTION_COEFF", 1.0);
ConfigurableValue<uint32_t>
discontinueProcessingValue("SHOPPING_DIVERSITY", "MAX_DISCONTINUE_PROCESSING_VALUE", 1000000);
ConfigurableValue<double>
basicModelfareCutoffCoeff("SHOPPING_DIVERSITY", "BASIC_MODEL_FARE_CUTOFF_CORRECTION_COEFF", 1.0);
ConfigurableValue<ConfigList<std::string, VectorPolicy<std::string>>>
cutoffPriceRangeExpandPrefCxr("SHOPPING_DIVERSITY", "CUTOFF_PRICE_RANGE_EXPAND_PREF_CXR");
ConfigurableValue<ConfigList<std::string, VectorPolicy<std::string>>>
cutoffCoeffRangeExpandPrefCxr("SHOPPING_DIVERSITY", "CUTOFF_COEFF_RANGE_EXPAND_PREF_CXR");
ConfigurableValue<ConfigList<std::string, VectorPolicy<std::string>>>
cutoffPriceRange("SHOPPING_DIVERSITY", "CUTOFF_PRICE_RANGE");
ConfigurableValue<ConfigList<std::string, VectorPolicy<std::string>>>
cutoffCoeffRange("SHOPPING_DIVERSITY", "CUTOFF_COEFF_RANGE");
ConfigurableValue<double>
travelTimeSeparatorCoef("SHOPPING_DIVERSITY",
                        "BASIC_MODEL_TRAVEL_TIME_SEPARATOR_CORRECTION_COEFF",
                        1.0);
}
static DmcRequirement::Printer
printStatus(int status)
{
  return DmcRequirement::print(status);
}

// ExpPrefCxr = Expand Preferred Carrier
class DiversityModelBasic::FareCutoffCalcMethod
{
public:
  FareCutoffCalcMethod(const DiversityModelBasic& ctx)
    : _ctx(ctx),
      _priceByMultiplierCoefUsed(-1.),
      _priceByMultiplierCoefUsedExpPrefCxr(-1.),
      _priceByMultiplierSource("uninitialized"),
      _priceByMultiplierSourceExpPrefCxr("uninitialized"),
      _priceByCalculation(-1.)
  {
    calcPriceByMultiplier();
    calcPriceByCalculation();
  }

  MoneyAmount getFareCutoffAmount() const
  {
    return isPriceByMultiplierMethod() ? *_priceByMultiplier : _priceByCalculation;
  }

  MoneyAmount getFareCutoffAmountExpPrefCxr() const
  {
    return isPriceByMultiplierMethodExpPrefCxr() ? *_priceByMultiplierExpPrefCxr
                                                 : _priceByCalculation;
  }

  bool isPriceByMultiplierMethod() const
  {
    return (_priceByMultiplier && *_priceByMultiplier > _priceByCalculation);
  }

  bool isPriceByMultiplierMethodExpPrefCxr() const
  {
    return (_priceByMultiplierExpPrefCxr && *_priceByMultiplierExpPrefCxr > _priceByCalculation);
  }

  float getPriceByMultiplierCoefUsed() const { return _priceByMultiplierCoefUsed; }
  float getPriceByMultiplierCoefUsedExpPrefCxr() const
  {
    return _priceByMultiplierCoefUsedExpPrefCxr;
  }

  // for diag
  const char* getSource() const
  {
    return isPriceByMultiplierMethod() ? _priceByMultiplierSource : "price by calculation";
  }

  const char* getSourceExpPrefCxr() const
  {
    return isPriceByMultiplierMethodExpPrefCxr() ? _priceByMultiplierSourceExpPrefCxr
                                                 : "price by calculation";
  }

private:
  const DiversityModelBasic& _ctx;

  boost::optional<MoneyAmount> _priceByMultiplier;
  boost::optional<MoneyAmount> _priceByMultiplierExpPrefCxr;
  float _priceByMultiplierCoefUsed;
  float _priceByMultiplierCoefUsedExpPrefCxr;
  const char* _priceByMultiplierSource;
  const char* _priceByMultiplierSourceExpPrefCxr;
  MoneyAmount _priceByCalculation;

  void calcPriceByMultiplier()
  {
    float multiplier;
    float multiplierExpPrefCxr = 1;
    const MoneyAmount minPrice = _ctx._stats.getMinPrice();

    if (_ctx._diversityParams.getFareCutoffCoefDefined())
    {
      multiplier = _ctx._diversityParams.getFareCutoffCoef();
      if (!fallback::fallbackExpandPreferredCxrCutoffCoeff(&_ctx._trx))
        multiplierExpPrefCxr = multiplier;
      _priceByMultiplierSource = "specified in request";
      _priceByMultiplierSourceExpPrefCxr = "specified in request";
    }
    else
    {
      uint16_t rangeMultiplier = 0;
      uint16_t rangeMultiplierExpPrefCxr = 0;
      if (DiversityModelBasic::getFareCutoffMultiplier(minPrice, rangeMultiplier) &&
          DiversityModelBasic::getFareCutoffMultiplierExpPrefCxr(minPrice,
                                                                 rangeMultiplierExpPrefCxr))
      {
        multiplier = rangeMultiplier;
        multiplierExpPrefCxr = rangeMultiplierExpPrefCxr;
        _priceByMultiplierSource = "SHOPPING_DIVERSITY\\CUTOFF_PRICE_RANGE";
        _priceByMultiplierSourceExpPrefCxr =
            "SHOPPING_DIVERSITY\\CUTOFF_PRICE_RANGE_EXPAND_PREF_CXR";
      }
      else
        return; // _priceByMultiplier will remain uninitialized
    }

    _priceByMultiplier = round(minPrice * multiplier);
    _priceByMultiplierCoefUsed = multiplier;
    _priceByMultiplierExpPrefCxr = round(minPrice * multiplierExpPrefCxr);
    _priceByMultiplierCoefUsedExpPrefCxr = multiplierExpPrefCxr;
  }

  void calcPriceByCalculation()
  {
    const MoneyAmount minPrice = _ctx._stats.getMinPrice();
    const MoneyAmount avgPrice = _ctx._stats.getAvgPrice();
    const MoneyAmount maxPrice = _ctx._stats.getMaxPrice();

    _priceByCalculation =
        round(maxPrice * (1.0 +
                          (avgPrice + 1.0 - minPrice) / (maxPrice + 1.0 - minPrice) *
                              basicModelFareCutoffCoeffForCalc.getValue() / log10(maxPrice)) *
              100.0) /
        100.0;
  }
};

Logger
DiversityModelBasic::_logger("atseintl.ShoppingPQ.DiversityModelBasic");

DiversityModelBasic::DiversityModelBasic(ShoppingTrx& trx, ItinStatistic& stats, DiagCollector* dc)
  : _trx(trx),
    _stats(stats),
    _diversityParams(trx.diversity()),
    _swapper(trx, stats, this, dc),
    _numberOfSolutionsRequired(trx.diversity().getNumberOfOptionsToGenerate()),
    _allSpecifiedCarrierOptionsMet(false)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic941)
    _dc941 = dynamic_cast<Diag941Collector*>(dc);
  else if (trx.diagnostic().diagnosticType() == Diagnostic942)
    _dc = dc;

  _requirements = &trx.dataHandle().safe_create<DmcRequirementsFacade>(stats, dc, trx);
  _discontinueProcessingValue = discontinueProcessingValue.getValue();

  initializeDiagnostic();
  initializeStatistic();
}

bool
DiversityModelBasic::continueProcessing(const MoneyAmount pqScore)
{
  bool result = false;
  bool pqScoreMet = pqScore > _diversityParams.getFareCutoffAmount();
  bool pqScoreMetExpand = false;

  _allSpecifiedCarrierOptionsMet = true;
  
  if (_diversityParams.hasDCL())
  {
    std::map<CarrierCode, size_t>::const_iterator dclit = _diversityParams.getDCLMap().begin();
    std::map<CarrierCode, size_t>::const_iterator dclitEnd = _diversityParams.getDCLMap().end();
    for (; dclit != dclitEnd; ++dclit)
    {
      size_t count = _stats.getNumOfItinsForCarrier(dclit->first);
      size_t required = (size_t)(dclit->second);
      if (count < required)
      _allSpecifiedCarrierOptionsMet = false;
    }

    pqScoreMetExpand = pqScore > _diversityParams.getFareCutoffAmountExpandPreferredCarrier();
  }

  if (fallback::fallbackExpandCarrierContinueProcessing(&_trx))
  {
    // Check for fare amount cut-off hit
    updateStatus();
    if (pqScoreMetExpand)
    {
      setFareCutoffReachedGetNeedToContinue();
    }
    else if (pqScoreMet && _allSpecifiedCarrierOptionsMet)
    {
      setFareCutoffReachedGetNeedToContinue();
    }
    else if (_allSpecifiedCarrierOptionsMet && _currentStatus == 0)
    {
      printContinueProcessing(result, false, true);
    }
    else
    {
      result = true;
      printContinueProcessing(result, false, false);
    }
  }
  else
  {
    // Check for fare amount cut-off hit
    if (pqScoreMetExpand || (pqScoreMet && _allSpecifiedCarrierOptionsMet))
    {
      setFareCutoffReached(pqScoreMet);
    }
    else
      result = true;

    printContinueProcessing(
        result, _isFareCutoffReached, !_allSpecifiedCarrierOptionsMet && pqScoreMet, pqScore);
  }
  if (isAnyBucketChanged())
  {
    _continueProcessingCount = 0;
  }

  if ((result == true) && (_continueProcessingCount++ >= _discontinueProcessingValue))
  {
    result = false; // return false so getPQItemAction returns STOP
    _continueProcessingCount = 0;
    printDiscontinueProcessing();
  }

  _newFarePath = false;

  return result;
}

bool
DiversityModelBasic::isAnyBucketChanged()
{
  bool rc = true;
  int currGold = _requirements->getBucketStatus(Diversity::GOLD);
  int currJunk = _requirements->getBucketStatus(Diversity::JUNK);
  int currUgly = _requirements->getBucketStatus(Diversity::UGLY);
  int currLux = _requirements->getBucketStatus(Diversity::LUXURY);

  if ((_lastBucketStatus.goldStatus == currGold) && (_lastBucketStatus.uglyStatus == currUgly) &&
      (_lastBucketStatus.luxuryStatus == currLux) && (_lastBucketStatus.junkStatus == currJunk))
  {
    rc = false;
  }
  else
  {
    _lastBucketStatus.goldStatus = currGold;
    _lastBucketStatus.uglyStatus = currUgly;
    _lastBucketStatus.luxuryStatus = currLux;
    _lastBucketStatus.junkStatus = currJunk;
  }
  return rc;
}

DiversityModel::PQItemAction
DiversityModelBasic::getPQItemAction(const shpq::SoloPQItem* pqItem)
{
  if (!_isParametersSet)
    return USE;

  updateNSParametersIfNeeded();
  if (!continueProcessing(pqItem->getScore()))
  {
    printPQItemAction(STOP, pqItem, 0);
    return STOP;
  }

  int couldSatisfy = _requirements->getPQItemCouldSatisfy(pqItem);
  PQItemAction result = couldSatisfy ? USE : SKIP;
  printPQItemAction(result, pqItem, couldSatisfy);
  return result;
}

bool
DiversityModelBasic::isNonStopNeededOnly()
{
  bool result = _currentStatus != 0 &&
                (_currentStatus & DmcRequirement::NEED_NONSTOPS_MASK) == _currentStatus;
  return result;
}

bool
DiversityModelBasic::isNonStopNeededOnlyFrom(const shpq::SoloPQItem* pqItem)
{
  // fast checks before go down to lengthy pqItemCouldSatisfy()
  bool isNSNeeded = (_currentStatus & DmcRequirement::NEED_NONSTOPS_MASK);
  if (!isNSNeeded)
  {
    return false;
  }
  if (isNonStopNeededOnly())
  {
    return true;
  }

  // both non-stop & other are needed, let us look into details for this particular fare path
  int couldSatisfy = _requirements->getPQItemCouldSatisfy(pqItem);
  bool couldSatisfyNSBits = (couldSatisfy & DmcRequirement::NEED_NONSTOPS_MASK);
  bool couldSatisfyOtherBits = (couldSatisfy & ~DmcRequirement::NEED_NONSTOPS_MASK);

  bool result = (couldSatisfyNSBits && !couldSatisfyOtherBits);
  return result;
}

bool
DiversityModelBasic::shouldPerformNSPreValidation(const shpq::SoloPQItem* pqItem)
{
  return isAdditionalNonStopEnabled();
}

DiversityModel::SOPCombinationList::iterator
DiversityModelBasic::getDesiredSOPCombination(SOPCombinationList& combinations,
                                              MoneyAmount score,
                                              size_t fpKey)
{
  switch (checkEvaluateNewCombinationPrerequisites(score))
  {
  case USE_ALL:
    preFilterSOPCombinationList(combinations);
    return combinations.begin();

  case USE_NONE:
    return combinations.end();

  default:
    break;
  }

  return DiversityModelBasic::getDesiredSOPCombinationImpl(combinations, score, fpKey);
}

int
DiversityModelBasic::getBucketStatus(const Diversity::BucketType bucket) const
{
  return _requirements->getBucketStatus(bucket);
}

bool
DiversityModelBasic::addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                                 ShoppingTrx::FlightMatrix& flightMatrix,
                                 size_t farePathKey,
                                 const DatePair* datePair)
{
  bool swapCondition = false;
  bool gotBasicNumberOfOptionsFilled =
      (_stats.getTotalOptionsCount() >=
       _numberOfSolutionsRequired + _stats.getAdditionalNonStopsCount());
  bool additionalNonStopCondition = gotBasicNumberOfOptionsFilled
                                        ? _requirements->checkAdditionalNonStopCondition(solution)
                                        : false;

  swapCondition = (!_requirements->isHdmNSActiveOnly() && gotBasicNumberOfOptionsFilled);

  if (swapCondition && !additionalNonStopCondition)
    _swapper.swapSolution(solution, farePathKey);

  const std::pair<ShoppingTrx::FlightMatrix::iterator, bool>& insertResult =
      flightMatrix.insert(solution);

  if (UNLIKELY(!insertResult.second))
    return false;

  int isBrandedFaresPath = _trx.getRequest()->isBrandedFaresRequest();
  int optionSatisfiesBuckets;

  if (UNLIKELY(isBrandedFaresPath))
  {
    optionSatisfiesBuckets = _requirements->getCombinationCouldSatisfy(solution.first, 0);
    _stats.addToBucketMatchingVec(std::make_pair(solution.first, optionSatisfiesBuckets));
  }

  if (additionalNonStopCondition)
  {
    _stats.addNonStopSolution(*insertResult.first);
    printAdditionalNonStopAdded(solution);
    print941AdditionalNSAdded(solution.first);
  }
  else
  {
    _stats.addSolution(*insertResult.first);
    print941NSAdded(solution.first);
  }

  if (!_isParametersSet && _stats.getTotalOptionsCount() == _numberOfSolutionsRequired)
    setupParameters();

  if (_isParametersSet)
  {
    _requirements->legacyAdjustCarrierRequirementsIfNeeded(_numberOfSolutionsRequired,
                                                           _stats.getTotalOptionsCount());
  }

  return true;
}

DiversityModel::SOPCombinationList::iterator
DiversityModelBasic::getDesiredSOPCombinationImpl(SOPCombinationList& combinations,
                                                  MoneyAmount score,
                                                  size_t fpKey)
{
  _newFarePath = true;
  printDesiredSOPCombination(fpKey, score, combinations.size());

  if (getCurrentStatus() == 0)
  {
    return combinations.end();
  }

  SOPCombinationList::iterator bestCombIt = combinations.takeCombinationFromCache();
  if (bestCombIt != combinations.end())
  {
    return bestCombIt;
  }

  int firstCouldSatisfy = 0;
  SOPCombinationList::iterator combIt = combinations.begin();
  // find first combination that satisfies any of requirements
  while (combIt != combinations.end() && firstCouldSatisfy == 0)
  {
    firstCouldSatisfy = _requirements->getCombinationCouldSatisfy(combIt->oSopVec, score);
    const bool isThrowAwayCombination = _requirements->getThrowAwayCombination(combIt->oSopVec);

    if (firstCouldSatisfy != 0 && !isThrowAwayCombination)
    {
      bestCombIt = combIt;
      ++combIt;
      break;
    }
    else
    {
      combIt = combinations.erase(combIt);
    }
  }

  if (bestCombIt == combinations.end())
  {
    return bestCombIt;
  }

  DsciDiversityModelBasicAdapter interpreter(_trx, _stats, _requirements, score, firstCouldSatisfy);

  // look for better combiantion
  while (combIt != combinations.end())
  {
    const DsciDiversityModelBasicAdapter::CompareResult compareResult =
        interpreter.compare(*combIt, *bestCombIt);

    const int combCouldSatisfy = interpreter.getLastObtainedCouldSatisfy();
    if (!combCouldSatisfy)
    {
      combIt = combinations.erase(combIt);
      continue;
    }

    if (compareResult == DsciDiversityModelBasicAdapter::GREATER)
    {
      bestCombIt = combIt;
      combinations.clearCombinationsCache();
    }
    else if (compareResult == DsciDiversityModelBasicAdapter::EQUAL)
    {
      combinations.addCombinationToCache(*combIt);
    }

    const bool isBetter = compareResult == DsciDiversityModelBasicAdapter::GREATER;
    printSOPCombinationEvaluation(fpKey, *combIt, isBetter, combCouldSatisfy, score);

    ++combIt;
  }

  return bestCombIt;
}

bool
DiversityModelBasic::getIsNewCombinationDesired(const SOPCombination& combination,
                                                MoneyAmount score)
{
  switch (checkEvaluateNewCombinationPrerequisites(score))
  {
  case USE_ALL:
    return !_requirements->getThrowAwayCombination(combination.oSopVec);
  case USE_NONE:
    return false;
  default:
    break;
  }

  return (_requirements->getCombinationCouldSatisfy(combination.oSopVec, score) != 0);
}

DiversityModelBasic::UseCombinations
DiversityModelBasic::checkEvaluateNewCombinationPrerequisites(MoneyAmount score)
{
  if (_stats.getTotalOptionsCount() < _numberOfSolutionsRequired)
    return USE_ALL;

  _requirements->setNumberSolutionsRequiredCollected(
      _numberOfSolutionsRequired, _stats.getTotalOptionsCount(), score);

  if (_stats.getTotalOptionsCount() == _numberOfSolutionsRequired)
  {
    // Check for fare amount cut-off hit
    bool reachedCutoffAmount = false;
    bool expandedCutOffAmountReached = false;
    if (!_diversityParams.hasDCL())
      reachedCutoffAmount = score > _diversityParams.getFareCutoffAmount();
    else
      expandedCutOffAmountReached =
          score > _diversityParams.getFareCutoffAmountExpandPreferredCarrier();

    if (reachedCutoffAmount || expandedCutOffAmountReached)
    {
      setFareCutoffReached(reachedCutoffAmount);
      if (!getCurrentStatus())
      {
        printContinueProcessing(false, true, false, score);
        return USE_NONE;
      }
    }
  }

  return USE_SOME;
}

void
DiversityModelBasic::preFilterSOPCombinationList(SOPCombinationList& combinations)
{
  while (!combinations.empty() &&
         _requirements->getThrowAwayCombination(combinations.begin()->oSopVec))
  {
    combinations.erase(combinations.begin());
  }
}

bool
DiversityModelBasic::isNonStopOptionNeeded() const
{
  bool result = true;
  if (!_diversityParams.isHighDensityMarket())
    result = (_stats.getNonStopsCount() == 0);

  return result;
}

bool
DiversityModelBasic::isAdditionalNonStopEnabled() const
{
  return _diversityParams.isAdditionalNonStopsEnabled();
}

bool
DiversityModelBasic::isAdditionalNonStopOptionNeeded() const
{
  bool result = (_requirements->getStatus() & DmcRequirement::NEED_ADDITIONAL_NONSTOPS);

  return result;
}

void
DiversityModelBasic::initializeDiagnostic()
{
  if (!_dc)
    return;

  const std::string& diagArg = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
  if (diagArg == "PQ")
    _diagPQ = true;
  else if (diagArg == "SOP")
    _diagSOP = true;
  else if (diagArg == "SOLUTIONS")
    _diagSolutions = true;
  else if (diagArg == "NONSTOPS")
    _diagNS = true;
  else if (diagArg == "ALL")
  {
    _diagPQ = true;
    _diagSOP = true;
    _diagSolutions = true;
    _diagNS = true;
  }

  *_dc << "Requirements bit explanation:\n";
  *_dc << "G - Need more gold solutions\n";
  *_dc << "U - Need more ugly solutions\n";
  *_dc << "L - Need more luxury solutions\n";
  *_dc << "J - Need more junk solutions\n";
  *_dc << "R - Need more custom routing and carrier solutions\n";
  *_dc << "C - Solution(s) from specific carrier(s) needed\n";
  *_dc << "A - Need more additional non-stop solutions\n";
  *_dc << "N - Need more non-stop solutions\n";
  *_dc << "S - Need more non-stop solutions for specific carrier(s)\n";
  *_dc << "O - Need all outbounds in the result set, but still some are missing\n";
  *_dc << "I - Need all inbounds in the result set, but still some are missing\n";
  *_dc << "Q - Need more Requesting Carrier Online Options\n";
  _dc->printLine();
  *_dc << "Carriers explanations: CXR[XXX/YYY]\n";
  *_dc << "CXR - carrier code\n";
  *_dc << "XXX - collected number of options per carrier\n";
  *_dc << "YYY - requested number of options per carrier\n";
  _dc->printLine();

  if (_diagNS || _diagSolutions || diagArg == "SWAPPER")
  {
    *_dc << "NS flag explanations: NS: [O/I/N]\n"
         << "O - online non stop\n"
         << "I - interline non stop\n"
         << "N - not a non stop\n";
    _dc->printLine();
  }

  if (_diagSOP)
  {
    _fpKey =
        strtoul(_trx.diagnostic().diagParamMapItem(Diagnostic::FARE_PATH).c_str(), nullptr, 16);
    *_dc << "Get desired SOP combination fields explanation:\n";
    *_dc << "R      - Whether combination was select as result\n";
    *_dc << "OUT    - Outbound SOP number\n";
    *_dc << "IN     - Inbound SOP number\n";
    *_dc << "S      - Combination status\n";
    *_dc << "\t- - All bits have passed\n";
    *_dc << "\tS - At least one of bits was skipped\n";
    *_dc << "RQBITS - Which diversity requirements this combination could satisfy\n";
    *_dc << "SCORE  - Score for requirements bit composition as octal number\n";
    *_dc << "DRTN   - Duration in minutes\n";
    *_dc << "CXR    - Carrier\n";
    *_dc << "N      - Non-stop travel\n";
    *_dc << "IBN    - OB/IB pairing value\n";
    *_dc << "TODB   - TOD bucket number\n";
    *_dc << "TODN   - Number of options needed for this TOD bucket\n";
    _dc->printLine();
  }
}

void
DiversityModelBasic::initializeStatistic()
{
  int32_t statEnabled = ItinStatistic::STAT_MIN_PRICE | ItinStatistic::STAT_AVG_PRICE |
                        ItinStatistic::STAT_MAX_PRICE | ItinStatistic::STAT_MIN_DURATION |
                        ItinStatistic::STAT_AVG_DURATION | ItinStatistic::STAT_NON_STOP_COUNT |
                        ItinStatistic::STAT_CARRIERS | ItinStatistic::STAT_TOD |
                        ItinStatistic::STAT_SOP_PAIRING;

  if (_diversityParams.getNumOfCustomSolutions())
    statEnabled |= ItinStatistic::STAT_CUSTOM_SOLUTION;

  if (_diversityParams.getNonStopOptionsPerCarrierEnabled())
    statEnabled |= ItinStatistic::STAT_NON_STOP_CARRIERS;

  statEnabled |= ItinStatistic::STAT_NON_STOP_CARRIERS;

  if (_diversityParams.getMaxLongConnectionSolutions())
    statEnabled |= ItinStatistic::STAT_LONG_CONNECTION;

  if (_trx.getRequest()->isBrandedFaresRequest())
  {
    statEnabled |= ItinStatistic::STAT_UNUSED_SOPS;
    statEnabled |= ItinStatistic::STAT_RC_ONLINES;
    statEnabled |= ItinStatistic::STAT_UNUSED_COMB;

    if (_trx.getRequest()->isAllFlightsRepresented())
    {
      statEnabled |= ItinStatistic::STAT_UNUSED_DIRECTS;
    }
  }

  _stats.setEnabledStatistics(statEnabled, this);
}

void
DiversityModelBasic::updateStatus()
{
  int status = _requirements->getStatus();

  if (_currentStatus != status)
  {
    std::swap(_currentStatus, status);
    printStatusChanged(status);
  }
}

bool
DiversityModelBasic::canProduceMoreNonStop() const
{
  return _requirements->canProduceMoreNonStop();
}

void
DiversityModelBasic::updateNSParametersIfNeeded()
{
  bool isNSInfoUpdateNeeded = _isParametersSet && isNonStopNeededOnly() && canProduceMoreNonStop();
  if (!isNSInfoUpdateNeeded)
    return;

  updateNSParametersUnconditionally();
}

void
DiversityModelBasic::updateNSParametersUnconditionally()
{
  bool isEffective = _diversityParams.updateNSBookingCodeInfo(_trx);
  if (UNLIKELY(isEffective && _dc))
  {
    *_dc << "Booking code validation update has caused total max non-stop options dropped to: "
         << "\n" << _diversityParams.getMaxOnlineNonStopCount() << " online options"
         << "\n" << _diversityParams.getMaxInterlineNonStopCount() << " interline options"
         << "\n";

    _requirements->printCarriersRequirements(true);
  }
}

int
DiversityModelBasic::getCurrentStatus()
{
  updateStatus();
  return _currentStatus;
}

bool
DiversityModelBasic::setFareCutoffReachedGetNeedToContinue()
{
  bool continueProcessing = false;
  if (!_isFareCutoffReached)
  {
    _isFareCutoffReached = true;
    _requirements->setFareCutoffReached();
    updateStatus();
    continueProcessing = (_currentStatus > 0);
    if (_dc)
    {
      *_dc << "Fare cut-off amount reached\n";
      printContinueProcessing(continueProcessing, true); // print requirements
    }
  }
  else
    continueProcessing = (_currentStatus > 0);

  return continueProcessing;
}

void
DiversityModelBasic::setFareCutoffReached(const bool isOldScore)
{
  _requirements->setFareCutoffReached();
  _isFareCutoffReached = true;
  if (_dc)
  {
    if (isOldScore)
      *_dc << "Fare cut-off amount reached\n";
    else
      *_dc << "Expanded fare cut-off amount reached\n";
  }
}

void
DiversityModelBasic::printContinueProcessing(const bool result,
                                             const bool fromSetFareCutoffReached,
                                             const bool carrierOptionNotMetButFarecutoffMet,
                                             const MoneyAmount pqScore)
{
  if (LIKELY(!_dc))
    return;
  if (!_newFarePath && !fromSetFareCutoffReached)
    return;

  getCurrentStatus();

  *_dc << "Continue Processing: " << (result ? "TRUE" : "FALSE") << " ";
  if (!fallback::fallbackExpandCarrierContinueProcessing(&_trx))
    *_dc << "PQ Score: " << pqScore << "\n";
  switch (_currentStatus)
  {
  case 0:
    *_dc << "All requirements met\n";
    break;
  default:
    *_dc << "Non-met requirements " << printStatus(_currentStatus) << "\n";
    if (!_requirements->isHdmNSActiveOnly())
    {
      _requirements->printRequirements();
    }
    if (carrierOptionNotMetButFarecutoffMet)
      *_dc << "\nFare cut off met but Carriers options is not met, continue\n";
    break;
  }
}

void
DiversityModelBasic::printDiscontinueProcessing()
{
  if (!_dc)
    return;

  *_dc << "\nDiscontinue from Processing: ";
  *_dc << "processed " << _continueProcessingCount << " times\n";
  switch (_currentStatus)
  {
  case 0:
    *_dc << "All requirements met\n";
    break;
  default:
    *_dc << "Non-met requirements " << printStatus(_currentStatus) << "\n";
    if (!_requirements->isHdmNSActiveOnly())
    {
      _requirements->printRequirements();
    }
    break;
  }
}

void
DiversityModelBasic::printPQItemAction(DiversityModel::PQItemAction result,
                                       const shpq::SoloPQItem* pqItem,
                                       int couldSatisfy) const
{
  if (LIKELY(!_diagPQ))
    return;

  *_dc << "PQ Item Action: ";
  switch (result)
  {
  case DiversityModel::USE:
    *_dc << "USE ";
    break;
  case DiversityModel::STOP:
    *_dc << "STOP";
    break;
  case DiversityModel::SKIP:
    *_dc << "SKIP";
    break;
  default:
    break;
  }
  *_dc << " " << pqItem->str(shpq::SoloPQItem::SVL_HEADINGONLY);
  if (result == DiversityModel::STOP)
  {
    *_dc << " All requirements met\n";
  }
  else
  {
    *_dc << " could satisfy " << printStatus(couldSatisfy);
    bool needHeader = true;
    for (const CarrierCode& cxr : pqItem->getApplicableCxrs())
    {
      if (cxr == Diversity::INTERLINE_CARRIER)
        continue;
      if (needHeader)
      {
        *_dc << "CXR:";
        needHeader = false;
      }
      *_dc << "  " << cxr;
    }
    *_dc << "\n";
  }
}

void
DiversityModelBasic::printDesiredSOPCombination(size_t fpKey, MoneyAmount price, size_t combCount)
    const
{
  if (LIKELY(!_diagSOP))
    return;

  *_dc << "Get desired SOP combination: " << '[' << std::hex << fpKey << std::dec << "] "
       << std::setw(7) << price << ' ' << combCount << " combination(s) to evaluate\n";

  if (fpKey == _fpKey)
  {
    *_dc << "R|OUT|";
    if (_trx.legs().size() == 2)
      *_dc << " IN|";
    *_dc << "S|";

    *_dc << std::setw(DmcRequirement::REQUIREMENTS_COUNT) << std::left << "RQBITS"
         << std::right; // restore previous

    *_dc << "|SCORE|DRTN|CXR|N|";
    if (_trx.legs().size() == 2)
      *_dc << "IBN|";
    *_dc << "TODB|TODN\n";
  }
}

void
DiversityModelBasic::printSOPCombinationEvaluation(size_t fpKey,
                                                   const DiversityModel::SOPCombination& comb,
                                                   bool isSelected,
                                                   int couldSatisfy,
                                                   MoneyAmount price) const
{
  if (LIKELY(!_diagSOP || fpKey != _fpKey))
    return;

  *_dc << (isSelected ? "* " : "  ") << std::setw(3)
       << ShoppingUtil::findSopId(_trx, 0, comb.oSopVec[0]);
  if (comb.oSopVec.size() == 2)
    *_dc << ' ' << std::setw(3) << ShoppingUtil::findSopId(_trx, 1, comb.oSopVec[1]);

  *_dc << ' ' << (comb.status == 0 ? '-' : comb.status) << ' ';

  if (couldSatisfy == -1)
    couldSatisfy = _requirements->getCombinationCouldSatisfy(comb.oSopVec, price);

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_trx, comb.oSopVec, &outbound, &inbound);

  CarrierCode cxr = SopCombinationUtil::detectCarrier(outbound, inbound);
  if (cxr.empty())
    cxr = ANY_CARRIER;

  *_dc << printStatus(couldSatisfy);

  // fit the next field under "SCORE" title
  *_dc << " " << std::setw(3) << "=" << std::setw(2) << std::oct << couldSatisfy << std::dec;

  *_dc << " " << std::setw(4) << SopCombinationUtil::getDuration(outbound, inbound) << "  " << cxr
       << ' ' << SopCombinationUtil::getDiagNonStopType(outbound, inbound) << ' ';

  if (comb.oSopVec.size() == 2)
    *_dc << std::setw(3) << _stats.getInboundOptionsNeeded(comb.oSopVec[0]) << ' ';

  const std::vector<std::pair<uint16_t, uint16_t>>& todRanges = _diversityParams.getTODRanges();
  size_t todBucket = outbound->itin()->getTODBucket(todRanges);

  *_dc << "   " << todBucket << ' ' << std::setw(4)
       << _stats.getOptionsNeededForTODBucket(todBucket) << "\n";
}

void
DiversityModelBasic::printAdditionalNonStopAdded(
    const ShoppingTrx::FlightMatrix::value_type& solution) const
{
  if (!_dc || !_diagNS)
    return;

  const std::vector<int>& sopVec = solution.first;

  *_dc << "Additional non stop combination: "
       << ShoppingUtil::findSopId(static_cast<PricingTrx&>(*_dc->trx()), 0, sopVec[0]);

  if (sopVec.size() == 2)
    *_dc << "x" << ShoppingUtil::findSopId(static_cast<PricingTrx&>(*_dc->trx()), 1, sopVec[1]);

  CarrierCode cxr = SopCombinationUtil::detectCarrier(_trx, sopVec);
  *_dc << " CXR: " << (cxr.empty() ? "**" : cxr) << std::endl;
}

void
DiversityModelBasic::printStatusChanged(int oldStatus) const
{
  if (!_dc)
    return;

  *_dc << "Status Changed: before " << printStatus(oldStatus) << ", after "
       << printStatus(_currentStatus) << "\n";
}

void
DiversityModelBasic::printSolutions() const
{
  if (!_dc)
    return;

  if (_isParametersSet)
    _requirements->printRequirements();

  if (!_diagSolutions)
    return;

  const char bucketLetters[] = "GULJ";

  *_dc << "OUT|";
  if (_trx.legs().size() == 2)
    *_dc << " IN|";
  *_dc << "CXR|NS|   PRICE|DRTN|DEPAT|BKT|TODB";
  if (_trx.legs().size() == 2)
    *_dc << "|PAIRING";
  if (_diversityParams.getMaxLongConnectionSolutions())
    *_dc << "|LC";
  *_dc << "|FP";
  *_dc << "\n";

  const ShoppingTrx::FlightMatrix& flightMatrix = _trx.flightMatrix();
  for (const auto& fm : flightMatrix)
  {
    const std::vector<int>& sopVec = fm.first;
    const MoneyAmount price = fm.second->getTotalNUCAmount();
    const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
    SopCombinationUtil::getSops(_trx, sopVec, &outbound, &inbound);
    CarrierCode cxr = SopCombinationUtil::detectCarrier(outbound, inbound);
    if (cxr.empty())
      cxr = ANY_CARRIER;

    *_dc << std::setw(3) << ShoppingUtil::findSopId(_trx, 0, sopVec[0]);
    if (_trx.legs().size() == 2)
      *_dc << ' ' << std::setw(3) << ShoppingUtil::findSopId(_trx, 1, sopVec[1]);
    *_dc << "  " << cxr << ' ' << std::setw(2)
         << SopCombinationUtil::getDiagNonStopType(outbound, inbound) << ' ' << std::setw(8)
         << std::fixed << std::setprecision(2) << price
         << std::resetiosflags(std::ios_base::floatfield) << ' ' << std::setw(4)
         << SopCombinationUtil::getDuration(outbound, inbound) << ' ' << std::setw(2)
         << std::setfill('0') << outbound->itin()->getDepartTime().hours() << ':' << std::setw(2)
         << outbound->itin()->getDepartTime().minutes() << std::setfill(' ') << std::setprecision(6)
         << ' ' << std::setw(3) << bucketLetters[_stats.detectBucket(outbound, inbound, price)]
         << ' ' << std::setw(4) << outbound->itin()->getTODBucket(_diversityParams.getTODRanges());
    if (_trx.legs().size() == 2)
    {
      *_dc << ' ' << std::setw(3) << std::right << _stats.getInboundCount(sopVec[0]) << std::setw(4)
           << ' ';
    }
    if (_diversityParams.getMaxLongConnectionSolutions())
    {
      *_dc << "  " << (ShoppingUtil::isLongConnection(_trx, sopVec) ? 'T' : 'F');
    }

    const shpq::SoloGroupFarePath& gfp = dynamic_cast<const shpq::SoloGroupFarePath&>(*fm.second);
    *_dc << " " << std::hex << gfp.getFPKey() << std::dec;

    *_dc << "\n";
  }
}

void
DiversityModelBasic::printSummary() const
{
  if (!_dc)
    return;

  *_dc << "Number of solutions generated: " << _stats.getTotalOptionsCount() << "\n";
  if (_trx.getRequest()->isBrandedFaresRequest())
  {
    *_dc << "Outbound sops used: " << _stats.getNumOfUniqueSops(0) << "/"
         << _trx.legs()[0].requestSops() << "\n";
    if (_trx.legs().size() > 1)
      *_dc << "Inbound sops used: " << _stats.getNumOfUniqueSops(1) << "/"
           << _trx.legs()[1].requestSops() << "\n";
    if (_trx.getRequest()->isAllFlightsRepresented())
    {
      *_dc << "Direct solutions used: " << _stats.getDirectOptionsCount() << "/"
           << _stats.getDirectOptionsCount() + _stats.getMissingDirectOptionsCount() << "\n";
    }
    *_dc << "Req.Carrier Online Combinations Generated/bucket goal : "
         << _stats.getRCOnlineOptionsCount() << "/";
    *_dc << _stats.getRCOnlineOptionsCount() + _stats.getMissingRCOnlineOptionsCount() << "\n";
  }
}

void
DiversityModelBasic::handlePQStop()
{
  // This update is needed mainly to display 942 diag info, which will correspond to 910 FOS diag
  updateNSParametersUnconditionally();
}

void
DiversityModelBasic::handlePQHurryOutCondition()
{
  // This update is needed mainly to display 942 diag info,  which will correspond to 910 FOS diag
  updateNSParametersUnconditionally();
}

std::unique_ptr<DiversityModelBasic::SOPInfos>
DiversityModelBasic::filterSolutionsPerLeg(MoneyAmount score,
                                           const SOPInfos& sopInfos,
                                           SOPInfos* thrownSopInfos)
{
  if (_stats.getTotalOptionsCount() < _numberOfSolutionsRequired)
  {
    return std::unique_ptr<DiversityModelBasic::SOPInfos>();
  }
  else
  {
    return _requirements->filterSolutionsPerLeg(score, sopInfos, thrownSopInfos);
  }
}

void
DiversityModelBasic::setupParameters()
{
  if (_dc)
  {
    *_dc << "Reached requested number of solutions, calculating diversity parameters\n";
  }

  MoneyAmount minPrice = _stats.getMinPrice();
  MoneyAmount avgPrice = _stats.getAvgPrice();
  MoneyAmount maxPrice = _stats.getMaxPrice();

  _diversityParams.setFareAmountSeparator(
      round((minPrice + (avgPrice - minPrice) * (avgPrice / maxPrice)) * 100.0) / 100.0);

  _diversityParams.setTravelTimeSeparator(calcTravelTimeSeparator());

  FareCutoffCalcMethod fareCutoff(*this);
  _diversityParams.setFareCutoffAmount(fareCutoff.getFareCutoffAmount());
  _diversityParams.setFareCutoffAmountExpandPreferredCarrier(
      fareCutoff.getFareCutoffAmountExpPrefCxr());

  int32_t statEnabled = ItinStatistic::STAT_NON_STOP_COUNT | ItinStatistic::STAT_CARRIERS |
                        ItinStatistic::STAT_TOD | ItinStatistic::STAT_BUCKETS |
                        ItinStatistic::STAT_BUCKETS_PAIRING |
                        ItinStatistic::STAT_BUCKETS_MIN_PRICE | ItinStatistic::STAT_SOP_PAIRING;

  if (_trx.getRequest()->isBrandedFaresRequest())
  {
    statEnabled |= ItinStatistic::STAT_UNUSED_SOPS | ItinStatistic::STAT_RC_ONLINES |
                   ItinStatistic::STAT_UNUSED_COMB;

    if (_trx.getRequest()->isAllFlightsRepresented())
    {
      statEnabled |= ItinStatistic::STAT_UNUSED_DIRECTS;
    }
  }

  if (_diversityParams.getNumOfCustomSolutions())
    statEnabled |= ItinStatistic::STAT_CUSTOM_SOLUTION;

  if (_diversityParams.getNonStopOptionsPerCarrierEnabled())
  {
    statEnabled |= ItinStatistic::STAT_NON_STOP_CARRIERS;
  }

  _stats.setEnabledStatistics(statEnabled, this);

  _isParametersSet = true;
  printParameters(fareCutoff);
  printSolutions();
}

int32_t
DiversityModelBasic::calcTravelTimeSeparator() const
{
  int32_t result;
  if (_diversityParams.getTravelTimeSeparatorCoefDefined())
  {
    result = (int32_t)(_diversityParams.getTravelTimeSeparatorCoef() *
                       DiversityUtil::getBestDuration(_trx));
  }
  else
  {
    const int32_t minDuration = _stats.getMinDuration();
    const double avgDuration = _stats.getAvgDuration();

    double divLogPart = travelTimeSeparatorCoef.getValue() / log10(minDuration);

    result = static_cast<int32_t>(
        minDuration + ((avgDuration - minDuration) * cos(minDuration / avgDuration) * divLogPart));
  }
  return result;
}

// Returns false if the multiplyer is not determined by this method.
bool
DiversityModelBasic::getFareCutoffMultiplier(const MoneyAmount minPrice, uint16_t& multiplier)
{
  std::vector<uint16_t> priceVec;
  std::vector<uint16_t> multiplierVec;

  for (const auto elem : cutoffPriceRange.getValue())
    priceVec.push_back(std::stoi(elem));
  for (const auto elem : cutoffCoeffRange.getValue())
    multiplierVec.push_back(std::stoi(elem));

  int index = -1;
  size_t i = 0;

  for (; i < priceVec.size(); i++)
  {
    if (minPrice <= priceVec[i])
    {
      index = i;
      multiplier = multiplierVec[i];
      break;
    }
  }

  return (index >= 0);
}

bool
DiversityModelBasic::getFareCutoffMultiplierExpPrefCxr(const MoneyAmount minPrice,
                                                       uint16_t& multiplier)
{
  std::vector<uint16_t> priceVec;
  std::vector<uint16_t> multiplierVec;

  for (const auto elem : cutoffPriceRangeExpandPrefCxr.getValue())
    priceVec.push_back(std::stoi(elem));
  for (const auto elem : cutoffCoeffRangeExpandPrefCxr.getValue())
    multiplierVec.push_back(std::stoi(elem));

  int index = -1;
  size_t i = 0;

  for (; i < priceVec.size(); i++)
  {
    if (minPrice <= priceVec[i])
    {
      index = i;
      multiplier = multiplierVec[i];
      break;
    }
  }

  return (index >= 0);
}

void
DiversityModelBasic::printParameters(const FareCutoffCalcMethod& fareCutoff) const
{
  if (!_dc)
    return;

  *_dc << "Setting up diversity parameters:\n";
  *_dc << std::fixed << std::setprecision(2);
  *_dc << "\tMinimal price: " << _stats.getMinPrice() << "\n";
  *_dc << "\tAverage price: " << _stats.getAvgPrice() << "\n";
  *_dc << "\tMaximal price: " << _stats.getMaxPrice() << "\n";
  *_dc << std::resetiosflags(std::ios_base::floatfield) << std::setprecision(6);
  *_dc << "\tMinimal duration: " << _stats.getMinDuration() << "\n";
  *_dc << "\tAverage duration: " << _stats.getAvgDuration() << "\n";
  *_dc << "\tBest duration:    " << DiversityUtil::getBestDuration(_trx) << "\n";
  *_dc << std::fixed << std::setprecision(2);
  *_dc << " Fare amount separator: " << _diversityParams.getFareAmountSeparator() << "\n";
  *_dc << " Travel time separator: " << _diversityParams.getTravelTimeSeparator() << "\n";
  *_dc << " Fare cut-off amount: " << _diversityParams.getFareCutoffAmount() << "\n";
  *_dc << " Expand Preferred CXR cut-off amount: "
       << _diversityParams.getFareCutoffAmountExpandPreferredCarrier() << "\n";

  if (fareCutoff.isPriceByMultiplierMethod())
  {
    *_dc << " Using fare cut-off coefficient: " << fareCutoff.getPriceByMultiplierCoefUsed()
         << "\n";
    *_dc << "                           from: " << fareCutoff.getSource() << "\n";

    *_dc << " Expand Preferred CXR cutoff coef: "
         << fareCutoff.getPriceByMultiplierCoefUsedExpPrefCxr() << "\n";
    *_dc << "                           from: " << fareCutoff.getSourceExpPrefCxr() << "\n";
  }
  else
  {
    *_dc << " Fare cut-off calculation method: " << fareCutoff.getSource() << "\n";
  }

  *_dc << std::resetiosflags(std::ios_base::floatfield) << std::setprecision(6);
}

void
DiversityModelBasic::print941NSAdded(const std::vector<int>& comb) const
{
  if (LIKELY(!_dc941))
    return;
  if (SopCombinationUtil::detectNonStop(_trx, comb) == SopCombinationUtil::NOT_A_NON_STOP)
    return;

  _dc941->printNonStopAction(Diag941Collector::ADD_NS, comb, _stats);
}

void
DiversityModelBasic::print941AdditionalNSAdded(const std::vector<int>& comb) const
{
  if (!_dc941)
    return;

  _dc941->printNonStopAction(Diag941Collector::ADD_ANS, comb, _stats);
}
}
