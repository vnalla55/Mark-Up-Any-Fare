//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


#include <vector>

namespace tse
{

/** @class FlightTracker
 * This class implements the flight tracking requirement of a FareMarket for it's GoveringCarrier.
 * A vector of carriers requires to consider hidden points while building
 * route of travel and also for applying rules. These carriers are listed
 * in FltTrkCntryGrp table. Flight tracking always apply for governing carrier.
 *
 * */

class TravelSeg;
class FareMarket;
class FltTrkCntryGrp;
class PricingTrx;

class FlightTracker
{

public:
  friend class FlightTrackerTest;

  FlightTracker(const PricingTrx& trx);

  virtual ~FlightTracker();

  /**
   * Main method to process flight traking validation proecess.
   * If the method returns tue, then it sets the isFltTrackingIndicator
   * flag in the FareMarket.
   * @param fareMarket  A reference to the FareMarket object.
   * @return bool true if flight tracking applies.
   *
   */

  bool process(FareMarket& fareMarket);

  /**
   * Public interface to call FlightTracker to apply flight tracking logic.
   * For secondary sector validation is required this method can be called
   * from anywhere with a vector of travel segments and goverining carrier
   * to validate flight tracking logic.
   * @param tvlSegs A vector of pointers of travel segments.
   * @param govCxr  govering carrier.
   * @returns ture - if it passes the flight tracking requirements.
   *
   * */

  bool getFltTrackingInfo(const std::vector<TravelSeg*>& tvlSegs, const CarrierCode govCxr);

private:
  /**
    * This will work as a utility function for flight tracking validation.
    * @param NationCode  A reference to the nation of origin.
    * @param NationCode  A reference to the nation of destination.
    * @param CarrierCode  A reference to the gov carrier of the FareMarket or TravelSegs.
    * */

  bool validateFltTracking(const NationCode& origin,
                           const NationCode& destination,
                           const CarrierCode& carrier,
                           const std::vector<TravelSeg*>& tvlSegs);

  /**
   * DBAccess method to retrieve Flight Tracking data.
   * First the govering carrier is matched in the FltTrackCntryGrp table.
   * If a match is found, FltTrkCntrySeg table is accessed and corresponding
   * vector of country is retrieved.
   * @param CarrierCode GoveringCarrier.
   * @param tvlSegs A vector of travel segments.
   * vector of nations for which the carrier requires flight tracking.
   *
   * */
  virtual const FltTrkCntryGrp*
  getData(const CarrierCode& carrier, const std::vector<TravelSeg*>& tvlSegs);
  /**
   * Utility function to retrieve Date.
   * */

  virtual DateTime getDate(const std::vector<TravelSeg*>& tvlSegs);
  const PricingTrx& _trx;
  const FareMarket* _fareMarket;
};
}
