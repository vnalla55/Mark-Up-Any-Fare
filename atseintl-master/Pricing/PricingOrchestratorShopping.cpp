// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/Logger.h"
#include "Common/MatrixUtils.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/Diag993Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/CustomSolutionBuilder.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/MultiAirportAgent.h"
#include "Pricing/MultiAirportAgentFactory.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/RequiredNonStopsCalculator.h"
#include "Pricing/Shopping/FOS/AdditionalDirectFosGenerator.h"
#include "Pricing/Shopping/FOS/AdditionalDirectFosTaskScope.h"
#include "Pricing/Shopping/FOS/FosBoss.h"
#include "Pricing/Shopping/IBF/IbfData.h"
#include "Pricing/Shopping/IBF/IbfIsStatusWriter.h"
#include "Pricing/Shopping/IBF/IbfUtils.h"
#include "Pricing/Shopping/IBF/V2IbfManager.h"
#include "Pricing/Shopping/PQ/SoloOrchestrator.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/ShoppingPQ.h"
#include "Pricing/ShoppingSurcharges.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <boost/bind.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/tokenizer.hpp>

#include <sstream>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackNGSJCBPaxTuning);
FALLBACK_DECL(reworkTrxAborter);

namespace
{
ConfigurableValue<uint64_t>
minFamilySizeCfg("PRICING_SVC", "MIN_FAMILY_SIZE", 3);
ConfigurableValue<uint64_t>
minFamilySizeInterlineCfg("PRICING_SVC", "MIN_FAMILY_SIZE_INTERLINE");
ConfigurableValue<int32_t>
numEstimatesPerIterationCfg("PRICING_SVC", "ESTIMATES_PER_ITERATION", 3);
ConfigurableValue<uint32_t>
numEstimatesPerSolutionCfg("SHOPPING_OPT", "ESTIMATES_PER_SOLUTION", 20);
ConfigurableValue<uint32_t>
numEstimatesFltOnlySolFamilyCfg("SHOPPING_OPT", "ESTIMATES_FLT_ONLY_SOLUTION_FAMILY", 30);
ConfigurableValue<uint32_t>
additionalSolutionPercent("SHOPPING_OPT", "ADDITIONAL_SOLUTION_PERCENT", 10);
ConfigurableValue<std::string>
lookForAlternateAirport("SHOPPING_SVC", "LOOK_FOR_ALTERNATE_AIRPORT");
ConfigurableValue<ConfigSet<CarrierCode>>
isJcbCarrierCfg("SHOPPING_SVC", "IS_JCB_CARRIER");
}
bool operator<(ShoppingTrx::EstimateMatrix::iterator const& lhs,
               ShoppingTrx::EstimateMatrix::iterator const& rhs)
{
  return *lhs < *rhs;
}

bool operator<(ShoppingTrx::FlightMatrix::iterator const& lhs,
               ShoppingTrx::FlightMatrix::iterator const& rhs)
{
  return *lhs < *rhs;
}

namespace
{
struct ItinSwapper
{
  ItinSwapper(std::vector<Itin*>& a, std::vector<Itin*>& b) : _a(a), _b(b) { _a.swap(_b); }

  ~ItinSwapper() { _a.swap(_b); }

private:
  std::vector<Itin*>& _a;
  std::vector<Itin*>& _b;
};

bool
optionHasFare(const GroupFarePath* gfp)
{
  if (!gfp)
    return false;

  if (gfp->getTotalNUCAmount() == 1000000)
    return false;

  return true;
}

class FlightMatrixOption
{
  GroupFarePath* value;
  std::vector<int> key;
  MoneyAmount estimatedTax;

public:
  FlightMatrixOption(std::vector<int> k, GroupFarePath* gfp) : value(gfp), key(k), estimatedTax(0)
  {
    if ((nullptr != value) && !value->groupFPPQItem().empty())
    {
      FarePath* fp = (*(value->groupFPPQItem().begin()))->farePath();

      if (nullptr != fp)
        estimatedTax = fp->itin()->estimatedTax();
    }
  }

  std::vector<int> getKey() const { return key; }

  GroupFarePath* getGFP() const { return value; }

  MoneyAmount amount() const
  {
    if (nullptr == value)
      return 1000000; // in case of fltOnlySol return this high value to place the sol at the end

    return value->getTotalNUCAmount() + estimatedTax;
  }

  void setKey(std::vector<int>& vec) { key = vec; }

  class GreaterFMO
  {
  public:
    bool operator()(const FlightMatrixOption left, const FlightMatrixOption right) const
    {
      return (left.amount() > right.amount());
    }
  };
};

class FlightMatrixOptionPQ : public std::priority_queue<FlightMatrixOption,
                                                        std::vector<FlightMatrixOption>,
                                                        FlightMatrixOption::GreaterFMO>
{
};

typedef std::pair<const std::vector<int>, GroupFarePath*> FltMatrixPair;
typedef std::map<DatePair, FlightMatrixOptionPQ> FltMatrixSort;
typedef std::pair<const DatePair, FlightMatrixOptionPQ> FltMatrixSortPair;
typedef std::pair<FltMatrixSort*, ShoppingTrx*> DataPair1;
typedef std::pair<Diag910Collector*, ShoppingTrx*> DataPair2;

class ProcessFlightMatrixOption : public std::binary_function<FltMatrixPair&, DataPair1, void>
{
public:
  void operator()(FltMatrixPair& in, DataPair1 out) const
  {
    FltMatrixSort* fltMatrixOut = out.first;
    ShoppingTrx* trx = out.second;
    const std::vector<int>& sops = in.first;
    const DatePair& dp = ShoppingAltDateUtil::getDatePairSops(*trx, sops);

    if (fltMatrixOut->find(dp) == fltMatrixOut->end())
    {
      FlightMatrixOptionPQ pq = FlightMatrixOptionPQ();
      pq.push(FlightMatrixOption(in.first, in.second));
      (*fltMatrixOut)[dp] = pq;
    }
    else
    {
      (*fltMatrixOut)[dp].push(FlightMatrixOption(in.first, in.second));
    }
  }
};

class OptionClean : public std::binary_function<FltMatrixSortPair&, int, void>
{
public:
  void operator()(FltMatrixSortPair& in, int no) const
  {
    for (int count = no; (count > 0) && !in.second.empty(); --count)
      in.second.pop();
  }
};

class PurgeFlightMatrix
    : public std::binary_function<FltMatrixSortPair&, ShoppingTrx::FlightMatrix*, void>
{
public:
  void operator()(FltMatrixSortPair& in, ShoppingTrx::FlightMatrix* out) const
  {
    if (in.second.empty())
      return;

    FlightMatrixOptionPQ& pq = in.second;

    while (!pq.empty())
    {
      out->erase(pq.top().getKey());
      pq.pop();
    }
  }
};

class PrintExOptions : public std::binary_function<FltMatrixSortPair, DataPair2, void>
{
public:
  void operator()(FltMatrixSortPair in, DataPair2 d) const
  {
    *(d.first) << "OPTIONS REMOVED FOR " << in.first.first.dateToString(MMDDYY, "") << "-"
               << in.first.second.dateToString(MMDDYY, "") << " DATE PAIR\n";
    d.first->flushMsg();
    FlightMatrixOptionPQ pq = in.second;

    for (; !pq.empty(); pq.pop())
    {
      d.first->outputExcessiveOptions(*d.second, pq.top().getKey(), pq.top().getGFP());
    }
  }
};

struct EqualMatrixRater
{
  int operator()(const std::vector<int>& v) const { return 1; }
};

void
ibfGenerateDirectFos(ShoppingTrx& trx)
{
  trx.setIbfData(IbfData::createInitialIbfDataV2(trx));
  trx.getIbfData().getV2IbfManager().performInitialPOTasks();
}

} // namespace

void
PricingOrchestrator::doQueuePostprocessing(ShoppingTrx& trx,
                                           ShoppingTrx::ShoppingPQVector& shoppingPQVector,
                                           bool& mustHurry,
                                           tse::ConfigMan& _config,
                                           std::deque<Itin>& journeyItins,
                                           bool interlineQueueHasNoSolution)
{
  bool someCXRQueueHasSolution = false;
  bool someCXRQueueHasNoSolution = false;
  // get number of queue that is able to find more solution which has equal or more solution than it
  // is needed.
  // get more number of solution needed for each queue and send to process.
  int solutionsFound = 0;
  int estimatedSolutionsFound = 0;
  int numberOfValidQueues = 0;
  int nbrSolutionPerQueue(0);
  //****** START Analyze status after Q's processing ******
  //{
  // Check number of solutions found yet by each q
  // Check number of estimated solutions found yet by each q
  // If q were not able to find anything yet disable create more solutions flag
  // If q produced more or same as expected set flag that some Q has solutions
  // If any online Q has no solutions set flag that some cxr Q has no solutions
  for (const auto& shoppingPQTask : shoppingPQVector)
  {
    solutionsFound += shoppingPQTask->getFlightMatrix().size();
    estimatedSolutionsFound +=
        shoppingPQTask->getEstimateMatrix().size() - shoppingPQTask->numBadEstimates();

    if (shoppingPQTask->getFlightMatrix().empty())
    {
      shoppingPQTask->setCreateMoreSolution(false);
    }

    if (shoppingPQTask->getNoOfOptionsRequested() <= shoppingPQTask->getFlightMatrix().size())
    {
      someCXRQueueHasSolution = true;
      ++numberOfValidQueues;
    }
    else if (!someCXRQueueHasNoSolution && shoppingPQTask->getFlightMatrix().empty() &&
             !shoppingPQTask->isInterline())
    {
      someCXRQueueHasNoSolution = true;
    }
  }

  // If we have any Q with solutions
  //  - calculate average number of solutions to be found (to get more) per Q with solutions; 0 if
  // not solutions found
  nbrSolutionPerQueue =
      numberOfValidQueues > 0
          ? ((trx.getOptions()->getRequestedNumberOfSolutions() - solutionsFound) /
             numberOfValidQueues)
          : 0;

  if (numberOfValidQueues > 0 &&
      ((trx.getOptions()->getRequestedNumberOfSolutions() - solutionsFound) % numberOfValidQueues) >
          0)
  {
    ++nbrSolutionPerQueue;
  }
  //}
  //****** END Analyze status after Q's processing ******

  //****** START Get additional solutions from Q's ******
  //{
  // if we don't have as many solutions as we asked for, ask the queues
  // to provide additional solutions until we have as many as we wanted
  for (ShoppingTrx::ShoppingPQVector::reverse_iterator task = shoppingPQVector.rbegin();
       task != shoppingPQVector.rend() && !mustHurry &&
           solutionsFound < trx.getOptions()->getRequestedNumberOfSolutions() && !trx.isAltDates();
       ++task)
  {
    // if this queue has given us as many as it was asked for
    // previously, ask it for more to make up the shortfall
    const std::size_t taskSolutions = (*task)->getFlightMatrix().size();

    if ((*task)->getNoOfOptionsRequested() <= taskSolutions)
    {
      (*task)->getAdditionalSolutions(static_cast<uint32_t>(nbrSolutionPerQueue));
      solutionsFound += (*task)->getFlightMatrix().size() - taskSolutions;
    }

    if (fallback::reworkTrxAborter(&trx))
      mustHurry = checkTrxMustHurry(trx);
    else
      mustHurry = trx.checkTrxMustHurry();
  }
  //}
  //****** END Get additional solutions from valid Q's ******

  //****** START Get additional solutions with no fares from Q's (default GFP)******
  //
  uint32_t additonalSolutionPercent = additionalSolutionPercent.getValue();
  // If no solutions found yet then try to produce solutions with no fares from each Q
  if (solutionsFound == 0)
  {
    for (const auto& shoppingPQTask : shoppingPQVector)
    {
      // If estimates requested recalculate no of solutions we want from Q
      //
      if (trx.getRequestedNumOfEstimatedSolutions() > 0)
      {
        uint32_t additionalNoOptions =
            (shoppingPQTask->getNoOfOptionsRequested() * additonalSolutionPercent) / 100;
        uint32_t noSolution = additionalNoOptions > 0
                                  ? shoppingPQTask->getNoOfOptionsRequested() + additionalNoOptions
                                  : shoppingPQTask->getNoOfOptionsRequested() + 1;

        shoppingPQTask->setNoOfOptionsRequested(noSolution);
      }

      GroupFarePath* gfp = trx.dataHandle().create<GroupFarePath>();
      gfp->setTotalNUCAmount(1000000);
      shoppingPQTask->generateSolutionsWithNoFares(gfp);
      solutionsFound += shoppingPQTask->getFlightMatrix().size();
    }
  }
  else if (someCXRQueueHasNoSolution || interlineQueueHasNoSolution)
  {
    for (const auto& shoppingPQTask : shoppingPQVector)
    {
      if (shoppingPQTask->getFlightMatrix().empty())
      {
        if (shoppingPQTask->isInterline())
        {
          GroupFarePath* gfp = trx.dataHandle().create<GroupFarePath>();
          gfp->setTotalNUCAmount(1000000);
          (*(shoppingPQVector.back())).generateSolutionsWithNoFares(gfp);
          solutionsFound += shoppingPQTask->getFlightMatrix().size();
        }
        else
        {
          // create an default group fare path to be used if some cxr queue cannot find through fare
          // solution,
          // flight only solution will be used with the default group farepath
          GroupFarePath* gfp = trx.dataHandle().create<GroupFarePath>();
          gfp->setTotalNUCAmount(1000000);
          shoppingPQTask->generateCNXSolutionsWithNoFares(gfp);

          // for complex trip more than 4 legs
          // try to generate 1 flt only solution
          if ((trx.legs().size() > 3))
          {
            uint32_t noSolutionRequired = shoppingPQTask->getFlightMatrix().size() + 1;
            shoppingPQTask->setNoOfOptionsRequested(noSolutionRequired);
            shoppingPQTask->generateSolutionsWithNoFares(gfp);
          }
        }
      }
    }
  }
  //}
  //****** END Get additional solutions with no fares from Q's ******
  size_t minFamilySize = minFamilySizeCfg.getValue();
  size_t minFamilySizeInterline = minFamilySizeInterlineCfg.getValue();
  if (minFamilySizeInterlineCfg.isDefault())
    minFamilySizeInterline = minFamilySize;
  int numEstimatesPerIteration = numEstimatesPerIterationCfg.getValue();
  uint32_t numEstimatesPerSolution = numEstimatesPerSolutionCfg.getValue();
  uint32_t numEstimatesFltOnlySolFamily = numEstimatesFltOnlySolFamilyCfg.getValue();

  const int totalSolutionsFound = solutionsFound + estimatedSolutionsFound;
  printNumOption(trx, "FOUND FROM MAIN PROCESS", shoppingPQVector);
  //****** START Get estimated solutions ******
  //{
  int additionalEstimatesWanted =
      trx.getRequestedNumOfEstimatedSolutions() > 0
          ? trx.getRequestedNumOfEstimatedSolutions() - totalSolutionsFound
          : 0;

  if (shoppingPQVector.size() <= 0)
  {
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  uint32_t additionalEstimates =
      static_cast<uint32_t>(additionalEstimatesWanted) / shoppingPQVector.size();
  numEstimatesPerSolution =
      numEstimatesPerSolution > additionalEstimates ? numEstimatesPerSolution : additionalEstimates;
  uint32_t numGoodQueue = 0;

  //****** START Find valid Q's ******
  // Check if Q were able to find as many solutions as requested from it
  // Check if Q were able to find more solutions than estimated solutions requested from it then it
  // is valid Q
  for (const auto& shoppingPQTask : shoppingPQVector)
  {
    if (shoppingPQTask->getFlightMatrix().size() < shoppingPQTask->getNoOfOptionsRequested())
      shoppingPQTask->setCreateMoreSolution(false);

    uint32_t estReq = static_cast<uint32_t>(shoppingPQTask->getEstimatedSolutionsRequested());

    if (((shoppingPQTask->getEstimateMatrix().size() + shoppingPQTask->getFlightMatrix().size()) >=
         estReq) &&
        shoppingPQTask->createMoreSolution())
      ++numGoodQueue;
  }
  //****** END Find valid Q's ******

  bool allowToGetMore = true;
  bool foundNewEstimates = true;
  uint32_t shortEstOptions = 0;
  uint32_t shortEstOptionsPerQ = 0;
  uint32_t shortEstOptionsAccum = 0;
  uint32_t numGoodQueueAfterGetAddSol = 0;

  //****** START Produce estimated solutions ******
  while (!trx.isAltDates() && additionalEstimatesWanted > 0 && !mustHurry && foundNewEstimates &&
         allowToGetMore)
  {
    TSELatencyData metrics(trx, "PO GET ADDITIONAL OPTIONS");
    foundNewEstimates = false;

    if (numGoodQueue > 0)
      foundNewEstimates = true; // first time only

    if (numGoodQueueAfterGetAddSol > 0)
    {
      foundNewEstimates = true;
      numGoodQueue = numGoodQueueAfterGetAddSol;
      numGoodQueueAfterGetAddSol = 0;
    }

    shortEstOptions = static_cast<uint32_t>(additionalEstimatesWanted);

    if (numGoodQueue > 0)
      shortEstOptions = shortEstOptions / numGoodQueue;
    else
      shortEstOptions = shortEstOptions / shoppingPQVector.size();

    if (shortEstOptions < shoppingPQVector.size())
      shortEstOptions = static_cast<uint32_t>(additionalEstimatesWanted);

    shortEstOptions =
        (shortEstOptions * static_cast<uint32_t>(_hundredsOptionsReqAdjustPercent)) / 100;
    shortEstOptionsAccum = shortEstOptions;

    for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
         task != shoppingPQVector.end() && !mustHurry && allowToGetMore &&
             additionalEstimatesWanted > 0;
         ++task)
    {
      const std::size_t oldSolutions = (*task)->getFlightMatrix().size() +
                                       (*task)->getEstimateMatrix().size() -
                                       (*task)->numBadEstimates();
      uint32_t estReq = static_cast<uint32_t>((*task)->getEstimatedSolutionsRequested());

      if (((*task)->getNoOfOptionsRequested() <= (*task)->getFlightMatrix().size()) ||
          ((*task)->getEstimateMatrix().size() >= estReq))
      {
        if (interlineQueueHasNoSolution && someCXRQueueHasSolution == false)
        {
          (*task)->getMoreEstimatedFlightOnlySolutions();
          (*task)->getAdditionalSolutions(numEstimatesPerSolution);
        }
        else if (allowToGetMore)
        {
          if (!(*task)->createMoreSolution())
            continue;

          (*task)->getMoreEstimatedSolutions();
          (*task)->setMinimumFamilySize((*task)->isInterline() ? minFamilySizeInterline
                                                               : minFamilySize);
          shortEstOptionsPerQ =
              shortEstOptionsAccum + static_cast<uint32_t>((*task)->getEstimateMatrix().size() +
                                                           (*task)->getFlightMatrix().size());

          if (static_cast<int>(shortEstOptionsPerQ) > trx.getRequestedNumOfEstimatedSolutions())
          {
            shortEstOptionsPerQ = static_cast<uint32_t>(trx.getRequestedNumOfEstimatedSolutions());
          }

          (*task)->setEstimatedSolutionsRequested(shortEstOptionsPerQ);

          if (numGoodQueue > 0)
          {
            if (((*task)->getEstimateMatrix().size() + (*task)->getFlightMatrix().size()) >= estReq)
            {
              (*task)->generateEstimatedSolutions();

              if (((*task)->getEstimateMatrix().size() + (*task)->getFlightMatrix().size()) <
                  shortEstOptionsPerQ)
              {
                --numGoodQueue;
              }
            }
            else
            {
              shortEstOptionsAccum += shortEstOptions;
              continue;
            }
          }
          else
          {
            if ((*task)->getAdditionalSolutions(numEstimatesPerIteration))
            {
              foundNewEstimates = true;

              if (((*task)->getEstimateMatrix().size() + (*task)->getFlightMatrix().size()) >=
                  shortEstOptionsPerQ)
                ++numGoodQueueAfterGetAddSol;
            }
            else
            {
              (*task)->setCreateMoreSolution(false);
            }
          }
        }
      }
      else
      {
        shortEstOptionsAccum += shortEstOptions;
      }

      const std::size_t newSolutions = (*task)->getFlightMatrix().size() +
                                       (*task)->getEstimateMatrix().size() -
                                       (*task)->numBadEstimates();

      if (newSolutions > oldSolutions)
      {
        additionalEstimatesWanted -= (newSolutions - oldSolutions);
        foundNewEstimates = true;
      }

      allowToGetMore =
          ((trx.getRequestedNumOfEstimatedSolutions() * _hundredsOptionsResAdjustPercent) /
           RuleConst::HUNDRED_PERCENTS) >
                  (trx.getRequestedNumOfEstimatedSolutions() - additionalEstimatesWanted)
              ? true
              : false;
    }

    numGoodQueue = 0;
    if (fallback::reworkTrxAborter(&trx))
      mustHurry = checkTrxMustHurry(trx);
    else
      mustHurry = trx.checkTrxMustHurry();
  }

  //****** END Produce estimated solutions ******
  printNumOption(trx, "AFTER GETTING ADDITIONAL OPTIONS", shoppingPQVector);

  //****** START Cleanup locking flags on Q's ******
  for (const auto& shoppingPQTask : shoppingPQVector)
    shoppingPQTask->setCreateMoreSolution(true); // clean up existing set up before reuse

  //****** END Cleanup locking flags on Q's ******
  std::size_t fltonlySol = 0;
  foundNewEstimates = true;
  //****** START Get additional estimated solutions ******
  //{
  while (!trx.isAltDates() && additionalEstimatesWanted > 0 && allowToGetMore &&
         foundNewEstimates && fltonlySol < numEstimatesFltOnlySolFamily)
  {
    TSELatencyData metrics(trx, "PO GET FLIGHT ONLY OPTIONS");
    foundNewEstimates = false;
    numEstimatesPerSolution =
        numEstimatesPerSolution > additionalEstimatesWanted / shoppingPQVector.size()
            ? numEstimatesPerSolution
            : additionalEstimatesWanted / shoppingPQVector.size();

    for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
         task != shoppingPQVector.end() && !mustHurry && additionalEstimatesWanted > 0;
         ++task)
    {
      if (!(*task)->createMoreSolution())
        continue;

      std::size_t oldSolutions = (*task)->getFlightMatrix().size() +
                                 (*task)->getEstimateMatrix().size() - (*task)->numBadEstimates();
      std::size_t previousSol = (*task)->getFlightMatrix().size();
      (*task)
          ->setNoOfOptionsRequested((*task)->getFlightMatrix().size() + numEstimatesPerIteration);
      GroupFarePath* gfp = trx.dataHandle().create<GroupFarePath>();
      gfp->setTotalNUCAmount(1000000);
      (*task)->generateSolutionsWithNoFares(gfp);
      fltonlySol += (*task)->getFlightMatrix().size() - previousSol;

      if ((*task)->getFlightMatrix().size() - previousSol > 0)
      {
        (*task)->getMoreEstimatedFlightOnlySolutions();
        (*task)->getAdditionalSolutions(numEstimatesPerSolution);
      }

      const std::size_t newSolutions = (*task)->getFlightMatrix().size() +
                                       (*task)->getEstimateMatrix().size() -
                                       (*task)->numBadEstimates();

      if (newSolutions > oldSolutions)
      {
        additionalEstimatesWanted -= (newSolutions - oldSolutions);
        foundNewEstimates = true;
      }
      else
      {
        (*task)->setCreateMoreSolution(false);
      }
    }
  }

  printNumOption(trx, "AFTER GETTING ADDITIONAL ESTIMATED OPTIONS", shoppingPQVector);
  //}
  //****** END Get additional estimated solutions ******
  //}
  //****** END Get estimated solutions ******
  //****** START Get non stop solutions with no fares if not found yet ******
  //{
  bool directFltOnly = true;
  GroupFarePath* gfp = nullptr;

  for (const auto& shoppingPQTask : shoppingPQVector)
  {
    if (shoppingPQTask->foundNonStopOption())
      continue;

    shoppingPQTask->setNoOfOptionsRequested(shoppingPQTask->getFlightMatrix().size() +
                                            _maxDirectFlightOnlySolution);
    shoppingPQTask->generateSolutionsWithNoFares(gfp, directFltOnly);
  }

  printNumOption(trx, "AFTER GETTING ADDITIONAL DIRECT OPTIONS", shoppingPQVector);
  //}
  //****** END Get non stop solutions if not found yet ******
  if (fallback::reworkTrxAborter(&trx))
    mustHurry = checkTrxMustHurry(trx);
  else
    mustHurry = trx.checkTrxMustHurry();
  //****** START Get additional solution from same level GFP ******
  //{
  for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
       task != shoppingPQVector.end() && !mustHurry;
       ++task)
  {
    GroupFarePath* lastSolution = (*task)->getLastSolution();

    if (!lastSolution)
      continue;

    const MoneyAmount lastAmount = lastSolution->getTotalNUCAmount();
    GroupFarePathFactory* groupFarePathFactory = (*task)->getGroupFarePathFactory();

    if (groupFarePathFactory && groupFarePathFactory->isEqualToTopOrLastGroupFarePath(lastAmount))
    {
      lastSolution = (*task)->generateSolution(lastAmount);

      if (lastSolution && (lastAmount == lastSolution->getTotalNUCAmount()))
      {
        bool extraFarePath = true;
        (*task)->processSolution(lastSolution, extraFarePath);
      }
    }

    if (fallback::reworkTrxAborter(&trx))
      mustHurry = checkTrxMustHurry(trx);
    else
      mustHurry = trx.checkTrxMustHurry();
  }

  printNumOption(trx, "AFTER GETTING SAME LEVEL FP OPTIONS", shoppingPQVector);
  //}
  //****** END Get additional solution from same level GFP ******
  //****** START Get additional solution from GFP we already found (Search Beyond) ******
  //{
  if (!trx.isAltDates())
  {
    for (ShoppingTrx::ShoppingPQVector::iterator task = shoppingPQVector.begin();
         task != shoppingPQVector.end() && !mustHurry;
         ++task)
    {
      if ((*task)->isInterline())
        continue;

      (*task)->searchBeyondStateSet(true);
      std::vector<GroupFarePath*> results;
      (*task)->getSortedSolutionsGFP(results);

      if (results.empty())
        continue;

      std::vector<GroupFarePath*>::const_iterator it = results.begin();

      for (; it != results.end() && (*task)->needMoreSBSolutions() &&
                 !(*task)->checkProcessDoneWithCond() && !mustHurry;
           ++it)
      {
        (*task)->processSolution(*it, true);
        if (fallback::reworkTrxAborter(&trx))
          mustHurry = checkTrxMustHurry(trx);
        else
          mustHurry = trx.checkTrxMustHurry();
      }

      for (; (*task)->needMoreSBSolutions() && !(*task)->checkProcessDoneWithCond() && !mustHurry;)
      {
        (*task)->getSolutions();

        if (!(*task)->getLastSolution())
          break;

        if (fallback::reworkTrxAborter(&trx))
          mustHurry = checkTrxMustHurry(trx);
        else
          mustHurry = trx.checkTrxMustHurry();
      }

      (*task)->searchBeyondStateSet(false);
      if (fallback::reworkTrxAborter(&trx))
        mustHurry = checkTrxMustHurry(trx);
      else
        mustHurry = trx.checkTrxMustHurry();
    }

    printNumOption(trx, " AFTER SEARCH BEYOND", shoppingPQVector);
  }
  //}
  //****** END Get additional solution from GFP we already found (Search Beyond) ******

  {
    const std::string& cityList = lookForAlternateAirport.getValue();
    MultiAirportAgentFactory f;
    MultiAirportAgent* t = f.getAgent(cityList, trx);

    if (t)
      t->perform();
  }

  if (trx.isAltDates() && (trx.diagnostic().diagnosticType() == Diagnostic910))
  {
    ShoppingTrx::AltDateLowestAmount::iterator itr = trx.altDateLowestAmount().begin();
    ShoppingTrx::AltDateLowestAmount::iterator endItr = trx.altDateLowestAmount().end();
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic910);
    diag << "\nFINAL LOWEST AMOUNT FOR EACH DATE PAIR \n";

    for (; itr != endItr; ++itr)
    {
      const DatePair& dates = itr->first;
      diag << " LOWEST AMT PLUS TAX " << itr->second->lowestOptionAmount
           << " LOWEST AMT FOR SNOWMAN PLUS TAX " << itr->second->lowestOptionAmountForSnowman
           << " FOR DATE " << dates.first.dateToString(DDMMM, "") << "-"
           << dates.second.dateToString(DDMMM, "") << " \n";
    }

    diag.flushMsg();
  }

  if (trx.getNumOfCustomSolutions() > 0)
  {
    TSELatencyData metrics(trx, "PO GET MORE CUSTOM SOLUTIONS");

    //****** START getting Additional Custom solutions as needed ******
    CustomSolutionBuilder customSolutionBuilder(trx);

    if (trx.diagnostic().diagnosticType() == Diagnostic910)
    {
      Diag910Collector* const diag =
          dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(trx));
      TSE_ASSERT(diag != nullptr);
      (*diag) << "\nNUMBER OF CUSTOM SOLUTIONS REQUESTED: " << trx.getNumOfCustomSolutions()
              << "\n";
      (*diag) << "NUMBER OF CUSTOM SOLUTIONS FOUND BY REGULAR PROCESS: "
              << trx.pqDiversifierResult().currentCustomOptionCount() << "\n";
      diag->flushMsg();
    }
    customSolutionBuilder.getAdditionalSolutionsFromExistingGFP();
    printNumOption(trx, " AFTER ADDITIONAL CUSTOM SOLUTIONS FROM EXISTING GFP", shoppingPQVector);

    customSolutionBuilder.getAdditionalSolutionsFromQueue();
    printNumOption(trx, " AFTER ADDITIONAL CUSTOM SOLUTIONS FROM QUEUE", shoppingPQVector);

    customSolutionBuilder.getFlightOnlySolutions();
    printNumOption(trx, " AFTER ADDITIONAL FLIGHT ONLY CUSTOM SOLUTIONS", shoppingPQVector);
    //****** END getting Additional Custom solutions as needed ******
  }

  //****** START getting online FOS ******
  for (const auto& shoppingPQTask : shoppingPQVector)
  {
    if (!shoppingPQTask->foundOnlineOption())
    {
      GroupFarePath* gfp = trx.dataHandle().create<GroupFarePath>();
      gfp->setTotalNUCAmount(1000000);
      shoppingPQTask->generateCNXSolutionsWithNoFares(gfp, true, true);
    }
  }
  printNumOption(trx, " AFTER GETTING ONLINE FLIGHT ONLY SOLUTIONS", shoppingPQVector);
  //****** END getting online FOS ******

  //****** START getting Additional Non-Stop solutions as needed ******
  processNonStopShoppingQueue(trx, journeyItins, shoppingPQVector);
  //****** END getting Additional Non-Stop solutions as needed ******
}

/*-----------------------------------------------------------------------------
 * process function
 *
 * main Pricing Orchestrator process transaction method
 *
 * @param trx - reference to a valid transaction object
 * @return    - returns true on success, false on error
 *----------------------------------------------------------------------------*/
bool
PricingOrchestrator::process(ShoppingTrx& trx)
{
  LOG4CXX_INFO(_logger, "PO_process Shopping");

  if (PricingTrx::ESV_TRX == trx.getTrxType())
  {
    EstimatedSeatValue poEsv(trx);
    poEsv.process();
    return true;
  }

  if (trx.isSumOfLocalsProcessingEnabled())
  {
    shpq::SoloOrchestrator solo(trx, *this);
    solo.process();
    return true;
  }

  if (trx.diversity().isEnabled())
  {
    trx.diversity().initializeV2(trx);
  }

  // ShoppingUtil::removeFaresWithInvalidBitmap(trx);
  ShoppingUtil::orderSegsByLeg(trx);
  TSELatencyData metrics(trx, "PO PROCESS");

  if (trx.getRequest()->isParityBrandsPath() || trx.diversity().isExchangeForAirlines())
  {
    ibfGenerateDirectFos(trx);
    trx.interlineSolutionsOnly() = true;
  }

  // Call PQDiversifier
  _pqDiversifier.process(trx);
  ShoppingTrx::PQDiversifierResult& pqResult = trx.pqDiversifierResult();

  if (pqResult.validResults == false)
  {
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS,
                                 "INVALID DIVERSIFICATION RESULTS");
  }

  {
    ShoppingSurcharges surcharges(trx);
    surcharges.process();

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic993))
    {
      DCFactory* factory = DCFactory::instance();
      Diag993Collector* dc = static_cast<Diag993Collector*>(factory->create(trx));

      dc->enable(Diagnostic993);

      dc->printAllFareMarkets(trx);

      dc->flushMsg();
    }
  }

  // set things up so that during the processing, the empty Itin objects
  // are removed. After processing, the itins will be restored to their
  // original state.
  std::vector<Itin*> filteredItin;
  filteredItin.push_back(trx.journeyItin());
  const ItinSwapper swapper(filteredItin, trx.itin());
  LOG4CXX_INFO(_logger, "Num of Itin passed with Trx=" << trx.itin().size());
  bool mustHurry = false;

  // we want a copy of the journey itin for each PQ, so they can manipulate
  // the itin temporarily without affecting other threads
  // This container could not be destroyed if there is a chance any Q will be used
  std::deque<Itin> journeyItins;

  bool interlineQueueHasNoSolution = false;
  ShoppingTrx::ShoppingPQVector& shoppingPQVector = trx.shoppingPQVector();
  //****** START Q's processing ******
  //{
  trx.setOnlineShoppingPQProcessing();
  // --------  Process ONLine Queue ----------
  processOnlineShoppingQueue(trx, journeyItins, shoppingPQVector);
  trx.setOnlineShoppingPQProcessing(false);
  if (fallback::reworkTrxAborter(&trx))
    mustHurry = checkTrxMustHurry(trx);
  else
    mustHurry = trx.checkTrxMustHurry();
  // --------  Process InterLine Queue ----------
  processInterlineShoppingQueue(trx, journeyItins, interlineQueueHasNoSolution, shoppingPQVector);
  // --------  Process OneWay Fares Queue --------
  if (trx.getOptions()->isCarnivalSumOfLocal() && (trx.legs().size() >= 2))
  {
    processOwFaresShoppingQueue(trx);
  }
  //}
  //****** END Q's processing ******

  if (trx.getRequest()->isParityBrandsPath() || trx.diversity().isExchangeForAirlines())
  {
    LOG4CXX_INFO(_logger,
                 "SOP usage after queue processing: "
                     << trx.getIbfData().getV2IbfManager().getNbrOfUnusedSops() << "/"
                     << trx.getIbfData().getV2IbfManager().getNbrOfSops() << " unused SOPs");
    LOG4CXX_DEBUG(_logger,
                  "Sop usage counter after queue processing:\n"
                      << trx.getIbfData().getV2IbfManager().sopUsageToString());

    IbfIsStatusWriter ibfWriter(trx, trx.getIbfData().getV2IbfManager());

    ibfWriter.writeIsStatus(Diagnostic930, "status after queue processing");
    trx.getIbfData().getV2IbfManager().removeNonSwapperOptions();

    trx.getIbfData().getV2IbfManager().generateFinalFosSolutions();
    ibfWriter.writeIsStatus(Diagnostic930, "status after final FOS generation");
  }

  if (!trx.getRequest()->isParityBrandsPath())
  {
    doQueuePostprocessing(
        trx, shoppingPQVector, mustHurry, _config, journeyItins, interlineQueueHasNoSolution);
  }

  ShoppingTrx::FlightMatrix& flightMatrix = trx.flightMatrix();
  ShoppingTrx::EstimateMatrix& estimateMatrix = trx.estimateMatrix();
  //****** START Found solutions final processing and merging ******
  //{
  bool matchedJCB = ShoppingUtil::isJCBMatched(trx);

  int totalFamiliesAfter = 0;
  int totalFamiliesBefore = 0;
  // allow carrier listed in the IS_JCB_CARRIER config to try mip for jcb flight only solution
  // option.
  for (const auto& shoppingPQTask : shoppingPQVector)
  {
    if (fallback::fallbackNGSJCBPaxTuning(&trx))
    {
      bool jcbCarrierFound = false;

      if (matchedJCB)
      {
        if (shoppingPQTask->carrier())
        {
          jcbCarrierFound = isJcbCarrierCfg.getValue().has(*(shoppingPQTask->carrier()));
        }
        if (!jcbCarrierFound && !(shoppingPQTask->foundFarePathOption()))
          continue;
      }
    }
    else // fallbackNGSJCBPaxTuning is false, keep here
    {
      if (matchedJCB && !TrxUtil::isJcbCarrier(shoppingPQTask->carrier()) &&
          !shoppingPQTask->foundFarePathOption())
        continue;
    }

    totalFamiliesBefore += shoppingPQTask->getFlightMatrix().size();
    shoppingPQTask->propagateError();
    shoppingPQTask->removeBadEstimates();
    shoppingPQTask->removeHighAmountAltDates();
    shoppingPQTask->groupMother();

    // Change amount to zero in case there is only one option found in the queue and the option
    // must
    // be the flight only solution.
    shoppingPQTask->makeOptionHigherPriority();

    const ShoppingTrx::FlightMatrix& res = shoppingPQTask->getFlightMatrix();
    std::copy(res.begin(), res.end(), std::inserter(flightMatrix, flightMatrix.end()));
    const ShoppingTrx::EstimateMatrix& est = shoppingPQTask->getEstimateMatrix();
    std::copy(est.begin(), est.end(), std::inserter(estimateMatrix, estimateMatrix.end()));
  }

  removeDuplicatedSolutions(flightMatrix, estimateMatrix);

  totalFamiliesAfter = flightMatrix.size();
  const std::string strBefore = "TOTAL FAMILIES BEFORE GROUP MOTHER AND REMOVE BAD ESTIMATE ";
  printNumOption(trx, strBefore, totalFamiliesBefore);
  const std::string strAfter = "TOTAL FAMILIES AFTER GROUP MOTHER AND REMOVE BAD ESTIMATE ";
  printNumOption(trx, strAfter, totalFamiliesAfter);

  if (!trx.isAltDates() && trx.getRequestedNumOfEstimatedSolutions() > 0)
  {
    splitFamilies(trx);
    totalFamiliesAfter = flightMatrix.size();
    const std::string strBefore = "TOTAL FAMILIES AFTER SPLITTING FAMILIES BY DOMESTIC CNX TIME ";
    printNumOption(trx, strBefore, totalFamiliesAfter);
  }
  //}
  //****** END Found solutions final processing and merging ******
  //****** START AltDates solutions post processing and FOS generation ******
  //{
  if (trx.isAltDates())
  {
    removeExcessiveOptions(trx, flightMatrix);
    fillMissingAltDatePairsWithFOS(trx);
  } // if(trx.altDates())
  //}
  //****** END AltDates solutions post processing and FOS generation ******

  if (flightMatrix.empty())
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);

  if (trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry(false);
    else
      trx.setAbortOnHurry(false);
  }

  if (!trx.isAltDates())
    createItinTaxShopping(trx, filteredItin);

  // In case we found no more itins than options->numberOfSolutions() we need to price all of them
  // In order to do this, move all estimated solutions to flightMartix
  if (!trx.isAltDates() &&
      (static_cast<int>(trx.flightMatrix().size() + trx.estimateMatrix().size()) <=
       trx.getOptions()->getRequestedNumberOfSolutions()))
  {
    moveEstimatedSolutionsToFlightMatrix(trx);
  }

  saveNUCBaseFareAmount(trx);

  if (trx.getRequestedNumOfEstimatedSolutions() > 0)
  {
    addTaxAmount(trx);
    filteredItin.clear();
  }

  return true;
}

void
PricingOrchestrator::removeDuplicatedSolutions(ShoppingTrx::FlightMatrix& flightMatrix,
                                               ShoppingTrx::EstimateMatrix& estimateMatrix)
{
  for (ShoppingTrx::EstimateMatrix::iterator i = estimateMatrix.begin(); i != estimateMatrix.end();)
  {
    ShoppingTrx::EstimateMatrix::iterator next = i;
    ++next;
    ShoppingTrx::FlightMatrix::iterator flight = flightMatrix.find(i->first);

    if (flight != flightMatrix.end())
    {
      // prefer option with a fare, otherwise drop the estimate
      if (optionHasFare(i->second.second) && !optionHasFare(flight->second))
      {
        flightMatrix.erase(flight);
      }
      else
      {
        estimateMatrix.erase(i);
      }
    }

    i = next;
  }
}

void
PricingOrchestrator::saveNUCBaseFareAmount(ShoppingTrx& trx)
{
  for (ShoppingTrx::FlightMatrix::value_type& solution : trx.flightMatrix())
  {
    GroupFarePath* gfp(solution.second);

    if (gfp != nullptr)
      gfp->setTotalNUCBaseFareAmount(gfp->getTotalNUCAmount());
  }
  for (ShoppingTrx::EstimateMatrix::value_type& solution : trx.estimateMatrix())
  {
    GroupFarePath* gfp(solution.second.second);

    if (gfp != nullptr)
      gfp->setTotalNUCBaseFareAmount(gfp->getTotalNUCAmount());
  }
}

void
PricingOrchestrator::addTaxAmount(ShoppingTrx& trx)
{
  for (const auto& flightMatrixElement : trx.flightMatrix())
  {
    GroupFarePath* motherGroupFP = flightMatrixElement.second;

    if (motherGroupFP == nullptr)
      continue;

    if (motherGroupFP->groupFPPQItem().size() == 0 || motherGroupFP->getTotalNUCAmount() >= 1000000)
      continue;

    FarePath* farePath = motherGroupFP->groupFPPQItem().front()->farePath();
    Itin& curItin = *(farePath->itin());

    if (curItin.validatingCarrier().empty())
    {
      updateValidatingCarrier(trx, *farePath);
    }

    MoneyAmount taxAmount = getTax(trx, curItin, farePath);
    motherGroupFP->increaseTotalNUCAmount(taxAmount);
    farePath->increaseTotalNUCAmount(taxAmount);

    for (const auto& estimateMatrixElement : trx.estimateMatrix())
    {
      const std::vector<int>& childSolKey = estimateMatrixElement.second.first;
      const std::vector<int>& motherSolKey = flightMatrixElement.first;

      if (motherSolKey == childSolKey)
      {
        GroupFarePath* childGroupFP = estimateMatrixElement.second.second;
        childGroupFP->setTotalNUCAmount(motherGroupFP->getTotalNUCAmount());
      }
    }
  }
}

MoneyAmount
PricingOrchestrator::getTax(ShoppingTrx& trx, Itin& curItin, FarePath* farePath)
{
  MoneyAmount taxAmt = 0;

  if (curItin.getTaxResponses().empty())
  {
    TaxMap::TaxFactoryMap taxFactoryMap;
    TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);
    TaxItinerary taxItinerary;
    taxItinerary.initialize(trx, curItin, taxFactoryMap);
    taxItinerary.accumulator();
  }

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
  for (const auto taxItem : taxResponse->taxItemVector())
  {
    if (taxItem->taxAmount() != 0)
    {
      taxCurrencyCode = taxItem->paymentCurrency(); // assume all taxes have the same currency
      taxAmt += taxItem->taxAmount();
    }
  }

  CurrencyCode farePathCurrency = curItin.calculationCurrency();

  if (!taxCurrencyCode.empty() && farePathCurrency != taxCurrencyCode)
  {
    CurrencyConversionFacade ccFacade;
    Money fareCurrency(farePathCurrency);
    Money taxCurrency(taxAmt, taxCurrencyCode);

    if (ccFacade.convert(fareCurrency, taxCurrency, trx))
    {
      taxAmt = fareCurrency.value();
    }
  }

  return taxAmt;
}

//------------------------------------------------------------------------------------------------

void
PricingOrchestrator::moveEstimatedSolutionsToFlightMatrix(ShoppingTrx& trx)
{
  for (auto& estimatedSolution : trx.estimateMatrix())
  {
    std::pair<std::vector<int>, GroupFarePath*> flightMatrixEl(std::move(estimatedSolution.first),
                                                               estimatedSolution.second.second);
    trx.flightMatrix().insert(std::move(flightMatrixEl));
  }

  trx.estimateMatrix().clear();
}

//------------------------------------------------------------------------------------------------

inline bool
PricingOrchestrator::canUseMultiThreadPQ(ShoppingTrx& trx)
{
  uint32_t numberOfFaresToProcess = DELAY_VALIDATION_OFF;

  if (trx.isAltDates() && trx.paxType().size() <= 1)
  {
    std::string numberOfFaresToProcessStr;

    if (_config.getValue("SHOPPING_ALTDATE_NUMBER_OF_FARES_TO_PROCESS_THRESHOLD",
                         numberOfFaresToProcessStr,
                         "FARESV_SVC"))
    {
      numberOfFaresToProcess = atoi(numberOfFaresToProcessStr.c_str());
    }
  }

  return (numberOfFaresToProcess == DELAY_VALIDATION_OFF) && !trx.isIataFareSelectionApplicable();
}

void
PricingOrchestrator::processOnlineShoppingQueue(ShoppingTrx& trx,
                                                std::deque<Itin>& journeyItins,
                                                ShoppingTrx::ShoppingPQVector& tasks)
{
  const bool canUseMultiThread = canUseMultiThreadPQ(trx);

  ShoppingTrx::PQDiversifierResult& pqResult = trx.pqDiversifierResult();
  printAsDia910(trx, pqResult);

  if (pqResult.totalOnlineOptions <= 0)
    return;

  bool mustHurry = false;
  if (fallback::reworkTrxAborter(&trx))
    mustHurry = checkTrxMustHurry(trx);
  else
    mustHurry = trx.checkTrxMustHurry();

  // make it so that when the queues check for timing out, they will consider
  // the need to hurry to be a time out, as we don't want to hold up the
  // whole transaction just because one queue can't produce results in time.
  if (trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry();
    else
      trx.setAbortOnHurry();
  }

  TseRunnableExecutor shoppingPQExecutor(TseThreadingConst::SHOPPINGPQ_TASK);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  TSELatencyData metrics(trx, "PO ONLINE QUEUES");
  int numEstimateOnlineQ = trx.getRequestedNumOfEstimatedSolutions();
  int numEstimateEachFarePath = numEstimateOnlineQ;
  int totalAdjustedEstimatedOptions = 0;

  if (_hundredsOptionsReqAdjustPercent != 100 && numEstimateOnlineQ > 0)
  {
    const int totalNumberOptions = pqResult.totalOnlineOptions + pqResult.numberInterlineOptions;
    totalAdjustedEstimatedOptions = (numEstimateOnlineQ * _hundredsOptionsReqAdjustPercent) / 100;
    numEstimateEachFarePath = totalAdjustedEstimatedOptions / totalNumberOptions;
  }

  uint32_t numRemaining = pqResult.onlineCarrierList.size();

  for (const CarrierCode& cxr : pqResult.onlineCarrierList)
  {
    --numRemaining;

    const auto cxrIt = pqResult.cxrOnlinOptions.find(cxr);

    if (cxrIt == pqResult.cxrOnlinOptions.end())
      continue;

    ShoppingTrx::CxrOnlinOptions& cxrOnlinOptions = cxrIt->second;

    if (cxrOnlinOptions.numberOnlineOptions == 0)
      continue;

    if (_hundredsOptionsReqAdjustPercent != 100 && numEstimateOnlineQ > 0)
      numEstimateOnlineQ = numEstimateEachFarePath * cxrOnlinOptions.numberOnlineOptions;

    journeyItins.push_back(*trx.journeyItin());
    tasks.emplace_back(new ShoppingPQ(*this,
                                      trx,
                                      &journeyItins.back(),
                                      cxrOnlinOptions.numberOnlineOptions,
                                      numEstimateOnlineQ,
                                      &cxr,
                                      _bitmapOpOrderer,
                                      _factoriesConfig.searchAlwaysLowToHigh()));

    if (!mustHurry)
    {
      if (canUseMultiThread && numRemaining > 0)
        shoppingPQExecutor.execute(*tasks.back());
      else
        synchronousExecutor.execute(*tasks.back());
    }

    if (fallback::reworkTrxAborter(&trx))
      mustHurry = checkTrxMustHurry(trx);
    else
      mustHurry = trx.checkTrxMustHurry();
  }

  shoppingPQExecutor.wait();
}

//------------------------------------------------------------------------------------------------

void
PricingOrchestrator::processInterlineShoppingQueue(ShoppingTrx& trx,
                                                   std::deque<Itin>& journeyItins,
                                                   bool& interlineQueueHasNoSolution,
                                                   ShoppingTrx::ShoppingPQVector& tasks)
{
  const ShoppingTrx::PQDiversifierResult& pqResult = trx.pqDiversifierResult();

  if (pqResult.numberInterlineOptions == 0)
    return;

  TSELatencyData metrics(trx, "PO INTERLINE QUEUES");
  int numEstimateInterlineQ = trx.getRequestedNumOfEstimatedSolutions();

  if ((_hundredsOptionsReqAdjustPercent != 100) && (numEstimateInterlineQ > 0))
  {
    const int totalNumberOptions = pqResult.totalOnlineOptions + pqResult.numberInterlineOptions;
    const int totalAdjustedEstimatedOptions =
        (numEstimateInterlineQ * _hundredsOptionsReqAdjustPercent) / 100;
    const int numEstimateEachFarePath = totalAdjustedEstimatedOptions / totalNumberOptions;
    numEstimateInterlineQ = numEstimateEachFarePath * pqResult.numberInterlineOptions;
  }

  journeyItins.push_back(*trx.journeyItin());
  tasks.emplace_back(new ShoppingPQ(*this,
                                    trx,
                                    &journeyItins.back(),
                                    pqResult.numberInterlineOptions,
                                    numEstimateInterlineQ,
                                    nullptr,
                                    _bitmapOpOrderer,
                                    _factoriesConfig.searchAlwaysLowToHigh()));

  if (trx.getRequest()->isParityBrandsPath() || trx.diversity().isExchangeForAirlines())
  {
    // Use it while generating final FOS solutions
    trx.getIbfData().getV2IbfManager().setFlightMatrix(&tasks.back()->flightMatrix());

    // IBF: before interline processing, feed the interline
    // queue flight matrix with FOS solutions
    utils::addToFlightMatrix(tasks.back()->flightMatrix(),
                             trx.getIbfData().getV2IbfManager().getDirectFosSolutions());

    if (trx.getIbfData().getV2IbfManager().hasRequestedNbrOfSolutions())
    {
      // No need to run the queue since all options
      // have been generated using direct FOSes
      return;
    }
  }

  if (trx.isAltDates())
  {
    int numOfDatePairs = trx.altDatePairs().size();
    tasks.back()->setNoOfOptionsRequested(numOfDatePairs * pqResult.numberInterlineOptions);

    for (const auto& altDatePair : trx.altDatePairs())
    {
      ShoppingTrx::AltDateInfo* altDateInfo =
          &trx.dataHandle().safe_create<ShoppingTrx::AltDateInfo>();
      tasks.back()->altDatePairsPQ()[altDatePair.first] = altDateInfo;
      altDateInfo->numOfSolutionNeeded = pqResult.numberInterlineOptions;
    }
  }

  if ((fallback::reworkTrxAborter(&trx) ? !checkTrxMustHurry(trx) : !trx.checkTrxMustHurry()))
  {
    tasks.back()->run();

    if (tasks.back()->getFlightMatrix().empty()) // no interline solution found
    {
      interlineQueueHasNoSolution = true;
    }
  }
}

void
PricingOrchestrator::processOwFaresShoppingQueue(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "PO ONE-WAY QUEUE");
  int16_t numOwFareItins = trx.getOptions()->getAdditionalItinsRequestedForOWFares();
  int16_t fareCombRepeatLimit = trx.getOptions()->getMaxAllowedUsesOfFareCombination();

  if (numOwFareItins > 0)
  {
    Itin journeyItinClone = (*trx.journeyItin());
    ShoppingTrx::PQPtr pqPtr =
        ShoppingTrx::PQPtr(new ShoppingPQ(*this,
                                          trx,
                                          &journeyItinClone,
                                          numOwFareItins,
                                          0,
                                          nullptr,
                                          _bitmapOpOrderer,
                                          _factoriesConfig.searchAlwaysLowToHigh(),
                                          true,
                                          fareCombRepeatLimit));

    if ((fallback::reworkTrxAborter(&trx) ? !checkTrxMustHurry(trx) : !trx.checkTrxMustHurry()))
    {
      pqPtr->run();
    }

    const ShoppingTrx::FlightMatrix& flightMatrix = pqPtr->getFlightMatrix();
    std::copy(flightMatrix.begin(),
              flightMatrix.end(),
              std::inserter(trx.flightMatrix(), trx.flightMatrix().end()));
    const ShoppingTrx::EstimateMatrix& estimateMatrix = pqPtr->getEstimateMatrix();
    std::copy(estimateMatrix.begin(),
              estimateMatrix.end(),
              std::inserter(trx.estimateMatrix(), trx.estimateMatrix().end()));
  }
}

void
PricingOrchestrator::processNonStopShoppingQueue(ShoppingTrx& trx,
                                                 std::deque<Itin>& journeyItins,
                                                 ShoppingTrx::ShoppingPQVector& tasks)
{
  if (trx.isAltDates())
    return;

  const Diversity& diversity = trx.diversity();
  if (diversity.getNonStopOptionsCount() == 0u)
    return;

  TSELatencyData metrics(trx, "PO ADDITIONAL NONSTOP QUEUE");

  RequiredNonStopsCalculator requiredNSCalculator;
  requiredNSCalculator.init(trx);
  requiredNSCalculator.countAlreadyGeneratedNS(trx);
  std::size_t requiredNSCount = requiredNSCalculator.calcRequiredNSCount(trx);

  if (requiredNSCount == 0)
    return;

  journeyItins.push_back(*trx.journeyItin());
  tasks.emplace_back(new ShoppingPQ(*this,
                                    trx,
                                    &journeyItins.back(),
                                    requiredNSCount,
                                    0,
                                    nullptr,
                                    _bitmapOpOrderer,
                                    _factoriesConfig.searchAlwaysLowToHigh(),
                                    false,
                                    -1,
                                    true));

  tasks.back()->run();
  printNumOption(trx, "FOUND AFTER ADDITIONAL NONSTOP QUEUE PROCESSING", tasks);

  int fosToProduce = requiredNSCount - tasks.back()->getFlightMatrix().size() -
                     tasks.back()->getEstimateMatrix().size();

  if (fosToProduce <= 0)
    return;

  requiredNSCalculator.countAlreadyGeneratedNSPerCarrier(trx);
  std::map<CarrierCode, uint32_t> requiredNumPerCarrier;
  requiredNSCalculator.calcRequiredNSCountPerCarrier(trx, fosToProduce, requiredNumPerCarrier);

  fos::AdditionalDirectFosTaskScope taskScope(fosToProduce, requiredNumPerCarrier);
  fos::FosBoss fosBoss(trx);
  fosBoss.process<fos::AdditionalDirectFosGenerator>(taskScope);

  printNumOption(trx, "FOUND AFTER ADDITIONAL NONSTOP FOS PROCESSING", tasks);
}

inline static auto calcMissingSolutionsPerDatePair(ShoppingTrx& trx)
{
  boost::container::flat_map<DatePair, int32_t> missingSolutionsMap;

  for (const auto& dpInfo : trx.altDatePairs())
    missingSolutionsMap[dpInfo.first] = dpInfo.second->numOfSolutionNeeded;

  for (const ShoppingTrx::FlightMatrix::value_type& solution : trx.flightMatrix())
  {
    const SopIdVec& sops = solution.first;
    const DatePair dp = ShoppingAltDateUtil::getDatePairSops(trx, sops);
    --missingSolutionsMap[dp];
  }

  return missingSolutionsMap;
}

void
PricingOrchestrator::fillMissingAltDatePairsWithFOS(ShoppingTrx& trx)
{
  GroupFarePath& gfp = *trx.dataHandle().create<GroupFarePath>();
  gfp.setTotalNUCAmount(1000000);

  const auto missingSolutionsMap = calcMissingSolutionsPerDatePair(trx);

  for (const auto& dpCount : missingSolutionsMap)
  {
    const DatePair dp = dpCount.first;
    const int32_t count = dpCount.second;
    if (count <= 0)
      continue;
    generateAltDatePairSolutionWithNoFares(trx, dp, gfp, count);
  }
}

void
PricingOrchestrator::removeExcessiveOptions(ShoppingTrx& trx, ShoppingTrx::FlightMatrix& fltMatrix)
{
  if (!trx.isAltDates() || altDateOptionRemoveFactor() > 1.0)
    return;

  TSELatencyData metrics(trx, "PO EXCESSIVE OPTION REMOVE");
  FltMatrixSort optlist;
  DataPair1 helper(&optlist, &trx);
  uint32_t noOfExpectedSolutionsPerDate = static_cast<uint32_t>(
      ceil(trx.getOptions()->getRequestedNumberOfSolutions() * altDateOptionRemoveFactor()));
  uint32_t originalMatrixSize = fltMatrix.size();
  LOG4CXX_INFO(_logger, "No of expected solutions per date " << noOfExpectedSolutionsPerDate);
  // 1. Split datepairs from flight matrix into map and attach appropriate solutions per each date
  // pair
  std::for_each(
      fltMatrix.begin(), fltMatrix.end(), boost::bind(ProcessFlightMatrixOption(), _1, helper));
  // 2. Remove from the map the number of options we want to return to the client
  std::for_each(
      optlist.begin(), optlist.end(), boost::bind(OptionClean(), _1, noOfExpectedSolutionsPerDate));

  if (trx.diagnostic().diagnosticType() == Diagnostic910)
  {
    Diag910Collector* const diag =
        dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(trx));
    TSE_ASSERT(diag != nullptr);
    DataPair2 helper(diag, &trx);
    (*diag) << "\nREMOVED ALTERNATE DATES EXCESSIVE OPTIONS:\n";
    std::for_each(optlist.begin(), optlist.end(), boost::bind(PrintExOptions(), _1, helper));
    (*diag) << "\n";
    diag->flushMsg();
  }

  // 3. Remove options wich last in the map from the flight matrix
  std::for_each(optlist.begin(), optlist.end(), boost::bind(PurgeFlightMatrix(), _1, &fltMatrix));
  LOG4CXX_INFO(_logger,
               "Removed " << originalMatrixSize - fltMatrix.size()
                          << " options from flight matrix");
}

//------------------------------------------------------------------------------------------------
void
PricingOrchestrator::printNumOption(tse::ShoppingTrx& trx, const std::string& str, const int& opt)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic910)
    return;

  std::stringstream os;
  os << str << opt;
  printAsDia910(trx, os.str());
}

void
PricingOrchestrator::printNumOption(ShoppingTrx& trx,
                                    const char* const stage,
                                    const ShoppingTrx::ShoppingPQVector& shoppingPQVector)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic910)
    return;

  size_t currentSolutions = trx.flightMatrix().size() + trx.estimateMatrix().size();
  size_t totalFamilies = trx.flightMatrix().size();
  for (const ShoppingTrx::PQPtr& task : shoppingPQVector)
  {
    currentSolutions +=
        task->getFlightMatrix().size() + task->getEstimateMatrix().size() - task->numBadEstimates();
    totalFamilies += task->getFlightMatrix().size();
  }
  std::stringstream msg;
  msg << "TOTAL OPTIONS " << stage << " " << currentSolutions;
  printAsDia910(trx, msg.str());
  msg.str("");
  msg << "TOTAL FAMILIES " << stage << " " << totalFamilies;
  printAsDia910(trx, msg.str());
}

void
PricingOrchestrator::printAsDia910(ShoppingTrx& trx,
                                   const ShoppingTrx::PQDiversifierResult& pqResult)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic910)
    return;

  std::stringstream msg;
  msg << "NUMBER OF ONLINE OPTIONS PER PQ " << std::setw(3)
      << pqResult._numberOnlineOptionsPerCarrier << "\n"
      << "NUMBER OF INTERLINE OPTIONS     " << std::setw(3) << pqResult.numberInterlineOptions
      << "\n"
      << "NUMBER OF ONLINE CARRIERS       " << std::setw(3) << pqResult.onlineCarrierList.size()
      << "\n"
      << "TOTAL NUMBER OF OPTIONS         " << std::setw(3) << pqResult.totalOnlineOptions << "\n";
  printAsDia910(trx, msg.str());
}

void
PricingOrchestrator::printAsDia910(ShoppingTrx& trx, const std::string& msg)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  diag.enable(Diagnostic910);
  diag << msg << std::endl;
  diag.flushMsg();
}

void
PricingOrchestrator::createItinTaxShopping(tse::ShoppingTrx& trx, std::vector<Itin*>& filteredItin)
{
  filteredItin.clear();
  uint16_t itinSeq = 0;
  Itin& journeyItin = *trx.journeyItin();
  ShoppingTrx::FlightMatrix& flightMatrix = trx.flightMatrix();

  for (auto& fightMatrixElement : flightMatrix)
  {
    if ((fightMatrixElement.second == nullptr) ||
        (fightMatrixElement.second->getTotalNUCAmount() == 1000000) ||
        (fightMatrixElement.second->getTotalNUCAmount() == 0))
    {
      continue;
    }

    std::vector<FPPQItem*>::iterator it = fightMatrixElement.second->groupFPPQItem().begin();
    std::vector<FPPQItem*>::iterator itEnd = fightMatrixElement.second->groupFPPQItem().end();
    Itin* itin;
    trx.dataHandle().get(itin);
    filteredItin.push_back(itin);
    // update new itin
    itin->sequenceNumber() = itinSeq++;
    itin->geoTravelType() = journeyItin.geoTravelType();
    itin->ticketingCarrier() = journeyItin.ticketingCarrier();
    itin->setTravelDate(journeyItin.travelDate());
    itin->intlSalesIndicator() = journeyItin.intlSalesIndicator();
    itin->calculationCurrency() = journeyItin.calculationCurrency();
    itin->originationCurrency() = journeyItin.originationCurrency();
    // update travelseg
    std::vector<ShoppingTrx::Leg>& legs = trx.legs();
    std::vector<ShoppingTrx::Leg>::iterator legIter;
    std::vector<ShoppingTrx::Leg>::iterator legEndIter = legs.end();
    std::vector<uint32_t> sopIDVect;
    std::vector<int>::const_iterator i = fightMatrixElement.first.begin();
    std::vector<int>::const_iterator iE = fightMatrixElement.first.end();

    for (; i != iE; ++i)
    {
      sopIDVect.push_back(uint32_t(*i));
    }

    legIter = legs.begin();
    std::vector<uint32_t>::const_iterator idIt = sopIDVect.begin();
    std::vector<uint32_t>::const_iterator idItEnd = sopIDVect.end();

    for (uint32_t legID = 0; idIt != idItEnd && legIter != legEndIter; ++idIt, ++legIter, ++legID)
    {
      ShoppingTrx::Leg& curLeg = *legIter;
      ShoppingTrx::SchedulingOption& curSop = curLeg.sop()[size_t(*idIt)];
      Itin& sopItin = *curSop.itin();

      for (const auto travelSeg : sopItin.travelSeg())
      {
        AirSeg* airSeg = travelSeg->toAirSeg();

        if (airSeg)
          airSeg->segmentType() = Air;

        itin->travelSeg().push_back(travelSeg);
      }

      itin->legID().push_back(std::pair<int, int>(legID, (*idIt)));
    }

    FarePath* fpPtr = (*it)->farePath();
    fpPtr->itin() = itin;
    updateValidatingCarrier(trx, *fpPtr);

    for (; it != itEnd; ++it)
    {
      FarePath* fp = (*it)->farePath();
      fp->itin() = itin;
      itin->farePath().push_back(fp);
      fp->baseFareCurrency() = fp->itin()->originationCurrency();
      fp->calculationCurrency() = fp->itin()->calculationCurrency();
    }
  }
}

void
PricingOrchestrator::generateAltDatePairSolutionWithNoFares(ShoppingTrx& trx,
                                                            const DatePair& dp,
                                                            GroupFarePath& gfp,
                                                            uint32_t count)

{
  if (trx.legs().empty() || trx.legs().size() > 2 || count == 0)
    return;

  ShoppingTrx::FlightMatrix& flightMatrix = trx.flightMatrix();
  VITAValidator vitaValidator(trx);
  SopIdVec combination;

  const auto validateSop = [](const DateTime& dt, const ShoppingTrx::SchedulingOption& sop)
  {
    return !sop.getDummy() && sop.cabinClassValid() && ShoppingAltDateUtil::getDateSop(sop) == dt;
  };

  const auto validateCombination = [&](const SopIdVec& comb)
  {
    return !flightMatrix.count(comb) &&
           ShoppingUtil::checkMinConnectionTime(trx.getOptions(), comb, trx.legs()) &&
           vitaValidator(comb) && (trx.getOptions()->isEnableCalendarForInterlines() ||
                                   !ShoppingUtil::isSimilarOption(trx, flightMatrix, comb));
  };

  // OW-optimized part
  if (trx.legs().size() == 1)
  {
    const ShoppingTrx::Leg& leg = trx.legs().front();
    SopId sopId = 0;

    for (const ShoppingTrx::SchedulingOption& sop : leg.sop())
    {
      combination = {sopId++};
      if (!validateSop(dp.first, sop) || !validateCombination(combination))
        continue;
      flightMatrix[combination] = &gfp;
      if (--count == 0)
        return;
    }

    return;
  }

  // Multi-leg processing
  std::vector<SopId> sopMap[2];

  for (size_t legId = 0; legId != trx.legs().size(); ++legId)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legId];
    const DateTime& requiredDt = (legId == 0) ? dp.first : dp.second;
    SopId sopId = 0;

    for (const ShoppingTrx::SchedulingOption& sop : leg.sop())
    {
      if (validateSop(requiredDt, sop))
        sopMap[legId].push_back(sopId);
      ++sopId;
    }
  }

  const std::vector<SopId> dims = {(SopId)sopMap[0].size(), (SopId)sopMap[1].size()};

  for (MatrixRatingIterator<EqualMatrixRater> mri(dims); !mri.atEnd(); mri.next())
  {
    const auto& rawCombination = mri.value();
    combination = {sopMap[0][rawCombination[0]], sopMap[1][rawCombination[1]]};
    if (!validateCombination(combination))
      continue;
    flightMatrix[combination] = &gfp;
    if (--count == 0)
      return;
  }
}

void
PricingOrchestrator::splitFamilies(ShoppingTrx& trx)
{
  LOG4CXX_INFO(_logger, "Check families for domestic connection time integrity");
  FamiliesChildsVec familiesChilds;

  for (auto& flightMatrixElement : trx.flightMatrix())
  {
    FamilySopsVec familySopVec;

    const bool split = !checkDomesticCnxTimeIntegrity(trx, flightMatrixElement.first, familySopVec);
    if (split)
    {
      LOG4CXX_DEBUG(_logger, "Found non integral family");
      familiesChilds.push_back(familySopVec);
    }
  }

  if (familiesChilds.empty())
    return;

  LOG4CXX_INFO(_logger,
               "Found " << familiesChilds.size() << " families split by domestic connection time");
  ShoppingTrx::FlightMatrix newFamilyItems;
  ShoppingTrx::EstimateMatrix newEstimateItems;
  FamiliesChildsVecI fltItemsIt = familiesChilds.begin();

  for (; fltItemsIt != familiesChilds.end(); ++fltItemsIt)
  {
    createFamily(trx, *fltItemsIt, newFamilyItems, newEstimateItems);
  }

  LOG4CXX_INFO(_logger,
               "Created " << newFamilyItems.size() << " mothers and " << newEstimateItems.size()
                          << " childs");
  trx.flightMatrix().insert(newFamilyItems.begin(), newFamilyItems.end());
  trx.estimateMatrix().insert(newEstimateItems.begin(), newEstimateItems.end());
  LOG4CXX_INFO(_logger,
               "Total no of families after merge " << trx.flightMatrix().size() << " and "
                                                   << trx.estimateMatrix().size() << " childs");
}

void
PricingOrchestrator::createFamily(ShoppingTrx& trx,
                                  FamilySopsVec& itinElements,
                                  ShoppingTrx::FlightMatrix& family,
                                  ShoppingTrx::EstimateMatrix& estimates)
{
  if (itinElements.empty())
    return;

  LOG4CXX_DEBUG(_logger, "Creating new family from " << itinElements.size() << " elements");
  SopIdVec mother = itinElements.front();
  itinElements.erase(itinElements.begin());

  if (trx.estimateMatrix().count(mother) <= 0)
  {
    LOG4CXX_ERROR(_logger, "Estimate matrix does not contain expected element");
    return;
  }

  ShoppingTrx::EstimateMatrix::iterator it = trx.estimateMatrix().find(mother);
  const ShoppingTrx::FlightMatrix::value_type item(mother, it->second.second);
  family.insert(item);
  // Remove new mother from original estimateMatrix
  trx.estimateMatrix().erase(it);

  if (itinElements.empty())
    return;

  FamilySopsVecIC estListIt = itinElements.begin();

  for (; estListIt != itinElements.end(); ++estListIt)
  {
    it = trx.estimateMatrix().find(*estListIt);
    TSE_ASSERT(it != trx.estimateMatrix().end());
    const ShoppingTrx::EstimatedSolution estimate(mother, it->second.second);
    estimates[*estListIt] = estimate;
    // Remove new child from original estimateMatrix
    trx.estimateMatrix().erase(it);
  }
}

bool
PricingOrchestrator::checkDomesticCnxTimeIntegrity(const ShoppingTrx& trx,
                                                   const SopIdVec& key,
                                                   FamilySopsVec& childs)
{
  // Check cnx time change in the mother
  //  true - means cnx more than 4 hours apears
  bool mother = checkSopsCnxTimeMoreThan4hExist(trx, key);

  for (const auto& estimateMatrixElement : trx.estimateMatrix())
    if ((estimateMatrixElement.second.first == key) &&
        (mother != checkSopsCnxTimeMoreThan4hExist(trx, estimateMatrixElement.first)))
      childs.push_back(estimateMatrixElement.first);

  if (childs.empty())
    return true;
  else
    //  false means family is not integral
    return false;
}

bool
PricingOrchestrator::checkSopsCnxTimeMoreThan4hExist(const ShoppingTrx& trx, const SopIdVec& sops)
    const
{
  SopIdVecIC it = sops.begin();

  for (uint32_t idx = 0; it != sops.end(); ++it, ++idx)
  {
    if (((trx.legs()[idx]).sop()[size_t(*it)]).domesticCnxTimeMoreThan4() == true)
    {
      return true;
    }
  }

  return false;
}

} // namespace tse
