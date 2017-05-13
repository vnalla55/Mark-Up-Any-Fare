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

#include "DataModel/Services/ReportingRecord.h"
#include "ServiceInterfaces/ReportingRecordService.h"

namespace tax
{

class ReportingRecordServiceMock : public ReportingRecordService
{
public:
  ReportingRecordServiceMock();
  virtual ~ReportingRecordServiceMock() {}

  virtual SharedConstSingleValue
  getReportingRecord(const type::Vendor& /*vendor*/,
                     const type::Nation& /*nation*/,
                     const type::CarrierCode& /*taxCarrierCode*/,
                     const type::TaxCode& taxCode,
                     const type::TaxType& taxType) const;

  virtual SharedConstGroupValue
  getReportingRecords(const type::TaxCode& /*taxCode*/) const { return _records; }

  SharedGroupValue& records() { return _records; }

private:
  SharedGroupValue _records;
};

} /* namespace tax */
