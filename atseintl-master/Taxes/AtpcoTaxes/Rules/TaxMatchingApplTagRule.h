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
class Services;
class TaxMatchingApplTagApplicator;

class TaxMatchingApplTagRule : public BusinessRule
{
public:
  typedef TaxMatchingApplTagApplicator ApplicatorType;
  TaxMatchingApplTagRule(type::TaxMatchingApplTag const& taxMatchingApplTag,
                         bool alternateTurnaroundDeterminationLogic);
  virtual ~TaxMatchingApplTagRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::TaxMatchingApplTag const& taxMatchingApplTag() const { return _taxMatchingApplTag; }

  bool alternateTurnaroundDeterminationLogic() const
  {
    return _alternateTurnaroundDeterminationLogic;
  }

private:
  type::TaxMatchingApplTag _taxMatchingApplTag;
  bool _alternateTurnaroundDeterminationLogic;
};
}
