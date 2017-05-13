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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TypeConvert.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxType.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "DBAccess/FareCalcConfig.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/STLTktFeesPricingResponseFormatter.h"
#include <Common/ErrorResponseException.h>
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{

class STLTktFeesPricingResponseFormatterTest : public CppUnit::TestFixture
{
  class STLTktFeesPricingResponseFormatterMock : public STLTktFeesPricingResponseFormatter,
                                                 public CppUnit::TestFixture
  {
    friend class STLTktFeesPricingResponseFormatterTest;

  public:
    STLTktFeesPricingResponseFormatterMock() : STLTktFeesPricingResponseFormatter() {}
    ~STLTktFeesPricingResponseFormatterMock() {}
  };

  CPPUNIT_TEST_SUITE(STLTktFeesPricingResponseFormatterTest);
  CPPUNIT_TEST(testProcessTktFeesSolutionPerPaxNullItinerary);
  CPPUNIT_TEST(testProcessTktFeesSolutionPerPaxTypeMinFees);
  CPPUNIT_TEST(testProcessTktFeesSolutionPerPaxTypeMaxFees);
  CPPUNIT_TEST(testProcessPassengerIdentity);
  CPPUNIT_TEST(testPrepareMessage);
  CPPUNIT_TEST(testPrepareOBFeeWithoutItin);
  CPPUNIT_TEST(testPrepareOBFeeWithItin);
  CPPUNIT_TEST(testPrepareOBFeeWhenFeeAmmntLTZero);
  CPPUNIT_TEST(testPrepareOBFeeWhenFeePcntGTZero);
  CPPUNIT_TEST(testPrepareOBFeeNullPointerToPTPData);
  CPPUNIT_TEST(testPrepareOBFeeWhenChargeAmountNonZero);
  CPPUNIT_TEST(testGetCurrencyCodeModified);
  CPPUNIT_TEST(testGetCurrencyCodeSetCorrectly);
  CPPUNIT_TEST(testPrepareHostPortInfo);
  CPPUNIT_TEST(testClearAllFeesEmpty);
  CPPUNIT_TEST(testCheckForZeroMaximumFalseWhenNonZero);
  CPPUNIT_TEST(testCheckForZeroMaximumTrueWhenZeroAndBin);
  CPPUNIT_TEST(testCheckForZeroMaximumTrueWhenZeroAndBinEmpty);
  CPPUNIT_TEST(testGetChargeAmount);
  CPPUNIT_TEST(testGetChargeAmountWhenAmtNotInRequest);
  CPPUNIT_TEST(testGetPaymentCurrencyWhenPtpNotFound);
  CPPUNIT_TEST(testGetPaymentCurrencyWhenCurrencyFound);

  CPPUNIT_TEST_SUITE_END();

public:
  void testProcessTktFeesSolutionPerPaxNullItinerary()
  {
    _farePath->maximumObFee() = _feeInfo;
    _feeInfo->feeAmount() = 100;
    _tktFeesReq->paxTypePaymentPerItin()[_itin] = 0;
    //_mock->processTktFeesSolutionPerPaxType(*_tktFeesPricingTrx, _itin, *_construct, *_farePath);
    CPPUNIT_ASSERT_THROW(_mock->processTktFeesSolutionPerPaxType(
        *_tktFeesPricingTrx, _itin, *_construct, *_farePath);
                         , ErrorResponseException);
  }

  void testProcessPassengerIdentity()
  {
    _tktFeesPricingTrx->setRequest(_tktFeesReq);
    _tktFeesReq->paxId().push_back(_passID);
    _passID->firstNameNumber() = 1;
    _passID->pnrNameNumber() = 2;
    _passID->surNameNumber() = 3;
    _mock->processPassengerIdentity(*_tktFeesPricingTrx, *_construct, 0);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<PassengerIdentity") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("firstNameNumber=\"1\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("pnrNameNumber=\"2\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("surNameNumber=\"3\"") != std::string::npos);
  }

  void testPrepareMessage()
  {
    _formatter->prepareMessage(*_construct, "type", 9, "message");
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT(_construct->getXMLData() ==
                   "<Message type=\"type\" code=\"9\">message</Message>");
  }

  void testPrepareOBFeeWithoutItin()
  {
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _feeInfo->noDec() = 1;
    _feeInfo->cur() = NUC;
    _paxTypePayment->currency() = NUC;
    _mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin));
    CPPUNIT_ASSERT(_construct->getXMLData().find("<TicketingFee") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("feeAmount=\"100.0\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("iataIndicators=\"   \"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("noChargeInd=\" \"") != std::string::npos);
  }

  void testPrepareOBFeeWithItin()
  {
    _paxTypePayment->amount() = 199.43;
    _paxTypePayment->currency() = USD;
    _feeInfo->noDec() = 2;
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin));
  }

  void testPrepareOBFeeWhenFeeAmmntLTZero()
  {
    _feeInfo->feeAmount() = -1;
    _feeInfo->noDec() = 2;
    _paxTypePayment->currency() = NUC;
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin));
    CPPUNIT_ASSERT(_construct->getXMLData().find("feeAmount=\"-1.00\"") != std::string::npos);
  }

  void testPrepareOBFeeWhenFeePcntGTZero()
  {
    _feeInfo->feePercent() = 100;
    _feeInfo->noDec() = 2;
    _feeInfo->cur() = NUC;
    _paxTypePayment->currency() = NUC;
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin));
    CPPUNIT_ASSERT(_construct->getXMLData().find("serviceFeePercentage=\"100\"") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("maxAmount=\"0\"") != std::string::npos);
  }

  void testPrepareOBFeeNullPointerToPTPData()
  {
    _request->paxTypePaymentPerItin()[_itin] = 0;
    CPPUNIT_ASSERT_THROW(_mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin));
                         , std::runtime_error);
  }

  void testPrepareOBFeeWhenChargeAmountNonZero()
  {
    const DateTime tktDate = DateTime(2051, 1, 23, 1, 1, 0);
    _request->ticketingDT() = tktDate;

    _feeInfo->feePercent() = 2.0;
    _feeInfo->noDec() = 2;
    _feeInfo->cur() = NUC;
    _paxTypePayment->currency() = NUC;
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _mock->prepareOBFee(*_pricingTrx, *_construct, _feeInfo, &(*_itin), 1500.00);
    std::cout<<_construct->getXMLData();
    CPPUNIT_ASSERT(_construct->getXMLData().find("feeAmount=\"30.00\"") != std::string::npos);
  }

  void testGetCurrencyCodeModified()
  {
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    CurrencyCode curr = "ZWR";
    curr = _mock->getCurrencyCode(*_pricingTrx, *_itin);
    CPPUNIT_ASSERT(curr != "ZWR");
  }

  void testGetCurrencyCodeSetCorrectly()
  {
    _request->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    CurrencyCode curr = "ZWR";
    _paxTypePayment->currency() = "EUR";
    curr = _mock->getCurrencyCode(*_pricingTrx, *_itin);
    CPPUNIT_ASSERT(curr == "EUR");
  }

  void testPrepareHostPortInfo()
  {
    _mock->prepareHostPortInfo(*_pricingTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT(_construct->getXMLData().find("type=\"Diagnostic\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("code=\"3\"") != std::string::npos);

    CPPUNIT_ASSERT(_construct->getXMLData().find("HOSTNAME/PORT") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("BASELINE: MANUALBUILD") != std::string::npos ||
                   _construct->getXMLData().find("BASELINE: SCONSBUILD") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("DATABASE") != std::string::npos);
  }

  void testProcessTktFeesSolutionPerPaxTypeMaxFees()
  {
    _farePath->maximumObFee() = _feeInfo;
    _feeInfo->feeAmount() = 100;
    _feeInfo->noDec() = 2;
    _tktFeesReq->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _fop->fopBinNumber() = "123456";
    _paxTypePayment->ppiV().push_back(_ppiV);
    _feeInfo->cur() = NUC;
    _paxTypePayment->currency() = NUC;
    _mock->processTktFeesSolutionPerPaxType(*_tktFeesPricingTrx, _itin, *_construct, *_farePath);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT(_construct->getXMLData().find("<PassengerOBFees") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("totalPriceAmount=\"10\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("paymentCurrency=") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<TicketingFee") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("feeAmount=\"100.00\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("iataIndicators=\"   \"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("noChargeInd=\" \"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("showNoOBFeeInd=\"true\"") != std::string::npos);

    CPPUNIT_ASSERT(_construct->getXMLData().find("</PassengerOBFees>") != std::string::npos);
  }

  void testProcessTktFeesSolutionPerPaxTypeMinFees()
  {
    _farePath->maximumObFee() = _feeInfo;
    _feeInfo->feeAmount() = 100;
    _tktFeesReq->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    _mock->processTktFeesSolutionPerPaxType(*_tktFeesPricingTrx, _itin, *_construct, *_farePath);
    CPPUNIT_ASSERT(_construct->getXMLData().length() == 0);
  }

  void testClearAllFeesEmpty()
  {
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    _mock->clearAllFees(*_tktFeesPricingTrx);
    CPPUNIT_ASSERT(_farePath->collectedTktOBFees().empty());
  }

  void testCheckForZeroMaximumFalseWhenNonZero()
  {
    _feeInfo->feeAmount() = 100;
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    TicketingFeesInfo* feeInfo1 = _memH.insert(new TicketingFeesInfo);
    feeInfo1->feeAmount() = 0;
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    CPPUNIT_ASSERT(!_mock->checkForZeroMaximum(*_tktFeesPricingTrx, *_farePath));
  }

  void testCheckForZeroMaximumFalseWhenNonZeroPercent()
  {
    _feeInfo->feeAmount() = 0;
    _feeInfo->feePercent() = 7.5;
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    TicketingFeesInfo* feeInfo1 = _memH.insert(new TicketingFeesInfo);
    feeInfo1->feeAmount() = 0;
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    CPPUNIT_ASSERT(!_mock->checkForZeroMaximum(*_tktFeesPricingTrx, *_farePath));
  }

  void testCheckForZeroMaximumTrueWhenZeroAndBin()
  {
    _feeInfo->feeAmount() = 0;
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    TicketingFeesInfo* feeInfo1 = _memH.insert(new TicketingFeesInfo);
    feeInfo1->feeAmount() = 0;
    feeInfo1->fopBinNumber() = "123456";
    _farePath->collectedTktOBFees().push_back(feeInfo1);
    CPPUNIT_ASSERT(_mock->checkForZeroMaximum(*_tktFeesPricingTrx, *_farePath));
    CPPUNIT_ASSERT(_farePath->collectedTktOBFees().empty());
  }

  void testCheckForZeroMaximumTrueWhenZeroAndBinEmpty()
  {
    _feeInfo->feeAmount() = 0;
    _feeInfo->fopBinNumber() = "123456";
    _farePath->collectedTktOBFees().push_back(_feeInfo);
    TicketingFeesInfo* feeInfo1 = _memH.insert(new TicketingFeesInfo);
    feeInfo1->feeAmount() = 0;
    feeInfo1->fopBinNumber() = "";
    _farePath->collectedTktOBFees().push_back(feeInfo1);
    CPPUNIT_ASSERT(_mock->checkForZeroMaximum(*_tktFeesPricingTrx, *_farePath));
    CPPUNIT_ASSERT(!_farePath->collectedTktOBFees().empty());
    CPPUNIT_ASSERT(_farePath->collectedTktOBFees().size() == 1);
    CPPUNIT_ASSERT(_farePath->collectedTktOBFees().back()->fopBinNumber().empty());
  }

  void testGetChargeAmount()
  {
    MoneyAmount testAmt = 10.00;
    _fop->chargeAmount() = testAmt;
    _paxTypePayment->ppiV().push_back(_ppiV);
    CPPUNIT_ASSERT(_mock->getChargeAmount(_paxTypePayment, testAmt));
    CPPUNIT_ASSERT(_fop->chargeAmount() == testAmt);
  }

  void testGetChargeAmountWhenAmtNotInRequest()
  {
    MoneyAmount testAmt = 20.00;
    _paxTypePayment->amount() = testAmt;
    _fop->chargeAmount() = 0.0;
    _fop->chargeAmountInRequest() = false;
    _paxTypePayment->ppiV().push_back(_ppiV);
    CPPUNIT_ASSERT(_mock->getChargeAmount(_paxTypePayment, testAmt));
  }

  void testGetPaymentCurrencyWhenPtpNotFound()
  {
    _request->paxTypePaymentPerItin()[_itin] = 0;
    CPPUNIT_ASSERT_THROW(_mock->getPaymentCurrency(*_pricingTrx, _itin);, std::runtime_error);
  }

  void testGetPaymentCurrencyWhenCurrencyFound()
  {
    CurrencyCode ccTest = "GBP";
    _paxTypePayment->currency() = ccTest;
    _tktFeesReq->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
    CurrencyCode cc = _mock->getPaymentCurrency(*_tktFeesPricingTrx, _itin);
    CPPUNIT_ASSERT(cc == ccTest);
  }

  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _construct = _memH.insert(new XMLConstruct);
    _itin = _memH.insert(new Itin);
    _passID = _memH.insert(new TktFeesRequest::PassengerIdentity);
    _ppiV = _memH.insert(new TktFeesRequest::PassengerPaymentInfo);
    _fop = _memH.insert(new TktFeesRequest::FormOfPayment);
    _ppiV->fopVector().push_back(_fop);
    _farePath = _memH.insert(new FarePath);
    _itin->farePath().push_back(_farePath);
    _tktFeesReq = _memH.insert(new TktFeesRequest);
    _tktFeesPricingTrx = _memH.insert(new TktFeesPricingTrx);
    _tktFeesPricingTrx->setRequest(_tktFeesReq);
    _tktFeesPricingTrx->itin().push_back(_itin);
    _feeInfo = _memH.insert(new TicketingFeesInfo);
    _formatter = _memH.insert(new STLTktFeesPricingResponseFormatterMock);
    _request = _memH.insert(new TktFeesRequest);
    _pricingTrx = _memH.insert(new PricingTrx);
    _pricingTrx->setRequest(_request);
    _pricingTrx->ticketingDate();
    _mock = _memH.insert(new STLTktFeesPricingResponseFormatterMock);
    _paxTypePayment = _memH.insert(new TktFeesRequest::PaxTypePayment);
    _paxTypePayment->amount() = 10.00;
    _paxType = _memH.create<PaxType>();
    _paxTypePayment->paxType() = _paxType;
    _paxTypePayment->currency() = 7;
    _feeInfo->feeAmount() = 100;
    _tktFeesReq->paxTypePaymentPerItin()[_itin] = _paxTypePayment;
  }

  void tearDown() { _memH.clear(); }

private:
  mutable TestMemHandle _memH;
  XMLConstruct* _construct;
  STLTktFeesPricingResponseFormatterMock* _formatter;
  Itin* _itin;
  TicketingFeesInfo* _feeInfo;
  PricingTrx* _pricingTrx;
  TktFeesRequest* _request;
  STLTktFeesPricingResponseFormatterMock* _mock;
  TktFeesRequest::PaxTypePayment* _paxTypePayment;
  TktFeesPricingTrx* _tktFeesPricingTrx;
  PaxType* _paxType;
  FarePath* _farePath;
  TktFeesRequest* _tktFeesReq;
  TktFeesRequest::PassengerPaymentInfo* _ppiV;
  TktFeesRequest::PassengerIdentity* _passID;
  TktFeesRequest::FormOfPayment* _fop;
};
CPPUNIT_TEST_SUITE_REGISTRATION(STLTktFeesPricingResponseFormatterTest);
} // tse
