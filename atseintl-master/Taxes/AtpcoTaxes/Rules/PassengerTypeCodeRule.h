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

namespace tax
{
class PassengerTypeCodeApplicatorFacade;
class Request;

class PassengerTypeCodeRule : public BusinessRule
{
public:
  typedef PassengerTypeCodeApplicatorFacade ApplicatorType;
  PassengerTypeCodeRule(const type::Vendor& vendor,
                        const type::Index& itemNo,
                        tax::type::TaxRemittanceId taxRemittanceId)
    : _vendor(vendor), _itemNo(itemNo), _taxRemittanceId(taxRemittanceId)
  {
  }
  virtual ~PassengerTypeCodeRule() {}
  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& itinPayments) const;
  const type::Vendor& vendor() const { return _vendor; }

  const type::Index& itemNo() const { return _itemNo; }

private:
  type::Vendor _vendor;
  type::Index _itemNo;
  type::TaxRemittanceId _taxRemittanceId;
};
}
