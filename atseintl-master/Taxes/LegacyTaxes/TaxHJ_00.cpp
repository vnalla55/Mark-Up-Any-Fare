//---------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxCodeValidatorNoPsg.h"
#include "Taxes/LegacyTaxes/TaxHJ_00.h"

namespace tse
{

bool
TaxHJ_00::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxCodeValidatorNoPsg taxCodeValidator;

  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}

bool
TaxHJ_00::validateFinalGenericRestrictions(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& /*endIndex*/)
{
  const PaxType* paxType = taxUtil::findActualPaxType (trx, taxResponse.farePath(), startIndex);
  TaxCodeValidator taxCodeValidator;
  return taxCodeValidator.validatePassengerRestrictions(trx, taxResponse, taxCodeReg, paxType);
}



}; //namespace tse
