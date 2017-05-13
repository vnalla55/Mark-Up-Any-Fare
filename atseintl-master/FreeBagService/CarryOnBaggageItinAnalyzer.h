//-------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include "FreeBagService/BaggageItinAnalyzer.h"


#include <vector>

namespace tse
{

class PricingTrx;
class Itin;
class TravelSeg;
class BaggageTravel;

class CarryOnBaggageItinAnalyzer : public BaggageItinAnalyzer
{
  friend class CarryOnBaggageItinAnalyzerTest;

public:
  CarryOnBaggageItinAnalyzer(PricingTrx& trx, Itin& itin);

  void analyze();
  void displayDiagnostic(bool processCarryOn, bool processEmbargo) const;

private:
  void createBaggageTravels();
  BaggageTravel* createBaggageTravel(const TravelSegPtrVecCI& travelSeg,
                                     const TravelSegPtrVecCI& mssJourney) const;
  virtual void addToFarePath(BaggageTravel* baggageTravel) const override;
};

} // tse

