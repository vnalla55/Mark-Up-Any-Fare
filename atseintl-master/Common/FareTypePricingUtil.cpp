//----------------------------------------------------------------------------
//
//  File:        FareTypePricingUtil.cpp
//  Created:     5/24/2006
//  Author :     Kul Shekhar
//
//  Description: Common functions required for processing for WPT/NL, WPT/EX
//               and WPT/IT entries.
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
//----------------------------------------------------------------------------

#include "Common/FareTypePricingUtil.h"

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
namespace FareTypePricingUtil
{
bool
matchNormalPaxType(const PaxTypeCode& paxType)
{
  return (paxType.empty() || paxType == ADULT || paxType == CHILD || paxType == INFANT ||
          paxType == INS);
}
}
}
