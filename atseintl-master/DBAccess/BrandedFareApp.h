//----------------------------------------------------------------------------
//   2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class BrandedFareApp
{
public:
  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  uint64_t& seqno() { return _seqno; }
  const uint64_t& seqno() const { return _seqno; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& tvlEffDate() { return _tvlEffDate; }
  const DateTime& tvlEffDate() const { return _tvlEffDate; }

  DateTime& tvlDiscDate() { return _tvlDiscDate; }
  const DateTime& tvlDiscDate() const { return _tvlDiscDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  uint64_t& newSeqno() { return _newSeqno; }
  const uint64_t& newSeqno() const { return _newSeqno; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  std::string& marketTypeCode() { return _marketTypeCode; }
  const std::string& marketTypeCode() const { return _marketTypeCode; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  BrandCode& brandId() { return _brandId; }
  const BrandCode& brandId() const { return _brandId; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  bool operator==(const BrandedFareApp& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_carrier == rhs._carrier) && (_versionDate == rhs._versionDate) &&
            (_seqno == rhs._seqno) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_tvlEffDate == rhs._tvlEffDate) &&
            (_tvlDiscDate == rhs._tvlDiscDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_newSeqno == rhs._newSeqno) &&
            (_directionality == rhs._directionality) && (_marketTypeCode == rhs._marketTypeCode) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_brandId == rhs._brandId) &&
            (_bookingCode == rhs._bookingCode));
  }

  static void dummyData(BrandedFareApp& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._carrier = "FGH";
    obj._versionDate = time(nullptr);
    obj._seqno = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._tvlEffDate = time(nullptr);
    obj._tvlDiscDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._newSeqno = 2;
    obj._directionality = BOTH;
    obj._marketTypeCode = "aaaaaaaa";

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._brandId = "bbbbbbbb";
    obj._bookingCode = "IJ";
  }

private:
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  CarrierCode _carrier;
  DateTime _versionDate;
  uint64_t _seqno = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _tvlEffDate;
  DateTime _tvlDiscDate;
  DateTime _effDate;
  DateTime _discDate;
  uint64_t _newSeqno = 0;
  Directionality _directionality = Directionality::TERMINATE;
  std::string _marketTypeCode;
  LocKey _loc1;
  LocKey _loc2;
  BrandCode _brandId;
  BookingCode _bookingCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqno);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _tvlEffDate);
    FLATTENIZE(archive, _tvlDiscDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _newSeqno);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _marketTypeCode);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _brandId);
    FLATTENIZE(archive, _bookingCode);
  }

};
}

