//-------------------------------------------------------------------
//
//  File:        AccTvlFarePath.h
//  Created:     Feb 27, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description: Object store/restore all accompanied travel restriction for
//             group fare path validation
//
//  Copyright Sabre 2006
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

#include "DataModel/PaxType.h"

namespace tse
{
class SimplePaxTypeFare;

class AccTvlFarePath
{
public:
  PaxType& paxType() { return _paxType; }
  const PaxType& paxType() const { return _paxType; }

  PaxTypeCode& truePaxType() { return _truePaxType; }
  const PaxTypeCode& truePaxType() const { return _truePaxType; }

  std::vector<SimplePaxTypeFare*>& paxTypeFares() { return _paxTypeFares; }
  const std::vector<SimplePaxTypeFare*>& paxTypeFares() const { return _paxTypeFares; }

private:
  PaxType _paxType;
  PaxTypeCode _truePaxType; // same as CalcTotals.truePaxType
  std::vector<SimplePaxTypeFare*> _paxTypeFares;
};

} // tse namespace

