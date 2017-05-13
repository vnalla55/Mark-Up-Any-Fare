//-------------------------------------------------------------------
//
//  File:        AccTvlDetailIn.h
//  Created:     Feb 27, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description: Object restore all accompanied travel restriction for
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

#include "Common/AccTvlDetailReader.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AccTvlFarePath.h"
#include "DataModel/SimplePaxTypeFare.h"
#include "DBAccess/DataHandle.h"

namespace tse
{

class AccTvlDetailIn
{
public:
  // functions to restore accompanied restriciton
  bool restoreAccTvlDetail(DataHandle& dataHandle,
                           const std::vector<const std::string*>& accTvlData,
                           std::vector<const AccTvlFarePath*>& farePaths);

  bool restoreAccTvlDetail(DataHandle& dataHandle,
                           const std::string& accTvlDataStr,
                           AccTvlFarePath& farePath);

  static constexpr char RuleHead = 'R';

private:
  bool restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                           DataHandle& dataHandle,
                           AccTvlFarePath& farePath);

  bool restoreAccTvlDetail(AccTvlDetailReader& inputObj, PaxType& paxType);

  bool restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                           DataHandle& dataHandle,
                           SimplePaxTypeFare& paxTypeFare);

  bool restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                           DataHandle& dataHandle,
                           SimpleAccTvlRule& simpleAccTvlRule);

  bool restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                           DataHandle& dataHandle,
                           SimpleAccTvlRule::AccTvlOpt& accTvlOpt);
};

} // tse namespace

