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

#include "Pricing/Shopping/Utils/FosPipelineGenerator.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <sstream>

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.IBF.FosPipelineGenerator");
}

FosPipelineGenerator::FosPipelineGenerator(
    unsigned int legsCount, unsigned int combinationsLimit,
    size_t maxFilterRetries, utils::ILogger* logger,
    ISopCombinationsGenerator* childGenerator):
        _combinationsLimit(combinationsLimit),
        _inputSopsFilter(_filteredInputSopsFork)
  {
    TSE_ASSERT(childGenerator != nullptr);
    _generator.reset(childGenerator);
    _sopCombinationsfilter.reset(new GeneratingFilter<SopCombination>(
        *childGenerator, maxFilterRetries, logger));
    _generatorAdapter.reset(new GeneratorAdapter(*childGenerator));
    initializeFiltersAndPipes(legsCount);
  }

void
FosPipelineGenerator::GeneratorAdapter::collect(const SopCandidate& sopInfo)
{
  LOG4CXX_DEBUG(logger, "Feeding FOS generator with SOP " << sopInfo);
  _generator.addSop(sopInfo.legId, sopInfo.sopId);
}

SopCombination
FosPipelineGenerator::next() 
{
  // We do not ask underlying generator for more options
  // since we hit the limit and want to return an empty option
  // as the end marker
  if ((_combinationsLimit != 0) && (_generatedCount >= _combinationsLimit))
  {
    return SopCombination();
  }

  const SopCombination sc = _sopCombinationsfilter->next();
  if (sc.size() == 0)
  {
    // All combinations from underlying generator exhausted
    // -- forward an empty option
    return sc;
  }

  ++_generatedCount;
  return sc;
}

std::ostream& operator<<(std::ostream& out, const FosPipelineGenerator& g)
{
  out << g.getChildGenerator();
  return out;
}

} // namespace utils

} // namespace tse
