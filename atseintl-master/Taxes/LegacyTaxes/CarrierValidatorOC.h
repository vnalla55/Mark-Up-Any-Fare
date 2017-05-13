// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/TaxExemptionCarrier.h"
#include "DataModel/AirSeg.h"
#include <boost/utility.hpp>

namespace tse

{
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class OCFees;

class CarrierValidatorOC : public CarrierValidator
{
public:
  virtual bool validateCarrier(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t startIndex) override;

  void setOcFees(OCFees* ocFees) { _ocFees = ocFees; }

protected:
  virtual bool validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier);

private:
  OCFees* _ocFees = nullptr;
};
}
