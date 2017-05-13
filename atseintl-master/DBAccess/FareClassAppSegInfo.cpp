//----------------------------------------------------------------------------
//
//  File:           FareClassAppSegInfo.cpp
//  Created:        4/12/2004
//  Authors:        KS
//
//  Description:    FareClassAppSegInfo class is cacheable object, which stores
//                  fields for one FareClassAppSegInfo record.
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "DBAccess/FareClassAppSegInfo.h"

#include "DBAccess/DataHandle.h"

#include <vector>

namespace tse
{
const FareClassAppSegInfo FareClassAppSegInfo::_emptyFareClassAppSeg;

FareClassAppSegInfo*
FareClassAppSegInfo::clone(DataHandle& dataHandle) const
{
  FareClassAppSegInfo* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(*cloneObj); // lint !e413

  return cloneObj;
}

void
FareClassAppSegInfo::clone(FareClassAppSegInfo& cloneObj) const
{

  cloneObj._directionality = _directionality;
  cloneObj._bookingCodeTblItemNo = _bookingCodeTblItemNo;
  cloneObj._overrideDateTblNo = _overrideDateTblNo;
  cloneObj._minAge = _minAge;
  cloneObj._maxAge = _maxAge;
  cloneObj._tktCode = _tktCode;
  cloneObj._tktCodeModifier = _tktCodeModifier;
  cloneObj._tktDesignator = _tktDesignator;
  cloneObj._tktDesignatorModifier = _tktDesignatorModifier;
  cloneObj._paxType = _paxType;

  for (int i = 0; i < BK_CODE_SIZE; ++i)
    cloneObj._bookingCode[i] = _bookingCode[i];

  cloneObj._seqNo = _seqNo;
  cloneObj._carrierApplTblItemNo = _carrierApplTblItemNo;
}
}
