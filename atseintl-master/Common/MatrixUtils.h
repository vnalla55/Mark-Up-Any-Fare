/*---------------------------------------------------------------------------
 *  File:   MatrixUtils.h
 *  Author: David White
 *
 *  Copyright Sabre 2003
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include "Common/Assert.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

namespace tse
{
// iterator which will iterate over a matrix (generally a series of flight
// combinations), in a sequence that produces nice diverse results.
// Results closer to the top and left of the matrix are considered
// more desirable. Also, a 'Rating' functor is passed in which can
// rate the qualify of any cell. It's assumed that cells closer to the
// top-left will always have the same or better results than cells
// further right or down.
//
// To give a diverse set of results, the iterator starts by iterating
// down the long diagonal of the matrix, from the top-left to bottom-right,
// until it strikes a cell that is rated worse than the top-left cell.
// then it will
template <typename Rating>
class MatrixRatingIterator
{
public:
  typedef std::vector<int> Vector;
  explicit MatrixRatingIterator(const Vector& v, Rating rater = Rating())
    : _pos(v.size(), 0), _dimensions(v), _sumTo(1), _diagonal(-1), _rater(rater), _rating(0)
  {
    if (_dimensions.empty() ||
        std::find(_dimensions.begin(), _dimensions.end(), 0) != _dimensions.end())
    {
      _pos.clear();
    }
    else
    {
      _rating = rater(_pos);
    }
  }

  const Vector& value() const { return _pos; }

  void next()
  {
    if (_diagonal == -1)
    {
      bool inLoc = true;
      for (size_t n = 0; n != _pos.size(); ++n)
      {
        _pos[n]++;
        if (_pos[n] >= _dimensions[n])
        {
          inLoc = false;
          break;
        }
      }

      // we start the next phase now
      if (!inLoc || (_dimensions.size() > 1 && _rater(_pos) != _rating))
      {
        if (_dimensions.size() == 1)
        {
          _pos.clear();
          return;
        }

        _diagonal = _pos.front();
        const bool res =
            initSumTo(_pos.begin(), _pos.end(), _sumTo, _dimensions.begin(), _dimensions.end());
        if (res == false)
        {
          _pos.clear();
        }
      }
    }
    else
    {
      const bool res =
          nextComboSumTo(_pos.begin(), _pos.end(), _sumTo, _dimensions.begin(), _dimensions.end());
      if (res == false)
      {
        ++_sumTo;
        const bool res =
            initSumTo(_pos.begin(), _pos.end(), _sumTo, _dimensions.begin(), _dimensions.end());
        if (res == false)
        {
          _pos.clear();
          return;
        }
      }

      if (UNLIKELY(_pos.front() < _diagonal && onDiagonal(_pos.begin(), _pos.end())))
      {
        next();
      }
    }
  }

  bool atEnd() const { return _pos.empty(); }

private:
  Vector _pos, _dimensions;
  int _sumTo;
  int _diagonal;
  Rating _rater;
  int _rating;

  typedef Vector::iterator iterator;
  typedef Vector::const_iterator const_iterator;

  static bool onDiagonal(const_iterator i1, const_iterator i2)
  {
    return std::adjacent_find(i1, i2, std::not_equal_to<int>()) == i2;
  }

  static bool initSumTo(iterator i1, iterator i2, int sumto, const_iterator m1, const_iterator m2)
  {
    if (i1 == i2)
    {
      return sumto == 0;
    }

    *i1 = *m1 <= sumto ? *m1 - 1 : sumto;
    return initSumTo(i1 + 1, i2, sumto - *i1, m1 + 1, m2);
  }

  static bool
  nextComboSumTo(iterator i1, iterator i2, int sumto, const_iterator m1, const_iterator m2)
  {
    if (UNLIKELY(i2 - i1 < 2))
    {
      return false;
    }

    if (i2 - i1 == 2)
    {
      if (*i1 == 0)
      {
        return false;
      }
      else
      {
        *i1 = *i1 - 1;
        ++i1;
        ++m1;
        *i1 = *i1 + 1;
        return *i1 < *m1;
      }
    }

    if (nextComboSumTo(i1 + 1, i2, sumto - *i1, m1 + 1, m2))
    {
      return true;
    }
    else if (*i1 == 0)
    {
      return false;
    }
    else
    {
      *i1 = *i1 - 1;
      return initSumTo(i1 + 1, i2, sumto - *i1, m1 + 1, m2);
    }
  }
};
}

