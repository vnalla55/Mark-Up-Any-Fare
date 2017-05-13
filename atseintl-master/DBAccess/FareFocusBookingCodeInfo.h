#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusBookingCodeInfo
{
 public:

  FareFocusBookingCodeInfo()
    : _bookingCodeItemNo(0)
  {
  }

  uint64_t& bookingCodeItemNo() { return _bookingCodeItemNo; }
  uint64_t bookingCodeItemNo() const { return _bookingCodeItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<BookingCode>& bookingCode() { return _bookingCode; }
  const std::vector<BookingCode>& bookingCode() const { return _bookingCode; }

  bool operator==(const FareFocusBookingCodeInfo& rhs) const
  {
    return _bookingCodeItemNo == rhs._bookingCodeItemNo
           && _createDate == rhs._createDate
           && _expireDate == rhs._expireDate
           && _bookingCode == rhs._bookingCode;
  }

  static void dummyData(FareFocusBookingCodeInfo& obj)
  {
    obj._bookingCodeItemNo = 111111;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._bookingCode.push_back("YY");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _bookingCodeItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _bookingCode);
  }

 private:
  uint64_t _bookingCodeItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<BookingCode> _bookingCode;
};

}// tse

