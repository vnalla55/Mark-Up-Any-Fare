#pragma once

#include "test/include/MockTseServer.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestItinFactory.h"
#include "test/testdata/TestTravelSegFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestArunkSegFactory.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "Common/ClassOfService.h"
#include "Common/ShoppingUtil.h"

namespace tse
{

class ItinAnalyzerServiceDerived : public ItinAnalyzerService
{
  friend class ItinAnalyzerService_SegmentOrderWithoutArunkTest;
  friend class ItinAnalyzerService_FindCOSKeysTest;
  friend class ItinAnalyzerService_CheckItinTest;
  friend class ItinAnalyzerService_CheckItinFamilyTest;

public:
  ItinAnalyzerServiceDerived(TseServer& _svr) : ItinAnalyzerService("ITIN_SVC", _svr) {};
};

class ItinAnalyzerServiceTestShoppingCommon
{
public:
  /*
      static AirSeg* buildSegment( string origin, string destination, string carrier,
                                  DateTime depDT = DateTime::localTime(), uint32_t noOfDays = 0)
      {
        AirSeg* airSeg = memHandle.create<AirSeg>();

        airSeg->geoTravelType() = International;
        airSeg->origAirport() = origin;
        airSeg->origin() = memHandle.getLoc(airSeg->origAirport(), airSeg->departureDT());
        airSeg->destAirport() = destination;
        airSeg->departureDT() = depDT;
        airSeg->arrivalDT() = depDT.addDays(noOfDays);
        airSeg->destination() = memHandle.getLoc(airSeg->destAirport(), airSeg->arrivalDT());
        airSeg->boardMultiCity() = origin;
        airSeg->offMultiCity() = destination;
        airSeg->carrier() = carrier;

        return airSeg;
      }
  */
  static AirSeg* buildSegment(std::string filename)
  {
    return TestAirSegFactory::create(filename, true);
  }

  static ArunkSeg* buildArunkSegment(std::string filename)
  {
    return TestArunkSegFactory::create(filename, true);
  }

  static PricingTrx* createTrx(std::string filename)
  {
    return TestPricingTrxFactory::create(filename, true);
  }

  static Itin* buildItin(std::string filename) { return TestItinFactory::create(filename, true); }

  static void setupClassOfService(ClassOfService& cos, BookingCode bkc, uint16_t no)
  {
    cos.bookingCode() = bkc;
    cos.numSeats() = no;
    cos.cabin().setEconomyClass();
  }

  static void
  addAvailabilityMapItem(PricingTrx& trx, TravelSeg* seg, ClassOfServiceList& cosList)
  {
    PricingTrx::ClassOfServiceKey key;
    key.push_back(seg);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    cosListVec->push_back(cosList);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(key)] = cosListVec;
  }
};
}
