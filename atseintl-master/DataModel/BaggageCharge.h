//-------------------------------------------------------------------
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

#include "Common/TseConsts.h"
#include "ServiceFees/OCFees.h"

#include <bitset>

namespace tse
{
class BaggageCharge : public OCFees
{
public:
  BaggageCharge() = default;

  void setMatched1stBag(bool mp) { _matchedBags.set(0, mp); }
  bool matched1stBag() const { return _matchedBags[0]; }

  void setMatched2ndBag(bool mp) { _matchedBags.set(1, mp); }
  bool matched2ndBag() const { return _matchedBags[1]; }

  void setMatchedBag(const size_t bagNo, const bool val = true) { _matchedBags.set(bagNo, val); }
  bool matchedBag(const size_t bagNo) const { return _matchedBags[bagNo]; }

  const std::bitset<MAX_BAG_PIECES>& matchedBags() const { return _matchedBags; }
  std::bitset<MAX_BAG_PIECES>& mutableMatchedBags() { return _matchedBags; }

  void cleanBaggageResults()
  {
    OCFees::cleanBaggageResults();
    _matchedBags.reset();
  }

private:
  std::bitset<MAX_BAG_PIECES> _matchedBags;
};
} // tse
