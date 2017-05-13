//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include "Pricing/Shopping/Utils/RankedCombinations.h"

#include "Common/Assert.h"

namespace tse
{

namespace utils
{

IndexGenerator::IndexGenerator(IndexGenerator* next): _nextGenerator(next)
{
  if (_nextGenerator != nullptr)
  {
    _capacity.reset(new GeneratorCapacity(_nextGenerator->getCapacity()));
  }
  else
  {
    _capacity.reset(new GeneratorCapacity());
  }
  reset(0);
}

void IndexGenerator::reset(size_t rank)
{
  _rank = rank;
  _currentIndex = 0;
  _exhausted = false;
  if (_nextGenerator != nullptr)
  {
    _nextGenerator->reset(rank);
  }
}

bool IndexGenerator::next()
{
  if (_exhausted)
  {
    return false;
  }

  if (_rank > _capacity->getRecursiveCapacity())
  {
    _exhausted = true;
    return false;
  }

  if (_nextGenerator == nullptr)
  {
    _exhausted = true;
    _currentIndex = _rank;
    return true;
  }

  while (true)
  {
    const bool nextHasNewCombination = _nextGenerator->next();
    if (nextHasNewCombination)
    {
      return true;
    }
    ++_currentIndex;
    if (_currentIndex > std::min(_rank, (_capacity->getLocalCapacity())))
    {
      _exhausted = true;
      return false;
    }
    _nextGenerator->reset(_rank - _currentIndex);
  }
}

RankedCombinations::RankedCombinations(size_t width)
{
  TSE_ASSERT(width != 0);
  _indexGenerators.resize(width);

  const int maxi = static_cast<int>(width-1);
  for (int i = maxi; i >= 0; --i)
  {
    if (i == maxi)
    {
      _indexGenerators[i].reset(new IndexGenerator());
    }
    else
    {
      _indexGenerators[i].reset(new IndexGenerator(_indexGenerators[i+1].get()));
    }
  }
}


void RankedCombinations::setDimensionFinite(size_t pos, size_t dimensionLength)
{
  TSE_ASSERT(pos < _indexGenerators.size());
  if (dimensionLength == 0)
  {
    _exhausted = true;
    return;
  }
  _indexGenerators[pos]->setDimensionLength(dimensionLength);
}


RankedCombinationsIndices RankedCombinations::next()
{
  if (_exhausted)
  {
    return RankedCombinationsIndices();
  }

  while (true)
  {
    const bool hasNewCombination = _indexGenerators[0]->next();
    if (hasNewCombination)
    {
      _anyCombinationForCurrentRank = true;
      return buildIndices();
    }

    if (_anyCombinationForCurrentRank)
    {
      _anyCombinationForCurrentRank = false;
      ++_rank;
      _indexGenerators[0]->reset(_rank);
    }
    else
    {
      _exhausted = true;
      return RankedCombinationsIndices();
    }
  }
}


RankedCombinationsIndices RankedCombinations::buildIndices()
{
  RankedCombinationsIndices indices;
  for (auto& elem : _indexGenerators)
  {
    indices.push_back(elem->getIndex());
  }
  return indices;
}
} // namespace utils
} // namespace tse
