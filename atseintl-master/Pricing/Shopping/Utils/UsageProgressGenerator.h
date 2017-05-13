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

#include "Common/Assert.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/Utils/DecLenCombinationsGenerator.h"
#include "Pricing/Shopping/Utils/LoggerHandle.h"
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Util/CartesianProduct.h"

#include <boost/utility.hpp>

#include <iostream>
#include <memory>
#include <set>
#include <vector>

namespace tse
{

namespace utils
{

enum UPG_DIMENSION_TYPE
{
  UNUSED = 0, // has only unused elements
  USED, // has only used elements
  MIXED, // has both used/unused elements
  EMPTY // has no elements
};

// Holds elements divided into two categories: used and unused
// for a single dimension in UsageProgressGenerator
template <typename T>
class DimensionData
{
public:
  typedef std::set<T> ElementSetType;

  // Raises exception when trying to add
  // the same element to both used and unused
  // categories
  void addElement(const T& elem, bool isUsed)
  {
    if (isUsed)
    {
      TSE_ASSERT(_unused.find(elem) == _unused.end());
      _used.insert(elem);
    }
    else
    {
      TSE_ASSERT(_used.find(elem) == _used.end());
      _unused.insert(elem);
    }
  }

  // Returns the number of removed elements
  // (zero or one)
  unsigned int removeElement(const T& elem)
  {
    if (_unused.erase(elem) == 0)
    {
      return static_cast<unsigned int>(_used.erase(elem));
    }
    return 1;
  }

  const ElementSetType& getUnusedElements() const { return _unused; }

  const ElementSetType& getUsedElements() const { return _used; }

  unsigned int getSize() const { return static_cast<unsigned int>(_unused.size() + _used.size()); }

  UPG_DIMENSION_TYPE getType() const
  {
    if (!getUnusedElements().empty())
    {
      if (!getUsedElements().empty())
      {
        return MIXED;
      }
      return UNUSED;
    }

    if (!getUsedElements().empty())
    {
      return USED;
    }
    return EMPTY;
  }

private:
  ElementSetType _unused;
  ElementSetType _used;
};

// This class generates boolean vectors which select
// the dimensions (indices) from which the subset
// of unused elements has to be taken for cartesian
// generation.
//
// For example:
// * if there is false at index i, it means that for this
//    index we take unused elements
// * if there is true at index i, it means that for this
//    index we take used elements
// E.g.
// Dimensions:      d1      d2      d3
// -----------------------------------
// Unused elements:  5       2      12
//                   7               5
//                  31
//
// Used elements:    2       8      11
//                           6
// vector = [false, true, false]
// => we use {5, 7, 31}, {8, 6} and {12, 5}
// for cartesian generation
//
class DimSelectionGenerator : public IGenerator<std::vector<bool> >, boost::noncopyable
{
public:
  DimSelectionGenerator() : _exhausted(false) {}

  void addDimension(UPG_DIMENSION_TYPE type)
  {
    TSE_ASSERT(type != EMPTY);

    if (type == UNUSED)
    {
      // For this dimension, we generate always
      // from unused pool
      _base.push_back(false);
      return;
    }

    if (type == USED)
    {
      // For this dimension, we generate always
      // from used pool
      _base.push_back(true);
      return;
    }

    // Type == MIXED
    const uint32_t currentIndex = static_cast<uint32_t>(_base.size());
    // On this index, in general it will vary if we use
    // unused or used elements. _generator will
    // produce for us all combinations of used/unused
    // on each mixed index when consecutive
    // calls to this object's next() will be invoked
    _generator.addElement(currentIndex);
    // By default, we generate from the used pool
    // if an index will be returned by _generator,
    // we will update it to unused
    _base.push_back(true);
  }

  std::vector<bool> next() override
  {
    if (_exhausted)
    {
      return std::vector<bool>();
    }

    // Start from the base, then update
    // dimensions from which we take unused elements
    // using generator output
    const std::vector<unsigned int> takeUnusedFromTheseIndices = _generator.next();

    if (takeUnusedFromTheseIndices.empty())
    {
      _exhausted = true;
      if (baseHasUnusedDimensions())
      {
        // Still a 'raw' base combination is useful,
        // since it contains at least one dimensions
        // with unused elements only
        return _base;
      }
      return std::vector<bool>();
    }

    return buildAnswer(takeUnusedFromTheseIndices);
  }

private:
  bool baseHasUnusedDimensions() const
  {
    for (auto elem : _base)
    {
      if (!elem)
      {
        return true;
      }
    }
    return false;
  }

  std::vector<bool> buildAnswer(const std::vector<unsigned int>& takeUnusedFromTheseIndices) const
  {
    std::vector<bool> ans = _base;
    for (const auto index : takeUnusedFromTheseIndices)
    {
      TSE_ASSERT(index < ans.size());
      ans[index] = false;
    }
    return ans;
  }

  DecLenCombinationsGenerator<unsigned int> _generator;
  std::vector<bool> _base;
  bool _exhausted;
};

// This object is fed with a number of elements:
// a) coming with various indices, where an index corresponds to a dimension
// b) grouped within a dimension into used and unused elements
//
// The role of this object is to generate combinations of elements
// (one from every dimension in a single combination),
// which cover maximum number of unused elements, i.e.
// combinations are generated from those containing most unused elements
// to those that contain least unused elements (but always
// at least one such element).
//
// Example:
// Consider following input elements:
//
// Dimensions:      d1      d2      d3
// -----------------------------------
// Unused elements:  5       2       5
//                   7              12
//                  31
//
// Used elements:    2       6      11
//                           8
//
//
// The generator will emit the following combinations:
// Combination       #Unused elems   Source
//                                   e.g. d1 = unused elements from dim 1
//                                        !d1 = used elements from dim 1
//
//  5   2   5        3               d1 x d2 x d3
//  5   2  12        3
//  7   2   5        3
//  7   2  12        3
// 31   2   5        3
// 31   2  12        3
//
//  5   2  11        2               d1 x d2 x !d3
//  7   2  11        2
// 31   2  11        2
//
//  5   6   5        2               d1 x !d2 x d3
//  5   6  12        2
//  5   8   5        2
//  5   8  12        2
//  7   6   5        2
//  7   6  12        2
//  7   8   5        2
//  7   8  12        2
// 31   6   5        2
// 31   6  12        2
// 31   8   5        2
// 31   8  12        2
//
//  2   2   5        2               !d1 x d2 x d3
//  2   2  12        2
//
//  5   6  11        1               d1 x !d2 x !d3
//  5   8  11        1
//  7   6  11        1
//  7   8  11        1
// 31   6  11        1
// 31   8  11        1
//
//  2   2  11        1               !d1 x d2 x !d3
//
//  2   6   5        1               !d1 x !d2 x d3
//  2   6  12        1
//  2   8   5        1
//  2   8  12        1
// end.
//
// As seen above, first the generator emits combinations
// from unused subsets of elements for each dimension.
// Then it exploits all combinations with exactly one dimension
// collaborating with the subset of used elements.
// Then all combinations with exactly two such dimensions. Etc.
//
template <typename T>
class UsageProgressGenerator : public IGenerator<std::vector<T> >, boost::noncopyable
{
public:
  UsageProgressGenerator(unsigned int dimensions, ILogger* logger = nullptr)
  {
    if (logger != nullptr)
    {
      _log.install(logger);
    }
    TSE_ASSERT(dimensions > 0);
    _dimensions.resize(dimensions);
    reset();
  }

  // Resets the generator, i.e. it will start
  // generating combinations from the beginning
  void reset()
  {
    // Delete subobjects
    _dimSelectionGen.reset();
    _cartProd.reset();

    _exhausted = false;
  }

  // Adds an element
  // (it results in resetting generator state)
  // index -- the dimension where given element is added
  // used -- tells if we are adding elment to unused/used dimension subset
  void addElement(const T& elem, unsigned int index, bool used)
  {
    TSE_ASSERT(index < _dimensions.size());
    if (_log->enabled(LOGGER_LEVEL::DEBUG))
      _log->debug(Fmt("Adding element %s at index %u with used = %d") % elem % index % toStr(used));

    _dimensions[index].addElement(elem, used);
    reset();
  }

  // Removes an element
  // (it results in resetting generator state)
  // index -- the dimension where given element is removed
  //
  // Returns: the number of removed elements (zero or one)
  unsigned int removeElement(const T& elem, unsigned int index)
  {
    TSE_ASSERT(index < _dimensions.size());
    if (_log->enabled(LOGGER_LEVEL::DEBUG))
      _log->debug(Fmt("Removing element %s at index %u") % elem % index);
    const unsigned int ans = _dimensions[index].removeElement(elem);
    reset();
    return ans;
  }

  // Returns the number of dimensions for this generator,
  // as specified in the constructor
  unsigned int getDimensionsCount() const { return static_cast<unsigned int>(_dimensions.size()); }

  // Returns the number of elements for the dimension
  // at index
  unsigned int getDimensionSize(unsigned int index) const
  {
    TSE_ASSERT(index < _dimensions.size());
    return _dimensions[index].getSize();
  }

  // Returns the total number of unused elements
  unsigned int getUnusedCount() const
  {
    size_t answer = 0;
    for (auto& elem : _dimensions)
    {
      answer += elem.getUnusedElements().size();
    }
    return static_cast<unsigned int>(answer);
  }

  // Returns the total number of used elements
  unsigned int getUsedCount() const
  {
    size_t answer = 0;
    for (size_t i = 0; i < _dimensions.size(); ++i)
    {
      answer += _dimensions[i].getUsedElements().size();
    }
    return static_cast<unsigned int>(answer);
  }

  // Returns the total number of stored elements
  unsigned int getSize() const { return getUsedCount() + getUnusedCount(); }

  // Returns new combination (one element per dimension)
  // in the descending order of unused elements count.
  std::vector<T> next() override
  {
    // Not started
    if (_cartProd.get() == nullptr)
    {
      if (getUnusedCount() == 0)
      {
        // Stay in state "not started",
        // since no combinations needed to
        // increase usage
        return std::vector<T>();
      }

      // Initialize
      initDimSelectionGenerator();
      const std::vector<bool> newDimSelection = _dimSelectionGen->next();
      TSE_ASSERT(!newDimSelection.empty());
      _cartProd.reset(cartesianForNewPdim(newDimSelection));
    }

    while (!_exhausted)
    {
      TSE_ASSERT(_cartProd.get() != nullptr);
      TSE_ASSERT(_dimSelectionGen.get() != nullptr);
      std::deque<T> newCombination = _cartProd->getNext();
      if (_log->enabled(LOGGER_LEVEL::DEBUG))
        _log->debug(Fmt("New return combination: %s") % toStr(newCombination));

      if (!newCombination.empty())
      {
        return std::vector<T>(newCombination.begin(), newCombination.end());
      }

      // Generator exhausted for this dimensions selection,
      // try next selection
      const std::vector<bool> newDimSelection = _dimSelectionGen->next();
      if (!newDimSelection.empty())
      {
        _cartProd.reset(cartesianForNewPdim(newDimSelection));
      }
      else
      {
        // No further progress possible = game over
        _exhausted = true;
      }
    }
    return std::vector<T>();
  }

private:
  using ElementVectorType = std::vector<T>;
  using CartProduct = CartesianProduct<ElementVectorType>;

  void initDimSelectionGenerator()
  {
    _dimSelectionGen.reset(new DimSelectionGenerator());
    TSE_ASSERT(_dimSelectionGen.get() != nullptr);
    for (size_t i = 0; i < _dimensions.size(); ++i)
    {
      _dimSelectionGen->addDimension(_dimensions[i].getType());
      if (_log->enabled(LOGGER_LEVEL::DEBUG))
        _log->debug(Fmt("Index %u added to dim selection generator with type %u") % i %
                    _dimensions[i].getType());
    }
  }

  void prepareDimSubsetForGenerator(const typename DimensionData<T>::ElementSetType& elems)
  {
    const ElementVectorType v(elems.begin(), elems.end());
    TSE_ASSERT(!v.empty());
    _currentCartProdInput.push_back(v);
    if (_log->enabled(LOGGER_LEVEL::DEBUG))
      _log->debug(Fmt("Taking subset for generator: %s") % toStr(v));
  }

  CartProduct* cartesianForNewPdim(const std::vector<bool>& dimSelection)
  {
    if (_log->enabled(LOGGER_LEVEL::DEBUG))
      _log->debug(Fmt("New dim selection: %s") % toStr(dimSelection));

    TSE_ASSERT(dimSelection.size() == _dimensions.size());

    CartProduct* c = new CartProduct();
    _currentCartProdInput.clear();

    for (size_t i = 0; i < dimSelection.size(); ++i)
    {
      if (dimSelection[i] == false)
      {
        if (_log->enabled(LOGGER_LEVEL::DEBUG))
          _log->debug(Fmt("Index %u, taking unused elements") % i);
        prepareDimSubsetForGenerator(_dimensions[i].getUnusedElements());
      }
      else
      {
        if (_log->enabled(LOGGER_LEVEL::DEBUG))
          _log->debug(Fmt("Index %u, taking used elements") % i);
        prepareDimSubsetForGenerator(_dimensions[i].getUsedElements());
      }
    }

    transferVectorsToGenerator(c);
    return c;
  }

  void transferVectorsToGenerator(CartProduct* cartProd)
  {
    TSE_ASSERT(cartProd != nullptr);
    for (auto& elem : _currentCartProdInput)
    {
      cartProd->addSet(elem);
    }
  }

  std::vector<DimensionData<T> > _dimensions;
  bool _exhausted = false;
  std::unique_ptr<DimSelectionGenerator> _dimSelectionGen;
  std::unique_ptr<CartProduct> _cartProd;

  // We must hold elements for the cartesian generator
  // separately - the generator does not copy passed
  // containers
  std::vector<ElementVectorType> _currentCartProdInput;
  LoggerHandle _log;
};

} // namespace utils

} // namespace tse

