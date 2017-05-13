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

#include "Fares/RoutingController.h"


namespace tse
{

/** @class SpecialRouting
 * Populates FareMarket with MPM and travelSegments with TPM.
 * FareCollectorOrchestrator calls this per FareMarket.
 */
class PaxTypeFare;
class PricingTrx;
class RtgKey;
class SpecialRouting
{

public:
  friend class SpecialRoutingTest;
  SpecialRouting();
  virtual ~SpecialRouting();

  bool validate(PricingTrx& trx,
                std::vector<PaxTypeFare*>&,
                RoutingInfos&,
                const TravelRoute& tvlRoute) const;

protected:
  bool matchKey(PricingTrx& trx, PaxTypeFare& ptFare, const RtgKey& rKey) const;
  bool processEmptyRouting(PricingTrx&, PaxTypeFare&) const;
  void updateFBRPaxTypewithRouting(PricingTrx& trx, PaxTypeFare&, RoutingInfos::iterator&) const;
  void updateFBRPaxTypewithMileage(PricingTrx& trx, PaxTypeFare&, RoutingInfos::iterator&) const;
  virtual void validateMileage(PricingTrx& trx, PaxTypeFare&, const TravelRoute& tvlRoute) const;
  bool isSMF(PricingTrx& trx, const PaxTypeFare& fare) const;

private:
  static constexpr char SMF_VENDOR = 'T';
};
}
