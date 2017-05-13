
#include "Common/TseConsts.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "Taxes/Common/ReissueExchangeDateSetter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class ReissueExchangeDateSetterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissueExchangeDateSetterTest);
  CPPUNIT_TEST(testSecondRoeSetInTrxWhenFarePathUsingSecondRoe);
  CPPUNIT_TEST(testTicketingDateChangedToCurrentDateWhenTheyAreNotSame);
  CPPUNIT_TEST(testTicketingDateNotChangedWhenAlreadySetCorrect);
  CPPUNIT_TEST(testTicketingDateNotChangedWhenAlreadySetCorrectReissue);

  CPPUNIT_TEST(testUseHistoricalDateReturnFalseWhenAnyCurrentFare);
  CPPUNIT_TEST(testUseHistoricalDateReturnTrueWhenNoCurrentFare);
  CPPUNIT_TEST(testSecondRoeNotSetInTrxWhenFarePathUsingFirstRoe);
  CPPUNIT_TEST(testSecondRoeNotSetInTrxWhenSecondRoeDateEmpty);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fu1.paxTypeFare() = &_ptf1;
    _fu2.paxTypeFare() = &_ptf2;
    _pu.fareUsage().push_back(&_fu1);
    _pu.fareUsage().push_back(&_fu2);

    _fp.pricingUnit().push_back(&_pu);
    _info1._flag = FareMarket::RetrievCurrent;
    _info2._flag = FareMarket::RetrievCurrent;
    _ptf1.retrievalInfo() = &_info1;
    _ptf2.retrievalInfo() = &_info2;

    _rexTrx.setOriginalTktIssueDT() = DateTime(2009, 12, 12);
    _rexTrx.previousExchangeDT() = DateTime::emptyDate();
    _seg1 = _memHandle.create<AirSeg>();
    _pu.travelSeg().push_back(_seg1);

    _exchangeReissue = EXCHANGE;
    _fp.exchangeReissue() = _exchangeReissue;

    _dateSetter = _memHandle.insert(new ReissueExchangeDateSetter(_rexTrx, _fp));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testSecondRoeSetInTrxWhenFarePathUsingSecondRoe()
  {
    setTrxAsReissueExchange();
    _fp.useSecondRoeDate() = true;
    _rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 12);
    {
      ReissueExchangeDateSetter dateSetter = ReissueExchangeDateSetter(_rexTrx, _fp);
      CPPUNIT_ASSERT(_rexTrx.useSecondROEConversionDate());
      CPPUNIT_ASSERT(dateSetter._resetTrxSecondRoeIndicator);
    }
    // following test testing destructor sets the indicator back
    CPPUNIT_ASSERT(!_rexTrx.useSecondROEConversionDate());
  }

  void testTicketingDateChangedToCurrentDateWhenTheyAreNotSame()
  {
    setTrxAsReissueExchange();
    _rexTrx.ticketingDate() = DateTime(2010, 3, 12);
    _rexTrx.currentTicketingDT() = DateTime(2010, 3, 30);
    {
      ReissueExchangeDateSetter dateSetter = ReissueExchangeDateSetter(_rexTrx, _fp);
      CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 12), dateSetter._savedTicketingDate);
      CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 30), _rexTrx.ticketingDate());
    }
    // now test if destrcutor reset the ticketing date
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 12), _rexTrx.ticketingDate());
  }

  void testTicketingDateNotChangedWhenAlreadySetCorrect()
  {
    setTrxAsReissueExchange();
    _rexTrx.ticketingDate() = DateTime(2010, 3, 30);
    _rexTrx.currentTicketingDT() = DateTime(2010, 3, 30);
    {
      CPPUNIT_ASSERT(_dateSetter->_savedTicketingDate.isEmptyDate());
      CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 30), _rexTrx.ticketingDate());
    }
    // now test if destrcutor reset the ticketing date
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 30), _rexTrx.ticketingDate());
  }

  void testTicketingDateNotChangedWhenAlreadySetCorrectReissue()
  {
    _exchangeReissue = REISSUE;
    setTrxAsReissueExchange();
    _rexTrx.ticketingDate() = DateTime(2010, 3, 30);
    _rexTrx.currentTicketingDT() = DateTime(2010, 3, 30);
    {
      CPPUNIT_ASSERT(_dateSetter->_savedTicketingDate.isEmptyDate());
      CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 30), _rexTrx.ticketingDate());
    }
    // now test if destrcutor reset the ticketing date
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 3, 30), _rexTrx.ticketingDate());
  }

  void testUseHistoricalDateReturnFalseWhenAnyCurrentFare()
  {
    _info1._flag = FareMarket::RetrievHistorical;
    _info2._flag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(!_dateSetter->useHistoricalDate());
  }

  void testUseHistoricalDateReturnTrueWhenNoCurrentFare()
  {
    _info1._flag = FareMarket::RetrievHistorical;
    _info2._flag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(_dateSetter->useHistoricalDate());
  }

  void testSecondRoeNotSetInTrxWhenFarePathUsingFirstRoe()
  {
    setTrxAsReissueExchange();
    _fp.useSecondRoeDate() = false;
    _rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 12);
    CPPUNIT_ASSERT(!_rexTrx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(!_dateSetter->_resetTrxSecondRoeIndicator);
  }

  void testSecondRoeNotSetInTrxWhenSecondRoeDateEmpty()
  {
    setTrxAsReissueExchange();
    _fp.useSecondRoeDate() = true;
    _rexTrx.newItinSecondROEConversionDate() = DateTime::emptyDate();
    CPPUNIT_ASSERT(!_rexTrx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(!_dateSetter->_resetTrxSecondRoeIndicator);
  }

private:
  void setTrxAsReissueExchange()
  {
    _rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    _rexTrx.setRexPrimaryProcessType('A');
  }

  RexPricingTrx _rexTrx;
  Indicator _exchangeReissue;
  FarePath _fp;
  TestMemHandle _memHandle;
  ReissueExchangeDateSetter* _dateSetter;

  PricingUnit _pu;
  FareUsage _fu1;
  PaxTypeFare _ptf1;
  FareUsage _fu2;
  PaxTypeFare _ptf2;
  AirSeg* _seg1;
  FareMarket::RetrievalInfo _info1;
  FareMarket::RetrievalInfo _info2;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReissueExchangeDateSetterTest);
};
