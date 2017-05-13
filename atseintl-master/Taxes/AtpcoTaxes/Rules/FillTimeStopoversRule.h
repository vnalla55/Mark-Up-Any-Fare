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
class FillTimeStopoversApplicatorFacade;
class Request;

class FillTimeStopoversRule : public BusinessRule
{
public:
  typedef FillTimeStopoversApplicatorFacade ApplicatorType;
  FillTimeStopoversRule(type::StopoverTimeTag number, type::StopoverTimeUnit const& unit);
  virtual ~FillTimeStopoversRule();

  type::StopoverTimeTag number() const { return _number; }
  type::StopoverTimeUnit unit() const { return _unit; }

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

private:
  type::StopoverTimeTag _number;
  type::StopoverTimeUnit _unit;
};
}

