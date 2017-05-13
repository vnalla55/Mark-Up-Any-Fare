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
class SectorDetailApplicator;
class Request;

class SectorDetailRule : public BusinessRule
{
public:
  typedef SectorDetailApplicator ApplicatorType;
  SectorDetailRule(type::SectorDetailApplTag const& sectorDetailApplTag,
                   type::Index const& itemNo,
                   type::Vendor const& vendor);

  virtual ~SectorDetailRule();

  virtual std::string getDescription(Services& services) const override;

  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::Index const& itemNo() const { return _itemNo; }
  type::Vendor const& vendor() const { return _vendor; }

private:
  type::SectorDetailApplTag _sectorDetailApplTag;
  type::Index _itemNo;
  type::Vendor _vendor;
};
}
