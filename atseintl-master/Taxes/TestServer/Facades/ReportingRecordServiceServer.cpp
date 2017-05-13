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
#include "ReportingRecordServiceServer.h"

namespace tax
{

ReportingRecordServiceServer::ReportingRecordServiceServer()
{
}

ReportingRecordServiceServer::~ReportingRecordServiceServer()
{
}

ReportingRecordServiceServer::SharedConstSingleValue
ReportingRecordServiceServer::getReportingRecord(const tax::type::Vendor& vendor,
                                                 const tax::type::Nation& nation,
                                                 const tax::type::CarrierCode& taxCarrier,
                                                 const tax::type::TaxCode& taxCode,
                                                 const tax::type::TaxType& taxType) const
{
  for (SharedSingleValue elem : _reportingRecords)
  {
    if (elem->vendor == vendor && elem->nation == nation && elem->taxCarrier == taxCarrier &&
        elem->taxCode == taxCode && elem->taxType == taxType)
    {
      return elem;
    }
  }
  return SharedSingleValue();
}

ReportingRecordServiceServer::SharedConstGroupValue
ReportingRecordServiceServer::getReportingRecords(const type::TaxCode& taxCode) const
{
  SharedGroupValue ans = std::make_shared<GroupValue>();
  for (SharedSingleValue elem : _reportingRecords)
  {
    if (elem->taxCode == taxCode)
    {
      ans->push_back(elem);
    }
  }
  return SharedGroupValue(ans);
}

} // namespace tax
