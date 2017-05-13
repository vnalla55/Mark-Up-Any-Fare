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
class ReportingRecordApplicator;
class Request;

class ReportingRecordRule : public BusinessRule
{
public:
  typedef ReportingRecordApplicator ApplicatorType;
  ReportingRecordRule(const tax::type::Vendor& vendor,
                      const tax::type::Nation& nation,
                      const tax::type::CarrierCode& taxCarrierCode,
                      const tax::type::TaxCode& taxCode,
                      const tax::type::TaxType& taxType);

  virtual ~ReportingRecordRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  const type::Vendor& getVendor() const { return _vendor; }
  const type::Nation& getNation() const { return _nation; }
  const type::TaxCode& getTaxCode() const { return _taxCode; }
  const type::TaxType& getTaxType() const { return _taxType; }

private:
  type::Vendor _vendor;
  type::Nation _nation;
  type::CarrierCode _taxCarrierCode;
  type::TaxCode _taxCode;
  type::TaxType _taxType;
};
}
