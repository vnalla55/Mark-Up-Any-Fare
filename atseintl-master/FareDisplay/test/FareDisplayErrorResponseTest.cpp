#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "FareDisplay/FareDisplayErrorResponse.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const DateTime& date)
  {
    if (market1 == "DFW" && market2 == "LON" && cxr == "BA")
    {
      std::vector<const FareInfo*>* ret = _memHandle.create<std::vector<const FareInfo*> >();
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->globalDirection() = GlobalDirection::AT;
      ret->push_back(fi);
      return *ret;
    }
    else if (market1 == "SYD" && market2 == "SYD" && cxr == "BA")
    {
      std::vector<const FareInfo*>* ret = _memHandle.create<std::vector<const FareInfo*> >();
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->globalDirection() = GlobalDirection::CT;
      ret->push_back(fi);
      fi = _memHandle.create<FareInfo>();
      fi->globalDirection() = GlobalDirection::RW;
      ret->push_back(fi);
      return *ret;
    }

    return DataHandleMock::getFaresByMarketCxr(market1, market2, cxr, date);
  }
};
}
class FareDisplayErrorResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayErrorResponseTest);
  CPPUNIT_TEST(testErrorResponse);
  CPPUNIT_TEST(testErrorResponseWithGlobalOverride);
  CPPUNIT_TEST(testErrorResponseSYDSYD);
  CPPUNIT_TEST_SUITE_END();

public:
  void testErrorResponse()
  {
    FareDisplayTrx trx;
    // Create The Trx
    FareDisplayRequest request;
    FareDisplayOptions options;

    trx.setRequest(&request);
    trx.getRequest()->salePointOverride() = "";
    request.ticketingDT() = DateTime::localTime();

    trx.setOptions(&options);
    options.currencyOverride() = "EUR";
    options.alternateCurrency() = EMPTY_STRING();
    options.templateOverride() = 99;
    options.privateFares() = 'T';

    Loc origin;
    Loc destination;
    LocCode orig = "DFW";
    LocCode dest = "LON";

    origin.loc() = orig;
    destination.loc() = dest;

    trx.preferredCarriers().insert("AA");

    Itin itin;
    FareMarket fm;
    fm.origin() = &origin;
    fm.destination() = &destination;

    trx.setTravelDate(DateTime::localTime());
    trx.itin().push_back(&itin);
    itin.fareMarket().push_back(&fm);
    trx.fareMarket().push_back(&fm);

    PaxTypeFare ptf;
    trx.allPaxTypeFare().push_back(&ptf);

    AirSeg aS;
    aS.departureDT() = DateTime::localTime();
    aS.carrier() = "BA";

    aS.origin() = &origin;
    aS.destination() = &destination;
    aS.boardMultiCity() = orig;
    aS.offMultiCity() = dest;
    fm.setGlobalDirection(GlobalDirection::ZZ);
    fm.geoTravelType() = GeoTravelType::International;
    itin.geoTravelType() = GeoTravelType::International;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = NUC;

    itin.travelSeg().push_back(&aS);
    trx.travelSeg().push_back(&aS);
    fm.travelSeg().push_back(&aS);
    fm.governingCarrier() = "BA";
    fm.boardMultiCity() = "DFW";
    fm.offMultiCity() = "LON";
    fm.direction() = FMDirection::OUTBOUND;

    FareDisplayErrorResponse errResponse(trx);
    std::string res = trx.response().str();
    std::string err = trx.errorResponse().str();

    CPPUNIT_ASSERT(res.empty());
    CPPUNIT_ASSERT(err.empty());
    errResponse.process();

    std::string res2 = trx.response().str();
    std::string err2 = trx.errorResponse().str();

    CPPUNIT_ASSERT(res2.empty());
    CPPUNIT_ASSERT(!err2.empty());
  }

  void testErrorResponseWithGlobalOverride()
  {
    MyDataHandle mdh;
    FareDisplayTrx trx;
    // Create The Trx
    FareDisplayRequest request;
    FareDisplayOptions options;

    trx.setRequest(&request);
    trx.getRequest()->salePointOverride() = "";
    request.ticketingDT() = DateTime::localTime();

    trx.setOptions(&options);
    options.currencyOverride() = "EUR";
    options.alternateCurrency() = EMPTY_STRING();
    options.templateOverride() = 99;

    Loc origin;
    Loc destination;
    LocCode orig = "DFW";
    LocCode dest = "LON";

    origin.loc() = orig;
    destination.loc() = dest;

    trx.preferredCarriers().insert("AA");

    Itin itin;
    FareMarket fm;
    fm.origin() = &origin;
    fm.destination() = &destination;

    trx.setTravelDate(DateTime::localTime());
    trx.itin().push_back(&itin);
    itin.fareMarket().push_back(&fm);
    trx.fareMarket().push_back(&fm);

    AirSeg aS;
    aS.departureDT() = DateTime::localTime();
    aS.carrier() = "BA";

    aS.origin() = &origin;
    aS.destination() = &destination;
    aS.boardMultiCity() = orig;
    aS.offMultiCity() = dest;
    fm.setGlobalDirection(GlobalDirection::ZZ);
    fm.geoTravelType() = GeoTravelType::International;
    itin.geoTravelType() = GeoTravelType::International;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = NUC;

    itin.travelSeg().push_back(&aS);
    trx.travelSeg().push_back(&aS);
    fm.travelSeg().push_back(&aS);
    fm.governingCarrier() = "BA";
    fm.boardMultiCity() = "DFW";
    fm.offMultiCity() = "LON";
    fm.direction() = FMDirection::OUTBOUND;

    FareDisplayErrorResponse errResponse(trx);
    std::string res = trx.response().str();
    std::string err = trx.errorResponse().str();

    CPPUNIT_ASSERT(res.empty() == true);
    CPPUNIT_ASSERT(err.empty() == true);
    request.globalDirection() = GlobalDirection::WH;
    errResponse.process();

    std::string res2 = trx.response().str();
    std::string err2 = trx.errorResponse().str();

    CPPUNIT_ASSERT(res2.empty() == true);
    CPPUNIT_ASSERT(err2.empty() == false);
    CPPUNIT_ASSERT(err2 == " GLOBAL INDICATOR WH NOT AVAILABLE - TRY AT");
  }

  void testErrorResponseSYDSYD()
  {
    MyDataHandle mdh;
    FareDisplayTrx trx;
    // Create The Trx
    FareDisplayRequest request;
    FareDisplayOptions options;

    trx.setRequest(&request);
    trx.getRequest()->salePointOverride() = "";
    request.ticketingDT() = DateTime::localTime();

    trx.setOptions(&options);
    options.currencyOverride() = "EUR";
    options.alternateCurrency() = EMPTY_STRING();
    options.templateOverride() = 99;

    Loc origin;
    Loc destination;
    LocCode orig = "SYD";
    LocCode dest = "SYD";

    origin.loc() = orig;
    destination.loc() = dest;

    trx.preferredCarriers().insert("MH");

    Itin itin;
    FareMarket fm;
    fm.origin() = &origin;
    fm.destination() = &destination;

    trx.setTravelDate(DateTime::localTime());
    trx.itin().push_back(&itin);
    itin.fareMarket().push_back(&fm);
    trx.fareMarket().push_back(&fm);

    AirSeg aS;
    aS.departureDT() = DateTime::localTime();
    aS.carrier() = "MH";

    aS.origin() = &origin;
    aS.destination() = &destination;
    aS.boardMultiCity() = orig;
    aS.offMultiCity() = dest;
    fm.setGlobalDirection(GlobalDirection::ZZ);
    fm.geoTravelType() = GeoTravelType::International;
    itin.geoTravelType() = GeoTravelType::International;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = NUC;

    itin.travelSeg().push_back(&aS);
    trx.travelSeg().push_back(&aS);
    fm.travelSeg().push_back(&aS);
    fm.governingCarrier() = "BA";
    fm.boardMultiCity() = "SYD";
    fm.offMultiCity() = "SYD";
    fm.direction() = FMDirection::OUTBOUND;

    FareDisplayErrorResponse errResponse(trx);
    std::string res = trx.response().str();
    std::string err = trx.errorResponse().str();

    CPPUNIT_ASSERT(res.empty() == true);
    CPPUNIT_ASSERT(err.empty() == true);
    request.globalDirection() = GlobalDirection::PA;
    errResponse.process();

    std::string res2 = trx.response().str();
    std::string err2 = trx.errorResponse().str();
    CPPUNIT_ASSERT(res2.empty() == true);
    CPPUNIT_ASSERT(err2.empty() == false);
    CPPUNIT_ASSERT_EQUAL((std::string) " GLOBAL INDICATOR PA NOT AVAILABLE - TRY CT RW", err2);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayErrorResponseTest);
}
