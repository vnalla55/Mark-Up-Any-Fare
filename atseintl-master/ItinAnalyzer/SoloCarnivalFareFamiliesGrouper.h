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

class SoloCarnivalFareFamiliesGrouper
{
public:
  // Group Itins into families in-place.
  //
  // Itins are grouped in the same family if
  // travel segments sequence from Itin1 correspond travel segments sequence from Itin2
  // in orig/dest airport pair and carrier.
  //
  // ACMS config variable SOLO_CARNIVAL_OPT\REGROUP_THRU_FAMILIES = "Y"
  //      makes regroup ignore existing family boundaries.
  //
  // NOTE: This function doesn't check/affect neither
  //         Itin::itinFamily,
  //           nor
  //         Itin::headOfFamily.
  //
  static void group(std::vector<tse::Itin*>& inplace);
};
}

