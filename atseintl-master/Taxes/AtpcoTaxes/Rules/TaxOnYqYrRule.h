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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxOnYqYrApplicator;

class TaxOnYqYrRule : public BusinessRule
{
public:
  typedef TaxOnYqYrApplicator ApplicatorType;
  TaxOnYqYrRule(type::Index const& itemNo,
                type::Vendor const& vendor,
                type::TaxAppliesToTagInd const& taxAppliesToTagInd);

  virtual ~TaxOnYqYrRule() {}

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& rawPayments) const;

  type::Index const& itemNo() const { return _itemNo; }
  type::TaxAppliesToTagInd const& taxAppliesToTagInd() const { return _taxAppliesToTagInd; }

private:
  type::Index _itemNo;
  type::Vendor _vendor;
  type::TaxAppliesToTagInd _taxAppliesToTagInd;
};
}
