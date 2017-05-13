// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "DataModel/TaxResponse.h"

#include "Common/Assert.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

using namespace std;

namespace tse
{
const TaxResponse*
TaxResponse::findFor(const FarePath* farepath)
{
  TSE_ASSERT(farepath);
  return findFor(farepath->itin(), farepath);
}

const TaxResponse*
TaxResponse::findFor(const Itin* itin, const FarePath* farepath)
{
  TSE_ASSERT(itin);
  TSE_ASSERT(farepath);

  for (const TaxResponse* response : itin->getTaxResponses())
  {
    if (response->farePath() == farepath)
    {
      return response;
    }
  }
  return nullptr;
}

void
TaxResponse::getTaxRecordTotal(Money& total) const
{
  if(!taxRecordVector().empty())
  {
    total.setCode(taxRecordVector().front()->taxCurrencyCode());
    for (const TaxRecord* taxRecord : taxRecordVector())
    {
      total.value() += taxRecord->getTaxAmount();
    }
  }
}
} // tse namespace
