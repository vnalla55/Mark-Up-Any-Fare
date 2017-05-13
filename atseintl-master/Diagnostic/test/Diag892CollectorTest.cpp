#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "DataModel/TNBrandsTypes.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "DBAccess/DataHandle.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandedFaresComparator.h"

using namespace std;
namespace tse
{
class Diag892CollectorTest : public Diag892Collector, public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag892CollectorTest);
  CPPUNIT_TEST(testprintBrands);
  CPPUNIT_TEST(testprintBrandsLongCodes);
  CPPUNIT_TEST(testprintFareMarketBrandsWithODCAndBrandsLongCodes);
  CPPUNIT_TEST(testprintFareMarketBrandsWithODCAndBrands);
  CPPUNIT_TEST(testprintFareMarketBrandsWithNoBrandsAndNoODC);
  CPPUNIT_TEST(testdisplayAllBrandIndices);
  CPPUNIT_TEST(testprintItinInfo);
  CPPUNIT_TEST(testprintContextShoppingItinInfo);
  CPPUNIT_TEST(testprintContextShoppingItinParityInfo);
  CPPUNIT_TEST(testprintContextShoppingItinParityInfoWithCurrentLegbrands);
  CPPUNIT_TEST(testprintRemovedFareMarkets);
  CPPUNIT_TEST(testprintBrandingOptionSpaces);
  CPPUNIT_TEST(testprintBrandingOptionSpacesLongCodes);
  CPPUNIT_TEST(testprintSegmentOrientedBrandCodes);
  CPPUNIT_TEST(testprintComparator);
  CPPUNIT_TEST(testbrandingOptionSpaceToString);
  CPPUNIT_TEST(testprintCarrierBrandAndCabinInfo);
  CPPUNIT_TEST(testprintDeduplicationInfo);
  CPPUNIT_TEST(testprintBrandsRemovedFromTrx);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {}
  virtual void tearDown() {}

  DataHandle dataHandle;

  Diag892CollectorTest() { _active = true; }

protected:
  void testprintBrandsLongCodes()
  {
    std::set<BrandCode> brands;
    for (size_t index = 0; index < 30; index++)
    {
      std::stringstream tmp;
      int cycle = 1;
      if (index % 2 == 0)
        cycle = 5;
      for (int i = 0; i < cycle; ++i)
          tmp << char(int('A') + (index / 10)) << ((29 - index) % 10);
      brands.insert(tmp.str());
    }
    printBrands(brands);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "A0,A1A1A1A1A1,A2,A3A3A3A3A3,A4,A5A5A5A5A5,A6,A7A7A7A7A7,A8,\n"
     "A9A9A9A9A9,B0,B1B1B1B1B1,B2,B3B3B3B3B3,B4,B5B5B5B5B5,B6,\n"
     "B7B7B7B7B7,B8,B9B9B9B9B9,C0,C1C1C1C1C1,C2,C3C3C3C3C3,C4,\n"
     "C5C5C5C5C5,C6,C7C7C7C7C7,C8,C9C9C9C9C9");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintBrands()
  {
    std::set<BrandCode> brands;
    for (size_t index = 0; index < 30; index++)
    {
      std::stringstream tmp;
      tmp << char(int('A') + (index / 10)) << ((29 - index) % 10);
      brands.insert(tmp.str());
    }
    printBrands(brands);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9,C0,\n"
     "C1,C2,C3,C4,C5,C6,C7,C8,C9");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintFareMarketBrandsWithODCAndBrandsLongCodes()
  {
    const size_t BRANDS = 30;

    PricingTrx trx;
    BrandProgram p;
    BrandInfo brands[BRANDS];
    FareMarket fm1;
    p.programID() = "123456";

    for (size_t index = 0; index < BRANDS; ++index)
    {
      std::stringstream tmp;
      int cycle = 1;
      if (index % 2 == 0)
        cycle = 5;
      for (int i = 0; i < cycle; ++i)
        tmp << char(int('A') + int(index / 10)) << ((BRANDS - 1 - index) % 10);
      brands[index].brandCode() = tmp.str();

      trx.brandProgramVec().push_back(std::make_pair(&p, &(brands[index])));
      if (index < 26)
      {
        fm1.brandProgramIndexVec().push_back(index);
      }
    }

    OdcsForBranding odcs;
    odcs[&fm1].insert(OdcTuple("XX", "AAA", "BBB"));
    odcs[&fm1].insert(OdcTuple("XX", "BBB", "CCC"));
    odcs[&fm1].insert(OdcTuple("XX", "CCC", "DDD"));
    odcs[&fm1].insert(OdcTuple("XX", "DDD", "EEE"));
    odcs[&fm1].insert(OdcTuple("XX", "EEE", "FFF"));

    Loc l1, l2;
    l1.loc() = "AAA";
    l2.loc() = "BBB";

    fm1.origin() = &l1;
    fm1.destination() = &l2;
    fm1.boardMultiCity() = "AAA";
    fm1.offMultiCity() = "BBB";
    fm1.governingCarrier() = "XX";

    trx.fareMarket().push_back(&fm1);

    BrandedDiagnosticUtil::displayFareMarketsWithBrands(*this, trx.fareMarket(),
                                                        trx.brandProgramVec(), &odcs);
    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "***** START FARE MARKET BRANDS *****\n"
     "FARE MARKET 1: AAA[AAA]-XX-BBB[BBB]\n"
     " BRANDS RETRIEVED USING ODCs: AAA-XX-BBB BBB-XX-CCC CCC-XX-DDD\n"
     "                              DDD-XX-EEE EEE-XX-FFF\n"
     " BRAND INDICES: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,\n"
     "                19,20,21,22,23,24,25\n"
     " BRANDS: 123456: A9A9A9A9A9,A8,A7A7A7A7A7,A6,A5A5A5A5A5,A4,\n"
     "                 A3A3A3A3A3,A2,A1A1A1A1A1,A0,B9B9B9B9B9,B8,\n"
     "                 B7B7B7B7B7,B6,B5B5B5B5B5,B4,B3B3B3B3B3,B2,\n"
     "                 B1B1B1B1B1,B0,C9C9C9C9C9,C8,C7C7C7C7C7,C6,\n"
     "                 C5C5C5C5C5,C4\n"
     "****** END FARE MARKET BRANDS ******\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintFareMarketBrandsWithODCAndBrands()
  {
    const size_t BRANDS = 30;

    PricingTrx trx;
    BrandProgram p;
    BrandInfo brands[BRANDS];
    FareMarket fm1;
    p.programID() = "123456";

    for (size_t index = 0; index < BRANDS; ++index)
    {
      std::stringstream tmp;
      tmp << char(int('A') + int(index / 10)) << ((BRANDS - 1 - index) % 10);
      brands[index].brandCode() = tmp.str();

      trx.brandProgramVec().push_back(std::make_pair(&p, &(brands[index])));
      if (index < 26)
      {
        fm1.brandProgramIndexVec().push_back(index);
      }
    }

    OdcsForBranding odcs;
    odcs[&fm1].insert(OdcTuple("XX", "AAA", "BBB"));
    odcs[&fm1].insert(OdcTuple("XX", "BBB", "CCC"));
    odcs[&fm1].insert(OdcTuple("XX", "CCC", "DDD"));
    odcs[&fm1].insert(OdcTuple("XX", "DDD", "EEE"));
    odcs[&fm1].insert(OdcTuple("XX", "EEE", "FFF"));

    Loc l1, l2;
    l1.loc() = "AAA";
    l2.loc() = "BBB";

    fm1.origin() = &l1;
    fm1.destination() = &l2;
    fm1.boardMultiCity() = "AAA";
    fm1.offMultiCity() = "BBB";
    fm1.governingCarrier() = "XX";

    trx.fareMarket().push_back(&fm1);

    BrandedDiagnosticUtil::displayFareMarketsWithBrands(*this, trx.fareMarket(),
                                                        trx.brandProgramVec(), &odcs);


    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "***** START FARE MARKET BRANDS *****\n"
     "FARE MARKET 1: AAA[AAA]-XX-BBB[BBB]\n"
     " BRANDS RETRIEVED USING ODCs: AAA-XX-BBB BBB-XX-CCC CCC-XX-DDD\n"
     "                              DDD-XX-EEE EEE-XX-FFF\n"
     " BRAND INDICES: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,\n"
     "                19,20,21,22,23,24,25\n"
     " BRANDS: 123456: A9,A8,A7,A6,A5,A4,A3,A2,A1,A0,B9,B8,B7,B6,B5,\n"
     "                 B4,B3,B2,B1,B0,C9,C8,C7,C6,C5,C4\n"
     "****** END FARE MARKET BRANDS ******\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintFareMarketBrandsWithNoBrandsAndNoODC()
  {
    PricingTrx trx;
    BrandProgram p;
    BrandInfo b0;
    b0.brandCode() = "B0";

    trx.brandProgramVec().push_back(std::make_pair(&p, &b0));

    Loc l1, l2;
    l1.loc() = "AAA";
    l2.loc() = "BBB";

    FareMarket fm1;
    fm1.origin() = &l1;
    fm1.destination() = &l2;
    fm1.boardMultiCity() = "AAA";
    fm1.offMultiCity() = "BBB";
    fm1.governingCarrier() = "XX";

    trx.fareMarket().push_back(&fm1);

    OdcsForBranding odcs;

    BrandedDiagnosticUtil::displayFareMarketsWithBrands(*this, trx.fareMarket(),
                                                        trx.brandProgramVec(), &odcs);


    string expected(
    //         1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "***** START FARE MARKET BRANDS *****\n"
     "FARE MARKET 1: AAA[AAA]-XX-BBB[BBB]\n"
     "! N/A !\n"
     " NO BRANDS FOUND\n"
     "****** END FARE MARKET BRANDS ******\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testdisplayAllBrandIndices()
  {
    PricingTrx trx;
    BrandProgram p0;
    p0.programID() = "123456";
    p0.programCode() = "CODE56";
    p0.systemCode() = '0';
    p0.programName() = "PROGRAM NAME";
    BrandProgram p1;
    p1.programID() = "123456789012345";
    p1.programCode() = "CODE567890";
    p1.systemCode() = '1';
                      //         1         2         3
                      //123456789012345678901234567890
    p1.programName() = "VERY LONG PROGRAM NAME 4567890";

    BrandInfo b0;
    b0.brandCode() = "A0";
    b0.brandName() = "BRAND NAME 123";
    BrandInfo b1;
    b1.brandCode() = "DEFINED AS STRING WITH NO RESTRICTIONS!";
                    //         1         2         3
                    //123456789012345678901234567890
    b1.brandName() = "VERY LONG BRAND NAME 234567890";

    trx.brandProgramVec().push_back(std::make_pair(&p0, &b0));
    trx.brandProgramVec().push_back(std::make_pair(&p1, &b1));

    BrandedDiagnosticUtil::displayAllBrandIndices(*this, trx.brandProgramVec());

    string expected(
        //         1         2         3         4         5         6         7
        //123456789012345678901234567890123456789012345678901234567890123456789012345678
        "***** START BRANDS INFORMATION *****\n"
        "RECEIVED PROGRAMS AND BRANDS:\n"
        "PROGRAM ID 123456:\n"
        " CODE=CODE56\n"
        " SYSTEM=0\n"
        " NAME=PROGRAM NAME\n"
        "PROGRAM ID 123456789012345:\n"
        " CODE=CODE567890\n"
        " SYSTEM=1\n"
        " NAME=VERY LONG PROGRAM NAME 4567890\n"
        "\n"
        "BRAND INDEX 0:\n"
        " PROGRAM ID=123456\n"
        " CODE=A0\n"
        " NAME=BRAND NAME 123\n"
        "BRAND INDEX 1:\n"
        " PROGRAM ID=123456789012345\n"
        " CODE=DEFINED AS STRING WITH NO RESTRICTIONS!\n"
        " NAME=VERY LONG BRAND NAME 234567890\n"
        "****** END BRANDS INFORMATION ******\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintItinInfo()
  {
    Itin itin;
    itin.itinNum() = 123;

    AirSeg air;
    air.origAirport() = "AAA";
    air.destAirport() = "BBB";

    ArunkSeg arung;
    arung.origAirport() = "CCC";
    arung.destAirport() = "DDD";

    std::vector<TravelSegPtrVec> legs;
    std::vector<tse::TravelSeg*> travel;
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&arung);
    legs.push_back(travel);

    itin.itinLegs() = legs;
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM9");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM2");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM3");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM4");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM5");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM6");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM7");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM8");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM1");
    itin.brandFilterMap()["OTHER BRAND"];

    printItinInfo(&itin, false);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "** PROCESSING ITIN 123 **\n"
     "\n"
     "LEG 0: AAA-BBB AAA-BBB AAA-BBB AAA-BBB\n"
     "LEG 1: AAA-BBB AAA-BBB AAA-BBB AAA-BBB\n"
     "LEG 2: AAA-BBB AAA-BBB AAA-BBB AAA-BBB\n"
     "LEG 3: CCC-DDD (ARUNK)\n"
     "\n"
     "REQUESTED BRAND FILTERS FOR ITIN 123:\n"
     "BRAND ID = BRAND NAME PROGRAM IDs : PROGRAM1 PROGRAM2 PROGRAM3\n"
     "                                    PROGRAM4 PROGRAM5 PROGRAM6\n"
     "                                    PROGRAM7 PROGRAM8 PROGRAM9\n"
     "BRAND ID = OTHER BRAND PROGRAM IDs : N/A\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintContextShoppingItinInfo()
  {
    Itin itin;
    itin.itinNum() = 123;

    PricingRequest request;
    request.setBrandedFaresRequest(true);
    request.setContextShoppingRequest();

    std::vector<bool> fixedLegs;
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);

    skipper::FareComponentShoppingContext segXX;
    segXX.brandCode = "XX";
    skipper::FareComponentShoppingContext segYY;
    segYY.brandCode = "YY";
    skipper::FareComponentShoppingContext segNONE;
    skipper::FareComponentShoppingContextsForSegments segMap;
    segMap[1] = &segXX;
    segMap[2] = &segNONE;
    segMap[5] = &segYY;
    //segMap[6] = &segYY; <- NO CONTEXT DATA expected

    PricingTrx trx;
    trx.setRequest(&request);
    trx.getMutableFixedLegs() = fixedLegs;
    trx.getMutableFareComponentShoppingContexts() = segMap;

    AirSeg air1, air2, air3, air4, air5, air6;
    air1.origAirport() = "AAA";
    air1.destAirport() = "BBB";
    air1.pnrSegment() = 1;
    air2.origAirport() = "AAA";
    air2.destAirport() = "BBB";
    air2.pnrSegment() = 2;
    air3.origAirport() = "AAA";
    air3.destAirport() = "BBB";
    air3.pnrSegment() = 3;
    air4.origAirport() = "AAA";
    air4.destAirport() = "BBB";
    air4.pnrSegment() = 4;
    air5.origAirport() = "AAA";
    air5.destAirport() = "BBB";
    air5.pnrSegment() = 5;
    air6.origAirport() = "AAA";
    air6.destAirport() = "BBB";
    air6.pnrSegment() = 6;

    ArunkSeg arung;
    arung.origAirport() = "CCC";
    arung.destAirport() = "DDD";

    std::vector<TravelSegPtrVec> legs;
    std::vector<tse::TravelSeg*> travel;
    travel.push_back(&air1); // <- XX
    travel.push_back(&air2); // <- no brand set
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&air3);
    travel.push_back(&air4);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&air5); // <- YY
    travel.push_back(&air6); // <- no data
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&arung);
    legs.push_back(travel);

    itin.itinLegs() = legs;
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM9");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM2");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM3");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM4");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM5");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM6");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM7");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM8");
    itin.brandFilterMap()["BRAND NAME"].insert("PROGRAM1");
    itin.brandFilterMap()["OTHER BRAND"];

    printContextShoppingItinInfo(&itin, &trx);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "** PROCESSING ITIN 123 **\n"
     "\n"
     "LEG  0: FIXED\n"
     "        AAA-BBB: XX\n"
     "        AAA-BBB: NO BRAND DATA\n"
     "LEG  1: NOT FIXED\n"
     "        AAA-BBB\n"
     "        AAA-BBB\n"
     "LEG  2: FIXED\n"
     "        AAA-BBB: YY\n"
     "        AAA-BBB: NO CONTEXT DATA\n"
     "(ARUNK) CCC-DDD\n"
     "\n"
     "REQUESTED BRAND FILTERS FOR ITIN 123:\n"
     "BRAND ID = BRAND NAME PROGRAM IDs : PROGRAM1 PROGRAM2 PROGRAM3\n"
     "                                    PROGRAM4 PROGRAM5 PROGRAM6\n"
     "                                    PROGRAM7 PROGRAM8 PROGRAM9\n"
     "BRAND ID = OTHER BRAND PROGRAM IDs : N/A\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintContextShoppingItinParityInfo()
  {
    Itin itin;
    itin.itinNum() = 123;

    PricingRequest request;
    request.setBrandedFaresRequest(true);
    request.setContextShoppingRequest();

    std::vector<bool> fixedLegs;
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);

    PricingTrx trx;
    trx.setRequest(&request);
    trx.getMutableFixedLegs() = fixedLegs;

    AirSeg air;
    air.origAirport() = "AAA";
    air.destAirport() = "BBB";

    ArunkSeg arung;
    arung.origAirport() = "CCC";
    arung.destAirport() = "DDD";

    std::vector<TravelSegPtrVec> legs;
    std::vector<tse::TravelSeg*> travel;
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&arung);
    legs.push_back(travel);
    itin.itinLegs() = legs;

    skipper::UnorderedBrandCodes brands;
    brands.insert("XX");
    brands.insert("YY");
    brands.insert("ZZ");

    skipper::UnorderedBrandCodes currentLegbrands;
    currentLegbrands.insert("AA");
    currentLegbrands.insert("BB");

    printContextShoppingItinParityInfo(&itin, &trx, brands, currentLegbrands);

    string expected(
    //         1         2         3         4         5         6         7
    //123456789012345678901234567890123456789012345678901234567890123456789012345678
     "\n** PARITY CALCULATION FOR ITIN 123 **\n"
     "\n"
     "LEG  0: FIXED\n"
     "LEG  1: XX YY ZZ\n"
     "LEG  2: FIXED\n"
     "(ARUNK)\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintContextShoppingItinParityInfoWithCurrentLegbrands()
  {
    Itin itin;
    itin.itinNum() = 123;

    PricingRequest request;
    request.setBrandedFaresRequest(true);
    request.setContextShoppingRequest();
    request.setProcessParityBrandsOverride(true);

    std::vector<bool> fixedLegs;
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);
    fixedLegs.push_back(true);
    fixedLegs.push_back(false);

    PricingTrx trx;
    trx.setRequest(&request);
    trx.getMutableFixedLegs() = fixedLegs;

    AirSeg air;
    air.origAirport() = "AAA";
    air.destAirport() = "BBB";

    ArunkSeg arung;
    arung.origAirport() = "CCC";
    arung.destAirport() = "DDD";

    std::vector<TravelSegPtrVec> legs;
    std::vector<tse::TravelSeg*> travel;
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel);
    travel.clear();
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    travel.push_back(&air);
    legs.push_back(travel); // <- fixed
    travel.clear();
    travel.push_back(&arung);
    legs.push_back(travel);
    itin.itinLegs() = legs;

    skipper::UnorderedBrandCodes brands;
    brands.insert("XX");
    brands.insert("YY");
    brands.insert("ZZ");

    skipper::UnorderedBrandCodes currentLegbrands;
    currentLegbrands.insert("AA");
    currentLegbrands.insert("BB");

    printContextShoppingItinParityInfo(&itin, &trx, brands, currentLegbrands);

    string expected(
    //         1         2         3         4         5         6         7
    //123456789012345678901234567890123456789012345678901234567890123456789012345678
     "\n** PARITY CALCULATION FOR ITIN 123 **\n"
     "\n"
     "LEG  0: FIXED\n"
     "LEG  1: XX YY ZZ | AA BB\n"
     "LEG  2: FIXED\n"
     "(ARUNK)\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintRemovedFareMarkets()
  {
    std::set<FareMarket*> fms;

    Loc l1, l2, l3, l4;
    l1.loc() = "AAAAAAAA";
    l2.loc() = "BBBBBBBB";
    l3.loc() = "CCCCCCCC";
    l4.loc() = "DDDDDDDD";

    FareMarket fmsa[2];
    FareMarket& fm1 = fmsa[0];
    fm1.origin() = &l1;
    fm1.destination() = &l2;
    fm1.governingCarrier() = "CRX";
    fm1.failCode() = ErrorResponseException::NO_ERROR;

    FareMarket& fm2 = fmsa[1];
    fm2.origin() = &l3;
    fm2.destination() = &l4;
    fm2.governingCarrier() = "XXX";
    fm2.failCode() = ErrorResponseException::NO_ERROR;

    fms.insert(&fm1);
    fms.insert(static_cast<FareMarket*>(0));
    fms.insert(&fm2);

    printRemovedFareMarkets(fms);

    string expected(
    //         1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "FARE MARKETS REMOVED IN ITIN ANALYZER:\n"
     "AAAAAAAA-CRX-BBBBBBBB BECAUSE OF ERROR CODE : 0\n"
     "CCCCCCCC-XXX-DDDDDDDD BECAUSE OF ERROR CODE : 0\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintBrandingOptionSpacesLongCodes()
  {
    skipper::BrandingOptionSpaces spaces;
    for (size_t spacesCount = 0; spacesCount < 2; ++spacesCount)
    {
      skipper::BrandingOptionSpace space;
      for (size_t segmentIndex = 0; segmentIndex < 16; ++segmentIndex)
      {
        skipper::CarrierBrandPairs pairs;
        size_t carriersCount = (segmentIndex % 4) + 1;
        for (size_t cr = 0; cr < carriersCount; ++cr)
        {
          std::stringstream tmp;
          if (cr == 0)
          {
            tmp << "C";
          }
          else if ((cr == 1) || (cr == 2))
          {
            tmp << "C1"; //for 1 and 2
          }
          else
          {
            tmp << "CR2";
          }

          Direction dir = Direction::BOTHWAYS;
          if (cr == 1)
            dir = Direction::ORIGINAL;
          else if (cr == 2)
            dir = Direction::REVERSED;
          if (cr == 3 || spacesCount == 0)
          {
            pairs[skipper::CarrierDirection(tmp.str(), dir)] = "--";
          }
          else
          {
            char letter = char(int('A') + segmentIndex + cr);
            size_t bcLen = 2;
            if (segmentIndex % 2)
              bcLen = 10;
            pairs[skipper::CarrierDirection(tmp.str(), dir)] = std::string(bcLen, letter);
          }
        }
        space.push_back(pairs);
      }
      spaces.push_back(space);
    }

    printBrandingOptionSpaces(spaces);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "BRANDING OPTION SPACES GENARATED FOR THIS ITIN:\n"
     " 0:  SEG. 00             SEG. 01             SEG. 02           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "                         C1 (O): --          C1 (O): --        \n"
     "                                             C1 (R): --        \n"
     "\n"
     "     SEG. 03             SEG. 04             SEG. 05           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --                              C1 (O): --        \n"
     "     C1 (R): --                                                \n"
     "     CR2(B): --                                                \n"
     "\n"
     "     SEG. 06             SEG. 07             SEG. 08           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --          C1 (O): --                            \n"
     "     C1 (R): --          C1 (R): --                            \n"
     "                         CR2(B): --                            \n"
     "\n"
     "     SEG. 09             SEG. 10             SEG. 11           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --          C1 (O): --          C1 (O): --        \n"
     "                         C1 (R): --          C1 (R): --        \n"
     "                                             CR2(B): --        \n"
     "\n"
     "     SEG. 12             SEG. 13             SEG. 14           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "                         C1 (O): --          C1 (O): --        \n"
     "                                             C1 (R): --        \n"
     "\n"
     "     SEG. 15           \n"
     "     C  (B): --        \n"
     "     C1 (O): --        \n"
     "     C1 (R): --        \n"
     "     CR2(B): --        \n"
     "\n"
     "\n"
     " 1:  SEG. 00             SEG. 01             SEG. 02           \n"
     "     C  (B): AA          C  (B): BBBBBBBBBB  C  (B): CC        \n"
     "                         C1 (O): CCCCCCCCCC  C1 (O): DD        \n"
     "                                             C1 (R): EE        \n"
     "\n"
     "     SEG. 03             SEG. 04             SEG. 05           \n"
     "     C  (B): DDDDDDDDDD  C  (B): EE          C  (B): FFFFFFFFFF\n"
     "     C1 (O): EEEEEEEEEE                      C1 (O): GGGGGGGGGG\n"
     "     C1 (R): FFFFFFFFFF                                        \n"
     "     CR2(B): --                                                \n"
     "\n"
     "     SEG. 06             SEG. 07             SEG. 08           \n"
     "     C  (B): GG          C  (B): HHHHHHHHHH  C  (B): II        \n"
     "     C1 (O): HH          C1 (O): IIIIIIIIII                    \n"
     "     C1 (R): II          C1 (R): JJJJJJJJJJ                    \n"
     "                         CR2(B): --                            \n"
     "\n"
     "     SEG. 09             SEG. 10             SEG. 11           \n"
     "     C  (B): JJJJJJJJJJ  C  (B): KK          C  (B): LLLLLLLLLL\n"
     "     C1 (O): KKKKKKKKKK  C1 (O): LL          C1 (O): MMMMMMMMMM\n"
     "                         C1 (R): MM          C1 (R): NNNNNNNNNN\n"
     "                                             CR2(B): --        \n"
     "\n"
     "     SEG. 12             SEG. 13             SEG. 14           \n"
     "     C  (B): MM          C  (B): NNNNNNNNNN  C  (B): OO        \n"
     "                         C1 (O): OOOOOOOOOO  C1 (O): PP        \n"
     "                                             C1 (R): QQ        \n"
     "\n"
     "     SEG. 15           \n"
     "     C  (B): PPPPPPPPPP\n"
     "     C1 (O): QQQQQQQQQQ\n"
     "     C1 (R): RRRRRRRRRR\n"
     "     CR2(B): --        \n"
     "\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintBrandingOptionSpaces()
  {
    skipper::BrandingOptionSpaces spaces;
    for (size_t spacesCount = 0; spacesCount < 2; ++spacesCount)
    {
      skipper::BrandingOptionSpace space;
      for (size_t segmentIndex = 0; segmentIndex < 16; ++segmentIndex)
      {
        skipper::CarrierBrandPairs pairs;
        size_t carriersCount = (segmentIndex % 4) + 1;
        for (size_t cr = 0; cr < carriersCount; ++cr)
        {
          std::stringstream tmp;
          if (cr == 0)
          {
            tmp << "C";
          }
          else if ((cr == 1) || (cr == 2))
          {
            tmp << "C1"; //for 1 and 2
          }
          else
          {
            tmp << "CR2";
          }

          Direction dir = Direction::BOTHWAYS;
          if (cr == 1)
            dir = Direction::ORIGINAL;
          else if (cr == 2)
            dir = Direction::REVERSED;
          if (cr == 3 || spacesCount == 0)
          {
            pairs[skipper::CarrierDirection(tmp.str(), dir)] = "--";
          }
          else
          {
            char letter = char(int('A') + segmentIndex + cr);
            pairs[skipper::CarrierDirection(tmp.str(), dir)] = std::string(2, letter);
          }
        }
        space.push_back(pairs);
      }
      spaces.push_back(space);
    }

    printBrandingOptionSpaces(spaces);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "BRANDING OPTION SPACES GENARATED FOR THIS ITIN:\n"
     " 0:  SEG. 00             SEG. 01             SEG. 02           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "                         C1 (O): --          C1 (O): --        \n"
     "                                             C1 (R): --        \n"
     "\n"
     "     SEG. 03             SEG. 04             SEG. 05           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --                              C1 (O): --        \n"
     "     C1 (R): --                                                \n"
     "     CR2(B): --                                                \n"
     "\n"
     "     SEG. 06             SEG. 07             SEG. 08           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --          C1 (O): --                            \n"
     "     C1 (R): --          C1 (R): --                            \n"
     "                         CR2(B): --                            \n"
     "\n"
     "     SEG. 09             SEG. 10             SEG. 11           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "     C1 (O): --          C1 (O): --          C1 (O): --        \n"
     "                         C1 (R): --          C1 (R): --        \n"
     "                                             CR2(B): --        \n"
     "\n"
     "     SEG. 12             SEG. 13             SEG. 14           \n"
     "     C  (B): --          C  (B): --          C  (B): --        \n"
     "                         C1 (O): --          C1 (O): --        \n"
     "                                             C1 (R): --        \n"
     "\n"
     "     SEG. 15           \n"
     "     C  (B): --        \n"
     "     C1 (O): --        \n"
     "     C1 (R): --        \n"
     "     CR2(B): --        \n"
     "\n"
     "\n"
     " 1:  SEG. 00             SEG. 01             SEG. 02           \n"
     "     C  (B): AA          C  (B): BB          C  (B): CC        \n"
     "                         C1 (O): CC          C1 (O): DD        \n"
     "                                             C1 (R): EE        \n"
     "\n"
     "     SEG. 03             SEG. 04             SEG. 05           \n"
     "     C  (B): DD          C  (B): EE          C  (B): FF        \n"
     "     C1 (O): EE                              C1 (O): GG        \n"
     "     C1 (R): FF                                                \n"
     "     CR2(B): --                                                \n"
     "\n"
     "     SEG. 06             SEG. 07             SEG. 08           \n"
     "     C  (B): GG          C  (B): HH          C  (B): II        \n"
     "     C1 (O): HH          C1 (O): II                            \n"
     "     C1 (R): II          C1 (R): JJ                            \n"
     "                         CR2(B): --                            \n"
     "\n"
     "     SEG. 09             SEG. 10             SEG. 11           \n"
     "     C  (B): JJ          C  (B): KK          C  (B): LL        \n"
     "     C1 (O): KK          C1 (O): LL          C1 (O): MM        \n"
     "                         C1 (R): MM          C1 (R): NN        \n"
     "                                             CR2(B): --        \n"
     "\n"
     "     SEG. 12             SEG. 13             SEG. 14           \n"
     "     C  (B): MM          C  (B): NN          C  (B): OO        \n"
     "                         C1 (O): OO          C1 (O): PP        \n"
     "                                             C1 (R): QQ        \n"
     "\n"
     "     SEG. 15           \n"
     "     C  (B): PP        \n"
     "     C1 (O): QQ        \n"
     "     C1 (R): RR        \n"
     "     CR2(B): --        \n"
     "\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintSegmentOrientedBrandCodes()
  {
    skipper::SegmentOrientedBrandCodesPerCarrier brands;

    for (size_t segmentIndex = 0; segmentIndex < 8; ++segmentIndex)
    {
      skipper::BrandCodesPerCarrier map;
      //std::map<CarrierCode, UnorderedBrandCodes>

      size_t carriersCount = (segmentIndex % 4) + 1;
      for (size_t cr = 0; cr < carriersCount; ++cr)
      {
        std::stringstream tmp;
        if (cr == 0)
        {
          tmp << "C";
        }
        else if ((cr == 1) || (cr == 2))
        {
          tmp << "C1"; //for 1 and 2
        }
        else
        {
          tmp << "CR2";
        }

        if (cr == 3)
        {
          map[skipper::CarrierDirection(tmp.str(), Direction::BOTHWAYS)].insert("--");
        }
        else
        {
          size_t brandsCount = (segmentIndex % 4) + cr + 1;
          if (segmentIndex == 1 && cr == 1)
            brandsCount = 25;
          Direction dir = Direction::BOTHWAYS;
          if (cr == 1)
            dir = Direction::ORIGINAL;
          else if (cr == 2)
            dir = Direction::REVERSED;
          for (size_t br = 0; br < brandsCount; ++br)
          {
            char letter = char(int('A') + segmentIndex + br);
            map[skipper::CarrierDirection(tmp.str(), dir)].insert(std::string(2, letter));
          }
        }
      }
      brands.push_back(map);
    }
    skipper::BrandCodesPerCarrier map;
    map[skipper::CarrierDirection("", Direction::BOTHWAYS)].insert(""); //unexpected!
    brands.push_back(map);

    printSegmentOrientedBrandCodes(brands);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "BRAND CODES PER CARRIER:\n"
     "SEGMENT 00: C   (B): AA\n"
     "\n"
     "SEGMENT 01: C   (B): BB,CC\n"
     "            C1  (O): BB,CC,DD,EE,FF,GG,HH,II,JJ,KK,LL,MM,NN,OO,\n"
     "                     PP,QQ,RR,SS,TT,UU,VV,WW,XX,YY,ZZ\n"
     "\n"
     "SEGMENT 02: C   (B): CC,DD,EE\n"
     "            C1  (O): CC,DD,EE,FF\n"
     "            C1  (R): CC,DD,EE,FF,GG\n"
     "\n"
     "SEGMENT 03: C   (B): DD,EE,FF,GG\n"
     "            C1  (O): DD,EE,FF,GG,HH\n"
     "            C1  (R): DD,EE,FF,GG,HH,II\n"
     "            CR2 (B): --\n"
     "\n"
     "SEGMENT 04: C   (B): EE\n"
     "\n"
     "SEGMENT 05: C   (B): FF,GG\n"
     "            C1  (O): FF,GG,HH\n"
     "\n"
     "SEGMENT 06: C   (B): GG,HH,II\n"
     "            C1  (O): GG,HH,II,JJ\n"
     "            C1  (R): GG,HH,II,JJ,KK\n"
     "\n"
     "SEGMENT 07: C   (B): HH,II,JJ,KK\n"
     "            C1  (O): HH,II,JJ,KK,LL\n"
     "            C1  (R): HH,II,JJ,KK,LL,MM\n"
     "            CR2 (B): --\n"
     "\n"
     "SEGMENT 08: ?   (B): ?\n"
     "\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintSegmentOrientedBrandCodesAfterSorting()
  {
    skipper::SegmentOrientedBrandCodeArraysPerCarrier brands;

    for (size_t segmentIndex = 0; segmentIndex < 8; ++segmentIndex)
    {
      skipper::BrandCodeArraysPerCarrier map;
      size_t carriersCount = (segmentIndex % 4) + 1;
      for (size_t cr = 0; cr < carriersCount; ++cr)
      {
        std::stringstream tmp;
        if (cr == 0)
        {
          tmp << "C";
        }
        else if ((cr == 1) || (cr == 2))
        {
          tmp << "C1"; //for 1 and 2
        }
        else
        {
          tmp << "CR2";
        }

        if (cr == 2)
        {
          map[skipper::CarrierDirection(tmp.str(), Direction::BOTHWAYS)].push_back("--");
        }
        else
        {
          size_t brandsCount = (segmentIndex % 4) + cr + 1;
          if (segmentIndex == 1 && cr == 1)
            brandsCount = 25;
          Direction dir = Direction::BOTHWAYS;
          if (cr == 1)
            dir = Direction::ORIGINAL;
          else if (cr == 2)
            dir = Direction::REVERSED;
          for (size_t br = 0; br < brandsCount; ++br)
          {
            char letter = char(int('A') + segmentIndex + br);
            map[skipper::CarrierDirection(tmp.str(), dir)].push_back(std::string(2, letter));
          }
        }
        if (segmentIndex == 7 && cr == 1)
        {
          Direction dir = Direction::BOTHWAYS;
          if (cr == 1)
            dir = Direction::ORIGINAL;
          else if (cr == 2)
            dir = Direction::REVERSED;
          map[skipper::CarrierDirection(tmp.str(), dir)].push_back("");
        }
      }
      brands.push_back(map);
    }
    skipper::BrandCodeArraysPerCarrier map;
    map[skipper::CarrierDirection("", Direction::BOTHWAYS)].push_back(""); //unexpected!
    brands.push_back(map);

    printSegmentOrientedBrandCodesAfterSorting(brands);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "SORTED BRAND CODES PER CARRIER:\n"
     "SEGMENT 00: C   (B): AA\n"
     "\n"
     "SEGMENT 01: C   (B): BB,CC\n"
     "            C1  (O): BB,CC,DD,EE,FF,GG,HH,II,JJ,KK,LL,MM,NN,OO,\n"
     "                     PP,QQ,RR,SS,TT,UU,VV,WW,XX,YY,ZZ\n"
     "\n"
     "SEGMENT 02: C   (B): CC,DD,EE\n"
     "            C1  (O): CC,DD,EE,FF\n"
     "            C1  (R): CC,DD,EE,FF,GG\n"
     "\n"
     "SEGMENT 03: C   (B): DD,EE,FF,GG\n"
     "            C1  (O): DD,EE,FF,GG,HH\n"
     "            C1  (R): DD,EE,FF,GG,HH,II\n"
     "            CR2 (B): --\n"
     "\n"
     "SEGMENT 04: C   (B): EE\n"
     "\n"
     "SEGMENT 05: C   (B): FF,GG\n"
     "            C1  (O): FF,GG,HH\n"
     "\n"
     "SEGMENT 06: C   (B): GG,HH,II\n"
     "            C1  (O): GG,HH,II,JJ\n"
     "            C1  (R): GG,HH,II,JJ\n"
     "\n"
     "SEGMENT 07: C   (B): HH,II,JJ,KK\n"
     "            C1  (O): ?,HH,II,JJ,KK,LL\n"
     "            C1  (R): ?,HH,II,JJ,KK,LL,MM\n"
     "\n"
     "SEGMENT 08: ?   (B): ?\n"
     "\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintComparator()
  {
    Logger logger("DUMMY TEST LOGGER");

    const size_t BRANDS = 30;

    PricingTrx trx;
    BrandProgram p;
    BrandInfo brands[BRANDS];

    for (size_t index = 0; index < BRANDS; ++index)
    {
      std::stringstream tmp;
      tmp << char(int('A') + int(index / 10)) << (index % 10);
      brands[index].brandCode() = tmp.str();

      trx.brandProgramVec().push_back(std::make_pair(&p, &(brands[index])));
    }

    BrandedFaresComparator comparator(trx.brandProgramVec(), logger);

    printComparator(comparator);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "ORDER OF BRANDS IN COMPARATOR:\n"
     "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9,C0,\n"
     "C1,C2,C3,C4,C5,C6,C7,C8,C9\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testbrandingOptionSpaceToString()
  {
    const CarrierCode CXR1 = "AA";
    const CarrierCode CXR2 = "CUA";
    const CarrierCode CXR3 = "";
    const BrandCode BRAND_1 = "ZZ";
    const BrandCode BRAND_2 = "XX";
    const BrandCode BRAND_3 = NO_BRAND;
    const BrandCode BRAND_4 = "";

    skipper::BrandingOptionSpace space;
    //2 segments
    space.resize(2);
    space[0][skipper::CarrierDirection(CXR1, Direction::ORIGINAL)] = BRAND_1;
    space[0][skipper::CarrierDirection(CXR2, Direction::BOTHWAYS)] = BRAND_2;
    space[1][skipper::CarrierDirection(CXR1, Direction::REVERSED)] = BRAND_3;
    space[1][skipper::CarrierDirection(CXR3, Direction::BOTHWAYS)] = BRAND_4;

    std::vector<std::vector<std::string> > result;
    std::vector<std::vector<std::string> > expected;
    std::vector<std::string> tmp;
    tmp.push_back(CXR1 + " (O): " + BRAND_1 + std::string(10 - BRAND_1.length(), ' '));
    tmp.push_back(CXR2 + "(B): " + BRAND_2 + std::string(10 - BRAND_2.length(), ' '));
    expected.push_back(tmp);
    tmp.clear();
    tmp.push_back("?  (B): ??" + std::string(8, ' ')); //improper carrier and brand
    tmp.push_back(CXR1 + " (R): " + "--" + std::string(8, ' ')); //NO_BRAND converted to --
    expected.push_back(tmp);

    brandingOptionSpaceToString(space, result);

    CPPUNIT_ASSERT_EQUAL(expected.size(), result.size());
    for (size_t index = 0; index < expected.size(); ++index)
    {
      CPPUNIT_ASSERT_EQUAL(expected[index].size(), result[index].size());
      for (size_t internalIndex = 0; internalIndex < expected[index].size(); ++internalIndex)
      {
        CPPUNIT_ASSERT_EQUAL(expected[index][internalIndex], result[index][internalIndex]);
      }
    }
  }

  void testprintCarrierBrandAndCabinInfo()
  {
    skipper::SegmentOrientedBrandCodesPerCarrier brands;

    for (size_t segmentIndex = 0; segmentIndex < 4; ++segmentIndex)
    {
      skipper::BrandCodesPerCarrier map;
      //std::map<CarrierCode, UnorderedBrandCodes>

      size_t carriersCount = (segmentIndex % 3) + 1;
      for (size_t cr = 0; cr < carriersCount; ++cr)
      {
        std::stringstream tmp;
        if (cr == 0)
        {
          tmp << "C";
        }
        else if (cr == 1)
        {
          tmp << "C" << cr;
        }
        else
        {
          tmp << "CR" << cr;
        }

        if (cr == 2)
        {
          map[skipper::CarrierDirection(tmp.str(), Direction::BOTHWAYS)].insert("--");
        }
        else
        {
          size_t brandsCount = (segmentIndex % 4) + cr + 1;
          if (segmentIndex == 1 && cr == 1)
            brandsCount = 25;
          Direction dir = Direction::REVERSED;
          if (cr == 0)
            dir = Direction::ORIGINAL;
          for (size_t br = 0; br < brandsCount; ++br)
          {
            char letter = char(int('A') + segmentIndex + br);
            map[skipper::CarrierDirection(tmp.str(), dir)].insert(std::string(2, letter));
          }
        }
      }
      brands.push_back(map);
    }
    skipper::BrandCodesPerCarrier map;
    map[skipper::CarrierDirection("", Direction::BOTHWAYS)].insert(""); //unexpected!
    brands.push_back(map);

    skipper::SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCabin;
    brandsPerCabin[0] = brands;
    brandsPerCabin[2] = brands;
    brandsPerCabin[4] = brands;
    brandsPerCabin[9999] = brands;

    printCarrierBrandAndCabinInfo(brandsPerCabin);

    string expected(
    //          1         2         3         4         5         6
    //1234567890123456789012345678901234567890123456789012345678901234
     "\n"
     "** BRANDS PER CABIN SEGMENT AND CARRIER: **\n"
     "\n"
     "CABIN: Y SEG: 00 CARRIER: C   (O) BRANDS: AA\n"
     "         SEG: 01 CARRIER: C   (O) BRANDS: BB,CC\n"
     "                 CARRIER: C1  (R) BRANDS: BB,CC,DD,EE,FF,GG,HH,\n"
     "                                          II,JJ,KK,LL,MM,NN,OO,\n"
     "                                          PP,QQ,RR,SS,TT,UU,VV,\n"
     "                                          WW,XX,YY,ZZ\n"
     "         SEG: 02 CARRIER: C   (O) BRANDS: CC,DD,EE\n"
     "                 CARRIER: C1  (R) BRANDS: CC,DD,EE,FF\n"
     "                 CARRIER: CR2 (B) BRANDS: --\n"
     "         SEG: 03 CARRIER: C   (O) BRANDS: DD,EE,FF,GG\n"
     "         SEG: 04 CARRIER: ?   (B) BRANDS: ?\n"
     "\n"
     "CABIN: C SEG: 00 CARRIER: C   (O) BRANDS: AA\n"
     "         SEG: 01 CARRIER: C   (O) BRANDS: BB,CC\n"
     "                 CARRIER: C1  (R) BRANDS: BB,CC,DD,EE,FF,GG,HH,\n"
     "                                          II,JJ,KK,LL,MM,NN,OO,\n"
     "                                          PP,QQ,RR,SS,TT,UU,VV,\n"
     "                                          WW,XX,YY,ZZ\n"
     "         SEG: 02 CARRIER: C   (O) BRANDS: CC,DD,EE\n"
     "                 CARRIER: C1  (R) BRANDS: CC,DD,EE,FF\n"
     "                 CARRIER: CR2 (B) BRANDS: --\n"
     "         SEG: 03 CARRIER: C   (O) BRANDS: DD,EE,FF,GG\n"
     "         SEG: 04 CARRIER: ?   (B) BRANDS: ?\n"
     "\n"
     "CABIN: F SEG: 00 CARRIER: C   (O) BRANDS: AA\n"
     "         SEG: 01 CARRIER: C   (O) BRANDS: BB,CC\n"
     "                 CARRIER: C1  (R) BRANDS: BB,CC,DD,EE,FF,GG,HH,\n"
     "                                          II,JJ,KK,LL,MM,NN,OO,\n"
     "                                          PP,QQ,RR,SS,TT,UU,VV,\n"
     "                                          WW,XX,YY,ZZ\n"
     "         SEG: 02 CARRIER: C   (O) BRANDS: CC,DD,EE\n"
     "                 CARRIER: C1  (R) BRANDS: CC,DD,EE,FF\n"
     "                 CARRIER: CR2 (B) BRANDS: --\n"
     "         SEG: 03 CARRIER: C   (O) BRANDS: DD,EE,FF,GG\n"
     "         SEG: 04 CARRIER: ?   (B) BRANDS: ?\n"
     "\n"
     "CABIN:   SEG: 00 CARRIER: C   (O) BRANDS: AA\n"
     "         SEG: 01 CARRIER: C   (O) BRANDS: BB,CC\n"
     "                 CARRIER: C1  (R) BRANDS: BB,CC,DD,EE,FF,GG,HH,\n"
     "                                          II,JJ,KK,LL,MM,NN,OO,\n"
     "                                          PP,QQ,RR,SS,TT,UU,VV,\n"
     "                                          WW,XX,YY,ZZ\n"
     "         SEG: 02 CARRIER: C   (O) BRANDS: CC,DD,EE\n"
     "                 CARRIER: C1  (R) BRANDS: CC,DD,EE,FF\n"
     "                 CARRIER: CR2 (B) BRANDS: --\n"
     "         SEG: 03 CARRIER: C   (O) BRANDS: DD,EE,FF,GG\n"
     "         SEG: 04 CARRIER: ?   (B) BRANDS: ?\n"
     "\n");

    CPPUNIT_ASSERT_EQUAL(expected, str());
  }

  void testprintDeduplicationInfo()
  {
    Itin itin;
    itin.itinNum() = 123;

    string expected;

    skipper::BrandingOptionSpacesDeduplicator::KeyBrandsMap keys;
    skipper::FarePathBrandKeyBuilder<>::FarePathBrandKey key;
    key.push_back(std::make_pair<CarrierCode, BrandCode>("CX1", "AA"));
    key.push_back(std::make_pair<CarrierCode, BrandCode>("CX2", "BB"));
    keys[key].insert(4);
    keys[key].insert(2);
    keys[key].insert(0);
    key.clear();
    key.push_back(std::make_pair<CarrierCode, BrandCode>("CX1", NO_BRAND.c_str()));
    key.push_back(std::make_pair<CarrierCode, BrandCode>("CX2", "BB"));
    keys[key].insert(3);
    keys[key].insert(1);

    expected.append(
      //         1         2         3         4         5         6         7
      //123456789012345678901234567890123456789012345678901234567890123456789012345678
       "ITIN: 123\n"
       "KEY: CX1:AA CX2:BB \n"
       "INDICES: 0,2,4\n\n"
       "KEY: CX1:-- CX2:BB \n"
       "INDICES: 1,3\n\n"
       "\n");

    printDeduplicationInfo(&itin, keys);
    CPPUNIT_ASSERT_EQUAL(expected, str());

  }

  void testprintBrandsRemovedFromTrx()
  {
    const int BRANDS = 10;
    const int MAX_BRANDS = 15;

    PricingTrx trx;
    BrandProgram p;
    BrandInfo brands[MAX_BRANDS];
    std::vector<QualifiedBrand> filteredBrands;

    for (size_t index = 0; index < MAX_BRANDS; ++index)
    {
      std::stringstream tmp;
      tmp << char(int('A') + int(index / 10)) << (index % 10);
      brands[index].brandCode() = tmp.str();

      if (index < BRANDS)
      {
        trx.brandProgramVec().push_back(std::make_pair(&p, &(brands[index])));
        if (((index % 2) == 0) || ((index % 3) == 0))
          filteredBrands.push_back(std::make_pair(&p, &(brands[index])));
      }
      if (((index % 2) == 0) || ((index % 3) == 0))
        trx.getMutableBrandsFilterForIS().insert(brands[index].brandCode());
    }

    printBrandsRemovedFromTrx(trx, filteredBrands);

    string expected(
    //         1         2         3         4         5         6         7
    //123456789012345678901234567890123456789012345678901234567890123456789012345678
     "\n"
     "ALLOWED BRANDS: A0,A2,A3,A4,A6,A8,A9,B0,B2,B4\n"
     "\n"
     "TRX BRANDS BEFORE FILTERING: A0,A1,A2,A3,A4,A5,A6,A7,A8,A9\n"
     "TRX BRANDS AFTER FILTERING:  A0,A2,A3,A4,A6,A8,A9\n"
     "\n");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag892CollectorTest);
}
