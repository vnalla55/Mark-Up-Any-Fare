//-------------------------------------------------------------------
//
//  File:        PaxType.cpp
//  Created:     March 8, 2004
//  Authors:     Vadim Nikushin
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
//
//  Copyright Sabre 2004
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

#include "DataModel/PaxType.h"

#include "Common/TseConsts.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PaxTypeMatrix.h"

#include <vector>

namespace tse
{
/* @TODO need to add UnknownPaxType.
*/

bool
PaxType::
operator==(const PaxType& paxType) const
{ // lint !e578
  if ((_paxType == paxType._paxType) && (_requestedPaxType == paxType._requestedPaxType) &&
      (_number == paxType._number) && (_age == paxType._age) &&
      (_stateCode == paxType._stateCode) && (_totalPaxNumber == paxType._totalPaxNumber) &&
      (_inputOrder == paxType._inputOrder) && (_vendorCode == paxType._vendorCode) &&
      (_paxTypeInfo == paxType._paxTypeInfo) && (_actualPaxType == paxType._actualPaxType))
  {
    return true;
  }
  return false;
}
} // tse namespace
