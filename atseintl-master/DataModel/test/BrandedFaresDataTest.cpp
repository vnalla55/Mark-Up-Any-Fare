#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/BrandedFaresData.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class BrandedFaresDataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFaresDataTest);

  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testGetFareBookingCodeIndicator_Empty);
  CPPUNIT_TEST(testGetFareBookingCodeIndicator_OtherCode);
  CPPUNIT_TEST(testGetFareBookingCodeIndicator);

  CPPUNIT_TEST(testGetFareFamilyIndicator_Empty);
  CPPUNIT_TEST(testGetFareFamilyIndicator_OtherCode);
  CPPUNIT_TEST(testGetFareFamilyIndicator);

  CPPUNIT_TEST(testGetFareBasisCodeIndicator_Empty);
  CPPUNIT_TEST(testGetFareBasisCodeIndicator_OtherCode);
  CPPUNIT_TEST(testGetFareBasisCodeIndicator);

  CPPUNIT_TEST(testHasConsistentBookingCodes_Empty);
  CPPUNIT_TEST(testHasConsistentBookingCodes_EmptyBoth);
  CPPUNIT_TEST(testHasConsistentBookingCodes_EmptyPrimary);
  CPPUNIT_TEST(testHasConsistentBookingCodes_EmptySecondary);
  CPPUNIT_TEST(testHasConsistentBookingCodes_Overlapping);
  CPPUNIT_TEST(testHasConsistentBookingCodes_Separation);

  CPPUNIT_TEST_SUITE_END();

protected:
  BrandedFaresData* _data;

  TestMemHandle _mem;

  static const uint16_t INDEX = 8;
  static const char BLANK, INDICATOR;
  static const BookingCode BOOKING_CODE, OTHER_BOOKING_CODE;
  static const FareClassCode FARE_CLASS_CODE, OTHER_FARE_CLASS_CODE;

public:
  void setUp() { _data = _mem.create<BrandedFaresData>(); }

  void tearDown() { _mem.clear(); }

  void testConstructor() { CPPUNIT_ASSERT_EQUAL(uint16_t(0), _data->getSize()); }

  void testGetFareBookingCodeIndicator_Empty()
  {
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareBookingCodeIndicator(BOOKING_CODE, INDEX));
  }

  void testGetFareBookingCodeIndicator_OtherCode()
  {
    _data->fareBookingCodeData(INDEX)[OTHER_BOOKING_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareBookingCodeIndicator(BOOKING_CODE, INDEX));
  }

  void testGetFareBookingCodeIndicator()
  {
    _data->fareBookingCodeData(INDEX)[BOOKING_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(INDICATOR, _data->getFareBookingCodeIndicator(BOOKING_CODE, INDEX));
  }

  void testGetFareFamilyIndicator_Empty()
  {
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareFamilyIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testGetFareFamilyIndicator_OtherCode()
  {
    _data->fareFamilyData(INDEX)[OTHER_FARE_CLASS_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareFamilyIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testGetFareFamilyIndicator()
  {
    _data->fareFamilyData(INDEX)[FARE_CLASS_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(INDICATOR, _data->getFareFamilyIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testGetFareBasisCodeIndicator_Empty()
  {
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareBasisCodeIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testGetFareBasisCodeIndicator_OtherCode()
  {
    _data->fareBasisCodeData(INDEX)[OTHER_FARE_CLASS_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(BLANK, _data->getFareBasisCodeIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testGetFareBasisCodeIndicator()
  {
    _data->fareBasisCodeData(INDEX)[FARE_CLASS_CODE] = INDICATOR;
    CPPUNIT_ASSERT_EQUAL(INDICATOR, _data->getFareBasisCodeIndicator(FARE_CLASS_CODE, INDEX));
  }

  void testHasConsistentBookingCodes_Empty() { CPPUNIT_ASSERT(_data->hasConsistentBookingCodes()); }

  void testHasConsistentBookingCodes_EmptyBoth()
  {
    _data->fareBookingCode(INDEX).clear();
    _data->fareSecondaryBookingCode(INDEX).clear();

    CPPUNIT_ASSERT(_data->hasConsistentBookingCodes());
  }

  void testHasConsistentBookingCodes_EmptyPrimary()
  {
    BookingCode codes[] = { "A", "B", "C", "D" };
    _data->fareBookingCode(INDEX).clear();
    _data->fareSecondaryBookingCode(INDEX).assign(codes, codes + 4);

    CPPUNIT_ASSERT(_data->hasConsistentBookingCodes());
  }

  void testHasConsistentBookingCodes_EmptySecondary()
  {
    BookingCode codes[] = { "A", "B", "C", "D" };
    _data->fareBookingCode(INDEX).assign(codes, codes + 4);
    _data->fareSecondaryBookingCode(INDEX).clear();

    CPPUNIT_ASSERT(_data->hasConsistentBookingCodes());
  }

  void testHasConsistentBookingCodes_Overlapping()
  {
    BookingCode codes[] = { "A", "B", "C", "D" };
    _data->fareBookingCode(INDEX).assign(codes, codes + 4);
    _data->fareSecondaryBookingCode(INDEX).assign(codes + 2, codes + 4);
    ;

    CPPUNIT_ASSERT(!_data->hasConsistentBookingCodes());
  }

  void testHasConsistentBookingCodes_Separation()
  {
    BookingCode codes[] = { "A", "B", "C", "D" };
    _data->fareBookingCode(INDEX).assign(codes, codes + 2);
    _data->fareSecondaryBookingCode(INDEX).assign(codes + 2, codes + 4);
    ;

    CPPUNIT_ASSERT(_data->hasConsistentBookingCodes());
  }
};

const char BrandedFaresDataTest::BLANK = ' ';
const char BrandedFaresDataTest::INDICATOR = 'X';
const BookingCode BrandedFaresDataTest::BOOKING_CODE = "Y";
const BookingCode BrandedFaresDataTest::OTHER_BOOKING_CODE = "T";
const FareClassCode BrandedFaresDataTest::FARE_CLASS_CODE = "CX";
const FareClassCode BrandedFaresDataTest::OTHER_FARE_CLASS_CODE = "XC";

CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresDataTest);

} // tse
