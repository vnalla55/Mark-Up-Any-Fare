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

#include "Rules/BusinessRuleApplicator.h"
#include "Rules/PaymentDetail.h"

#include <memory>

namespace tax
{

class ReportingRecord;
class ReportingRecordRule;

class ReportingRecordApplicator : public BusinessRuleApplicator
{
public:
  ReportingRecordApplicator(const ReportingRecordRule* rule,
                            const std::shared_ptr<const ReportingRecord> reportingRecordCode);

  ~ReportingRecordApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const std::shared_ptr<const ReportingRecord> _reportingRecord;
};

} // namespace tax
