//----------------------------------------------------------------------------
//     2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class Differentials
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  uint16_t& seqNo() { return _seqNo; }
  const uint16_t& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  LocKey& viaLoc() { return _viaLoc; }
  const LocKey& viaLoc() const { return _viaLoc; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  Indicator& flightAppl() { return _flightAppl; }
  const Indicator& flightAppl() const { return _flightAppl; }

  Indicator& calculationInd() { return _calculationInd; }
  const Indicator& calculationInd() const { return _calculationInd; }

  Indicator& hipExemptInd() { return _hipExemptInd; }
  const Indicator& hipExemptInd() const { return _hipExemptInd; }

  CarrierCode& intermedCarrier() { return _intermedCarrier; }
  const CarrierCode& intermedCarrier() const { return _intermedCarrier; }

  LocKey& intermedLoc1a() { return _intermedLoc1a; }
  const LocKey& intermedLoc1a() const { return _intermedLoc1a; }

  LocKey& intermedLoc2a() { return _intermedLoc2a; }
  const LocKey& intermedLoc2a() const { return _intermedLoc2a; }

  LocKey& intermedLoc1b() { return _intermedLoc1b; }
  const LocKey& intermedLoc1b() const { return _intermedLoc1b; }

  LocKey& intermedLoc2b() { return _intermedLoc2b; }
  const LocKey& intermedLoc2b() const { return _intermedLoc2b; }

  FareType& intermedFareType() { return _intermedFareType; }
  const FareType& intermedFareType() const { return _intermedFareType; }

  BookingCode& intermedBookingCode() { return _intermedBookingCode; }
  const BookingCode& intermedBookingCode() const { return _intermedBookingCode; }

  bool operator==(const Differentials& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_viaLoc == rhs._viaLoc) && (_globalDir == rhs._globalDir) &&
            (_fareClass == rhs._fareClass) && (_fareType == rhs._fareType) &&
            (_bookingCode == rhs._bookingCode) && (_flightAppl == rhs._flightAppl) &&
            (_calculationInd == rhs._calculationInd) && (_hipExemptInd == rhs._hipExemptInd) &&
            (_intermedCarrier == rhs._intermedCarrier) && (_intermedLoc1a == rhs._intermedLoc1a) &&
            (_intermedLoc2a == rhs._intermedLoc2a) && (_intermedLoc1b == rhs._intermedLoc1b) &&
            (_intermedLoc2b == rhs._intermedLoc2b) &&
            (_intermedFareType == rhs._intermedFareType) &&
            (_intermedBookingCode == rhs._intermedBookingCode));
  }

  static void dummyData(Differentials& obj)
  {
    obj._carrier = "ABC";
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._directionality = 'D';
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
    LocKey::dummyData(obj._viaLoc);
    obj._globalDir = GlobalDirection::US;
    obj._fareClass = "aaaaaaaa";
    obj._fareType = "bbbbbbbb";
    obj._bookingCode = "EF";
    obj._flightAppl = 'G';
    obj._calculationInd = 'H';
    obj._hipExemptInd = 'I';
    obj._intermedCarrier = "JKL";
    LocKey::dummyData(obj._intermedLoc1a);
    LocKey::dummyData(obj._intermedLoc2a);
    LocKey::dummyData(obj._intermedLoc1b);
    LocKey::dummyData(obj._intermedLoc2b);
    obj._intermedFareType = "cccccccc";
    obj._intermedBookingCode = "MN";
  }

private:
  CarrierCode _carrier;
  uint16_t _seqNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _directionality = ' ';
  LocKey _loc1;
  LocKey _loc2;
  LocKey _viaLoc;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  FareClassCode _fareClass;
  FareType _fareType;
  BookingCode _bookingCode;
  Indicator _flightAppl = ' ';
  Indicator _calculationInd = ' ';
  Indicator _hipExemptInd = ' ';
  CarrierCode _intermedCarrier;
  LocKey _intermedLoc1a;
  LocKey _intermedLoc2a;
  LocKey _intermedLoc1b;
  LocKey _intermedLoc2b;
  FareType _intermedFareType;
  BookingCode _intermedBookingCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _viaLoc);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _bookingCode);
    FLATTENIZE(archive, _flightAppl);
    FLATTENIZE(archive, _calculationInd);
    FLATTENIZE(archive, _hipExemptInd);
    FLATTENIZE(archive, _intermedCarrier);
    FLATTENIZE(archive, _intermedLoc1a);
    FLATTENIZE(archive, _intermedLoc2a);
    FLATTENIZE(archive, _intermedLoc1b);
    FLATTENIZE(archive, _intermedLoc2b);
    FLATTENIZE(archive, _intermedFareType);
    FLATTENIZE(archive, _intermedBookingCode);
  }
};
}
