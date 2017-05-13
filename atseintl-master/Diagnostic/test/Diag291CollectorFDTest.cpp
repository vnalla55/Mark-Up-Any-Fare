//-----------------------------------------------------------------------------
//
//  File:     Diag291CollectorFDTest.cpp
//
//  Author :  Slawek Machowicz
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
#include <time.h>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag291CollectorFD.h"
#include "Diagnostic/Diagnostic.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareCalcConfig.h"

using namespace std;
namespace tse
{
class Diag291CollectorFDTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag291CollectorFDTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintFooter);
  CPPUNIT_TEST(testPrintFare);
  CPPUNIT_TEST(testPrintHeaderNotActive);
  CPPUNIT_TEST(testPrintFooterNotActive);
  CPPUNIT_TEST(testPrintFareNotActive);
  CPPUNIT_TEST(testPrintFareMatched);
  CPPUNIT_TEST(testPrintFareNotSimilar);
  CPPUNIT_TEST_SUITE_END();

private:
  DataHandle _dataHandle;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  Diag291CollectorFD* _collector;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _diagroot = _memHandle(new Diagnostic(Diagnostic291));
      _diagroot->activate();
      _collector = _memHandle(new Diag291CollectorFD(*_diagroot));
      _collector->enable(Diagnostic291);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    _dataHandle.get(_trx);
    _trx->dataHandle().get(_request);
    _trx->dataHandle().get(_options);
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _options->constructedAttributes().isConstructedFare() = true;

    _collector->init(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testPrintHeader()
  {
    std::string response = "**************************************************************\n"
                           "RULE DISPLAY FARE SELECTION DIAGNOSTIC 291\n"
                           "**************************************************************\n"
                           "REQUEST INFORMATION:\n"
                           "FARE CLASS               AAA\n"
                           "LINK NUMBER              1\n"
                           "SEQUENCE NUMBER          2\n"
                           "DATE/TIME                N/A 00:00:00\n"
                           "FAREBASIS CODE           BBB\n"
                           "FARE AMOUNT              100\n"
                           "FARE TYPE                FT\n"
                           "REQUESTED DISCOUNT       N\n"
                           "REQUESTED CAT25          N\n"
                           "REQUESTED NEG            N\n"
                           "SOURCE PCC               N/A\n"
                           "REQUESTED CONSTRUCTED    Y\n"
                           "CAT 35 TYPE              A\n"
                           "CARRIER                  LO\n"
                           "CURRENCY                 PLN\n"
                           "VENDOR                   ATP\n"
                           "TARIFF                   3\n"
                           "RULE NUMBER              XXXX\n"
                           "BOOKING CODE             Y\n"
                           "FARE ORIGIN              KRK\n"
                           "FARE DESTINATION         KTW\n"
                           "GATEWAY 1                POZ\n"
                           "GATEWAY 2                WAW\n"
                           "ORIGIN ADD ON            A*****\n"
                           "DEST ADD ON              B*****\n"
                           "CONSTRUCTION TYPE        3\n"
                           "SPECIFIED AMOUNT         50\n"
                           "CONSTRUCTED NUC AMOUNT   20\n";

    _options->fareClass() = "AAA";
    _options->linkNumber() = 1;
    _options->sequenceNumber() = 2;
    _options->createTime() = "00:00:00";
    _request->fareBasisCode() = "BBB";
    _request->calcFareAmount() = 100;
    _options->fareType() = "FT";
    _options->FDcat35Type() = 'A';
    _options->carrierCode() = "LO";
    _options->baseFareCurrency() = "PLN";
    _options->vendorCode() = "ATP";
    _options->fareTariff() = 3;
    _options->ruleNumber() = "XXXX";
    _request->bookingCode() = "Y";
    _request->fareOrigin() = "KRK";
    _request->fareDestination() = "KTW";
    _options->constructedAttributes().gateway1() = "POZ";
    _options->constructedAttributes().gateway2() = "WAW";
    _options->origAttributes().addonFareClass() = "A*****";
    _options->destAttributes().addonFareClass() = "B*****";
    _options->constructedAttributes().constructionType() = ConstructedFareInfo::DOUBLE_ENDED;
    _options->constructedAttributes().specifiedFareAmount() = 50;
    _options->constructedAttributes().constructedNucAmount() = 20;

    _collector->printHeader();

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFooter()
  {
    std::string response = "**************************************************************\n"
                           "RULE DISPLAY MATCHED 10 FARES OF TOTAL 20\n"
                           "**************************************************************\n";

    _collector->printFooter(10, 20);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFare()
  {
    std::string response = "**************************************************************\n"
                           "REQUESTED DISCOUNT MISMATCH\n"
                           "**************************************************************\n"
                           "PROCESSED FARE INFORMATION:\n"
                           "FARE CLASS               AAA\n"
                           "LINK NUMBER              1\n"
                           "SEQUENCE NUMBER          2\n"
                           "DATE/TIME                N/A 00:00:00\n"
                           "FAREBASIS CODE           AAA\n"
                           "FARE AMOUNT              100\n"
                           "FARE TYPE                FT\n"
                           "REQUESTED DISCOUNT       N\n"
                           "REQUESTED CAT25          N\n"
                           "REQUESTED NEG            N\n"
                           "SOURCE PCC               \n"
                           "REQUESTED CONSTRUCTED    Y\n"
                           "CAT 35 TYPE              A\n"
                           "CARRIER                  LO\n"
                           "CURRENCY                 PLN\n"
                           "VENDOR                   ATP\n"
                           "TARIFF                   3\n"
                           "RULE NUMBER              XXXX\n"
                           "BOOKING CODE             Y\n"
                           "FARE ORIGIN              KRK\n"
                           "FARE DESTINATION         KTW\n"
                           "GATEWAY 1                POZ\n"
                           "GATEWAY 2                WAW\n"
                           "ORIGIN ADD ON            A*****\n"
                           "DEST ADD ON              B*****\n"
                           "CONSTRUCTION TYPE        3\n"
                           "SPECIFIED AMOUNT         50\n"
                           "CONSTRUCTED NUC AMOUNT   20\n";
    MockPaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    ConstructedFareInfo constr;
    FareClassAppInfo fca;
    ptf.fareClassAppInfo() = &fca;
    fare.setFareInfo(&fareInfo);
    fare.constructedFareInfo() = &constr;

    fareInfo.fareClass() = "AAA";
    fareInfo.linkNumber() = 1;
    fareInfo.sequenceNumber() = 2;
    fareInfo.carrier() = "LO";
    fareInfo.vendor() = "ATP";
    fareInfo.ruleNumber() = "XXXX";
    fareInfo.fareTariff() = 3;
    fareInfo.currency() = "PLN";
    fareInfo.fareAmount() = 100;
    fareInfo.directionality() = FROM;
    fareInfo.market1() = "KRK";
    fareInfo.market2() = "KTW";

    ptf.setFare(&fare);

    FareCalcConfig* config = new FareCalcConfig();
    _trx->fareCalcConfig() = config;

    ptf.bookingCode() = "Y";
    ptf.fareDisplayCat35Type() = 'A';

    constr.gateway1() = "POZ";
    constr.gateway2() = "WAW";
    constr.origAddonFareClass() = "A*****";
    constr.destAddonFareClass() = "B*****";
    constr.constructionType() = ConstructedFareInfo::DOUBLE_ENDED;
    constr.specifiedFareAmount() = 50;
    constr.constructedNucAmount() = 20;

    fca._fareType = "FT";

    _collector->printFare(ptf, Diag291CollectorFD::FAIL_RD_DISCOUNTED);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());

    delete config;
  }

  void testPrintHeaderNotActive()
  {
    std::string response = "";

    _collector->disable(Diagnostic291);
    _collector->printHeader();

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFooterNotActive()
  {
    std::string response = "";

    _collector->disable(Diagnostic291);
    _collector->printFooter(10, 20);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFareNotActive()
  {
    std::string response = "";
    MockPaxTypeFare ptf;

    _collector->disable(Diagnostic291);
    _collector->printFare(ptf, Diag291CollectorFD::FAIL_RD_DISCOUNTED);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFareMatched()
  {
    std::string response = "**************************************************************\n"
                           "FARE MATCHED\n"
                           "**************************************************************\n";
    MockPaxTypeFare ptf;

    _collector->printFare(ptf, Diag291CollectorFD::FAIL_RD_MATCHED);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintFareNotSimilar()
  {
    std::string response = "";
    MockPaxTypeFare ptf;

    _collector->printFare(ptf, Diag291CollectorFD::FAIL_RD_FARECLASS);

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

private:
  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare() : PaxTypeFare() {}

  protected:
    std::string createFareBasisCodeFD(FareDisplayTrx& trx) const { return fareClass().c_str(); }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag291CollectorFDTest);
}
