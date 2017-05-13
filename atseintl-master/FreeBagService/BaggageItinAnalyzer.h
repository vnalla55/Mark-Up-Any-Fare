//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "DataModel/BaggageTravel.h"
#include "Diagnostic/DiagManager.h"

#include <vector>

#include <boost/container/flat_set.hpp>

#define UT_DATA_PATH "/vobs/atseintl/FreeBagService/test/testdata/"

namespace tse
{
class PricingTrx;
class Itin;
class TravelSeg;
class Loc;
class Mileage;
class Diag852Collector;

class BaggageItinAnalyzer
{
  friend class BaggageItinAnalyzerTest;

public:
  BaggageItinAnalyzer(PricingTrx& trx, Itin& itin);
  virtual ~BaggageItinAnalyzer() {}

  void analyzeAndSaveIntoFarePaths();
  void analyzeTravels();
  void displayDiagnostic() const;

  const CheckedPointVector& checkedPoints() const { return _checkedPoints; }
  const CheckedPoint& furthestCheckedPoint() const { return _furthestCheckedPoint; }
  const TravelSeg* furthestTicketedPoint() const;
  const std::vector<BaggageTravel*>& baggageTravels() const { return _baggageTravels; }

  const std::map<uint32_t, std::vector<BaggageTravel*>>& farePathIndex2baggageTravels() const
  {
    return _farePathIndex2baggageTravels;
  }

protected:
  using ApplicableGDs = boost::container::flat_set<GlobalDirection>;

  void determineCheckedPoints();
  void determineCheckedPointsForAirOnly();
  void finalizeCheckedPointDetermination();
  void removeMultiAirportConnectionCheckedPoints();
  bool isCheckedPoint(const Loc& loc) const;
  void determineFurthestTicketedPoint();

  GlobalDirection determineGlobalDir(const std::vector<TravelSeg*>& tsvec) const;
  ApplicableGDs determineGlobalDirs(TravelSegPtrVecCI beg,
                                    TravelSegPtrVecCI turnaround,
                                    TravelSegPtrVecCI end) const;
  uint32_t getMileage(const Loc& originLoc, const Loc& destinationLoc, const ApplicableGDs& gdirs) const;
  uint32_t getMileage(const LocCode& origin,
                      const LocCode& destination,
                      const ApplicableGDs& gdirs,
                      Indicator mileageType) const;

  void insertFurthestTicketedPointAsCheckedPoint();
  void determineFurthestCheckedPoint();

  void determineBaggageTravels();
  void determineBaggageTravelsForNonUsDot();
  void makeBaggageTravel(const CheckedPoint& current, const CheckedPoint& next);
  void addBaggageTravelForUsDot(TravelSegPtrVecCI segBegin, TravelSegPtrVecCI segEnd);
  void determineBaggageTravelsForUsDot();
  BaggageTravel* createBaggageTravel() const;
  void removeDummyBaggageTravels();
  void cloneBaggageTravelsForAllFarePaths();
  void setUsDot();
  void setBaggageTripType();
  void determineMss();
  void determineMssForNonUsDot();
  void determineMssForUsDot();
  TravelSegPtrVecCI
  determineMss(TravelSegPtrVecCI tvlSegIter, TravelSegPtrVecCI tvlSegEndIter) const;

  int64_t getStopOverLength() const;
  bool isWholeTravelInCAOrPanama(const std::vector<TravelSeg*>& travelSegs) const;
  bool isCAorPanama(const Loc* loc) const;
  bool isDestinationOnlyCheckedPoint() const;
  bool useFurthestTicketedPoint() const;
  bool isSameCity(const LocCode& airport1, const LocCode& airport2) const;
  bool isWithinMulticity(const CheckedPoint& cp1, const CheckedPoint& cp2) const;

  bool removeRetransitPointsFromBaggageTravels();
  bool retransitPointExist(const BaggageTravel& baggageTravel) const;
  void redefineBaggageTravel(std::vector<BaggageTravel*>& baggageTravels,
                             const BaggageTravel& baggageTravel) const;
  void printItinAnalysisResults() const;
  void printBaggageTravels() const;
  void printUsTariffCarriers() const;
  void printMileage(const LocCode& origin,
                    const LocCode& destination,
                    const std::vector<Mileage*>& mil,
                    const ApplicableGDs& gdirs,
                    Indicator mileageType) const;

  virtual void addToFarePath(BaggageTravel* baggageTravel) const;
  bool isStopoverWithCondition(bool (*condition) (const Loc& loc)) const;

protected:
  PricingTrx& _trx;
  Itin& _itin;
  int64_t _stopOverLength;
  bool _wholeTravelInUS;
  CheckedPointVector _checkedPoints;
  CheckedPoint _furthestCheckedPoint;
  TravelSegPtrVecCI _furthestTicketedPointIter;
  std::vector<BaggageTravel*> _baggageTravels;
  std::map<uint32_t, std::vector<BaggageTravel*>> _farePathIndex2baggageTravels;
  bool _retransitPointsExist = false;
  DiagManager _diagMgr;
  Diag852Collector* _diag852 = nullptr;
  std::vector<FarePath*>& _farePath;
};
} // tse
