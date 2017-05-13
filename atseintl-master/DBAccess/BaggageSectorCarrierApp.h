#pragma once
//----------------------------------------------------------------------------
//
// Copyright Sabre 2010
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class BaggageSectorCarrierApp
{
public:
  BaggageSectorCarrierApp() : _seqNo(0), _inclExclInd(' '), _flt1(0), _flt2(0) {}
  // Primary Key Fields

  CarrierCode& marketingCarrier() { return _marketingCarrier; }
  const CarrierCode& marketingCarrier() const { return _marketingCarrier; }

  uint32_t& seqNo() { return _seqNo; }
  const uint32_t& seqNo() const { return _seqNo; }

  DateTime& createDate()
  {
    return _createDate;
  };
  const DateTime& createDate() const
  {
    return _createDate;
  };

  // Non-Key Data
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CarrierCode& operatingCarrier() { return _operatingCarrier; }
  const CarrierCode& operatingCarrier() const { return _operatingCarrier; }

  Indicator& inclExclInd() { return _inclExclInd; }
  const Indicator& inclExclInd() const { return _inclExclInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  FlightNumber& flt1() { return _flt1; }
  const FlightNumber& flt1() const { return _flt1; }

  FlightNumber& flt2() { return _flt2; }
  const FlightNumber& flt2() const { return _flt2; }

  bool operator==(const BaggageSectorCarrierApp& rhs) const;

  static void dummyData(BaggageSectorCarrierApp& obj);

  void flattenize(Flattenizable::Archive& archive);

private:
  CarrierCode _marketingCarrier;
  uint32_t _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  CarrierCode _operatingCarrier;
  Indicator _inclExclInd;
  LocKey _loc1;
  LocKey _loc2;
  FlightNumber _flt1;
  FlightNumber _flt2;

};

} // namespace tse

