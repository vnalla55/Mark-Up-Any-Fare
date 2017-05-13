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
#include "Pricing/Shopping/Diversity/DiversityModelAltDates.h"

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "Diagnostic/Diag942Collector.h"
#include "Diagnostic/SopVecOutputDecorator.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic2.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/FarePathFactoryPQItem.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloADGroupFarePath.h"

#include <boost/format.hpp>

#include <iomanip>

#include <tr1/functional>

namespace tse
{

static const char* const ADD_SOLUTION_ACTION = "Added";
static const char* const SKIP_SOLUTION_ACTION = "Skipped";
static const char* const REMOVE_SOLUTION_ACTION = "Removed";

Logger
DiversityModelAltDates::_logger("atseintl.ShoppingPQ.DiversityModelAltDates");

DiversityModelAltDates::DiversityModelAltDates(ShoppingTrx& trx,
                                               ItinStatistic& stats,
                                               DiagCollector* dc)
  : _trx(trx),
    _stats(stats),
    _diversityParams(trx.diversity()),
    _dc(dc),
    _isFareCutOffUsed(trx.diversity().getAltDates()._fareCutoffCoef > 1.0),
    _isDatePairFareCutOffUsed(trx.diversity().getAltDates()._fareCutoffCoefDatePair > 1.0),
    _allDatePairsCount(trx.altDatePairs().size())
{
  _stats.setEnabledStatistics(ItinStatistic::STAT_MIN_PRICE, this);
  stats.setupAltDates(&trx.dataHandle().safe_create<AltDatesStatistic2>(trx, stats));

  initializeDiagnostic(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL));
  initializeDatePairFilt(trx.diagnostic().diagParamMapItem("DATEPAIR"));

  for (const ShoppingTrx::AltDatePairs::value_type& trxDatePairStruct : trx.altDatePairs())
  {
    const DatePair& datePair = trxDatePairStruct.first;

    if (checkDatePairFilt(datePair))
      continue;

    _datePairsToFulfill.insert(datePair);
  }

  printParameters();
}

bool
DiversityModelAltDates::checkDatePairFilt(DatePair datePair) const
{
  if (UNLIKELY((!_filterByDatePair.first.isEmptyDate() && datePair.first != _filterByDatePair.first) ||
      (!_filterByDatePair.second.isEmptyDate() && datePair.second != _filterByDatePair.second)))
  {
    return true;
  }

  return false;
}

DiversityModel::PQItemAction
DiversityModelAltDates::getPQItemAction(const shpq::SoloPQItem* pqItem)
{
  PQItemAction result = USE;

  boost::optional<Carrier> diagCxrCode;

  if (!continueProcessing(pqItem->getScore()))
  {
    result = STOP;
  }
  else
  {
    bool skip;
    Carrier cxrCode;
    std::tr1::tie(skip, cxrCode) = skipPQItem(*pqItem);

    if (UNLIKELY(skip))
      result = SKIP;
    diagCxrCode = cxrCode;
  }

  printPQItemAction(result, *pqItem, (diagCxrCode ? &diagCxrCode.get() : nullptr));

  _isNewFarePath = false;

  return result;
}

DiversityModel::SOPCombinationList::iterator
DiversityModelAltDates::getDesiredSOPCombination(SOPCombinationList& combinations,
                                                 MoneyAmount score,
                                                 size_t fpKey)
{
  SOPCombinationList::iterator combIt(combinations.begin());
  while (combIt != combinations.end())
  {
    IsSolutionNeededCheck combResult;
    if (combResult.examine(*this, *combIt, score).isNeeded())
      break;
    else
    {
      // Erase those elements in-place, because otherwise significant overhead is introduced
      // by unnecessary isSolutionNeeded() calls (+500% SoloItinGenerator::generateSolutions() CPU
      // time)
      combIt = combinations.erase(combIt);

      printSolutionAction(combResult, SKIP_SOLUTION_ACTION);
    }
  }

  return combIt;
}

bool
DiversityModelAltDates::getIsNewCombinationDesired(const SOPCombination& combination,
                                                   MoneyAmount score)
{
  return IsSolutionNeededCheck().examine(*this, combination, score).isNeeded();
}

bool
DiversityModelAltDates::continueProcessing(MoneyAmount pqScore)
{
  std::size_t numMaxFareLevelReached(0), numDatePairFareCutoffAmountReached(0);

  bool continueProcessing;
  const bool isFareCutoffReached = this->isFareCutoffReached(pqScore);
  if (isFareCutoffReached)
    continueProcessing = false;
  else
  {
    std::tr1::tie(numMaxFareLevelReached, numDatePairFareCutoffAmountReached) =
        _stats.getAltDates()->filterDatePairsBy(
            pqScore,
            _diversityParams.getAltDates()._fareLevelNumber,
            _diversityParams.getAltDates()._fareCutoffCoefDatePair,
            _datePairsToFulfill);
    continueProcessing = !_datePairsToFulfill.empty();
  }

  printContinueProcessing(continueProcessing,
                          numMaxFareLevelReached > 0,
                          isFareCutoffReached,
                          numDatePairFareCutoffAmountReached > 0,
                          pqScore);

  return continueProcessing;
}

std::pair<bool, DiversityModelAltDates::Carrier>
DiversityModelAltDates::skipPQItem(const shpq::SoloPQItem& pqItem) const
{
  bool skip = false;
  Carrier cxrCode;

  if (boost::optional<CxrSPInfo> cxrSPInfo = getCxrSPInfo(pqItem))
  {
    AltDatesStatistic::DatePairSkipCallback callback(std::tr1::bind(
        &DiversityModelAltDates::skipPQItemCallback, this, std::tr1::placeholders::_1));

    bool isNeeded = _stats.getAltDates()->findAnyDatePair(
        pqItem.getScore(), *cxrSPInfo, callback, _datePairsToFulfill);
    skip = !isNeeded;
    cxrCode = cxrSPInfo->_carrier;
  }

  return std::make_pair(skip, cxrCode);
}

// We don't check fare cut-off here as far we assume this check was applied before
// on _datePairsToFulfill via continueProcessing()
bool
DiversityModelAltDates::skipPQItemCallback(const AltDatesStatistic::Stat& stat) const
{
  bool isFirstCxrFareLevel = this->isFirstCxrFareLevel(stat);
  bool isOptionNeeded = this->isNumOptionNeeded(stat);

  bool skip = (!isFirstCxrFareLevel || !isOptionNeeded);
  return skip;
}

void
DiversityModelAltDates::traceDatePairsToFullfill()
{
  if (!_logger->isTraceEnabled())
    return;

  int datePairsToFullfill = _datePairsToFulfill.size();
  if (_datePairsToFulfillTraceOnly == 0 // not initialized
      ||
      datePairsToFullfill != _datePairsToFulfillTraceOnly)
  {
    LOG4CXX_TRACE(_logger,
                  datePairsToFullfill << "/" << _allDatePairsCount << " dates pairs to fulfill");

    _datePairsToFulfillTraceOnly = datePairsToFullfill;
  }
}

bool
DiversityModelAltDates::isFirstCxrFareLevel(const AltDatesStatistic::Stat& stat) const
{
  bool result(stat._firstCxrFareLevel.get_value_or(stat._fareLevel) == stat._fareLevel);
  return result;
}

bool
DiversityModelAltDates::isNumOptionNeeded(const AltDatesStatistic::Stat& stat) const
{
  bool isNeeded = (stat._numOpt < _diversityParams.getAltDates()._optionsPerCarrier);
  return isNeeded;
}

bool
DiversityModelAltDates::isFareCutoffReached(MoneyAmount price) const
{
  if (UNLIKELY(!_isFareCutOffUsed))
    return false;

  MoneyAmount minPrice = _stats.getMinPrice();
  const bool isMinNotInitialized = (price < minPrice);
  if (isMinNotInitialized)
    return false;

  return isFareCutoffReached(minPrice, price, _diversityParams.getAltDates()._fareCutoffCoef);
}

bool
DiversityModelAltDates::isDatePairFareCutoffReached(MoneyAmount datePairFirstFareLevelAmount,
                                                    MoneyAmount price) const
{
  if (UNLIKELY(!_isDatePairFareCutOffUsed))
    return false;

  const bool isNotInitialized = (datePairFirstFareLevelAmount < 0.);
  if (isNotInitialized)
    return false;

  return isFareCutoffReached(
      datePairFirstFareLevelAmount, price, _diversityParams.getAltDates()._fareCutoffCoefDatePair);
}

bool
DiversityModelAltDates::isFareCutoffReached(MoneyAmount min, MoneyAmount price, float coef)
{
  MoneyAmount fareCutoffAmount = min * coef;
  bool isFareCutoffReached = (fareCutoffAmount < price);
  return isFareCutoffReached;
}

bool
DiversityModelAltDates::addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                                    ShoppingTrx::FlightMatrix& flightMatrix,
                                    size_t farePathKey,
                                    const DatePair* datePair)
{
  // here we are double-checking, as solution price can change by PricingUnitRuleController
  IsSolutionNeededCheck checkResult;

  bool insOk = false;
  if (checkResult.examine(*this, solution, *datePair).isNeeded())
  {
    insOk = flightMatrix.insert(solution).second;
    TSE_ASSERT(insOk && "Adding the same solution twice");

    _stats.addSolution(solution, datePair);
  }

  printSolutionAction(checkResult, insOk ? ADD_SOLUTION_ACTION : SKIP_SOLUTION_ACTION);
  traceDatePairsToFullfill();
  _isNewFarePath = true;

  return insOk;
}

void
DiversityModelAltDates::printSolutions() const
{
  if (!_dc)
    return;

  AltDatesStatistic::DumpToDiagParam param(dumpToDiagParam());
  param.setEmptyMsg(AltDatesStatistic::DIAG_EMPTY_FINAL_MSG);
  param.datePairsToFulfill(&_datePairsToFulfill);

  _stats.getAltDates()->dumpToDiag(*_dc, param);
}

void
DiversityModelAltDates::removeUnwantedSolutions(ShoppingTrx::FlightMatrix& flightMatrix)
{
  // print diag message only if removed had an effect
  bool isEffective = false;
  AltDatesStatistic2& ads = dynamic_cast<AltDatesStatistic2&>(*_stats.getAltDates());

  ShoppingTrx::FlightMatrix::iterator removeIter(flightMatrix.begin());
  while (removeIter != flightMatrix.end())
  {
    const shpq::SoloADGroupFarePath& gfp(
        dynamic_cast<const shpq::SoloADGroupFarePath&>(*removeIter->second));

    IsSolutionNeededCheck checkResult;
    if (!checkResult.examine(*this, *removeIter, gfp._datePair, true).isNeeded())
    {
      if (!isEffective)
      {
        if (_dc && isTraceSolutionActionEnabled())
          *_dc << " -- REMOVE UNWANTED SOLUTIONS --\n";

        isEffective = true;
      }

      printSolutionAction(checkResult, REMOVE_SOLUTION_ACTION);

      ads.removeSolution(*removeIter, gfp._datePair);
      flightMatrix.erase(removeIter++);
    }
    else
    {
      ++removeIter;
    }
  }
}

AltDatesStatistic::Stat
DiversityModelAltDates::getStat(const DiversityModel::SOPCombination& comb, MoneyAmount score) const
{
  return _stats.getAltDates()->getStat(comb, score);
}

AltDatesStatistic::Stat
DiversityModelAltDates::getStat(const ShoppingTrx::FlightMatrix::value_type& solution,
                                const DatePair& datePair) const
{
  return _stats.getAltDates()->getStat(solution, datePair);
}

void
DiversityModelAltDates::printSolutionAction(
    const DiversityModelAltDates::IsSolutionNeededCheck& details, const char* action)
{
  if (LIKELY(!_dc || !isTraceSolutionActionEnabled()))
    return;

  if (!details.shallExplain())
    return;

  if ((action == SKIP_SOLUTION_ACTION) && !_diagSkips)
    return;

  *_dc << action << " " << details;
}

void
DiversityModelAltDates::initializeDiagnostic(const std::string& diagArg)
{
  if (!_dc)
    return;

  if (diagArg == "ALL")
  {
    _diagAll = true;
    _diagPQ = true;
    _diagSkips = true;
    _diagSops = true;
  }
  else if (diagArg == "PQ")
  {
    _diagPQ = true;
  }
  else if (diagArg == "SKIPS")
  {
    _diagSkips = true;
  }
  else if (diagArg == "SOPS")
  {
    _diagSops = true;
  }
}

void
DiversityModelAltDates::initializeDatePairFilt(const std::string& filtByDatePair)
{
  if (!filtByDatePair.empty())
  {
    std::istringstream ss(filtByDatePair);
    std::string date;
    if (ss >> date)
      _filterByDatePair.first = DateTime(date, /*min*/ 0);
    if (ss >> date)
      _filterByDatePair.second = DateTime(date, /*min*/ 0);
  }
}

void
DiversityModelAltDates::printContinueProcessing(bool resultIsContinue,
                                                bool isMaxFareLevelReached,
                                                bool isFareCutoffReached,
                                                bool isDatePairFareCutoffReached,
                                                MoneyAmount score) const
{
  if (LIKELY(!_dc || (resultIsContinue && !_isNewFarePath)))
    return;

  *_dc << "DM Continue Processing: " << (resultIsContinue ? "TRUE" : "FALSE");
  *_dc << "\n\tSolutions generated: " << _stats.getTotalOptionsCount();
  *_dc << "\n\tDates pairs to fulfill: " << _datePairsToFulfill.size() << "/" << _allDatePairsCount
       << "\n";

  if (!resultIsContinue)
  {
    if (isMaxFareLevelReached)
      *_dc << "\tMax fare level amount reached: " << score << "\n";
    if (isFareCutoffReached)
      *_dc << "\tFare cut-off amount reached: " << score << "\n";
    if (isDatePairFareCutoffReached)
      *_dc << "\tFare cut-off amount reached in all date pairs: " << score << "\n";
  }
  else if (_diagAll || _diagPQ || _diagSkips)
  {
    _stats.getAltDates()->dumpToDiag(*_dc,
                                     dumpToDiagParam().datePairsToFulfill(&_datePairsToFulfill));
  }
}

void
DiversityModelAltDates::printParameters() const
{
  if (!_dc)
    return;

  const Diversity::AltDatesParam& altDatesParam = _diversityParams.getAltDates();

  *_dc << "Setting up alt. dates diversity parameters:";
  *_dc << "\n\tFare levels: " << altDatesParam._fareLevelNumber
       << "\n\tOptions per carrier: " << altDatesParam._optionsPerCarrier
       << "\n\tFare level delta: " << std::fixed << std::setprecision(2)
       << _stats.getAltDates()->getFareLevelDelta();
  *_dc << "\n\tFare cut-off amount coefficient per date pair: "
       << altDatesParam._fareCutoffCoefDatePair;
  *_dc << "\n\tFare cut-off amount coefficient: " << altDatesParam._fareCutoffCoef << "\n";

  if (!_filterByDatePair.first.isEmptyDate() || !_filterByDatePair.second.isEmptyDate())
  {
    *_dc << "\tDate pair filter for options: " << _filterByDatePair.first.dateToIsoExtendedString()
         << "/" << _filterByDatePair.second.dateToIsoExtendedString() << "\n";
  }
}

void
DiversityModelAltDates::printPQItemAction(DiversityModel::PQItemAction result,
                                          const shpq::SoloPQItem& pqItem,
                                          const DiversityModelAltDates::Carrier* cxrCode) const
{
  if (LIKELY(!_dc))
    return;

  if (!_diagPQ)
    return;

  *_dc << "DM PQ Item Action: ";
  switch (result)
  {
  case DiversityModel::USE:
    *_dc << "USE";
    break;
  case DiversityModel::SKIP:
    *_dc << "SKIP";
    break;
  case DiversityModel::STOP:
    *_dc << "STOP";
    break;
  default:
    break;
  }
  *_dc << " " << pqItem.str(shpq::SoloPQItem::SVL_BARE);
  if (result == DiversityModel::SKIP && cxrCode != nullptr)
  {
    *_dc << ", carrier " << *cxrCode << " is already collected in all fare levels:\n";
    _stats.getAltDates()->dumpToDiag(
        *_dc, dumpToDiagParam().displayCarrier(cxrCode).datePairsToFulfill(&_datePairsToFulfill));
  }
  *_dc << "\n";
}

boost::optional<AltDatesStatistic::CxrSPInfo>
DiversityModelAltDates::getCxrSPInfo(const shpq::SoloPQItem& pqItem) const
{
  static const boost::optional<CxrSPInfo> nonCxrCapableItem;

  // we are stub
  return nonCxrCapableItem;
}

AltDatesStatistic::DumpToDiagParam
DiversityModelAltDates::dumpToDiagParam() const
{
  return AltDatesStatistic::DumpToDiagParam().displaySops(_diagSkips || _diagSops || _diagAll);
}

} // ns tse
