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

#ifndef FARE_CLASS_VALIDATOR_H
#define FARE_CLASS_VALIDATOR_H

#include "Common/TseCodeTypes.h"

namespace tse

{
class PricingTrx;
class TaxCodeReg;
class TaxResponse;

class FareClassValidator
{

public:
  FareClassValidator();
  virtual ~FareClassValidator();

  bool validateFareClassRestriction(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t travelSegIndex);

private:
  static constexpr char TAX_EXLUDE = 'Y';
  static constexpr char TAX_NOT_EXLUDE = 'N';

  FareClassValidator(const FareClassValidator& validation);
  FareClassValidator& operator=(const FareClassValidator& validation);
};
}

#endif // FARE_CLASS_VALIDATOR_H
