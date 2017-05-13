//-----------------------------------------------------------------------------
//
//  File:     Diag194CollectorTest.cpp
//
//  Author :  Grzegorz Wanke
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag194Collector.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RefundPricingTrx.h"

using namespace tse;

namespace tse
{

namespace
{
const std::string expectedSegDescRow = " 3 AA 123X 06JUN06 KRKDFW  C/F\n";
const std::string flownSegDescRow = " 3 AA 123X 06JUN06 KRKDFW  F\n";
const std::string header = "***************** START DIAG 194 ************************** \n";
const std::string footer = "***************** END   DIAG 194 ************************** \n";
}
class Diag194CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag194CollectorTest);
  CPPUNIT_TEST(testAddRefundCodeF);
  CPPUNIT_TEST(testAddRefundCodeU);
  CPPUNIT_TEST(testAddChangeStatusC);
  CPPUNIT_TEST(testAddChangeStatusU);
  CPPUNIT_TEST(testAddChangeStatusI);
  CPPUNIT_TEST(testAddChangeStatusO);
  CPPUNIT_TEST(testAddReshopCodeNone);
  CPPUNIT_TEST(testAddReshopCodeShopped);
  CPPUNIT_TEST(testPrepareSegmentDescriptionRow);
  CPPUNIT_TEST(testOutputOperatorItin);
  CPPUNIT_TEST(testOutputOperatorRexPricingTrx);
  CPPUNIT_TEST(testOutputOperatorRefundPricingTrx);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();

    _diag194Collector = _memH.create<Diag194Collector>();
    _diag194Collector->_active = true;

    AirSeg* airSeg = _memH.create<AirSeg>();
    airSeg->carrier() = "AA";
    airSeg->flightNumber() = 123;
    airSeg->setBookingCode(BookingCode('X'));
    airSeg->departureDT() = DateTime(2006, 6, 6);
    airSeg->pssDepartureDate() = "2007-07-07";
    airSeg->origAirport() = "KRK";
    airSeg->destAirport() = "DFW";
    airSeg->pnrSegment() = 3;
    airSeg->unflown() = false;
    _diag194Collector->_travelSeg = airSeg;

    _itin = _memH.create<ExcItin>();
    _itin->travelSeg().assign(2, const_cast<TravelSeg*>(_diag194Collector->_travelSeg));
  }

  void tearDown() { _memH.clear(); }

  void testAddRefundCodeF()
  {
    _diag194Collector->addRefundCode();

    CPPUNIT_ASSERT_EQUAL(std::string("F"), _diag194Collector->str());
  }

  void testAddRefundCodeU()
  {
    (const_cast<TravelSeg*>(_diag194Collector->_travelSeg))->unflown() = true;
    _diag194Collector->addRefundCode();

    CPPUNIT_ASSERT_EQUAL(std::string("U"), _diag194Collector->str());
  }

  void testAddChangeStatusC()
  {
    _diag194Collector->addChangeStatus();

    CPPUNIT_ASSERT_EQUAL(std::string("C"), _diag194Collector->str());
  }

  void testAddChangeStatusU()
  {
    (const_cast<TravelSeg*>(_diag194Collector->_travelSeg))->changeStatus() = TravelSeg::UNCHANGED;
    _diag194Collector->addChangeStatus();

    CPPUNIT_ASSERT_EQUAL(std::string("U"), _diag194Collector->str());
  }

  void testAddChangeStatusI()
  {
    (const_cast<TravelSeg*>(_diag194Collector->_travelSeg))->changeStatus() =
        TravelSeg::INVENTORYCHANGED;
    _diag194Collector->addChangeStatus();

    CPPUNIT_ASSERT_EQUAL(std::string("I"), _diag194Collector->str());
  }

  void testAddChangeStatusO()
  {
    (const_cast<TravelSeg*>(_diag194Collector->_travelSeg))->changeStatus() =
        TravelSeg::CONFIRMOPENSEGMENT;
    _diag194Collector->addChangeStatus();

    CPPUNIT_ASSERT_EQUAL(std::string("O"), _diag194Collector->str());
  }

  void testAddReshopCodeNone()
  {
    _diag194Collector->_processingExItin = true;
    _diag194Collector->addReshopCode();

    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag194Collector->str());
  }

  void testAddReshopCodeShopped()
  {
    (const_cast<TravelSeg*>(_diag194Collector->_travelSeg))->isShopped() = true;
    _diag194Collector->addReshopCode();

    CPPUNIT_ASSERT_EQUAL(std::string("C  SHOPPED"), _diag194Collector->str());
  }

  void testPrepareSegmentDescriptionRow()
  {
    _diag194Collector->prepareSegmentDescriptionRow(_diag194Collector->_travelSeg);

    CPPUNIT_ASSERT_EQUAL(expectedSegDescRow, _diag194Collector->str());
  }

  void testOutputOperatorItin()
  {
    *_diag194Collector << *_itin;

    std::string expected = expectedSegDescRow;
    expected.append(expectedSegDescRow);

    CPPUNIT_ASSERT_EQUAL(expected, _diag194Collector->str());
  }

  void testOutputOperatorRexPricingTrx()
  {
    RexPricingTrx rexPricingTrx;
    rexPricingTrx.setOriginalTktIssueDT() = DateTime(2008, 6, 6);
    rexPricingTrx.ticketingDate() = DateTime(2009, 6, 6);
    rexPricingTrx.exchangeItin().push_back(_itin);
    rexPricingTrx.newItin().push_back(_itin);

    *_diag194Collector << rexPricingTrx;

    std::string expected = header;
    expected.append("    ORIGINAL TICKET DATE D92: 2008-06-06\n");
    expected.append("     CURRENT TICKET DATE D07: 2009-06-06\n");
    expected.append("*** PREVIOUSLY TICKETED ITINERARY ***\n");
    expected.append(expectedSegDescRow);
    expected.append(expectedSegDescRow);
    expected.append("*** NEW  ITINERARY ***\n");
    expected.append(expectedSegDescRow);
    expected.append(expectedSegDescRow);
    expected.append(" EXCHANGE REISSUE: N\n\n");
    expected.append(footer);

    CPPUNIT_ASSERT_EQUAL(expected, _diag194Collector->str());
  }

  void testOutputOperatorRefundPricingTrx()
  {
    TestConfigInitializer::setValue("AUTOMATED_REFUND_CAT33_ENABLED", "Y", "TAX_SVC", true);

    RexPricingOptions options;
    options.setNetSellingIndicator(true);

    RefundPricingTrx refundPricingTrx;
    refundPricingTrx.setOptions(&options);
    refundPricingTrx.setOriginalTktIssueDT() = DateTime(2008, 6, 6);
    refundPricingTrx.currentTicketingDT() = DateTime(2009, 6, 6);
    refundPricingTrx.exchangeItin().push_back(_itin);
    refundPricingTrx.newItin().push_back(_itin);

    *_diag194Collector << refundPricingTrx;

    std::string expected = header;
    expected.append("    ORIGINAL TICKET DATE D92: 2008-06-06\n");
    expected.append("     CURRENT TICKET DATE D07: 2009-06-06\n");
    expected.append("*** ORIGINALLY TICKETED ITINERARY ***\n");
    expected.append("FARE LEVEL: NET\n");
    expected.append(flownSegDescRow);
    expected.append(flownSegDescRow);
    expected.append("*** FLOWN SEGMENTS ***\n");
    expected.append(flownSegDescRow);
    expected.append(flownSegDescRow);
    expected.append(footer);

    CPPUNIT_ASSERT_EQUAL(expected, _diag194Collector->str());
  }

private:
  Diag194Collector* _diag194Collector;
  ExcItin* _itin;
  TestMemHandle _memH;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag194CollectorTest);
}
