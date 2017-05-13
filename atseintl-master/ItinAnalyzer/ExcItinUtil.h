//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ExcItin.h"

namespace tse
{
class RexNewItin;
class RexPricingTrx;

class ExcItinUtil
{
  friend class ExcItinUtilTest;

public:
  static void DetermineChanges(ExcItin* excItin, Itin* newItin);
  static void IsStopOverChange(ExcItin* exchangeItin, Itin* newItin);
  static void matchCoupon(ExcItin* exchangeItin, Itin* newItin);
  static void SetFareMarketChangeStatus(Itin* itinFirst, const Itin* itinSecond);
  static void CheckSegmentsStatus(ExcItin* excItin, Itin* newItin);
  static void calculateMaxPriceForFlownOrNotShopped(RexPricingTrx& trx, Itin& itin);

protected:
  typedef std::vector<TravelSeg*>::iterator TravelSegIterator;
  typedef std::vector<TravelSeg*>::const_iterator TravelSegConstIterator;
  typedef std::pair<TravelSegConstIterator, TravelSegConstIterator> TvlSegPair;
  typedef std::vector<std::pair<TvlSegPair, TvlSegPair> > TvlSegPairV;

  static bool isChanged(const std::vector<TravelSeg*>& travelSegs,
                        const TravelSeg::ChangeStatus status);

  static void setUnchangedStatus(TravelSeg& excTvlSeg,
                                 TravelSeg& newTvlSeg,
                                 RexNewItin* rexNewItin);

  static void markUnchangedSegmentsOnly(ExcItin* excItin, Itin* newItin, TvlSegPairV& tvlSegPairs);

private:
  ExcItinUtil();

  static void findStopOvers(const Itin* itin, std::vector<LocCode>& stopOvers);
  static int findPointOfChange(const Itin* itin);
};

} // tse

