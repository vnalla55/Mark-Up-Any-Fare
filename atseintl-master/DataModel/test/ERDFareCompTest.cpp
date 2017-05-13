
//----------------------------------------------------------------------------
//
//      File: ERDRequestProcessorTest.cpp
//      Description: Unit test for ERDFareComp class
//      Created: March 12, 2009
//      Authors: Slawek Machowicz
//
//  Copyright Sabre 2008
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

#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ERDFareComp.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }
};
}
class ERDFareCompTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ERDFareCompTest);
  CPPUNIT_TEST(testSuccess);
  CPPUNIT_TEST(testFailNullOptions);
  CPPUNIT_TEST(testFailNullRequest);
  CPPUNIT_TEST(testFailIncorrectOrig);
  CPPUNIT_TEST(testFailIncorrectDest);
  CPPUNIT_TEST(testCat25);
  CPPUNIT_TEST(testCat35);
  CPPUNIT_TEST(testDiscounted);
  CPPUNIT_TEST(testNet);
  CPPUNIT_TEST(testNetWithUniqueFBC);
  CPPUNIT_TEST(testNetFail);
  CPPUNIT_TEST(testConstructed);
  CPPUNIT_TEST(testConstructedOrig);
  CPPUNIT_TEST(testConstructedDest);
  CPPUNIT_TEST(testConstructedFail);
  CPPUNIT_TEST(testIsRtw);
  CPPUNIT_TEST(testIsNotRtw);
  CPPUNIT_TEST(testMatchUniqueFareBasis);
  CPPUNIT_TEST(testConditionallyMatchUniqueFareBasis);
  CPPUNIT_TEST(testMatchCat35FareBasis);
  CPPUNIT_TEST(testConditionallyMatchCat35FareBasis);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fareComp = _memHandle.create<ERDFareComp>();
    _fareComp->departureAirport() = "LHR";
    _fareComp->arrivalAirport() = "CDG";
    _fareComp->departureCity() = "LON";
    _fareComp->arrivalCity() = "PAR";
    _fareComp->fareBasis() = "BASIS";

    _dataHandle.get(_trx);
    _trx->dataHandle().get(_request);
    _trx->dataHandle().get(_options);
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _time = DateTime::fromMinutes(60);

    AirSeg* airSeg;
    _trx->dataHandle().get(airSeg);
    airSeg->departureDT() = _time;
    _trx->travelSeg().push_back(airSeg);
  }

  void tearDown() { _memHandle.clear(); }

  void testSuccess()
  {
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT_EQUAL(FareClassCode("BASIS"), _request->fareBasisCode());
  }

  void testFailNullOptions()
  {
    _trx->setOptions(NULL);
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT_EQUAL(FareClassCode(""), _request->fareBasisCode());
  }

  void testFailNullRequest()
  {
    _trx->setRequest(NULL);
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT_EQUAL(FareClassCode(""), _request->fareBasisCode());
  }

  void testFailIncorrectOrig()
  {
    MyDataHandle mdh;
    _fareComp->arrivalAirport() = "";
    CPPUNIT_ASSERT_THROW(_fareComp->select(*_trx), NonFatalErrorResponseException);
  }

  void testFailIncorrectDest()
  {
    MyDataHandle mdh;
    _fareComp->departureAirport() = "";
    CPPUNIT_ASSERT_THROW(_fareComp->select(*_trx), NonFatalErrorResponseException);
  }

  void testCat25()
  {
    _fareComp->cat25Values().isNonPublishedFare() = true;
    _fareComp->cat25Values().vendorCode() = "A";
    _fareComp->cat25Values().itemNo() = 1;
    _fareComp->cat25Values().createDate() = _time;
    _fareComp->cat25Values().directionality() = "FR";

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->cat25Values().isNonPublishedFare());
    CPPUNIT_ASSERT_EQUAL(VendorCode("A"), _options->cat25Values().vendorCode());
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, _options->cat25Values().itemNo());
    CPPUNIT_ASSERT_EQUAL(_time, _options->cat25Values().createDate());
    CPPUNIT_ASSERT(!_options->swappedDirectionality());
  }

  void testCat35()
  {
    _fareComp->cat35Values().isNonPublishedFare() = true;
    _fareComp->cat35Values().vendorCode() = "B";
    _fareComp->cat35Values().itemNo() = 2;
    _fareComp->cat35Values().createDate() = _time;
    _fareComp->cat35Values().directionality() = "TO";

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->cat35Values().isNonPublishedFare());
    CPPUNIT_ASSERT_EQUAL(VendorCode("B"), _options->cat35Values().vendorCode());
    CPPUNIT_ASSERT_EQUAL((unsigned int)2, _options->cat35Values().itemNo());
    CPPUNIT_ASSERT_EQUAL(_time, _options->cat35Values().createDate());
    CPPUNIT_ASSERT(_options->swappedDirectionality());
  }

  void testDiscounted()
  {
    _fareComp->discountedValues().isNonPublishedFare() = true;
    _fareComp->discountedValues().vendorCode() = "C";
    _fareComp->discountedValues().itemNo() = 3;
    _fareComp->discountedValues().createDate() = _time;
    _fareComp->discountedValues().directionality() = "";

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->discountedValues().isNonPublishedFare());
    CPPUNIT_ASSERT_EQUAL(VendorCode("C"), _options->discountedValues().vendorCode());
    CPPUNIT_ASSERT_EQUAL((unsigned int)3, _options->discountedValues().itemNo());
    CPPUNIT_ASSERT_EQUAL(_time, _options->discountedValues().createDate());
    CPPUNIT_ASSERT(_options->swappedDirectionality() == false);
  }

  void testNet()
  {
    _request->inclusionCode() = "";
    _fareComp->cat35Type() = NET_QUALIFIER;
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT_EQUAL(FD_NET, _request->inclusionCode());
    CPPUNIT_ASSERT_EQUAL(FD_NET, _request->requestedInclusionCode());
  }

  void testNetWithUniqueFBC()
  {
    _request->inclusionCode() = "";
    _fareComp->cat35Type() = NET_QUALIFIER_WITH_UNIQUE_FBC;
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT_EQUAL(FD_NET, _request->inclusionCode());
    CPPUNIT_ASSERT_EQUAL(FD_NET, _request->requestedInclusionCode());
  }

  void testNetFail()
  {
    _request->inclusionCode() = "";
    _fareComp->cat35Type() = "";
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT(_request->inclusionCode() != FD_NET);
    CPPUNIT_ASSERT(_request->requestedInclusionCode() != FD_NET);
  }

  void testConstructed()
  {
    _fareComp->constructedAttributes().isConstructedFare() = true;
    _fareComp->constructedAttributes().gateway1() = "AAA";
    _fareComp->constructedAttributes().gateway2() = "BBB";
    _fareComp->constructedAttributes().specifiedFareAmount() = 100;
    _fareComp->constructedAttributes().constructedNucAmount() = 200;

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->constructedAttributes().isConstructedFare());
    CPPUNIT_ASSERT_EQUAL(LocCode("AAA"), _options->constructedAttributes().gateway1());
    CPPUNIT_ASSERT_EQUAL(LocCode("BBB"), _options->constructedAttributes().gateway2());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, _options->constructedAttributes().specifiedFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        200, _options->constructedAttributes().constructedNucAmount(), 0.01);
    CPPUNIT_ASSERT(!_options->destAttributes().isAddOn());
    CPPUNIT_ASSERT(!_options->destAttributes().isAddOn());
  }

  void testConstructedOrig()
  {
    _fareComp->constructedAttributes().isConstructedFare() = true;
    _fareComp->origAttributes().isAddOn() = true;
    _fareComp->origAttributes().addonAmount() = 10;

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->constructedAttributes().isConstructedFare());
    CPPUNIT_ASSERT(_options->origAttributes().isAddOn());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10, _options->origAttributes().addonAmount(), 0.01);
  }

  void testConstructedDest()
  {
    _fareComp->constructedAttributes().isConstructedFare() = true;
    _fareComp->destAttributes().isAddOn() = true;
    _fareComp->destAttributes().addonAmount() = 20;

    _fareComp->select(*_trx);

    CPPUNIT_ASSERT(_options->constructedAttributes().isConstructedFare());
    CPPUNIT_ASSERT(_options->destAttributes().isAddOn());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20, _options->destAttributes().addonAmount(), 0.01);
  }

  void testConstructedFail()
  {
    _fareComp->constructedAttributes().isConstructedFare() = false;
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT(!_options->constructedAttributes().isConstructedFare());
  }

  void testIsRtw()
  {
    _fareComp->arrivalCity() = "LON";
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT(_options->isRtw());
  }

  void testIsNotRtw()
  {
    _fareComp->select(*_trx);
    CPPUNIT_ASSERT(!_options->isRtw());
  }

  void testMatchUniqueFareBasis()
  {
    _fareComp->uniqueFareBasis() = "ABCDEF";
    _fareComp->fareClassLength() = 3;

    ERDFareComp::MatchFareBasis match1("ABCDEF");
    ERDFareComp::MatchFareBasis match2("ABC");
    ERDFareComp::MatchFareBasis match3("ABCD");

    CPPUNIT_ASSERT(match1.operator()(_fareComp));
    CPPUNIT_ASSERT(!match2.operator()(_fareComp));
    CPPUNIT_ASSERT(!match3.operator()(_fareComp));
  }

  void testConditionallyMatchUniqueFareBasis()
  {
    _fareComp->uniqueFareBasis() = "ABCDEF";
    _fareComp->fareClassLength() = 3;

    ERDFareComp::MatchConditionalFareBasis match1("ABCDEF");
    ERDFareComp::MatchConditionalFareBasis match2("ABC");
    ERDFareComp::MatchConditionalFareBasis match3("ABCD");

    CPPUNIT_ASSERT(match1.operator()(_fareComp));
    CPPUNIT_ASSERT(match2.operator()(_fareComp));
    CPPUNIT_ASSERT(!match3.operator()(_fareComp));
  }

  void testMatchCat35FareBasis()
  {
    _fareComp->cat35FareBasis() = "ABCDEF";

    ERDFareComp::MatchFareBasis match1("ABCDEF");
    ERDFareComp::MatchFareBasis match2("ABC");

    CPPUNIT_ASSERT(match1.operator()(_fareComp));
    CPPUNIT_ASSERT(!match2.operator()(_fareComp));
  }

  void testConditionallyMatchCat35FareBasis()
  {
    _fareComp->cat35FareBasis() = "ABCDEF";

    ERDFareComp::MatchConditionalFareBasis match1("ABCDEF");
    ERDFareComp::MatchConditionalFareBasis match2("ABC");

    CPPUNIT_ASSERT(match1.operator()(_fareComp));
    CPPUNIT_ASSERT(!match2.operator()(_fareComp));
  }

private:
  ERDFareComp* _fareComp;
  DataHandle _dataHandle;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  DateTime _time;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ERDFareCompTest);
}
