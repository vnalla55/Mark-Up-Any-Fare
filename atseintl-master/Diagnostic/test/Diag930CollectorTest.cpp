//----------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Diagnostic/Diag930Collector.h"
#include "DataModel/ShoppingTrx.h"
#include "Common/ClassOfService.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
class Diag930CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag930CollectorTest);
  CPPUNIT_TEST(testGetBookingCodeCabin_NoReBook);
  CPPUNIT_TEST(testGetBookingCodeCabin_ReBookHigherCabin);
  CPPUNIT_TEST(testGetBookingCodeCabin_ReBookLowerCabin);
  CPPUNIT_TEST_SUITE_END();

public:
  Diag930CollectorTest() : _trx(0), _dc(0) {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    Diagnostic& root = _trx->diagnostic();
    _dc = _memHandle.create<Diag930Collector>(root);
    _dc->rootDiag() = &root;
    _dc->diagnosticType() = Diagnostic930;
    _dc->enable(Diagnostic930);
    _dc->activate();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetBookingCodeCabin_NoReBook()
  {
    CabinType economyCabin;
    economyCabin.setEconomyClass();

    TravelSeg* tvlSeg = createTravelSeg("S", economyCabin);
    PaxTypeFare::SegmentStatus* segStat = createSegStat("", economyCabin);

    BookingCode bookingCode;
    CabinType bookedCabin;
    CabinType bookedCabinSentBack;

    _dc->getBookingCodeCabin(*tvlSeg, segStat, bookingCode, bookedCabin, bookedCabinSentBack);

    CPPUNIT_ASSERT_EQUAL(BookingCode("S"), bookingCode);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabin);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabinSentBack);
  }

  void testGetBookingCodeCabin_ReBookHigherCabin()
  {
    CabinType economyCabin;
    CabinType businessCabin;
    economyCabin.setEconomyClass();
    businessCabin.setBusinessClass();

    TravelSeg* tvlSeg = createTravelSeg("J", businessCabin);
    PaxTypeFare::SegmentStatus* segStat = createSegStat("S", economyCabin);

    BookingCode bookingCode;
    CabinType bookedCabin;
    CabinType bookedCabinSentBack;

    _dc->getBookingCodeCabin(*tvlSeg, segStat, bookingCode, bookedCabin, bookedCabinSentBack);

    CPPUNIT_ASSERT_EQUAL(BookingCode("S"), bookingCode);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabin);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabinSentBack);
  }

  void testGetBookingCodeCabin_ReBookLowerCabin()
  {
    CabinType economyCabin;
    CabinType businessCabin;
    economyCabin.setEconomyClass();
    businessCabin.setBusinessClass();

    TravelSeg* tvlSeg = createTravelSeg("S", economyCabin);
    tvlSeg->classOfService().clear();
    PaxTypeFare::SegmentStatus* segStat = createSegStat("J", businessCabin);

    BookingCode bookingCode;
    CabinType bookedCabin;
    CabinType bookedCabinSentBack;

    _dc->getBookingCodeCabin(*tvlSeg, segStat, bookingCode, bookedCabin, bookedCabinSentBack);

    // COS vector is empty so Y booking code is used instead of S
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), bookingCode);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabin);
    CPPUNIT_ASSERT_EQUAL(economyCabin, bookedCabinSentBack);
  }

private:
  TravelSeg* createTravelSeg(const BookingCode& bc, const CabinType& cabin)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->setBookingCode(bc);
    seg->bookedCabin() = cabin;
    return seg;
  }

  PaxTypeFare::SegmentStatus*
  createSegStat(const BookingCode& bkgCodeReBook, const CabinType& reBookedCabin)
  {
    PaxTypeFare::SegmentStatus* seg = _memHandle.create<PaxTypeFare::SegmentStatus>();
    seg->_bkgCodeReBook = bkgCodeReBook;
    seg->_reBookCabin = reBookedCabin;
    seg->_bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, bkgCodeReBook != "");
    return seg;
  }

  ShoppingTrx* _trx;
  Diag930Collector* _dc;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag930CollectorTest);
}
