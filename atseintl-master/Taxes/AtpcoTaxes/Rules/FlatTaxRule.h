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

#include <string>

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class FlatTaxApplicator;
class Request;
class Services;

class FlatTaxRule : public BusinessRule
{
public:
  typedef FlatTaxApplicator ApplicatorType;
  FlatTaxRule(TaxableUnitTagSet const& applicableTaxableUnits);
  virtual ~FlatTaxRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

private:
  TaxableUnitTagSet _taxableUnitSet;
};
}

