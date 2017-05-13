#include <sstream>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag893Collector.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "BrandedFares/BrandFeatureItem.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/MarketCriteria.h"

using namespace std;
namespace tse
{
class Diag893CollectorTest : public Diag893Collector, public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag893CollectorTest);
  CPPUNIT_TEST(testdisplaySvcFeesFareIdInfo);
  CPPUNIT_TEST(testdisplayBrandInfo);
  CPPUNIT_TEST(testdisplayBrandProgram);
  CPPUNIT_TEST(testdisplayMarketResponse);
  CPPUNIT_TEST(testdisplayBrandedResonseMap);
  CPPUNIT_TEST(testdisplayBrandFeatureItem);
  CPPUNIT_TEST(testjoinElementsInCollection);
  CPPUNIT_TEST(testsplitIntoLines);
  CPPUNIT_TEST(testaddMultilineInfo);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {}
  virtual void tearDown() {}

  DataHandle dataHandle;

  Diag893CollectorTest() { _active = true; }

protected:

  void testdisplaySvcFeesFareIdInfo()
  {
    SvcFeesFareIdInfo fareIdInfo;
    fareIdInfo.itemNo() = 1111;
    fareIdInfo.seqNo() = 2222;
    fareIdInfo.vendor() = "VEND";
    fareIdInfo.validityInd() = 'Y';
    fareIdInfo.fareApplInd() = 'N';
    fareIdInfo.owrt() = 'A';
    fareIdInfo.ruleTariff() = 33;
    fareIdInfo.ruleTariffInd() = "AAA";
    fareIdInfo.fareClass() = "Class";
    fareIdInfo.fareType() = "Type";
    fareIdInfo.paxType() = "ADT";
    fareIdInfo.routing() = 44;
    fareIdInfo.source() = 'S';
    fareIdInfo.rule() = "Rule";
    fareIdInfo.bookingCode1() = "A";
    fareIdInfo.bookingCode2() = "B";
    fareIdInfo.minFareAmt1() = 1.1;
    fareIdInfo.minFareAmt2() = 2.2;
    fareIdInfo.maxFareAmt1() = 3.3;
    fareIdInfo.maxFareAmt2() = 4.4;
    fareIdInfo.cur1() = "USD";
    fareIdInfo.cur2() = "PLN";
    fareIdInfo.noDec1() = 1;
    fareIdInfo.noDec2() = 2;

    *this << &fareIdInfo;

    std::string svcFeesFareIdInfoDescription =
      "---------------------------------------------------------------\n"
      "ITEM NR     : 1111             SEQ NR       : 2222             \n"
      "VENDOR      : VEND             VALIDITYIND  : Y                \n"
      "FAREAPPLIND : N                OWRT         : A                \n"
      "RULETARIFF  : 33               RULETARIFFIND: AAA              \n"
      "FARECLASS   : CLASS            FARETYPE     : TYPE             \n"
      "PSGTYPE     : ADT              ROUTING      : 44               \n"
      "SOURCE      : S                RULE         : RULE             \n"
      "BOOKINGCODE1: A                BOOKINGCODE2 : B                \n"
      "MINFAREAMT1 : 1.1              MINFAREAMT2  : 2.2              \n"
      "MAXFAREAMT1 : 3.3              MAXFAREAMT2  : 4.4              \n"
      "CUR1        : USD              CUR2         : PLN              \n"
      "NODEC1      : 1                NODEC2       : 2                \n";

    CPPUNIT_ASSERT_EQUAL(svcFeesFareIdInfoDescription, str());
  }

  void testdisplayBrandInfo()
  {
    BrandInfo brandInfo;
    brandInfo.brandCode() = "AA";
    brandInfo.brandName() = "Name";
    brandInfo.tier() = 10;
    brandInfo.primaryFareIdTable() = 11;
    brandInfo.secondaryFareIdTable() = 12;
    brandInfo.primaryBookingCode().push_back("P1");
    brandInfo.primaryBookingCode().push_back("P2");
    brandInfo.secondaryBookingCode().push_back("S1");
    brandInfo.secondaryBookingCode().push_back("S2");
    brandInfo.includedFareBasisCode().push_back("I1");
    brandInfo.includedFareBasisCode().push_back("I2");
    brandInfo.excludedFareBasisCode().push_back("E1");
    brandInfo.excludedFareBasisCode().push_back("E2");

    *this << &brandInfo;

    std::string brandCodeDescription =
      "---------------------------------------------------------------\n"
      "BRAND CODE: AA                                                \n"
      "BRAND NAME: NAME                          \n"
      "TIER NUMBER: 10    \n"
      "PRIMARY FARE ID TABLE: 11        \n"
      "SECONDARY FARE ID TABLE: 12        \n"
      "PRIMARY BOOKING CODES: P1,P2                                   \n"
      "SECONDARY BOOKING CODES: S1,S2                                 \n"
      "INCLUDED FARE BASIS CODES: I1,I2                               \n"
      "EXCLUDED FARE BASIS CODES: E1,E2                               \n";

    CPPUNIT_ASSERT_EQUAL(brandCodeDescription, str());
  }

  void testdisplayBrandProgram()
  {
    BrandProgram program;
    program.programName() = "PROGRAM NAME";
    program.programID() = "ID";
    program.programCode() = "CD";
    program.programDescription() = "DESC";

    BrandInfo brandInfo1;
    brandInfo1.brandCode() = "AA";

    BrandInfo brandInfo2;
    brandInfo2.brandCode() = "BB";

    program.brandsData().push_back(&brandInfo1);
    program.brandsData().push_back(&brandInfo2);

    std::string brandProgramDescription =
      "==========================PROGRAM NAME=========================\n"
      "PROGRAM ID: ID             \n"
      "PROGRAM CODE: CD        \n"
      "PROGRAM DESCRIPTION: DESC                                      \n"
      "BRAND COUNTER: 2         \n";

    *this << &brandInfo1 << &brandInfo2;
    brandProgramDescription += str();
    str("");

    *this << &program;
    CPPUNIT_ASSERT_EQUAL(brandProgramDescription, str());
  }

  void testdisplayBrandFeatureItem()
  {
    BrandFeatureItem bfi;
    bfi.setSequenceNumber(1);
    bfi.setSubCode("03Z");
    bfi.setCommercialName("MILEAGE ACCRUAL");
    bfi.setServiceType('Z');
    bfi.setApplication('F');

    *this << &bfi;

    std::string brandedFeatureItemDescription =
      "---------------------------------------------------------------\n"
      "SEQ NR      : 1                SUBCODE      : 03Z              \n"
      "SERVICE     : Z                APPLICATION  : F                \n"
      "NAME        : MILEAGE ACCRUAL                                  \n";
    CPPUNIT_ASSERT_EQUAL(brandedFeatureItemDescription, str());
  }

  void testdisplayMarketResponse()
  {
    MarketResponse marketResponse;
    MarketCriteria criteria;
    criteria.departureAirportCode() = "BNE";
    criteria.arrivalAirportCode() = "MEL";
    marketResponse.marketCriteria() = &criteria;
    marketResponse.dataSource() = BRAND_SOURCE_S8;
    BrandProgram program1;
    program1.programName() = "PROGRAM NAME1";
    BrandProgram program2;
    program2.programName() = "PROGRAM NAME2";
    marketResponse.brandPrograms().push_back(&program1);
    marketResponse.brandPrograms().push_back(&program2);

    std::string marketResponseDescription =
      "************************ MARKET BNE-MEL ***********************\n"
      "BRAND SOURCE: S8  \n"
      "BRAND PROGRAMS COUNTER: 2         \n";

    *this << &program1 << &program2;
    marketResponseDescription += str();
    str("");

    *this << &marketResponse;
    CPPUNIT_ASSERT_EQUAL(marketResponseDescription, str());
  }

  void testdisplayBrandedResonseMap()
  {
    MarketResponse marketResponse;
    MarketCriteria criteria;
    criteria.departureAirportCode() = "BNE";
    criteria.arrivalAirportCode() = "MEL";
    marketResponse.marketCriteria() = &criteria;
    marketResponse.dataSource() = BRAND_SOURCE_S8;
    MarketResponse marketResponse1;
    MarketCriteria criteria1;
    criteria1.departureAirportCode() = "MEL";
    criteria1.arrivalAirportCode() = "SYD";
    marketResponse1.marketCriteria() = &criteria1;
    marketResponse1.dataSource() = BRAND_SOURCE_CBAS;

    PricingTrx::BrandedMarketMap brandedMarketMap;
    std::vector<MarketResponse*> vec1;
    vec1.push_back(&marketResponse);
    std::vector<MarketResponse*> vec2;
    vec2.push_back(&marketResponse1);

    brandedMarketMap[1] = vec1;
    brandedMarketMap[2] = vec2;

    *this << brandedMarketMap;

    std::string brandedMarketMapDescription =
      "***************************************************************\n"
      "*       DIAGNOSTIC 893 - PARSED BRANDED SERVICE RESPONSE      *\n"
      "************************ MARKET BNE-MEL ***********************\n"
      "BRAND SOURCE: S8  \n"
      "BRAND PROGRAMS COUNTER: 0         \n"
      "************************ MARKET MEL-SYD ***********************\n"
      "BRAND SOURCE: CBAS\n"
      "BRAND PROGRAMS COUNTER: 0         \n";

    CPPUNIT_ASSERT_EQUAL(brandedMarketMapDescription, str());
  }

  void testjoinElementsInCollection()
  {
    std::vector<BookingCode> vec;
    vec.push_back("T1");
    vec.push_back("T2");
    vec.push_back("T3");
    vec.push_back("T4");
    CPPUNIT_ASSERT_EQUAL(std::string("T1joinT2joinT3joinT4"), joinElementsInCollection(vec, "join"));
    CPPUNIT_ASSERT_EQUAL(std::string("T1,T2,T3,T4"), joinElementsInCollection(vec));
  }

  void testsplitIntoLines()
  {
    std::string veryLongString =
      "1111111111aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeeffffffffffgggggggggg"
      "hhhhhhhhhhiiiiiiiiiijjjjjjjjjjkkkkkkkkkkllllllllllmmmmmmmmmmnnnnnnnnnn"
      "oooooooooopppppppppp";
    std::vector<std::string> vec;
    splitIntoLines(veryLongString, vec);
    CPPUNIT_ASSERT_EQUAL(std::string("1111111111aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeefff"), vec.at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("fffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjjkkkkkkkkkkllllll"), vec.at(1));
    CPPUNIT_ASSERT_EQUAL(std::string("llllmmmmmmmmmmnnnnnnnnnnoooooooooopppppppppp"), vec.at(2));
  }

  void testaddMultilineInfo()
  {
    std::string veryLongString =
      "1111111111aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeeffffffffffgggggggggg"
      "hhhhhhhhhhiiiiiiiiiijjjjjjjjjjkkkkkkkkkkllllllllllmmmmmmmmmmnnnnnnnnnn"
      "oooooooooopppppppppp";
    std::stringstream stream;
    addMultilineInfo(stream, veryLongString);
    CPPUNIT_ASSERT_EQUAL(
      std::string("1111111111aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeeefff\n"
                  "fffffffgggggggggghhhhhhhhhhiiiiiiiiiijjjjjjjjjjkkkkkkkkkkllllll\n"
                  "llllmmmmmmmmmmnnnnnnnnnnoooooooooopppppppppp                   \n"), stream.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag893CollectorTest);
}
