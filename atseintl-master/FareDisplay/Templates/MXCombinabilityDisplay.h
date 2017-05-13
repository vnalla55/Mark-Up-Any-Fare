//-------------------------------------------------------------------
//
//  File:        MXCombinabilityDisplay.h
//  Author:      Doug Batchelor
//  Created:     Jan 10, 2006
//  Description: A class to provide the processing
//               for displaying combinability MX info.
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//---------------------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{
class CombinabilityRuleInfo;
class FareDisplayTrx;
class PaxTypeFare;

// -------------------------------------------------------------------
// <PRE>
//
// @class MXCombinabilityDisplay
//
// A class to provide an interface API for the Fare Display Template processing to use
// to retrieve the data the Display Template must show.
//
//
// </PRE>
// -------------------------------------------------------------------------

class MXCombinabilityDisplay
{
public:
  MXCombinabilityDisplay();
  virtual ~MXCombinabilityDisplay();

  // Call this method after instancing an InfoBase object to
  // initialize it properly.
  bool getMXInfo(FareDisplayTrx& trx,
                 PaxTypeFare& paxTypeFare,
                 bool isLocationSwapped,
                 std::string& rep);
  void displayMXInfo(const PaxTypeFare& ptf, const CombinabilityRuleInfo* cri, std::string& rep);
};

} // namespace tse

