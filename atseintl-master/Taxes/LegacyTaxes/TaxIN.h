//---------------------------------------------------------------------------
//  Copyright Sabre 2012
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
#ifndef TAX_IN_H
#define TAX_IN_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxIN : public Tax
{

public:
  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  uint16_t findMirrorImage(TaxLocIterator locIt);

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override
  {
    return true;
  }

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override
  {
    return true;
  }

private:
  virtual bool checkMileage(PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      uint16_t startIndex,
      uint16_t endIndex,
      uint16_t& newEndIndex) const
  {
    newEndIndex = endIndex;
    return true;
  }

};

} /* end tse namespace */

#endif /* TAX_IN_H */
