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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class OptionalServiceTagsApplicator;
class Request;
class Services;

class OptionalServiceTagsRule : public BusinessRule
{
public:
  typedef OptionalServiceTagsApplicator ApplicatorType;
  explicit OptionalServiceTagsRule(TaxableUnitTagSet const& applicableTaxableUnits);

  virtual ~OptionalServiceTagsRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  TaxableUnitTagSet const& taxableUnitSet() const { return _taxableUnitSet; }

private:
  TaxableUnitTagSet _taxableUnitSet;
};
}

