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

#pragma once

#include "Common/Assert.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"

#include <climits>
#include <memory>
#include <vector>

namespace tse
{

namespace utils
{

class GeneratorCapacity
{
public:
  GeneratorCapacity(const GeneratorCapacity* next = nullptr) : _next(next) {}

  size_t getDimensionLength() const
  {
    if (_isDimensionLengthInfinity)
    {
      return INFINITY_VALUE;
    }
    return _dimensionLength;
  }

  void setDimensionLength(size_t length)
  {
    // A generator with a zero-length dimension
    // is degenerated (produces nothing)
    TSE_ASSERT(length != 0);
    _isDimensionLengthInfinity = false;
    _dimensionLength = length;
  }

  size_t getLocalCapacity() const
  {
    return getDimensionLength() - 1;
  }

  size_t getRecursiveCapacity() const
  {
    if (!_next)
    {
      return getLocalCapacity();
    }

    size_t nextCap = _next->getRecursiveCapacity();
    if (nextCap == INFINITY_VALUE)
    {
      return INFINITY_VALUE;
    }
    return nextCap + getLocalCapacity();
  }

  static const size_t INFINITY_VALUE = UINT_MAX;

private:
  const GeneratorCapacity* _next = nullptr;
  bool _isDimensionLengthInfinity = true;
  size_t _dimensionLength = 0;
};


class IndexGenerator
{
public:
  IndexGenerator(IndexGenerator* next = nullptr);

  void reset(size_t rank);

  bool next();

  size_t getIndex() const
  {
    return _currentIndex;
  }

  void setDimensionLength(size_t length)
  {
    _capacity->setDimensionLength(length);
  }

  const GeneratorCapacity* getCapacity() const
  {
    return _capacity.get();
  }

private:
  using GeneratorCapacityPtr = std::shared_ptr<GeneratorCapacity>;
  size_t _rank = 0;
  size_t _currentIndex = 0;
  IndexGenerator* _nextGenerator;
  GeneratorCapacityPtr _capacity;
  bool _exhausted = false;
};

using RankedCombinationsIndices = std::vector<size_t>;

class RankedCombinations: public IGenerator<RankedCombinationsIndices>
{
public:
  RankedCombinations(size_t width);

  void setDimensionFinite(size_t pos, size_t dimensionLength);

  RankedCombinationsIndices next() override;

private:
  RankedCombinationsIndices buildIndices();

  using IndexGeneratorPtr = std::shared_ptr<IndexGenerator>;
  std::vector<IndexGeneratorPtr> _indexGenerators;
  size_t _rank = 0;
  bool _anyCombinationForCurrentRank = false;
  bool _exhausted = false;
};
} // namespace utils
} // namespace tse
