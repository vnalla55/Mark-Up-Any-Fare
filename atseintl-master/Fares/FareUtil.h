//-------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/JourneyItinWrapper.h"

namespace tse
{

class Itin;
class FareClassAppSegInfo;
class PricingTrx;
class JourneyItinWrapper;

class FareUtil
{
public:
  static bool isNegFareCarrierValid(const CarrierCode& carrier,
                                    const Indicator& tktAppl,
                                    const Itin& itin,
                                    const bool isFareDisplay);

  static bool isNegFareCarrierValid(const CarrierCode& carrier,
                                    const Indicator& tktAppl,
                                    const CarrierCode& cxr,
                                    const bool isFareDisplay);

  static bool isNegFareCarrierValid(const CarrierCode& carrier,
                                    const Indicator& tktAppl,
                                    PaxTypeFare* paxTypeFare,
                                    const bool isFareDisplay);

  static bool failFareClassAppSegDirectioanlity(const FareClassAppSegInfo& fcas,
                                                const PricingTrx& trx,
                                                bool flipGeo);

  // for use in NGS
  static void invalidateFailedSops(ShoppingTrx& trx,
                                   FareMarket* fareMarket,
                                   JourneyItinWrapper& journeyItinWrapper);

  // for use in IS old path
  static void invalidateFailedSops(ShoppingTrx& trx,
                                   FareMarket* fareMarket,
                                   const ItinIndex::Key& cxrKey,
                                   ShoppingTrx::Leg& curLeg,
                                   uint32_t legId);

  static bool postCheckOutboundFare(const bool hasRemoveOutboundFares,
                                    const bool isReversed,
                                    const Directionality directionality);

private:
  static void invalidateFailedSops_impl(ShoppingTrx& trx,
                                        FareMarket* fareMarket,
                                        ItinIndex::ItinIndexIterator& ItinIter,
                                        ItinIndex::ItinIndexIterator& IterEnd,
                                        uint32_t legId);

  // returns std::numeric_limits<uint32_t>::max() if no valid sop found
  static uint32_t getIndexOfTheFirstSopCoveredByValidFare(FareMarket* fareMarket);

  // Marks all direct sops in the range specified up to a given intex as invalid
  // using the CabinClassValid() flag
  static std::vector<int> markDirectSopsUpToIndexAsInvalid(uint32_t bitValue,
                                                           ShoppingTrx& trx,
                                                           ItinIndex::ItinIndexIterator& ItinIter,
                                                           ItinIndex::ItinIndexIterator& IterEnd,
                                                           uint32_t legId);

  static void printInvalidatedSopsToDiag911(FareMarket* fareMarket,
                                            std::vector<int>& invalidSops,
                                            DiagManager& diag);
};
}
