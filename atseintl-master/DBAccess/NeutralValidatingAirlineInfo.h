//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class NeutralValidatingAirlineInfo
{
public:
  NeutralValidatingAirlineInfo();
  virtual ~NeutralValidatingAirlineInfo() {}

  const NationCode& getCountryCode() const { return _countryCode; }
  void setCountryCode(const NationCode& country) { _countryCode = country; }

  const CrsCode& getGds() const { return _gds; }
  void setGds(const CrsCode& gds) { _gds = gds; }

  const CarrierCode& getAirline() const { return _airline; }
  void setAirline(const CarrierCode& carrier) { _airline = carrier; }

  const SettlementPlanType& getSettlementPlanType() const { return _settlementPlanType; }
  void setSettlementPlanType(const SettlementPlanType& spType) { _settlementPlanType = spType; }

  const DateTime& effDate() const { return _effDate; }
  void setEffDate(const DateTime& dt) { _effDate = dt; }

  const DateTime& discDate() const { return _discDate; }
  void setDiscDate(const DateTime& dt) { _discDate = dt; }

  const DateTime& createDate() const { return _createDate; }
  void setCreateDate(const DateTime& dt) { _createDate = dt; }

  const DateTime& expireDate() const { return _expireDate; }
  void setExpireDate(const DateTime& dt) { _expireDate = dt; }

  virtual void flattenize(Flattenizable::Archive&);
  static void dummyData(NeutralValidatingAirlineInfo&);

  virtual bool operator==(const NeutralValidatingAirlineInfo&) const;

protected:
  NationCode _countryCode;
  CrsCode _gds;
  CarrierCode _airline;
  SettlementPlanType _settlementPlanType;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;

private:
  NeutralValidatingAirlineInfo(const NeutralValidatingAirlineInfo&);
  NeutralValidatingAirlineInfo& operator=(const NeutralValidatingAirlineInfo&);
};
}

