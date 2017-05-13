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

#include "Pricing/Shopping/Utils/RankedCombinations.h"
#include "Pricing/Shopping/Utils/SopCombinationsGenerator.h"

#include <memory>

namespace tse
{

namespace utils
{

template<typename RankedCombinationsT>
class IRankedCombinationsFactory
{
public:
  virtual RankedCombinationsT* create(size_t width) = 0;
  virtual ~IRankedCombinationsFactory(){}
};

template<typename RankedCombinationsT>
class BasicRankedCombinationsFactory:
    public IRankedCombinationsFactory<RankedCombinationsT>
{
  RankedCombinationsT* create(size_t width) override { return new RankedCombinationsT(width); }
};

template<typename RankedCombinationsT>
class SopRankedGenerator: public BaseSopCombinationsGenerator
{
public:
  typedef IRankedCombinationsFactory<RankedCombinationsT> FactoryT;
  SopRankedGenerator(
      FactoryT* factory = new BasicRankedCombinationsFactory<RankedCombinationsT>(),
      ISopBank* sopBank = new SopBank()):
        BaseSopCombinationsGenerator(sopBank)
  {
    _factory.reset(factory);
  }

protected:
  void initialize() override
  {
    const size_t dimensions_count = _userInputSops->getNumberOfLegs();
    _rankedCombinations.reset(_factory->create(dimensions_count));

    for (size_t i = 0; i < dimensions_count; ++i)
    {
      const SopCombination& current_leg = _userInputSops->getSopsOnLeg(i);
      _rankedCombinations->setDimensionFinite(i, current_leg.size());
    }
  }

  SopCombination nextElement() override
  {
    RankedCombinationsIndices next_indices = _rankedCombinations->next();
    const size_t dimensions_count = next_indices.size();
    SopCombination answer;

    if (dimensions_count == 0)
    {
      // If the indices generator is exhausted,
      // this generator also is.
      return answer;
    }

    for (size_t i = 0; i < dimensions_count; ++i)
    {
      const SopCombination& current_leg = _userInputSops->getSopsOnLeg(i);
      const size_t inLegIndex = next_indices[i];
      TSE_ASSERT(inLegIndex < current_leg.size());
      answer.push_back(current_leg[inLegIndex]);
    }
    return answer;
  }

private:
  typedef std::shared_ptr<RankedCombinationsT> RankedCombinationsPtr;
  RankedCombinationsPtr _rankedCombinations;
  std::shared_ptr<FactoryT> _factory;
};

typedef SopRankedGenerator<RankedCombinations> BasicSopRankedGenerator;

class BasicSopRankedGeneratorFactory: public ISopCombinationsGeneratorFactory
{
public:
  virtual ISopCombinationsGenerator* create() override
  {
    return new BasicSopRankedGenerator();
  }
};


} // namespace utils

} // namespace tse



