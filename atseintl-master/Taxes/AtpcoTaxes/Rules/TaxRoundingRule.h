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
class TaxRoundingApplicator;

class TaxRoundingRule : public BusinessRule
{
public:
  typedef TaxRoundingApplicator ApplicatorType;
  TaxRoundingRule(const type::TaxRoundingUnit& taxRoundingUnit,
                  const type::TaxRoundingDir& taxRoundingDir,
                  const type::TaxRoundingPrecision& taxRoundingPrecision);
  virtual ~TaxRoundingRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  const type::TaxRoundingUnit& taxRoundingUnit() const { return _taxRoundingUnit; }

  const type::TaxRoundingDir& taxRoundingDir() const { return _taxRoundingDir; }

  const type::TaxRoundingPrecision& taxRoundingPrecision() const { return _taxRoundingPrecision; }

private:
  type::TaxRoundingUnit _taxRoundingUnit;
  type::TaxRoundingDir _taxRoundingDir;
  type::TaxRoundingPrecision _taxRoundingPrecision;
};
}
