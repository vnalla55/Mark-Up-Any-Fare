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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/TaxExemptionCarrier.h"

#include <boost/utility.hpp>

namespace tse
{
class AirSeg;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;

class CarrierValidator : private boost::noncopyable
{
public:
  virtual ~CarrierValidator() = default;

  virtual bool validateCarrier(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t startIndex);

protected:
  static constexpr char TAX_EXCLUDE = 'Y';
  static constexpr char TAX_BETWEEN = 'B';

  virtual bool
  hasMatchingAirSeg(const std::vector<TaxExemptionCarrier>& taxExemptionCxrs,
                    const AirSeg& airSeg);
  virtual bool
  validateExemptCxrRecord(const TaxExemptionCarrier& taxExemptionCarrier, const AirSeg& airSeg);
  virtual bool
  validateCarrierCode(const CarrierCode& marketingCarrierCode, const CarrierCode& carrierCode);
  virtual bool validateFlightNumber(uint16_t marketingFlight, uint16_t flight1, uint16_t flight2);
  virtual bool validateFlightDirection(const LocCode& airport1,
                                       const LocCode& airport2,
                                       const LocCode& originAirport,
                                       const LocCode& destinationAirport,
                                       const Indicator& direction);

  template <typename StringLikeType>
  bool equalOrEmpty(const StringLikeType& lhs, const StringLikeType& rhs)
  {
    return lhs == rhs || lhs.empty() || rhs.empty();
  }
};
}
