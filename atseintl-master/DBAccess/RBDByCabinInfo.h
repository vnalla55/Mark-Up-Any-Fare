#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TSEDateInterval.h"

#include <sstream>

namespace tse
{

typedef std::map<BookingCode, char> BookingCodeCabinMap;

class RBDByCabinInfo
{
 public:
  RBDByCabinInfo()
    : _sequenceNo(0), _flightNo1(0), _flightNo2(0)
  {
  }

  const VendorCode& vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }

  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  uint64_t sequenceNo() const { return _sequenceNo; }
  uint64_t& sequenceNo() { return _sequenceNo; }

  const TSEDateInterval& effInterval() const { return _effInterval; }
  TSEDateInterval& effInterval() { return _effInterval; }

  DateTime createDate() const { return _effInterval.createDate(); }
  DateTime& createDate() { return _effInterval.createDate(); }

  DateTime effDate() const { return _effInterval.effDate(); }
  DateTime& effDate() { return _effInterval.effDate(); }

  DateTime expireDate() const { return _effInterval.expireDate(); }
  DateTime& expireDate() { return _effInterval.expireDate(); }

  DateTime discDate() const { return _effInterval.discDate(); }
  DateTime& discDate() { return _effInterval.discDate(); }

  const Code<2>& globalDir() const { return _globalDir; }
  Code<2>& globalDir() { return _globalDir; }

  const LocKey& locKey1() const { return _locKey1; }
  LocKey& locKey1() { return _locKey1; }

  const LocKey& locKey2() const { return _locKey2; }
  LocKey& locKey2() { return _locKey2; }

  DateTime firstTicketDate() const { return _firstTicketDate; }
  DateTime& firstTicketDate() { return _firstTicketDate; }

  DateTime lastTicketDate() const { return _lastTicketDate; }
  DateTime& lastTicketDate() { return _lastTicketDate; }

  FlightNumber flightNo1() const { return _flightNo1; }
  FlightNumber& flightNo1() { return _flightNo1; }

  FlightNumber flightNo2() const { return _flightNo2; }
  FlightNumber& flightNo2() { return _flightNo2; }

  const EquipmentType& equipmentType() const { return _equipType; }
  EquipmentType& equipmentType() { return _equipType; }

  const BookingCodeCabinMap& bookingCodeCabinMap() const { return _bookingCodeCabinMap; }
  BookingCodeCabinMap& bookingCodeCabinMap() { return _bookingCodeCabinMap; }

  std::string writeObject() const
  {
    std::ostringstream oss;
    oss << _vendor << ',' << _carrier << ',' << _sequenceNo << ','
        << createDate() << ',' << effDate() << ',' << expireDate() << ',' << discDate() << ','
        << _globalDir << ',' << _locKey1 << ',' << _locKey2 << ',' << _firstTicketDate
        << ',' << _lastTicketDate << ',' << _flightNo1 << ',' << _flightNo2 << ',' << _equipType << '\n';
    for (const auto& elem : _bookingCodeCabinMap)
    {
      oss << elem.first << ':' << elem.second << '\n';
    }
    oss << "****\n";
    return oss.str();
  }

  bool operator ==(const RBDByCabinInfo& rhs) const
  {
    return _vendor == rhs._vendor
           && _carrier == rhs._carrier
           && _sequenceNo == rhs._sequenceNo
           && _effInterval == rhs._effInterval
           && _globalDir == rhs._globalDir
           && _locKey1 == rhs._locKey1
           && _locKey2 == rhs._locKey2
           && _firstTicketDate == rhs._firstTicketDate
           && _lastTicketDate == rhs._lastTicketDate
           && _flightNo1 == rhs._flightNo1
           && _flightNo2 == rhs._flightNo2
           && _equipType == rhs._equipType
           && _bookingCodeCabinMap == rhs._bookingCodeCabinMap;
  }

  bool operator !=(const RBDByCabinInfo& rhs) const
  {
    return !operator == (rhs);
  }

  static void dummyData(RBDByCabinInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._sequenceNo = 1111111111;
    TSEDateInterval::dummyData(obj._effInterval);
    obj._globalDir = "AI";
    LocKey::dummyData(obj._locKey1);
    LocKey::dummyData(obj._locKey2);
    DateTime currentDateTime(::time(nullptr));
    obj._firstTicketDate = currentDateTime;
    obj._lastTicketDate = currentDateTime;
    obj._flightNo1 = 2222;
    obj._flightNo2 = 3333;
    obj._equipType = "FGH";
    obj._bookingCodeCabinMap["FN"] = 'C';
    obj._bookingCodeCabinMap["W"] = 'D';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _sequenceNo);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _locKey1);
    FLATTENIZE(archive, _locKey2);
    FLATTENIZE(archive, _firstTicketDate);
    FLATTENIZE(archive, _lastTicketDate);
    FLATTENIZE(archive, _flightNo1);
    FLATTENIZE(archive, _flightNo2);
    FLATTENIZE(archive, _equipType);
    FLATTENIZE(archive, _bookingCodeCabinMap);
  }

 private:
  VendorCode _vendor;
  CarrierCode _carrier;
  uint64_t _sequenceNo;
  TSEDateInterval _effInterval;
  Code<2> _globalDir;
  LocKey _locKey1;
  LocKey _locKey2;
  DateTime _firstTicketDate;
  DateTime _lastTicketDate;
  FlightNumber _flightNo1;
  FlightNumber _flightNo2;
  EquipmentType _equipType;
  BookingCodeCabinMap _bookingCodeCabinMap;
};

}// tse
