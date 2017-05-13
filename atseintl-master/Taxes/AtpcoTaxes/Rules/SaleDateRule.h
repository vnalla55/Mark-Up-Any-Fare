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
class SaleDateApplicator;
class Request;
class Services;

class SaleDateRule : public BusinessRule
{
public:
  typedef SaleDateApplicator ApplicatorType;
  SaleDateRule(type::Date effDate, type::Timestamp discDate);
  virtual ~SaleDateRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

private:
  type::Date _effDate;
  type::Timestamp _discDate;
};
}
