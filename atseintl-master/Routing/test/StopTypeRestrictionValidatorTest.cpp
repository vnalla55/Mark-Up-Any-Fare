#include "test/include/CppUnitHelperMacros.h"
#include <vector>
#include <sstream>
#include <cstdlib> // for getenv

#include "Common/SimpleIniMap.h"
#include "Common/Vendor.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RoutingRestriction.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/TravelSeg.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/RoutingConsts.h"
#include "Routing/StopTypeRestrictionValidator.h"
#include "Routing/TravelRoute.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestCityCarrierFactory.h"
#include "test/testdata/TestCityFactory.h"
#include "test/testdata/TestLocKeyFactory.h"
#include "test/testdata/TestRoutingRestrictionFactory.h"
#include "test/testdata/TestTravelRouteFactory.h"
#include "test/include/TestConfigInitializer.h"


namespace tse
{
namespace
{
class StubValidator : public StopTypeRestrictionValidator
{
public:
  StubValidator() {};
  virtual ~StubValidator() {};
  bool isDirect(const TravelRoute& tvlRoute) { return true; }
  bool isNonStop(const TravelRoute& tvlRoute) { return false; }
};
class MyDataHandle : public DataHandleMock
{
public:
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "NYC")
      return "NYC";
    else if (locCode == "MSY")
      return "MSY";
    else if (locCode == "ATL")
      return "ATL";
    else if (locCode == "POS")
      return "POS";
    else if (locCode == "SAN")
      return "SAN";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    return DataHandleMock::getLoc(locCode, date);
  }

  const ZoneInfo* getZone(const VendorCode& vendor, const Zone& zone,
                          Indicator zoneType, const DateTime& date)
  {
    return DataHandleMock::getZone(Vendor::ATPCO, zone, RESERVED, date);
  }

};
}

class StopTypeRestrictionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StopTypeRestrictionValidatorTest);

  CPPUNIT_TEST(testErrorCondition);
  CPPUNIT_TEST(testRestriction3Validation);
  CPPUNIT_TEST(testRestriction3ValidationRtw);
  CPPUNIT_TEST(testRestriction3ValidationWithHiddenStops);
  CPPUNIT_TEST(testisDirectNonStopWithoutHiddenCity);
  CPPUNIT_TEST(testisNonStopDirectWithHiddenCity);
  CPPUNIT_TEST(testfindCitywithRegularScenario);
  CPPUNIT_TEST(testValidateStopType);
  CPPUNIT_TEST(testRestriction4);
  CPPUNIT_TEST(testRestriction4a);
  CPPUNIT_TEST(testRestriction4b);
  CPPUNIT_TEST(testRestriction4_nation);
  CPPUNIT_TEST(testRestriction4_zone);
  CPPUNIT_TEST(testRestriction4_genericCityCode);
  CPPUNIT_TEST(testRestriction6a);
  CPPUNIT_TEST(testRestriction6b);
  CPPUNIT_TEST(testRestriction6_nation);
  CPPUNIT_TEST(testRestriction6_zone);
  CPPUNIT_TEST(testRestriction6_genericCityCode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
      _memHandle.create<TestConfigInitializer>();
      _memHandle.create<MyDataHandle>();
      _options.setRtw(false);
      _trx.setOptions(&_options);
  }

  void tearDown() { _memHandle.clear(); }

  void testRestriction3Validation()
  {
    TravelRoute tvlRoute;
    StopTypeRestrictionValidator validator;

    createTravelRouteAndTravelSeg(tvlRoute, "NYC", "US", "AA", "ATL", "US");
    createTravelRouteAndTravelSeg(tvlRoute, "ATL", "US", "AA", "MSY", "US");

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "3";
    rest.nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    bool result = validator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false); // Cities not found: Restriction not applicable
  }

  void testRestriction3ValidationRtw()
    {
      TravelRoute tvlRoute;
      StopTypeRestrictionValidator validator;
      RoutingRestriction rest;
      rest.restriction() = "3";
      _trx.getOptions()->setRtw(true);

      CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx)); // Cities not found: Restriction not applicable
    }

  void testisDirectNonStopWithoutHiddenCity()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.isDirect(tvlRoute));
    CPPUNIT_ASSERT(validator.isNonStop(tvlRoute));

    // test with multiple segment keep the same tvlRoute just add more segments
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));

    CPPUNIT_ASSERT(!validator.isDirect(tvlRoute));
    CPPUNIT_ASSERT(!validator.isNonStop(tvlRoute));
  }

  void testisNonStopDirectWithHiddenCity()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute()[0].offCity().isHiddenCity() = true;

    // test with only one segment
    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.isDirect(tvlRoute));
    CPPUNIT_ASSERT(!validator.isNonStop(tvlRoute)); // tvlRoute size == 1 but ATL is Hidden

    // test with multiple segment keep the same tvlRoute just add more segments
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute()[1].boardCity().isHiddenCity() = true;
    tvlRoute.travelRoute()[1].offCity().isHiddenCity() = false;


    CPPUNIT_ASSERT(validator.isDirect(tvlRoute));
    CPPUNIT_ASSERT(!validator.isNonStop(tvlRoute));
  }

  void testfindCitywithRegularScenario()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));

    std::vector<TravelRoute::CityCarrier>::const_iterator itr1;
    LocCode city = "NYC";

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.findCity(_dataHandle, tvlRoute.travelRoute(), itr1, std::make_pair('C', city)));

    city = "DFW";
    CPPUNIT_ASSERT(!validator.findCity(_dataHandle, tvlRoute.travelRoute(), itr1, std::make_pair('C', city)));
  }

  void testValidateStopType()
  {
    // case1 : Must be Direct
    RoutingRestriction rest;
    rest.nonStopDirectInd() = DIRECT; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.negViaAppl() = REQUIRED;

    TravelRoute tvlRoute;
    StubValidator validator;
    CPPUNIT_ASSERT(validator.validateStopType(tvlRoute, rest));

    // case2 : Must be NonStop
    rest.nonStopDirectInd() = NONSTOP;
    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!validator.validateStopType(tvlRoute, rest));

    // case4 : Must be NonStop or Direct
    rest.nonStopDirectInd() = EITHER;
    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(validator.validateStopType(tvlRoute, rest));

    // case5 : Must Not be NonStop or Direct
    rest.viaType() = EITHER;
    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!validator.validateStopType(tvlRoute, rest));
  }

  void testRestriction4()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PHX", "DFW"));

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.market1() = "ATL"; // City1
    rest.market2() = "PHX"; // City2
    rest.negViaAppl() = 'R';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(!validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction4a()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PHX", "DFW"));

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "ATL";
    rest.market2() = "MSY";
    rest.negViaAppl() = 'R';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction4b()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PHX", "DFW"));

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "BOS";
    rest.market2() = "MSY";
    rest.negViaAppl() = 'R';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx)); // 1st city not found

    // create the restriction To test the 2nd city is not found
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "ATl";
    rest.market2() = "COL";
    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create the restriction To test the 2nd city is not found
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "ATl";
    rest.market2() = "COL";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // City one occurs after city 2
    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "PHX";
    rest.market2() = "ATL";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction4_nation()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL"));

    RoutingRestriction rest = prepareRestriction(1, "04", ' ', ' ', ' ', "", "", "US", "", 'N');

    rest.negViaAppl() = REQUIRED;
    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction4_zone()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "PL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "US"));

    RoutingRestriction rest = prepareRestriction(1, "04", ' ', ' ', ' ', "", "", "210", "", 'Z');

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction4_genericCityCode()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "USWA", "AA", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL"));

    RoutingRestriction rest = prepareRestriction(1, "04", ' ', ' ', ' ', "", "", "WCC", "", 'C');

    rest.negViaAppl() = REQUIRED;
    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction6a()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PHX", "DFW"));
    tvlRoute.travelRoute()[0].offCity().isHiddenCity() = true;

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "DFW";
    rest.market2() = "";
    rest.negViaAppl() = 'R';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the last city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "DFW";
    rest.market2() = "";
    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the start city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "ATL";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the middle city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "MSY";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(!validator.validate(tvlRoute, rest, _trx));

    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
    // test cases if the city is not found.

    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "BOS";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction6b()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MSY", "PHX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PHX", "DFW"));
    tvlRoute.travelRoute()[0].offCity().isHiddenCity() = true;

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'D';
    rest.market1() = "NYC";
    rest.market2() = "";
    rest.negViaAppl() = 'R';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the last city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "DFW";
    rest.market2() = "";
    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the start city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "ATL";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // create restriction to validate the middle city for to/from NonStop
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "MSY";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(!validator.validate(tvlRoute, rest, _trx));

    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    // test cases if the city is not found.
    rest.restrSeqNo() = 1;
    rest.restriction() = "6";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "BOS";
    rest.market2() = "";
    rest.negViaAppl() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction6_nation()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "US", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL", "PL"));

    RoutingRestriction rest = prepareRestriction(1, "06", ' ', ' ', ' ', "US", "US", "US", "", 'N', 'N', 'N');

    rest.negViaAppl() = REQUIRED;
    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction6_zone()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "PL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "US"));

    RoutingRestriction rest = prepareRestriction(1, "06", ' ', ' ', ' ', "", "", "210", "", 'Z', 'Z', 'Z');

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testRestriction6_genericCityCode()
  {
    _trx.getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "WAS", "AA", "USWA", "USNY"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL**", "PL**"));

    RoutingRestriction rest = prepareRestriction(1, "06", ' ', ' ', ' ', "ECC", "WCC", "WCC", "", 'C');

    rest.negViaAppl() = REQUIRED;
    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void testErrorCondition()
  {
    TravelRoute tvlRoute;
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "7";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'N';
    rest.market1() = "BOS";
    rest.market2() = "";
    rest.negViaAppl() = 'N';

    StopTypeRestrictionValidator validator;
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.negViaAppl() = 'R';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.restrSeqNo() = 1;
    rest.restriction() = "4";
    rest.marketAppl() = 'B';
    rest.nonStopDirectInd() = 'X';
    rest.market1() = "BOS";
    rest.market2() = "";
    rest.negViaAppl() = 'X';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.restriction() = "3";
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.nonStopDirectInd() = 'N';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));

    rest.nonStopDirectInd() = 'D';
    CPPUNIT_ASSERT(validator.validate(tvlRoute, rest, _trx));
  }

  void handleVobDir(std::string& filename)
  {
    static const char defaultVobDir[] = "/vobs/atseintl";
    static const std::string vobDir = TSE_VOB_DIR;
    if (!vobDir.empty() &&
        !filename.compare(0, sizeof(defaultVobDir) - 1, defaultVobDir, sizeof(defaultVobDir) - 1))
      filename.replace(0, sizeof(defaultVobDir) - 1, vobDir);
  }

  void testRestriction3ValidationWithHiddenStops()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    SimpleIniMap params;
    SimpleIniMap::iterator pos;
    std::string key, RoutingRestrictionXMLFileName, TravelRouteXMLFileName, CityCarrierXMLFileName,
        LocXMLFileName;

    if (!params.GetParams(std::string(__func__) + ".cfg"))
    {
      {
        std::ofstream ofs((std::string(__func__) + ".cfg_").c_str());
        ofs << "[test 1]" << std::endl;
        ofs << "TravelRoute=/vobs/atseintl/Routing/test/data/"
               "testRestriction3ValidationWithHiddenStops_TravelRoute.xml" << std::endl;
        ofs << "RoutingRestriction=/vobs/atseintl/Routing/test/data/"
               "testRestriction3ValidationWithHiddenStops_RoutingRestriction.xml" << std::endl;
        ofs << "TravelRoute1=/vobs/atseintl/Routing/test/data/"
               "testRestriction3ValidationWithHiddenStops_TravelRoute1CityCarrier.xml" << std::endl;
        ofs << "TravelSeg1HiddenStop1=/vobs/atseintl/Routing/test/data/"
               "testRestriction3ValidationWithHiddenStops_TravelSeg1HiddenStop1Loc.xml"
            << std::endl;
        ofs << "ValidationResult=true" << std::endl;
        ofs << std::endl;
      }

      // CPPUNIT_ASSERT(!("Cannot find cfg file: "+ std::string(__func__) +".cfg").c_str());
    }

    for (pos = params.begin(); pos != params.end(); pos++)
    {
      bool ConfigValidationResult = true;

      if (params.exist(pos->first, "ValidationResult"))
      {
        if (params[pos->first]["ValidationResult"] == "true")
          ConfigValidationResult = true;
        else
          ConfigValidationResult = false;
      }
      if (!params.exist(pos->first, "RoutingRestriction"))
        continue;

      RoutingRestrictionXMLFileName = params[pos->first]["RoutingRestriction"];
      CPPUNIT_ASSERT(RoutingRestrictionXMLFileName != "");
      {
        handleVobDir(RoutingRestrictionXMLFileName);
        if (!std::ifstream(RoutingRestrictionXMLFileName.c_str()))
          CPPUNIT_ASSERT(!"Cannot find RoutingRestriction xml file");
      }

      if (!params.exist(pos->first, "TravelRoute"))
        continue;

      TravelRouteXMLFileName = params[pos->first]["TravelRoute"];
      CPPUNIT_ASSERT(TravelRouteXMLFileName != "");
      {
        handleVobDir(TravelRouteXMLFileName);
        if (!std::ifstream(TravelRouteXMLFileName.c_str()))
          CPPUNIT_ASSERT(!"Cannot find TravelRoute xml file");
      }

      TravelRoute* tvlRoute = TestTravelRouteFactory::create(TravelRouteXMLFileName.c_str());
      RoutingRestriction* rest =
          TestRoutingRestrictionFactory::create(RoutingRestrictionXMLFileName.c_str());

      int ipos = 1;
      while (true)
      {
        std::stringstream istream;
        istream << ipos;
        istream >> key;
        key = "TravelRoute" + key;

        if (!params.exist(pos->first, key))
          break;

        CityCarrierXMLFileName = params[pos->first][key];
        CPPUNIT_ASSERT(CityCarrierXMLFileName != "");
        {
          handleVobDir(CityCarrierXMLFileName);
          if (!std::ifstream(CityCarrierXMLFileName.c_str()))
            CPPUNIT_ASSERT(!"Cannot find CityCarrier xml file");
        }

        TravelRoute::CityCarrier* cityCarrier =
            TestCityCarrierFactory::create(CityCarrierXMLFileName.c_str());
        tvlRoute->travelRoute().push_back(*cityCarrier);

        AirSeg* newTvlSeg = NULL;
        int jpos = 1;
        while (true)
        {
          {
            std::stringstream istream;
            istream << ipos;
            istream >> key;
          }
          LocXMLFileName = "TravelSeg" + key;
          {
            std::stringstream istream;
            istream << jpos;
            istream >> key;
          }
          key = LocXMLFileName + "HiddenStop" + key;

          if (!params.exist(pos->first, key))
            break;

          LocXMLFileName = params[pos->first][key];
          CPPUNIT_ASSERT(LocXMLFileName != "");
          {
            handleVobDir(LocXMLFileName);
            if (!std::ifstream(LocXMLFileName.c_str()))
              CPPUNIT_ASSERT(!"Cannot find Loc xml file");
          }

          trx->dataHandle().get(newTvlSeg);
          newTvlSeg->origAirport() = cityCarrier->boardCity().loc();
          newTvlSeg->destAirport() = cityCarrier->offCity().loc();
          newTvlSeg->carrier() = cityCarrier->carrier();

          const Loc* board =
              trx->dataHandle().getLoc(cityCarrier->boardCity().loc(), tvlRoute->travelDate());
          const Loc* off =
              trx->dataHandle().getLoc(cityCarrier->offCity().loc(), tvlRoute->travelDate());

          if (board != 0 && off != 0)
          {
            newTvlSeg->origin() = board;
            newTvlSeg->destination() = off;
          }

          LocKey* hiddenLocKey = TestLocKeyFactory::create(LocXMLFileName.c_str());
          Loc* hiddenLoc =
              (Loc*)trx->dataHandle().getLoc(hiddenLocKey->loc(), tvlRoute->travelDate());
          newTvlSeg->hiddenStops().push_back(hiddenLoc);

          jpos++;
        }
        if (newTvlSeg)
          tvlRoute->mileageTravelRoute().push_back(newTvlSeg);

        ipos++;
      }

      StopTypeRestrictionValidator validator;
      bool result = validator.validate(*tvlRoute, *rest, _trx);
      CPPUNIT_ASSERT(result == ConfigValidationResult);
    }
  }

protected:
  TravelRoute::CityCarrier prepareCityCarrier(LocCode bordCity, LocCode offCity,
                                              CarrierCode carrierCode = "AA",
                                              NationCode offNation = "",
                                              NationCode boardNation = "")
  {
      TravelRoute::City boardCty, offCty;
      boardCty.loc() = bordCity;
      boardCty.isHiddenCity() = false;
      offCty.loc() = offCity;
      offCty.isHiddenCity() = false;

      TravelRoute::CityCarrier cityCarrier;
      cityCarrier.boardCity() = boardCty;
      cityCarrier.carrier() = carrierCode;
      cityCarrier.offCity() = offCty;

      cityCarrier.offNation() = offNation;
      cityCarrier.boardNation() = boardNation;

      return cityCarrier;
  }

  RoutingRestriction prepareRestriction(int seqNo, RestrictionNumber res, Indicator marketAppl,
                                        Indicator nonStopDirectInd, Indicator airSurfaceInd,
                                        LocCode market1, LocCode market2, LocCode viaMarket,
                                        LocCode viaCarrier, Indicator viaType = 'C',
                                        Indicator market1type = 'C', Indicator market2type = 'C')
  {
      RoutingRestriction rest;
      rest.restrSeqNo() = seqNo;
      rest.restriction() = res;
      rest.marketAppl() = marketAppl; // B=Between City1 and City2, T=To/From City1
      rest.viaType() = viaType; // A=Airline, C=City, Blank=Not Applicable
      rest.nonStopDirectInd() = nonStopDirectInd; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
      rest.airSurfaceInd() = airSurfaceInd; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
      rest.market1() = market1; // City1
      rest.market2() = market2; // City2
      rest.viaMarket() = viaMarket; // Via City
      rest.viaCarrier() = viaCarrier; // Via Carrier
      rest.market1type() = market1type;  // A=Airline, C=City, Blank=Not Applicable
      rest.market2type() = market2type; // A=Airline, C=City, Blank=Not Applicable
      rest.negViaAppl() = REQUIRED;

      return rest;
  }

  void createTravelRouteAndTravelSeg(TravelRoute& travelRoute,
                                     const char* boardCity,
                                     const char* boardNation,
                                     const char* carrier,
                                     const char* offCity,
                                     const char* offNation)
  {
    createTravelRoute(
        travelRoute.travelRoute(), boardCity, boardNation, carrier, offCity, offNation);
    createTravelSeg(
        travelRoute.mileageTravelRoute(), boardCity, boardNation, carrier, offCity, offNation);
  }

  void createTravelRoute(std::vector<TravelRoute::CityCarrier>& travelRoute,
                         const char* boardCity,
                         const char* boardNation,
                         const char* carrier,
                         const char* offCity,
                         const char* offNation)
  {
    TravelRoute::CityCarrier cc;
    cc.boardCity().loc() = boardCity;
    cc.boardCity().isHiddenCity() = false; // don't care really
    cc.offCity().loc() = offCity;
    cc.offCity().isHiddenCity() = false; // don't care really
    cc.carrier() = carrier;
    cc.stopover() = false; // don't care really
    travelRoute.push_back(cc);
  }

  void createTravelSeg(std::vector<TravelSeg*>& travelSeg,
                       const char* boardCity,
                       const char* boardNation,
                       const char* carrier,
                       const char* offCity,
                       const char* offNation)
  {
    AirSeg* as = _memHandle.create<AirSeg>();
    Loc* lc1 = _memHandle.create<Loc>();
    lc1->loc() = boardCity;
    lc1->nation() = boardNation;
    as->origin() = lc1;
    as->origAirport() = boardCity;
    Loc* lc2 = _memHandle.create<Loc>();
    lc2->loc() = offCity;
    lc2->nation() = offNation;
    as->destination() = lc2;
    as->destAirport() = offCity;
    as->carrier() = carrier;
    travelSeg.push_back(as);
  }

  TestMemHandle _memHandle;
  PricingOptions _options;
  PricingTrx _trx;
  DataHandle _dataHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(StopTypeRestrictionValidatorTest);
}
