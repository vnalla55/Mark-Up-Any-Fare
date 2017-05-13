//----------------------------------------------------------------------------
//  File:        WnSnapUtil.h
//  Created:     2010-05-27
//
//  Description: WN SNAP utility class
//
//  Updates:
//
//  Copyright Sabre 2010
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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

namespace tse
{
class FarePath;
class Itin;
class PricingTrx;
class TravelSeg;

class WnSnapUtil
{

public:
  static void splitItinsByDirection(PricingTrx& trx, bool useOriginalItin = false);

  static void createFarePathsForOutIn(PricingTrx& trx, bool useOriginalItin = false);

  static bool completeFarePaths(const PricingTrx& trx, Itin* itinFirst, Itin* itinSecond);

  static void addArunkSegments(PricingTrx& trx, Itin* itin);

  static bool applyToEntireTrip(TaxItem const* taxItem,
                                const Itin* outItin,
                                const Itin* inItin,
                                PricingTrx& trxForFallbackOnly);

  static void copyTaxResponse(PricingTrx& trx,
                              Itin* itin,
                              Itin* resultItin,
                              FarePath* farePath,
                              int16_t legId,
                              const uint16_t outBrandIndex = INVALID_BRAND_INDEX,
                              const uint16_t inBrandIndex = INVALID_BRAND_INDEX,
                              const Itin* outItin = nullptr,
                              const Itin* inItin = nullptr);

  static void buildSubItinVecWithEmptyValues(PricingTrx& trx);

  static MoneyAmount divideAmountWithRounding(TaxItem* taxItem);

private:
  static bool farePathForPaxFound(const PaxType* paxType, const std::vector<FarePath*>& farePathVec);

  static bool startFromFirstCxr(Itin* itin);

  static void
  getTravelSegmentsBasedOnItin(Itin* itin, Itin* resultItin, int16_t legId, bool& bFound);

  static bool createFarePathBasedOnItin(
      PricingTrx& trx, Itin* itin, Itin* outboundItin, Itin* inboundItin, int16_t legId);

  static void getApplicablePricingUnits(PricingTrx& trx,
                                        const FarePath* farePath,
                                        const int16_t legId,
                                        std::vector<PricingUnit*>& outPUVector);

  static bool containArunkSeg(const std::vector<FareUsage*>& fuVec);

  static bool cleanupArunkSegments(FarePath* farePath);

  static PricingUnit*
  getPricingUnitForLeg(PricingTrx& trx, const PricingUnit* pu, const int16_t legId);

  static void pricingUnitOnLegWithId(const PricingUnit* pricingUnit,
                                     const int16_t legId,
                                     bool& foundTripPartOnReqLeg,
                                     bool& foundTripPartOnOtherLegs);

  static void calcTotalAmountForFarePaths(Itin* itin);

  static ArunkSeg* buildArunkSegment(PricingTrx& trx, TravelSeg* firstTs, TravelSeg* secondTs);
};

} // namespace tse

