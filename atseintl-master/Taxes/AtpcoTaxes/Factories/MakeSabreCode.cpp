// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "Factories/MakeSabreCode.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/ReportingRecordService.h"

namespace tax
{

type::SabreTaxCode
makeItinSabreCode(type::TaxCode taxCode,
                  const type::TaxType& taxType,
                  const type::PercentFlatTag& percentFlatTag)
{
  type::SabreTaxCode sabreTaxCode = taxCode.asString();
  char* end = nullptr;
  long i = strtol(taxType.asString().c_str(), &end, 10);

  if (sabreTaxCode == "US")
  {
    sabreTaxCode += (percentFlatTag == type::PercentFlatTag::Percent ? '1' : '2');
  }
  else if (*end != '\0' || i >= 22)
  {
    sabreTaxCode += 'L';
  }
  else if (i != 1) // "GB001" should convert to "GB"
  {
    static const char suffix[22] = { 'L', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
                                     'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L' };

    sabreTaxCode += suffix[i];
  }

  return sabreTaxCode;
}

type::SabreTaxCode
makeServiceSabreCode(type::TaxCode taxCode)
{
  return taxCode.asString() + 'A';
}

} // namespace tax
