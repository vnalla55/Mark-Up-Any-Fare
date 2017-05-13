//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/CabinType.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class FreeBaggageInfo
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  int& units() { return _units; }
  const int& units() const { return _units; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& globalAppl() { return _globalAppl; }
  const Indicator& globalAppl() const { return _globalAppl; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  EquipmentType& equipmentCode() { return _equipmentCode; }
  const EquipmentType& equipmentCode() const { return _equipmentCode; }

  Indicator& measurementType() { return _measurementType; }
  const Indicator& measurementType() const { return _measurementType; }

  PaxTypeCode& psgType() { return _psgType; }
  const PaxTypeCode& psgType() const { return _psgType; }

  Indicator& psgTypeInfant() { return _psgTypeInfant; }
  const Indicator& psgTypeInfant() const { return _psgTypeInfant; }

  Indicator& psgTypeChild() { return _psgTypeChild; }
  const Indicator& psgTypeChild() const { return _psgTypeChild; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  TariffNumber& tariffNumber() { return _tariffNumber; }
  const TariffNumber& tariffNumber() const { return _tariffNumber; }

  RuleNumber& ruleNumber() { return _ruleNumber; }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  FareClassCode& fareClassCode() { return _fareClassCode; }
  const FareClassCode& fareClassCode() const { return _fareClassCode; }

  bool operator==(const FreeBaggageInfo& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_expireDate == rhs._expireDate) &&
            (_seqNo == rhs._seqNo) && (_units == rhs._units) &&
            (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_globalAppl == rhs._globalAppl) &&
            (_globalDir == rhs._globalDir) && (_equipmentCode == rhs._equipmentCode) &&
            (_measurementType == rhs._measurementType) && (_psgType == rhs._psgType) &&
            (_psgTypeInfant == rhs._psgTypeInfant) && (_psgTypeChild == rhs._psgTypeChild) &&
            (_bookingCode == rhs._bookingCode) && (_fareType == rhs._fareType) &&
            (_cabin == rhs._cabin) && (_vendorCode == rhs._vendorCode) &&
            (_tariffNumber == rhs._tariffNumber) && (_ruleNumber == rhs._ruleNumber) &&
            (_fareClassCode == rhs._fareClassCode));
  }

  static void dummyData(FreeBaggageInfo& obj)
  {
    obj._carrier = "ABC";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._seqNo = 1;
    obj._units = 2;
    obj._directionality = 'D';

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._globalAppl = 'E';
    obj._globalDir = GlobalDirection::US;
    obj._equipmentCode = "FGH";
    obj._measurementType = 'I';
    obj._psgType = "JKL";
    obj._psgTypeInfant = 'M';
    obj._psgTypeChild = 'N';
    obj._bookingCode = "OP";
    obj._fareType = "aaaaaaaa";
    obj._cabin.setPremiumFirstClass();
    obj._vendorCode = "ATP";
    obj._tariffNumber = 1;
    obj._ruleNumber = 1;
    obj._fareClassCode = "Y";
  }

private:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _expireDate;
  int _seqNo = 0;
  int _units = 0;
  Indicator _directionality = ' ';
  LocKey _loc1;
  LocKey _loc2;
  Indicator _globalAppl = ' ';
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  EquipmentType _equipmentCode;
  Indicator _measurementType = ' ';
  PaxTypeCode _psgType;
  Indicator _psgTypeInfant = ' ';
  Indicator _psgTypeChild = ' ';
  BookingCode _bookingCode;
  FareType _fareType;
  CabinType _cabin;
  VendorCode _vendorCode;
  TariffNumber _tariffNumber = 0;
  RuleNumber _ruleNumber;
  FareClassCode _fareClassCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _units);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalAppl);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _equipmentCode);
    FLATTENIZE(archive, _measurementType);
    FLATTENIZE(archive, _psgType);
    FLATTENIZE(archive, _psgTypeInfant);
    FLATTENIZE(archive, _psgTypeChild);
    FLATTENIZE(archive, _bookingCode);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _cabin);
    FLATTENIZE(archive, _vendorCode);
    FLATTENIZE(archive, _tariffNumber);
    FLATTENIZE(archive, _ruleNumber);
    FLATTENIZE(archive, _fareClassCode);
  }
};
}
