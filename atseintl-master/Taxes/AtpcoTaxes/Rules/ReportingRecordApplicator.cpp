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

#include "DataModel/Services/ReportingRecord.h"
#include "Rules/PaymentDetail.h"
#include "Rules/ReportingRecordApplicator.h"
#include "Rules/ReportingRecordRule.h"

namespace tax
{
ReportingRecordApplicator::ReportingRecordApplicator(
    const ReportingRecordRule* rule, const std::shared_ptr<const ReportingRecord> reportingRecord)
  : BusinessRuleApplicator(rule), _reportingRecord(reportingRecord)
{
}

ReportingRecordApplicator::~ReportingRecordApplicator() {}

bool
ReportingRecordApplicator::apply(PaymentDetail& paymentDetail) const
{
  if ((_reportingRecord != nullptr) && !_reportingRecord->entries.empty())
  {
    paymentDetail.taxLabel() = _reportingRecord->entries.front().taxLabel;

    if(_reportingRecord->isVatTax)
      paymentDetail.setIsVatTax();
  }

  return true;
}

} // namespace tax
