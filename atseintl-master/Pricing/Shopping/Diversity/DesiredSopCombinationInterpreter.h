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
#pragma once

#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <utility>

#include <tr1/functional>

/**
 * DesiredSopCombinationIterpreter and adaptors for SOPCombinationList and simply SopIdxVec
 */
namespace tse
{

class ItinStatistic;
class ShoppingTrx;

/**
 * Create linear structure of isOptionBetterByXXYY predicates and run lexicographical compare
 * to find out which option is better considering as many predicates, as needed to find it out
 */
template <typename T>
class DesiredSopCombinationInterpreter
{
public:
  enum CompareResult
  {
    LESS = -1,
    EQUAL = 0,
    GREATER = 1
  };

  // interpreter "operand"
  typedef T Operand;

  DesiredSopCombinationInterpreter(const ShoppingTrx& trx, const ItinStatistic& stats)
    : _trx(trx), _stats(stats)
  {
  }

  /**
   * @return true if lhs is better than rhs
   */
  CompareResult compare(const Operand& lhs, const Operand& rhs)
  {
    typename ClauseList::iterator clauseIt = _clauseList.begin();
    while (clauseIt != _clauseList.end())
    {
      Clause& comparator = *clauseIt;
      int cmpResult = comparator(lhs, rhs);

      if (cmpResult < 0)
        return LESS;

      if (cmpResult > 0)
        return GREATER;

      ++clauseIt;
    }

    return EQUAL;
  }

  bool operator()(const Operand& lhs, const Operand& rhs) { return compare(lhs, rhs) == GREATER; }

protected:
  typedef std::tr1::function<int(const Operand& lhs, const Operand& rhs)> Clause;
  typedef std::list<Clause> ClauseList;

  ClauseList _clauseList;

  const ShoppingTrx& _trx;
  const ItinStatistic& _stats;
};

} // ns tse

