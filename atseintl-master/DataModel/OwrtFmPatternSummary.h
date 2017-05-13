// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#include "Common/ShpqTypes.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"

#include <boost/array.hpp>

namespace tse
{
namespace shpq
{

class OwrtFmPatternSummary
{
public:
  enum Pattern
  {
    EOE = 0,
    NOT_EOE,
    NUM_PATTERNS
  };

  typedef boost::array<MoneyAmount, NUM_PATTERNS> LowerBoundPerPattern;

  OwrtFmPatternSummary() : _isInitialized(false)
  {
    _lowerBound.assign(static_cast<MoneyAmount>(UNKNOWN_MONEY_AMOUNT));
  }

  bool isInitialized() const { return _isInitialized; }
  void setInitialized(bool v) { _isInitialized = v; }

  bool isInvalidForPattern(Pattern p) const
  {
    return _lowerBound[p] >= static_cast<MoneyAmount>(UNKNOWN_MONEY_AMOUNT) - EPSILON;
  }

  MoneyAmount lowerBoundForPattern(Pattern p) const { return _lowerBound[p]; }

  void setLowerBoundForPattern(Pattern p, MoneyAmount lb) { _lowerBound[p] = lb; }

private:
  LowerBoundPerPattern _lowerBound;
  bool _isInitialized;
};
}
}

