#include "test/include/CppUnitHelperMacros.h"
#include "DSS/PreferredDateCalculator.h"
#include "DataModel/FareDisplayRequest.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class PreferredDateCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PreferredDateCalculatorTest);
  CPPUNIT_TEST(testCalculate_NoPreferDate);
  CPPUNIT_TEST(testCalculate_RangeFrom5To5);
  CPPUNIT_TEST(testCalculate_RangeFrom5To2);
  CPPUNIT_TEST(testCalculate_RangeFrom2To5);
  CPPUNIT_TEST(testCalculate_RangeFrom2To2);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCalculate_NoPreferDate()
  {
    _req->preferredTravelDate() = DateTime::emptyDate();
    PreferredDateCalculator::calculate(*_req, _tBegin, _tEnd);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), _tBegin);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), _tEnd);
  }
  void testCalculate_RangeFrom5To5()
  {
    _req->dateRangeLower() = DateTime(2030, 5, 5, 12, 0, 0);
    _req->dateRangeUpper() = DateTime(2030, 5, 15, 12, 0, 0);
    PreferredDateCalculator::calculate(*_req, _tBegin, _tEnd);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), _tBegin);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), _tEnd);
  }
  void testCalculate_RangeFrom5To2()
  {
    _req->dateRangeLower() = DateTime(2030, 5, 5, 12, 0, 0);
    PreferredDateCalculator::calculate(*_req, _tBegin, _tEnd);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), _tBegin);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _tEnd);
  }
  void testCalculate_RangeFrom2To5()
  {
    _req->dateRangeUpper() = DateTime(2030, 5, 15, 12, 0, 0);
    PreferredDateCalculator::calculate(*_req, _tBegin, _tEnd);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _tBegin);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), _tEnd);
  }
  void testCalculate_RangeFrom2To2()
  {
    PreferredDateCalculator::calculate(*_req, _tBegin, _tEnd);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _tBegin);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _tEnd);
  }
  void setUp()
  {
    _tBegin = 0;
    _tEnd = 0;
    _req = _memHandle.create<FareDisplayRequest>();
    _req->preferredTravelDate() = DateTime(2030, 5, 10, 12, 0, 0);
    _req->dateRangeLower() = DateTime(2030, 5, 8, 12, 0, 0);
    _req->dateRangeUpper() = DateTime(2030, 5, 12, 12, 0, 0);
  }
  void tearDown() { _memHandle.clear(); }

private:
  uint16_t _tBegin;
  uint16_t _tEnd;
  TestMemHandle _memHandle;
  FareDisplayRequest* _req;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PreferredDateCalculatorTest);
}
