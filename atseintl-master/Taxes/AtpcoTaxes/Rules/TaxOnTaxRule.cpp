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
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/TaxOnTaxApplicator.h"
#include "Rules/TaxOnTaxRule.h"

namespace tax
{

TaxOnTaxRule::TaxOnTaxRule(type::Index const& itemNo, type::Vendor const& vendor)
  : _itemNo(itemNo), _vendor(vendor)
{
}

TaxOnTaxRule::ApplicatorType
TaxOnTaxRule::createApplicator(const type::Index& /*itinIndex*/,
                               const Request& /*request*/,
                               Services& services,
                               RawPayments& rawPayments) const
{
  std::shared_ptr<ServiceBaggage const> serviceBaggage;
  bool restrictServiceBaggage = (_itemNo != 0);
  if (restrictServiceBaggage)
  {
    serviceBaggage = services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo);
  }

  return ApplicatorType(*this, services, serviceBaggage, restrictServiceBaggage, rawPayments);
}

std::string
TaxOnTaxRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "CALCULATE TAX ON TAXES";
  if (_itemNo == 0)
  {
    return buf.str();
  }

  buf << " THAT SATISFY RESTRICTIONS FROM\n"
      << " SERVICE BAGGAGE TABLE ITEM " << _itemNo << " FROM VENDOR " << _vendor << "\n"
      << " ITEM " << _itemNo;

  RuleDescriptionFormater::format(buf,
                                  services.serviceBaggageService().getServiceBaggage(_vendor, _itemNo));

  return buf.str();
}
}
