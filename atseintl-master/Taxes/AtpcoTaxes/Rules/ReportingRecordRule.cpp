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
#include <sstream>

#include "DataModel/Services/ReportingRecord.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/ReportingRecordApplicator.h"
#include "Rules/ReportingRecordRule.h"

namespace tax
{

ReportingRecordRule::ReportingRecordRule(const tax::type::Vendor& vendor,
                                         const tax::type::Nation& nation,
                                         const tax::type::CarrierCode& taxCarrierCode,
                                         const tax::type::TaxCode& taxCode,
                                         const tax::type::TaxType& taxType)
  : _vendor(vendor),
    _nation(nation),
    _taxCarrierCode(taxCarrierCode),
    _taxCode(taxCode),
    _taxType(taxType)
{
}

ReportingRecordRule::~ReportingRecordRule() {}

ReportingRecordRule::ApplicatorType
ReportingRecordRule::createApplicator(type::Index const& /*itinIndex*/,
                                      const Request& /*request*/,
                                      Services& services,
                                      RawPayments& /*itinPayments*/) const
{
  const std::shared_ptr<const ReportingRecord> ptc =
      services.reportingRecordService().getReportingRecord(
          _vendor, _nation, _taxCarrierCode, _taxCode, _taxType);

  return ApplicatorType(this, ptc);
}

std::string
ReportingRecordRule::getDescription(Services&) const
{
  return std::string("SET TAXLABEL FROM REPORTING RECORD CACHE");
}
}
