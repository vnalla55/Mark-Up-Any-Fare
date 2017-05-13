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
#include "Pricing/Shopping/Swapper/PositionalScore.h"

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace swp
{

// Adds a number of score vectors with offsets to create a positional
// score, in the manner below:
//
// vector a, offset = 1:       [a0         a1         a2]
// vector b, offset = 0:  [b0         b1         b2]
// vector c, offset = 1:       [c0         c1         c2]
//
// + : ---------------------------------------------------
//
// PositionalScore        [P00][P01]  [P10][P11]  [P20][P21]
// with wordsCount = 3     =    =      =    =      =    =
// and offsetsCount = 2    b0   a0+    b1   a1+    b2   a2+
//                              c0          c1          c2
//
class ScoreVectorSummator : boost::noncopyable
{
public:
  typedef std::vector<int> ScoreVector;

  // The number of words is fixed for the lifetime of
  // object, the number of offsets can be changed in newScore()
  ScoreVectorSummator(unsigned int wordsCount) : _offsetsCount(0)
  {
    TSE_ASSERT(wordsCount > 0);
    _wordsCount = wordsCount;
  }

  // Creates a new clear PositionalScore with the
  // number of words set in constructor and
  // given offsetCount (which must be positive)
  void newScore(unsigned int offsetCount)
  {
    TSE_ASSERT(offsetCount > 0);
    _offsetsCount = offsetCount;
    clearScore();
  }

  // Adds vector v with given offset to the contained score
  void addScoreVector(unsigned int offset, const ScoreVector& v)
  {
    TSE_ASSERT(v.size() == _wordsCount);
    TSE_ASSERT(offset < _offsetsCount);
    addVector(offset, v);
  }

  // Returns the contained score
  const PositionalScore& getScore() const
  {
    // positive _offsetsCount = at least one call
    // to newScore in the past
    TSE_ASSERT(_offsetsCount > 0);
    return _currentScore;
  }

  // Returns the number N of valid offsets
  // [0 .. N-1] for this object
  unsigned int getOffsetsCount() const { return _offsetsCount; }

  // Returns the number of words in the contained
  // positional score (lifetime fixed)
  unsigned int getWordsCount() const { return _wordsCount; }

private:
  void clearScore() { _currentScore = PositionalScore(_wordsCount, _offsetsCount, 0); }

  void addVector(unsigned int offset, const ScoreVector& v)
  {
    for (unsigned int i = 0; i < _wordsCount; ++i)
    {
      const int current = _currentScore.getValue(i, offset);
      _currentScore.setValue(i, offset, current + v[i]);
    }
  }

  unsigned int _wordsCount;
  unsigned int _offsetsCount;
  PositionalScore _currentScore;
};

} // namespace swp

} // namespace tse

