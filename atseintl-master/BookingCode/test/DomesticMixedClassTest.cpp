#include <time.h>
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxType.h"
#include "DBAccess/DataHandle.h"
#include "BookingCode/DomesticMixedClass.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag411Collector.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DBAccess/CarrierMixedClass.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  CarrierMixedClassSeg* getSeg(Indicator hierarchy, BookingCode bkgcd)
  {
    CarrierMixedClassSeg* s = new CarrierMixedClassSeg();
    s->hierarchy() = hierarchy;
    s->bkgcd() = bkgcd;
    return s;
  }

public:
  const std::vector<CarrierMixedClass*>&
  getCarrierMixedClass(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "AA")
    {
      std::vector<CarrierMixedClass*>* ret = _memHandle.create<std::vector<CarrierMixedClass*> >();
      CarrierMixedClass* s = _memHandle.create<CarrierMixedClass>();
      s->segs().push_back(getSeg('2', "F"));
      s->segs().push_back(getSeg('4', "J"));
      s->segs().push_back(getSeg('6', "Y"));
      ret->push_back(s);
      return *ret;
    }
    return DataHandleMock::getCarrierMixedClass(carrier, date);
  }
};
}

class DomesticMixedClassTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DomesticMixedClassTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testvalidate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try
    {
      DomesticMixedClass domesticMixedClass;
      CPPUNIT_ASSERT(true);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testvalidate()
  {
    MyDataHandle mdh;
    FareMarket fareMarket;
    PaxTypeFare paxTypeFare;
    DomesticMixedClass domMClass;
    AirSeg airSeg;
    Fare mFare;
    FareInfo mFareInfo;
    PaxType paxType;
    TariffCrossRefInfo tariffCrossRefInfo;
    FareClassAppInfo mockFareClassAppInfo;
    FareClassAppSegInfo mockFareClassAppSegInfo;
    PricingTrx trx;
    PricingRequest req;

    Loc loc1;
    Loc loc2;

    Loc origin;
    Loc dest;

    // Travel / AirSeg
    TravelSegType segType = Air;
    airSeg.segmentType() = segType;
    airSeg.setBookingCode("Y");

    BookingCode bkg = "Y";

    airSeg.origin() = &origin;
    airSeg.destination() = &dest;

    FareClassCode fareClass = "Y26S";
    CarrierCode Carrier = "AA"; // The BookingCode objects carrier.

    airSeg.carrier() = Carrier;

    uint16_t number = 1;

    airSeg.segmentOrder() = number;
    airSeg.pnrSegment() = number;

    DateTime l_time = DateTime::localTime() + Hours(24);
    airSeg.departureDT() = l_time;

    fareMarket.travelSeg().push_back(&airSeg);

    // Intialize the faremarket object with any data you need
    origin.loc() = "DFW";
    dest.loc() = "ORD";

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;
    fareMarket.governingCarrier() = Carrier;

    // PaxTypeFare
    paxTypeFare.fareClassAppInfo() = &mockFareClassAppInfo;
    paxTypeFare.fareClassAppSegInfo() = &mockFareClassAppSegInfo;

    FareType fareType = "ER";
    PaxTypeCode paxT = "CHH";
    mockFareClassAppSegInfo._paxType = paxT;
    mockFareClassAppSegInfo._bookingCode[0] = "Y";
    mockFareClassAppInfo._fareType = fareType;

    paxTypeFare.initialize(&mFare, &paxType, &fareMarket);

    fareMarket.allPaxTypeFare().push_back(&paxTypeFare); // add paxtypefare to the faremarket

    //  Fare

    TariffNumber tariff = 10101;
    RuleNumber rule = "2020";

    mFareInfo._carrier = Carrier;

    mFareInfo._fareTariff = tariff;

    mFareInfo._ruleNumber = rule;

    mFare.setFareInfo(&mFareInfo);
    mFare.status() = Fare::FS_PublishedFare;

    mFareInfo._vendor = Vendor::ATPCO;

    mFare.initialize(Fare::FS_PublishedFare, &mFareInfo, fareMarket, &tariffCrossRefInfo);

    // TRX
    trx.fareMarket().push_back(&fareMarket);

    req.fareClassCode() = fareClass.c_str();
    trx.setRequest(&req);

    Diagnostic diagroot(Diagnostic411);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag411Collector diag(diagroot);

    diag.enable(Diagnostic411);
    CPPUNIT_ASSERT(diag.isActive());

    DataHandle dataHandle;

    CPPUNIT_ASSERT(domMClass.validate(trx, paxTypeFare, fareMarket, bkg, &diag));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DomesticMixedClassTest);

} // namespace tse
