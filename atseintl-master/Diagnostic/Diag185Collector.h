//----------------------------------------------------------------------------
//  File:        Diag185Collector.h
//  Description: Diagnostic 185 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004
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

#include "DataModel/PaxType.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;
class TravelSeg;

class Diag185Collector : public DiagCollector
{
public:

  friend class Diag185CollectorTest;

  explicit Diag185Collector(Diagnostic& root) : DiagCollector(root) {}
    Diag185Collector() {}

  void printHeader() const;
  void printGcmSelectionHeader() const;
  void printSelectedFmNotFound() const;
  void printRTWSelectionHeader() const;
  void startDiag185(const PricingTrx& trx) const;
  void printSegmentSelectedHeader() const;
  void printSelectedSegments(const std::vector<TravelSeg*>& tvlSegs);
  void printProcessSelectedFareMarkets() const;
  void printFareMarket(const PricingTrx& trx, FareMarket& fm);
  void printInvalidFareMarket();
  void displayChangesDiag185(const ClassOfService* cos, const TravelSeg* tvlSeg, bool avType);
  void printNoChanges(const TravelSeg* tvlSeg);
  void printSkippedSegment(const TravelSeg* tvlSeg, CabinType reqCabin);
  void printSegmentGcm(const LocCode& origin, const TravelSeg& tvlSeg, const uint32_t gcm) const;
  void printNoMarketsSelected() const;
  void printNoOneSegmentInCabinRequested() const;
  void printNoRBDfoundForSecondarySegment(const TravelSeg* tvlSeg, const FareMarket& fm);
  void printFurthestPoint(const TravelSeg& tvlSeg, const uint32_t highestGcm);
  void printFurthestPointNotSet() const;
  void printGcmNotCalculated() const;
  void printInvalidFM(const FareMarket& fm, bool primary=true );
  void finishDiag185(const PricingTrx& trx);
  virtual std::vector<ClassOfService*>*
                            getJourneyAvailability(const TravelSeg* travelSeg, Itin* itin);
private:

  void addFareMarket(const PricingTrx& trx, const FareMarket& fareMarket);
  void addTravelSegment(const TravelSeg* tvlSeg, bool shift=false);
  void addCOS(const std::vector<ClassOfService*>* cosVec);
  void displayChangedInfo(const ClassOfService* cos, bool avType) const;
  void displayChangedHeader() const;
  void displayAllTravelSeg(const PricingTrx& trx);
  bool isItinOutsideEurope(const Itin& itin);
  bool isItinOutsideNetherlandAntilles(const Itin& itin);
};

} // namespace tse

