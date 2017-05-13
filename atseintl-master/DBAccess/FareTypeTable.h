//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

#include <cstdlib>

namespace tse
{

class FareTypeTable
{
public:
  FareTypeTable() = default;
  FareTypeTable(const FareTypeTable&) = delete;
  FareTypeTable& operator=(const FareTypeTable&) = delete;

  bool operator==(const FareTypeTable& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_fareType == rhs._fareType) &&
            (_expireDate == rhs._expireDate) && (_inhibit == rhs._inhibit) &&
            (_fareTypeAppl == rhs._fareTypeAppl));
  }

  static void dummyData(FareTypeTable& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 123;
    obj._createDate = time(nullptr);
    obj._fareType = "ABC";
    obj._expireDate = time(nullptr);
    obj._inhibit = 'X';
    obj._fareTypeAppl = 'X';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _fareTypeAppl);
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  FareTypeAbbrev& fareType() { return _fareType; }
  const FareTypeAbbrev& fareType() const { return _fareType; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator inhibit() const { return _inhibit; }

  Indicator& fareTypeAppl() { return _fareTypeAppl; }
  const Indicator fareTypeAppl() const { return _fareTypeAppl; }

private:
  VendorCode _vendor;
  int _itemNo = 0;
  DateTime _createDate;
  FareTypeAbbrev _fareType;
  DateTime _expireDate;
  Indicator _inhibit = ' ';
  Indicator _fareTypeAppl = ' ';

};
}

