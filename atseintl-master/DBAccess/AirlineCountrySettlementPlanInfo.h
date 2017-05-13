//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class AirlineCountrySettlementPlanInfo
{
public:
  AirlineCountrySettlementPlanInfo();
  virtual ~AirlineCountrySettlementPlanInfo() {}

  const NationCode& getCountryCode() const { return _nation; }
  void setCountryCode(const NationCode& country) { _nation = country; }

  const CrsCode& getGds() const { return _gds; }
  void setGds(const CrsCode& gds) { _gds = gds; }

  const CarrierCode& getAirline() const { return _airline; }
  void setAirline(const CarrierCode& airline) { _airline = airline; }

  const SettlementPlanType& getSettlementPlanType() const { return _settlementPlan; }
  void setSettlementPlanType(const SettlementPlanType& spType) { _settlementPlan = spType; }

  Indicator getPreferredTicketingMethod() const { return _preferredTicketingMethod; }
  void setPreferredTicketingMethod(Indicator ticketingMethod)
  {
    _preferredTicketingMethod = ticketingMethod;
  }

  Indicator getRequiredTicketingMethod() const { return _requiredTicketingMethod; }
  void setRequiredTicketingMethod(Indicator ticketingMethod)
  {
    _requiredTicketingMethod = ticketingMethod;
  }

  const DateTime& effDate() const { return _effDate; }
  void setEffDate(const DateTime& dt) { _effDate = dt; }

  const DateTime& discDate() const { return _discDate; }
  void setDiscDate(const DateTime& dt) { _discDate = dt; }

  const DateTime& createDate() const { return _createDate; }
  void setCreateDate(const DateTime& dt) { _createDate = dt; }

  const DateTime& expireDate() const { return _expireDate; }
  void setExpireDate(const DateTime& dt) { _expireDate = dt; }

  virtual void flattenize(Flattenizable::Archive&);
  static void dummyData(AirlineCountrySettlementPlanInfo&);

  virtual bool operator==(const AirlineCountrySettlementPlanInfo&) const;

protected:
  NationCode _nation;
  CrsCode _gds;
  CarrierCode _airline;
  SettlementPlanType _settlementPlan;
  Indicator _preferredTicketingMethod;
  Indicator _requiredTicketingMethod;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;

private:
  AirlineCountrySettlementPlanInfo(const AirlineCountrySettlementPlanInfo&);
  AirlineCountrySettlementPlanInfo& operator=(const AirlineCountrySettlementPlanInfo&);
};
}

