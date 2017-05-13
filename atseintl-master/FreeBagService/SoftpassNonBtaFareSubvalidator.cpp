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

#include "FreeBagService/SoftpassNonBtaFareSubvalidator.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
StatusS7Validation SoftpassNonBtaFareSubvalidator::validate(const OptionalServicesInfo& s7, OCFees& ocFees)
{
  if (!s7.ruleTariffInd().empty())
    ocFees.mutableBagSoftPass().set(OCFees::BAG_SP_RULETARIFF);

  if (!s7.tourCode().empty())
    ocFees.mutableBagSoftPass().set(OCFees::BAG_SP_TOURCODE);

  return PASS_S7;
}
}
