// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/CodeIO.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/TaxOnOptionalServiceApplicator.h"
#include "Rules/TaxOnOptionalServiceRule.h"

namespace tax
{

TaxOnOptionalServiceRule::TaxOnOptionalServiceRule(type::Index const& itemNo,
                                                   type::Vendor const& vendor)
  : _itemNo(itemNo), _vendor(vendor)
{
}

TaxOnOptionalServiceRule::ApplicatorType
TaxOnOptionalServiceRule::createApplicator(const type::Index& /*itinIndex*/,
                                           const Request& /*request*/,
                                           Services& services,
                                           RawPayments& /*rawPayments*/) const
{
  std::shared_ptr<ServiceBaggage const> serviceBaggage;
  bool restrictServiceBaggage = (_itemNo != 0);
  if (restrictServiceBaggage)
  {
    serviceBaggage = services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo);
  }

  return ApplicatorType(*this, serviceBaggage, services.fallbackService(), restrictServiceBaggage);
}

std::string
TaxOnOptionalServiceRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "CALCULATE TAX ON OCS AND BAGGAGE";
  if (_itemNo == 0)
  {
    return buf.str();
  }

  buf << " THAT SATISFY RESTRICTIONS\n"
      << " FROM SERVICE BAGGAGE TABLE ITEM " << _itemNo << " FROM VENDOR " << _vendor << "\n"
      << " ITEM " << _itemNo;

  RuleDescriptionFormater::format(buf,
                                  services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo));

  return buf.str();
}
}
