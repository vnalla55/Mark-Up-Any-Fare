// Copyright Sabre 2011
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.

#pragma once

#include <vector>

namespace tse
{
class Itin;

class SoloCarnivalFareFamiliesGrouperImpl
{
public:
  typedef std::vector<tse::Itin*> ItinVec;

  static void groupWithinFamilies(ItinVec& inplace);

  // This will group itins not within original (IS) families only,
  // but thru all of the families in common, ignoring existing family boundaries.
  static void groupThruFamilies(ItinVec& inplace);

private:
  static void createFareFamilies(const ItinVec& input, ItinVec& globalResult);
};

} // namespace tse

