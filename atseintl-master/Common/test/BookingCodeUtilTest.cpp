#include "Common/BookingCodeUtil.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag411Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;
using namespace tse;

namespace tse
{

FALLBACKVALUE_DECL(fallbackAAExcludedBookingCode);

class BookingCodeUtilTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(BookingCodeUtilTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testValidateEBC_Empty_FB_On);
  CPPUNIT_TEST(testValidateEBC_Empty_FB_Off);
  CPPUNIT_TEST(testEBC_ExactMatch_FB_On);
  CPPUNIT_TEST(testEBC_ExactMatch_FB_Off);
  CPPUNIT_TEST(testEBC_SomeMatch_FB_On);
  CPPUNIT_TEST(testEBC_SomeMatch_FB_Off);
  CPPUNIT_TEST(testEBC_ExcessMatch_FB_On);
  CPPUNIT_TEST(testEBC_ExcessMatch_FB_Off);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  PricingTrx* trx;
  FarePath* farePath;
  PricingUnit* pricingUnit;
  FareUsage* fareUsage;
  PricingOptions options;
public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    trx = _memHandle.create<PricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    //trx->setOptions(_memHandle.create<PricingOptions>());
    options.alternateCurrency() = USD;
    options.aaBasicEBC() = "ABC";
    trx->setOptions(&options);
  }

  void tearDown() { _memHandle.clear(); }
  void testConstructor() {}
  /////////////////// EBC Empty case /////////////////////////////////
  void testValidateEBC_Empty_FB_On()
  {
    options.aaBasicEBC() = "";
    trx->setOptions(&options);

    fallback::value::fallbackAAExcludedBookingCode.set(true);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> bkgCodes, vModifiedBkgCodes;
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }
  void testValidateEBC_Empty_FB_Off()
  {
    options.aaBasicEBC() = "";
    trx->setOptions(&options);

    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> bkgCodes, vModifiedBkgCodes;
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }

  /////////////////// EBC Exactly matches with Booking Codes /////////////////////////////////
  void testEBC_ExactMatch_FB_On()
  {
    fallback::value::fallbackAAExcludedBookingCode.set(true);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('A');
    BookingCode b('B');
    BookingCode c('C');
    std::vector<BookingCode> bkgCodes{a,b,c};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }
  void testEBC_ExactMatch_FB_Off()
  {
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('A');
    BookingCode b('B');
    BookingCode c('C');
    std::vector<BookingCode> bkgCodes{a,b,c};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(!bResult);
  }

  /////////////////// EBC is smaller and has some  matches with Booking Codes /////////////////////////////////
  void testEBC_SomeMatch_FB_On()
  {
    options.aaBasicEBC() = "DF";
    trx->setOptions(&options);
    fallback::value::fallbackAAExcludedBookingCode.set(true);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('C');
    BookingCode b('D');
    BookingCode c('E');
    BookingCode d('F');
    BookingCode e('G');
    std::vector<BookingCode> bkgCodes{a,b,c,d,e};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }
  void testEBC_SomeMatch_FB_Off()
  {
    options.aaBasicEBC() = "DF";
    trx->setOptions(&options);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('C');
    BookingCode b('D');
    BookingCode c('E');
    BookingCode d('F');
    BookingCode e('G');
    std::vector<BookingCode> bkgCodes{a,b,c,d,e};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }

  /////////////////// EBC is larger and matches with Booking Codes /////////////////////////////////
  void testEBC_ExcessMatch_FB_On()
  {
    options.aaBasicEBC() = "ABCDEFGH";
    trx->setOptions(&options);
    fallback::value::fallbackAAExcludedBookingCode.set(true);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('C');
    BookingCode b('D');
    BookingCode c('E');
    BookingCode d('F');
    BookingCode e('G');
    std::vector<BookingCode> bkgCodes{a,b,c,d,e};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(bResult);
  }
  void testEBC_ExcessMatch_FB_Off()
  {
    options.aaBasicEBC() = "ABCDEFGH";
    trx->setOptions(&options);
    CPPUNIT_ASSERT(trx);
    Diag411Collector diag;
    PaxTypeFare::SegmentStatus ss;
    std::vector<BookingCode> vModifiedBkgCodes;
    BookingCode a('C');
    BookingCode b('D');
    BookingCode c('E');
    BookingCode d('F');
    BookingCode e('G');
    std::vector<BookingCode> bkgCodes{a,b,c,d,e};
    bool bResult = BookingCodeUtil::validateExcludedBookingCodes(*trx, bkgCodes, ss, vModifiedBkgCodes, &diag);
    CPPUNIT_ASSERT(!bResult);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BookingCodeUtilTest);
} // namespace tse
