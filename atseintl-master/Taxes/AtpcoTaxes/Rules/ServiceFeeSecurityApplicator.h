// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "DataModel/Services/ServiceFeeSecurity.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/ServiceFeeSecurityRule.h"

namespace tax
{

class PointOfSale;
class LocService;

class ServiceFeeSecurityApplicator : public BusinessRuleApplicator
{
public:
  ServiceFeeSecurityApplicator(
      const ServiceFeeSecurityRule* rule,
      const PointOfSale& pointOfSale,
      const LocService& locService,
      const std::shared_ptr<const ServiceFeeSecurityItems>& serviceFeeSecurity)
    : BusinessRuleApplicator(rule),
      _pointOfSale(pointOfSale),
      _locService(locService),
      _vendor(rule->getVendor()),
      _serviceFeeSecurity(serviceFeeSecurity)
  {
  }

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const PointOfSale& _pointOfSale;
  const LocService& _locService;
  const type::Vendor& _vendor;
  std::shared_ptr<const ServiceFeeSecurityItems> _serviceFeeSecurity;

  bool agencyMatches(type::TravelAgencyIndicator tai, const type::PseudoCityCode& pcc) const;
  bool carrierMatches(const type::CarrierGdsCode& ruleCode,
                      const type::CarrierGdsCode& vendorCrsCode,
                      const type::CarrierCode& carrierCode) const;
  bool dutyFunctionMatches(const type::DutyFunctionCode& ruleDutyFunction,
                           const type::DutyFunctionCode& agentDuty,
                           const type::DutyFunctionCode& agentFunction) const;
  bool locationMatches(const LocZone& ruleLocation, const type::AirportOrCityCode& agentLocation) const;
  bool codeMatches(const type::CodeType& type, const std::string& code, const PointOfSale& pos) const;
  bool matches(const ServiceFeeSecurityItem& item) const;
  bool isCrsCodeMatches(const type::CarrierGdsCode& ruleCode, const type::CarrierGdsCode& vendorCrsCode) const;
  bool isAirlineDepartmentMatches(const std::string& code, const PointOfSale& pos) const;

  friend class ServiceFeeSecurityApplicatorTest;
};

} // namespace tax

