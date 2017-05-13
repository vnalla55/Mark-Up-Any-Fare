#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class CommissionMarketSegInfo
{
 public:
  CommissionMarketSegInfo()
    : _itemNo(0)
    , _orderNo(0)
    , _bidirectional(' ')
    , _inclExclInd(' ') 
  {
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  unsigned& itemNo() { return _itemNo; }
  unsigned itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  DateTime createDate() const { return _createDate; }

  unsigned& orderNo() { return _orderNo; }
  unsigned orderNo() const { return _orderNo; }

  LocKey& origin() { return _origin; }
  const LocKey& origin() const { return _origin; }

  LocKey& destination() { return _destination; }
  const LocKey& destination() const { return _destination; }

  Indicator&  bidirectional() { return _bidirectional; }
  Indicator  bidirectional() const { return _bidirectional; }

  Indicator& inclExclInd() { return _inclExclInd; }
  Indicator inclExclInd() const { return _inclExclInd; }

  bool operator==(const CommissionMarketSegInfo& rhs) const
  {
    return _vendor == rhs._vendor
           && _itemNo == rhs._itemNo
           && _createDate == rhs._createDate
           && _orderNo == rhs._orderNo
           && _origin == rhs._origin
           && _destination == rhs._destination
           && _bidirectional == rhs._bidirectional
           && _inclExclInd == rhs._inclExclInd;
  }

  static void dummyData(CommissionMarketSegInfo& obj)
  {
    obj._vendor = "COS";
    obj._itemNo = 1234567;
    obj._createDate = std::time(0);
    obj._orderNo = 8912345;
    LocKey::dummyData(obj._origin);
    LocKey::dummyData(obj._destination);
    obj._bidirectional = 'Y';
    obj._inclExclInd = 'I';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _origin);
    FLATTENIZE(archive, _destination);
    FLATTENIZE(archive, _bidirectional);
    FLATTENIZE(archive, _inclExclInd);
  }

 private:
  VendorCode _vendor;
  unsigned _itemNo;
  DateTime _createDate;
  unsigned _orderNo;
  LocKey _origin;
  LocKey _destination;
  Indicator _bidirectional;
  Indicator _inclExclInd;
};

}// tse
