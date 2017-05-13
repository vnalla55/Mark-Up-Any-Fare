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

#include "Rules/BusinessRule.h"
#include "DataModel/Services/ServiceFeeSecurity.h"

namespace tax
{
class Request;
class ServiceFeeSecurityApplicatorFacade;

class ServiceFeeSecurityRule : public BusinessRule
{
public:
  typedef ServiceFeeSecurityApplicatorFacade ApplicatorType;
  ServiceFeeSecurityRule(const type::Vendor& vendor, const type::Index& itemNo)
    : _vendor(vendor), _itemNo(itemNo)
  {
  }

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& itinPayments) const;

  const type::Vendor& getVendor() const { return _vendor; }
  const type::Index& getItemNo() const { return _itemNo; }

private:
  type::Vendor _vendor;
  type::Index _itemNo;
};
}
