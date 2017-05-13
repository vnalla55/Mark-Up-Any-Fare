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

#include "Pricing/Shopping/FOS/FosBaseGenerator.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <map>

namespace tse
{
namespace
{
ConfigurableValue<uint32_t>
maxNumFilterCombinations("SHOPPING_OPT", "FOS_MAX_NUM_FILTER_COMBINATIONS", 5000);
}
namespace fos
{
static Logger
logger("atseintl.pricing.FOS.FosBaseGenerator");

FosBaseGenerator::FosBaseGenerator(ShoppingTrx& trx,
                                   FosFilterComposite& fosFilterComposite,
                                   Diag910Collector* dc910)
  : _trx(trx),
    _fosFilterComposite(fosFilterComposite),
    _dc910(dc910),
    _currentGenerator(0),
    _maxNumFilterCombinations(maxNumFilterCombinations.getValue())
{
  _genStats.currentFilterCutoff = _maxNumFilterCombinations;
}

utils::FosGenerator*
FosBaseGenerator::createFosGenerator() const
{
  utils::FosGenerator* gen = &_trx.dataHandle().safe_create<utils::FosGenerator>(_trx);
  gen->setNumberOfLegs(utils::getNonAsoLegsCount(_trx));
  return gen;
}

bool
FosBaseGenerator::updateFilters(ValidatorBitMask lackingValidators)
{
  while (!_fosFilterComposite.empty() &&
         !(_fosFilterComposite.getAffectedValidators() & lackingValidators))
  {
    popFilter();
  }
  return !_fosFilterComposite.empty();
}

void
FosBaseGenerator::popFilter()
{
  _fosFilterComposite.pop();
  while (!_fosFilterComposite.empty() && _fosFilterComposite.isFilterToPop())
    _fosFilterComposite.pop();

  if (!_fosFilterComposite.empty())
  {
    _toCheckCombinations.splice(_toCheckCombinations.begin(), _checkedCombinations);
  }
  _genStats.currentFilterCutoff = _genStats.totalProcessedCombinations + _maxNumFilterCombinations;
}

bool
FosBaseGenerator::getNextCombination(ValidatorBitMask lackingValidators,
                                     SopCombination& outCombination)
{
  while (updateFilters(lackingValidators))
  {
    while (_fosGenerators.size() &&
           _genStats.totalProcessedCombinations < _genStats.currentFilterCutoff)
    {
      outCombination = _fosGenerators[_currentGenerator]->next();
      if (outCombination.size())
      {
        ++_genStats.totalProcessedCombinations;
        ++_genStats.uniqueProcessedCombinations;
        if (++_currentGenerator >= _fosGenerators.size())
          _currentGenerator = 0;
        if (_fosFilterComposite.isApplicableSolution(outCombination))
        {
          ++_genStats.validatedCombinations;
          return true;
        }
        else if (!_fosFilterComposite.isLastFilter())
          _checkedCombinations.push_front(outCombination);
      }
      else
      {
        _fosGenerators.erase(_fosGenerators.begin() + _currentGenerator);
        if (_currentGenerator >= _fosGenerators.size())
          _currentGenerator = 0;
      }
    }

    while (!_toCheckCombinations.empty() &&
           _genStats.totalProcessedCombinations < _genStats.currentFilterCutoff)
    {
      ++_genStats.totalProcessedCombinations;
      outCombination = _toCheckCombinations.front();
      _toCheckCombinations.pop_front();
      if (_fosFilterComposite.isApplicableSolution(outCombination))
      {
        ++_genStats.validatedCombinations;
        return true;
      }
      else if (!_fosFilterComposite.isLastFilter())
        _checkedCombinations.push_front(outCombination);
    }

    popFilter();
  }

  return false;
}

SopDetailsPtrVec
FosBaseGenerator::getSopDetails(uint32_t legId, uint32_t sopId)
{
  SopsWithDetailsSet::iterator currentSop = _sopsWithDetailsSet.find(DetailedSop(legId, sopId));

  const DetailedSop* sopNeedDetails;
  if (currentSop != _sopsWithDetailsSet.end())
    sopNeedDetails = &(*currentSop);
  else
  {
    sopNeedDetails = generateSopDetails(legId, sopId);
    _sopsWithDetailsSet.insert(*sopNeedDetails);
  }

  return _fosFilterComposite.getFilteredSopDetails(*sopNeedDetails);
}

DetailedSop*
FosBaseGenerator::generateSopDetails(uint32_t legId, uint32_t sopId)
{
  DetailedSop* newSop = &_trx.dataHandle().safe_create<DetailedSop>(legId, sopId);
  newSop->generateSopDetails(
      _trx.dataHandle(), _trx.legs()[legId].sop()[sopId].itin()->travelSeg(), legId);
  return newSop;
}

void
FosBaseGenerator::addCombinationForReuse(SopCombination sopComb)
{
  if (_fosFilterComposite.isCombinationForReuse(sopComb))
    _checkedCombinations.push_back(sopComb);
}

} // fos
} // tse
