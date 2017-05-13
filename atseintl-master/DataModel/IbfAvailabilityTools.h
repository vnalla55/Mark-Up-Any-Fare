//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/IbfAvailabilityStatusAccumulators.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{

class Itin;
class FareMarket;
class PaxTypeFare;
class PricingTrx;
class TravelSeg;
class FareMarketPath;
class MergedFareMarket;

struct IbfAvailabilityTools
{
  static void setBrandsStatus(IbfErrorMessage status,
                              PaxTypeFare& ptf,
                              const PricingTrx& trx,
                              const std::string& location = "");

  // Iterates over all mfms in the supplied fare market path
  // and calculates the final status per leg.
  //
  // See IbfErrorMessageSequenceAcc for priorities explanation.
  //
  // Example:
  //           leg0     |     leg1     |    leg2
  //                    |              |
  //        [ mfm1 ] [ mfm2 ] [ mfm3 ] | [ mfm4 ]
  //           NA       NO       NA         NA
  //                    |              |
  // legs:  0        0,1      1          2
  //
  // final
  // status:   NO             NO         NA
  //
  // NO = not offered
  // NA = not available
  //
  // Returns: a mapping with calculated errors per leg.
  static std::map<LegId, IbfErrorMessageSequenceAcc>
  calculateLegStatuses(const FareMarketPath& fmp);

  // As calculateLegStatuses, but calculates for each
  // fare market path and pushes all the results to the
  // result vector (first cleared).
  static void
  calculateLegStatusesForMatrix(std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& result,
                                const std::vector<FareMarketPath*>& fareMarketPathMatrix);

  // Takes a vector of mappings legId->status (coming e.g. from multiple fare paths)
  // and calculates the final mapping with statuses per leg. Uses the choice strategy
  // for prioritization inside leg (see IbfErrorMessageChoiceAcc).
  // Example:
  // [A, O]
  // [F, A]
  // [O, A]
  // ------
  // [F, A]
  static std::map<LegId, IbfErrorMessageChoiceAcc> summarizeLegStatuses(
      const std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& legStatuses);

  // Calculates the final status for itin, e.g.:
  //
  //                 status for a given fare path
  //                 \/
  // [A, O]       :  O <- calculated using the sequence priorities (IbfErrorMessageSequenceAcc)
  // [F, A]       :  A
  // [O, A]       :  O
  //               ------
  //                 A <- calculated using the choice priorities (IbfErrorMessageChoiceAcc)
  //
  static IbfErrorMessage
  calculateItinStatus(const std::vector<std::map<LegId, IbfErrorMessageSequenceAcc> >& legStatuses);

  // Returns the final status for an itin, as for calculateItinStatus
  // Fills up statusesPerLeg with the final leg statuses, as for summarizeLegStatuses
  static IbfErrorMessage
  calculateAllStatusesForMatrix(std::map<LegId, IbfErrorMessage>& legStatuses,
                                const std::vector<FareMarketPath*>& fareMarketPathMatrix);

  // Returns a set containing ids of legs
  // covered by the merged fare market.
  static std::set<unsigned int> getLegsForMfm(const MergedFareMarket& mfm);

  // Translates status to one of the valid values
  // being placed in the XML response.
  // Translation table:
  // IBF_EM_NO_FARES_FILED -> IBF_EM_NO_FARES_FOUND
  // IBF_EM_EARLY_DROP     -> IBF_EM_NO_FARE_FOUND
  // IBF_EM_NOT_OFFERED    -> IBF_EM_NOT_OFFERED
  // IBF_EM_NOT_AVAILABLE  -> IBF_EM_NOT_AVAILABLE
  // IBF_EM_NO_FARE_FOUND  -> IBF_EM_NO_FARE_FOUND
  // IBF_EM_NOT_SET        -> IBF_EM_NO_FARE_FOUND
  //
  // Basically, we want to transform IBF_EM_EARLY_DROP, IBF_EM_NO_FARES_FILED and
  // IBF_EM_NOT_SET into IBF_EM_NO_FARE_FOUND.
  // IBF_EM_EARLY_DROP indicates that a fare market was discarded before
  // the availability check due to a fail on some initial
  // fare validations. So this is similar reason to IBF_EM_NO_FARE_FOUND.
  // We treat IBF_EM_NOT_SET the same way (which means that there
  // were no calculations for a given fare market at all).
  static IbfErrorMessage translateForOutput(const IbfErrorMessage msg);

  // In this version of this function we also return NO_FARES_FILED status (X)
  // which, in the original one was translated into NO_FARES_FOUND (F)
  static IbfErrorMessage translateForOutput_newCbs(const IbfErrorMessage msg);

  // Returns the final status for an itin, as for calculateItinStatus
  // Fills up statusesPerLeg with the final leg statuses, as for summarizeLegStatuses
  // Used if fareMarketPathMatrix is unavailable (empty) - calculates status using
  // all possible fare paths but to speed up calculations it calculates
  // status for all travel segments using choice priorities and then leg statuses
  // using sequence priorities for travel segments on this leg.
  static IbfErrorMessage
  calculateAllStatusesForItinBrand(const Itin* itin, const BrandCode& brand,
    std::map<LegId, IbfErrorMessage>& legStatuses);

  // Updates travelSegStatus map on fare market travel segments its status for
  // given brand.
  static void
  calculateStatusesForFareMarketTravelSegments(const FareMarket* fm,
    const BrandCode& brand, std::map<const TravelSeg*, IbfErrorMessageChoiceAcc>& travelSegStatus);

  // Updates itins getMutableIbfAvailabilityTracker() for given brand (as total
  // and on each leg)
  static void
  updateIbfAvailabilitySoldoutsForItin(Itin* itin, const BrandCode& brandCode);
};

} // namespace tse
