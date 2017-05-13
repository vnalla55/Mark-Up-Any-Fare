//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include <iostream>
#include <memory>
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag455Collector.h"
#include "Diagnostic/DCFactory.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "Routing/TravelRoute.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "Routing/SpecifiedRoutingValidator.h"
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "DataModel/AirSeg.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/Config/ConfigMan.h"

using namespace std;
namespace tse
{
class SpecifiedRoutingValidatorTest : public CppUnit::TestFixture
{
  class SRVDataHandleMock : public DataHandleMock
  {
    RoutingMap* makeRoutingMap(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& routingTariff,
                               const RoutingNumber& routingNumber,
                               const DateTime& date,
                               int seqNumber,
                               int nextLocNo,
                               LocCode locCode)
    {
      RoutingMap* rm = new RoutingMap();
      rm->vendor() = vendor;
      rm->carrier() = carrier;
      rm->routingTariff() = routingTariff;
      rm->routing() = routingNumber;

      DateTime effDate = DateTime::localTime() + Hours(-240);
      rm->effDate() = effDate;
      rm->lnkmapsequence() = seqNumber;
      rm->loc1No() = rm->lnkmapsequence();
      rm->loctag() = MapNode::ENTRY;
      rm->nextLocNo() = nextLocNo;
      rm->altLocNo() = 0;

      rm->loc1().loc() = locCode;
      rm->loc1().locType() = MapNode::CITY;
      return rm;
    }

    TestMemHandle _memHandle;

  public:
    ~SRVDataHandleMock() { _memHandle.clear(); }

    const std::vector<Routing*>& getRouting(const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& routingTariff,
                                            const RoutingNumber& routingNumber,
                                            const DateTime& date)
    {
      std::vector<Routing*>* ret = _memHandle.create<std::vector<Routing*> >();
      Routing* routing = _memHandle.create<Routing>();
      routing->vendor() = vendor;
      routing->carrier() = carrier;
      routing->routingTariff() = routingTariff;
      routing->routing() = routingNumber;
      routing->createDate() = date;

      switch (routingTariff)
      {
      case 17:
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 1, 2, "TCQ"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 2, 3, "LIM"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 3, 0, "BUE"));
        break;

      case 3:
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 1, 2, "DFW"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 2, 3, "LAX"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 3, 4, "AKL"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 4, 0, "SYD"));
        break;

      case 99:
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 1, 2, "LAX"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 2, 3, "DFW"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 3, 0, "NYC"));
        break;

      case 4:
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 1, 2, "ALK"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 2, 3, "LAX"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 3, 4, "SFO"));
        routing->rmaps().push_back(
            makeRoutingMap(vendor, carrier, routingTariff, routingNumber, date, 4, 0, "FRA"));
        break;

      default:
        // std::cout << std::endl << "Not handled call to SRVDataHandleMock::getRouting with
        // parameters: "<<"routingTariff="<<routingTariff;
        break;
      }
      ret->push_back(routing);
      return (*ret);
    }

    const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
    {
      if ((carrier == "NZ"))
      {
        return TestCarrierPreferenceFactory::create(
            "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_AA.xml");
      }
      if ((carrier == "QF"))
      {
        return TestCarrierPreferenceFactory::create(
            "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_BA.xml");
      }
      if ((carrier == "TN"))
      {
        return TestCarrierPreferenceFactory::create(
            "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_BA.xml");
      }
      if ((carrier == "CO"))
      {
        return TestCarrierPreferenceFactory::create(
            "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_BA.xml");
      }
      return DataHandleMock::getCarrierPreference(carrier, date);
    }
  };

  CPPUNIT_TEST_SUITE(SpecifiedRoutingValidatorTest);
  CPPUNIT_TEST(testAddOn);
  CPPUNIT_TEST(testFound);
  CPPUNIT_TEST(testNotFound);
  CPPUNIT_TEST(testSurfaceSector);
  CPPUNIT_TEST(testSlash_CHI_DFW_ORF);
  CPPUNIT_TEST(testSlash_CHI_RDU_ORF);
  CPPUNIT_TEST(testSlash_CHI_DFW_RDU_ORF);
  CPPUNIT_TEST(testSlash_ORF_DFW_CHI);
  CPPUNIT_TEST(testSlash_ORF_RDU_CHI);
  CPPUNIT_TEST(testSlash_ORF_RDU_DFW_CHI);
  CPPUNIT_TEST(testDash);
  CPPUNIT_TEST(testGeoOrder);
  CPPUNIT_TEST(testOneCarrier);
  CPPUNIT_TEST(testMultipleCarrier);
  CPPUNIT_TEST(testAlternateCarrier);
  CPPUNIT_TEST(testAlternateCarrierB);
  CPPUNIT_TEST(testAlternateCarrierC);
  CPPUNIT_TEST(testIndustryCarrier);
  CPPUNIT_TEST(testSurfaceTravel);
  CPPUNIT_TEST(testAddOn_OnSecondMapOnly_SEA_FLL);
  CPPUNIT_TEST(testAddOn_ReversedOnSecondMapOnly_SEA_FLL);
  CPPUNIT_TEST(testAddOnEntryExit_LAX_SFO_DFW_WAS);
  CPPUNIT_TEST(testAddOnEntryExit_LAX_SFO_DFW_LAX);
  CPPUNIT_TEST(testAddOnEntryExit_NYC_DFW_CHI_WAS);
  CPPUNIT_TEST(testAddOnEntryExit_DFW_NYC_CHI_SFO);
  CPPUNIT_TEST(testAddOnEntryExit_SFO_LAX_DFW_WAS);
  CPPUNIT_TEST(testAddOnEntryExit_DFW_NYC_WAS_SFO);
  CPPUNIT_TEST(testAddOnEntryExit_LAX_SFO_CHI_NYC);
  CPPUNIT_TEST(testAddOnEntryExit_BOS_DFW_CHI_LON);
  CPPUNIT_TEST(testAddOnEntryExit_LAX_DFW_CHI_LON_NYC);
  CPPUNIT_TEST(testAddOnEntryExit_NYC_DFW_CHI_LON);
  CPPUNIT_TEST(testAddOnEntryExit_LAX_DFW_NYC_LON);
  CPPUNIT_TEST(testAddOnCommonPoint_LAX_DFW_CHI_LON);
  CPPUNIT_TEST(testAddOnCommonPoint_LAX_DFW_NYC_LON);
  CPPUNIT_TEST(testAddOnCommonPoint_NYC_DFW_CHI_LON);
  CPPUNIT_TEST(testAddOnCommonPoint_DFW_CHI_LAX_SFO);
  CPPUNIT_TEST(testAddOnCommonPoint_LON_CHI_DFW_LAX_SFO);
  CPPUNIT_TEST(testFirstCityValidation);
  CPPUNIT_TEST(testOverflownCities_CHI_NYC_CO_NYC_AA_MIA);
  CPPUNIT_TEST(testOverflownCities_CHI_NYC_CO_NYC_AA_MIA_carrierCO);
  CPPUNIT_TEST(testOverflownCities_CHI_AA_NYC_CO_NYC_AA_MIA);
  CPPUNIT_TEST(testOverflownCities_CHI_CO_NYC_CO_NYC_AA_MIA);
  CPPUNIT_TEST(testOverflownCities_CHI_WAS_CO_NYC_AA_MIA);
  CPPUNIT_TEST(testOverflownCities_CHI_WAS_CO_NYC_AA_MIA_carrierCO);
  CPPUNIT_TEST(testOverflownCities_CHI_AA_WAS_CO_NYC_AA_MIA);
  CPPUNIT_TEST(testOverflownCities_CHI_CO_WAS_CO_NYC_AA_MIA);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx _trx;
  SRVDataHandleMock* _srvDataHandle;
  TestMemHandle _memHandle;
  std::unique_ptr<PricingRequest> _request;
  PricingOptions* _pricingOptions;

public:
  SpecifiedRoutingValidatorTest()
  {
    tse::AirSeg* as =
        TestAirSegFactory::create("/vobs/atseintl/test/sampledata/DFW_LAX_AirSegment.xml");
    _trx.travelSeg().push_back(as);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", "2013-06-16", "PRICING_SVC");

    _request.reset(new PricingRequest);
    _request->ticketingDT() = DateTime(2014, 6, 16); // flag active
    _trx.setRequest(_request.get());
    _pricingOptions = _memHandle.create<PricingOptions>();
    _trx.setOptions(_pricingOptions);

    _srvDataHandle = _memHandle.create<SRVDataHandleMock>();
  }

  void tearDown()
  {
    _memHandle.clear();
    _request.reset();
  }

  void testAddOn()
  {
    const DateTime travelDate = DateTime::localTime();
    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;
    const std::vector<Routing*>& routingVect =
        _trx.dataHandle().getRouting("ATP", "AA", 17, "0003", DateTime::localTime());
    const Routing* routing = routingVect.front();
    const std::vector<Routing*>& routingVectAdd =
        _trx.dataHandle().getRouting("ATP", "AA", 17, "0005", DateTime::localTime());
    const Routing* addOn = routingVectAdd.front();

    TravelRoute travelRoute;
    createSeg(travelRoute, "TCQ", "AA", "LIM", true);
    createSeg(travelRoute, "LIM", "AA", "BUE", true);

    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;
    CPPUNIT_ASSERT(validator.validate(_trx, travelRoute, routing, &mapInfo, travelDate, addOn));
    CPPUNIT_ASSERT(!(mapInfo.routeStrings()->empty()));
  }

  void testFound()
  {
    const DateTime travelDate = DateTime::localTime();
    Diagnostic& diag = _trx.diagnostic();

    diag.diagnosticType() = Diagnostic455;
    const std::vector<Routing*>& routingVect =
        _trx.dataHandle().getRouting("ATP", "SQ", 3, "7058", DateTime::localTime());
    const Routing* routing = routingVect.front();

    TravelRoute travelRoute;
    createSeg(travelRoute, "DFW", "SQ", "LAX", true);
    createSeg(travelRoute, "LAX", "SQ", "AKL", true);
    createSeg(travelRoute, "AKL", "SQ", "SYD", true);

    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;
    CPPUNIT_ASSERT(validator.validate(_trx, travelRoute, routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT(!(mapInfo.routeStrings()->empty()));
  }

  void testNotFound()
  {
    const DateTime travelDate = DateTime::localTime();
    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;
    const std::vector<Routing*>& routingVect =
        _trx.dataHandle().getRouting("ATP", "AA", 99, "0519", DateTime::localTime());
    const Routing* routing = routingVect.front();

    TravelRoute travelRoute;
    createSeg(travelRoute, "LAX", "AA", "DFW", true);
    createSeg(travelRoute, "DFW", "SQ", "MYS", true);

    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;
    CPPUNIT_ASSERT(!validator.validate(_trx, travelRoute, routing, &mapInfo, travelDate));
  }

  void testSurfaceSector()
  {
    const DateTime travelDate = DateTime::localTime();
    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;
    const std::vector<Routing*>& routingVect =
        _trx.dataHandle().getRouting("ATP", "NZ", 4, "0129", DateTime::localTime());
    const Routing* routing = routingVect.front();

    TravelRoute travelRoute;
    createSeg(travelRoute, "AKL", "NZ", "LAX", true);
    createSeg(travelRoute, "LAX", "XX", "SFO", true);
    createSeg(travelRoute, "SFO", "NZ", "FRA", true);

    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;
    CPPUNIT_ASSERT(!validator.validate(_trx, travelRoute, routing, &mapInfo, travelDate));
  }

  /**
   * When two or more cities are separated by the symbol "/",
   * only one of these cities may be used as a ticketed transit point.
   */

  void slashTestRouteSetup(Routing& routing)
  {
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 1;
    routing.routing() = "0058";
    // CHI-DFW/RDU-ORF
    createMap(routing, 1, '1', 2, 0, 'C', "CHI");
    createMap(routing, 2, ' ', 4, 3, 'C', "DFW");
    createMap(routing, 3, ' ', 4, 0, 'C', "RDU");
    createMap(routing, 4, 'X', 0, 0, 'C', "ORF");
  }

  void testSlash_CHI_RDU_ORF()
  {
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    TravelRoute travelRoute;
    MapInfo mapInfo;
    Routing routing;
    SpecifiedRoutingValidator validator;

    slashTestRouteSetup(routing);
    createSeg(travelRoute, "CHI", "AA", "RDU");
    createSeg(travelRoute, "RDU", "AA", "ORF");
    CPPUNIT_ASSERT_MESSAGE(
        "CHI-RDU-ORF",
        validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
  }

  void testSlash_CHI_DFW_RDU_ORF()
  {
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    TravelRoute travelRoute;
    MapInfo mapInfo;
    Routing routing;
    SpecifiedRoutingValidator validator;

    slashTestRouteSetup(routing);
    createSeg(travelRoute, "CHI", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "RDU");
    createSeg(travelRoute, "RDU", "AA", "ORF");
    CPPUNIT_ASSERT_MESSAGE(
        "CHI-DFW-RDU-ORF",
        !validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
    CPPUNIT_ASSERT_MESSAGE("CHI-DFW-rdu-ORF", mapInfo.missingCityIndex() == 1);
    CPPUNIT_ASSERT(!mapInfo.missingCarrier());
  }

  void testSlash_ORF_DFW_CHI()
  {
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    TravelRoute travelRoute;
    MapInfo mapInfo;
    Routing routing;
    SpecifiedRoutingValidator validator;

    slashTestRouteSetup(routing);
    createSeg(travelRoute, "ORF", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    CPPUNIT_ASSERT_MESSAGE(
        "ORF-DFW-CHI",
        validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
  }

  void testSlash_ORF_RDU_CHI()
  {
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    TravelRoute travelRoute;
    MapInfo mapInfo;
    Routing routing;
    SpecifiedRoutingValidator validator;

    slashTestRouteSetup(routing);
    createSeg(travelRoute, "ORF", "AA", "RDU");
    createSeg(travelRoute, "RDU", "AA", "CHI");
    CPPUNIT_ASSERT_MESSAGE(
        "ORF-RDU-CHI",
        validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
  }

  void testSlash_ORF_RDU_DFW_CHI()
  {
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    TravelRoute travelRoute;
    MapInfo mapInfo;
    Routing routing;
    SpecifiedRoutingValidator validator;

    slashTestRouteSetup(routing);
    createSeg(travelRoute, "ORF", "AA", "RDU");
    createSeg(travelRoute, "RDU", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    CPPUNIT_ASSERT_MESSAGE(
        "ORF-RDU-DFW-CHI",
        !validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
    CPPUNIT_ASSERT_MESSAGE("ORF-RDU-dfw-CHI", mapInfo.missingCityIndex() == 1);
    CPPUNIT_ASSERT(!mapInfo.missingCarrier());
  }

  void testSlash_CHI_DFW_ORF()
  {
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;
    Routing routing;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    slashTestRouteSetup(routing);

    createSeg(travelRoute, "CHI", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "ORF");
    CPPUNIT_ASSERT_MESSAGE(
        "CHI-DFW-ORF",
        validator.validate(_trx, travelRoute, &routing, &mapInfo, DateTime::localTime()));
  }

  /**
   * When two or more cities are separated by the symbol "-",
   * both cities may be used as ticketed transit points.
   */
  void testDash()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "SQ";
    routing.routingTariff() = 1;
    routing.routing() = "0210";
    // NYC-SQ-HKG-SQ-SIN-SQ-JKT
    createMap(routing, 1, '1', 2, 0, 'C', "NYC");
    createMap(routing, 2, ' ', 3, 0, 'A', "SQ");
    createMap(routing, 3, ' ', 4, 0, 'C', "HKG");
    createMap(routing, 4, ' ', 5, 0, 'A', "SQ");
    createMap(routing, 5, ' ', 6, 0, 'C', "SIN");
    createMap(routing, 6, ' ', 7, 0, 'A', "SQ");
    createMap(routing, 7, 'X', 0, 0, 'C', "JKT");

    TravelRoute travelRoute;

    createSeg(travelRoute, "NYC", "SQ", "HKG");
    createSeg(travelRoute, "HKG", "SQ", "SIN");
    createSeg(travelRoute, "SIN", "SQ", "JKT");
    CPPUNIT_ASSERT_MESSAGE("NYC-HKG-SIN-JKT",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
  }

  /**
   * A routing must be traveled in the geographic order as published on the
   * routing map.
   */
  void testGeoOrder()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "QF";
    routing.routingTariff() = 1;
    routing.routing() = "0058";
    createMap(routing, 1, '1', 2, 0, 'C', "ADL");
    createMap(routing, 2, ' ', 3, 0, 'C', "MEL");
    createMap(routing, 3, ' ', 4, 0, 'C', "SYD");
    createMap(routing, 4, ' ', 5, 0, 'C', "BNE");
    createMap(routing, 5, ' ', 6, 0, 'C', "ROK");
    createMap(routing, 6, 'X', 0, 0, 'C', "TSV");

    TravelRoute travelRoute;
    createSeg(travelRoute, "MEL", "QF", "SYD");
    createSeg(travelRoute, "SYD", "QF", "BNE");
    createSeg(travelRoute, "BNE", "QF", "ROK");
    createSeg(travelRoute, "ROK", "QF", "TSV");
    CPPUNIT_ASSERT_MESSAGE("MEL-SYD-BNE-ROK-TSV",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "MEL", "QF", "BNE");
    createSeg(travelRoute, "BNE", "QF", "SYD");
    createSeg(travelRoute, "SYD", "QF", "ROK");
    createSeg(travelRoute, "ROK", "QF", "TSV");
    CPPUNIT_ASSERT_MESSAGE("MEL-BNE-SYD-ROK-TSV",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT_MESSAGE("MEL-BNE-syd-ROK-TSV", mapInfo.missingCityIndex() == 1);
    CPPUNIT_ASSERT(!mapInfo.missingCarrier());
  }

  /**
   * A routing specifying only one designated carrier may only be traveled via any,
   * all, or none of the ticketed points named.
   */
  void testOneCarrier()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "QF";
    routing.routingTariff() = 1;
    routing.routing() = "0058";
    createMap(routing, 1, '1', 2, 0, 'C', "ADL");
    createMap(routing, 2, ' ', 3, 0, 'C', "MEL");
    createMap(routing, 3, ' ', 4, 0, 'C', "SYD");
    createMap(routing, 4, ' ', 5, 0, 'C', "BNE");
    createMap(routing, 5, ' ', 6, 0, 'C', "ROK");
    createMap(routing, 6, 'X', 0, 0, 'C', "TSV");

    TravelRoute travelRoute;

    createSeg(travelRoute, "ADL", "QF", "MEL");
    createSeg(travelRoute, "MEL", "QF", "SYD");
    createSeg(travelRoute, "SYD", "QF", "BNE");
    createSeg(travelRoute, "BNE", "QF", "ROK");
    createSeg(travelRoute, "ROK", "QF", "TSV");
    CPPUNIT_ASSERT_MESSAGE("ADL-MEL-SYD-BNE-ROK-TSV",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "ADL", "QF", "TSV");
    CPPUNIT_ASSERT_MESSAGE("ADL-TSV",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "ADL", "QF", "MEL");
    createSeg(travelRoute, "MEL", "QF", "SYD");
    createSeg(travelRoute, "SYD", "NZ", "BNE");
    createSeg(travelRoute, "BNE", "QF", "ROK");
    createSeg(travelRoute, "ROK", "QF", "TSV");
    CPPUNIT_ASSERT_MESSAGE("ADL-MEL-SYD-NZ-BNE-ROK-TSV",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT_MESSAGE("ADL-MEL-SYD-nz-BNE-ROK-TSV", mapInfo.missingCityIndex() == 2);
    CPPUNIT_ASSERT(mapInfo.missingCarrier());
  }

  /**
   * A routing specifying more than one designated carrier may be traveled via any,
   * all, or none of the ticketed points named for portion where a single carrier
   * is designated.
   */
  void testMultipleCarrier()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 1;
    routing.routing() = "0200";
    createMap(routing, 1, '1', 2, 0, 'C', "NYC");
    createMap(routing, 2, ' ', 3, 0, 'C', "LON");
    createMap(routing, 3, ' ', 4, 0, 'A', "SQ");
    createMap(routing, 4, ' ', 5, 0, 'C', "SIN");
    createMap(routing, 5, ' ', 6, 0, 'A', "SQ");
    createMap(routing, 6, 'X', 0, 0, 'C', "BKK");

    TravelRoute travelRoute;

    // travelRoute.travelRoute().clear();
    createSeg(travelRoute, "NYC", "AA", "LON");
    createSeg(travelRoute, "LON", "SQ", "SIN");
    createSeg(travelRoute, "SIN", "SQ", "BKK");
    CPPUNIT_ASSERT_MESSAGE("NYC-AA-LON-SQ-SIN-SQ-BKK",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "NYC", "AA", "LON");
    createSeg(travelRoute, "LON", "SQ", "BKK");
    CPPUNIT_ASSERT_MESSAGE("NYC-AA-LON-SQ-BKK",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
  }

  /**
   * A carrier cannot be validated from origin to destination of a specified routing
   * when alternate carriers are named for a portion of travel.
   */
  void testAlternateCarrier()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "JL";
    routing.routingTariff() = 1;
    routing.routing() = "0121";
    createMap(routing, 1, '1', 2, 0, 'C', "PMI");
    createMap(routing, 2, ' ', 3, 0, 'A', "IB");
    createMap(routing, 3, ' ', 4, 0, 'C', "MAD");
    createMap(routing, 4, 'X', 0, 0, 'C', "AMM");

    TravelRoute travelRoute;

    createSeg(travelRoute, "PMI", "IB", "MAD");
    createSeg(travelRoute, "MAD", "JL", "AMM");
    CPPUNIT_ASSERT_MESSAGE("PMI-IB-MAD-AMM",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "PMI", "JL", "AMM");
    CPPUNIT_ASSERT_MESSAGE("PMI-JL-AMM",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PMI-JL-AMM", 0, (int)mapInfo.missingCityIndex());
  }

  void testAlternateCarrierB()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "JL";
    routing.routingTariff() = 1;
    routing.routing() = "0121";
    createMap(routing, 1, '1', 2, 0, 'C', "PMI");
    createMap(routing, 2, ' ', 3, 0, 'A', "IB");
    createMap(routing, 3, ' ', 4, 0, 'C', "MAD");
    createMap(routing, 4, ' ', 5, 0, 'A', "IB");
    createMap(routing, 5, 'X', 0, 0, 'C', "AMM");

    createSeg(travelRoute, "PMI", "JL", "MAD");
    createSeg(travelRoute, "MAD", "IB", "AMM");

    CPPUNIT_ASSERT_MESSAGE("PMI-JL-MAD-IB-AMM",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PMI-JL-AMM", 0, (int)mapInfo.missingCityIndex());
    CPPUNIT_ASSERT(mapInfo.missingCarrier());
  }

  void testAlternateCarrierC()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "JL";
    routing.routingTariff() = 1;
    routing.routing() = "0121";

    createMap(routing, 1, '1', 2, 0, 'C', "PMI");
    createMap(routing, 2, ' ', 3, 0, 'A', "IB");
    createMap(routing, 3, ' ', 4, 0, 'C', "MAD");
    createMap(routing, 4, ' ', 5, 0, 'C', "NYC");
    createMap(routing, 5, ' ', 6, 0, 'C', "AMM");
    createMap(routing, 6, 'X', 0, 0, 'C', "LON");

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "PMI", "IB", "MAD");
    createSeg(travelRoute, "MAD", "JL", "NYC");
    createSeg(travelRoute, "NYC", "IB", "AMM");
    createSeg(travelRoute, "AMM", "JL", "LON");

    CPPUNIT_ASSERT_MESSAGE("PMI-IB-MAD-NYC",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PMI-IB-MAD-IB-AMM", 2, (int)mapInfo.missingCityIndex());
    CPPUNIT_ASSERT(mapInfo.missingCarrier());
  }

  /**
   * A routing specifying YY as the designated carrier for all or a portion of a routing
   * may be traveled via any, all, or none of the ticketed points named for that portion
   * of the routing where YY is designated.
   */
  void testIndustryCarrier()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 1;
    routing.routing() = "0103";
    createMap(routing, 1, '1', 2, 0, 'C', "FAI");
    createMap(routing, 2, ' ', 3, 0, 'A', "YY");
    createMap(routing, 3, ' ', 4, 0, 'C', "ANC");
    createMap(routing, 4, ' ', 5, 0, 'A', "YY");
    createMap(routing, 5, ' ', 6, 0, 'C', "SEA");
    createMap(routing, 6, ' ', 7, 0, 'C', "STL");
    createMap(routing, 7, ' ', 0, 0, 'C', "PAR");

    TravelRoute travelRoute;

    // travelRoute.travelRoute().clear();
    createSeg(travelRoute, "FAI", "DL", "ANC");
    createSeg(travelRoute, "ANC", "DL", "SEA");
    CPPUNIT_ASSERT_MESSAGE("FAI-DL-ANC-DL-SEA",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "FAI", "AA", "PAR");
    CPPUNIT_ASSERT_MESSAGE("FAI-AA-PAR",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "FAI", "AA", "ANC");
    createSeg(travelRoute, "ANC", "AA", "SEA");
    createSeg(travelRoute, "SEA", "AA", "STL");
    createSeg(travelRoute, "STL", "AA", "PAR");
    CPPUNIT_ASSERT_MESSAGE("FAI-ANC-SEA-STL-PAR",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
  }

  /**
   * Surface travel is permitted between any two ticketed points on a published
   * routing as long as both ticketed points are published on the specified route
   * string and no surface restriction items that prohibit the surface sector are filed
   * in conjunction with the routing.  The designated carrier code between the
   * terminal points of the surface sector may be ignored when validating the
   * routing between these points.
   */
  void testSurfaceTravel()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "NZ";
    routing.routingTariff() = 4;
    routing.routing() = "0139";
    createMap(routing, 1, '1', 2, 0, 'C', "AKL");
    createMap(routing, 2, ' ', 4, 3, 'C', "LAX");
    createMap(routing, 3, ' ', 5, 0, 'C', "SFO");
    createMap(routing, 4, ' ', 5, 0, 'C', "NYC");
    createMap(routing, 5, 'X', 0, 0, 'C', "FRA");

    TravelRoute travelRoute;

    // travelRoute.travelRoute().clear();
    createSeg(travelRoute, "AKL", "NZ", "LAX");
    createSeg(travelRoute, "LAX", "XX", "SFO");
    createSeg(travelRoute, "SFO", "NZ", "FRA");
    CPPUNIT_ASSERT_MESSAGE("AKL-LAX//SFO-FRA",
                           !validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "AKL", "NZ", "LAX");
    createSeg(travelRoute, "LAX", "XX", "NYC");
    createSeg(travelRoute, "NYC", "NZ", "FRA");
    CPPUNIT_ASSERT_MESSAGE("AKL-LAX//NYC-FRA",
                           validator.validate(_trx, travelRoute, &routing, &mapInfo, travelDate));
  }

  void addOnEntryExitSetup(Routing& routing, Routing& routing2)
  {
    // Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 4;
    routing.routing() = "0129";
    routing.commonPointInd() = '1';
    createMap(routing, 1, '1', 2, 0, 'C', "SFO");
    createMap(routing, 2, ' ', 3, 0, 'C', "LAX");
    createMap(routing, 3, ' ', 4, 0, 'C', "BOS");
    createMap(routing, 4, ' ', 5, 0, 'C', "DFW");
    createMap(routing, 5, 'X', 0, 0, 'C', "NYC");

    // Routing routing2;DFW-NYC-CHI-SFO
    routing2.vendor() = "ATP";
    routing2.carrier() = "AA";
    routing2.routingTariff() = 4;
    routing2.routing() = "0123";
    routing2.commonPointInd() = '1';
    createMap(routing2, 1, '1', 2, 0, 'C', "LAX");
    createMap(routing2, 2, ' ', 3, 0, 'C', "DFW");
    createMap(routing2, 3, ' ', 4, 0, 'C', "SFO");
    createMap(routing2, 4, ' ', 6, 0, 'C', "CHI");
    createMap(routing2, 5, '1', 6, 0, 'C', "NYC");
    createMap(routing2, 6, 'X', 0, 0, 'C', "LON");
    createMap(routing2, 7, '1', 8, 0, 'C', "NYC");
    createMap(routing2, 8, ' ', 9, 0, 'C', "CHI");
    createMap(routing2, 9, 'X', 0, 0, 'C', "SFO");
    createMap(routing2, 10, '1', 11, 0, 'C', "SFO");
    createMap(routing2, 11, ' ', 12, 0, 'C', "WAS");
    createMap(routing2, 12, ' ', 13, 0, 'C', "NYC");
    createMap(routing2, 13, 'X', 0, 0, 'C', "CHI");
    createMap(routing2, 14, '1', 15, 0, 'C', "WAS");
    createMap(routing2, 15, ' ', 16, 0, 'C', "CHI");
    createMap(routing2, 16, 'X', 0, 0, 'C', "DFW");
    createMap(routing2, 17, '1', 18, 0, 'C', "WAS");
    createMap(routing2, 18, ' ', 19, 0, 'C', "DFW");
    createMap(routing2, 19, 'X', 0, 0, 'C', "SFO");
    createMap(routing2, 20, '1', 21, 0, 'C', "SEA");
    createMap(routing2, 21, 'X', 0, 0, 'C', "FLL");
  }

  void testAddOn_OnSecondMapOnly_SEA_FLL()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "SEA", "AA", "FLL");
    CPPUNIT_ASSERT_MESSAGE(
        "SEA_FLL",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOn_ReversedOnSecondMapOnly_SEA_FLL()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "FLL", "AA", "SEA");
    CPPUNIT_ASSERT_MESSAGE(
        "FLL_SEA",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_LAX_DFW_NYC_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "NYC");
    createSeg(travelRoute, "NYC", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-DFW-NYC-LON",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_NYC_DFW_CHI_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    // reversed origin addon and not reversed specified
    createSeg(travelRoute, "NYC", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "NYC-DFW-CHI-LON",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_LAX_DFW_CHI_LON_NYC()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    // travelRoute.travelRoute().clear();
    createSeg(travelRoute, "LAX", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LON");
    createSeg(travelRoute, "LON", "AA", "NYC");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-DFW-CHI-LON-NYC",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LAX-DFW-CHI-LON-NYC", 2, (int)mapInfo.missingCityIndex());
    CPPUNIT_ASSERT(!mapInfo.missingCarrier());
  }

  void testAddOnEntryExit_BOS_DFW_CHI_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "BOS", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "BOS-DFW-LON",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_LAX_SFO_CHI_NYC()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "SFO");
    createSeg(travelRoute, "SFO", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "NYC");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-SFO-CHI-NYC",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_DFW_NYC_WAS_SFO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    // not reversed origin addon and reversed specified
    createSeg(travelRoute, "DFW", "AA", "NYC");
    createSeg(travelRoute, "NYC", "AA", "WAS");
    createSeg(travelRoute, "WAS", "AA", "SFO");
    CPPUNIT_ASSERT_MESSAGE(
        "DFW-NYC-WAS-SFO",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_SFO_LAX_DFW_WAS()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "SFO", "AA", "LAX");
    createSeg(travelRoute, "LAX", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "WAS");
    CPPUNIT_ASSERT_MESSAGE(
        "SFO-LAX-DFW-WAS",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_DFW_NYC_CHI_SFO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "DFW", "AA", "NYC");
    createSeg(travelRoute, "NYC", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "SFO");
    CPPUNIT_ASSERT_MESSAGE(
        "DFW-NYC-CHI-SFO",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_NYC_DFW_CHI_WAS()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    // reversed origin addon and reversed specified
    createSeg(travelRoute, "NYC", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "WAS");
    CPPUNIT_ASSERT_MESSAGE(
        "NYC-DFW-CHI-WAS",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_LAX_SFO_DFW_LAX()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "SFO");
    createSeg(travelRoute, "SFO", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "LAX");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-SFO-DFW-LAX",
        !validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnEntryExit_LAX_SFO_DFW_WAS()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnEntryExitSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "SFO");
    createSeg(travelRoute, "SFO", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "WAS");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-SFO-DFW-WAS",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void addOnCommonPointSetup(Routing& routing, Routing& routing2)
  {
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 4;
    routing.routing() = "0129";
    routing.commonPointInd() = ' ';
    createMap(routing, 1, '1', 2, 0, 'C', "SFO");
    createMap(routing, 2, ' ', 3, 0, 'C', "LAX");
    createMap(routing, 3, ' ', 4, 5, 'C', "DFW");
    createMap(routing, 4, 'X', 0, 0, 'C', "NYC");
    createMap(routing, 5, 'X', 0, 0, 'C', "CHI");

    routing2.vendor() = "ATP";
    routing2.carrier() = "AA";
    routing2.routingTariff() = 4;
    routing2.routing() = "0123";
    routing2.commonPointInd() = ' ';
    createMap(routing2, 1, '1', 2, 0, 'C', "NYC");
    createMap(routing2, 2, 'X', 0, 0, 'C', "LON");
    createMap(routing2, 3, '1', 4, 0, 'C', "LON");
    createMap(routing2, 4, ' ', 5, 0, 'C', "CHI");
    createMap(routing2, 5, ' ', 6, 0, 'C', "DFW");
    createMap(routing2, 6, 'X', 0, 0, 'C', "LAX");
  }

  void testAddOnCommonPoint_LAX_DFW_CHI_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnCommonPointSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-DFW-CHI-LON",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnCommonPoint_LAX_DFW_NYC_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnCommonPointSetup(routing, routing2);

    createSeg(travelRoute, "LAX", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "NYC");
    createSeg(travelRoute, "NYC", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "LAX-DFW-NYC-LON",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnCommonPoint_NYC_DFW_CHI_LON()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnCommonPointSetup(routing, routing2);

    // reversed origin addon and not reversed specified
    createSeg(travelRoute, "NYC", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LON");
    CPPUNIT_ASSERT_MESSAGE(
        "NYC-DFW-CHI-LON",
        validator.validate(
            _trx, travelRoute, &routing2, &mapInfo, DateTime::localTime(), &routing));
  }

  void testAddOnCommonPoint_DFW_CHI_LAX_SFO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnCommonPointSetup(routing, routing2);

    // not reversed origin addon and reversed specified
    createSeg(travelRoute, "DFW", "AA", "CHI");
    createSeg(travelRoute, "CHI", "AA", "LAX");
    createSeg(travelRoute, "LAX", "AA", "SFO");
    CPPUNIT_ASSERT_MESSAGE(
        "DFW-CHI-LAX-SFO",
        validator.validate(
            _trx, travelRoute, &routing, &mapInfo, DateTime::localTime(), &routing2));
  }

  void testAddOnCommonPoint_LON_CHI_DFW_LAX_SFO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routing;
    Routing routing2;
    addOnCommonPointSetup(routing, routing2);

    // reversed origin addon and reversed specified
    createSeg(travelRoute, "LON", "AA", "NYC");
    createSeg(travelRoute, "NYC", "AA", "DFW");
    createSeg(travelRoute, "DFW", "AA", "LAX");
    createSeg(travelRoute, "LAX", "AA", "SFO");
    CPPUNIT_ASSERT_MESSAGE(
        "LON-CHI-DFW-LAX-SFO",
        validator.validate(
            _trx, travelRoute, &routing, &mapInfo, DateTime::localTime(), &routing2));
  }

  void testFirstCityValidation()
  {
    const DateTime travelDate = DateTime::localTime();
    SpecifiedRoutingValidator validator;
    MapInfo mapInfo;

    Diagnostic& diag = _trx.diagnostic();
    diag.diagnosticType() = Diagnostic455;

    Routing routingBase;
    routingBase.vendor() = "ATP";
    routingBase.carrier() = "TN";
    routingBase.routingTariff() = 8;
    routingBase.routing() = "0002";
    routingBase.commonPointInd() = ' ';

    // TYO-JL-OSA/SPK/FUK/NGO/OKA
    // OSA-JL-FUK/TYO/SPK/OKA
    createMap(routingBase, 1, '1', 3, 0, 'C', "TYO");
    createMap(routingBase, 2, '1', 4, 0, 'C', "OSA");
    createMap(routingBase, 3, ' ', 5, 0, 'A', "JL");
    createMap(routingBase, 4, ' ', 10, 0, 'A', "JL");
    createMap(routingBase, 5, 'X', 0, 6, 'C', "OSA");
    createMap(routingBase, 6, 'X', 0, 7, 'C', "SPK");
    createMap(routingBase, 7, 'X', 0, 8, 'C', "FUK");
    createMap(routingBase, 8, 'X', 0, 9, 'C', "NGO");
    createMap(routingBase, 9, 'X', 0, 0, 'C', "OKA");
    createMap(routingBase, 10, 'X', 0, 11, 'C', "FUK");
    createMap(routingBase, 11, 'X', 0, 12, 'C', "TYO");
    createMap(routingBase, 12, 'X', 0, 13, 'C', "NGO");
    createMap(routingBase, 13, 'X', 0, 0, 'C', "OKA");

    Routing routingDestAddon;
    routingDestAddon.vendor() = "ATP";
    routingDestAddon.carrier() = "TN";
    routingDestAddon.routingTariff() = 8;
    routingDestAddon.routing() = "0005";
    routingDestAddon.commonPointInd() = ' ';
    // PPT-AKL
    // PPT-TYO/OSA
    createMap(routingDestAddon, 1, '1', 3, 0, 'C', "PPT");
    createMap(routingDestAddon, 2, ' ', 5, 0, 'C', "PPT");
    createMap(routingDestAddon, 3, 'X', 0, 4, 'C', "TYO");
    createMap(routingDestAddon, 4, 'X', 0, 0, 'C', "OSA");
    createMap(routingDestAddon, 5, 'X', 0, 0, 'C', "AKL");

    TravelRoute travelRoute;

    // we cannot let on direct validation FUK-TN-PPT
    createSeg(travelRoute, "FUK", "TN", "PPT");
    CPPUNIT_ASSERT_MESSAGE(
        "FUK-TN-PPT",
        !validator.validate(
            _trx, travelRoute, &routingBase, &mapInfo, travelDate, NULL, &routingDestAddon));

    travelRoute.travelRoute().clear();
    createSeg(travelRoute, "FUK", "JL", "OSA");
    createSeg(travelRoute, "OSA", "TN", "PPT");
    CPPUNIT_ASSERT_MESSAGE(
        "FUK-JL-OSA-TN-PPT",
        validator.validate(
            _trx, travelRoute, &routingBase, &mapInfo, travelDate, NULL, &routingDestAddon));
  }

  void overflownCitiesSetUp(Routing& routingBase, TravelRoute& travelRoute)
  {
    routingBase.vendor() = "ATP";
    routingBase.carrier() = "AA";
    routingBase.routingTariff() = 8;
    routingBase.routing() = "0001";
    routingBase.commonPointInd() = ' ';

    createSeg(travelRoute, "CHI", "CO", "NYC");
    createSeg(travelRoute, "NYC", "AA", "MIA");
  }

  void testOverflownCities_CHI_CO_WAS_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'A', "CO");
    createMap(routingBase, 3, ' ', 4, 0, 'C', "WAS");
    createMap(routingBase, 4, ' ', 5, 0, 'A', "CO");
    createMap(routingBase, 5, ' ', 6, 0, 'C', "NYC");
    createMap(routingBase, 6, ' ', 7, 0, 'A', "AA");
    createMap(routingBase, 7, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-CO-[WAS]-CO-NYC-AA-MIA,   []-overflown city",
        validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_AA_WAS_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    routingBase.routing() = "0002";
    routingBase.carrier() = "CO";
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'A', "AA");
    createMap(routingBase, 3, ' ', 4, 0, 'C', "WAS");
    createMap(routingBase, 4, ' ', 5, 0, 'A', "CO");
    createMap(routingBase, 5, ' ', 6, 0, 'C', "NYC");
    createMap(routingBase, 6, ' ', 7, 0, 'A', "AA");
    createMap(routingBase, 7, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-AA-[WAS]-CO-NYC-AA-MIA,   []-we can't overflown "
        "city because AA != CO on first segment",
        !validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void overflownCitiesMakeMap(Routing& routingBase)
  {
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'C', "WAS");
    createMap(routingBase, 3, ' ', 4, 0, 'A', "CO");
    createMap(routingBase, 4, ' ', 5, 0, 'C', "NYC");
    createMap(routingBase, 5, ' ', 6, 0, 'A', "AA");
    createMap(routingBase, 6, 'X', 0, 0, 'C', "MIA");
  }

  void testOverflownCities_CHI_WAS_CO_NYC_AA_MIA_carrierCO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    overflownCitiesMakeMap(routingBase);
    routingBase.routing() = "0003";
    routingBase.carrier() = "CO";

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-[WAS]-CO-NYC-AA-MIA,   []-overflown city",
        validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_WAS_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);
    overflownCitiesMakeMap(routingBase);
    routingBase.routing() = "0004";

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-[WAS]-CO-NYC-AA-MIA,   []-we can't overflown city "
        "because governing AA <> CO",
        !validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_CO_NYC_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    routingBase.routing() = "0005";
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'A', "CO");
    createMap(routingBase, 3, ' ', 4, 0, 'C', "NYC");
    createMap(routingBase, 4, ' ', 5, 0, 'A', "CO");
    createMap(routingBase, 5, ' ', 6, 0, 'C', "NYC");
    createMap(routingBase, 6, ' ', 7, 0, 'A', "AA");
    createMap(routingBase, 7, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-CO-[NYC]-CO-NYC-AA-MIA,   []-overflown city from "
        "itinerary",
        validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_AA_NYC_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    routingBase.routing() = "0006";
    routingBase.carrier() = "CO";
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'A', "AA");
    createMap(routingBase, 3, ' ', 4, 0, 'C', "NYC");
    createMap(routingBase, 4, ' ', 5, 0, 'A', "CO");
    createMap(routingBase, 5, ' ', 6, 0, 'C', "NYC");
    createMap(routingBase, 6, ' ', 7, 0, 'A', "AA");
    createMap(routingBase, 7, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-AA-[NYC]-CO-NYC-AA-MIA,   []-we can't overflown "
        "city because AA != CO on first segment",
        !validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_NYC_CO_NYC_AA_MIA_carrierCO()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    routingBase.routing() = "0007";
    routingBase.carrier() = "CO";
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'C', "NYC");
    createMap(routingBase, 3, ' ', 4, 0, 'A', "CO");
    createMap(routingBase, 4, ' ', 5, 0, 'C', "NYC");
    createMap(routingBase, 5, ' ', 6, 0, 'A', "AA");
    createMap(routingBase, 6, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-[NYC]-CO-NYC-AA-MIA,   []-overflown city from "
        "itinerary",
        validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void testOverflownCities_CHI_NYC_CO_NYC_AA_MIA()
  {
    SpecifiedRoutingValidator validator;
    TravelRoute travelRoute;
    _trx.diagnostic().diagnosticType() = Diagnostic455;
    MapInfo mapInfo;
    Routing routingBase;
    overflownCitiesSetUp(routingBase, travelRoute);

    routingBase.routing() = "0009";
    routingBase.carrier() = "AA";
    createMap(routingBase, 1, '1', 2, 0, 'C', "CHI");
    createMap(routingBase, 2, ' ', 3, 0, 'C', "NYC");
    createMap(routingBase, 3, ' ', 4, 0, 'A', "CO");
    createMap(routingBase, 4, ' ', 5, 0, 'C', "NYC");
    createMap(routingBase, 5, ' ', 6, 0, 'A', "AA");
    createMap(routingBase, 6, 'X', 0, 0, 'C', "MIA");

    CPPUNIT_ASSERT_MESSAGE(
        "Itin: CHI-CO-NYC-AA-MIA,   routing: CHI-[NYC]-CO-NYC-AA-MIA,   []-we can't overflown city "
        "because governing AA <> CO",
        !validator.validate(_trx, travelRoute, &routingBase, &mapInfo, DateTime::localTime()));
  }

  void createMap(Routing& routing,
                 int loc1No,
                 Indicator loctag,
                 int nextLocNo,
                 int altLocNo,
                 char loc1Type,
                 const char* loc1Code)
  {
    RoutingMap* map = new RoutingMap();
    map->vendor() = routing.vendor();
    map->carrier() = routing.carrier();
    map->routingTariff() = routing.routingTariff();
    map->routing() = routing.routing();
    map->effDate() = DateTime::localTime();
    map->lnkmapsequence() = loc1No;
    map->loc1No() = loc1No;
    map->loctag() = loctag;
    map->nextLocNo() = nextLocNo;
    map->altLocNo() = altLocNo;
    map->loc1().locType() = loc1Type;
    map->loc1().loc() = loc1Code;
    map->localRouting() = "";
    routing.rmaps().push_back(map);
  }

  void createSeg(TravelRoute& travelRoute,
                 const char* boardCity,
                 const char* carrier,
                 const char* offCity,
                 bool stopover = false)
  {
    TravelRoute::CityCarrier cc;
    cc.boardCity().loc() = boardCity;
    cc.boardCity().isHiddenCity() = false; // don't care really
    cc.offCity().loc() = offCity;
    cc.offCity().isHiddenCity() = false; // don't care really
    cc.carrier() = carrier;
    cc.stopover() = stopover;
    travelRoute.travelRoute().push_back(cc);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecifiedRoutingValidatorTest);
}
