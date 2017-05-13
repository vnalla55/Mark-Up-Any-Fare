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
#include "ServiceFeeSecurityApplicator.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/LocService.h"
#include "AtpcoTaxes/DataModel/Common/CodeOps.h"
#include "Util/BranchPrediction.h"

namespace tax
{

bool
ServiceFeeSecurityApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  for(const ServiceFeeSecurityItem& item : *_serviceFeeSecurity)
  {
    if (matches(item))
    {
      return item.viewBookTktInd != type::ViewBookTktInd::ViewOnly;
    }
  }

  return false;
}

bool
ServiceFeeSecurityApplicator::matches(const ServiceFeeSecurityItem& item) const
{
  if (UNLIKELY(!agencyMatches(item.travelAgencyIndicator, _pointOfSale.agentPcc())))
    return false;
  if (!carrierMatches(item.carrierGdsCode,
                      _pointOfSale.vendorCrsCode(),
                      _pointOfSale.carrierCode()))
    return false;
  if (UNLIKELY(!dutyFunctionMatches(
          item.dutyFunctionCode, _pointOfSale.agentDuty(), _pointOfSale.agentFunction())))
    return false;
  type::AirportOrCityCode agentLocation = _pointOfSale.loc();
  if (LIKELY(!_pointOfSale.agentCity().empty()))
    agentLocation = _pointOfSale.agentCity();
  if (!locationMatches(item.location, agentLocation))
    return false;
  if (!codeMatches(item.codeType, item.code, _pointOfSale))
    return false;
  return true;
}

bool
ServiceFeeSecurityApplicator::agencyMatches(type::TravelAgencyIndicator tai,
                                            const type::PseudoCityCode& pcc) const
{
  return tai == type::TravelAgencyIndicator::Blank || !pcc.empty();
}

bool
ServiceFeeSecurityApplicator::carrierMatches(const type::CarrierGdsCode& ruleCode,
                                             const type::CarrierGdsCode& vendorCrsCode,
                                             const type::CarrierCode& carrierCode) const
{
  if (UNLIKELY(ruleCode.empty()))
    return true;

  if (ruleCode[0] == '1')
    return isCrsCodeMatches(ruleCode, vendorCrsCode);
  else
    return equal(carrierCode, ruleCode);
}

bool
ServiceFeeSecurityApplicator::isCrsCodeMatches(const type::CarrierGdsCode& ruleCode,
                                             const type::CarrierGdsCode& vendorCrsCode) const
{
  if (LIKELY(ruleCode == "1S")) // Sabre matches to...
    if (vendorCrsCode == "1B" || //   Abacus
        vendorCrsCode == "1J" || //   Axess
        vendorCrsCode == "1F") //   Infini
       return true;
  return ruleCode == vendorCrsCode;
}

bool
ServiceFeeSecurityApplicator::dutyFunctionMatches(const type::DutyFunctionCode& ruleDutyFunction,
                                                  const type::DutyFunctionCode& agentDuty,
                                                  const type::DutyFunctionCode& agentFunction) const
{
  if (LIKELY(ruleDutyFunction.empty())) // if no duty/function in the rule, match
    return true;
  if (agentDuty.empty()) // if no agent duty info, no match
    return false;
  char ruleCode = ruleDutyFunction[0];
  char agentCode = agentDuty[0];
  if (ruleCode >= 'A' && ruleCode <= 'E')
  { // rule duty codes "ABCDE" map to agent codes "$@*-/", respectively:
    ruleCode = "$@*-/"[ruleCode - 'A'];
  }
  if (ruleCode != agentCode) // validation of duty code ends here
    return false;
  if (ruleDutyFunction.size() == 1) // if no function code in the rule, stop here
    return true;
  if (agentFunction.empty()) // if no agent function info, no match
    return false;
  return ruleDutyFunction[1] == agentFunction[0];
}

bool
ServiceFeeSecurityApplicator::locationMatches(const LocZone& ruleLocation,
                                              const type::AirportOrCityCode& agentLocation) const
{
  if (ruleLocation.type() == type::LocType::Blank)
    return true;
  if (UNLIKELY(agentLocation.empty()))
    return false;
  return _locService.isInLoc(agentLocation, ruleLocation, _vendor);
}

bool
ServiceFeeSecurityApplicator::codeMatches(const type::CodeType& type,
                                          const std::string& code,
                                          const PointOfSale& pos) const
{
  if (type == type::CodeType::Blank)
    return true;
  if (type == type::CodeType::AgencyPCC)
    return equal(pos.agentPcc(), code);
  if (type == type::CodeType::AgencyNumber)
    return pos.iataNumber() == code;
  if (type == type::CodeType::Department) // not supported by SABRE
    return false;
  if (type == type::CodeType::CarrierDept)
    return isAirlineDepartmentMatches(code, pos);
  if (type == type::CodeType::ElecResServProvider)
    return pos.ersp() == code;
  if (type == type::CodeType::LNIATA) // not supported by SABRE
    return false;
  if (type == type::CodeType::AirlineSpec) // not supported by SABRE
    return false;
  return false;
}

bool
ServiceFeeSecurityApplicator::isAirlineDepartmentMatches(const std::string& code,
                                                       const PointOfSale& pos) const
{
  return pos.agentAirlineDept() == code || pos.agentOfficeDesignator() == code;
}
}
