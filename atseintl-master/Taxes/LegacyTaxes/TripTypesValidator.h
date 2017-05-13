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

#ifndef TRIP_TYPES_VALIDATOR_H
#define TRIP_TYPES_VALIDATOR_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"

namespace tse

{
class PricingTrx;
class TaxCodeReg;
class TaxResponse;

class TripTypesValidator
{
  friend class TripTypesValidatorTest;

public:
  TripTypesValidator() = default;
  virtual ~TripTypesValidator() = default;

  void setTravelSeg(const std::vector<TravelSeg*>* seg) { _travelSeg=seg; };

  bool validateTrip(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t& startIndex,
                    uint16_t& endIndex);

  //-------------------------------
  //--- Tax Database definition ---
  //-------------------------------

  static constexpr char YES = 'Y';

  //------------------------
  //--- apply to LocAppl ---
  //------------------------

  static constexpr char TAX_ORIGIN = 'O'; // first board pt
  static constexpr char TAX_DESTINATION = 'D'; // furthest point
  static constexpr char TAX_ENPLANEMENT = 'E'; // any board pt in travelSeg
  static constexpr char TAX_DEPLANEMENT = 'X'; // any off pt in travelSeg
  static constexpr char TAX_TERMINATION = 'T'; // the last off point

  // apply to Trip Type

  static constexpr char TAX_FROM_TO = 'F'; // From 1 to 2
  static constexpr char TAX_BETWEEN = 'B'; // From 1 to 2  OR  From 2 to 1
  static constexpr char TAX_WITHIN_SPEC = 'S';
  static constexpr char TAX_WITHIN_WHOLLY = 'W';

  // apply to Travel Type

  static constexpr char TAX_DOMESTIC = 'D';
  static constexpr char TAX_INTERNATIONAL = 'I';

  // apply to Itinerary Type

  static constexpr char TAX_CIRCLETRIP = 'R';
  static constexpr char TAX_ONEWAYTRIP = 'O';

  static const std::string TAX_CODE_TS;

protected:
  TripTypesValidator(const TripTypesValidator& map) = delete;
  TripTypesValidator& operator=(const TripTypesValidator& map) = delete;

  virtual bool validateFromTo(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex);

  bool validateBetween(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t& startIndex,
                       uint16_t& endIndex);

  bool validateWithinSpec(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t& startIndex,
                          uint16_t& endIndex);

  bool validateWithinWholly(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex);

  virtual uint16_t findTaxStopOverIndex(const PricingTrx& trx,
                                        const TaxResponse& taxResponse,
                                        const TaxCodeReg& taxCodeReg,
                                        uint16_t startIndex);

  uint16_t findTaxStopOverIndex(const PricingTrx& trx,
                                       const TaxResponse& taxResponse,
                                       const TaxCodeReg& taxCodeReg,
                                       uint16_t startIndex,
                                       MirrorImage& mirrorImage,
                                       TransitValidator& transitValidator);

  uint16_t findTaxStopOverIndex_new(const PricingTrx& trx,
                                           const TaxResponse& taxResponse,
                                           const TaxCodeReg& taxCodeReg,
                                           uint16_t startIndex,
                                           MirrorImage& mirrorImage,
                                           TransitValidator& transitValidator);

  uint16_t findFarthestPointIndex(const PricingTrx& trx,
                                         const TaxResponse& taxResponse,
                                         const TaxCodeReg& taxCodeReg,
                                         uint16_t startIndex,
                                         uint16_t stopOverIndex);

private:
  const std::vector<TravelSeg*>* _travelSeg = nullptr;

  const std::vector<TravelSeg*>& getTravelSeg(const TaxResponse& taxResponse) const;
};
}

#endif // TRIP_TYPES_VALIDATOR_H
