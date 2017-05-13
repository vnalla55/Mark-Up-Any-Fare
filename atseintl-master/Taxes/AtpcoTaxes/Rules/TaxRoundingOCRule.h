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
#include "Rules/TaxRoundingRule.h"

namespace tax
{

class Request;
class Services;
class TaxRoundingOCApplicator;

class TaxRoundingOCRule : public TaxRoundingRule
{
public:
  typedef TaxRoundingOCApplicator ApplicatorType;
  TaxRoundingOCRule(const type::TaxRoundingUnit& taxRoundingUnit,
                    const type::TaxRoundingDir& taxRoundingDir);

  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& rawPayments) const;
};
}
