//----------------------------------------------------------------------------
//
//  File:           SurfaceSectorExemptionInfo.h
//  Created:        12 Jan 2009
//  Author:         Marcin Augustyniak
//  Description:
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//

//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class SurfaceSectorExemptionInfo
{

private:
  // Validating carrier for the selected record.
  CarrierCode _validatingCarrier;

  // System generated Sequence Number for the selected record.
  int _seqNo;

  // version date
  DateTime _versionDate;

  // Record creation date.
  DateTime _createDate;

  // Restricts the application of the Fare Display record to a
  // specific partition or user. Data is added manually by APO,
  // (no vendor transmitted data is received), and is applied as
  // match criteria.
  Indicator _userApplType;

  // User application
  std::string _userAppl;

  // Date record is effective for sale.
  DateTime _effDate;

  // Date record is no longer valid for pricing.
  DateTime _discDate;

  // Date record is expired.
  DateTime _expireDate;

  // When equal to 'Y', Point of Sale geographic locations other than
  // those defined in the following Loc field will ignore the TPM
  // Surface Restriction.
  Indicator _posLocException;

  // Identifies the type of location data to follow in Point of
  // Sale Loc field.
  LocTypeCode _posLocType;

  // Identifies the Point of Sale specific geographic location
  // applicable to the TPM Surface record.
  LocCode _posLoc;

  // When equal to 'Y', geographic locations other than those defined
  // in the following Loc fields will ignore the TPM Surface Restriction.
  Indicator _locException;

  // Identifies the type of location data to follow in Loc 1.
  LocTypeCode _loc1Type;

  // Identifies the specific geographic location applicable to
  // the TPM Surface record.
  LocCode _loc1;

  // Identifies the type of location data to follow in Loc 2.
  LocTypeCode _loc2Type;

  // Identifies the specific geographic location applicable to
  // the TPM Surface record.
  LocCode _loc2;

  // When equals to 'Y', participating carriers other than those defined
  // in the following marketing carriers list will ignore the TPM
  // Surface Restriction.
  Indicator _exceptMarketingCarriers;

  // Specifies the marketing carrier/s on journey where TPM surface may
  // or may not be restricted.
  std::set<CarrierCode> _marketingCarriers;

  // When equals to 'Y', operating carriers other than those defined in the
  // following marketing carriers list will ignore the TPM Surface
  // Restriction.
  Indicator _exceptOperatingCarriers;

  // Specifies the operating carrier/s on journey where TPM surface may or
  // may not be restricted.
  std::set<CarrierCode> _operatingCarriers;

  // When equals to 'Y', fare passenger type/s other than those defined
  // in the following fare type/generic list will ignore the TPM Surface
  // Restriction.
  Indicator _exceptPassengersTypes;

  // Child table to input one or more passenger type codes.
  std::set<PaxTypeCode> _paxTypes;

  // CRS
  std::string _crs;

public:
  SurfaceSectorExemptionInfo()
    : _seqNo(0),
      _userApplType(' '),
      _posLocException(' '),
      _posLocType(' '),
      _locException(' '),
      _loc1Type(' '),
      _loc2Type(' '),
      _exceptMarketingCarriers(' '),
      _exceptOperatingCarriers(' '),
      _exceptPassengersTypes(' ')
  {
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _validatingCarrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _posLocException);
    FLATTENIZE(archive, _posLocType);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _locException);
    FLATTENIZE(archive, _loc1Type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2Type);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _exceptMarketingCarriers);
    FLATTENIZE(archive, _exceptOperatingCarriers);
    FLATTENIZE(archive, _exceptPassengersTypes);
    FLATTENIZE(archive, _marketingCarriers);
    FLATTENIZE(archive, _operatingCarriers);
    FLATTENIZE(archive, _paxTypes);
    FLATTENIZE(archive, _crs);
  }

  bool operator==(const SurfaceSectorExemptionInfo& rhs) const
  {
    return ((_validatingCarrier == rhs._validatingCarrier) && (_expireDate == rhs._expireDate) &&
            (_versionDate == rhs._versionDate) && (_createDate == rhs._createDate) &&
            (_seqNo == rhs._seqNo) && (_userApplType == rhs._userApplType) &&
            (_userAppl == rhs._userAppl) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_posLocException == rhs._posLocException) &&
            (_posLocType == rhs._posLocType) && (_posLoc == rhs._posLoc) &&
            (_locException == rhs._locException) && (_loc1Type == rhs._loc1Type) &&
            (_loc1 == rhs._loc1) && (_loc2Type == rhs._loc2Type) && (_loc2 == rhs._loc2) &&
            (_exceptMarketingCarriers == rhs._exceptMarketingCarriers) &&
            (_exceptOperatingCarriers == rhs._exceptOperatingCarriers) &&
            (_exceptPassengersTypes == rhs._exceptPassengersTypes) &&
            (_marketingCarriers == rhs._marketingCarriers) &&
            (_operatingCarriers == rhs._operatingCarriers) && (_paxTypes == rhs._paxTypes) &&
            (_crs == rhs._crs));
  }

  static void dummyData(SurfaceSectorExemptionInfo& obj)
  {
    obj._validatingCarrier = "ABC";
    obj._seqNo = 1;
    obj._versionDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._userApplType = 'D';
    obj._userAppl = "aaaaaaaa";
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._posLocException = 'E';
    obj._posLocType = 'F';
    obj._posLoc = "GHIJK";
    obj._locException = 'L';
    obj._loc1Type = 'M';
    obj._loc1 = "NOPQR";
    obj._loc2Type = 'S';
    obj._loc2 = "TUVWX";
    obj._exceptMarketingCarriers = 'Y';
    obj._exceptOperatingCarriers = 'Z';
    obj._exceptPassengersTypes = 'a';

    obj._marketingCarriers.insert("bcd");
    obj._marketingCarriers.insert("efg");

    obj._operatingCarriers.insert("hij");
    obj._operatingCarriers.insert("klm");

    obj._paxTypes.insert("nop");
    obj._paxTypes.insert("qrs");

    obj._crs = "bbbbbbbb";
  }

  CarrierCode& validatingCarrier() { return _validatingCarrier; }

  const CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  int& seqNo() { return _seqNo; }

  const int seqNo() const { return _seqNo; }

  DateTime& versionDate() { return _versionDate; }

  const DateTime& versionDate() const { return _versionDate; }

  DateTime& createDate() { return _createDate; }

  const DateTime& createDate() const { return _createDate; }

  Indicator& userApplType() { return _userApplType; }

  const Indicator userApplType() const { return _userApplType; }

  std::string& userAppl() { return _userAppl; }

  const std::string& userAppl() const { return _userAppl; }

  DateTime& effDate() { return _effDate; }

  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }

  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }

  const DateTime& expireDate() const { return _expireDate; }

  Indicator& posLocException() { return _posLocException; }

  const Indicator posLocException() const { return _posLocException; }

  LocTypeCode& posLocType() { return _posLocType; }

  const LocTypeCode& posLocType() const { return _posLocType; }

  LocCode& posLoc() { return _posLoc; }

  const LocCode& posLoc() const { return _posLoc; }

  Indicator& locException() { return _locException; }

  const Indicator locException() const { return _locException; }

  LocTypeCode& loc1Type() { return _loc1Type; }

  const LocTypeCode loc1Type() const { return _loc1Type; }

  LocCode& loc1() { return _loc1; }

  const LocCode& loc1() const { return _loc1; }

  LocTypeCode& loc2Type() { return _loc2Type; }

  const LocTypeCode loc2Type() const { return _loc2Type; }

  LocCode& loc2() { return _loc2; }

  const LocCode& loc2() const { return _loc2; }

  Indicator& exceptMarketingCarriers() { return _exceptMarketingCarriers; }

  const Indicator exceptMarketingCarriers() const { return _exceptMarketingCarriers; }

  std::set<CarrierCode>& marketingCarriers() { return _marketingCarriers; }

  const std::set<CarrierCode>& marketingCarriers() const { return _marketingCarriers; }

  Indicator& exceptOperatingCarriers() { return _exceptOperatingCarriers; }

  const Indicator exceptOperatingCarriers() const { return _exceptOperatingCarriers; }

  std::set<CarrierCode>& operatingCarriers() { return _operatingCarriers; }

  const std::set<CarrierCode>& operatingCarriers() const { return _operatingCarriers; }

  Indicator& exceptPassengersTypes() { return _exceptPassengersTypes; }

  const Indicator exceptPassengersTypes() const { return _exceptPassengersTypes; }

  std::set<PaxTypeCode>& paxTypes() { return _paxTypes; }

  const std::set<PaxTypeCode>& paxTypes() const { return _paxTypes; }

  std::string& crs() { return _crs; }

  const std::string& crs() const { return _crs; }
};

} // namespace tse

