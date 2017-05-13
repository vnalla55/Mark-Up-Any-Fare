#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CommissionTravelDatesSegInfo
{
 public:
  CommissionTravelDatesSegInfo()
    : _itemNo(0)
    , _orderNo(0)
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

  DateTime& firstTravelDate() { return _firstTravelDate; }
  DateTime firstTravelDate() const { return _firstTravelDate; }

  DateTime& endTravelDate() { return _endTravelDate; }
  DateTime endTravelDate() const { return _endTravelDate; }

  bool operator==(const CommissionTravelDatesSegInfo& rhs) const
  {
    return _vendor == rhs._vendor
           && _itemNo == rhs._itemNo
           && _createDate == rhs._createDate
           && _orderNo == rhs._orderNo
           && _firstTravelDate == rhs._firstTravelDate
           && _endTravelDate == rhs._endTravelDate;
  }

  static void dummyData(CommissionTravelDatesSegInfo& obj)
  {
    obj._vendor = "COS";
    obj._itemNo = 1234567;
    obj._createDate = std::time(0);
    obj._orderNo = 891234567;
    obj._firstTravelDate = std::time(0);
    obj._endTravelDate = std::time(0);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _firstTravelDate);
    FLATTENIZE(archive, _endTravelDate);
  }

 private:
  VendorCode _vendor;
  unsigned long _itemNo;
  DateTime _createDate;
  unsigned long _orderNo;
  DateTime _firstTravelDate;
  DateTime _endTravelDate;
};

}// tse
