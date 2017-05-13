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

#include "Common/Consts.h"
#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class LocZone;
class Request;
class TaxPointLoc1Applicator;

class TaxPointLoc1Rule : public BusinessRule
{
public:
  typedef TaxPointLoc1Applicator ApplicatorType;
  TaxPointLoc1Rule(LocZone const& locZone, type::Vendor const& vendor);
  virtual ~TaxPointLoc1Rule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  LocZone const& locZone() const { return _locZone; }
  type::Vendor const& vendor() const { return _vendor; }

private:
  LocZone _locZone;
  type::Vendor _vendor;
};
}
