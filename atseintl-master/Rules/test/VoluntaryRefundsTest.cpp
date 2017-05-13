#include "test/include/CppUnitHelperMacros.h"
#include "Rules/VoluntaryRefunds.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "Rules/PeriodOfStay.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class VoluntaryRefundsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VoluntaryRefundsTest);
  CPPUNIT_TEST(testPrintDates);
  CPPUNIT_TEST(testFullyFlown_Blank);
  CPPUNIT_TEST(testFullyFlown_WrongIndicator);
  CPPUNIT_TEST(testFullyFlown_A_fail);
  CPPUNIT_TEST(testFullyFlown_B_fail);
  CPPUNIT_TEST(testFullyFlown_A_pass);
  CPPUNIT_TEST(testFullyFlown_B_pass);

  CPPUNIT_TEST(testTktValidityInd_Blank);
  CPPUNIT_TEST(testTktValidityInd_WrongIndicator);
  CPPUNIT_TEST(testTktValidityInd_A_pass);
  CPPUNIT_TEST(testTktValidityInd_A_fail);
  CPPUNIT_TEST(testTktValidityInd_B_pass);
  CPPUNIT_TEST(testTktValidityInd_B_fail);
  CPPUNIT_TEST(testTktValidityInd_CheckPOSadd_pass);
  CPPUNIT_TEST(testTktValidityInd_CheckPOSadd_fail);

  CPPUNIT_TEST(testOriginalTicketIssueLocNULL);
  CPPUNIT_TEST(testOriginalTicketIssueLoc);
  CPPUNIT_TEST(testDetermineTicketValidityDeadline);

  CPPUNIT_TEST(testDetermineCustomer1Deadline);
  CPPUNIT_TEST(testMatchCustomer1_Blank);
  CPPUNIT_TEST(testMatchCustomer1_WrongIndicator);
  CPPUNIT_TEST(testMatchCustomer1_T_pass);
  CPPUNIT_TEST(testMatchCustomer1_T_fail);
  CPPUNIT_TEST(testMatchCustomer1_R_pass);
  CPPUNIT_TEST(testMatchCustomer1_R_fail);

  CPPUNIT_TEST(testOverrideDateTbl_Zero);
  CPPUNIT_TEST_SUITE_END();

public:
  static const Indicator NOT_VALID_INDICATOR = 'L';

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.insert(new RefundPricingTrx);
    _itin = _memHandle.insert(new Itin);
    _pricingUnit = _memHandle.insert(new PricingUnit);
    _ptf = _memHandle.insert(new PaxTypeFare);
    _vcRec3 = _memHandle.insert(new VoluntaryRefundsInfo);
    _voluntaryRefunds = _memHandle.insert(new VoluntaryRefunds(*_trx, 0, 0, *_ptf, *_vcRec3, 0));
    _trx->exchangeItin().push_back(_memHandle.insert(new ExcItin));
    _voluntaryRefunds =
        _memHandle.insert(new VoluntaryRefunds(*_trx, _itin, _pricingUnit, *_ptf, *_vcRec3, 0));
    _travelSegments =
        _memHandle.insert(new std::vector<TravelSeg*>(1, _memHandle.insert(new AirSeg)));
    _trx->setRequest(_memHandle.insert(new RexBaseRequest));
  }

  void tearDown() { _memHandle.clear(); }

  void testPrintDates()
  {
    _voluntaryRefunds->_dc = _memHandle.insert(new DiagCollector);
    _voluntaryRefunds->_dc->activate();
    std::list<std::pair<std::string, const DateTime*> > fields;
    DateTime dt(2000, 2, 2, 2, 2, 2);
    fields.push_back(std::make_pair("ONE POTATO", &dt));
    fields.push_back(std::make_pair("TWO POTATOES", &dt));
    std::string title = "\nTITLE";

    _voluntaryRefunds->printDates(title, 20, fields);
    CPPUNIT_ASSERT_EQUAL(std::string("\nTITLE\n"
                                     " ONE POTATO           : 2000-02-02T02:02:02\n"
                                     " TWO POTATOES         : 2000-02-02T02:02:02\n"),
                         _voluntaryRefunds->_dc->str());
  }

  void fullyFlownCallAndAssert(const Indicator byteValue, const bool expectedResult)

  {
    _vcRec3->fullyFlown() = byteValue;
    CPPUNIT_ASSERT_EQUAL(expectedResult, _voluntaryRefunds->matchFullyFlown());
  }

  void setSegmentUnflownStatus(const bool firstUnflown, const bool secondUnflown)
  {
    _travelSegments->push_back(_memHandle.insert(new AirSeg));
    (*_travelSegments)[0]->unflown() = firstUnflown;
    (*_travelSegments)[1]->unflown() = secondUnflown;
    _ptf->fareMarket() = _memHandle.insert(new FareMarket);
    _ptf->fareMarket()->travelSeg() = *_travelSegments;
  }

  void testFullyFlown_Blank() { fullyFlownCallAndAssert(VoluntaryRefunds::BLANK, true); }

  void testFullyFlown_WrongIndicator()
  {
    setSegmentUnflownStatus(true, false);
    fullyFlownCallAndAssert(NOT_VALID_INDICATOR, false);
  }

  void testFullyFlown_A_fail()
  {
    setSegmentUnflownStatus(false, true);
    fullyFlownCallAndAssert(VoluntaryRefunds::FULLY_FLOWN, false);
  }

  void testFullyFlown_B_fail()
  {
    setSegmentUnflownStatus(true, true);
    fullyFlownCallAndAssert(VoluntaryRefunds::PARTIALLY_FLOWN, false);
  }

  void testFullyFlown_A_pass()
  {
    setSegmentUnflownStatus(false, false);
    fullyFlownCallAndAssert(VoluntaryRefunds::FULLY_FLOWN, true);
  }

  void testFullyFlown_B_pass()
  {
    setSegmentUnflownStatus(false, true);
    fullyFlownCallAndAssert(VoluntaryRefunds::PARTIALLY_FLOWN, true);
  }

  void setUpTktValidityInd_A(const DateTime& originalDT)
  {
    _vcRec3->tktValidityInd() = VoluntaryRefunds::MATCH_ORIGINAL_TICKET_DATE;
    _trx->setOriginalTktIssueDT() = originalDT;
    _trx->currentTicketingDT() = DateTime(2005, 1, 1);
  }

  void setUpTktValidityInd_B(const DateTime& commenceDT)
  {
    _vcRec3->tktValidityInd() = VoluntaryRefunds::MATCH_COMMENCE_DATE;
    _trx->fullRefund() = false;
    _trx->exchangeItin().front()->travelSeg().swap(*_travelSegments);
    _trx->exchangeItin().front()->setTravelDate(commenceDT);
    _trx->currentTicketingDT() = DateTime(2005, 1, 1);
  }

  void testTktValidityInd_WrongIndicator()
  {
    _vcRec3->tktValidityInd() = NOT_VALID_INDICATOR;
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_Blank()
  {
    _vcRec3->tktValidityInd() = VoluntaryRefunds::BLANK;
    CPPUNIT_ASSERT(_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_A_pass()
  {
    setUpTktValidityInd_A(DateTime(2004, 9, 9));
    CPPUNIT_ASSERT(_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_A_fail()
  {
    setUpTktValidityInd_A(DateTime(2003, 9, 9));
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_B_pass()
  {
    setUpTktValidityInd_B(DateTime(2004, 9, 9));
    CPPUNIT_ASSERT(_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_B_fail()
  {
    setUpTktValidityInd_B(DateTime(2003, 9, 9));
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchTicketValidity());
  }

  void setUpPeriodOfStay()
  {
    _vcRec3->tktValidityPeriod() = "180";
    _vcRec3->tktValidityUnit() = "Db";
  }

  void testTktValidityInd_CheckPOSadd_pass()
  {
    setUpPeriodOfStay();
    setUpTktValidityInd_A(DateTime(2004, 9, 9));
    CPPUNIT_ASSERT(_voluntaryRefunds->matchTicketValidity());
  }

  void testTktValidityInd_CheckPOSadd_fail()
  {
    setUpPeriodOfStay();
    setUpTktValidityInd_A(DateTime(2004, 3, 3));
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchTicketValidity());
  }

  void testValidate_PASS()
  {
    std::vector<const tse::VoluntaryRefundsInfo*> r3vec(1, _vcRec3);
    _vcRec3->fullyFlown() = VoluntaryRefunds::BLANK;
    _vcRec3->tktValidityInd() = VoluntaryRefunds::BLANK;
    CPPUNIT_ASSERT_EQUAL(size_t(0), _trx->refundOptions(_ptf));
    CPPUNIT_ASSERT_EQUAL(PASS, _voluntaryRefunds->validate());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _trx->refundOptions(_ptf));
  }

  void testValidate_FAIL()
  {
    std::vector<const tse::VoluntaryRefundsInfo*> r3vec(1, _vcRec3);
    setUpTktValidityInd_A(DateTime(2003, 9, 9));
    CPPUNIT_ASSERT_EQUAL(size_t(0), _trx->refundOptions(_ptf));
    CPPUNIT_ASSERT_EQUAL(FAIL, _voluntaryRefunds->validate());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _trx->refundOptions(_ptf));
  }

  void testDetermineTicketValidityDeadline()
  {
    setUpTktValidityInd_B(DateTime(2003, 9, 9));
    _trx->exchangeItin().front()->travelSeg().front()->origin() = _memHandle.insert(new Loc);
    DateTime deadline;
    CPPUNIT_ASSERT_EQUAL(_trx->exchangeItin().front()->travelSeg().front()->origin(),
                         _voluntaryRefunds->determineTicketValidityDeadline(deadline));
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 9, 9), deadline);
  }

  void setUpRexBaseRequest()
  {
    _trx->setRequest(_memHandle.insert(new RexBaseRequest));
    _agent = _memHandle.insert(new Agent);
    _agent->agentCity() = "KRK";
    _agent->agentLocation() = _memHandle.insert(new Loc);
  }

  void testOriginalTicketIssueLoc()
  {
    setUpRexBaseRequest();
    static_cast<RexBaseRequest&>(*_trx->getRequest()).prevTicketIssueAgent() = _agent;
    CPPUNIT_ASSERT_EQUAL(_agent->agentLocation(), _voluntaryRefunds->originalTicketIssueLoc());
  }

  void testOriginalTicketIssueLocNULL()
  {
    setUpRexBaseRequest();
    CPPUNIT_ASSERT_EQUAL(static_cast<const Loc*>(0), _voluntaryRefunds->originalTicketIssueLoc());
  }

  void setUpCustomer1Common(const Indicator customer1)
  {
    _trx->fullRefund() = true;
    _vcRec3->customer1ResTkt() = customer1;
    _trx->exchangeItin().front()->travelSeg().swap(*_travelSegments);
    _trx->currentTicketingDT() = DateTime(2005, 1, 1);
    _vcRec3->customer1Period() = "1";
    _vcRec3->customer1Unit() = "Nb";
  }

  void setCustomer1Ind_T(const DateTime& originalDT)
  {
    setUpCustomer1Common(VoluntaryRefunds::MATCH_TICKET_DATE);
    _trx->setOriginalTktIssueDT() = originalDT;
  }

  void setCustomer1Ind_R(const DateTime& resTktDT)
  {
    setUpCustomer1Common(VoluntaryRefunds::MATCH_EARLIEST_RESERVATIONS_DATE);
    _trx->exchangeItin().front()->travelSeg().front()->bookingDT() = resTktDT;
  }

  void testDetermineCustomer1Deadline()
  {
    setCustomer1Ind_R(DateTime(2003, 9, 9, 9, 9, 9));
    DateTime deadline;
    CPPUNIT_ASSERT_EQUAL(_trx->exchangeItin().front()->travelSeg().front()->origin(),
                         _voluntaryRefunds->determineCustomer1Deadline(deadline));
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 9, 9, 9, 9, 9), deadline);
  }

  void testMatchCustomer1_Blank()
  {
    setUpCustomer1Common(VoluntaryRefunds::BLANK);
    CPPUNIT_ASSERT(_voluntaryRefunds->matchCustomer1());
  }

  void testMatchCustomer1_WrongIndicator()
  {
    setUpCustomer1Common(NOT_VALID_INDICATOR);
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchCustomer1());
  }

  void testMatchCustomer1_T_pass()
  {
    setCustomer1Ind_T(DateTime(2006, 1, 1));
    CPPUNIT_ASSERT(_voluntaryRefunds->matchCustomer1());
  }

  void testMatchCustomer1_T_fail()
  {
    setCustomer1Ind_T(DateTime(2004, 1, 1));
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchCustomer1());
  }
  void testMatchCustomer1_R_pass()
  {
    setCustomer1Ind_R(DateTime(2006, 1, 1));
    CPPUNIT_ASSERT(_voluntaryRefunds->matchCustomer1());
  }
  void testMatchCustomer1_R_fail()
  {
    setCustomer1Ind_R(DateTime(2004, 1, 1));
    CPPUNIT_ASSERT(!_voluntaryRefunds->matchCustomer1());
  }

  void testOverrideDateTbl_Zero()
  {
    _vcRec3->overrideDateTblItemNo() = 0;
    CPPUNIT_ASSERT_EQUAL(true, _voluntaryRefunds->matchOverrideDateTable());
  }

private:
  TestMemHandle _memHandle;
  RefundPricingTrx* _trx;
  Itin* _itin;
  PricingUnit* _pricingUnit;
  PaxTypeFare* _ptf;
  VoluntaryRefundsInfo* _vcRec3;
  VoluntaryRefunds* _voluntaryRefunds;
  std::vector<TravelSeg*>* _travelSegments;
  Agent* _agent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(VoluntaryRefundsTest);
};
