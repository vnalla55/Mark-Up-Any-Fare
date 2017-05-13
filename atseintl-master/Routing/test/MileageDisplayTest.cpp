#include "test/include/CppUnitHelperMacros.h"
#include <vector>
#include "Common/DateTime.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TpdPsrViaGeoLoc.h"
#include "DataModel/Agent.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingOptions.h"
#include "Routing/MileageService.h"
#include "Routing/MileageDisplay.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageRoute.h"
#include "DataModel/AirSeg.h"
#include "Routing/TravelRoute.h"
#include <boost/tokenizer.hpp>
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <boost/assign/std/vector.hpp>
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/MultiTransport.h"

using namespace boost::assign;
using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  MultiTransport* getMC(LocCode city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitranscity() = city;
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "ARN")
      return "STO";
    else if (locCode == "CPH")
      return "CPH";
    else if (locCode == "OSL")
      return "OSL";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
    if (locCode == "ARN")
    {
      ret += getMC("STO", "ARN");
      return ret;
    }
    else if (locCode == "CPH")
    {
      ret += getMC("CPH", "CPH");
      return ret;
    }
    else if (locCode == "OSL")
    {
      ret += getMC("OSL", "OSL");
      return ret;
    }

    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
};
}
class MileageDisplayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageDisplayTest);
  CPPUNIT_TEST(testPSRDisplay);

  CPPUNIT_TEST(testDisplayMileageRequest);
  CPPUNIT_TEST(testDisplayMileageRequest_Stopover);

  CPPUNIT_TEST(testDisplayEqualization_NoOrigSurcharge);
  CPPUNIT_TEST(testDisplayEqualization_OrigSurcharge);
  CPPUNIT_TEST(testDisplayEqualization_DestSurcharge);

  CPPUNIT_TEST(testDisplayPSRMayApply_FirstStopover);
  CPPUNIT_TEST(testDisplayPSRMayApply_OtherStopover);

  CPPUNIT_TEST(testDisplaySurfaceSector);
  CPPUNIT_TEST(testDisplayConditionalTPDs);
  CPPUNIT_TEST(testDisplayMileageRoute);
  CPPUNIT_TEST(testDisplayMileageRouteItem);
  CPPUNIT_TEST(testDisplaySurchargeInfo);
  CPPUNIT_TEST(testDisplayEMS);
  CPPUNIT_TEST(testDisplayHeader);
  CPPUNIT_TEST(testIsFailedDirServiceReturnTrueWhenOneItemFailed);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _pricingTrx->setRequest(_request);
    _mDisplay = _memHandle.insert(new MileageDisplay(_trx));
    initTrxData();
  }

  void tearDown()
  {
    _dataHandle.clear();
    _memHandle.clear();
  }

  void initTrxData()
  {
    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using _dataHandle to get these out of the database.

    _trx.setRequest(_memHandle.create<PricingRequest>());
    _trx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    _trx.inputDT() = DateTime::localTime();

    _loc1 = _dataHandle.getLoc("ARN", DateTime::localTime());
    _loc2 = _dataHandle.getLoc("CPH", DateTime::localTime());
    _loc3 = _dataHandle.getLoc("OSL", DateTime::localTime());
    _loc4 = _dataHandle.getLoc("SVO", DateTime::localTime());

    _as1.origin() = const_cast<Loc*>(_loc1);
    _as1.destination() = const_cast<Loc*>(_loc2);
    _as1.boardMultiCity() = "ARN";
    _as1.offMultiCity() = "CPH";
    _as1.carrier() = "SK";

    _as2.origin() = const_cast<Loc*>(_loc2);
    _as2.destination() = const_cast<Loc*>(_loc3);
    _as2.boardMultiCity() = "CPH";
    _as2.offMultiCity() = "OSL";
    _as2.carrier() = "SK";

    _as3.origin() = const_cast<Loc*>(_loc3);
    _as3.destination() = const_cast<Loc*>(_loc4);
    _as3.boardMultiCity() = "OSL";
    _as3.offMultiCity() = "SVO";
    _as3.carrier() = "SK";

    _tvlSegs.clear();
    _tvlSegs.push_back(&_as1);
    _tvlSegs.push_back(&_as2);
    _tvlSegs.push_back(&_as3);
    _tvlRoute.mileageTravelRoute() = _tvlSegs;

    _tvlRoute.globalDir() = GlobalDirection::EH;
    _tvlRoute.govCxr() = "SK";
    _tvlRoute.travelDate() = DateTime::localTime();

    _ticketingDT = DateTime::localTime();

    MileageRouteBuilder mBuilder;
    bool rc =
        mBuilder.buildMileageRoute(*_pricingTrx, _tvlRoute, _mRoute, _dataHandle, _ticketingDT);
    CPPUNIT_ASSERT(rc);

    // Set stopover indicators in mileageRoute
    _mRoute.mileageRouteItems()[0].psrMayApply() = true;
    _mRoute.mileageRouteItems()[1].psrMayApply() = true;
    _mRoute.mileageRouteItems()[0].isStopover() = true;
    _mRoute.mileageRouteItems()[1].isStopover() = true;

    _mileageItems[0].cityLoc = _loc1;
    _mileageItems[1].cityLoc = _loc2;
    _mileageItems[2].cityLoc = _loc3;

    for (int i = 0; i < 3; ++i)
    {
      _mileageItems[i].stopType = StopType::Connection;
      _mileageItems[i].isHidden = false;
      _mileageItems[i].isSurface = false;
    }

    _trx.items().push_back(&_mileageItems[0]);
    _trx.items().push_back(&_mileageItems[1]);
    _trx.items().push_back(&_mileageItems[2]);

    _trx.response().str("");
  }

  // tests

  void testPSRDisplay()
  {
    CPPUNIT_ASSERT(_mRoute.mileageRouteItems().size() == 3);

    string expected = " *  SPECIFIED ROUTING APPLIES WHEN NO STOPOVER AT CPH\n"
                      "    AND/OR OSL\n";

    CPPUNIT_ASSERT(_mDisplay->displayPSRMayApply(_mRoute));
    CPPUNIT_ASSERT_EQUAL(expected, _trx.response().str());
  }

  void testDisplayMileageRequest()
  {
    _mDisplay->displayMileageRequest(_trx);

    CPPUNIT_ASSERT(_trx.response().str().find("X/ARN"));
    CPPUNIT_ASSERT(_trx.response().str().find("X/CPH"));
    CPPUNIT_ASSERT(_trx.response().str().find("X/OSL"));
  }

  void testDisplayMileageRequest_Stopover()
  {
    _mileageItems[0].stopType = StopType::StopOver;
    _mileageItems[0].isSurface = true;
    _mileageItems[1].stopType = StopType::UnknownStopType;
    _mileageItems[1].isHidden = true;
    _mileageItems[2].stopType = StopType::StopOver;

    _mDisplay->displayMileageRequest(_trx);

    // no connection indicators shown
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("X/"), string::npos);
    // don't show hidden city
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("CPH"), string::npos);
    // arunk segment indicator ( '//'  ) shown after ARN (STO)
    CPPUNIT_ASSERT(_trx.response().str().find("//") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("STO") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("//") > _trx.response().str().find("STO"));
  }

  void testDisplayEqualization_NoOrigSurcharge()
  {
    _mDisplay->displayEqualization(_mRoute);

    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("RIO MILEAGE SURCHARGE"), string::npos);
    // do NOT count 'NEED NOT EXCEED SAO MILEAGE SURCHARGE"
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("\nSAO MILEAGE SURCHARGE"), string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("SAO MILEAGE SURCHARGE") != 0);
  }

  void testDisplayEqualization_OrigSurcharge()
  {
    _mRoute.mileageRouteItems().front().origCityOrAirport()->loc() = RIO_DE_JANEIRO;

    _mDisplay->displayEqualization(_mRoute);

    CPPUNIT_ASSERT(_trx.response().str().find("RIO MILEAGE SURCHARGE") != string::npos);
  }

  void testDisplayEqualization_DestSurcharge()
  {
    _mRoute.mileageRouteItems().front().origCityOrAirport()->loc() = "WAW";
    _mRoute.mileageRouteItems().back().destCityOrAirport()->loc() = SAO_PAULO;

    _mDisplay->displayEqualization(_mRoute);

    // do NOT count 'NEED NOT EXCEED SAO MILEAGE SURCHARGE'
    std::size_t found_str = _trx.response().str().find("SAO MILEAGE SURCHARGE");
    std::size_t found_str_nne = _trx.response().str().find("NEED NOT EXCEED SAO MILEAGE SURCHARGE");
    CPPUNIT_ASSERT(found_str != string::npos);
    CPPUNIT_ASSERT(found_str != found_str_nne);
  }

  void testDisplayPSRMayApply_FirstStopover()
  {
    _mRoute.mileageRouteItems()[0].isStopover() = true;
    _mRoute.mileageRouteItems()[1].isStopover() = false;
    _mRoute.mileageRouteItems()[2].isStopover() = false;
    std::string firstDestCity = _mRoute.mileageRouteItems().front().city2()->loc();

    _mDisplay->displayPSRMayApply(_mRoute);

    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find(" AND/OR " + firstDestCity), string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find(firstDestCity) != string::npos);
  }

  void testDisplayPSRMayApply_OtherStopover()
  {
    _mRoute.mileageRouteItems()[0].isStopover() = true;
    _mRoute.mileageRouteItems()[1].isStopover() = true;
    _mRoute.mileageRouteItems()[2].isStopover() = false;
    std::string secondDestCity = _mRoute.mileageRouteItems()[1].city2()->loc();

    _mDisplay->displayPSRMayApply(_mRoute);

    CPPUNIT_ASSERT(_trx.response().str().find(" AND/OR " + secondDestCity) != string::npos);
  }

  void testDisplaySurfaceSector()
  {
    MileageDisplay mDisplay(_trx);
    LocCode* krk = _memHandle.insert(new LocCode("KRK"));
    LocCode* waw = _memHandle.insert(new LocCode("WAW"));

    std::vector<std::pair<const LocCode*, const LocCode*> > surfaceSectors;
    _trx.response().str("");
    mDisplay.displaySurfaceSector(surfaceSectors);
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("KRK - WAW"), string::npos);

    surfaceSectors.push_back(std::make_pair(krk, waw));
    _trx.response().str("");
    mDisplay.displaySurfaceSector(surfaceSectors);
    CPPUNIT_ASSERT(_trx.response().str().find("KRK - WAW") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("SURFACE") != string::npos);
  }

  void testDisplayConditionalTPDs()
  {
    MileageDisplay mDisplay(_trx);
    std::vector<const TpdPsrViaGeoLoc*> vec1, vec2;

    std::vector<const std::vector<const TpdPsrViaGeoLoc*>*> conditionalTPDs;
    conditionalTPDs.resize(1);
    conditionalTPDs[0] = &vec1;

    TpdPsrViaGeoLoc geoloc_AND;
    geoloc_AND.relationalInd() = VIAGEOLOCREL_AND;
    geoloc_AND.loc().loc() = "AAA";

    TpdPsrViaGeoLoc geoloc_ANDOR;
    geoloc_ANDOR.relationalInd() = VIAGEOLOCREL_ANDOR;
    geoloc_ANDOR.loc().loc() = "BBB";

    TpdPsrViaGeoLoc geoloc_OR;
    geoloc_OR.relationalInd() = VIAGEOLOCREL_OR;
    geoloc_OR.loc().loc() = "CCC";

    // no elements
    vec1.clear();
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT(true); // just assert that crash didn't happen

    // 1 element
    vec1.clear();
    vec1.push_back(&geoloc_AND);
    _trx.response().str("");
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT(_trx.response().str().find("AAA") != string::npos);
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("AND AAA"), string::npos);

    // 2 elements with AND
    vec1.clear();
    vec1.push_back(&geoloc_AND);
    vec1.push_back(&geoloc_AND);
    _trx.response().str("");
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT(_trx.response().str().find("AND AAA") != string::npos);

    // 2 elements with AND/OR
    vec1.clear();
    vec1.push_back(&geoloc_ANDOR);
    vec1.push_back(&geoloc_ANDOR);
    _trx.response().str("");
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT(_trx.response().str().find("AND/OR BBB") != string::npos);

    // 2 elements with OR
    vec1.clear();
    vec1.push_back(&geoloc_OR);
    vec1.push_back(&geoloc_OR);
    _trx.response().str("");
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT(_trx.response().str().find("OR CCC") != string::npos);

    // mixed elements
    conditionalTPDs.resize(2);
    conditionalTPDs[1] = &vec2;

    vec1.clear();
    vec2.clear();

    vec1.push_back(&geoloc_ANDOR);
    vec1.push_back(&geoloc_OR);

    vec2.push_back(&geoloc_ANDOR);
    vec2.push_back(&geoloc_AND);

    _trx.response().str("");
    mDisplay.displayConditionalTPDs(conditionalTPDs);
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("AND/OR BBB"), string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("BBB") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("OR CCC") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("AND AAA") != string::npos);
  }

  void testDisplayMileageRoute()
  {
    MileageDisplay mDisplay(_trx);

    CPPUNIT_ASSERT(mDisplay.displayMileageRoute(_mRoute));

    _trx.response().str("");
    _mRoute.mileageRouteItems().resize(0);
    CPPUNIT_ASSERT(mDisplay.displayMileageRoute(_mRoute));
    CPPUNIT_ASSERT(_trx.response().str().size() == 0);
  }

  void testDisplayMileageRouteItem()
  {
    MileageDisplay mDisplay(_trx);

    _mRoute.mileageRouteItems()[0].mpm() = 500;
    _mRoute.mileageRouteItems()[0].tpm() = 500;
    _mRoute.mileageRouteItems()[1].mpm() = 500;
    _mRoute.mileageRouteItems()[1].tpm() = 500;
    _mRoute.mileageRouteItems()[1].mpmGlobalDirection() = GlobalDirection::PA;
    _mRoute.mileageRouteItems()[1].isStopover() = false;
    _mRoute.mileageRouteItems()[1].southAtlanticExclusion() = false;
    _mRoute.mileageRouteItems()[1].isConstructed() = false;

    _trx.response().str("");
    CPPUNIT_ASSERT(mDisplay.displayMileageRouteItem(
        _mRoute, _mRoute.mileageRouteItems()[1], 1000, 2, false, false));

    CPPUNIT_ASSERT(_trx.response().str().find("XOSL 2 PA") != string::npos);
    CPPUNIT_ASSERT_EQUAL(_trx.response().str().find("@"), string::npos);

    _mRoute.mileageRouteItems()[1].isStopover() = true;
    _mRoute.mileageRouteItems()[1].southAtlanticExclusion() = true;
    _mRoute.mileageRouteItems()[1].isConstructed() = true;

    _trx.response().str("");
    CPPUNIT_ASSERT(mDisplay.displayMileageRouteItem(
        _mRoute, _mRoute.mileageRouteItems()[1], 1000, 2, false, false));

    CPPUNIT_ASSERT(_trx.response().str().find(" OSL*2 PA") != string::npos);
    CPPUNIT_ASSERT(_trx.response().str().find("@") != string::npos);
  }

  void testDisplaySurchargeInfo()
  {
    MileageDisplay mDisplay(_trx);
    MileageRouteItem& mItem = _mRoute.mileageRouteItems()[0];
    mItem.mpm() = 1000;

    _trx.response().str("");
    CPPUNIT_ASSERT(mDisplay.displaySurchargeInfo(mItem, 1170, 20));

    std::string res(_trx.response().str());

    boost::tokenizer<> tok(res);
    std::string expected[] = { "20", "30", "1250" };

    int i = 0;
    for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it, ++i)
      CPPUNIT_ASSERT_MESSAGE(std::string("EXPECTED ") + expected[i], *it == expected[i]);
  }

  void testDisplayEMS()
  {
    MileageDisplay mDisplay(_trx);
    MileageRouteItem& mItem = _mRoute.mileageRouteItems()[0];
    mItem.mpm() = 1000;

    uint16_t res;

    mDisplay.displayEMS(mItem, 800, res); // expect 0
    CPPUNIT_ASSERT(res == 0);

    mDisplay.displayEMS(mItem, 1000, res); // expect 0
    CPPUNIT_ASSERT(res == 0);

    mDisplay.displayEMS(mItem, 1070, res); // expect 10
    CPPUNIT_ASSERT(res == 10);

    mDisplay.displayEMS(mItem, 1180, res); // expect 20
    CPPUNIT_ASSERT(res == 20);

    _trx.response().str("");
    mDisplay.displayEMS(mItem, 1800, res); // expect 30
    CPPUNIT_ASSERT(res == 30);
    // assert that 'EXC' ('exceeded') is shown
    CPPUNIT_ASSERT(_trx.response().str().find("EXC") != string::npos);
  }

  void testDisplayHeader()
  {
    MileageDisplay mDisplay(_trx);
    CPPUNIT_ASSERT(mDisplay.displayHeader(_trx));
  }

  void testIsFailedDirServiceReturnTrueWhenOneItemFailed()
  {
    MileageRouteItem* item1 = _memHandle.create<MileageRouteItem>();
    MileageRouteItem* item2 = _memHandle.create<MileageRouteItem>();
    MileageRouteItem* item3 = _memHandle.create<MileageRouteItem>();
    MileageRouteItem* item4 = _memHandle.create<MileageRouteItem>();
    item2->failedDirService() = true;

    MileageRoute mileageRoute;
    MileageRouteItems& routeItems(mileageRoute.mileageRouteItems());
    routeItems += *item1, *item2, *item3, *item4;
    MileageDisplay mDisplay(_trx);

    CPPUNIT_ASSERT(mDisplay.isFailedDirService(mileageRoute));
  }

private:
  MileageTrx _trx;
  DataHandle _dataHandle;
  MileageRoute _mRoute;
  MileageDisplay* _mDisplay;
  AirSeg _as1, _as2, _as3, _as4;
  TravelRoute _tvlRoute;
  const Loc* _loc1, *_loc2, *_loc3, *_loc4;
  std::vector<TravelSeg*> _tvlSegs;

  MileageTrx::MileageItem _mileageItems[3];

  DateTime _ticketingDT;

  PricingTrx* _pricingTrx;
  PricingRequest* _request;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageDisplayTest);
} // tse
