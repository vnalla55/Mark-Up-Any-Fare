#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class CommissionLocSegInfo
{
 public:

  CommissionLocSegInfo()
    : _itemNo(0)
    , _orderNo(0)
    , _inclExclInd(' ')
  {
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  unsigned long& itemNo() { return _itemNo; }
  unsigned long itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  DateTime createDate() const { return _createDate; }

  unsigned long& orderNo() { return _orderNo; }
  unsigned long orderNo() const { return _orderNo; }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  Indicator& inclExclInd() { return _inclExclInd; }
  Indicator inclExclInd() const { return _inclExclInd; }

  bool operator==(const CommissionLocSegInfo& rhs) const
  {
    return _vendor == rhs._vendor
           && _itemNo == rhs._itemNo
           && _createDate == rhs._createDate
           && _orderNo == rhs._orderNo
           && _loc == rhs._loc
           && _inclExclInd == rhs._inclExclInd;
  }

  static void dummyData(CommissionLocSegInfo& obj)
  {
    obj._vendor = "COS";
    obj._itemNo = 1234567;
    obj._createDate = std::time(0);
    obj._orderNo = 891234567;
    LocKey::dummyData(obj._loc);
    obj._inclExclInd = 'Y';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _inclExclInd);
  }

 private:

  VendorCode _vendor;
  unsigned long _itemNo;
  DateTime _createDate;
  unsigned long _orderNo;
  LocKey _loc;
  Indicator _inclExclInd;
};

}// tse
