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

#include "ServiceInterfaces/ReportingRecordService.h"

namespace tax
{

class ReportingRecordServiceServer : public ReportingRecordService
{
public:
  ReportingRecordServiceServer();
  virtual ~ReportingRecordServiceServer();

  virtual SharedConstSingleValue getReportingRecord(const tax::type::Vendor& vendor,
                                                    const tax::type::Nation& nation,
                                                    const tax::type::CarrierCode& taxCarrier,
                                                    const tax::type::TaxCode& taxCode,
                                                    const tax::type::TaxType& taxType) const;

  virtual SharedConstGroupValue
  getReportingRecords(const type::TaxCode& taxCode) const;

  GroupValue& reportingRecords() { return _reportingRecords; }
  const GroupValue& reportingRecords() const { return _reportingRecords; };

private:
  GroupValue _reportingRecords;
};
}

