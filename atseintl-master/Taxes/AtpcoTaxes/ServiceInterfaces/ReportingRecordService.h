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

#include <memory>
#include <tuple>
#include <vector>

namespace tax
{

class ReportingRecord;

class ReportingRecordService
{
public:
  typedef std::tuple<type::Vendor, type::Nation, type::CarrierCode, type::TaxCode, type::TaxType>
  SingleKey;
  typedef ReportingRecord SingleValue;
  typedef std::shared_ptr<SingleValue> SharedSingleValue;
  typedef std::shared_ptr<const SingleValue> SharedConstSingleValue;

  typedef type::TaxCode GroupKey;
  typedef std::vector<SharedSingleValue> GroupValue;
  typedef std::shared_ptr<GroupValue> SharedGroupValue;
  typedef std::shared_ptr<const GroupValue> SharedConstGroupValue;

  ReportingRecordService() {}

  virtual ~ReportingRecordService() {}

  virtual SharedConstSingleValue getReportingRecord(const type::Vendor& vendor,
                                                    const type::Nation& nation,
                                                    const type::CarrierCode& taxCarrierCode,
                                                    const type::TaxCode& taxCode,
                                                    const type::TaxType& taxType) const = 0;

  virtual SharedConstGroupValue
  getReportingRecords(const type::TaxCode& taxCode) const = 0;

private:
  ReportingRecordService(const ReportingRecordService&);
  ReportingRecordService& operator=(const ReportingRecordService&);
};
}

