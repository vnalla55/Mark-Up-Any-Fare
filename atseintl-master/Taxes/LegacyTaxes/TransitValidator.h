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

#ifndef TRANSIT_VALIDATOR_H
#define TRANSIT_VALIDATOR_H

#include <stdlib.h>
#include <vector>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse

{
class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class TaxRestrictionTransit;
class TravelSeg;

class TransitValidator
{

public:
  static constexpr char YES = 'Y';
  enum
  {
    SHOULD_NOT_CHECK_OPEN = false,
    SHOULD_CHECK_OPEN = true
  };

  TransitValidator() = default;
  virtual ~TransitValidator() = default;

  virtual bool validateTransitTime(const PricingTrx& trx,
                                   const TaxResponse& taxResponse,
                                   const TaxCodeReg& taxCodeReg,
                                   uint16_t startIndex,
                                   bool shouldCheckOpen = SHOULD_NOT_CHECK_OPEN);

  bool validateTransitRestriction(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t startIndex);

  bool validateTransitRestriction(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t startIndex,
                                  bool mirrorImage,
                                  bool landToAirStopover);

  void setTravelSeg(const std::vector<TravelSeg*>* seg) { _travelSeg=seg; };

private:
  bool validateTransitIndicators(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 TaxRestrictionTransit& restrictTransit,
                                 uint16_t startIndex);

  virtual bool validTransitIndicatorsIfMultiCity(TravelSeg* travelSegTo, TravelSeg* travelSegFrom);

  const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const;

  TransitValidator(const TransitValidator& validation);
  TransitValidator& operator=(const TransitValidator& validation);

  const std::vector<TravelSeg*>* _travelSeg = nullptr;
};
}

#endif // FARE_CLASS_VALIDATOR_H
