// ---------------------------------------------------------------------------
//  File:         BookingCodeExceptionSequence.h
//          ****** This is NOT a C++ Exception *****

// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

#include <stdlib.h>

namespace tse
{

/**
BookingCodeExceptionSequence contains the data base data from the
BookingCodeException table and includes a vector of segments.  This
//class is contained in a vector in BookingCodeException.
*/

class BookingCodeExceptionSequence
{

public:
  BookingCodeExceptionSequence()
    : _itemNo(0),
      _primeInd(' '),
      _tableType(' '),
      _seqNo(0),
      _constructSpecified(' '),
      _ifTag(' '),
      _segCnt(0),
      _skipFareClassOrTypeMatching(false),
      _skipMatching(false)
  {
  }
  virtual ~BookingCodeExceptionSequence()
  {
    BookingCodeExceptionSegmentVector::iterator i;
    for (i = _segmentVector.begin(); i < _segmentVector.end(); i++)
      delete *i;
  }

  const VendorCode& vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }

  const DateTime& createDate() const { return _createDate; }
  DateTime& createDate() { return _createDate; }

  const DateTime& expireDate() const { return _expireDate; }
  DateTime& expireDate() { return _expireDate; }

  const uint32_t& itemNo() const { return _itemNo; }
  uint32_t& itemNo() { return _itemNo; }

  const char& primeInd() const { return _primeInd; }
  char& primeInd() { return _primeInd; }

  const char& tableType() const { return _tableType; }
  char& tableType() { return _tableType; }

  const uint32_t& seqNo() const { return _seqNo; }
  uint32_t& seqNo() { return _seqNo; }

  const char& constructSpecified() const { return _constructSpecified; }
  char& constructSpecified() { return _constructSpecified; }

  const char& ifTag() const { return _ifTag; }
  char& ifTag() { return _ifTag; }

  const uint16_t& segCnt() const { return _segCnt; }
  uint16_t& segCnt() { return _segCnt; }

  const BookingCodeExceptionSegmentVector& segmentVector() const { return _segmentVector; }
  BookingCodeExceptionSegmentVector& segmentVector() { return _segmentVector; }

  // --------------------------------------------------------
  // This fields are never serialized in LDC cache.
  // They are lazily computed by booking code validation code (booking code index builder).
  bool isSkipFareClassOrTypeMatching() const { return _skipFareClassOrTypeMatching; }
  void setSkipFareClassOrTypeMatching(bool value) { _skipFareClassOrTypeMatching = value; }
  bool isSkipMatching() const { return _skipMatching; }
  void setSkipMatching(bool value) { _skipMatching = value; }
  // --------------------------------------------------------

  virtual bool operator==(const BookingCodeExceptionSequence& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_itemNo == rhs._itemNo) &&
            (_primeInd == rhs._primeInd) && (_tableType == rhs._tableType) &&
            (_seqNo == rhs._seqNo) && (_constructSpecified == rhs._constructSpecified) &&
            (_ifTag == rhs._ifTag) && (_segCnt == rhs._segCnt) &&
            (_segmentVector.size() == rhs._segmentVector.size()));

    for (size_t i = 0; (eq && (i < _segmentVector.size())); ++i)
    {
      eq = (*(_segmentVector[i]) == *(rhs._segmentVector[i]));
    }

    return eq;
  }

  static void dummyData(BookingCodeExceptionSequence& obj)
  {
    obj._vendor = "EFGH";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._itemNo = 1;
    obj._primeInd = 'A';
    obj._tableType = 'B';
    obj._seqNo = 2;
    obj._constructSpecified = 'C';
    obj._ifTag = 'D';
    obj._segCnt = 3;

    BookingCodeExceptionSegment* bces1 = new BookingCodeExceptionSegment;
    BookingCodeExceptionSegment* bces2 = new BookingCodeExceptionSegment;

    BookingCodeExceptionSegment::dummyData(*bces1);
    BookingCodeExceptionSegment::dummyData(*bces2);

    obj._segmentVector.push_back(bces1);
    obj._segmentVector.push_back(bces2);
  }

protected:
  //                    Data is from Booking Code Exception Table.

  VendorCode _vendor;
  DateTime _createDate;
  DateTime _expireDate;
  uint32_t _itemNo;
  char _primeInd;
  char _tableType;
  uint32_t _seqNo;
  char _constructSpecified;
  char _ifTag;
  uint16_t _segCnt;
  BookingCodeExceptionSegmentVector _segmentVector; // vector of  segments

  // --------------------------------------------------------
  // This fields are never serialized in LDC cache.
  // They are lazily computed by booking code validation code
  bool _skipFareClassOrTypeMatching;
  bool _skipMatching;

  // --------------------------------------------------------
public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _primeInd);
    FLATTENIZE(archive, _tableType);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _constructSpecified);
    FLATTENIZE(archive, _ifTag);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _segmentVector);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

private:
  BookingCodeExceptionSequence(const BookingCodeExceptionSequence&);
  BookingCodeExceptionSequence& operator=(const BookingCodeExceptionSequence&);

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_createDate & ptr->_expireDate & ptr->_itemNo &
           ptr->_primeInd & ptr->_tableType & ptr->_seqNo & ptr->_constructSpecified & ptr->_ifTag &
           ptr->_segCnt & ptr->_segmentVector;
  }
};

} // end tse namespace

//  Updates:
//          xx/xx/xx - iii - desc

