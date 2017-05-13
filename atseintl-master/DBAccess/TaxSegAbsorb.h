//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TaxSegAbsorb
{
public:
  TaxSegAbsorb()
    : _seqNo(0),
      _fareTariffNo(0),
      _noSeg1(0),
      _noSeg2(0),
      _absorptionInd(' '),
      _owRt(' '),
      _flt1(0),
      _flt2(0),
      _flt3(0),
      _flt4(0),
      _inhibit(' ')
  {
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  TariffNumber& fareTariffNo() { return _fareTariffNo; }
  const TariffNumber& fareTariffNo() const { return _fareTariffNo; }

  int& noSeg1() { return _noSeg1; }
  const int& noSeg1() const { return _noSeg1; }

  int& noSeg2() { return _noSeg2; }
  const int& noSeg2() const { return _noSeg2; }

  Indicator& absorptionInd() { return _absorptionInd; }
  const Indicator& absorptionInd() const { return _absorptionInd; }

  Indicator& owRt() { return _owRt; }
  const Indicator& owRt() const { return _owRt; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  LocKey& betwAndViaLoc1() { return _betwAndViaLoc1; }
  const LocKey& betwAndViaLoc1() const { return _betwAndViaLoc1; }

  LocKey& betwAndViaLoc2() { return _betwAndViaLoc2; }
  const LocKey& betwAndViaLoc2() const { return _betwAndViaLoc2; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  RoutingNumber& routing1() { return _routing1; }
  const RoutingNumber& routing1() const { return _routing1; }

  RoutingNumber& routing2() { return _routing2; }
  const RoutingNumber& routing2() const { return _routing2; }

  RuleNumber& ruleNo() { return _ruleNo; }
  const RuleNumber& ruleNo() const { return _ruleNo; }

  FlightNumber& flt1() { return _flt1; }
  const FlightNumber& flt1() const { return _flt1; }

  FlightNumber& flt2() { return _flt2; }
  const FlightNumber& flt2() const { return _flt2; }

  FlightNumber& flt3() { return _flt3; }
  const FlightNumber& flt3() const { return _flt3; }

  FlightNumber& flt4() { return _flt4; }
  const FlightNumber& flt4() const { return _flt4; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const TaxSegAbsorb& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_seqNo == rhs._seqNo) && (_discDate == rhs._discDate) &&
            (_expireDate == rhs._expireDate) && (_fareTariffNo == rhs._fareTariffNo) &&
            (_noSeg1 == rhs._noSeg1) && (_noSeg2 == rhs._noSeg2) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_absorptionInd == rhs._absorptionInd) &&
            (_betwAndViaLoc1 == rhs._betwAndViaLoc1) && (_betwAndViaLoc2 == rhs._betwAndViaLoc2) &&
            (_owRt == rhs._owRt) && (_fareClass == rhs._fareClass) &&
            (_routing1 == rhs._routing1) && (_routing2 == rhs._routing2) &&
            (_ruleNo == rhs._ruleNo) && (_flt1 == rhs._flt1) && (_flt2 == rhs._flt2) &&
            (_flt3 == rhs._flt3) && (_flt4 == rhs._flt4) && (_vendor == rhs._vendor) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(TaxSegAbsorb& obj)
  {
    obj._carrier = "ABC";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._seqNo = 1;
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._fareTariffNo = 2;
    obj._noSeg1 = 3;
    obj._noSeg2 = 4;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._absorptionInd = 'D';

    LocKey::dummyData(obj._betwAndViaLoc1);
    LocKey::dummyData(obj._betwAndViaLoc2);

    obj._owRt = 'E';
    obj._fareClass = "FGHIJKLM";
    obj._routing1 = "NOPQ";
    obj._routing2 = "RSTU";
    obj._ruleNo = "VWXY";
    obj._flt1 = 5555;
    obj._flt2 = 6666;
    obj._flt3 = 7777;
    obj._flt4 = 8888;
    obj._vendor = "Zabc";
    obj._inhibit = 'd';
  }

private:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _effDate;
  int _seqNo;
  DateTime _discDate;
  DateTime _expireDate;
  TariffNumber _fareTariffNo;
  int _noSeg1;
  int _noSeg2;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _absorptionInd;
  LocKey _betwAndViaLoc1;
  LocKey _betwAndViaLoc2;
  Indicator _owRt;
  FareClassCode _fareClass;
  RoutingNumber _routing1;
  RoutingNumber _routing2;
  RuleNumber _ruleNo;
  FlightNumber _flt1;
  FlightNumber _flt2;
  FlightNumber _flt3;
  FlightNumber _flt4;
  VendorCode _vendor;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fareTariffNo);
    FLATTENIZE(archive, _noSeg1);
    FLATTENIZE(archive, _noSeg2);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _absorptionInd);
    FLATTENIZE(archive, _betwAndViaLoc1);
    FLATTENIZE(archive, _betwAndViaLoc2);
    FLATTENIZE(archive, _owRt);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _routing1);
    FLATTENIZE(archive, _routing2);
    FLATTENIZE(archive, _ruleNo);
    FLATTENIZE(archive, _flt1);
    FLATTENIZE(archive, _flt2);
    FLATTENIZE(archive, _flt3);
    FLATTENIZE(archive, _flt4);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _inhibit);
  }
};
}
