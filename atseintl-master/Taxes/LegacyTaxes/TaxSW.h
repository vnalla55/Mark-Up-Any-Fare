//---------------------------------------------------------------------------
//  Copyright Sabre 2016
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
#pragma once

#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/TravelSeg.h"

#include <boost/utility.hpp>

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 4503 for Japan
//---------------------------------------------------------------------------

class TransitValidatorSW : public TransitValidator
{
  friend class TaxSWTest;

private:
  bool validTransitIndicatorsIfMultiCity(TravelSeg* travelSegTo, TravelSeg* travelSegFrom) override;
};

class TaxSW : public Tax, boost::noncopyable
{
public:
  bool validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  bool validateFinalGenericRestrictions(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& /*endIndex*/) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex,
                       bool isMirrorImage);

  bool validateTransitLoc(const LocCode& destAirport, const LocCode& origAirport) const
  {
    return destAirport == origAirport;
  }
};

} /* end tse namespace */
