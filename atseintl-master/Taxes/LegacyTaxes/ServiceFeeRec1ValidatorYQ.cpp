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

#include "DBAccess/YQYRFees.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1ValidatorYQ.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"

namespace tse
{
namespace YQYR
{

void
ServiceFeeRec1Validator::LoadData(YQYRFees& yqyrFees)
{
  taxCode() = yqyrFees.taxCode();
  taxCode() += yqyrFees.subCode();
  carrierCode() = yqyrFees.carrier();
  taxCur() = yqyrFees.cur();
  taxCurNodec() = yqyrFees.noDec();
  seqNo() = yqyrFees.seqNo();

  taxAmt() = yqyrFees.feeAmount();

  if (yqyrFees.percent())
    taxAmt() = yqyrFees.percent();

  taxCur() = yqyrFees.cur();
  taxNodec() = yqyrFees.noDec();

  expireDate() = yqyrFees.expireDate();
  effDate() = yqyrFees.effDate();
  discDate() = yqyrFees.discDate();
  tvlDateasoriginInd() = YES;
  modDate() = yqyrFees.modDate();

  firstTvlDate() = yqyrFees.effDate();
  lastTvlDate() = yqyrFees.discDate();

  // TODO this YQ/YR flag might not allpy to the whole itin, so fake out taxCodeValidator
  //  travelType() = yqyrFees.intlDomInd();
  travelType() = BLANK;
  itineraryType() = BLANK;

  if (yqyrFees.returnToOrigin() == YES)
    itineraryType() = TaxCodeValidator::CIRCLE_TRIP;

  if (yqyrFees.returnToOrigin() == NO)
    itineraryType() = TaxCodeValidator::ONEWAY_TRIP;

  taxIncludedInd() = yqyrFees.taxIncludedInd();
}
}
}
