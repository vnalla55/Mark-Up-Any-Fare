//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/Shopping/IBF/V2IbfManager.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/ContainerSink.h"
#include "Pricing/Shopping/FiltersAndPipes/NamedPredicateWrapper.h"
#include "Pricing/Shopping/IBF/IbfDiag910Collector.h"
#include "Pricing/Shopping/IBF/IbfUtils.h"
#include "Pricing/Shopping/Predicates/SopIsDirect.h"
#include "Pricing/Shopping/Predicates/SopIsOnlineForCarrier.h"
#include "Pricing/Shopping/Swapper/PrioritySwapperFormatter.h"
#include "Pricing/Shopping/Utils/DiagLogger.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/Shopping/Utils/SopPenaltyGenerator.h"

#include <boost/unordered_set.hpp>

#include <iterator>
#include <sstream>

namespace tse
{

using namespace utils;

namespace
{
Logger
logger("atseintl.Pricing.IBF.V2IbfManager");
}

const size_t V2IbfManager::NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD = 700;
const size_t V2IbfManager::ACCUMULATED_NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD = 1000;
const size_t V2IbfManager::NO_PROGRESS_FOS_ITERATIONS_ABORT_THRESHOLD = 20000;
const size_t V2IbfManager::DEFAULT_QUEUE_ITERATIONS_LIMIT = 1000;


V2IbfManager::V2IbfManager(ShoppingTrx& trx,
                           unsigned int requestedNbrOfSolutions,
                           const std::string& requestingCarrierCode,
                           size_t queueIterationsLimit,
                           ILogger* logger)
  : _trx(trx),
    _matrix(nullptr),
    _factory(trx),
    _accumulatedQ0sNoprogressIterations(0),
    _queueIterationsLimit(queueIterationsLimit)
{

  if (logger != nullptr)
  {
    _log.install(logger);
  }

  TSE_ASSERT(requestedNbrOfSolutions > 0);
  _requirementsTracker = &trx.dataHandle().safe_create<IbfRequirementsTracker>(
      trx, requestedNbrOfSolutions, requestingCarrierCode, logger);
  TSE_ASSERT(_requirementsTracker != nullptr);

  const IbfRequirementsTracker::ImplSwapper& swp = _requirementsTracker->getSwapper();
  _pFormatter =
      &trx.dataHandle().safe_create<swp::PriorityScoreFormatter<utils::SopCombination>>(swp);
  TSE_ASSERT(_pFormatter != nullptr);

  std::ostringstream out;
  out << "Sop is online for requesting carrier = " << requestingCarrierCode;
  _rcOnlinesGenerator.reset(createFosPipelineGenerator("RC online generator", false));
  _rcOnlinesGenerator->addPredicate(utils::wrapPredicateWithName(
      &trx.dataHandle().safe_create<SopIsOnlineForCarrier>(trx, requestingCarrierCode),
      out.str(),
      trx));
  _rcOnlinesGenerator->addPredicate(_factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _rcOnlinesGenerator->addPredicate(
      _factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  initSopUsageProgressGenerator(false);

  initFosLoggers();
}

bool V2IbfManager::isComplexTripLogic() const
{
  //treat Exchange Trx like complex trip for better diversity
  if(!_trx.isNotExchangeTrx())
  {
    return true;
  }

  const unsigned int legsCount = getNonAsoLegsCount(_trx);
  if (legsCount <= SIMPLE_TRIP_MAX_LEGS)
  {
    return false;
  }

  return true;
}

SopUsageProgressGenerator* V2IbfManager::createSopUsageProgressGenerator()
{
  const unsigned int legsCount = getNonAsoLegsCount(_trx);
  if (!isComplexTripLogic())
  {
    return new SopUsageProgressGenerator(legsCount);
  }

  utils::DiagLogger* logger = new utils::DiagLogger(_trx, Diagnostic930);
  logger->setName("Sop usage progress generator");
  return new SopUsageProgressGenerator(legsCount,
      0, NO_PROGRESS_FOS_ITERATIONS_ABORT_THRESHOLD, logger);
}

void
V2IbfManager::initSopUsageProgressGenerator(bool allowNonMctOptions)
{
  _sopProgressGen.reset(createSopUsageProgressGenerator());
  _sopProgressGen->addPredicate(_factory.createFosPredicate(
          allowNonMctOptions ?
          tse::utils::POSITIVE_CONNECT_TIME :
          tse::utils::MINIMUM_CONNECT_TIME));
  _sopProgressGen->addPredicate(
      _factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  // we expect schedule repeat limit = 0 if disabled
  TSE_ASSERT(_trx.getRequest() != nullptr);
  const unsigned int srl = _trx.getRequest()->getScheduleRepeatLimit();
  if (srl != 0)
  {
    _log->info(Fmt("Enabling SRL and setting to: %u") % srl);
    _requirementsTracker->enableSrl(srl);
    _sopProgressGen->setSrl(srl);
  }
  else
  {
    _log->info("Not enabling SRL");
  }
}

void
V2IbfManager::initFosLoggers()
{
  TSE_ASSERT(_log.get() != nullptr);
  _asrFosBadElemsLogger.reset(
      new BadSopCombinationLogger(*_log, utils::LOGGER_LEVEL::INFO, "[ASR] Sop combination"));
  _directFosBadElemsLogger.reset(
      new BadSopCombinationLogger(*_log, utils::LOGGER_LEVEL::INFO, "[DIRECT] Sop combination"));
  _rcoFosBadElemsLogger.reset(
      new BadSopCombinationLogger(*_log, utils::LOGGER_LEVEL::INFO, "[RCO] Sop combination"));
  _q0sFosBadElemsLogger.reset(
      new BadSopCombinationLogger(*_log, utils::LOGGER_LEVEL::INFO, "[Q0S] Sop combination"));
}

utils::FosPipelineGenerator* V2IbfManager::createFosPipelineGenerator(
    const std::string& loggerName, bool bounded)
{

  const unsigned int legsCount = getNonAsoLegsCount(_trx);

  // We never want bounded generator for complex trips in the new code.
  // In the old code or simple trips, we let the user decide.
  if (isComplexTripLogic())
  {
    bounded = false;
  }

  unsigned int reqNbrOfSolutions = 0; // generate all combinations
  if (bounded)
  {
    reqNbrOfSolutions = _requirementsTracker->getRequestedNbrOfSolutions();
  }

  if (!isComplexTripLogic())
  {
    return new utils::FosPipelineGenerator(legsCount, reqNbrOfSolutions);
  }

  utils::DiagLogger* logger = new utils::DiagLogger(_trx, Diagnostic930);
  logger->setName(loggerName);
  BasicSopPenaltyGenerator* penaltyGenerator = new BasicSopPenaltyGenerator(
      _requirementsTracker->getSwapper(),
      _requirementsTracker->getSopUsageTracker(),
      logger);

  for (const auto& leg : _trx.legs())
  {
    if (leg.isReturnAllFlights())
    {
      penaltyGenerator->setReturnAllFlightLegID(leg.originalId());
      break;
    }
  }

  utils::DiagLogger* pipelineLogger = new utils::DiagLogger(_trx, Diagnostic930);
  pipelineLogger->setName("Pipeline generator");
  return new FosPipelineGenerator(legsCount,
      reqNbrOfSolutions,
      NO_PROGRESS_FOS_ITERATIONS_ABORT_THRESHOLD,
      pipelineLogger,
      penaltyGenerator);
}

void
V2IbfManager::performInitialPOTasks()
{
  IbfDiag910Collector coll(_trx, "Direct Combinations as Flight Only Solutions");
  collectValidSopsFromTrx(&coll);
  fetchRcOnlinesGenerator();

  if(_trx.getRequest()->isAllFlightsRepresented())
  {
    initDirectFosGenerator(&coll);
  }
  else
  {
    generateInitialDirectFos(&coll);
  }
  estimateRCOnlinesDesiredCount();

  if (_log->enabled(utils::LOGGER_LEVEL::INFO))
  {
    std::ostringstream out;
    out << "Appraiser scores format:" << std::endl;
    out << std::endl;
    swp::printBasShortFormatDescription(out);
    out << std::endl;
    _log->info(out.str());
  }

  coll.flush();

  _requirementsTracker->getSwapper().resetNoProgressIterationsCount();

}

void
V2IbfManager::collectValidSopsFromTrx(IbfDiag910Collector* coll)
{
  _log->info("Collecting valid sops from trx");
  ContainerSink<SopCandidate, IbfUtils::SopCandidateOutputIterator> sink(
      std::back_inserter(_validSops));

  // Only if details are enabled, we want to
  // gather info on SOPs discarded due to cabin validation
  if ((coll != nullptr) && (coll->areDetailsFor910Enabled()))
  {
    IbfUtils::collectCabinClassValidSops(_trx, sink, coll);
  }
  else
  {
    IbfUtils::collectCabinClassValidSops(_trx, sink);
  }

  if (_trx.getRequest()->getContextShoppingRequestFlag())
  {
    uint16_t returnAllFlightsIndex = 0;
    for( const auto& leg : _trx.legs())
    {
      if (leg.isReturnAllFlights())
        break;

      ++returnAllFlightsIndex;
    }

    _requirementsTracker->setLegIdToTrack(returnAllFlightsIndex);
  }

  // Inform IbfRequirementsTracker on valid SOPs
  for (unsigned int i = 0; i < _validSops.size(); ++i)
  {
    const SopCandidate& sopInfo = _validSops[i];
    _requirementsTracker->addSopForUsageTracking(sopInfo.legId, sopInfo.sopId);
  }
  LOG4CXX_DEBUG(logger,
                "Sop usage counter after initial fill:\n"
                    << sopUsageToString());
  _requirementsTracker->informSrlAppraiserOnSopCounts();
}

void
V2IbfManager::fetchRcOnlinesGenerator()
{
  _log->debug("Fetching RC online generator");
  IbfUtils::loadGeneratorWithSops(*_rcOnlinesGenerator, _validSops);
}

void
V2IbfManager::generateInitialDirectFos(IbfDiag910Collector* coll)
{
  _log->info("Generating initial direct FOS solutions");

  std::shared_ptr<FosPipelineGenerator> generator(
      createFosPipelineGenerator("Initial direct fos generator"));

  generator->addPredicate(
      utils::wrapPredicateWithName(_trx.dataHandle().create<SopIsDirect>(), "Sop is direct", _trx));

  generator->addPredicate(_factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  generator->addPredicate(_factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  TSE_ASSERT(_directFosBadElemsLogger.get() != nullptr);
  generator->addObserver(_directFosBadElemsLogger.get());

  IbfUtils::loadGeneratorWithSops(*generator, _validSops);

  if (coll && (coll->areDetailsFor910Enabled()))
  {
    // Tell diagnostic 910, how many direct SOPs are on each leg
    // (in the generator)
    for (unsigned int i = 0; i < generator->getNumberOfLegs(); ++i)
    {
      coll->setNumberOfSopsForLeg(i, generator->getSopsNumberOnLeg(i));
    }

    // Gather info on SOP combinations discarded due to
    // interline ticketing agreement or minimum connect time

    // We must tell compiler explicitly which interface
    // we will use: ICollector for SOPs or SOP combinations
    generator->addObserver(static_cast<IFilterObserver<SopCombination>*>(coll));
  }

  LOG4CXX_INFO(logger, "Starting generation FOS for IBF in V2");

  boost::unordered_set<SopCombination> uniqueCombinations;
  const bool complexTripLogic = isComplexTripLogic();

  // When generating FOS solutions, in addition collect them:
  // a) IbfRequirementsTracker tracks which SOPs are already used
  // b) diag 910 Collector makes list of generated FOSes
  utils::SopCombination sc = generator->next();
  while (!sc.empty())
  {
    if (complexTripLogic)
    {
      if (uniqueCombinations.find(sc) != uniqueCombinations.end())
      {
        sc = generator->next();
        continue;
      }
      uniqueCombinations.insert(sc);
    }

    _directFosSolutions.push_back(sc);
    _requirementsTracker->newDirectFosSolution(sc);
    if (coll)
    {
      coll->collect(sc);
    }

    if (complexTripLogic)
    {
      // Generator has no upper limit on the number
      // of produced combinations. We check if we
      // produced the desired number of options at this point
      // so we can break.
      if(_requirementsTracker->getSwapper().isFull())
      {
        break;
      }
    }

    sc = generator->next();
  }

  LOG4CXX_INFO(logger, "Generated total " << _directFosSolutions.size() << " FOS solutions");

  LOG4CXX_INFO(logger,
               "SOP usage after FOS generation: "
                   << _requirementsTracker->getSopUsageTracker().getNbrOfUnusedSops() << "/"
                   << _requirementsTracker->getSopUsageTracker().getNbrOfSops() << " unused SOPs");
  LOG4CXX_DEBUG(logger,
                "Sop usage counter after FOS generation:\n"
                    << sopUsageToString());
}

void
V2IbfManager::initDirectFosGenerator(IbfDiag910Collector* coll)
{
  _log->info("Initializing generator for direct FOS solutions");
  _directFosGenerator.reset(createFosPipelineGenerator("Direct fos generator"));
  _directFosGenerator->addPredicate(
      utils::wrapPredicateWithName(_trx.dataHandle().create<SopIsDirect>(), "Sop is direct", _trx));
  _directFosGenerator->addPredicate(_factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  _directFosGenerator->addPredicate(_factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  TSE_ASSERT(_directFosBadElemsLogger.get() != nullptr);
  _directFosGenerator->addObserver(_directFosBadElemsLogger.get());

  IbfUtils::loadGeneratorWithSops(*_directFosGenerator, _validSops, this);

  if (coll && (coll->areDetailsFor910Enabled()))
  {
    // Tell diagnostic 910, how many direct SOPs are on each leg
    // (in the generator)
    for (unsigned int i = 0; i < _directFosGenerator->getNumberOfLegs(); ++i)
    {
      coll->setNumberOfSopsForLeg(i, _directFosGenerator->getSopsNumberOnLeg(i));
    }
  }

  TSE_ASSERT(_requirementsTracker != nullptr);
  _requirementsTracker->calculateDirectFosTargetCount();
}


void
V2IbfManager::estimateRCOnlinesDesiredCount()
{
  _log->info("Estimating initial RC onlines count");
  const unsigned int legsCount = _rcOnlinesGenerator->getNumberOfLegs();
  _requirementsEstimator.setLegsCount(legsCount);
  _requirementsEstimator.setRequiredSolutionsCount(
      _requirementsTracker->getRequestedNbrOfSolutions());
  _requirementsEstimator.setDirectFosCount(_directFosSolutions.size());

  // Inside tracker are only direct FOS solutions
  // so all directs are also direct FOS
  _requirementsEstimator.setRcoDirectFosCount(_requirementsTracker->getRcOnlinesCount());

  for (unsigned int i = 0; i < legsCount; ++i)
  {
    _requirementsEstimator.setUnusedSopsCount(i, _requirementsTracker->getSopUsageTracker().getNbrOfUnusedSopsOnLeg(i));
    _requirementsEstimator.setRcoSopsCount(i, _rcOnlinesGenerator->getSopsNumberOnLeg(i));
  }

  _requirementsEstimator.estimateRemainingRcoCount();

  // Since estimator estimates only remaining RC onlines,
  // the total desired number is a sum with the current number
  // of RC onlines from direct FOSes

  _log->info(Fmt("\n---- ESTIMATION RESULTS ----\n%s") % _requirementsEstimator.toString());

  unsigned int totalDesiredRcOnlines =
      _requirementsTracker->getRcOnlinesCount() + _requirementsEstimator.getEstimatedRemainingRco();
  _log->info(Fmt("Estimated total number of RC onlines: %u") % totalDesiredRcOnlines);

  if (totalDesiredRcOnlines > _requirementsTracker->getRequestedNbrOfSolutions())
  {
    totalDesiredRcOnlines = _requirementsTracker->getRequestedNbrOfSolutions();
    _log->error(Fmt("Total number of RC onlines greated than requested"
                    "number of solutions. Reducing to %u") %
                _requirementsTracker->getRequestedNbrOfSolutions());
  }

  _requirementsTracker->setRCOnlinesDesiredCount(totalDesiredRcOnlines);
}

void
V2IbfManager::setFlightMatrix(ShoppingTrx::FlightMatrix* matrix)
{
  TSE_ASSERT(matrix != nullptr);
  _matrix = matrix;
  _log->info("Starting to process the interline shopping queue");
}

void
V2IbfManager::newQueueSolution(const utils::SopCombination& comb)
{
  insertNewOption("Queue", comb);
}



void
V2IbfManager::generateFinalFosSolutions()
{
  _log->info("Starting to generate final FOS solutions");
  TSE_ASSERT(_matrix != nullptr);
  if (!_requirementsTracker->isAllSopsRepresentedSatisfied())
  {
    generateAsrFos(*_matrix);
  }
  else
  {
    _log->info("'All sops represented' satisfied. No need for FOS generation for this requirement");
  }

  if (_trx.getRequest()->isAllFlightsRepresented())
  {
    if (!_requirementsTracker->isAllDirectOptionsPresentSatisfied())
    {
      generateDirectFos(*_matrix);
    }
    else
    {
      _log->info("'All direct options present' satisfied. No need for FOS generation for this requirement");
    }
  }

  if (!_requirementsTracker->isAllOnlinesForCarrierSatisfied())
  {
    generateRcoFos(*_matrix);
  }
  else
  {
    _log->info("'RC onlines preferred' satisfied. No need for FOS generation for this requirement");
  }

  if (_trx.getRequest()->isReturnIllogicalFlights())
  {
    if (!_requirementsTracker->hasRequestedNbrOfSolutions())
    {
      _log->info("Non-MCT options requested. We haven't reached the desired number of solutions. "
                 "Generating non-MCT FOS.");
      initSopUsageProgressGenerator(true);
      generateAsrFos(*_matrix);
    }
    else
    {
      _log->info("Non-MCT options requested but we have collected the desired number of solutions. "
                 "No further FOS necessary.");
    }
  }

  if (!isResultSetDone())
  {
    generateQ0sFos(*_matrix);
  }
  else
  {
    _log->info("We have collected the desired number of solutions and SRL not violated. No further "
               "FOS necessary");
  }
}


void
V2IbfManager::generateDirectFos(ShoppingTrx::FlightMatrix& matrix)
{
  _log->info("Starting to generate FOS for requirement: all direct sop combinations");
  TSE_ASSERT(_directFosGenerator.get() != nullptr);

  LOG4CXX_INFO(logger, "Starting generation FOS for IBF in V2");

  // When generating FOS solutions, in addition collect them:
  // a) IbfRequirementsTracker tracks which SOPs are already used
  // b) diag 910 Collector makes list of generated FOSes
  utils::SopCombination sc = _directFosGenerator->next();
  while (!sc.empty())
  {
    if (matrix.find(sc) == matrix.end())
    {
      const utils::SopCombination toRemove = insertNewOption("All Direct FOS", sc);
      if (toRemove == sc)
      {
        _log->debug("All Direct FOS: no more space -- aborting");
        break;
      }
      matrix.insert(std::make_pair(sc, static_cast<GroupFarePath*>(nullptr)));
      if (toRemove.size() > 0)
      {
        matrix.erase(toRemove);
      }
    }

    sc = _directFosGenerator->next();
  }

  LOG4CXX_INFO(logger,
               "SOP usage after FOS generation: "
                   << _requirementsTracker->getSopUsageTracker().getNbrOfUnusedSops() << "/"
                   << _requirementsTracker->getSopUsageTracker().getNbrOfSops() << " unused SOPs");
  LOG4CXX_DEBUG(logger,
                "Sop usage counter after FOS generation:\n"
                    << sopUsageToString());

  // Release the generator to unsubscribe from the
  // Swapper object in case of SopPenaltyGenerator.
  if (isComplexTripLogic())
  {
    _directFosGenerator.reset();
  }
}

void
V2IbfManager::generateAsrFos(ShoppingTrx::FlightMatrix& matrix)
{
  _log->info("Starting to generate FOS for requirement: all sops represented");

  fetchSopUsageProgressGenerator();

  TSE_ASSERT(_asrFosBadElemsLogger.get() != nullptr);
  _sopProgressGen->addObserver(_asrFosBadElemsLogger.get());

  SopCombination comb = _sopProgressGen->next();

  // When the generator is exhausted,
  // we finish ASR FOS generation
  while (!comb.empty())
  {
    // A new combination should not appear
    // in the matrix since it contains an
    // unused SOP
    TSE_ASSERT(matrix.find(comb) == matrix.end());

    const utils::SopCombination toRemove = insertNewOption("ASR FOS", comb);

    if (toRemove == comb)
    {
      // No more place for options increasing
      // representation of SOPs
      _log->debug("No more space -- aborting");
      break;
    }

    // This updates which SOPs have been used
    // and resets the SOP covering generator
    for (size_t legId = 0; legId < comb.size(); ++legId)
    {
      const unsigned int sopId = comb[legId];
      _sopProgressGen->sopUsed(legId, sopId, _requirementsTracker->getSopUsageTracker().getUsageCount(legId, sopId));
    }
    // FOS <=> zero GroupFarePath pointer
    matrix.insert(std::make_pair(comb, (GroupFarePath*)nullptr));

    if (toRemove.size() > 0)
    {
      matrix.erase(toRemove);
    }

    _log->debug(Fmt("We have %u elements in swapper") % _requirementsTracker->getSolutionsCount());
    _log->debug(Fmt("We have %u elements in matrix") % matrix.size());
    TSE_ASSERT(_requirementsTracker->getSolutionsCount() == matrix.size());

    if (_requirementsTracker->isAllSopsRepresentedSatisfied())
    {
      // We are done with this requirement
      _log->info("All sops represented satisfied");
      break;
    }
    comb = _sopProgressGen->next();
  }
}

void
V2IbfManager::generateRcoFos(ShoppingTrx::FlightMatrix& matrix)
{
  _log->info("Starting to generate FOS for requirement: RC online flights preferred");

  TSE_ASSERT(_rcoFosBadElemsLogger.get() != nullptr);
  _rcOnlinesGenerator->addObserver(_rcoFosBadElemsLogger.get());

  _log->info(Fmt("RC online generator after initial fetch:\n%s") % *_rcOnlinesGenerator);


  // RC onlines generator is already loaded
  SopCombination comb = _rcOnlinesGenerator->next();

  // When the generator is exhausted,
  // we finish
  while (!comb.empty())
  {
    // If the combination
    // is already in the matrix
    // we skip it
    if (matrix.find(comb) == matrix.end())
    {
      const utils::SopCombination toRemove = insertNewOption("RCO FOS", comb);

      if (toRemove == comb)
      {
        // No more place for RC online SOPs
        _log->debug("No more space -- aborting");
        break;
      }

      // FOS <=> zero GroupFarePath pointer
      matrix.insert(std::make_pair(comb, (GroupFarePath*)nullptr));

      if (toRemove.size() > 0)
      {
        matrix.erase(toRemove);
      }

      _log->debug(Fmt("We have %u elements in swapper") %
                  _requirementsTracker->getSolutionsCount());
      _log->debug(Fmt("We have %u elements in matrix") % matrix.size());
      TSE_ASSERT(_requirementsTracker->getSolutionsCount() == matrix.size());

      if (_requirementsTracker->isAllOnlinesForCarrierSatisfied())
      {
        // We are done with this requirement
        _log->info("RC online flights preferred satisfied");
        break;
      }
    }
    comb = _rcOnlinesGenerator->next();
  }

  // Release the generator to unsubscribe from the
  // Swapper object in case of SopPenaltyGenerator.
  if (isComplexTripLogic())
  {
    _rcOnlinesGenerator.reset();
  }
}

void
V2IbfManager::generateQ0sFos(ShoppingTrx::FlightMatrix& matrix)
{
  _log->info("Starting to generate FOS to collect the requested number of solutions + satisfy SRL");

  std::shared_ptr<FosPipelineGenerator> generator(
      createFosPipelineGenerator("Q0S fos generator", false));
  generator->addPredicate(_factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  generator->addPredicate(_factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));

  TSE_ASSERT(_q0sFosBadElemsLogger.get() != nullptr);
  generator->addObserver(_q0sFosBadElemsLogger.get());

  // Fetch with all valid sops, no input filtering
  IbfUtils::loadGeneratorWithSops(*generator, _validSops);

  _log->info(Fmt("Q0S generator after initial fetch:\n%s") % *generator);


  SopCombination comb = generator->next();
  while (!comb.empty())
  {
    _log->debug(Fmt("New combination: %s") % toStr(comb));
    if (matrix.find(comb) == matrix.end())
    {
      const utils::SopCombination toRemove = insertNewOption("Q0s FOS", comb);

      // Even if this option is not added, we do not break
      // Maybe later on something will appear which will push out
      // an option bad for SRL
      if (toRemove != comb)
      {
        // FOS <=> zero GroupFarePath pointer
        matrix.insert(std::make_pair(comb, (GroupFarePath*)nullptr));

        if (toRemove.size() > 0)
        {
          matrix.erase(toRemove);
        }

        _log->debug(Fmt("We have %u elements in swapper") %
                    _requirementsTracker->getSolutionsCount());
        _log->debug(Fmt("We have %u elements in matrix") % matrix.size());
        TSE_ASSERT(_requirementsTracker->getSolutionsCount() == matrix.size());

        if (isResultSetDone())
        {
          // We are done with this requirement
          _log->info("Q0S + SRL reached");
          break;
        }
      }
      else
      {
        // toRemove == comb: no-progress Swapper iteration
        if (isComplexTripLogic())
        {
          ++_accumulatedQ0sNoprogressIterations;
        }
      }

      if (isComplexTripLogic())
      {
        if (_requirementsTracker->getSwapper().getNoProgressIterationsCount()
              >= NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD)
        {
          // We cannot generate a useful option for quite a long time
          _log->info(Fmt("Aborted Q0S FOS generation after %u iterations without progress") %
              NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD);
          break;
        }

        if (_accumulatedQ0sNoprogressIterations
            >= ACCUMULATED_NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD)
        {
          _log->info(Fmt("Aborted Q0S FOS generation after accumulated %u iterations without progress") %
              ACCUMULATED_NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD);
          break;
        }
      }

    }
    else
    {
      _log->info(Fmt("FOS combination %s already in matrix, generating next") % toStr(comb));
    }
    comb = generator->next();
  }

}

utils::SopCombination
V2IbfManager::insertNewOption(const std::string& prefix, const utils::SopCombination& added)
{
  TSE_ASSERT(_requirementsTracker != nullptr);
  const IbfRequirementsTracker::ImplSwapper& swp = _requirementsTracker->getSwapper();
  const IbfRequirementsTracker::AddResponse addResponse =
      _requirementsTracker->newQueueSolution(added);

  if (addResponse.first.empty())
  {
    // Option has been added
    _log->info(Fmt("(%s) Added option %s = [%s]") % prefix % toStr(added) %
               _pFormatter->formatScores(swp.getAppraiserScoresForItem(added)));
    return addResponse.first;
  }

  if (addResponse.first != added)
  {
    // Swapped
    _log->info(Fmt("(%s) Added option %s = [%s] and removed bottom %s = [%s]") % prefix %
               toStr(added) % _pFormatter->formatScores(swp.getAppraiserScoresForItem(added)) %
               toStr(addResponse.first) % _pFormatter->formatScores(addResponse.second));
  }
  else
  {
    // New option not added since removed immediately
    if (swp.getSize() == 0)
    {
      _log->error(Fmt("(%s) Unexpected empty Swapper") % prefix);
    }
    else
    {
      _log->info(Fmt("(%s) Not added option %s = [%s] since worse than bottom %s = [%s]") % prefix
                 // addResponse.first == added
                 %
                 toStr(addResponse.first) % _pFormatter->formatScores(addResponse.second) %
                 toStr(swp.begin()->key) %
                 _pFormatter->formatScores(swp.getAppraiserScoresForItem(swp.begin()->key)));
    }
  }

  return addResponse.first;
}

void
V2IbfManager::fetchSopUsageProgressGenerator()
{
  _log->info(Fmt("Fetching SOP usage progress generator"));

  for (SopUsageTracker::UnusedSopsIterator it = _requirementsTracker->getSopUsageTracker().unusedSopsBegin();
       it != _requirementsTracker->getSopUsageTracker().unusedSopsEnd();
       ++it)
  {
    _sopProgressGen->addSop(*it, 0);
  }

  for (SopUsageTracker::SopsUsageIterator it = _requirementsTracker->getSopUsageTracker().sopUsageBegin();
       it != _requirementsTracker->getSopUsageTracker().sopUsageEnd();
       ++it)
  {
    _sopProgressGen->addSop(it->first, it->second);
  }

  _sopProgressGen->loadGenerator();
}


void V2IbfManager::removeNonSwapperOptions()
{
  TSE_ASSERT(_matrix != nullptr);
  TSE_ASSERT(_requirementsTracker != nullptr);

  _log->info("Removing non-swapper options from the flight matrix");
  _log->info(Fmt("Flight matrix size before: %u") % _matrix->size());

  const IbfRequirementsTracker::ImplSwapper::ItemSet swapperItems =
      _requirementsTracker->getSwapper().getItems();

  ShoppingTrx::FlightMatrix newMatrix;
  for (ShoppingTrx::FlightMatrix::const_iterator it = _matrix->begin();
       it != _matrix->end(); ++it)
  {
    if (swapperItems.find(it->first) != swapperItems.end())
    {
      newMatrix.insert(*it);
      if (_log->enabled(utils::LOGGER_LEVEL::INFO))
      {
        _log->info(Fmt("%s") % toStr(it->first));
      }
    }
    else
    {
      if (_log->enabled(utils::LOGGER_LEVEL::INFO))
      {
        _log->info(Fmt("%s REMOVED") % toStr(it->first));
      }
    }
  }
  *_matrix = newMatrix;
  _log->info(Fmt("Flight matrix size after: %u") % _matrix->size());
}

bool V2IbfManager::areAllRequirementsMet() const
{
  if (_requirementsTracker->areAllRequirementsMet())
  {
    return true;
  }

  if (_requirementsTracker->getSwapper().getNoProgressIterationsCount()
        >= _queueIterationsLimit)
  {
    _log->info(Fmt("Aborted Queue generation after %u iterations without progress") %
        _queueIterationsLimit);
    return true;
  }

  return false;
}


void
generateInitialFosInNgs(ShoppingTrx& trx,
                        unsigned int requestedNbrOfSolutions,
                        utils::SopCombinationList& directFoses)
{
  IbfDiag910Collector coll(trx, "Direct Combinations");
  std::vector<utils::SopCandidate> validSops;
  ContainerSink<SopCandidate, IbfUtils::SopCandidateOutputIterator> sink(
      std::back_inserter(validSops));

  // Only if details are enabled, we want to
  // gather info on SOPs discarded due to cabin validation
  if (coll.areDetailsFor910Enabled())
  {
    IbfUtils::collectCabinClassValidSops(trx, sink, &coll);
  }
  else
  {
    IbfUtils::collectCabinClassValidSops(trx, sink);
  }

  FosPipelineGenerator generator(getNonAsoLegsCount(trx), requestedNbrOfSolutions);

  generator.addPredicate(
      utils::wrapPredicateWithName(trx.dataHandle().create<SopIsDirect>(), "Sop is direct", trx));

  FosPredicatesFactory factory(trx);
  generator.addPredicate(factory.createFosPredicate(tse::utils::MINIMUM_CONNECT_TIME));
  generator.addPredicate(factory.createFosPredicate(tse::utils::INTERLINE_TICKETING_AGREEMENT));
  IbfUtils::loadGeneratorWithSops(generator, validSops);

  // Tell diagnostic 910, how many direct SOPs are on each leg
  // (in the generator)
  if (coll.areDetailsFor910Enabled())
  {
    for (unsigned int i = 0; i < generator.getNumberOfLegs(); ++i)
    {
      coll.setNumberOfSopsForLeg(i, generator.getSopsNumberOnLeg(i));
    }
  }

  // Only if details are enabled, we want to
  // gather info on SOP combinations discarded due to
  // interline ticketing agreement or minimum connect time
  if (coll.areDetailsFor910Enabled())
  {
    // We must tell compiler explicitly which interface
    // we will use: ICollector for SOPs or SOP combinations
    generator.addObserver(static_cast<IFilterObserver<SopCombination>*>(&coll));
  }

  LOG4CXX_INFO(logger, "Starting generation FOS for IBF in NGS");

  // When generating FOS solutions, in addition collect them:
  // diag 910 Collector makes list of generated FOSes
  utils::SopCombination sc = generator.next();
  while (!sc.empty())
  {
    directFoses.push_back(sc);
    coll.collect(sc);
    sc = generator.next();
  }

  LOG4CXX_INFO(logger, "Generated total " << directFoses.size() << " FOS solutions");

  coll.flush();
}


} // namespace tse
