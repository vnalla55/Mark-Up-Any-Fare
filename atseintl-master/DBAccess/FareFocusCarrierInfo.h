#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusCarrierInfo
{
 public:

  FareFocusCarrierInfo()
    : _carrierItemNo(0)
  {
  }

  uint64_t& carrierItemNo() { return _carrierItemNo; }
  uint64_t carrierItemNo() const { return _carrierItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<CarrierCode>& carrier() { return _carrier; }
  const std::vector<CarrierCode>& carrier() const { return _carrier; }

  bool operator==(const FareFocusCarrierInfo& rhs) const
  {
    return _carrierItemNo == rhs._carrierItemNo
           && _createDate == rhs._createDate
           && _expireDate == rhs._expireDate
           && _carrier == rhs._carrier;
  }

  static void dummyData(FareFocusCarrierInfo& obj)
  {
    obj._carrierItemNo = 12345;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._carrier.push_back("AA");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrierItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _carrier);
  }

 private:
  uint64_t _carrierItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<CarrierCode> _carrier;
};

}// tse


