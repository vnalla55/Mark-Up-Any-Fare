//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
class OptionalServicesConcur
{
public:
  OptionalServicesConcur() : _seqNo(0), _mkgOperFareOwner(' '), _concur(' ') {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  uint32_t& seqNo() { return _seqNo; }
  const uint32_t& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  ServiceTypeCode& serviceTypeCode() { return _serviceTypeCode; }
  const ServiceTypeCode& serviceTypeCode() const { return _serviceTypeCode; }

  ServiceSubTypeCode& serviceSubTypeCode() { return _serviceSubTypeCode; }
  const ServiceSubTypeCode& serviceSubTypeCode() const { return _serviceSubTypeCode; }

  ServiceGroup& serviceGroup() { return _serviceGroup; }
  const ServiceGroup& serviceGroup() const { return _serviceGroup; }

  ServiceGroup& serviceSubGroup() { return _serviceSubGroup; }
  const ServiceGroup& serviceSubGroup() const { return _serviceSubGroup; }

  CarrierCode& accessedCarrier() { return _accessedCarrier; }
  const CarrierCode& accessedCarrier() const { return _accessedCarrier; }

  Indicator& mkgOperFareOwner() { return _mkgOperFareOwner; }
  const Indicator& mkgOperFareOwner() const { return _mkgOperFareOwner; }

  Indicator& concur() { return _concur; }
  const Indicator& concur() const { return _concur; }

  bool operator==(const OptionalServicesConcur& second) const
  {
    return (_vendor == second._vendor) && (_carrier == second._carrier) &&
           (_seqNo == second._seqNo) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_serviceTypeCode == second._serviceTypeCode) &&
           (_serviceSubTypeCode == second._serviceSubTypeCode) &&
           (_serviceGroup == second._serviceGroup) &&
           (_serviceSubGroup == second._serviceSubGroup) &&
           (_accessedCarrier == second._accessedCarrier) &&
           (_mkgOperFareOwner == second._mkgOperFareOwner) && (_concur == second._concur);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _serviceTypeCode);
    FLATTENIZE(archive, _serviceSubTypeCode);
    FLATTENIZE(archive, _serviceGroup);
    FLATTENIZE(archive, _serviceSubGroup);
    FLATTENIZE(archive, _accessedCarrier);
    FLATTENIZE(archive, _mkgOperFareOwner);
    FLATTENIZE(archive, _concur);
  }

  static void dummyData(OptionalServicesConcur& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EF";
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._serviceTypeCode = "GH";
    obj._serviceSubTypeCode = "IJK";
    obj._serviceGroup = "LMN";
    obj._serviceSubGroup = "OPQ";
    obj._accessedCarrier = "RS";
    obj._mkgOperFareOwner = 'T';
    obj._concur = 'U';
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  uint32_t _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  ServiceTypeCode _serviceTypeCode;
  ServiceSubTypeCode _serviceSubTypeCode;
  ServiceGroup _serviceGroup;
  ServiceGroup _serviceSubGroup;
  CarrierCode _accessedCarrier;
  Indicator _mkgOperFareOwner;
  Indicator _concur;

  friend class SerializationTestBase;
};
}
