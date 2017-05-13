//----------------------------------------------------------------------------
//
//  File:           FareClassAppSegInfo.h
//  Created:        2/3/2004
//  Authors:
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

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

//----------------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//----------------------------------------------------------------------------

namespace tse
{
class DataHandle;

class FareClassAppSegInfo
{
public:
  FareClassAppSegInfo()
    : _directionality(DIR_IND_NOT_DEFINED),
      _minAge(0),
      _bookingCodeTblItemNo(0),
      _overrideDateTblNo(0),
      _maxAge(0),
      _carrierApplTblItemNo(0),
      _seqNo(0)

  {
  }

  static constexpr int BK_CODE_SIZE = 8; // Bk Code vector max. size

  FCASegDirectionality _directionality; // directionality
  int _minAge;
  long _bookingCodeTblItemNo;
  long _overrideDateTblNo;
  int _maxAge;
  TktCode _tktCode;
  TktCodeModifier _tktCodeModifier;
  TktDesignator _tktDesignator;
  TktDesignatorModifier _tktDesignatorModifier;
  PaxTypeCode _paxType; // passenger type
  BookingCode _bookingCode[BK_CODE_SIZE]; // booking codes
  int _carrierApplTblItemNo;
  long _seqNo; // sequence number

  bool operator==(const FareClassAppSegInfo& rhs) const
  {
    bool eq =
        ((_directionality == rhs._directionality) &&
         (_bookingCodeTblItemNo == rhs._bookingCodeTblItemNo) &&
         (_overrideDateTblNo == rhs._overrideDateTblNo) && (_minAge == rhs._minAge) &&
         (_maxAge == rhs._maxAge) && (_tktCode == rhs._tktCode) &&
         (_tktCodeModifier == rhs._tktCodeModifier) && (_tktDesignator == rhs._tktDesignator) &&
         (_tktDesignatorModifier == rhs._tktDesignatorModifier) && (_paxType == rhs._paxType) &&
         (_seqNo == rhs._seqNo) && (_carrierApplTblItemNo == rhs._carrierApplTblItemNo));

    for (int i = 0; (eq && (i < BK_CODE_SIZE)); ++i)
    {
      eq = (_bookingCode[i] == rhs._bookingCode[i]);
    }

    return eq;
  }

  static void dummyData(FareClassAppSegInfo& obj)
  {
    obj._directionality = ORIGINATING_LOC1;
    obj._bookingCodeTblItemNo = 1111;
    obj._overrideDateTblNo = 2222;
    obj._minAge = 3;
    obj._maxAge = 4;
    obj._tktCode = "ABCDEFGHIJ";
    obj._tktCodeModifier = "KLM";
    obj._tktDesignator = "NOPQRSTUVW";
    obj._tktDesignatorModifier = "XYZ";
    obj._paxType = "abc";
    obj._seqNo = 5555;
    obj._carrierApplTblItemNo = 6;

    char bk = 'a';
    for (int i = 0; i < BK_CODE_SIZE; ++i, ++bk)
    {
      obj._bookingCode[i] = bk;
    }
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _bookingCodeTblItemNo);
    FLATTENIZE(archive, _overrideDateTblNo);
    FLATTENIZE(archive, _minAge);
    FLATTENIZE(archive, _maxAge);
    FLATTENIZE(archive, _tktCode);
    FLATTENIZE(archive, _tktCodeModifier);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _tktDesignatorModifier);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _bookingCode);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _carrierApplTblItemNo);
  }

  static const FareClassAppSegInfo* emptyFareClassAppSeg()
  {
    return &_emptyFareClassAppSeg;
  };

  /**
   * This methods obtains a new FareClassAppSegInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  FareClassAppSegInfo* clone(DataHandle& dataHandle) const;

  /**
   * This methods populates a given FareClassAppSegInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  void clone(FareClassAppSegInfo& cloneObj) const;

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  static const FareClassAppSegInfo _emptyFareClassAppSeg;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_directionality & ptr->_bookingCodeTblItemNo & ptr->_overrideDateTblNo &
           ptr->_minAge & ptr->_maxAge & ptr->_tktCode & ptr->_tktCodeModifier &
           ptr->_tktDesignator & ptr->_tktDesignatorModifier & ptr->_paxType & ptr->_bookingCode &
           ptr->_seqNo & ptr->_carrierApplTblItemNo;
  }
  // Placed here so the clone methods must be used
  FareClassAppSegInfo(const FareClassAppSegInfo& rhs);
  FareClassAppSegInfo& operator=(const FareClassAppSegInfo& rhs);
};

typedef std::vector<FareClassAppSegInfo*> FareClassAppSegInfoList;

} // namespace tse

