// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/Codes.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;

class CustomerRestrictionApplicator;

class CustomerRestrictionRule: public BusinessRule
{
public:
  typedef CustomerRestrictionApplicator ApplicatorType;

  CustomerRestrictionRule(const type::CarrierCode& carrier);

  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

  const type::CarrierCode& carrierCode() const {return _carrierCode;};

private:
  type::CarrierCode _carrierCode;
};

} /* namespace tax */

