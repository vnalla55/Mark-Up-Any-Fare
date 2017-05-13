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

#include "test/ReportingRecordServiceMock.h"

namespace tax
{

ReportingRecordServiceMock::ReportingRecordServiceMock() :
    _records(new ReportingRecordServiceMock::GroupValue())
{
}

ReportingRecordServiceMock::SharedConstSingleValue
ReportingRecordServiceMock::getReportingRecord(const type::Vendor& /*vendor*/,
                                               const type::Nation& /*nation*/,
                                               const type::CarrierCode& /*taxCarrierCode*/,
                                               const type::TaxCode& taxCode,
                                               const type::TaxType& taxType) const
{
  for(GroupValue::const_iterator record = _records->begin();
      record != _records->end(); ++record)
  {
    if((*record)->taxCode == taxCode && (*record)->taxType == taxType)
      return *record;
  }

  return SharedConstSingleValue();
}

}
