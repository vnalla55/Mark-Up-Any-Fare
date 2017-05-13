// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace tse

{
class TaxResponse;
class PricingTrx;
class TaxCodeReg;
class Money;
class CurrencyConversionFacade;
class OCFees;

class TaxOnOC : public Tax
{

public:
  virtual bool validateCarrierExemption(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t travelSegIndex) override;

  virtual bool validateSequence(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t& travelSegStartIndex,
                                uint16_t& travelSegEndIndex,
                                bool checkSpn = false) override;

  virtual void
  applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override
  {
  }

  virtual void taxCreate(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegStartIndex,
                         uint16_t travelSegEndIndex) override;

  void setOcFees(OCFees* ocFees) { _ocFees = ocFees; }
  OCFees* getOcFees() { return _ocFees; }

  MoneyAmount calculateTaxableAmount(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg,
                                     CurrencyConversionFacade& ccFacade);

private:
  OCFees* _ocFees = nullptr;
};
}

