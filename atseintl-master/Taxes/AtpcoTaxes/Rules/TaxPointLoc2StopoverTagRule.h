// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class LocZone;
class Request;
class TaxPointLoc2StopoverTagApplicator;

class TaxPointLoc2StopoverTagRule : public BusinessRule
{
public:
  typedef TaxPointLoc2StopoverTagApplicator ApplicatorType;
  TaxPointLoc2StopoverTagRule(const type::Loc2StopoverTag& loc2StopoverTag,
                              const type::TaxMatchingApplTag& taxMatchingApplTag,
                              const type::TaxPointTag& taxPointTag,
                              const type::TaxProcessingApplTag& taxProcessingApplTag,
                              LocZone taxPointLoc1Zone,
                              LocZone taxPointLoc2Zone);
  virtual ~TaxPointLoc2StopoverTagRule();

  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  virtual std::string getDescription(Services& services) const override;

  const type::Loc2StopoverTag& getLoc2StopoverTag() const { return _loc2StopoverTag; }
  const type::TaxMatchingApplTag& getTaxMatchingApplTag() const { return _taxMatchingApplTag; }
  const type::TaxPointTag& getTaxPointTag() const { return _taxPointTag; }
  const type::TaxProcessingApplTag& getTaxProcessingApplTag() const { return _taxProcessingApplTag; }
  bool hasAlternateTurnaroundDeterminationLogic() const
  {
    return _taxProcessingApplTag == "02";
  }
  LocZone getTaxPointLoc1Zone() const { return _taxPointLoc1Zone; }
  LocZone getTaxPointLoc2Zone() const { return _taxPointLoc2Zone; }

private:
  type::Loc2StopoverTag _loc2StopoverTag;
  type::TaxMatchingApplTag _taxMatchingApplTag;
  type::TaxPointTag _taxPointTag;
  type::TaxProcessingApplTag _taxProcessingApplTag;
  LocZone _taxPointLoc1Zone;
  LocZone _taxPointLoc2Zone;
};
}
