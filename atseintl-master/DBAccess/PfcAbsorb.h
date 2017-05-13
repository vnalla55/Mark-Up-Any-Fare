//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef PFCABSORB_H
#define PFCABSORB_H

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PfcAbsorb
{
public:
  PfcAbsorb()
    : _seqNo(0),
      _geoAppl(' '),
      _absorbType(' '),
      _fareTariff(0),
      _OwRt(' '),
      _flt1(0),
      _flt2(0),
      _flt3(0),
      _flt4(0),
      _inhibit(' ')
  {
  }
  LocCode& pfcAirport() { return _pfcAirport; }
  const LocCode& pfcAirport() const { return _pfcAirport; }

  CarrierCode& localCarrier() { return _localCarrier; }
  const CarrierCode& localCarrier() const { return _localCarrier; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& geoAppl() { return _geoAppl; }
  const Indicator& geoAppl() const { return _geoAppl; }

  Indicator& absorbType() { return _absorbType; }
  const Indicator& absorbType() const { return _absorbType; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  Indicator& OwRt() { return _OwRt; }
  const Indicator& OwRt() const { return _OwRt; }

  CarrierCode& jointCarrier() { return _jointCarrier; }
  const CarrierCode& jointCarrier() const { return _jointCarrier; }

  LocCode& absorbCity1() { return _absorbCity1; }
  const LocCode& absorbCity1() const { return _absorbCity1; }

  LocCode& absorbCity2() { return _absorbCity2; }
  const LocCode& absorbCity2() const { return _absorbCity2; }

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

  bool operator==(const PfcAbsorb& rhs) const
  {
    return ((_pfcAirport == rhs._pfcAirport) && (_localCarrier == rhs._localCarrier) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_seqNo == rhs._seqNo) && (_discDate == rhs._discDate) &&
            (_geoAppl == rhs._geoAppl) && (_absorbType == rhs._absorbType) &&
            (_fareTariff == rhs._fareTariff) && (_OwRt == rhs._OwRt) &&
            (_jointCarrier == rhs._jointCarrier) && (_absorbCity1 == rhs._absorbCity1) &&
            (_absorbCity2 == rhs._absorbCity2) && (_fareClass == rhs._fareClass) &&
            (_routing1 == rhs._routing1) && (_routing2 == rhs._routing2) &&
            (_ruleNo == rhs._ruleNo) && (_flt1 == rhs._flt1) && (_flt2 == rhs._flt2) &&
            (_flt3 == rhs._flt3) && (_flt4 == rhs._flt4) && (_vendor == rhs._vendor) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(PfcAbsorb& obj)
  {
    obj._pfcAirport = "ABCDE";
    obj._localCarrier = "FGH";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._seqNo = 1;
    obj._discDate = time(nullptr);
    obj._geoAppl = 'I';
    obj._absorbType = 'J';
    obj._fareTariff = 2;
    obj._OwRt = 'K';
    obj._jointCarrier = "LMN";
    obj._absorbCity1 = "OPQRSTUV";
    obj._absorbCity2 = "WXYZabcd";
    obj._fareClass = "efghijkl";
    obj._routing1 = "mnop";
    obj._routing2 = "qrst";
    obj._ruleNo = "uvwx";
    obj._flt1 = 3333;
    obj._flt2 = 4444;
    obj._flt3 = 5555;
    obj._flt4 = 6666;
    obj._vendor = "yz01";
    obj._inhibit = '2';
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  LocCode _pfcAirport;
  CarrierCode _localCarrier;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  int _seqNo;
  DateTime _discDate;
  Indicator _geoAppl;
  Indicator _absorbType;
  TariffNumber _fareTariff;
  Indicator _OwRt;
  CarrierCode _jointCarrier;
  LocCode _absorbCity1;
  LocCode _absorbCity2;
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
    FLATTENIZE(archive, _pfcAirport);
    FLATTENIZE(archive, _localCarrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _geoAppl);
    FLATTENIZE(archive, _absorbType);
    FLATTENIZE(archive, _fareTariff);
    FLATTENIZE(archive, _OwRt);
    FLATTENIZE(archive, _jointCarrier);
    FLATTENIZE(archive, _absorbCity1);
    FLATTENIZE(archive, _absorbCity2);
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

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_pfcAirport
           & ptr->_localCarrier
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_seqNo
           & ptr->_discDate
           & ptr->_geoAppl
           & ptr->_absorbType
           & ptr->_fareTariff
           & ptr->_OwRt
           & ptr->_jointCarrier
           & ptr->_absorbCity1
           & ptr->_absorbCity2
           & ptr->_fareClass
           & ptr->_routing1
           & ptr->_routing2
           & ptr->_ruleNo
           & ptr->_flt1
           & ptr->_flt2
           & ptr->_flt3
           & ptr->_flt4
           & ptr->_vendor
           & ptr->_inhibit;
  }
};
}
#endif
