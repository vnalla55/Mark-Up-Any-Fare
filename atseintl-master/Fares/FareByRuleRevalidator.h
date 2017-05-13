//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "DBAccess/FareByRuleItemInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "Fares/AvailabilityChecker.h"

#include <cstdint>
#include <vector>

namespace tse
{

class FareByRuleRevalidator
{
public:
  template <typename C = AvailabilityChecker>
  bool checkFBR(const PaxTypeFare& paxTypeFare,
                const uint16_t numSeats,
                const FareMarket& fareMarket,
                const std::vector<Itin*>& owningItins,
                const C& availChecker = C()) const
  {
    const bool fmCheck = checkFBR(&paxTypeFare,
                                  numSeats,
                                  fareMarket.classOfServiceVec(),
                                  fareMarket.travelSeg(),
                                  availChecker);
    if (fmCheck)
      return true;

    return std::any_of(
        owningItins.begin(),
        owningItins.end(),
        [&](const Itin* itin)
        { return checkFBR(paxTypeFare, numSeats, fareMarket, *itin, availChecker); });
  }

  template <typename CosV, typename Checker = AvailabilityChecker>
  bool checkFBR(const PaxTypeFare* paxTypeFare,
                const uint16_t numSeats,
                const CosV& classOfServiceVec,
                const std::vector<TravelSeg*>& segments,
                const Checker& availChecker = Checker()) const
  {
    TSE_ASSERT(paxTypeFare);
    if (paxTypeFare->fareByRuleInfo().fltSegCnt() != 0 &&
        paxTypeFare->fareByRuleInfo().fltSegCnt() < segments.size())
      return false;

    FBRPaxTypeFareRuleData* fbrPTFRuleData = paxTypeFare->getFbrRuleData();

    if (!fbrPTFRuleData || !fbrPTFRuleData->isBaseFareAvailBkcMatched())
      return true;

    const std::set<BookingCode>& baseFareInfBC = fbrPTFRuleData->baseFareInfoBookingCodes();

    return std::any_of(
        baseFareInfBC.cbegin(),
        baseFareInfBC.cend(),
        [&](BookingCode bkc)
        { return availChecker.checkAvailability(numSeats, bkc, classOfServiceVec, segments); });
  }

private:
  template <typename Checker>
  bool checkFBR(const PaxTypeFare& paxTypeFare,
                const uint16_t numSeats,
                const FareMarket& fareMarket,
                const Itin& owningItin,
                const Checker& availChecker) const
  {
    for (const SimilarItinData& data : owningItin.getSimilarItins())
    {
      auto fmData = data.fareMarketData.find(&fareMarket);
      if (fmData == data.fareMarketData.end())
        continue;
      if (checkFBR(&paxTypeFare,
                   numSeats,
                   fmData->second.classOfService,
                   fmData->second.travelSegments,
                   availChecker))
        return true;
    }

    return false;
  }
};
}

