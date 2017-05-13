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

#ifndef ADJUST_TAX_H
#define ADJUST_TAX_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class CalculationDetails;

class AdjustTax
{
public:
  static constexpr char PERCENTAGE = 'P';

  static MoneyAmount applyAdjust(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          const MoneyAmount& taxAmount,
                          const CurrencyCode& paymentCurrency,
                          TaxCodeReg& taxCodeReg,
                          CalculationDetails& details);

private:
  static const int NUM_COUPON = 4;
  static constexpr char TICKET_BOOKLET = 'B';

  AdjustTax(){};
};
}

#endif // ADJUST_TAX_H
