//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "FreeBagService/SoftpassBtaSubvalidator.h"

#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
StatusS7Validation
SoftpassBtaSubvalidator::validate(const OptionalServicesInfo& s7, OCFees& ocFees)
{
  Context ctx(s7, ocFees, _bt._trx->dataHandle());

  const StatusS7Validation res = validateImpl(ALL_CHECKS & ~REVALIDATION_CHECKS, ctx);

  if (res != PASS_S7)
    return res;

  if (adjustMask(REVALIDATION_CHECKS, ctx).any())
    ocFees.mutableBagSoftPass().set(OCFees::BAG_SP_BTA_FARE_CHECKS);

  return PASS_S7;
}
}
