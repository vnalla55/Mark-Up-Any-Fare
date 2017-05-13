#include "DataModel/test/ExchangePricingTrxTest.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/Billing.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/DCFactory.h"

#include <boost/assign/std/vector.hpp>
#include "DataModel/MileageTypeData.h"
#include "DataModel/AirSeg.h"

using namespace boost::assign;
using namespace std;
using namespace tse;

CPPUNIT_TEST_SUITE_REGISTRATION(ExchangePricingTrxTest);

//-----------------------------------------------
void
ExchangePricingTrxTest::setUp()
{
  _trx = new ExchangePricingTrx();
  _rexTrx = new RexPricingTrx();
  _req = new PricingRequest();
  _options = new PricingOptions();
  _excItin = new ExcItin();
  _fareCompInfo = new FareCompInfo();
  _itin = new Itin();
  _aborter = new TrxAborter(_trx);
  _billing = new Billing();
  _fareMarket = new FareMarket();
  _tvlSeg = new AirSeg();
  _paxType = new PaxType();
  _diff1 = 0;
  _mil2 = 0;
  _mil1 = 0;
  _pup2 = 0;
  _pup1 = 0;
  _sto2 = 0;
  _sto1 = 0;
  _diff2 = 0;
  _sur2 = 0;
  _sur1 = 0; // initialize to NULL

  _rexTrx->secondaryExcReqType() = TAG_10_EXCHANGE;
  _rexTrx->setRequest(_req);
  _rexTrx->setOptions(_options);
  _excItin->fareComponent().push_back(_fareCompInfo);
  _rexTrx->exchangeItin().push_back(_excItin);
  _rexTrx->newItin().push_back(_itin);
  _trx->aborter() = _aborter;
  _rexTrx->billing() = _billing;
  _rexTrx->fareMarket().push_back(_fareMarket);
  _rexTrx->travelSeg().push_back(_tvlSeg);
  _rexTrx->paxType().push_back(_paxType);

  _rexTrx->currentTicketingDT() = DateTime(2009, 02, 01);
  _rexTrx->setOriginalTktIssueDT() = DateTime(2009, 02, 01);
  _rexTrx->reissueLocation() = "DFW";
  _rexTrx->purchaseDT() = DateTime(2009, 02, 01);
  _rexTrx->previousExchangeDT() = DateTime(2009, 02, 01);

  _rexTrx->itin().front()->calculationCurrency() = "USD";
  _rexTrx->exchangeItin().front()->calcCurrencyOverride() = "CAD";
  _trx->setRequest(_req);
}

//-----------------------------------------------
void
ExchangePricingTrxTest::tearDown()
{
  delete _tvlSeg;
  delete _mil2;
  delete _mil1;
  delete _pup2;
  delete _pup1;
  delete _sto2;
  delete _sto1;
  delete _diff2;
  delete _diff1;
  delete _sur2;
  delete _sur1;

  delete _paxType;
  delete _fareMarket;
  delete _billing;
  delete _aborter;
  delete _itin;
  delete _fareCompInfo;
  delete _excItin;
  delete _options;
  delete _req;
  delete _rexTrx;
  delete _trx;
}

//-----------------------------------------------
void
ExchangePricingTrxTest::testDefaultInitialization()
{
  // removed in version 5
  //   CPPUNIT_ASSERT(_trx->reqType() == "FE");
  CPPUNIT_ASSERT(_trx->exchangeItin().empty());
  CPPUNIT_ASSERT(_trx->newItin().empty());
}

//-----------------------------------------------
void
ExchangePricingTrxTest::testSetRequestType()
{
  _trx->reqType() = PARTIAL_EXCHANGE;
  CPPUNIT_ASSERT(_trx->reqType() == PARTIAL_EXCHANGE);
}

//-----------------------------------------------
void
ExchangePricingTrxTest::testExchangeTrxInitializedWithRexTrxRedirectExType()
{
  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT(_trx->reqType() == _rexTrx->secondaryExcReqType());
}

//-----------------------------------------------
void
ExchangePricingTrxTest::testExchangeTrxInitializedFareCompInfoEqualsRexTrxFareCompInfo()
{
  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT(_trx->exchangeItin()[0]->fareComponent() ==
                 _rexTrx->exchangeItin()[0]->fareComponent());
}

void
ExchangePricingTrxTest::testTag10ExchangeTrxInitializedTicketDTWithCurrentTktDT()
{
  const DateTime D93Date = DateTime(2009, 3, 15);
  const DateTime D07Date = DateTime(2009, 3, 20, 1, 2, 0);
  _rexTrx->secondaryExcReqType() = TAG_10_EXCHANGE;
  _rexTrx->purchaseDT() = D93Date;
  _rexTrx->currentTicketingDT() = D07Date;

  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->purchaseDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testPEExchangeTrxInitializedTicketDTWithPurchaseDT()
{
  const DateTime D93Date = DateTime(2009, 3, 15);
  const DateTime D07Date = DateTime(2009, 3, 20, 1, 2, 0);
  _rexTrx->secondaryExcReqType() = PARTIAL_EXCHANGE;
  _rexTrx->purchaseDT() = D93Date;
  _rexTrx->currentTicketingDT() = D07Date;

  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->purchaseDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testFEExchangeTrxInitializedTicketDTWithPurchaseDT()
{
  const DateTime D93Date = DateTime(2009, 3, 15);
  const DateTime D07Date = DateTime(2009, 3, 20, 1, 2, 0);
  _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;
  _rexTrx->purchaseDT() = D93Date;
  _rexTrx->currentTicketingDT() = D07Date;

  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->purchaseDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(D93Date, _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testPEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing()
{
  const DateTime D07Date = DateTime(2009, 3, 20, 1, 2, 0);
  _rexTrx->secondaryExcReqType() = PARTIAL_EXCHANGE;
  _rexTrx->purchaseDT() = DateTime::emptyDate();
  _rexTrx->currentTicketingDT() = D07Date;

  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->purchaseDT());
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testFEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing()
{
  const DateTime D07Date = DateTime(2009, 3, 20, 1, 2, 0);
  _rexTrx->secondaryExcReqType() = FULL_EXCHANGE;
  _rexTrx->purchaseDT() = DateTime::emptyDate();
  _rexTrx->currentTicketingDT() = D07Date;

  _trx->initialize(*_rexTrx, false);
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->purchaseDT());
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(D07Date, _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::populateExchangeOverridesInRexTrx()
{
  _sur1 = new SurchargeOverride();
  _sur2 = new SurchargeOverride();
  _diff1 = new DifferentialOverride();
  _diff2 = new DifferentialOverride();
  _sto1 = new StopoverOverride();
  _sto2 = new StopoverOverride();
  _pup1 = new PlusUpOverride();
  _pup2 = new PlusUpOverride();
  _mil1 = new MileageTypeData();
  _mil2 = new MileageTypeData();
  _tvlSeg = new AirSeg();
  _rexTrx->exchangeOverrides().surchargeOverride() += _sur1, _sur2;
  _rexTrx->exchangeOverrides().differentialOverride() += _diff1, _diff2;
  _rexTrx->exchangeOverrides().stopoverOverride() += _sto1, _sto2;
  _rexTrx->exchangeOverrides().plusUpOverride() += _pup1, _pup2;
  _rexTrx->exchangeOverrides().mileageTypeData() += _mil1, _mil2;

  uint16_t expectedJourneyStopOverCount(1);
  _rexTrx->exchangeOverrides().journeySTOOverrideCnt() = expectedJourneyStopOverCount;

  LocCode expectedTktCity("DFW");
  _rexTrx->exchangeOverrides().dummyFareMileTktCity()[_tvlSeg] = expectedTktCity;

  uint16_t expectedSegmentorder(2);
  _rexTrx->exchangeOverrides().dummyFCSegs()[_tvlSeg] = expectedSegmentorder;

  int16_t expectedFareMiles(3);
  _rexTrx->exchangeOverrides().dummyFareMiles()[_tvlSeg] = expectedFareMiles;

  LocCode expectedFareCity("TUL");
  _rexTrx->exchangeOverrides().dummyFareMileCity()[_tvlSeg] = expectedFareCity;

  char expectedForcedSideTrip('4');
  _rexTrx->exchangeOverrides().forcedSideTrip()[_tvlSeg] = expectedForcedSideTrip;
}

void
ExchangePricingTrxTest::testInitializeOverridesCopyFromARtoPORTtrx()
{
  populateExchangeOverridesInRexTrx();
  _trx->initialize(*_rexTrx, false);

  CPPUNIT_ASSERT_EQUAL(_sur1, _trx->exchangeOverrides().surchargeOverride()[0]);
  CPPUNIT_ASSERT_EQUAL(_sur2, _trx->exchangeOverrides().surchargeOverride()[1]);
  CPPUNIT_ASSERT_EQUAL(_diff1, _trx->exchangeOverrides().differentialOverride()[0]);
  CPPUNIT_ASSERT_EQUAL(_diff2, _trx->exchangeOverrides().differentialOverride()[1]);
  CPPUNIT_ASSERT_EQUAL(_sto1, _trx->exchangeOverrides().stopoverOverride()[0]);
  CPPUNIT_ASSERT_EQUAL(_sto2, _trx->exchangeOverrides().stopoverOverride()[1]);
  CPPUNIT_ASSERT_EQUAL(_pup1, _trx->exchangeOverrides().plusUpOverride()[0]);
  CPPUNIT_ASSERT_EQUAL(_pup2, _trx->exchangeOverrides().plusUpOverride()[1]);
  CPPUNIT_ASSERT_EQUAL(_mil1, _trx->exchangeOverrides().mileageTypeData()[0]);
  CPPUNIT_ASSERT_EQUAL(_mil2, _trx->exchangeOverrides().mileageTypeData()[1]);

  uint16_t expectedJourneyStopOverCount(1);
  CPPUNIT_ASSERT_EQUAL(expectedJourneyStopOverCount,
                       _trx->exchangeOverrides().journeySTOOverrideCnt());

  LocCode expectedTktCity("DFW");
  CPPUNIT_ASSERT_EQUAL(expectedTktCity, _trx->exchangeOverrides().dummyFareMileTktCity()[_tvlSeg]);

  uint16_t expectedSegmentorder(2);
  CPPUNIT_ASSERT_EQUAL(expectedSegmentorder, _trx->exchangeOverrides().dummyFCSegs()[_tvlSeg]);

  int16_t expectedFareMiles(3);
  CPPUNIT_ASSERT_EQUAL(expectedFareMiles, _trx->exchangeOverrides().dummyFareMiles()[_tvlSeg]);

  LocCode expectedFareCity("TUL");
  CPPUNIT_ASSERT_EQUAL(expectedFareCity, _trx->exchangeOverrides().dummyFareMileCity()[_tvlSeg]);

  char expectedForcedSideTrip('4');
  CPPUNIT_ASSERT_EQUAL(expectedForcedSideTrip, _trx->exchangeOverrides().forcedSideTrip()[_tvlSeg]);
}
//-----------------------------------------------
void
ExchangePricingTrxTest::testInitialize()
{
  _trx->initialize(*_rexTrx, false);
  _trx->setParentTrx(_rexTrx);
  CPPUNIT_ASSERT(_trx->getParentTrx());
  CPPUNIT_ASSERT(!_rexTrx->secondaryExcReqType().empty());
  CPPUNIT_ASSERT(_trx->reqType() == _rexTrx->secondaryExcReqType());
  CPPUNIT_ASSERT(_rexTrx->billing());
  CPPUNIT_ASSERT(_trx->billing() == _rexTrx->billing());
  CPPUNIT_ASSERT(_trx->getRequest()->lowFareRequested() == 'N');
  CPPUNIT_ASSERT(_rexTrx->currentTicketingDT() != DateTime::emptyDate());
  CPPUNIT_ASSERT(_trx->getRequest()->ticketingDT() == _rexTrx->currentTicketingDT());
  CPPUNIT_ASSERT(!_rexTrx->newItin().empty());
  CPPUNIT_ASSERT(_trx->newItin() == _rexTrx->newItin());
  CPPUNIT_ASSERT(!_rexTrx->fareMarket().empty());
  CPPUNIT_ASSERT(_trx->fareMarket() == _rexTrx->fareMarket());
  CPPUNIT_ASSERT(!_rexTrx->travelSeg().empty());
  CPPUNIT_ASSERT(_trx->travelSeg() == _rexTrx->travelSeg());
  CPPUNIT_ASSERT(!_rexTrx->paxType().empty());
  CPPUNIT_ASSERT(_trx->paxType() == _rexTrx->paxType());
  CPPUNIT_ASSERT(_rexTrx->originalTktIssueDT() != DateTime::emptyDate());
  CPPUNIT_ASSERT(_trx->originalTktIssueDT() == _rexTrx->originalTktIssueDT());
  CPPUNIT_ASSERT(_trx->lastTktReIssueDT() == DateTime::emptyDate());
  CPPUNIT_ASSERT(_rexTrx->currentTicketingDT() != DateTime::emptyDate());
  CPPUNIT_ASSERT(_trx->currentTicketingDT() == _rexTrx->currentTicketingDT());
  CPPUNIT_ASSERT(!_rexTrx->exchangeItin().empty());
  CPPUNIT_ASSERT(_trx->exchangeItin() == _rexTrx->exchangeItin());
  CPPUNIT_ASSERT(_trx->accompanyPaxType() == _rexTrx->accompanyPaxType());
  CPPUNIT_ASSERT(!_rexTrx->reissueLocation().empty());
  CPPUNIT_ASSERT(_trx->reissueLocation() == _rexTrx->reissueLocation());
  CPPUNIT_ASSERT(_rexTrx->purchaseDT() != DateTime::emptyDate());
  CPPUNIT_ASSERT(_trx->purchaseDT() == _rexTrx->purchaseDT());

  CPPUNIT_ASSERT(_trx->itin().front()->calculationCurrency() == "");
  CPPUNIT_ASSERT(_trx->exchangeItin().front()->calculationCurrency() ==
                 _trx->exchangeItin().front()->calcCurrencyOverride());
}

void
ExchangePricingTrxTest::testInitializeWhenDiagnosticIsActive()
{
  _trx->initialize(*_rexTrx, true);

  CPPUNIT_ASSERT(&_trx->diagnostic() == &_rexTrx->diagnostic());
}

void
ExchangePricingTrxTest::testInitializeWhenDiagnosticIsInActive()
{
  _trx->initialize(*_rexTrx, false);

  CPPUNIT_ASSERT(&_trx->diagnostic() != &_rexTrx->diagnostic());
}

void
ExchangePricingTrxTest::testExchangeApplyCurrentDTAsTicketDT()
{
  setsUpDates();
  _trx->purchaseDT() = DateTime::emptyDate();
  _trx->applyCurrentBsrRoeDate();
  CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testExchangeSetBsrRoeDateAsCurrentDT()
{
  setsUpDates();
  _trx->setBsrRoeDate(EXCHANGE);
  CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testReissueApplyHistoricalDTAsTicketDT()
{
  setsUpDates();
  _trx->previousExchangeDT() = DateTime::emptyDate();
  _trx->setHistoricalBsrRoeDate();
  _trx->applyHistoricalBsrRoeDate();
  CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testReissueApplyHistoricalDTAsPreviousEchangeDT()
{
  setsUpDates();
  _trx->setHistoricalBsrRoeDate();
  _trx->applyHistoricalBsrRoeDate();
  CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testReissueSetBsrRoeDateAsHistoricalD95Empty()
{
  setsUpDates();
  _trx->previousExchangeDT() = DateTime::emptyDate();
  _trx->setBsrRoeDate(REISSUE);
  CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::testReissueSetBsrRoeDateAsHistoricalD95Present()
{
  setsUpDates();
  _trx->setBsrRoeDate(REISSUE);
  CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _trx->getRequest()->ticketingDT());
  CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _trx->dataHandle().ticketDate());
}

void
ExchangePricingTrxTest::setsUpDates()
{
  const DateTime D07Date = DateTime(2009, 11, 30, 1, 2, 0);
  const DateTime D92Date = DateTime(2009, 11, 10, 1, 2, 0);
  const DateTime D95Date = DateTime(2009, 11, 20, 1, 2, 0);
  _trx->currentTicketingDT() = D07Date;
  _trx->setOriginalTktIssueDT() = D92Date;
  _trx->previousExchangeDT() = D95Date;
}
