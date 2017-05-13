
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

#include "Pricing/Shopping/FiltersAndPipes/CollectingFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/Fork.h"
#include "Pricing/Shopping/FiltersAndPipes/GeneratingFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/SopCartesianGenerator.h"

#include <boost/utility.hpp>

#include <memory>
#include <iostream>

namespace tse
{

namespace utils
{

// Generates flight options as Flight Only Solutions.
// Sets up a pipeline for FOS construction where several
// predicates and observers can be installed for maximum
// flexibility.
//
// From one perspective, this class is a generator of SOP combinations
// representing flight options.
// On the other hand, it is also a collector, which must
// be supplied with input SOPs before generation is started.
//
// Both source SOPs and produced SOP combinations are filtered
// according to default or user-supplied predicates (see constructors
// documentation below)
//
// Processing inside this class can be depicted as follows:
//
//               |------------ FosPipelineGenerator ----------|
//               |                                            |
// input SOPs -> | [filter SOPs]--> [G] -> [filter SOP comb.] | -> output SOP comb.
//               |      |        |               |            |
//               |      |-> *    |-> **          |-> ***      |
//               |--------------------------------------------|
//
//               [G] : cartesian generator (SOPs per leg -> SOP combinations)
//               *   : place for observation of discarded input SOPs
//               **  : place for observation of input SOPs passed to generator
//               *** : place for observation of discarded SOP combinations

class FosPipelineGenerator : public IGenerator<SopCombination>,
                             public ICollector<SopCandidate>,
                             public IFilter<SopCombination>,
                             public IFilter<SopCandidate>,
                             boost::noncopyable
{
public:
  // Accepts the number of legs for the current transaction
  // and maximum number of combinations to generate
  // If combinationsLimit is zero (default), just all combinations
  // will be generated
  FosPipelineGenerator(unsigned int legsCount, unsigned int combinationsLimit = 0,
      size_t maxFilterRetries = 0, utils::ILogger* logger = nullptr,
      ISopCombinationsGenerator* childGenerator = new SopCartesianGenerator());

  // Collects an input SOP as an element of produced combinations
  // All such SOPs must be supplied before any calls
  // to next() are made
  void collect(const SopCandidate& sopInfo) override
  {
    _inputSopsFilter.collect(sopInfo);
  }

  // Returns the next direct flight combination as a Flight Only Solution
  // If all combinations are exhausted, returns an empty combination
  SopCombination next() override;

  // Returns supplied legsCount
  unsigned int getNumberOfLegs() const { return _generator->getNumberOfLegs(); }

  // Returns the number of SOPs on leg with given legId
  unsigned int getSopsNumberOnLeg(unsigned int legId) const
  {
    return _generator->getSopsOnLeg(legId).size();
  }

  // Adds observer being notified about input SOPs
  // which passed filtering (place **)
  void addObserverForPassedSops(ICollector<SopCandidate>* observer)
  {
    _filteredInputSopsFork.addCollector(observer);
  }

  // Adds observer being notified about discarded input SOPs
  // (place *)
  void addObserver(IFilterObserver<SopCandidate>* observer) override
  {
    _inputSopsFilter.addObserver(observer);
  }

  // Adds observer being notified about discarded SOP combinations
  // (place ***)
  void addObserver(IFilterObserver<SopCombination>* observer) override
  {
    _sopCombinationsfilter->addObserver(observer);
  }

  // Add a predicate for filtering input SOPs
  // A SOP for which any predicate returns false will be discarded
  void addPredicate(INamedPredicate<SopCandidate>* predicate) override
  {
    _inputSopsFilter.addPredicate(predicate);
  }

  // Add a predicate for filtering output SOP combinations
  // A SOP combination for which any predicate returns false will be discarded
  void addPredicate(INamedPredicate<SopCombination>* predicate) override
  {
    _sopCombinationsfilter->addPredicate(predicate);
  }

  const ISopCombinationsGenerator& getChildGenerator() const { return *_generator; }

private:
  using ChildGeneratorPtr = std::shared_ptr<ISopCombinationsGenerator>;

  void initializeFiltersAndPipes(unsigned int legsCount)
  {
    _generatedCount = 0;
    _generator->setNumberOfLegs(legsCount);
    _filteredInputSopsFork.addCollector(_generatorAdapter.get());
  }

  class GeneratorAdapter : public ICollector<SopCandidate>, boost::noncopyable
  {
  public:
    GeneratorAdapter(ISopCombinationsGenerator& generator) : _generator(generator) {}

    void collect(const SopCandidate& sopInfo) override;
  private:
    ISopCombinationsGenerator& _generator;
  };

  unsigned int _generatedCount = 0;
  unsigned int _combinationsLimit;

  // input SOPs -> SOP filter -> SOP fork -> generator adapter -> generator -> output
  //                   |            |                                  |
  //                   |-> *        |-> **                             |-> ***

  Fork<SopCandidate> _filteredInputSopsFork;
  CollectingFilter<SopCandidate> _inputSopsFilter;
  ChildGeneratorPtr _generator;
  std::shared_ptr<GeneratingFilter<SopCombination>> _sopCombinationsfilter;
  std::shared_ptr<GeneratorAdapter> _generatorAdapter;
};

std::ostream& operator<<(std::ostream& out, const FosPipelineGenerator& g);

} // namespace utils

} // namespace tse

