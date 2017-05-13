//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class CountrySettlementPlanInfo
{
public:
  CountrySettlementPlanInfo();
  CountrySettlementPlanInfo(const NationCode& countryCode, const SettlementPlanType& type);
  virtual ~CountrySettlementPlanInfo() {}

  const NationCode& getCountryCode() const { return _nation; }
  void setCountryCode(const NationCode& countryCode) { _nation = countryCode; }

  const SettlementPlanType& getSettlementPlanTypeCode() const { return _settlementPlan; }
  void setSettlementPlanTypeCode(const SettlementPlanType& type) { _settlementPlan = type; }

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
  static void dummyData(CountrySettlementPlanInfo&);

  virtual bool operator==(const CountrySettlementPlanInfo&) const;

protected:
  NationCode _nation;
  SettlementPlanType _settlementPlan;
  Indicator _preferredTicketingMethod;
  Indicator _requiredTicketingMethod;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;

private:
  CountrySettlementPlanInfo(const CountrySettlementPlanInfo&);
  CountrySettlementPlanInfo& operator=(const CountrySettlementPlanInfo&);
};
}

