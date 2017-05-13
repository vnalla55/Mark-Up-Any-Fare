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

#include "Common/TseCodeTypes.h"

#include <utility>
#include <vector>

namespace tse
{

namespace utils
{

// Info about a SOP which may be collected
struct SopCandidate
{
  uint32_t sopId;
  unsigned int legId;
  CarrierCode carrierCode;
  bool isFlightDirect;
};

// A tuple: (leg id, SOP id) for unique SOP identification
struct SopEntry
{
  SopEntry(unsigned int leg, unsigned int sop) : legId(leg), sopId(sop) {}
  unsigned int legId;
  unsigned int sopId;
  bool operator<(const SopEntry& right) const
  {
    if (legId < right.legId)
    {
      return true;
    }

    if (right.legId < legId)
    {
      return false;
    }

    return sopId < right.sopId;
  }
  bool operator==(const SopEntry& right) const
  {
    return (legId == right.legId) && (sopId == right.sopId);
  }
};

// Vector of ints is widely used in shopping code
// although single sop ids are stored as uint32_t
//
// Representation as vector [8, 5, 17]
// can be understood as a list: (leg 0, sop 8), (leg 1, sop 5), (leg 2, sop 17)
typedef std::vector<int> SopCombination;

typedef std::vector<SopCombination> SopCombinationList;

} // namespace utils

} // namespace tse

