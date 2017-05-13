//----------------------------------------------------------------------------
//  Copyright Sabre 2013
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
#include <string>
#include <map>
#include "Common/Code.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TypeConvert.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Xform/STLTicketingFeesRequestHandler.h"
#include <Common/ErrorResponseException.h>
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>


namespace tse
{
class MyDataHandleOB : public DataHandleMock
{
public:
  MyDataHandleOB(TestMemHandle& memHandle) : _memHandle(memHandle) {}
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    if (key == "DQ73")
    {
      std::vector<Customer*>* ret = _memHandle.create<std::vector<Customer*> >();
      Customer* c = _memHandle.create<Customer>();
      c->pseudoCity() = "DQ73";
      c->homePseudoCity() = "zzzz";
      c->requestCity() = "ABC";
      c->aaCity() = "DEF";
      c->defaultCur() = "CUR";
      ret->push_back(c);
      return *ret;
    }
    return DataHandleMock::getCustomer(key);
  }

  bool corpIdExists(const std::string& corpId, const DateTime& tvlDate)
  {
    if (corpId == "CRP01" || corpId == "CRP02")
      return true;
    return DataHandleMock::corpIdExists(corpId, tvlDate);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "DEF")
    {
      Loc* loc = 0;
      loc = _memHandle.create<Loc>();
      loc->loc() = "DEF";
      return loc;
    }
    return DataHandleMock::getLoc(locCode, date);
  }

private:
  TestMemHandle& _memHandle;
};

class STLTicketingFeesRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(STLTicketingFeesRequestHandlerTest);

  CPPUNIT_TEST(testParseAGI);
  CPPUNIT_TEST(testParseRequestOptions);
  CPPUNIT_TEST(testParseRequestedDiagnostic);
  CPPUNIT_TEST(testParseTravelSegmentThrowFlight);
  CPPUNIT_TEST(testParseFareBreakInformation);
  CPPUNIT_TEST(testParseFareBreakInformationThrowFareCompnentId);
  CPPUNIT_TEST(testParseFareBreakInformationThrowFareBasis);
  CPPUNIT_TEST(testParseFareBreakInformationThrowGoverningCarrier);
  CPPUNIT_TEST(testParseAccountCode);
  CPPUNIT_TEST(testParseCorpId);
  CPPUNIT_TEST(testParsePassengerIdentity);
  CPPUNIT_TEST(testPopulateCurrentItin);
  CPPUNIT_TEST(testPopulateCurrentItinThrowWhenNoFba);
  CPPUNIT_TEST(testFillMissingSegDataInCurrentItin);
  CPPUNIT_TEST(testParseFareBreakAccosiation);
  CPPUNIT_TEST(testAddTktDesignator);
  CPPUNIT_TEST(testAddTotalAmount);
  CPPUNIT_TEST(testonEndOBTicketingFeeRQ_IsRtw);
  CPPUNIT_TEST(testonEndOBTicketingFeeRQ_IsNotRtw);
  CPPUNIT_TEST(testSetSegmentTypeOpen);
  CPPUNIT_TEST(testSetSegmentTypeAir);
  CPPUNIT_TEST(testSetSegmentTypeUnknown);
  CPPUNIT_TEST(testSetSegmentTypeUnknownWhenArunk);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _tfpTrx = _memHandle.insert(new TktFeesPricingTrx);
    _trx = 0;
    _handler = _memHandle.insert(new STLTicketingFeesRequestHandler(_trx));
    _memHandle.insert(new MyDataHandleOB(_memHandle));
    _ptp = _memHandle.insert(new TktFeesRequest::PaxTypePayment);
    _fba = _memHandle.insert(new TktFeesRequest::TktFeesFareBreakAssociation);
    _itin = _memHandle.insert(new Itin);
    _tktFeesReq = _memHandle.insert(new TktFeesRequest);
    _todayDate = DateTime::localTime();
  }
  void tearDown() { _memHandle.clear(); }

  void createVector3Tseg()
  {
    AirSeg* ts1 = _memHandle.insert(new AirSeg);
    ts1->pnrSegment() = 567;
    ts1->segmentOrder() = 0;
    AirSeg* ts2 = _memHandle.insert(new AirSeg);
    ts2->pnrSegment() = 123;
    ts2->segmentOrder() = 1;
    AirSeg* ts3 = _memHandle.insert(new AirSeg);
    ts3->pnrSegment() = 234;
    ts3->segmentOrder() = 3;
    _handler->_tSeg.push_back(ts1);
    _handler->_tSeg.push_back(ts2);
    _handler->_tSeg.push_back(ts3);
  }
  void createVector3Tseg1Arunk()
  {
    AirSeg* ts1 = _memHandle.insert(new AirSeg);
    ts1->pnrSegment() = 567;
    ts1->segmentOrder() = 0;
    DateTime travelDate1 = _todayDate;
    ts1->departureDT() = travelDate1;
    DateTime arrDT1 = travelDate1.addDays(2);

    ts1->arrivalDT() = arrDT1;

    ArunkSeg* ts2 = _memHandle.insert(new ArunkSeg);
    ts2->segmentType() = Arunk;
    ts2->pnrSegment() = 123;
    ts2->segmentOrder() = 1;

    AirSeg* ts3 = _memHandle.insert(new AirSeg);
    ts3->pnrSegment() = 234;
    ts3->segmentOrder() = 3;
    DateTime travelDate3 = _todayDate;
    ts3->departureDT() = travelDate3.addDays(5);

    _handler->_tSeg.push_back(ts1);
    _handler->_tSeg.push_back(ts2);
    _handler->_tSeg.push_back(ts3);
    _handler->_itin->travelSeg() = _handler->_tSeg;
  }

  void testParseAGI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<Agent>DQ73</Agent>"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("DQ73"), tktFeesReq->ticketingAgent()->tvlAgencyPCC());
  }

  void testParseRequestOptions()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<RequestOptions validatingCarrier=\"BA\" "
                                            "requestTimeOfDay=\"600\" "
                                            "requestDate=\"2013-08-01\" />"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), trx->getRequest()->validatingCarrier());
  }

  void testParseRequestedDiagnostic()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<RequestedDiagnostic number=\"123\" />"));

    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);

    CPPUNIT_ASSERT_EQUAL(int16_t(123), trx->getRequest()->diagnosticNumber());
    CPPUNIT_ASSERT(trx->diagnostic().isActive());
    CPPUNIT_ASSERT_EQUAL(DiagnosticTypes(123), trx->diagnostic().diagnosticType());
  }

  void testParseTravelSegmentThrowFlight()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle,
                                         "<TravelSegment ticketDesignator=\"tDesignator\" "
                                         "segmentType=\"A\" segmentOrderNumber=\"4\" />"),
                                         std::runtime_error);
    CPPUNIT_ASSERT(_handler->_tSeg.empty());
  }

  void testParseFareBreakInformation()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<FareBreakInformation governingCarrier=\"CX\"  "
                                            "fareBasisCode=\"F666RT\" compOrderNum=\"2\" />"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());

    CPPUNIT_ASSERT(tktFeesReq != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tktFeesReq->tktFeesFareBreakPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(
        SequenceNumber(2),
        tktFeesReq->tktFeesFareBreakPerItin()[_handler->_itin].front()->fareComponentID());
    CPPUNIT_ASSERT_EQUAL(
        CarrierCode("CX"),
        tktFeesReq->tktFeesFareBreakPerItin()[_handler->_itin].front()->governingCarrier());
    CPPUNIT_ASSERT_EQUAL(
        FareClassCode("F666RT"),
        tktFeesReq->tktFeesFareBreakPerItin()[_handler->_itin].front()->fareBasis());
  }

  void testParseFareBreakInformationThrowFareCompnentId()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(
            _dataHandle,
            "<FareBreakInformation governingCarrier=\"CX\"  fareBasisCode=\"F666RT\"  />"),
        ErrorResponseException);
  }

  void testParseFareBreakInformationThrowFareBasis()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<FareBreakInformation governingCarrier=\"CX\"  compOrderNum=\"2\" />"),
        ErrorResponseException);
  }

  void testParseFareBreakInformationThrowGoverningCarrier()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<FareBreakInformation  fareBasisCode=\"F666RT\" compOrderNum=\"2\" />"),
        ErrorResponseException);
  }

  void testParseAccountCode()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, " <AccountCode>111111</AccountCode>"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());

    CPPUNIT_ASSERT(tktFeesReq != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tktFeesReq->accountCodeIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("111111"),
                         tktFeesReq->accountCodeIdPerItin()[_handler->_itin].front());
  }
  void testParseCorpId()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, " <CorpID>CRP01</CorpID>"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());

    CPPUNIT_ASSERT(tktFeesReq != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tktFeesReq->corpIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("CRP01"),
                         tktFeesReq->corpIdPerItin()[_handler->_itin].front());
  }

  void testParsePassengerIdentity()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle,
                        "<PassengerIdentity firstNameNumber=\"11\" surNameNumber=\"22\" "
                        "pnrNameNumber=\"33\" objectID=\"3\" />"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());

    CPPUNIT_ASSERT(tktFeesReq != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), tktFeesReq->paxId().size());

    CPPUNIT_ASSERT_EQUAL(SequenceNumber(11), tktFeesReq->paxId().front()->firstNameNumber());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(22), tktFeesReq->paxId().front()->surNameNumber());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(33), tktFeesReq->paxId().front()->pnrNameNumber());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(3), tktFeesReq->paxId().front()->objectId());
  }

  void testPopulateCurrentItin()
  {
    _handler->_itin = _itin;
    _handler->_pricingTrx = _tfpTrx;
    _handler->_tktFeesRequest = _tktFeesReq;

    _fba->segmentID() = 123;
    _handler->_tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_handler->_itin].push_back(
        _fba);
    createVector3Tseg();

    CPPUNIT_ASSERT_NO_THROW(_handler->populateCurrentItin());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _itin->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(int16_t(123), _itin->travelSeg().front()->pnrSegment());
  }

  void testPopulateCurrentItinThrowWhenNoFba()
  {
    _handler->_itin = _itin;
    _handler->_pricingTrx = _tfpTrx;
    _handler->_tktFeesRequest = _tktFeesReq;

    createVector3Tseg();

    CPPUNIT_ASSERT_THROW(_handler->populateCurrentItin(), std::runtime_error);
  }

  void testFillMissingSegDataInCurrentItin()
  {
    _handler->_itin = _itin;
    createVector3Tseg1Arunk();
    _handler->fillMissingSegDataInCurrentItin();

    CPPUNIT_ASSERT_EQUAL(size_t(3), _itin->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(_todayDate, _handler->_itin->travelSeg()[0]->departureDT());
    DateTime departureArunk1 = _todayDate.addDays(2);
    DateTime arrivalArunk1 = _todayDate.addDays(5);
    DateTime departure3Seg = _todayDate.addDays(5);

    CPPUNIT_ASSERT_EQUAL(departureArunk1, _handler->_itin->travelSeg()[1]->departureDT());
    CPPUNIT_ASSERT_EQUAL(arrivalArunk1, _handler->_itin->travelSeg()[1]->arrivalDT());
    CPPUNIT_ASSERT_EQUAL(departure3Seg, _handler->_itin->travelSeg()[2]->departureDT());
  }

  void testParseFareBreakAccosiation()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle,
                        "<FareBreakInformation governingCarrier=\"CX\"   fareBasisCode=\"F666RT\" "
                        "compOrderNum=\"2\" > 	<FareBreakAssociation ticketDesignator=\"AAAAAA\" "
                        "travelSegmentOrderNumber=\"5\" /> </FareBreakInformation>"));
    TktFeesPricingTrx* trx = dynamic_cast<TktFeesPricingTrx*>(_trx);
    TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(trx->getRequest());

    CPPUNIT_ASSERT(tktFeesReq != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         tktFeesReq->tktFeesFareBreakAssociationPerItin()[_handler->_itin].size());

    _fba = tktFeesReq->tktFeesFareBreakAssociationPerItin()[_handler->_itin].front();

    CPPUNIT_ASSERT_EQUAL(SequenceNumber(2), _fba->fareComponentID());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(5), _fba->segmentID());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("AAAAAA"), _fba->tktDesignator());
  }

  void testAddTktDesignator()
  {
    AirSeg* ts1 = _memHandle.insert(new AirSeg);
    ts1->pnrSegment() = 567;
    ts1->segmentOrder() = 0;
    AirSeg* ts2 = _memHandle.insert(new AirSeg);
    ts2->pnrSegment() = 123;
    ts2->segmentOrder() = 1;
    _handler->_itin = _itin;
    _handler->_itin->travelSeg().push_back(ts1);
    _handler->_itin->travelSeg().push_back(ts2);

    _fba->segmentID() = 123;
    _fba->tktDesignator() = "AAAAAA";
    _handler->_tktFeesRequest = _tktFeesReq;
    _handler->_tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_handler->_itin].push_back(
        _fba);

    CPPUNIT_ASSERT_NO_THROW(_handler->addTktDesignator());
    CPPUNIT_ASSERT_EQUAL(size_t(1),
                         _handler->_tktFeesRequest->tktDesignatorPerItin()[_handler->_itin].size());
  }

  void testAddTotalAmount()
  {
    _handler->_itin = _itin;
    _handler->_pricingTrx = _tfpTrx;
    _handler->_tktFeesRequest = _tktFeesReq;

    _ptp->amount() = 10;
    _ptp->currency() = "USD";
    _ptp->tktOverridePoint() = "DFW";
    _handler->addTotalAmount(_ptp);

    TktFeesRequest::PaxTypePayment* ptp = _tktFeesReq->paxTypePaymentPerItin()[_handler->_itin];
    CPPUNIT_ASSERT(ptp != 0);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), ptp->currency());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(10), ptp->amount());
    CPPUNIT_ASSERT_EQUAL(LocCode("DFW"), ptp->tktOverridePoint());
  }

  TktFeesRequest::TktFeesFareBreakInfo* prepareRtw()
  {
    _handler->_pricingTrx = _memHandle.create<PricingTrx>();
    _handler->_pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    Itin* itin = _memHandle.create<Itin>();
    _handler->_pricingTrx->itin().push_back(itin);
    TravelSeg* ts = _memHandle.create<AirSeg>();
    ts->origAirport() = ts->destAirport() = "RTW";
    itin->travelSeg().push_back(ts);
    _handler->_tktFeesRequest = _memHandle.create<TktFeesRequest>();

    for (size_t i = 0; i < 4; ++i)
    {
      _handler->_tktFeesRequest->tktFeesFareBreakPerItin()[itin].push_back(
          _memHandle.create<TktFeesRequest::TktFeesFareBreakInfo>());
      _handler->_tktFeesRequest->tktFeesFareBreakPerItin()[itin].back()->fareComponentID() = 1;
    }

    return _handler->_tktFeesRequest->tktFeesFareBreakPerItin()[itin][1];
  }

  void testonEndOBTicketingFeeRQ_IsRtw()
  {
    prepareRtw()->fareComponentID() = 1;
    _handler->onEndOBTicketingFeeRQ();
    CPPUNIT_ASSERT(_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testonEndOBTicketingFeeRQ_IsNotRtw()
  {
    prepareRtw()->fareComponentID() = 2;
    _handler->onEndOBTicketingFeeRQ();
    CPPUNIT_ASSERT(!_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testSetSegmentTypeOpen()
  {
    _handler->_currentTvlSegment = _memHandle.create<AirSeg>();
    _handler->setSegmentType(Open);
    CPPUNIT_ASSERT_EQUAL(Open, _handler->_currentTvlSegment->segmentType());
  }
  void testSetSegmentTypeAir()
  {
    _handler->_currentTvlSegment = _memHandle.create<AirSeg>();
    _handler->setSegmentType(Air);
    CPPUNIT_ASSERT_EQUAL(Air, _handler->_currentTvlSegment->segmentType());
  }
  void testSetSegmentTypeUnknown()
  {
    _handler->_currentTvlSegment = _memHandle.create<AirSeg>();
    _handler->setSegmentType('X');//Not valid type
    CPPUNIT_ASSERT_EQUAL(UnknownTravelSegType, _handler->_currentTvlSegment->segmentType());
  }
  void testSetSegmentTypeUnknownWhenArunk()
  {
    _handler->_currentTvlSegment = _memHandle.create<AirSeg>();
    _handler->setSegmentType(Arunk);//Arunk
    CPPUNIT_ASSERT_EQUAL(UnknownTravelSegType, _handler->_currentTvlSegment->segmentType());
  }

private:
  STLTicketingFeesRequestHandler* _handler;
  TestMemHandle _memHandle;
  Trx* _trx;
  DataHandle _dataHandle;
  TktFeesRequest* _tktFeesReq;
  TktFeesPricingTrx* _tfpTrx;
  TktFeesRequest::PaxTypePayment* _ptp;
  TktFeesRequest::TktFeesFareBreakAssociation* _fba;
  Itin* _itin;
  DateTime _todayDate;
};
CPPUNIT_TEST_SUITE_REGISTRATION(STLTicketingFeesRequestHandlerTest);
} // tse
