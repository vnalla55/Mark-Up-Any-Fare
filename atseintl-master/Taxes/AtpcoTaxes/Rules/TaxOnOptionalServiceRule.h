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
#pragma once

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxOnOptionalServiceApplicator;

class TaxOnOptionalServiceRule : public BusinessRule
{
public:
  typedef TaxOnOptionalServiceApplicator ApplicatorType;
  TaxOnOptionalServiceRule(type::Index const& itemNo, type::Vendor const& vendor);

  virtual ~TaxOnOptionalServiceRule() {}

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& rawPayments) const;

  type::Index const& itemNo() const { return _itemNo; }

private:
  type::Index _itemNo;
  type::Vendor _vendor;
};
}