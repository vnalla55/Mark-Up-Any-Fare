#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag312Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"

#include "Common/Vendor.h"

namespace tse
{

class Diag312CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag312CollectorTest);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_AnyPassenger);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_Child);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_AdultChildInfant);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_AdultChildDiscountInfantDiscount);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_Adult);
  CPPUNIT_TEST(testFormatSurchargeApplication_ATPCO_Unknown);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AnyPassenger);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_Child);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AdultChild);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AdultChildDiscount);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_Adult);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AdultDiscChildInfant);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AdultChildFreeInfant);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_AdultDiscChildFreeInfant);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_Infant);
  CPPUNIT_TEST(testFormatSurchargeApplication_SITA_Unknown);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeA);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeB);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeC);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeD);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeE);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeG);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeH);
  CPPUNIT_TEST(testFormatSurchargeApplication_negativeI);

  CPPUNIT_TEST(testFormatTravelPortion_RoundTrip);
  CPPUNIT_TEST(testFormatTravelPortion_PerTransfer);
  CPPUNIT_TEST(testFormatTravelPortion_PerTicket);
  CPPUNIT_TEST(testFormatTravelPortion_PerCoupon);
  CPPUNIT_TEST(testFormatTravelPortion_PerDirection);
  CPPUNIT_TEST(testFormatTravelPortion_Unknown);

  CPPUNIT_TEST(testFormatTimeApplication_None);
  CPPUNIT_TEST(testFormatTimeApplication_Range);
  CPPUNIT_TEST(testFormatTimeApplication_Daily);
  CPPUNIT_TEST(testFormatTimeApplication_Unknown);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  Diag312Collector* _collector;
  Diagnostic* _diagRoot;

public:
  void setUp()
  {
    _diagRoot = _memHandle.insert(new Diagnostic(Diagnostic312));
    _diagRoot->activate();
    _collector = _memHandle.insert(new Diag312Collector(*_diagRoot));
    _collector->enable(Diagnostic312);
  }

  void tearDown() { _memHandle.clear(); }

  void testFormatSurchargeApplication_ATPCO_AnyPassenger()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ANY PASSENGER"),
                         _collector->formatSurchargeApplication(' ', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_ATPCO_Child()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("CHILD"),
                         _collector->formatSurchargeApplication('1', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_ATPCO_AdultChildInfant()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD AND INFANT"),
                         _collector->formatSurchargeApplication('2', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_ATPCO_AdultChildDiscountInfantDiscount()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD DISCOUNT AND INFANT DISCOUNT"),
                         _collector->formatSurchargeApplication('3', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_ATPCO_Adult()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT"),
                         _collector->formatSurchargeApplication('4', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_ATPCO_Unknown()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"),
                         _collector->formatSurchargeApplication('5', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_SITA_AnyPassenger()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ANY PASSENGER"),
                         _collector->formatSurchargeApplication(' ', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_Child()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("CHILD"),
                         _collector->formatSurchargeApplication('1', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_AdultChild()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD"),
                         _collector->formatSurchargeApplication('2', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_AdultChildDiscount()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD DISCOUNT"),
                         _collector->formatSurchargeApplication('3', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_Adult()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT"),
                         _collector->formatSurchargeApplication('4', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_AdultDiscChildInfant()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND DISC CHILD AND INFANT"),
                         _collector->formatSurchargeApplication('5', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_AdultChildFreeInfant()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD AND FREE INFANT"),
                         _collector->formatSurchargeApplication('6', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_AdultDiscChildFreeInfant()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND DISC CHILD AND FREE INF"),
                         _collector->formatSurchargeApplication('7', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_Infant()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("INFANT"),
                         _collector->formatSurchargeApplication('8', Vendor::SITA));
  }

  void testFormatSurchargeApplication_SITA_Unknown()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"),
                         _collector->formatSurchargeApplication('9', Vendor::SITA));
  }

  void testFormatTravelPortion_PerComponent()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PER COMPONENT"), _collector->formatTravelPortion(' '));
    CPPUNIT_ASSERT_EQUAL(std::string("PER COMPONENT"), _collector->formatTravelPortion('1'));
  }

  void testFormatTravelPortion_RoundTrip()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ROUNDTRIP"), _collector->formatTravelPortion('2'));
  }

  void testFormatTravelPortion_PerTransfer()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PER TRANSFER"), _collector->formatTravelPortion('3'));
  }

  void testFormatTravelPortion_PerTicket()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PER TICKET"), _collector->formatTravelPortion('4'));
  }

  void testFormatTravelPortion_PerCoupon()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PER COUPON"), _collector->formatTravelPortion('5'));
  }

  void testFormatTravelPortion_PerDirection()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PER DIRECTION"), _collector->formatTravelPortion('6'));
  }

  void testFormatTravelPortion_Unknown()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), _collector->formatTravelPortion('7'));
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), _collector->formatTravelPortion('A'));
  }

  void testFormatTimeApplication_None()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NONE"), _collector->formatTimeApplication(' '));
  }

  void testFormatTimeApplication_Range()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("RANGE"), _collector->formatTimeApplication('R'));
  }

  void testFormatTimeApplication_Daily()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DAILY"), _collector->formatTimeApplication('D'));
  }

  void testFormatTimeApplication_Unknown()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), _collector->formatTimeApplication('S'));
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), _collector->formatTimeApplication('T'));
  }

  void testFormatSurchargeApplication_negativeA()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ANY PASSENGER"),
                         _collector->formatSurchargeApplication('A', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeB()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("CHILD AND INFANT"),
                         _collector->formatSurchargeApplication('B', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeC()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT, CHILD AND INFANT"),
                         _collector->formatSurchargeApplication('C', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeD()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD DISCOUNT AND INFANT DISCOUNT"),
                         _collector->formatSurchargeApplication('D', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeE()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT"),
                         _collector->formatSurchargeApplication('E', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeG()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND CHILD AND FREE INFANT"),
                         _collector->formatSurchargeApplication('G', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeH()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADULT AND DISC CHILD AND FREE INF"),
                         _collector->formatSurchargeApplication('H', Vendor::ATPCO));
  }

  void testFormatSurchargeApplication_negativeI()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("INFANT"),
                         _collector->formatSurchargeApplication('I', Vendor::ATPCO));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag312CollectorTest);
}
