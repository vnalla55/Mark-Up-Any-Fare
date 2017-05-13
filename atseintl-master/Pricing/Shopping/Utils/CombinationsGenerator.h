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

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace utils
{

// Returns combinations of given length (subsequences)
// from a sequence of input elements.
// Combinations are generated in lexicographical order,
// i.e. if the input sequence is sorted, combinations
// will be emitted in sorted order.
//
// E.g.
// Input sequence: abcd
// Length = 3
// Emitted combinations:
// abc
// abd
// acd
// bcd
template <typename T>
class CombinationsGenerator : public IGenerator<std::vector<T> >, boost::noncopyable
{
public:
  CombinationsGenerator(unsigned int length = 0) { reset(length); }

  void reset()
  {
    // Set indices to 0, 1, 2 ...
    for (size_t i = 0; i < _indices.size(); ++i)
    {
      _indices[i] = static_cast<int>(i);
    }
    _exhausted = false;
  }

  void reset(unsigned int length)
  {
    _indices.resize(length);
    reset();
  }

  void addElement(const T& element) { _inputElements.push_back(element); }

  std::vector<T> next() override
  {
    const size_t len = _indices.size();
    // If the combinations length is greater
    // than the number of elements, there is nothing
    // to generate
    // Same for zero-length combinations
    if ((len > _inputElements.size()) || (len == 0))
    {
      return std::vector<T>();
    }

    if (_exhausted)
    {
      return std::vector<T>();
    }

    std::vector<T> out;
    readCurrentCombination(out);

    if (advanceIndices() == false)
    {
      _exhausted = true;
    }
    return out;
  }

  unsigned int getLength() const { return static_cast<unsigned int>(_indices.size()); }

  unsigned int getElementsCount() const { return static_cast<unsigned int>(_inputElements.size()); }

private:
  void readCurrentCombination(std::vector<T>& out) const
  {
    const size_t len = _indices.size();
    out.resize(len);
    for (size_t j = 0; j < len; ++j)
    {
      out[j] = _inputElements[_indices[j]];
    }
  }

  // Moves indices to next position, e.g.
  // from 012 to 013 for 4 or more elems.
  // Returns true if there is a next valid
  // combination under the modified position of indices
  // Returns false otherwise, i.e. the generator is exhausted
  bool advanceIndices()
  {
    bool broken = false;

    const size_t len = _indices.size();
    int i = static_cast<int>(len) - 1;

    // Looks if there is an index which is not in its terminal position
    // For example, for five elements and three-length combinations,
    // the terminal positions of indices are 2, 3 and 4:
    // [--***]
    // If all indices are in the terminal positions, all combinations
    // are exhausted
    int terminalModifier = static_cast<int>(_inputElements.size()) - static_cast<int>(len);
    for (; i >= 0; --i)
    {
      if (_indices[i] != (i + terminalModifier))
      {
        broken = true;
        break;
      }
    }

    if (!broken)
    {
      // Generator exhausted
      return false;
    }

    // We recalculate indices for next (lexicographically)
    // valid combination to generate
    //
    // We found that i is the "oldest" index
    // we can increment (since not in its terminal position)
    ++_indices[i];

    // If the index is not the last one (i != len-1),
    // the next valid indices arrangement is ascending,
    // beginning from the value of i.
    // For example, for five elements and three-length combinations
    // after indices 034: [*--**]
    //         comes 123: [-***-]
    for (size_t j = i + 1; j < len; ++j)
    {
      _indices[j] = _indices[j - 1] + 1;
    }
    return true;
  }

  std::vector<T> _inputElements;
  std::vector<int> _indices;
  bool _exhausted = false;
};

} // namespace utils

} // namespace tse
