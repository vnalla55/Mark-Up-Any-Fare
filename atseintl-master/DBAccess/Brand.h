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

namespace tse
{

class Brand
{

public:
  Brand() : _userApplType(' '), _seqno(0), _newSeqno(0) {}

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  BrandCode& brandId() { return _brandId; }
  const BrandCode& brandId() const { return _brandId; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  uint64_t& seqno() { return _seqno; }
  const uint64_t& seqno() const { return _seqno; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  uint64_t& newSeqno() { return _newSeqno; }
  const uint64_t& newSeqno() const { return _newSeqno; }

  std::string& brandName() { return _brandName; }
  const std::string& brandName() const { return _brandName; }

  std::string& brandText() { return _brandText; }
  const std::string& brandText() const { return _brandText; }

  bool operator==(const Brand& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_carrier == rhs._carrier) && (_brandId == rhs._brandId) &&
            (_versionDate == rhs._versionDate) && (_seqno == rhs._seqno) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_newSeqno == rhs._newSeqno) && (_brandName == rhs._brandName) &&
            (_brandText == rhs._brandText));
  }

  static void dummyData(Brand& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._carrier = "FGH";
    obj._brandId = "IJ";
    obj._versionDate = time(nullptr);
    obj._seqno = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._newSeqno = 2;
    obj._brandName = "BrandName";
    obj._brandText = "BrandText";
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  CarrierCode _carrier;
  BrandCode _brandId;
  DateTime _versionDate;
  uint64_t _seqno;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  uint64_t _newSeqno;
  std::string _brandName;
  std::string _brandText;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _brandId);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqno);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _newSeqno);
    FLATTENIZE(archive, _brandName);
    FLATTENIZE(archive, _brandText);
  }

};
}

