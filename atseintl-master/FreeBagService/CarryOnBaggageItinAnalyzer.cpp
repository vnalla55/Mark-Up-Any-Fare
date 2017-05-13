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

#include "FreeBagService/CarryOnBaggageItinAnalyzer.h"

#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/BaggageItinAnalyzer.h"
#include "Xform/DataModelMap.h"

namespace tse
{

static Logger
logger("atseintl.FreeBagService.CarryOnBaggageItinAnalyzer");

CarryOnBaggageItinAnalyzer::CarryOnBaggageItinAnalyzer(PricingTrx& trx, Itin& itin)
  : BaggageItinAnalyzer(trx, itin)
{
}

void
CarryOnBaggageItinAnalyzer::analyze()
{
  LOG4CXX_DEBUG(logger, "Entering CarryOnBaggageItinAnalyzer::analyze()");
  createBaggageTravels();
  cloneBaggageTravelsForAllFarePaths();
  LOG4CXX_DEBUG(logger, "Leaving CarryOnBaggageItinAnalyzer::analyze()");
}

void
CarryOnBaggageItinAnalyzer::createBaggageTravels()
{
  LOG4CXX_DEBUG(logger, "Entering CarryOnBaggageItinAnalyzer::createBaggageTravels()");
  const TravelSegPtrVecCI travelSegBegin = _itin.travelSeg().begin();
  const TravelSegPtrVecCI travelSegEnd   = _itin.travelSeg().end();
  const TravelSegPtrVecCI mssJourney     = determineMss(travelSegBegin, travelSegEnd);

  TravelSegPtrVecCI travelSegIterator = travelSegBegin;

  for (; travelSegIterator != travelSegEnd; ++travelSegIterator)
  {
    if ((*travelSegIterator)->isAir() && (*travelSegIterator)->segmentType() != Open)
      _baggageTravels.push_back(createBaggageTravel(travelSegIterator, mssJourney));
  }
  LOG4CXX_DEBUG(logger, "Leaving CarryOnBaggageItinAnalyzer::createBaggageTravels()");
}

BaggageTravel*
CarryOnBaggageItinAnalyzer::createBaggageTravel(const TravelSegPtrVecCI& travelSeg,
                                                const TravelSegPtrVecCI& mssJourney)
    const
{
  LOG4CXX_DEBUG(logger, "Entering CarryOnBaggageItinAnalyzer::createBaggageTravel()");
  BaggageTravel* baggageTravel = nullptr;
  _trx.dataHandle().get(baggageTravel);

  if (baggageTravel)
  {
    baggageTravel->_trx = &_trx;
    baggageTravel->setupTravelData(*_itin.farePath().front());

    baggageTravel->updateSegmentsRange(travelSeg, travelSeg + 1);
    baggageTravel->_MSS = travelSeg;
    baggageTravel->_MSSJourney = mssJourney;
    baggageTravel->_stopOverLength = _stopOverLength;
  }
  LOG4CXX_DEBUG(logger, "Leaving CarryOnBaggageItinAnalyzer::createBaggageTravel()");
  return baggageTravel;
}

void
CarryOnBaggageItinAnalyzer::displayDiagnostic(bool processCarryOn, bool processEmbargo) const
{
  DiagManager diagMgr(_trx, Diagnostic852);

  if (diagMgr.isActive())
  {
    Diag852Collector& dc = dynamic_cast<Diag852Collector&>(diagMgr.collector());

    dc.printCarryOnBaggageHeader(processCarryOn, processEmbargo);
    dc.printCarryOnBaggageTravels(_itin.farePath().front()->baggageTravelsPerSector());
  }
}

void
CarryOnBaggageItinAnalyzer::addToFarePath(BaggageTravel* baggageTravel) const
{
  LOG4CXX_DEBUG(logger, "CarryOnBaggageItinAnalyzer::addToFarePath()");
  baggageTravel->farePath()->baggageTravelsPerSector().push_back(baggageTravel);
}

}
