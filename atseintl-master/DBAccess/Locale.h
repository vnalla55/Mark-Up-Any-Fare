//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class Locale
{
public:
  Locale() : _itemNo(0), _orderNo(0), _locAppl(' ') {}

  bool operator==(const Locale& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_orderNo == rhs._orderNo) &&
            (_locAppl == rhs._locAppl) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(Locale& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._orderNo = 2;
    obj._locAppl = 'E';
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
  }

private:
  VendorCode _vendor;
  int _itemNo;
  int _orderNo;
  Indicator _locAppl;
  LocKey _loc1;
  LocKey _loc2;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_itemNo & ptr->_orderNo & ptr->_locAppl & ptr->_loc1 &
           ptr->_loc2;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _locAppl);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  Indicator& locAppl() { return _locAppl; }
  const Indicator& locAppl() const { return _locAppl; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }
};
}

