#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/include/MockTseServer.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include <iostream>
#include <set>
#include <vector>

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  Mileage _ret;
  Mileage* getMil(int mileage)
  {
    _ret.mileage() = mileage;
    _ret.vendor() = "IATA";
    _ret.mileageType() = 'T';
    return &_ret;
  }

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (origin == "" && dest == "")
      return 0;
    else if (origin == "GLA" && dest == "LON")
      return getMil(345);
    else if (origin == "LON" && dest == "BRU")
      return getMil(203);
    else if (origin == "BRU" && dest == "ZRH")
      return getMil(309);
    else if (origin == "ZRH" && dest == "ROM")
      return getMil(435);
    else if (origin == "ROM" && dest == "ATH")
      return getMil(667);

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
}
class GoverningCarrierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GoverningCarrierTest);
  CPPUNIT_TEST(testGovCxrWithFmDirection);
  CPPUNIT_TEST(testSameSubAreaGovCxLogic);
  CPPUNIT_TEST(testHighestTPMCarrier);
  CPPUNIT_TEST(testselectGovCxrWithinUSCA);
  CPPUNIT_TEST(testGovCxrWithinSameCountryExceptUSCA);
  CPPUNIT_TEST(testGovCxrWithinSameSubIATA21);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessForEurope);
  CPPUNIT_TEST(testProcessOneSegment);
  CPPUNIT_TEST(testselectGovCxrWithinSameIATA);
  CPPUNIT_TEST(testselectGovCxrWithinSameIATA_Area1);
  CPPUNIT_TEST(testselectGovCxrWithinSameIATAInbound);
  CPPUNIT_TEST(testselectGovCxrWithinTwoIATA);
  CPPUNIT_TEST(testselectGovCxrWithinAllIATA);
  CPPUNIT_TEST(testselectGovCxrWithinSubIATA11);
  CPPUNIT_TEST(testselectGovCxrWithinSameSubIATA);
  CPPUNIT_TEST(testTravelBoundary);
  CPPUNIT_TEST(testselectTravelBoundary);
  CPPUNIT_TEST(testItinTicketingCarrierWithinSameIATA);
  CPPUNIT_TEST(testItinTicketingCarrierWithinAllIATA);
  CPPUNIT_TEST(testGetGoverningCarrierWithinSameCountry);
  CPPUNIT_TEST(testGetGoverningCarrierWithinEurope);
  CPPUNIT_TEST(testSameSubIATA1);
  CPPUNIT_TEST(testgetGoverningCarrier1);
  CPPUNIT_TEST(testgetGoverningCarrier2);
  CPPUNIT_TEST(testgetGoverningCarrier3);
  CPPUNIT_TEST(testgetGoverningCarrier4);
  CPPUNIT_TEST(testgetGoverningCarrier5);
  CPPUNIT_TEST(testisDomestic);
  CPPUNIT_TEST(testprimarySector);
  CPPUNIT_TEST(testgetFirstInternationalFlight);
  CPPUNIT_TEST(testGetGovCxrUSCAMEX);
  CPPUNIT_TEST_SUITE_END();

public:
  void testProcessForEurope()
  {
    FareMarket fm;
    fm.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA21, true);
    GoverningCarrier govCxr;
    bool rc = govCxr.process(fm);
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testHighestTPMCarrier()
  {
    MyDataHandle mdh;
    FareMarket fm;
    GoverningCarrier govCxr;
    AirSeg aS1, aS2, aS3, aS4, aS5;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("GLA", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("LON", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("BRU", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("ZRH", DateTime::localTime());
    const Loc* loc5 = dataHandle.getLoc("ROM", DateTime::localTime());
    const Loc* loc6 = dataHandle.getLoc("ATH", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    if (loc1 && loc2 && loc3 && loc4 && loc5 && loc6)
    {
      aS1.origin() = loc1;
      aS1.destination() = loc2;
      aS2.origin() = loc2;
      aS2.destination() = loc3;
      aS3.origin() = loc3;
      aS3.destination() = loc4;
      aS4.origin() = loc4;
      aS4.destination() = loc5;

      aS5.origin() = loc5;
      aS5.destination() = loc6;

      aS1.departureDT() = DateTime::localTime();
      aS2.departureDT() = DateTime::localTime();
      aS3.departureDT() = DateTime::localTime();
      aS4.departureDT() = DateTime::localTime();
      aS5.departureDT() = DateTime::localTime();

      aS1.carrier() = "BA";
      aS2.carrier() = "BA";
      aS3.carrier() = "SR";
      aS4.carrier() = "AZ";
      aS5.carrier() = "AZ";

      //----------------------
      // create the FareMarket
      //---------------------

      fm.origin() = loc1;
      fm.destination() = loc6;

      fm.travelSeg().push_back(&aS1);
      fm.travelSeg().push_back(&aS2);
      fm.travelSeg().push_back(&aS3);
      fm.travelSeg().push_back(&aS4);
      fm.travelSeg().push_back(&aS5);
      fm.setGlobalDirection(GlobalDirection::EH);

      std::set<CarrierCode> govCxrSet;

      CarrierCode carrier = govCxr.getFirstIntlFlt(fm.travelSeg());

      CPPUNIT_ASSERT_EQUAL((std::string) "BA", (std::string)carrier);

      carrier = govCxr.getHighestTPMCarrierOld(fm.travelSeg());

      CPPUNIT_ASSERT_EQUAL((std::string) "AZ", (std::string)carrier);
      bool rc = govCxr.getGoverningCarrier(fm.travelSeg(), govCxrSet);
      CPPUNIT_ASSERT_EQUAL((size_t)2, govCxrSet.size());

      CPPUNIT_ASSERT_EQUAL(true, rc);
    }
    else
    {
      std::cerr << "----------------- Test Failed Due to Database Error ---------------"
                << std::endl;
    }
  }

  void testgetGoverningCarrier1()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("US");
    loc2.nation() = ("US");
    loc3.nation() = ("US");
    loc4.nation() = ("US");
    loc5.nation() = ("CA");
    loc6.nation() = ("CA");

    loc1.area() = ("1");
    loc2.area() = ("1");
    loc3.area() = ("1");
    loc4.area() = ("1");
    loc5.area() = ("1");
    loc6.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("11");
    loc3.subarea() = ("11");
    loc4.subarea() = ("11");
    loc5.subarea() = ("11");
    loc6.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    std::set<CarrierCode> govCxrSet;
    std::vector<TravelSeg*> tvlSegs;

    tvlSegs.push_back(&airSeg1);
    tvlSegs.push_back(&airSeg2);
    tvlSegs.push_back(&airSeg3);
    tvlSegs.push_back(&airSeg4);
    tvlSegs.push_back(&airSeg5);

    GoverningCarrier GovCxr;

    bool result = GovCxr.getGoverningCarrier(tvlSegs, govCxrSet);

    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testgetGoverningCarrier2()
  {
    // selectWithinTwoIATA
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;

    loc1.nation() = ("US");
    loc2.nation() = ("TS");
    loc3.nation() = ("BS");
    loc4.nation() = ("US");

    loc1.area() = ("1");
    loc2.area() = ("2");
    loc3.area() = ("2");
    loc4.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("21");
    loc3.subarea() = ("21");
    loc4.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");

    std::set<CarrierCode> govCxrSet;
    std::vector<TravelSeg*> tvlSegs;

    tvlSegs.push_back(&airSeg1);
    tvlSegs.push_back(&airSeg2);
    tvlSegs.push_back(&airSeg3);

    GoverningCarrier GovCxr;

    bool result = GovCxr.getGoverningCarrier(tvlSegs, govCxrSet);

    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testgetGoverningCarrier3()
  {
    // testWithinAll_IATA()
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;

    loc1.nation() = ("UK");
    loc2.nation() = ("TS");
    loc3.nation() = ("BS");
    loc4.nation() = ("US");

    loc1.area() = ("2");
    loc2.area() = ("2");
    loc3.area() = ("3");
    loc4.area() = ("1");

    loc1.subarea() = ("21");
    loc2.subarea() = ("21");
    loc3.subarea() = ("31");
    loc4.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");

    std::set<CarrierCode> govCxrSet;
    std::vector<TravelSeg*> tvlSegs;

    tvlSegs.push_back(&airSeg1);
    tvlSegs.push_back(&airSeg2);
    tvlSegs.push_back(&airSeg3);

    PricingTrx trx;
    GoverningCarrier GovCxr(&trx);

    bool result = GovCxr.getGoverningCarrier(tvlSegs, govCxrSet);

    CPPUNIT_ASSERT_EQUAL(true, result);
    CPPUNIT_ASSERT_EQUAL(false, govCxrSet.empty());
  }

  void testgetGoverningCarrier4()
  {
    // testWithinSubIATA11()
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;

    loc1.nation() = ("US");
    loc2.nation() = ("TS");
    loc3.nation() = ("BS");
    loc4.nation() = ("US");

    loc1.area() = ("1");
    loc2.area() = ("1");
    loc3.area() = ("1");
    loc4.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("11");
    loc3.subarea() = ("11");
    loc4.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");

    std::set<CarrierCode> govCxrSet;
    std::vector<TravelSeg*> tvlSegs;

    tvlSegs.push_back(&airSeg1);
    tvlSegs.push_back(&airSeg2);
    tvlSegs.push_back(&airSeg3);

    GoverningCarrier GovCxr;

    bool result = GovCxr.getGoverningCarrier(tvlSegs, govCxrSet);

    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testgetGoverningCarrier5()
  {
    // test all error conditions
    Loc loc1;
    Loc loc2;
    Loc loc3;

    SurfaceSeg airSeg1;
    SurfaceSeg airSeg2;

    loc1.nation() = ("PS");
    loc2.nation() = ("TS");
    loc3.nation() = ("BS");

    loc1.area() = ("2");
    loc2.area() = ("2");
    loc3.area() = ("2");

    loc1.subarea() = ("21");
    loc2.subarea() = ("21");
    loc3.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    std::set<CarrierCode> govCxrSet;
    std::vector<TravelSeg*> tvlSegs;

    tvlSegs.push_back(&airSeg1);
    tvlSegs.push_back(&airSeg2);

    PricingTrx trx;
    trx.setIataFareSelectionApplicable(false);
    GoverningCarrier GovCxr(&trx);

    bool result = GovCxr.getGoverningCarrier(tvlSegs, govCxrSet);
    CPPUNIT_ASSERT_EQUAL(true, result);

    FareMarket faremkt;
    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc3);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    // error condition starts from here
    // all of them should fail as we dont have any AirSeg

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11, true);

    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(true, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA21, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA11, true);
    result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testSameSubIATA1()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("US");
    loc2.nation() = ("US");
    loc3.nation() = ("US");
    loc4.nation() = ("US");
    loc5.nation() = ("CA");
    loc6.nation() = ("MX");

    loc1.area() = ("1");
    loc2.area() = ("1");
    loc3.area() = ("1");
    loc4.area() = ("1");
    loc5.area() = ("1");
    loc6.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("11");
    loc3.subarea() = ("11");
    loc4.subarea() = ("11");
    loc5.subarea() = ("11");
    loc6.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);
    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11, true);
    GoverningCarrier GovCxr;

    bool result = GovCxr.process(faremkt);

    CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());

    CPPUNIT_ASSERT_EQUAL(true, result);

    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA21, true);

    result = GovCxr.process(faremkt);

    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testProcess()
  {
    try
    {
      FareMarket faremkt;
      PricingTrx trx;

      GoverningCarrier GovCxr;
      MockTseServer srv;

      TravelSegAnalysis itin;

      itin.selectTravelBoundary(faremkt.travelSeg());
      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL(false, result); // fareMarket doesnt have any data, should fail.
    }

    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testProcessOneSegment()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("US");
      loc2.nation() = ("US");
      loc3.nation() = ("US");
      loc4.nation() = ("US");
      loc5.nation() = ("CA");
      loc6.nation() = ("CA");

      loc1.area() = ("1");
      loc2.area() = ("1");
      loc3.area() = ("1");
      loc4.area() = ("1");
      loc5.area() = ("1");
      loc6.area() = ("1");

      loc1.subarea() = ("11");
      loc2.subarea() = ("11");
      loc3.subarea() = ("11");
      loc4.subarea() = ("11");
      loc5.subarea() = ("11");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);

      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA, true);

      GoverningCarrier GovCxr;

      MockTseServer srv;
      // ItinAnalyzerService itin("ITIN_SVC", srv);
      // itin.selectTravelBoundary(faremkt);
      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());

      CPPUNIT_ASSERT_EQUAL(true, result);
    }

    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testselectGovCxrWithinUSCA()
  {
    MyDataHandle mdh;
    Loc loc1;
    Loc loc2;
    Loc loc3;

    Loc loc4;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;

    try
    {
      loc1.nation() = ("US");
      loc2.nation() = ("CA");
      loc3.nation() = ("US");
      loc4.nation() = ("CA");
      loc1.area() = ("2");
      loc2.area() = ("2");
      loc3.area() = ("3");
      loc4.area() = ("1");
      loc1.subarea() = ("31");
      loc2.subarea() = ("31");
      loc3.subarea() = ("31");
      loc4.subarea() = ("31");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);
      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);
      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc4);
      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA, true);

      PricingTrx trx;
      GoverningCarrier GovCxr;

      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)3, faremkt.travelSeg().size());

      CPPUNIT_ASSERT_EQUAL(true, result);
    }

    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testselectGovCxrWithinSameIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("US");
      loc2.nation() = ("BG");
      loc3.nation() = ("US");
      loc4.nation() = ("DK");
      loc5.nation() = ("CA");
      loc6.nation() = ("CA");

      loc1.area() = ("1");

      loc2.area() = ("1");
      loc3.area() = ("1");
      loc4.area() = ("1");
      loc5.area() = ("1");
      loc6.area() = ("1");

      loc1.subarea() = ("11");
      loc2.subarea() = ("11");
      loc3.subarea() = ("11");
      loc4.subarea() = ("21");
      loc5.subarea() = ("21");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA, true);

      PricingTrx trx;
      GoverningCarrier GovCxr;

      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
      CPPUNIT_ASSERT_EQUAL(true, result);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  // Third Method
  void testselectGovCxrWithinTwoIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;

    FareMarket faremkt;
    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;

    try
    {
      loc1.nation() = ("TA");
      loc2.nation() = ("LK");
      loc3.nation() = ("US");
      loc4.nation() = ("FP");
      loc1.area() = ("1");
      loc2.area() = ("2");
      loc3.area() = ("1");
      loc4.area() = ("2");
      loc1.subarea() = ("11");
      loc2.subarea() = ("21");
      loc3.subarea() = ("31");
      loc4.subarea() = ("21");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);
      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);
      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc4);
      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA, true);

      GoverningCarrier GovCxr;
      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)3, faremkt.travelSeg().size());

      CPPUNIT_ASSERT_EQUAL(true, result);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  // 4th Method
  void testselectGovCxrWithinAllIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {
      loc1.nation() = ("TL");
      loc2.nation() = ("FP");

      loc3.nation() = ("SG");
      loc4.nation() = ("LA");
      loc5.nation() = ("FA");
      loc6.nation() = ("FG");

      loc1.area() = ("2");
      loc2.area() = ("3");
      loc3.area() = ("2");
      loc4.area() = ("1");
      loc5.area() = ("3");
      loc6.area() = ("3");

      loc1.subarea() = ("11");

      loc2.subarea() = ("21");
      loc3.subarea() = ("33");
      loc4.subarea() = ("11");
      loc5.subarea() = ("32");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);
      faremkt.direction() = FMDirection::UNKNOWN;
      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA, true);

      GoverningCarrier GovCxr;

      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
      CPPUNIT_ASSERT_EQUAL(true, result);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  // method 6
  void testselectGovCxrWithinSameSubIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("US");
    loc2.nation() = ("CA");
    loc3.nation() = ("US");
    loc4.nation() = ("CA");
    loc5.nation() = ("CA");
    loc6.nation() = ("US");

    loc1.area() = ("1");
    loc2.area() = ("1");
    loc3.area() = ("1");
    loc4.area() = ("1");
    loc5.area() = ("1");
    loc6.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("11");
    loc3.subarea() = ("11");
    loc4.subarea() = ("11");
    loc5.subarea() = ("11");
    loc6.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);
    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);
    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA, true);
    MockTseServer srv;
    TravelSegAnalysis itin2;
    Boundary tvlboundary = itin2.selectTravelBoundary(faremkt.travelSeg());

    ItinAnalyzerService itin("ITIN_SVC", srv);
    ItinUtil::setGeoTravelType(tvlboundary, faremkt);
    GoverningCarrier GovCxr;

    bool result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
    CPPUNIT_ASSERT_EQUAL((std::string) "BA", (std::string)faremkt.governingCarrier());
    CPPUNIT_ASSERT_EQUAL(true, result);

    faremkt.direction() = FMDirection::INBOUND;

    bool result2 = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL((std::string) "SW", (std::string)faremkt.governingCarrier());
    CPPUNIT_ASSERT_EQUAL(true, result2);
  }

  // Test the process for different travel boundary
  void testTravelBoundary()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;
    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("TL");
      loc2.nation() = ("FP");
      loc3.nation() = ("LS");
      loc4.nation() = ("CA");
      loc5.nation() = ("GB");
      loc6.nation() = ("CA");

      loc1.area() = ("3");
      loc2.area() = ("2");
      loc3.area() = ("2");
      loc4.area() = ("1");
      loc5.area() = ("3");
      loc6.area() = ("1");

      loc1.subarea() = ("31");
      loc2.subarea() = ("21");
      loc3.subarea() = ("33");
      loc4.subarea() = ("11");
      loc5.subarea() = ("32");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);

      MockTseServer srv;
      TravelSegAnalysis itin2;
      Boundary tvlboundary = itin2.selectTravelBoundary(faremkt.travelSeg());
      ItinAnalyzerService itin("ITIN_SVC", srv);
      ItinUtil::setGeoTravelType(tvlboundary, faremkt);

      PricingTrx trx;
      trx.setIataFareSelectionApplicable(false);
      GoverningCarrier GovCxr(&trx);

      std::set<CarrierCode> govCxrSet;
      bool result = GovCxr.process(faremkt);
      bool result1 = GovCxr.getGoverningCarrier(faremkt.travelSeg(), govCxrSet);
      CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());

      CPPUNIT_ASSERT_EQUAL(true, result);
      CPPUNIT_ASSERT_EQUAL(true, result1);
      CPPUNIT_ASSERT_EQUAL((size_t)1, govCxrSet.size());
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  // method 5
  void testselectGovCxrWithinSubIATA11()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("US");
      loc2.nation() = ("US");
      loc3.nation() = ("US");
      loc4.nation() = ("US");
      loc5.nation() = ("CA");
      loc6.nation() = ("CA");

      loc1.area() = ("1");
      loc2.area() = ("1");
      loc3.area() = ("1");
      loc4.area() = ("1");
      loc5.area() = ("1");
      loc6.area() = ("1");

      loc1.subarea() = ("11");
      loc2.subarea() = ("11");
      loc3.subarea() = ("11");
      loc4.subarea() = ("11");
      loc5.subarea() = ("11");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);

      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA11, true);

      PricingTrx trx;
      MockTseServer srv;

      TravelSegAnalysis itin2;
      Boundary tvlboundary = itin2.selectTravelBoundary(faremkt.travelSeg());
      ItinAnalyzerService itin("ITIN_SVC", srv);
      ItinUtil::setGeoTravelType(tvlboundary, faremkt);

      GoverningCarrier GovCxr;

      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
      CPPUNIT_ASSERT_EQUAL(true, result);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  //-----------------------
  // testTravelBoundary()
  //----------------------
  void testselectTravelBoundary()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("LA");
    loc2.nation() = ("TA");
    loc3.nation() = ("US");
    loc4.nation() = ("GA");
    loc5.nation() = ("KL");
    loc6.nation() = ("TA");

    loc1.area() = ("3");
    loc2.area() = ("2");
    loc3.area() = ("3");
    loc4.area() = ("3");
    loc5.area() = ("2");
    loc6.area() = ("2");

    loc1.subarea() = ("31");
    loc2.subarea() = ("21");
    loc3.subarea() = ("31");
    loc4.subarea() = ("31");
    loc5.subarea() = ("21");
    loc6.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);
    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    PricingTrx trx;
    GoverningCarrier GovCxr(&trx);
    MockTseServer srv;

    TravelSegAnalysis itin2;
    Boundary tvlboundary = itin2.selectTravelBoundary(faremkt.travelSeg());
    ItinAnalyzerService itin("ITIN_SVC", srv);
    ItinUtil::setGeoTravelType(tvlboundary, faremkt);

    bool result = GovCxr.process(faremkt);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "BA");
    CPPUNIT_ASSERT_EQUAL(true, result);
    faremkt.direction() = FMDirection::INBOUND;
    bool rc = GovCxr.process(faremkt);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "DL");
    CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testGovCxrWithinSameCountryExceptUSCA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("TA");
      loc2.nation() = ("TA");
      loc3.nation() = ("TA");
      loc4.nation() = ("TA");
      loc5.nation() = ("TA");
      loc6.nation() = ("TA");

      loc1.area() = ("3");
      loc2.area() = ("3");
      loc3.area() = ("3");
      loc4.area() = ("3");
      loc5.area() = ("3");
      loc6.area() = ("3");

      loc1.subarea() = ("31");
      loc2.subarea() = ("31");
      loc3.subarea() = ("31");
      loc4.subarea() = ("31");
      loc5.subarea() = ("31");
      loc6.subarea() = ("31");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("AA");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);

      PricingTrx trx;
      MockTseServer srv;
      TravelSegAnalysis itin2;

      GoverningCarrier GovCxr;

      Boundary tvlboundary = itin2.selectTravelBoundary(faremkt.travelSeg());
      ItinAnalyzerService itin("ITIN_SVC", srv);
      ItinUtil::setGeoTravelType(tvlboundary, faremkt);
      std::set<CarrierCode> govCxrSet;
      bool result = GovCxr.process(faremkt);
      bool result2 = GovCxr.getGoverningCarrier(faremkt.travelSeg(), govCxrSet);
      CPPUNIT_ASSERT(result2);
      CPPUNIT_ASSERT(govCxrSet.size() == 4);
      CPPUNIT_ASSERT(faremkt.travelSeg().size() == 5);
      CPPUNIT_ASSERT_EQUAL(true, result);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testGovCxrWithinSameSubIATA21()
  {
    MyDataHandle mdh;
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("TA");
    loc3.nation() = ("BL");
    loc4.nation() = ("TA");
    loc5.nation() = ("TA");
    loc6.nation() = ("IF");

    loc1.area() = ("2");
    loc2.area() = ("2");
    loc3.area() = ("2");
    loc4.area() = ("2");
    loc5.area() = ("2");
    loc6.area() = ("2");

    loc1.subarea() = ("21");
    loc2.subarea() = ("21");
    loc3.subarea() = ("21");
    loc4.subarea() = ("21");
    loc5.subarea() = ("21");
    loc6.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    PricingTrx trx;
    MockTseServer srv;
    ItinAnalyzerService itin("ITIN_SVC", srv);

    GoverningCarrier GovCxr(&trx);
    trx.setIataFareSelectionApplicable(false);
    std::set<CarrierCode> govCxrSet;
    bool result = GovCxr.getGoverningCarrier(faremkt.travelSeg(), govCxrSet);

    CPPUNIT_ASSERT_EQUAL((size_t)5, faremkt.travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testItinTicketingCarrierWithinSameIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("BA");
    loc3.nation() = ("BL");
    loc4.nation() = ("TA");
    loc5.nation() = ("TA");
    loc6.nation() = ("IF");

    loc1.area() = ("2");
    loc2.area() = ("2");
    loc3.area() = ("2");
    loc4.area() = ("2");
    loc5.area() = ("2");
    loc6.area() = ("2");

    loc1.subarea() = ("21");
    loc2.subarea() = ("21");
    loc3.subarea() = ("21");
    loc4.subarea() = ("21");
    loc5.subarea() = ("21");
    loc6.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    PricingTrx trx;
    MockTseServer srv;
    TravelSegAnalysis itin2;
    Itin itn;

    itn.travelSeg().push_back(&airSeg1);
    itn.travelSeg().push_back(&airSeg2);
    itn.travelSeg().push_back(&airSeg3);
    itn.travelSeg().push_back(&airSeg4);
    itn.travelSeg().push_back(&airSeg5);
    trx.itin().push_back(&itn);

    GoverningCarrier GovCxr;

    Boundary tvlboundary = itin2.selectTravelBoundary(itn.travelSeg());
    ItinAnalyzerService itin("ITIN_SVC", srv);
    ItinUtil::setGeoTravelType(itin2, tvlboundary, itn);
    itin.selectTicketingCarrier(trx);
    CPPUNIT_ASSERT(itn.ticketingCarrier() == "BA");
    CPPUNIT_ASSERT(itn.geoTravelType() == GeoTravelType::International);
  }

  void testItinTicketingCarrierWithinAllIATA()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("FA");
    loc3.nation() = ("BL");
    loc4.nation() = ("LA");
    loc5.nation() = ("MA");
    loc6.nation() = ("IF");

    loc1.area() = ("3");
    loc2.area() = ("2");
    loc3.area() = ("2");
    loc4.area() = ("1");
    loc5.area() = ("1");
    loc6.area() = ("2");

    loc1.subarea() = ("31");
    loc2.subarea() = ("21");
    loc3.subarea() = ("22");
    loc4.subarea() = ("11");
    loc5.subarea() = ("11");
    loc6.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("DL");
    airSeg5.carrier() = ("QF");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);
    GoverningCarrier GovCxr;
    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA, true);

    bool rc = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(true, rc);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "UA");
    faremkt.direction() = FMDirection::INBOUND;

    bool rc1 = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(true, rc1);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "QF");
  }

  void testGetGoverningCarrierWithinSameCountry()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("TA");
    loc3.nation() = ("TA");
    loc4.nation() = ("TA");
    loc5.nation() = ("TA");
    loc6.nation() = ("TA");

    loc1.area() = ("3");
    loc2.area() = ("3");
    loc3.area() = ("3");
    loc4.area() = ("3");
    loc5.area() = ("3");
    loc6.area() = ("3");

    loc1.subarea() = ("31");
    loc2.subarea() = ("31");
    loc3.subarea() = ("32");
    loc4.subarea() = ("31");
    loc5.subarea() = ("31");
    loc6.subarea() = ("31");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("BA");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("UA");
    airSeg4.carrier() = ("AA");
    airSeg5.carrier() = ("SW");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    GoverningCarrier GovCxr;
    std::set<CarrierCode> govCxrSet;
    bool result = GovCxr.getGoverningCarrier(faremkt.travelSeg(), govCxrSet);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(govCxrSet.size() == 4);
  }

  void testGetGoverningCarrierWithinEurope()
  {
    MyDataHandle mdh;
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("LA");
    loc3.nation() = ("TA");
    loc4.nation() = ("KA");
    loc5.nation() = ("KA");
    loc6.nation() = ("PA");

    loc1.area() = ("2");
    loc2.area() = ("2");
    loc3.area() = ("2");
    loc4.area() = ("2");
    loc5.area() = ("2");
    loc6.area() = ("2");

    loc1.subarea() = ("21");
    loc2.subarea() = ("21");
    loc3.subarea() = ("21");
    loc4.subarea() = ("21");
    loc5.subarea() = ("21");
    loc6.subarea() = ("21");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("PK");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("AA");
    airSeg4.carrier() = ("AA");
    airSeg5.carrier() = ("AA");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    PricingTrx trx;
    GoverningCarrier GovCxr(&trx);
    std::set<CarrierCode> govCxrSet;
    bool result = GovCxr.getGoverningCarrier(faremkt.travelSeg(), govCxrSet);
    CPPUNIT_ASSERT_EQUAL(true, result);
    CPPUNIT_ASSERT_EQUAL((size_t)2, govCxrSet.size());
  }

  void testisDomestic()
  {
    Loc loc1, loc2, loc3, loc4;
    AirSeg airSeg1, airSeg2;
    loc1.nation() = ("TA");
    loc2.nation() = ("LA");
    loc3.nation() = ("KA");
    loc4.nation() = ("KA");

    GoverningCarrier GovCxr;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);
    airSeg2.origin() = (&loc3);
    airSeg2.destination() = (&loc4);
    bool rc1 = GovCxr.isDomesticOld(airSeg1);
    bool rc2 = GovCxr.isDomesticOld(airSeg2);
    CPPUNIT_ASSERT(!rc1);
    CPPUNIT_ASSERT(rc2);
  }

  void testgetFirstInternationalFlight()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;
    FareMarket faremkt;

    loc1.nation() = ("TA");
    loc2.nation() = ("TA");
    loc3.nation() = ("TA");
    loc4.nation() = ("KA");
    loc5.nation() = ("KA");
    loc6.nation() = ("PA");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("PK");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("DL");
    airSeg4.carrier() = ("AA");
    airSeg5.carrier() = ("AA");
    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    GoverningCarrier GovCxr;
    CarrierCode rc = GovCxr.getFirstIntlFlt(faremkt.travelSeg());
    CPPUNIT_ASSERT(rc == "DL");

    // case2 : no international flight
    loc1.nation() = ("TA");
    loc2.nation() = ("TA");
    loc3.nation() = ("TA");
    loc4.nation() = ("TA");
    loc5.nation() = ("TA");
    loc6.nation() = ("TA");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("PK");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("DL");
    airSeg4.carrier() = ("AA");
    airSeg5.carrier() = ("AA");
    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    rc = GovCxr.getFirstIntlFlt(faremkt.travelSeg());
    CPPUNIT_ASSERT(rc == "XX");
  }

  void testGetGovCxrUSCAMEX()
  {
    FareMarket fm;
    GoverningCarrier govCxr;
    AirSeg aS1, aS2;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("YTO", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("NYC", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("MEX", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    aS1.origin() = loc1;
    aS1.destination() = loc2;
    aS2.origin() = loc2;
    aS2.destination() = loc3;

    aS1.departureDT() = DateTime::localTime();
    aS2.departureDT() = DateTime::localTime();

    aS1.carrier() = "AC";
    aS2.carrier() = "CO";

    //----------------------

    // create the FareMarket
    //---------------------

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    std::set<CarrierCode> govCxrSet;
    CPPUNIT_ASSERT(govCxrSet.empty() == true);
    bool rc = govCxr.getGoverningCarrier(tvlSegs, govCxrSet);
    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(govCxrSet.size() == 1);
    CarrierCode rc1 = govCxr.getFirstIntlFlt(tvlSegs);
    CPPUNIT_ASSERT(rc1 == "CO");
  }

  void testprimarySector()
  {
    GoverningCarrier govCxr;
    AirSeg aS1, aS2;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("NRT", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("PEK", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("HKG", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    aS1.origin() = loc1;
    aS1.destination() = loc2;
    aS2.origin() = loc2;
    aS2.destination() = loc3;

    aS1.departureDT() = DateTime::localTime();
    aS2.departureDT() = DateTime::localTime();

    aS1.carrier() = "CA";
    aS2.carrier() = "CA";

    //----------------------

    // create the FareMarket
    //---------------------

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    std::set<CarrierCode> govCxrSet;
    CPPUNIT_ASSERT(govCxrSet.empty() == true);
    bool rc = govCxr.getGoverningCarrier(tvlSegs, govCxrSet);
    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(govCxrSet.size() == 1);
    // CarrierCode rc1 = govCxr.getFirstIntlFlt(tvlSegs);
    // CPPUNIT_ASSERT(rc1=="CO");
  }

  void testselectGovCxrWithinSameIATAInbound()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    try
    {

      loc1.nation() = ("US");
      loc2.nation() = ("BG");
      loc3.nation() = ("US");
      loc4.nation() = ("DK");
      loc5.nation() = ("CA");
      loc6.nation() = ("CA");

      loc1.area() = ("1");

      loc2.area() = ("1");
      loc3.area() = ("1");
      loc4.area() = ("1");
      loc5.area() = ("1");
      loc6.area() = ("1");

      loc1.subarea() = ("11");
      loc2.subarea() = ("11");
      loc3.subarea() = ("11");
      loc4.subarea() = ("21");
      loc5.subarea() = ("21");
      loc6.subarea() = ("11");

      airSeg1.origin() = (&loc1);
      airSeg1.destination() = (&loc2);

      airSeg2.origin() = (&loc2);
      airSeg2.destination() = (&loc3);

      airSeg3.origin() = (&loc3);
      airSeg3.destination() = (&loc4);

      airSeg4.origin() = (&loc4);
      airSeg4.destination() = (&loc5);

      airSeg5.origin() = (&loc5);
      airSeg5.destination() = (&loc6);

      airSeg1.carrier() = ("BA");
      airSeg2.carrier() = ("AA");
      airSeg3.carrier() = ("UA");
      airSeg4.carrier() = ("DL");
      airSeg5.carrier() = ("SW");

      faremkt.origin() = (&loc1);
      faremkt.destination() = (&loc6);
      faremkt.direction() = FMDirection::INBOUND;

      faremkt.travelSeg().push_back(&airSeg1);
      faremkt.travelSeg().push_back(&airSeg2);
      faremkt.travelSeg().push_back(&airSeg3);
      faremkt.travelSeg().push_back(&airSeg4);
      faremkt.travelSeg().push_back(&airSeg5);
      faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA, true);

      PricingTrx trx;
      GoverningCarrier GovCxr;

      bool result = GovCxr.process(faremkt);

      CPPUNIT_ASSERT(faremkt.travelSeg().size() == 5);
      CPPUNIT_ASSERT(result);
      CPPUNIT_ASSERT(faremkt.governingCarrier() == "SW");
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testselectGovCxrWithinSameIATA_Area1()
  {
    // PL#6237
    GoverningCarrier govCxr;
    AirSeg aS1, aS2;

    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("BUE", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("RIO", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("MIA", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    aS1.origin() = loc1;
    aS1.destination() = loc2;
    aS2.origin() = loc2;
    aS2.destination() = loc3;

    aS1.departureDT() = DateTime::localTime();
    aS2.departureDT() = DateTime::localTime();

    aS1.carrier() = "JJ";
    aS2.carrier() = "AA";

    //----------------------

    // create the FareMarket
    //---------------------

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    std::set<CarrierCode> govCxrSet;
    std::set<CarrierCode>::iterator i = govCxrSet.end();
    CPPUNIT_ASSERT(govCxrSet.empty() == true);
    bool rc = govCxr.getGoverningCarrier(tvlSegs, govCxrSet);
    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(govCxrSet.size() == 1);
    i = govCxrSet.find("AA");
    CPPUNIT_ASSERT(i != govCxrSet.end());
  }

  void testSameSubAreaGovCxLogic()
  {
    Loc loc1;
    Loc loc2;
    Loc loc3;
    Loc loc4;
    Loc loc5;
    Loc loc6;

    FareMarket faremkt;

    AirSeg airSeg1;
    AirSeg airSeg2;
    AirSeg airSeg3;
    AirSeg airSeg4;
    AirSeg airSeg5;

    loc1.nation() = ("TA");
    loc2.nation() = ("TA");
    loc3.nation() = ("LA");
    loc4.nation() = ("PA");
    loc5.nation() = ("KA");
    loc6.nation() = ("KA");

    loc1.area() = ("1");
    loc2.area() = ("1");
    loc3.area() = ("1");
    loc4.area() = ("1");
    loc5.area() = ("1");
    loc6.area() = ("1");

    loc1.subarea() = ("11");
    loc2.subarea() = ("11");
    loc3.subarea() = ("11");
    loc4.subarea() = ("11");
    loc5.subarea() = ("11");
    loc6.subarea() = ("11");

    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    airSeg2.origin() = (&loc2);
    airSeg2.destination() = (&loc3);

    airSeg3.origin() = (&loc3);
    airSeg3.destination() = (&loc4);

    airSeg4.origin() = (&loc4);
    airSeg4.destination() = (&loc5);

    airSeg5.origin() = (&loc5);
    airSeg5.destination() = (&loc6);

    airSeg1.carrier() = ("PK");
    airSeg2.carrier() = ("AA");
    airSeg3.carrier() = ("AA");
    airSeg4.carrier() = ("QF");
    airSeg5.carrier() = ("BA");

    faremkt.origin() = (&loc1);
    faremkt.destination() = (&loc6);

    faremkt.travelSeg().push_back(&airSeg1);
    faremkt.travelSeg().push_back(&airSeg2);
    faremkt.travelSeg().push_back(&airSeg3);

    faremkt.travelSeg().push_back(&airSeg4);
    faremkt.travelSeg().push_back(&airSeg5);

    GoverningCarrier GovCxr;
    Boundary boundary = TravelSegAnalysis::selectTravelBoundary(faremkt.travelSeg());
    CPPUNIT_ASSERT(boundary == Boundary::AREA_11);
    faremkt.direction() = FMDirection::OUTBOUND;
    faremkt.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA11, true);

    bool rc = GovCxr.process(faremkt);
    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "AA");

    faremkt.direction() = FMDirection::INBOUND;
    bool rc1 = GovCxr.process(faremkt);
    CPPUNIT_ASSERT_EQUAL(true, rc1);
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "QF");
    std::set<CarrierCode> govCxrSet;
    bool rc2 = GovCxr.selectWithinSameSubIATA(faremkt.travelSeg(), govCxrSet, faremkt.direction());
    CPPUNIT_ASSERT_EQUAL(true, rc2);
    CPPUNIT_ASSERT_EQUAL(false, govCxrSet.empty());
    CPPUNIT_ASSERT(faremkt.governingCarrier() == "QF");
  }

  void testGovCxrWithFmDirection()
  {
    FareMarket fm;
    GoverningCarrier govCxr;
    AirSeg aS1, aS2, aS3;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("CCS", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("LIM", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("BOG", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("SJO", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    if (loc1 && loc2 && loc3 && loc4)
    {
      aS1.origin() = loc1;
      aS1.destination() = loc2;
      aS2.origin() = loc2;
      aS2.destination() = loc3;
      aS3.origin() = loc3;
      aS3.destination() = loc4;

      aS1.departureDT() = DateTime::localTime();
      aS2.departureDT() = DateTime::localTime();
      aS3.departureDT() = DateTime::localTime();

      aS1.carrier() = "TA";
      aS2.carrier() = "TA";
      aS3.carrier() = "LR";

      //----------------------
      // create the FareMarket
      //---------------------

      fm.origin() = loc1;
      fm.destination() = loc4;

      fm.travelSeg().push_back(&aS1);
      fm.travelSeg().push_back(&aS2);
      fm.travelSeg().push_back(&aS3);

      fm.setGlobalDirection(GlobalDirection::WH);
      fm.direction() = FMDirection::OUTBOUND;

      std::set<CarrierCode> govCxrSet;
      std::set<CarrierCode>::iterator i = govCxrSet.begin();
      bool rc = govCxr.getGoverningCarrier(fm.travelSeg(), govCxrSet);
      CPPUNIT_ASSERT_EQUAL(true, rc);
      CPPUNIT_ASSERT_EQUAL(false, govCxrSet.empty());
      i = govCxrSet.find("LR");
      CPPUNIT_ASSERT(i != govCxrSet.end());

      Boundary boundary = TravelSegAnalysis::selectTravelBoundary(fm.travelSeg());
      CPPUNIT_ASSERT(boundary == Boundary::ONE_IATA);
      fm.travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11, true);
      bool rc1 = govCxr.process(fm);
      CPPUNIT_ASSERT_EQUAL((std::string) "TA", (std::string)fm.governingCarrier());
      // TRY TO CHANGE THE DIRECTIONALITY
      fm.direction() = FMDirection::INBOUND;
      rc1 = govCxr.process(fm);
      CPPUNIT_ASSERT_EQUAL(true, rc1);
      CPPUNIT_ASSERT_EQUAL((std::string) "LR", (std::string)fm.governingCarrier());
    }
    else
    {
      std::cerr << "----------------- Test Failed Due to Database Error ---------------"
                << std::endl;
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(GoverningCarrierTest);
}
