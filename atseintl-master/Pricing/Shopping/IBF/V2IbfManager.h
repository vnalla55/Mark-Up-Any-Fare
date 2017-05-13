
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

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/IBF/IbfRequirementsEstimator.h"
#include "Pricing/Shopping/IBF/IbfRequirementsTracker.h"
#include "Pricing/Shopping/IBF/SopUsageProgressGenerator.h"
#include "Pricing/Shopping/Predicates/FosPredicatesFactory.h"
#include "Pricing/Shopping/Swapper/PrioritySwapperFormatter.h"
#include "Pricing/Shopping/Utils/BadElementLogger.h"
#include "Pricing/Shopping/Utils/FosPipelineGenerator.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <memory>
#include <sstream>
#include <string>

namespace tse
{
class IbfDiag910Collector;

// A facade class responsible for Interline Branded Fares
// aspect of processing in Pricing Orchestrator in V2
// It holds IBF data and coordinates IBF processing
class V2IbfManager: public utils::ICollector<utils::SopCandidate>
{
public:
  V2IbfManager(ShoppingTrx& trx,
               unsigned int requestedNbrOfSolutions,
               const std::string& requestingCarrierCode,
               size_t queueIterationsLimit = DEFAULT_QUEUE_ITERATIONS_LIMIT,
               utils::ILogger* logger = nullptr);

  void performInitialPOTasks();

  // trx -sops-> filter[cabin class] -sops-> v<valid SOPs>
  //                 |                            |
  //                 |-> 910: discarded SOPs      |-> req. tracker
  //                                                  (later tracking SOP coverage)
  void collectValidSopsFromTrx(IbfDiag910Collector* coll = nullptr);

  // v<valid SOPs> -sops-> RC Online FOS generator
  void fetchRcOnlinesGenerator();

  // The first step of processing IBF in Pricing Orchestrator:
  // initial generation of all direct flights combinations
  // as flight only solutions
  // Diag910 is updated
  // IbfRequirementsTracker is updated
  //
  // FETCH:
  // v<valid SOPs> -sops-> FOS generator
  //                        |-> 910: how many SOPs
  //                            on each leg
  //
  // GENERATE:
  // FOS generator -sop combinations --------> directFoses
  //   |                                 |-> tracker: used SOP combinations
  //   |                                 |-> 910: list of generated FOSes
  //   |- discarded sop comb. -> 910
  void generateInitialDirectFos(IbfDiag910Collector* coll = nullptr);


  void initDirectFosGenerator(IbfDiag910Collector* coll = nullptr);

  // Estimates the overall number of RC onlines
  // that we should generate in the PO queue.
  void estimateRCOnlinesDesiredCount();

  void setFlightMatrix(ShoppingTrx::FlightMatrix* matrix);

  void generateFinalFosSolutions();

  void newQueueSolution(const utils::SopCombination& comb);

  unsigned int getRequestedNbrOfSolutions() const
  {
    return _requirementsTracker->getRequestedNbrOfSolutions();
  }

  // Returns the number of SOPs tracked
  unsigned int getNbrOfSops() const { return _requirementsTracker->getSopUsageTracker().getNbrOfSops(); }

  // Returns the number of SOPs with usage zero
  unsigned int getNbrOfUnusedSops() const { return _requirementsTracker->getSopUsageTracker().getNbrOfUnusedSops(); }

  bool hasRequestedNbrOfSolutions() const
  {
    return _requirementsTracker->hasRequestedNbrOfSolutions();
  }

  bool areAllRequirementsMet() const;

  const utils::SopCombinationList& getDirectFosSolutions() const { return _directFosSolutions; }

  std::string sopUsageToString() const
  {
    std::ostringstream out;
    _requirementsTracker->getSopUsageTracker().toStream(out);
    return out.str();
  }

  std::string swapperToString() const { return _requirementsTracker->swapperToString(); }

  // Removes all options from the Flight Matrix
  // that are not present in the swapper
  void removeNonSwapperOptions();

  static const size_t DEFAULT_QUEUE_ITERATIONS_LIMIT;

private:
  typedef utils::BadElementLogger<utils::SopCombination> BadSopCombinationLogger;

  static const size_t SIMPLE_TRIP_MAX_LEGS = 2;
  static const size_t NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD;
  static const size_t ACCUMULATED_NO_PROGRESS_ITERATIONS_ABORT_THRESHOLD;
  static const size_t NO_PROGRESS_FOS_ITERATIONS_ABORT_THRESHOLD;

  void generateAsrFos(ShoppingTrx::FlightMatrix& matrix);
  void generateDirectFos(ShoppingTrx::FlightMatrix& matrix);
  void generateRcoFos(ShoppingTrx::FlightMatrix& matrix);
  void generateQ0sFos(ShoppingTrx::FlightMatrix& matrix);

  utils::SopCombination
  insertNewOption(const std::string& prefix, const utils::SopCombination& added);

  void fetchSopUsageProgressGenerator();

  bool isResultSetDone() const
  {
    return _requirementsTracker->hasRequestedNbrOfSolutions() &&
           _requirementsTracker->isSrlSatisfied();
  }

  // This collects direct SOPs on the stage of initializing
  // the pipeline generator for direct FOSes. The SOPs are
  // sent to the requirement tracker which needs to know
  // the direct SOPs for the current transaction.
  void collect(const utils::SopCandidate& candidate) override
  {
    TSE_ASSERT(_requirementsTracker != nullptr);
    _requirementsTracker->addDirectSopForTracking(candidate.legId, candidate.sopId);
  }

  void initFosLoggers();

  bool isComplexTripLogic() const;

  SopUsageProgressGenerator* createSopUsageProgressGenerator();

  void initSopUsageProgressGenerator(bool allowNonMctOptions);

  utils::FosPipelineGenerator* createFosPipelineGenerator(
      const std::string& loggerName, bool bounded = true);

  ShoppingTrx& _trx;
  IbfRequirementsTracker* _requirementsTracker;
  std::vector<utils::SopCandidate> _validSops;
  std::unique_ptr<utils::FosPipelineGenerator> _directFosGenerator;
  utils::SopCombinationList _directFosSolutions;
  std::unique_ptr<utils::FosPipelineGenerator> _rcOnlinesGenerator;
  IbfRequirementsEstimator _requirementsEstimator;
  std::unique_ptr<SopUsageProgressGenerator> _sopProgressGen;
  ShoppingTrx::FlightMatrix* _matrix;
  utils::FosPredicatesFactory _factory;
  swp::PriorityScoreFormatter<utils::SopCombination>* _pFormatter;
  utils::LoggerHandle _log;
  std::unique_ptr<BadSopCombinationLogger> _asrFosBadElemsLogger;
  std::unique_ptr<BadSopCombinationLogger> _directFosBadElemsLogger;
  std::unique_ptr<BadSopCombinationLogger> _rcoFosBadElemsLogger;
  std::unique_ptr<BadSopCombinationLogger> _q0sFosBadElemsLogger;

  // Here we store the total number (not necessarily
  // in a row) of no-progress Swapper iterations for the last
  // Q0S-SRL phase of FOS generation.
  size_t _accumulatedQ0sNoprogressIterations;
  size_t _queueIterationsLimit;
};

// The first step of processing IBF in NGS:
// initial generation of all direct flights combinations
// as flight only solutions
// Diag910 is updated
void
generateInitialFosInNgs(ShoppingTrx& trx,
                        unsigned int requestedNbrOfSolutions,
                        utils::SopCombinationList& directFoses);

} // namespace tse

