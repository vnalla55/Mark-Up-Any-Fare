// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------

#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include <boost/bind.hpp>

#include <algorithm>

namespace tse
{

bool
CarrierValidator::validateCarrier(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex)

{
  if (taxCodeReg.exemptionCxr().empty())
    return true;

  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  bool isValid = airSeg && hasMatchingAirSeg(taxCodeReg.exemptionCxr(), *airSeg);

  if ((isValid && taxCodeReg.exempcxrExclInd() != TAX_EXCLUDE) ||
      (!isValid && taxCodeReg.exempcxrExclInd() == TAX_EXCLUDE))
    return true;
  else
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::CARRIER_EXEMPTION, Diagnostic821);
    return false;
  }
}

bool
CarrierValidator::hasMatchingAirSeg(const std::vector<TaxExemptionCarrier>& taxExemptionCxrs,
                                    const AirSeg& airSeg)
{
  return std::find_if(taxExemptionCxrs.begin(),
                      taxExemptionCxrs.end(),
                      boost::bind(&CarrierValidator::validateExemptCxrRecord, this, _1, airSeg)
                     ) != taxExemptionCxrs.end();
}

bool
CarrierValidator::validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier,
                                          const AirSeg& airSeg)
{
  return validateCarrierCode(airSeg.marketingCarrierCode(), taxExemptionCarrier.carrier()) &&
         validateFlightNumber(airSeg.marketingFlightNumber(),
                              taxExemptionCarrier.flight1(),
                              taxExemptionCarrier.flight2()) &&
         validateFlightDirection(taxExemptionCarrier.airport1(),
                                 taxExemptionCarrier.airport2(),
                                 airSeg.origin()->loc(),
                                 airSeg.destination()->loc(),
                                 taxExemptionCarrier.direction());
}

bool
CarrierValidator::validateCarrierCode(const CarrierCode& marketingCarrierCode,
                                      const CarrierCode& carrierCode)
{
  return equalOrEmpty(carrierCode, marketingCarrierCode);
}

bool
CarrierValidator::validateFlightNumber(uint16_t marketingFlight, uint16_t flight1, uint16_t flight2)
{
  if (flight2 > flight1)
  {
    return (marketingFlight >= flight1 && marketingFlight <= flight2);
  }
  else
  {
    return (!flight1 || marketingFlight == flight1);
  }
}

bool
CarrierValidator::validateFlightDirection(const LocCode& airport1,
                                          const LocCode& airport2,
                                          const LocCode& originAirport,
                                          const LocCode& destinationAirport,
                                          const Indicator& direction)
{

  bool isValidFromTo =
      equalOrEmpty(airport1, originAirport) && equalOrEmpty(airport2, destinationAirport);

  if (direction != TAX_BETWEEN)
    return isValidFromTo;
  else
  {
    bool isValidToFrom =
        equalOrEmpty(airport2, originAirport) && equalOrEmpty(airport1, destinationAirport);

    return isValidFromTo || isValidToFrom;
  }
}
}
