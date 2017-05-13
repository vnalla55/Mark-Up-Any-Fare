//-------------------------------------------------------------------
//
//  File:        FlightApplicationTest.cpp
//  Created:     09/03/10
//  Author:      Tomasz Baranski
//
//  Description: Test case created automatically
//
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "Rules/FlightApplication.h"
#include "Rules/FlightAppPredicates.h"
#include "Rules/MockTable986.h"
#include "Rules/RuleItem.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FlightApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlightApplicationTest);
  CPPUNIT_TEST(appIndicatorExamle01);
  CPPUNIT_TEST(appIndicatorExamle02);
  CPPUNIT_TEST(appIndicatorExamle03);
  CPPUNIT_TEST(appIndicatorExamle04);
  CPPUNIT_TEST(appIndicatorExamle05);
  CPPUNIT_TEST(appIndicatorExamle06);
  CPPUNIT_TEST(appIndicatorExamle07);
  CPPUNIT_TEST(appIndicatorExamle08);
  CPPUNIT_TEST(appIndicatorExamle09);

  CPPUNIT_TEST(testConditinoalGeoNegFlt);

  CPPUNIT_TEST(dayOfWeekTestExample01_positive);
  CPPUNIT_TEST(dayOfWeekTestExample02_negative);

  CPPUNIT_TEST(flightTableExample01_positive_single_any);
  CPPUNIT_TEST(flightTableExample02_positive_single_range);
  CPPUNIT_TEST(flightTableExample03_positive_single_single);
  CPPUNIT_TEST(flightTableExample04_positive_two_riv1);
  CPPUNIT_TEST(flightTableExample05_positive_two_riv2);
  CPPUNIT_TEST(flightTableExample06_positive_twomix_riv1);
  CPPUNIT_TEST(flightTableExample08_positive_flttbl_riv1);
  CPPUNIT_TEST(flightTableExample09_positive_flttbl_riv2);
  CPPUNIT_TEST(flightTableExample10_positive_twoflight_riv1);
  CPPUNIT_TEST(flightTableExample11_positive_twoflight_riv2);
  CPPUNIT_TEST(flightTableExample12_positive_twoflight_riv12);
  CPPUNIT_TEST(flightTableExample13_negative_single_any);
  CPPUNIT_TEST(flightTableExample14_negative_single_range);
  CPPUNIT_TEST(flightTableExample15_negative_two_riv1);
  CPPUNIT_TEST(flightTableExample16_negative_two_riv2);
  CPPUNIT_TEST(flightTableExample17_negative_two_riv1_segboth);
  CPPUNIT_TEST(flightTableExample18_positive_surface_riv1);
  CPPUNIT_TEST(flightTableExample19_positive_surface_riv2);
  CPPUNIT_TEST(flightTableExample20_positive_connecting_fltTbl);
  CPPUNIT_TEST(flightTableExample21_negative_connecting_fltTbl);
  CPPUNIT_TEST(flightTableExample22_connecting_connecting_fltTbl);
  CPPUNIT_TEST(flightTableExample23_neg_connecting_connecting_fltTbl);

  CPPUNIT_TEST(flightTableExample01_RTW);
  CPPUNIT_TEST(flightTableExample02_RTW);
  CPPUNIT_TEST(flightTableExample03_RTW);
  CPPUNIT_TEST(flightTableExample04_RTW);
  CPPUNIT_TEST(flightTableExample05_RTW);
  CPPUNIT_TEST(flightTableExample06_RTW);
  CPPUNIT_TEST(flightTableExample07_RTW);

  CPPUNIT_TEST(mustBtAndGeoNegFltExample01);
  CPPUNIT_TEST(mustBtAndGeoNegViaPossitiveFltExample01);

  CPPUNIT_TEST(mustFromToExample01);
  CPPUNIT_TEST(mustFromToViaExample01);
  CPPUNIT_TEST(mustFromToViaHiddenNExample01);
  CPPUNIT_TEST(mustNotMustNotInterline);

  CPPUNIT_TEST(negBtAndGeoPossitiveFltExample01);
  CPPUNIT_TEST(negBtAndGeoPosViaPosFltExample01);
  CPPUNIT_TEST(negBtAndGeoPosViaPosFltExample02);

  CPPUNIT_TEST(negGeoPossitiveFltExample01);

  CPPUNIT_TEST(openSegOpenseg_mustnot0);
  CPPUNIT_TEST(openSegOpenseg_mustnot1);
  CPPUNIT_TEST(openSegOpenseg_must);
  CPPUNIT_TEST(openSegOpenseg_mustbtw);
  CPPUNIT_TEST(openSegOpenseg_mustnotbtw);
  CPPUNIT_TEST(openSegOpenseg_flttable_must);
  CPPUNIT_TEST(openSegOpenseg_flttable_mustnot);
  CPPUNIT_TEST(openSegOpenseg_flttbl_rlt1);

  CPPUNIT_TEST(relationsExample01);
  CPPUNIT_TEST(relationsExample02);
  CPPUNIT_TEST(relationsExample03);
  CPPUNIT_TEST(relationsExample04);
  CPPUNIT_TEST(relationsExample05);
  CPPUNIT_TEST(relationsExample06);
  CPPUNIT_TEST(relationsExample07);
  CPPUNIT_TEST(relationsExample08);

  CPPUNIT_TEST(test40Example01);
  CPPUNIT_TEST(test40Example02);

  CPPUNIT_TEST(testQualifierOpenseg_ifnot);
  CPPUNIT_TEST(testQualifierOpenseg_if);

  CPPUNIT_TEST(typeOfFlightExample01_nonstop);
  CPPUNIT_TEST(typeOfFlightExample02_direct);
  CPPUNIT_TEST(typeOfFlightExample03_multistop);
  CPPUNIT_TEST(typeOfFlightExample04_onestop);
  CPPUNIT_TEST(typeOfFlightExample05_online);
  CPPUNIT_TEST(typeOfFlightExample06_interline);
  CPPUNIT_TEST(typeOfFlightExample07_sameflight);
  CPPUNIT_TEST(typeOfFlightExample08_pos_multiple_pos_flight);
  CPPUNIT_TEST(typeOfFlightExample09_neg_multiple_pos_flight);
  CPPUNIT_TEST(typeOfFlightExample10_pos_mutliple_neg_flight);

  CPPUNIT_TEST(viaInterlineMustViaJFKInterline);
  CPPUNIT_TEST(viaJFKExample01);
  CPPUNIT_TEST(viaJFKHiddenNExample01);
  CPPUNIT_TEST(viaJFKHiddenYExample01);
  CPPUNIT_TEST(viaOnlineMustViaJFK_DLOnline);

  CPPUNIT_TEST(openSegOpenseg_must_pass_withFlight);
  CPPUNIT_TEST(openSegOpenseg_mustbtw_pass_withFlight);
  CPPUNIT_TEST(openSegOpenseg_must_fail_withFlight);
  CPPUNIT_TEST(openSegOpenseg_mustbtw_fail_withFlight);

  CPPUNIT_TEST(openSegOpenseg_must_pass_EQPwithFlight_WQ);
  CPPUNIT_TEST(openSegOpenseg_must_fail_EQPwithFlight_WQ);

  CPPUNIT_TEST(testInOrOutBoundFailSkip);
  CPPUNIT_TEST(testInOrOutBoundSkipFail);
  CPPUNIT_TEST(testInOrOutBoundPassSkip);
  CPPUNIT_TEST(testInOrOutBoundSkipPass);
  CPPUNIT_TEST(testInOrOutBoundSkipSkip);
  CPPUNIT_TEST(testInOrOutBoundFailFail);
  CPPUNIT_TEST(testInOrOutBoundPassPass);
  CPPUNIT_TEST(testInOrOutBoundFailPass);
  CPPUNIT_TEST(testInOrOutBoundPassFail);

  CPPUNIT_TEST(testRTW1);
  CPPUNIT_TEST(testRTW2);
  CPPUNIT_TEST(testRTW3);
  CPPUNIT_TEST(testRTW4);
  CPPUNIT_TEST(testRTW5);
  CPPUNIT_TEST(testRTW6);
  CPPUNIT_TEST(testRTW7);
  CPPUNIT_TEST(testRTW8);
  CPPUNIT_TEST(testRTW9);
  CPPUNIT_TEST(testRTW10);
  CPPUNIT_TEST(testRTW11);

  CPPUNIT_TEST(testNegateReturnValue_PASS);
  CPPUNIT_TEST(testNegateReturnValue_FAIL);
  CPPUNIT_TEST(testNegateReturnValue_SOFTPASS);
  CPPUNIT_TEST(testNegateReturnValue_SKIP);
  CPPUNIT_TEST(testNegateReturnValue_STOP);
  CPPUNIT_TEST(openSegOpenseg_must_Pass_EQPwithFlight_WQ_APO40449);

  CPPUNIT_TEST_SUITE_END();

protected:
  MockTable986* _table986;
  TestMemHandle _memHandle;
  Fare* _fare;
  FareInfo* _fareInfo;
  FareMarket* _fareMarket;
  FlightApplication* _flightApplication;
  FlightAppRule* _flightAppRule;
  PaxTypeFare* _paxTypeFare;
  PricingUnit* _pricingUnit;
  PricingTrx* _trx;
  PricingOptions* _pricOptions;

  static const Indicator NOT_APPLY = ' ';
  static const Indicator OUTBOUND = '1';
  static const Indicator INBOUND = '2';
  static const Indicator IN_XOR_OUTBOUND = '3';
  static const Indicator IN_OR_OUTBOUND = '4';
  static const Indicator IN_AND_OUTBOUND = '5';

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<FlightApplicationDataHandle>(_memHandle);
  }

  void tearDown() { _memHandle.clear(); }

  void appIndicatorExamle01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "US";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle02()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_RANGE;
    record3.flt1() = 1;
    record3.carrier1() = "AA";
    record3.flt2() = 1000;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 1000;

    itin1.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 1000;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 3000;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle03()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "BA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "US";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void appIndicatorExamle04()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT_RANGE;
    record3.flt1() = 100;
    record3.carrier1() = "AA";
    record3.flt2() = 200;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 99;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("MAN", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle05()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.equipType() = "JET";
    record3.fltAppl() = FlightApplication::MUST;
    record3.equipAppl() = FlightApplication::VALID;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->equipmentType() = "SWM";

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->equipmentType() = "JET";

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->equipmentType() = "JET";

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->equipmentType() = "JET";

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "UA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Open;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "NW";

    const Loc* airSeg6Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->segmentType() = Open;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "NW";

    const Loc* airSeg7Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->segmentType() = Open;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "NW";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->segmentType() = Open;

    itin4.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "NW";

    const Loc* airSeg9Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->equipmentType() = "SWM";

    itin4.push_back(airSeg9);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle06()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.equipType() = "747";
    record3.fltAppl() = FlightApplication::MUST;
    record3.equipAppl() = FlightApplication::INVALID;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "UA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->equipmentType() = "727";

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->equipmentType() = "747";

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->equipmentType() = "777";

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->equipmentType() = "727";

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "UA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "UA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Open;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "NW";

    const Loc* airSeg6Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->segmentType() = Open;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "UA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->equipmentType() = "727";

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "UA";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->segmentType() = Open;

    itin4.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "UA";

    const Loc* airSeg9Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->equipmentType() = "727";

    itin4.push_back(airSeg9);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "UA";

    const Loc* airSeg10Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->segmentType() = Open;

    itin5.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "UA";

    const Loc* airSeg11Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("FAR", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->equipmentType() = "747";

    itin5.push_back(airSeg11);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle07()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    record3.fltDirect() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void appIndicatorExamle08()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.equipType() = "JET";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.equipAppl() = FlightApplication::VALID;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->equipmentType() = "JET";

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->equipmentType() = "JET";

    airSeg1->flightNumber() = 300;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->equipmentType() = "JET";

    airSeg2->flightNumber() = 301;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->equipmentType() = "JET";

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void appIndicatorExamle09()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.equipType() = "747";
    record3.fltAppl() = FlightApplication::MUST;
    record3.equipAppl() = FlightApplication::INVALID;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "UA";
    record3.fltDirect() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->equipmentType() = "727";

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->equipmentType() = "777";

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->equipmentType() = "777";

    airSeg2->flightNumber() = 101;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->equipmentType() = "737";

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void testConditinoalGeoNegFlt()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.viaInd() = '2';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(SKIP, result);
  }

  void dayOfWeekTestExample01_positive()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.dow() = "24";
    record3.flt1() = 200;
    record3.carrier1() = "LH";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "LH";

    const Loc* airSeg0Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    tm departureDTTimeairSeg0;
    bzero(&departureDTTimeairSeg0, sizeof(tm));
    strptime("08/04/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg0);
    airSeg0->departureDT() = mktime(&departureDTTimeairSeg0);

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "LH";

    const Loc* airSeg1Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    tm departureDTTimeairSeg1;
    bzero(&departureDTTimeairSeg1, sizeof(tm));
    strptime("04/06/04", "%m/%d/%y %H:%M", &departureDTTimeairSeg1);
    airSeg1->departureDT() = mktime(&departureDTTimeairSeg1);

    airSeg1->segmentType() = Open;

    itin1.push_back(airSeg1);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "LH";

    const Loc* airSeg2Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    tm departureDTTimeairSeg2;
    bzero(&departureDTTimeairSeg2, sizeof(tm));
    strptime("04/06/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg2);
    airSeg2->departureDT() = mktime(&departureDTTimeairSeg2);

    airSeg2->flightNumber() = 200;

    itin2.push_back(airSeg2);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "LH";

    const Loc* airSeg3Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    tm departureDTTimeairSeg3;
    bzero(&departureDTTimeairSeg3, sizeof(tm));
    strptime("04/07/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg3);
    airSeg3->departureDT() = mktime(&departureDTTimeairSeg3);

    airSeg3->flightNumber() = 400;

    itin3.push_back(airSeg3);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "LH";

    const Loc* airSeg4Orig = validator.getDH().getLoc("DUS", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    tm departureDTTimeairSeg4;
    bzero(&departureDTTimeairSeg4, sizeof(tm));
    strptime("04/06/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg4);
    airSeg4->departureDT() = mktime(&departureDTTimeairSeg4);

    airSeg4->flightNumber() = 200;

    itin4.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "LH";

    const Loc* airSeg5Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    tm departureDTTimeairSeg5;
    bzero(&departureDTTimeairSeg5, sizeof(tm));
    strptime("04/08/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg5);
    airSeg5->departureDT() = mktime(&departureDTTimeairSeg5);

    airSeg5->flightNumber() = 200;

    itin4.push_back(airSeg5);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void dayOfWeekTestExample02_negative()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.dow() = "23";
    record3.flt1() = 200;
    record3.carrier1() = "LH";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "LH";

    const Loc* airSeg0Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    tm departureDTTimeairSeg0;
    bzero(&departureDTTimeairSeg0, sizeof(tm));
    strptime("04/06/04", "%m/%d/%y %H:%M", &departureDTTimeairSeg0);
    airSeg0->departureDT() = mktime(&departureDTTimeairSeg0);

    airSeg0->flightNumber() = 111;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "LH";

    const Loc* airSeg1Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    tm departureDTTimeairSeg1;
    bzero(&departureDTTimeairSeg1, sizeof(tm));
    strptime("04/06/04", "%m/%d/%y %H:%M", &departureDTTimeairSeg1);
    airSeg1->departureDT() = mktime(&departureDTTimeairSeg1);

    airSeg1->segmentType() = Open;

    itin1.push_back(airSeg1);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "LH";

    const Loc* airSeg2Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    tm departureDTTimeairSeg2;
    bzero(&departureDTTimeairSeg2, sizeof(tm));
    strptime("04/03/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg2);
    airSeg2->departureDT() = mktime(&departureDTTimeairSeg2);

    airSeg2->flightNumber() = 200;

    itin2.push_back(airSeg2);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "LH";

    const Loc* airSeg3Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    tm departureDTTimeairSeg3;
    bzero(&departureDTTimeairSeg3, sizeof(tm));
    strptime("11/25/05 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg3);
    airSeg3->departureDT() = mktime(&departureDTTimeairSeg3);

    airSeg3->flightNumber() = 234;

    itin3.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "LH";

    const Loc* airSeg4Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    tm departureDTTimeairSeg4;
    bzero(&departureDTTimeairSeg4, sizeof(tm));
    strptime("11/25/05 03:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg4);
    airSeg4->departureDT() = mktime(&departureDTTimeairSeg4);

    airSeg4->flightNumber() = 123;

    itin3.push_back(airSeg4);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "LH";

    const Loc* airSeg5Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    tm departureDTTimeairSeg5;
    bzero(&departureDTTimeairSeg5, sizeof(tm));
    strptime("04/06/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg5);
    airSeg5->departureDT() = mktime(&departureDTTimeairSeg5);

    airSeg5->flightNumber() = 200;

    itin4.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "LH";

    const Loc* airSeg6Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    tm departureDTTimeairSeg6;
    bzero(&departureDTTimeairSeg6, sizeof(tm));
    strptime("04/07/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg6);
    airSeg6->departureDT() = mktime(&departureDTTimeairSeg6);

    airSeg6->flightNumber() = 123;

    itin4.push_back(airSeg6);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "LH";

    const Loc* airSeg7Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    tm departureDTTimeairSeg7;
    bzero(&departureDTTimeairSeg7, sizeof(tm));
    strptime("04/05/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg7);
    airSeg7->departureDT() = mktime(&departureDTTimeairSeg7);

    airSeg7->flightNumber() = 200;

    itin5.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "LH";

    const Loc* airSeg8Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    tm departureDTTimeairSeg8;
    bzero(&departureDTTimeairSeg8, sizeof(tm));
    strptime("04/06/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg8);
    airSeg8->departureDT() = mktime(&departureDTTimeairSeg8);

    airSeg8->flightNumber() = 123;

    itin5.push_back(airSeg8);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin6;
    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "LH";

    const Loc* airSeg9Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    tm departureDTTimeairSeg9;
    bzero(&departureDTTimeairSeg9, sizeof(tm));
    strptime("04/08/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg9);
    airSeg9->departureDT() = mktime(&departureDTTimeairSeg9);

    airSeg9->flightNumber() = 200;

    itin6.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "LH";

    const Loc* airSeg10Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    tm departureDTTimeairSeg10;
    bzero(&departureDTTimeairSeg10, sizeof(tm));
    strptime("04/09/04 01:00", "%m/%d/%y %H:%M", &departureDTTimeairSeg10);
    airSeg10->departureDT() = mktime(&departureDTTimeairSeg10);

    airSeg10->flightNumber() = 200;

    itin6.push_back(airSeg10);

    PaxTypeFare ptf6;
    FareMarket fm6;
    fm6.travelSeg().insert(fm6.travelSeg().end(), itin6.begin(), itin6.end());
    fm6.origin() = itin6.front()->origin();
    fm6.destination() = itin6.back()->destination();
    FareInfo fareInfo6;
    fareInfo6.vendor() = "ATP";
    Fare fare6;
    fare6.initialize(Fare::FS_International, &fareInfo6, fm6, 0);
    ptf6.initialize(&fare6, 0, &fm6);
    result = validator.process(ptf6, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample01_positive_single_any()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "SR";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "SN";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "OS";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    CarrierFlightSeg* table1item3 = _memHandle.create<CarrierFlightSeg>();
    table1item3->marketingCarrier() = "LX";
    table1item3->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item3->flt1() = RuleConst::ANY_FLIGHT;
    table1item3->flt2() = 0;
    table1.segs.push_back(table1item3);

    CarrierFlightSeg* table1item4 = _memHandle.create<CarrierFlightSeg>();
    table1item4->marketingCarrier() = "DL";
    table1item4->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item4->flt1() = RuleConst::ANY_FLIGHT;
    table1item4->flt2() = 0;
    table1.segs.push_back(table1item4);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "LX";

    const Loc* airSeg0Orig = validator.getDH().getLoc("TLS", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "SR";

    const Loc* airSeg1Orig = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "SR";

    const Loc* airSeg2Orig = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "OS";

    const Loc* airSeg3Orig = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("TLS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AF";

    const Loc* airSeg4Orig = validator.getDH().getLoc("TLS", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "SR";

    const Loc* airSeg5Orig = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "SQ";

    const Loc* airSeg6Orig = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "AF";

    const Loc* airSeg7Orig = validator.getDH().getLoc("GVA", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("TLS", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample02_positive_single_range()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "DL";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = 7000;
    table1item2->flt2() = 7999;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 1000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 3100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 3200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 1100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "UA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 1020;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 3800;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 3200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 3300;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample03_positive_single_single()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "AA";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 100;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "AA";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 200;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "AA";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = 600;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 600;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "BA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 600;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("WAS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample04_positive_two_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 3000;
    table2item0->flt2() = 3999;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 7000;
    table2item1->flt2() = 7999;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 5000;
    table1item1->flt2() = 5999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 1000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 7000;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 3200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 5000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("TPA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 1000;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("TPA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 7100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 5100;

    itin2.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 3000;

    itin2.push_back(airSeg7);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 5000;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 1000;

    itin3.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 7000;

    itin3.push_back(airSeg10);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample05_positive_two_riv2()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 3000;
    table2item0->flt2() = 3900;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 7000;
    table2item1->flt2() = 7900;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 5000;
    table1item1->flt2() = 5999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::AND;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 1000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 7000;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 3200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 5000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("TPA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 1000;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("TPA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 7100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 5100;

    itin2.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 3000;

    itin2.push_back(airSeg7);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 5000;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 1000;

    itin3.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 7000;

    itin3.push_back(airSeg10);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample06_positive_twomix_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "DL";
    table2item0->flt1() = 3000;
    table2item0->flt2() = 3999;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = "OH";
    table2item1->flt1() = RuleConst::ANY_FLIGHT;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "DL";
    table1item0->flt1() = 100;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "DL";
    table1item1->flt1() = 5000;
    table1item1->flt2() = 5999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 3000;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 3200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 5000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("OH");

    airSeg4->flightNumber() = 302;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("DL");

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->setOperatingCarrierCode("DL");

    airSeg6->flightNumber() = 100;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->setOperatingCarrierCode("DL");

    airSeg7->flightNumber() = 5000;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->setOperatingCarrierCode("DL");

    airSeg8->flightNumber() = 5000;

    itin4.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->setOperatingCarrierCode("DL");

    airSeg9->flightNumber() = 100;

    itin4.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->setOperatingCarrierCode("DL");

    airSeg10->flightNumber() = 3000;

    itin4.push_back(airSeg10);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->setOperatingCarrierCode("DL");

    airSeg11->flightNumber() = 100;

    itin5.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->setOperatingCarrierCode("DL");

    airSeg12->flightNumber() = 3000;

    itin5.push_back(airSeg12);

    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "DL";

    const Loc* airSeg13Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->setOperatingCarrierCode("DL");

    airSeg13->flightNumber() = 5000;

    itin5.push_back(airSeg13);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample08_positive_flttbl_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "AA";
    table2item0->operatingCarrier() = "BA";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "BA";
    table2item1->operatingCarrier() = "BA";
    table2item1->flt1() = 200;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt1() = 400;
    record3.carrier1() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("AA");

    airSeg0->flightNumber() = 400;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "BA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("BA");

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("BA");

    airSeg2->flightNumber() = 1000;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("BA");

    airSeg3->flightNumber() = 400;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample09_positive_flttbl_riv2()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "AA";
    table2item0->operatingCarrier() = "BA";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "BA";
    table2item1->operatingCarrier() = "BA";
    table2item1->flt1() = 200;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt1() = 400;
    record3.carrier1() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("AA");

    airSeg0->flightNumber() = 400;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "BA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("BA");

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("BA");

    airSeg2->flightNumber() = 1000;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("BA");

    airSeg3->flightNumber() = 400;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample10_positive_twoflight_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "DL";
    table2item0->flt1() = 5000;
    table2item0->flt2() = 5999;
    table2.segs.push_back(table2item0);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "DL";
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational2() = FlightApplication::AND;
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt3() = 100;
    record3.carrier3() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 5000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 1000;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 1100;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("DL");

    airSeg4->flightNumber() = 5100;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("DL");

    airSeg5->flightNumber() = 100;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample11_positive_twoflight_riv2()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "DL";
    table2item0->flt1() = 5000;
    table2item0->flt2() = 5999;
    table2.segs.push_back(table2item0);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "DL";
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational2() = FlightApplication::AND;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt3() = 100;
    record3.carrier3() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 5000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 1000;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 1100;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("DL");

    airSeg4->flightNumber() = 5100;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("DL");

    airSeg5->flightNumber() = 100;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample12_positive_twoflight_riv12()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "DL";
    table2item0->flt1() = 5000;
    table2item0->flt2() = 5999;
    table2.segs.push_back(table2item0);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "DL";
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "OH";
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational2() = FlightApplication::CONNECTING;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt3() = 100;
    record3.carrier3() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 5000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 1000;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 5999;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("DL");

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LAX", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("OH");

    airSeg5->flightNumber() = 3000;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample13_negative_single_any()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "SN";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "SR";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "LH";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "LH";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample14_negative_single_range()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "DL";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = 7000;
    table1item2->flt2() = 7999;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("OH");

    airSeg0->flightNumber() = 1000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 4100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 4200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 2100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample15_negative_two_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 5000;
    table2item0->flt2() = 5999;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 7000;
    table2item1->flt2() = 7999;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 7000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 3000;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 3100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 1000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample16_negative_two_riv2()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 5000;
    table2item0->flt2() = 5999;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 7000;
    table2item1->flt2() = 7999;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::AND;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("DL");

    airSeg0->flightNumber() = 7000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("DL");

    airSeg1->flightNumber() = 3000;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 3100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("DL");

    airSeg3->flightNumber() = 1000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample17_negative_two_riv1_segboth()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 200;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 200;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 100;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 200;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ALT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample18_positive_surface_riv1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "QF";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 15;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "QF";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 16;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "QF";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "QF";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 15;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "QF";

    const Loc* airSeg2Orig = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("BNE", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "QF";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    const Loc* airSeg4Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Surface;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "QF";

    const Loc* airSeg5Orig = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("BNE", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 16;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample19_positive_surface_riv2()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "QF";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 15;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "QF";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 16;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "QF";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "QF";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "QF";

    const Loc* airSeg2Orig = validator.getDH().getLoc("SIN", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("BNE", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 16;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample20_positive_connecting_fltTbl()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "OH";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = "EV";
    table2item1->flt1() = RuleConst::ANY_FLIGHT;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "OH";
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "EV";
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "UA";
    table1item2->operatingCarrier() = "UA";
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("EV");

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("EV");

    airSeg1->flightNumber() = 100;

    itin1.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("OH");

    airSeg3->flightNumber() = 100;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("OH");

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("OH");

    airSeg5->flightNumber() = 100;

    itin3.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->setOperatingCarrierCode("OH");

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("PEK", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->setOperatingCarrierCode("UA");

    airSeg7->flightNumber() = 400;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->setOperatingCarrierCode("OH");

    airSeg8->flightNumber() = 100;

    itin4.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->setOperatingCarrierCode("OH");

    airSeg9->flightNumber() = 200;

    itin4.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("PEK", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->setOperatingCarrierCode("EV");

    airSeg10->flightNumber() = 400;

    itin4.push_back(airSeg10);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->setOperatingCarrierCode("OH");

    airSeg11->flightNumber() = 100;

    itin5.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->setOperatingCarrierCode("OH");

    airSeg12->flightNumber() = 200;

    itin5.push_back(airSeg12);

    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "UA";

    const Loc* airSeg13Orig = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("PEK", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->setOperatingCarrierCode("UA");

    airSeg13->flightNumber() = 400;

    itin5.push_back(airSeg13);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample21_negative_connecting_fltTbl()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "OH";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = "EV";
    table2item1->flt1() = RuleConst::ANY_FLIGHT;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "OH";
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "EV";
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "UA";
    table1item2->operatingCarrier() = "UA";
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("EV");

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("EV");

    airSeg1->flightNumber() = 100;

    itin1.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("DL");

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("EV");

    airSeg3->flightNumber() = 100;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("EV");

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample22_connecting_connecting_fltTbl()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "OH";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = "EV";
    table2item1->flt1() = RuleConst::ANY_FLIGHT;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "OH";
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "EV";
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "UA";
    table1item2->operatingCarrier() = "UA";
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational2() = FlightApplication::CONNECTING;
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt3() = 100;
    record3.carrier3() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("EV");

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("EV");

    airSeg1->flightNumber() = 200;

    itin1.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("EV");

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("EV");

    airSeg3->flightNumber() = 300;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("EV");

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("EV");

    airSeg5->flightNumber() = 300;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->setOperatingCarrierCode("EV");

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "AA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->setOperatingCarrierCode("EV");

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->setOperatingCarrierCode("EV");

    airSeg8->flightNumber() = 200;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->setOperatingCarrierCode("EV");

    airSeg9->flightNumber() = 300;

    itin3.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "AA";

    const Loc* airSeg10Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->setOperatingCarrierCode("EV");

    airSeg10->flightNumber() = 100;

    itin3.push_back(airSeg10);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->setOperatingCarrierCode("EV");

    airSeg11->flightNumber() = 200;

    itin4.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "AA";

    const Loc* airSeg12Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->setOperatingCarrierCode("EV");

    airSeg12->flightNumber() = 100;

    itin4.push_back(airSeg12);

    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "DL";

    const Loc* airSeg13Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->setOperatingCarrierCode("EV");

    airSeg13->flightNumber() = 300;

    itin4.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "AA";

    const Loc* airSeg14Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    airSeg14->setOperatingCarrierCode("EV");

    airSeg14->flightNumber() = 100;

    itin4.push_back(airSeg14);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg15 = 0;
    validator.getDH().get(airSeg15);
    airSeg15->carrier() = "DL";

    const Loc* airSeg15Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg15->origin() = airSeg15Orig;

    const Loc* airSeg15Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg15->destination() = airSeg15Dest;

    airSeg15->setOperatingCarrierCode("EV");

    airSeg15->flightNumber() = 200;

    itin5.push_back(airSeg15);

    AirSeg* airSeg16 = 0;
    validator.getDH().get(airSeg16);
    airSeg16->carrier() = "DL";

    const Loc* airSeg16Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg16->origin() = airSeg16Orig;

    const Loc* airSeg16Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg16->destination() = airSeg16Dest;

    airSeg16->setOperatingCarrierCode("EV");

    airSeg16->flightNumber() = 400;

    itin5.push_back(airSeg16);

    AirSeg* airSeg17 = 0;
    validator.getDH().get(airSeg17);
    airSeg17->carrier() = "DL";

    const Loc* airSeg17Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg17->origin() = airSeg17Orig;

    const Loc* airSeg17Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg17->destination() = airSeg17Dest;

    airSeg17->setOperatingCarrierCode("EV");

    airSeg17->flightNumber() = 300;

    itin5.push_back(airSeg17);

    AirSeg* airSeg18 = 0;
    validator.getDH().get(airSeg18);
    airSeg18->carrier() = "AA";

    const Loc* airSeg18Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg18->origin() = airSeg18Orig;

    const Loc* airSeg18Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg18->destination() = airSeg18Dest;

    airSeg18->setOperatingCarrierCode("EV");

    airSeg18->flightNumber() = 100;

    itin5.push_back(airSeg18);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample23_neg_connecting_connecting_fltTbl()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = "OH";
    table2item0->flt1() = RuleConst::ANY_FLIGHT;
    table2item0->flt2() = 0;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = "EV";
    table2item1->flt1() = RuleConst::ANY_FLIGHT;
    table2item1->flt2() = 0;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = "OH";
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = "EV";
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "UA";
    table1item2->operatingCarrier() = "UA";
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational2() = FlightApplication::CONNECTING;
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt3() = 100;
    record3.carrier3() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "AA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->setOperatingCarrierCode("EV");

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("EV");

    airSeg1->flightNumber() = 200;

    itin1.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->setOperatingCarrierCode("EV");

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("EV");

    airSeg3->flightNumber() = 300;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->setOperatingCarrierCode("EV");

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->setOperatingCarrierCode("EV");

    airSeg5->flightNumber() = 300;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->setOperatingCarrierCode("EV");

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "AA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->setOperatingCarrierCode("EV");

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->setOperatingCarrierCode("EV");

    airSeg8->flightNumber() = 200;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->setOperatingCarrierCode("EV");

    airSeg9->flightNumber() = 300;

    itin3.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "AA";

    const Loc* airSeg10Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->setOperatingCarrierCode("EV");

    airSeg10->flightNumber() = 100;

    itin3.push_back(airSeg10);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->setOperatingCarrierCode("EV");

    airSeg11->flightNumber() = 200;

    itin4.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "AA";

    const Loc* airSeg12Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->setOperatingCarrierCode("EV");

    airSeg12->flightNumber() = 100;

    itin4.push_back(airSeg12);

    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "DL";

    const Loc* airSeg13Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->setOperatingCarrierCode("EV");

    airSeg13->flightNumber() = 300;

    itin4.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "AA";

    const Loc* airSeg14Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    airSeg14->setOperatingCarrierCode("EV");

    airSeg14->flightNumber() = 100;

    itin4.push_back(airSeg14);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg15 = 0;
    validator.getDH().get(airSeg15);
    airSeg15->carrier() = "DL";

    const Loc* airSeg15Orig = validator.getDH().getLoc("ROM", validator.getDH().ticketDate());
    airSeg15->origin() = airSeg15Orig;

    const Loc* airSeg15Dest = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg15->destination() = airSeg15Dest;

    airSeg15->setOperatingCarrierCode("EV");

    airSeg15->flightNumber() = 200;

    itin5.push_back(airSeg15);

    AirSeg* airSeg16 = 0;
    validator.getDH().get(airSeg16);
    airSeg16->carrier() = "DL";

    const Loc* airSeg16Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg16->origin() = airSeg16Orig;

    const Loc* airSeg16Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg16->destination() = airSeg16Dest;

    airSeg16->setOperatingCarrierCode("EV");

    airSeg16->flightNumber() = 400;

    itin5.push_back(airSeg16);

    AirSeg* airSeg17 = 0;
    validator.getDH().get(airSeg17);
    airSeg17->carrier() = "DL";

    const Loc* airSeg17Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg17->origin() = airSeg17Orig;

    const Loc* airSeg17Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg17->destination() = airSeg17Dest;

    airSeg17->setOperatingCarrierCode("EV");

    airSeg17->flightNumber() = 300;

    itin5.push_back(airSeg17);

    AirSeg* airSeg18 = 0;
    validator.getDH().get(airSeg18);
    airSeg18->carrier() = "AA";

    const Loc* airSeg18Orig = validator.getDH().getLoc("BKK", validator.getDH().ticketDate());
    airSeg18->origin() = airSeg18Orig;

    const Loc* airSeg18Dest = validator.getDH().getLoc("MOS", validator.getDH().ticketDate());
    airSeg18->destination() = airSeg18Dest;

    airSeg18->setOperatingCarrierCode("EV");

    airSeg18->flightNumber() = 100;

    itin5.push_back(airSeg18);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample01_RTW()
  {
    // Positive case, all cxrs belongs to the same alliance
    commonRTW(NOT_APPLY, FlightApplication::MUST, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*A", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", EMPTY_CARRIER, 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", EMPTY_CARRIER, 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("LH", EMPTY_CARRIER, 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", EMPTY_CARRIER, 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample02_RTW()
  {
    // Negative case, one cxr (AB) doesn't belong to *A alliance
    commonRTW(NOT_APPLY, FlightApplication::MUST, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*A", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", EMPTY_CARRIER, 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", EMPTY_CARRIER, 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("AB", EMPTY_CARRIER, 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", EMPTY_CARRIER, 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample03_RTW()
  {
    // Positive case, all cxrs don't belong to *A
    commonRTW(NOT_APPLY, FlightApplication::MUST_NOT, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*A", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("AA", "AA", 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("AB", "AB", 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("AF", "AF", 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("AM", "AM", 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample04_RTW()
  {
    // Negative case, one cxr doesn't belong to *A
    commonRTW(NOT_APPLY, FlightApplication::MUST_NOT, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*A", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", "LO", 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", "LH", 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("AA", "AA", 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", "XZ", 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample05_RTW()
  {
    // Positive case, one cxr doesn't belong to *A but specified in T986 separately
    commonRTW(NOT_APPLY, FlightApplication::MUST, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*A", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWaddToTable986("XX", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", "LO", 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", "LH", 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("XX", "XX", 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", "XZ", 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void flightTableExample06_RTW()
  {
    // Negative case, one cxr is specified in T986 separately
    commonRTW(NOT_APPLY, FlightApplication::MUST_NOT, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("*O", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWaddToTable986("XX", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", "LO", 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", "LH", 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("XX", "XX", 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", "XZ", 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void flightTableExample07_RTW()
  {
    // Positive case, all cxrs are specified in T986 separately
    commonRTW(NOT_APPLY, FlightApplication::MUST, true);
    commonRTWCreateMockTable();
    commonRTWaddToTable986("LO", "LO", RuleConst::ANY_FLIGHT, 0);
    commonRTWaddToTable986("XX", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWaddToTable986("LH", EMPTY_CARRIER, RuleConst::ANY_FLIGHT, 0);
    commonRTWaddToTable986("LO", "XZ", RuleConst::ANY_FLIGHT, 0);
    commonRTWAddTableToFlightAppRules();

    commonRTWCreateFlightApplication();

#ifdef DEBUG
    _flightApplication->printTree();
#endif

    commonRTWCreateFareMarket();
    commonRTWAddAirSegmentToFareMarket("LO", "LO", 123, "KRK", "FRA");
    commonRTWAddAirSegmentToFareMarket("LH", "LH", 234, "FRA", "DFW");
    commonRTWAddAirSegmentToFareMarket("XX", "XX", 345, "DFW", "FRA");
    commonRTWAddAirSegmentToFareMarket("LO", "XZ", 456, "FRA", "KRK");
    commonRTWCreatePTF();

    Record3ReturnTypes result;
    result = _flightApplication->process(*_paxTypeFare, *_trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void mustBtAndGeoNegFltExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.geoTblItemNoBetwVia() = 533;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.geoTblItemNoAndVia() = 533;
    record3.geoAppl() = '1';
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NH";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 200;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "NH";

    const Loc* airSeg8Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 100;

    itin3.push_back(airSeg9);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void mustBtAndGeoNegViaPossitiveFltExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.fltAppl() = FlightApplication::MUST;
    record3.geoTblItemNoBetwVia() = 122;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.geoTblItemNoAndVia() = 122;
    record3.geoAppl() = '1';
    record3.viaInd() = '0';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 300;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("SFO", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    // must be via US on carrier DL
    record3.viaInd() = '1';

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("SFO", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    // must not be via DL (match segment, fail cxr)
    record3.viaInd() = '0';

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "AA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 200;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void mustFromToExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.geoTblItemNoBetwVia() = 1;
    record3.geoAppl() = '1';
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NH";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "NH";

    const Loc* airSeg7Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 200;

    itin4.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    const Loc* airSeg100 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg100));
    const Loc* airSeg101 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg101));

    itin4.push_back(airSeg10);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    const Loc* airSeg110 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg11->hiddenStops().push_back(const_cast<Loc*>(airSeg110));

    airSeg11->flightNumber() = 200;

    itin5.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    const Loc* airSeg120 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg12->hiddenStops().push_back(const_cast<Loc*>(airSeg120));

    airSeg12->flightNumber() = 100;

    itin5.push_back(airSeg12);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void mustFromToViaExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'X';
    record3.geoTblItemNoBetwVia() = 1;
    record3.geoAppl() = '1';
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NH";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "NH";

    const Loc* airSeg7Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 200;

    itin4.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    const Loc* airSeg100 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg100));
    const Loc* airSeg101 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg101));

    itin4.push_back(airSeg10);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    const Loc* airSeg110 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg11->hiddenStops().push_back(const_cast<Loc*>(airSeg110));

    airSeg11->flightNumber() = 200;

    itin5.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    const Loc* airSeg120 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg12->hiddenStops().push_back(const_cast<Loc*>(airSeg120));

    airSeg12->flightNumber() = 100;

    itin5.push_back(airSeg12);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin6;
    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "DL";

    const Loc* airSeg13Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    const Loc* airSeg130 = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg13->hiddenStops().push_back(const_cast<Loc*>(airSeg130));

    airSeg13->flightNumber() = 200;

    itin6.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "DL";

    const Loc* airSeg14Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    const Loc* airSeg140 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg14->hiddenStops().push_back(const_cast<Loc*>(airSeg140));

    airSeg14->flightNumber() = 100;

    itin6.push_back(airSeg14);

    PaxTypeFare ptf6;
    FareMarket fm6;
    fm6.travelSeg().insert(fm6.travelSeg().end(), itin6.begin(), itin6.end());
    fm6.origin() = itin6.front()->origin();
    fm6.destination() = itin6.back()->destination();
    FareInfo fareInfo6;
    fareInfo6.vendor() = "ATP";
    Fare fare6;
    fare6.initialize(Fare::FS_International, &fareInfo6, fm6, 0);
    ptf6.initialize(&fare6, 0, &fm6);
    result = validator.process(ptf6, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void mustFromToViaHiddenNExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'X';
    record3.geoTblItemNoBetwVia() = 1;
    record3.hidden() = 'N';
    record3.geoAppl() = '1';
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NH";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "NH";

    const Loc* airSeg7Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 200;

    itin4.push_back(airSeg9);

    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    const Loc* airSeg100 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg100));
    const Loc* airSeg101 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg101));

    itin4.push_back(airSeg10);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    const Loc* airSeg110 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg11->hiddenStops().push_back(const_cast<Loc*>(airSeg110));

    airSeg11->flightNumber() = 200;

    itin5.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    const Loc* airSeg120 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg12->hiddenStops().push_back(const_cast<Loc*>(airSeg120));

    airSeg12->flightNumber() = 100;

    itin5.push_back(airSeg12);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin6;
    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "DL";

    const Loc* airSeg13Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    const Loc* airSeg130 = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg13->hiddenStops().push_back(const_cast<Loc*>(airSeg130));

    airSeg13->flightNumber() = 200;

    itin6.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "DL";

    const Loc* airSeg14Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    const Loc* airSeg140 = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg14->hiddenStops().push_back(const_cast<Loc*>(airSeg140));

    airSeg14->flightNumber() = 100;

    itin6.push_back(airSeg14);

    PaxTypeFare ptf6;
    FareMarket fm6;
    fm6.travelSeg().insert(fm6.travelSeg().end(), itin6.begin(), itin6.end());
    fm6.origin() = itin6.front()->origin();
    fm6.destination() = itin6.back()->destination();
    FareInfo fareInfo6;
    fareInfo6.vendor() = "ATP";
    Fare fare6;
    fare6.initialize(Fare::FS_International, &fareInfo6, fm6, 0);
    ptf6.initialize(&fare6, 0, &fm6);
    result = validator.process(ptf6, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void mustNotMustNotInterline()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltInterline() = FlightApplication::INVALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 300;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "NW";

    const Loc* airSeg6Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void negBtAndGeoPossitiveFltExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.fltAppl() = FlightApplication::MUST;
    record3.geoTblItemNoBetwVia() = 533;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.geoTblItemNoAndVia() = 533;
    record3.geoAppl() = '0';
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NH";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 200;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "NH";

    const Loc* airSeg8Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 100;

    itin3.push_back(airSeg9);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void negBtAndGeoPosViaPosFltExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.fltAppl() = FlightApplication::MUST;
    record3.geoTblItemNoBetwVia() = 122;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.geoTblItemNoAndVia() = 122;
    record3.geoAppl() = '0';
    record3.viaInd() = '1';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NRT", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("OSA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 300;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("SFO", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("ORD", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("SFO", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "AA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 200;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void negBtAndGeoPosViaPosFltExample02()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.locAppl() = 'B';
    record3.fltAppl() = FlightApplication::MUST;
    record3.geoTblItemNoBetwVia() = 55;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.geoTblItemNoAndVia() = 50;
    record3.geoAppl() = '0';
    record3.viaInd() = '1';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void negGeoPossitiveFltExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.viaInd() = '0';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    const Loc* airSeg80 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg8->hiddenStops().push_back(const_cast<Loc*>(airSeg80));

    airSeg8->flightNumber() = 200;

    itin4.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 100;

    itin4.push_back(airSeg9);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin5;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    const Loc* airSeg100 = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg10->hiddenStops().push_back(const_cast<Loc*>(airSeg100));

    airSeg10->flightNumber() = 200;

    itin5.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->flightNumber() = 100;

    itin5.push_back(airSeg11);

    PaxTypeFare ptf5;
    FareMarket fm5;
    fm5.travelSeg().insert(fm5.travelSeg().end(), itin5.begin(), itin5.end());
    fm5.origin() = itin5.front()->origin();
    fm5.destination() = itin5.back()->destination();
    FareInfo fareInfo5;
    fareInfo5.vendor() = "ATP";
    Fare fare5;
    fare5.initialize(Fare::FS_International, &fareInfo5, fm5, 0);
    ptf5.initialize(&fare5, 0, &fm5);
    result = validator.process(ptf5, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_mustnot0()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Surface;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Surface;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_mustnot1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = 100;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Surface;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Surface;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_must()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Surface;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Surface;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_mustbtw()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_RANGE;
    record3.flt1() = 1;
    record3.carrier1() = "DL";
    record3.flt2() = 1000;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Surface;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "BA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_mustnotbtw()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT_RANGE;
    record3.flt1() = 1;
    record3.carrier1() = "DL";
    record3.flt2() = 1000;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 1200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "BA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    const Loc* airSeg7Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->segmentType() = Open;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "BA";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->setOperatingCarrierCode("DL");

    airSeg8->flightNumber() = 200;

    itin3.push_back(airSeg8);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_flttable_must()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "DL";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = 7000;
    table1item2->flt2() = 7999;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("BA");

    airSeg1->flightNumber() = 1200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("BA");

    airSeg3->flightNumber() = 1200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_flttable_mustnot()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 3000;
    table1item1->flt2() = 3999;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "DL";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = 7000;
    table1item2->flt2() = 7999;
    table1.segs.push_back(table1item2);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->setOperatingCarrierCode("BA");

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->setOperatingCarrierCode("BA");

    airSeg3->flightNumber() = 3200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_flttbl_rlt1()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;

    MockTable986 table2;
    CarrierFlightSeg* table2item0 = _memHandle.create<CarrierFlightSeg>();
    table2item0->marketingCarrier() = "DL";
    table2item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item0->flt1() = 3000;
    table2item0->flt2() = 3999;
    table2.segs.push_back(table2item0);

    CarrierFlightSeg* table2item1 = _memHandle.create<CarrierFlightSeg>();
    table2item1->marketingCarrier() = "DL";
    table2item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table2item1->flt1() = 7000;
    table2item1->flt2() = 7999;
    table2.segs.push_back(table2item1);

    record3.carrierFltTblItemNo2() = MockTable986::putTable(&table2);

    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "DL";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = 1000;
    table1item0->flt2() = 1999;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "DL";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = 5000;
    table1item1->flt2() = 5999;
    table1.segs.push_back(table1item1);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    record3.fltRelational1() = FlightApplication::CONNECTING;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 1000;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Open;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MIA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 3000;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void relationsExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltRelational1() = FlightApplication::AND_OR;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "KL";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "KL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "KL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void relationsExample02()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 201;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "CO";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 888;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "UA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "UA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 501;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "DL";

    const Loc* airSeg9Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 201;

    itin3.push_back(airSeg9);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 1000;

    itin4.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->flightNumber() = 400;

    itin4.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->flightNumber() = 333;

    itin4.push_back(airSeg12);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void relationsExample03()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 400;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 333;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void relationsExample04()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltRelational2() = FlightApplication::CONNECTING;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "UA";
    record3.flt3() = 300;
    record3.carrier3() = "DL";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 300;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 300;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 201;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "UA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 101;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin2.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "UA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin2.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 300;

    itin2.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "AA";

    const Loc* airSeg9Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 1555;

    itin2.push_back(airSeg9);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "AA";

    const Loc* airSeg10Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 200;

    itin3.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->flightNumber() = 300;

    itin3.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "UA";

    const Loc* airSeg12Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->flightNumber() = 100;

    itin3.push_back(airSeg12);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "AA";

    const Loc* airSeg13Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->flightNumber() = 200;

    itin4.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "AA";

    const Loc* airSeg14Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    airSeg14->flightNumber() = 1000;

    itin4.push_back(airSeg14);

    AirSeg* airSeg15 = 0;
    validator.getDH().get(airSeg15);
    airSeg15->carrier() = "UA";

    const Loc* airSeg15Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg15->origin() = airSeg15Orig;

    const Loc* airSeg15Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg15->destination() = airSeg15Dest;

    airSeg15->flightNumber() = 100;

    itin4.push_back(airSeg15);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void relationsExample05()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.fltRelational1() = FlightApplication::AND_OR;
    record3.flt1() = 100;
    record3.carrier1() = "KL";
    record3.flt2() = 200;
    record3.carrier2() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 400;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "KL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 500;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "KL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 501;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "DL";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "KL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 500;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void relationsExample06()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.fltRelational1() = FlightApplication::CONNECTING;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ALT", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 201;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 4001;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "UA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 300;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "UA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 3200;

    itin3.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "UA";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 100;

    itin3.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "AA";

    const Loc* airSeg9Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 4000;

    itin3.push_back(airSeg9);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "DL";

    const Loc* airSeg10Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 222;

    itin4.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "UA";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->flightNumber() = 100;

    itin4.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "DL";

    const Loc* airSeg12Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->flightNumber() = 300;

    itin4.push_back(airSeg12);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void relationsExample07()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt1() = 100;
    record3.carrier1() = "UA";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "UA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "UA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "UA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 400;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void relationsExample08()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.fltRelational2() = FlightApplication::CONNECTING;
    record3.fltRelational1() = FlightApplication::AND;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "UA";
    record3.flt3() = 300;
    record3.carrier3() = "DL";
    record3.flt2() = RuleConst::ANY_FLIGHT;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "UA";

    const Loc* airSeg0Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 300;

    itin0.push_back(airSeg2);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "DL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 300;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 201;

    itin1.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "UA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 101;

    itin1.push_back(airSeg5);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "AA";

    const Loc* airSeg6Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin2.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "UA";

    const Loc* airSeg7Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin2.push_back(airSeg7);

    AirSeg* airSeg8 = 0;
    validator.getDH().get(airSeg8);
    airSeg8->carrier() = "DL";

    const Loc* airSeg8Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg8->origin() = airSeg8Orig;

    const Loc* airSeg8Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg8->destination() = airSeg8Dest;

    airSeg8->flightNumber() = 300;

    itin2.push_back(airSeg8);

    AirSeg* airSeg9 = 0;
    validator.getDH().get(airSeg9);
    airSeg9->carrier() = "AA";

    const Loc* airSeg9Orig = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg9->origin() = airSeg9Orig;

    const Loc* airSeg9Dest = validator.getDH().getLoc("ZRH", validator.getDH().ticketDate());
    airSeg9->destination() = airSeg9Dest;

    airSeg9->flightNumber() = 1555;

    itin2.push_back(airSeg9);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg10 = 0;
    validator.getDH().get(airSeg10);
    airSeg10->carrier() = "AA";

    const Loc* airSeg10Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg10->origin() = airSeg10Orig;

    const Loc* airSeg10Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg10->destination() = airSeg10Dest;

    airSeg10->flightNumber() = 200;

    itin3.push_back(airSeg10);

    AirSeg* airSeg11 = 0;
    validator.getDH().get(airSeg11);
    airSeg11->carrier() = "DL";

    const Loc* airSeg11Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg11->origin() = airSeg11Orig;

    const Loc* airSeg11Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg11->destination() = airSeg11Dest;

    airSeg11->flightNumber() = 300;

    itin3.push_back(airSeg11);

    AirSeg* airSeg12 = 0;
    validator.getDH().get(airSeg12);
    airSeg12->carrier() = "UA";

    const Loc* airSeg12Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg12->origin() = airSeg12Orig;

    const Loc* airSeg12Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg12->destination() = airSeg12Dest;

    airSeg12->flightNumber() = 100;

    itin3.push_back(airSeg12);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin4;
    AirSeg* airSeg13 = 0;
    validator.getDH().get(airSeg13);
    airSeg13->carrier() = "AA";

    const Loc* airSeg13Orig = validator.getDH().getLoc("DFW", validator.getDH().ticketDate());
    airSeg13->origin() = airSeg13Orig;

    const Loc* airSeg13Dest = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg13->destination() = airSeg13Dest;

    airSeg13->flightNumber() = 200;

    itin4.push_back(airSeg13);

    AirSeg* airSeg14 = 0;
    validator.getDH().get(airSeg14);
    airSeg14->carrier() = "AA";

    const Loc* airSeg14Orig = validator.getDH().getLoc("ATL", validator.getDH().ticketDate());
    airSeg14->origin() = airSeg14Orig;

    const Loc* airSeg14Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg14->destination() = airSeg14Dest;

    airSeg14->flightNumber() = 1000;

    itin4.push_back(airSeg14);

    AirSeg* airSeg15 = 0;
    validator.getDH().get(airSeg15);
    airSeg15->carrier() = "UA";

    const Loc* airSeg15Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg15->origin() = airSeg15Orig;

    const Loc* airSeg15Dest = validator.getDH().getLoc("PAR", validator.getDH().ticketDate());
    airSeg15->destination() = airSeg15Dest;

    airSeg15->flightNumber() = 100;

    itin4.push_back(airSeg15);

    PaxTypeFare ptf4;
    FareMarket fm4;
    fm4.travelSeg().insert(fm4.travelSeg().end(), itin4.begin(), itin4.end());
    fm4.origin() = itin4.front()->origin();
    fm4.destination() = itin4.back()->destination();
    FareInfo fareInfo4;
    fareInfo4.vendor() = "ATP";
    Fare fare4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fm4, 0);
    ptf4.initialize(&fare4, 0, &fm4);
    result = validator.process(ptf4, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void test40Example01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    const Loc* airSeg2Orig = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void test40Example02()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Surface;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void testQualifierOpenseg_ifnot()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, true, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Open;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(SKIP, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "BA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "BA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Open;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "BA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "DL";

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Open;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void testQualifierOpenseg_if()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, true, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg1Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->segmentType() = Open;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(SKIP, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->segmentType() = Open;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->segmentType() = Open;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->segmentType() = Open;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = FlightApplication::ANY_CARRIER;

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->segmentType() = Open;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void typeOfFlightExample01_nonstop()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("ASP", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void typeOfFlightExample02_direct()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void typeOfFlightExample03_multistop()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 300;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "NW";

    const Loc* airSeg6Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "NW";

    const Loc* airSeg7Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin2.push_back(airSeg7);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void typeOfFlightExample04_onestop()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin1.push_back(airSeg4);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "NW";

    const Loc* airSeg6Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 100;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void typeOfFlightExample05_online()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "KL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "KL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 300;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void typeOfFlightExample06_interline()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::VALID;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "KL";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 200;

    itin2.push_back(airSeg5);

    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "KL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin2.push_back(airSeg6);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void typeOfFlightExample07_sameflight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltSame() = FlightApplication::VALID;
    record3.fltAppl() = FlightApplication::BLANK;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void typeOfFlightExample08_pos_multiple_pos_flight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltMultiStop() = FlightApplication::VALID;
    record3.fltNonStop() = FlightApplication::VALID;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 100;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void typeOfFlightExample09_neg_multiple_pos_flight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.fltMultiStop() = FlightApplication::INVALID;
    record3.fltNonStop() = FlightApplication::INVALID;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 300;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void typeOfFlightExample10_pos_mutliple_neg_flight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_NOT;
    record3.fltMultiStop() = FlightApplication::VALID;
    record3.fltNonStop() = FlightApplication::VALID;
    record3.flt1() = 100;
    record3.carrier1() = "NW";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("NSO", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 300;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "NW";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 199;

    itin2.push_back(airSeg3);

    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "NW";

    const Loc* airSeg4Orig = validator.getDH().getLoc("CHI", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "NW";

    const Loc* airSeg5Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 300;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void viaInterlineMustViaJFKInterline()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.viaInd() = '1';
    record3.fltInterline() = FlightApplication::VALID;
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "NW";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "NW";

    const Loc* airSeg1Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 200;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "AA";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 100;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "NW";

    const Loc* airSeg3Orig = validator.getDH().getLoc("NYC", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("AMS", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 200;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void viaJFKExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.viaInd() = '1';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void viaJFKHiddenNExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.hidden() = 'N';
    record3.viaInd() = '1';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    const Loc* airSeg40 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->hiddenStops().push_back(const_cast<Loc*>(airSeg40));

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void viaJFKHiddenYExample01()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.hidden() = 'Y';
    record3.viaInd() = '1';
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "AA";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    const Loc* airSeg40 = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->hiddenStops().push_back(const_cast<Loc*>(airSeg40));

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void viaOnlineMustViaJFK_DLOnline()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = RuleConst::ANY_FLIGHT;
    record3.carrier1() = "DL";
    record3.viaInd() = '1';
    record3.fltOnline() = FlightApplication::VALID;
    record3.geoTblItemNoVia() = 1;
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->flightNumber() = 200;

    itin0.push_back(airSeg0);

    AirSeg* airSeg1 = 0;
    validator.getDH().get(airSeg1);
    airSeg1->carrier() = "DL";

    const Loc* airSeg1Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg1->origin() = airSeg1Orig;

    const Loc* airSeg1Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg1->destination() = airSeg1Dest;

    airSeg1->flightNumber() = 100;

    itin0.push_back(airSeg1);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);

    // ------
    std::vector<TravelSeg*> itin1;
    AirSeg* airSeg2 = 0;
    validator.getDH().get(airSeg2);
    airSeg2->carrier() = "DL";

    const Loc* airSeg2Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg2->origin() = airSeg2Orig;

    const Loc* airSeg2Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg2->destination() = airSeg2Dest;

    airSeg2->flightNumber() = 200;

    itin1.push_back(airSeg2);

    AirSeg* airSeg3 = 0;
    validator.getDH().get(airSeg3);
    airSeg3->carrier() = "AA";

    const Loc* airSeg3Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg3->origin() = airSeg3Orig;

    const Loc* airSeg3Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg3->destination() = airSeg3Dest;

    airSeg3->flightNumber() = 100;

    itin1.push_back(airSeg3);

    PaxTypeFare ptf1;
    FareMarket fm1;
    fm1.travelSeg().insert(fm1.travelSeg().end(), itin1.begin(), itin1.end());
    fm1.origin() = itin1.front()->origin();
    fm1.destination() = itin1.back()->destination();
    FareInfo fareInfo1;
    fareInfo1.vendor() = "ATP";
    Fare fare1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fm1, 0);
    ptf1.initialize(&fare1, 0, &fm1);
    result = validator.process(ptf1, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin2;
    AirSeg* airSeg4 = 0;
    validator.getDH().get(airSeg4);
    airSeg4->carrier() = "AA";

    const Loc* airSeg4Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg4->origin() = airSeg4Orig;

    const Loc* airSeg4Dest = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg4->destination() = airSeg4Dest;

    airSeg4->flightNumber() = 200;

    itin2.push_back(airSeg4);

    AirSeg* airSeg5 = 0;
    validator.getDH().get(airSeg5);
    airSeg5->carrier() = "AA";

    const Loc* airSeg5Orig = validator.getDH().getLoc("JFK", validator.getDH().ticketDate());
    airSeg5->origin() = airSeg5Orig;

    const Loc* airSeg5Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg5->destination() = airSeg5Dest;

    airSeg5->flightNumber() = 100;

    itin2.push_back(airSeg5);

    PaxTypeFare ptf2;
    FareMarket fm2;
    fm2.travelSeg().insert(fm2.travelSeg().end(), itin2.begin(), itin2.end());
    fm2.origin() = itin2.front()->origin();
    fm2.destination() = itin2.back()->destination();
    FareInfo fareInfo2;
    fareInfo2.vendor() = "ATP";
    Fare fare2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fm2, 0);
    ptf2.initialize(&fare2, 0, &fm2);
    result = validator.process(ptf2, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);

    // ------
    std::vector<TravelSeg*> itin3;
    AirSeg* airSeg6 = 0;
    validator.getDH().get(airSeg6);
    airSeg6->carrier() = "DL";

    const Loc* airSeg6Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg6->origin() = airSeg6Orig;

    const Loc* airSeg6Dest = validator.getDH().getLoc("IAD", validator.getDH().ticketDate());
    airSeg6->destination() = airSeg6Dest;

    airSeg6->flightNumber() = 200;

    itin3.push_back(airSeg6);

    AirSeg* airSeg7 = 0;
    validator.getDH().get(airSeg7);
    airSeg7->carrier() = "DL";

    const Loc* airSeg7Orig = validator.getDH().getLoc("IAD", validator.getDH().ticketDate());
    airSeg7->origin() = airSeg7Orig;

    const Loc* airSeg7Dest = validator.getDH().getLoc("FRA", validator.getDH().ticketDate());
    airSeg7->destination() = airSeg7Dest;

    airSeg7->flightNumber() = 100;

    itin3.push_back(airSeg7);

    PaxTypeFare ptf3;
    FareMarket fm3;
    fm3.travelSeg().insert(fm3.travelSeg().end(), itin3.begin(), itin3.end());
    fm3.origin() = itin3.front()->origin();
    fm3.destination() = itin3.back()->destination();
    FareInfo fareInfo3;
    fareInfo3.vendor() = "ATP";
    Fare fare3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fm3, 0);
    ptf3.initialize(&fare3, 0, &fm3);
    result = validator.process(ptf3, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_must_pass_withFlight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_mustbtw_pass_withFlight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_RANGE;
    record3.flt1() = 1;
    record3.carrier1() = "DL";
    record3.flt2() = 1000;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_must_fail_withFlight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 101;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_mustbtw_fail_withFlight()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST_RANGE;
    record3.flt1() = 1;
    record3.carrier1() = "DL";
    record3.flt2() = 1000;
    record3.carrier2() = "AA";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 11100;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void openSegOpenseg_must_pass_EQPwithFlight_WQ()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    NoPNRPricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.equipType() = "747";
    record3.equipAppl() = FlightApplication::VALID;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 100;
    airSeg0->equipmentType() = "747";

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void openSegOpenseg_must_fail_EQPwithFlight_WQ()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    NoPNRPricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.equipType() = "747";
    record3.equipAppl() = FlightApplication::VALID;
    record3.carrier1() = "DL";
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;
    airSeg0->flightNumber() = 100;
    airSeg0->equipmentType() = "727";

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void commonInOrOutBound()
  {
    _trx = _memHandle.create<PricingTrx>();
    PricingRequest* request = _memHandle.create<PricingRequest>();
    _trx->setRequest(request);

    FlightAppRule* flightAppRule = _memHandle.create<FlightAppRule>();
    flightAppRule->carrierFltTblItemNo1() = 27533;
    flightAppRule->carrierFltTblItemNo2() = 0;
    flightAppRule->flt1() = 0;
    flightAppRule->flt2() = 0;
    flightAppRule->flt3() = 0;
    flightAppRule->equipAppl() = FlightApplication::BLANK;
    flightAppRule->fltNonStop() = FlightApplication::BLANK;
    flightAppRule->fltDirect() = FlightApplication::BLANK;
    flightAppRule->fltMultiStop() = FlightApplication::BLANK;
    flightAppRule->fltOneStop() = FlightApplication::BLANK;
    flightAppRule->fltOnline() = FlightApplication::BLANK;
    flightAppRule->fltInterline() = FlightApplication::BLANK;
    flightAppRule->fltSame() = FlightApplication::BLANK;
    flightAppRule->overrideDateTblItemNo() = 0;
    flightAppRule->geoAppl() = FlightApplication::CONDITIONAL;
    flightAppRule->locAppl() = FlightApplication::BETWEEN_AND;
    flightAppRule->viaInd() = FlightApplication::BLANK;
    flightAppRule->inOutInd() = '4';
    flightAppRule->geoTblItemNoBetwVia() = 622;
    flightAppRule->geoTblItemNoAndVia() = 3036;
    flightAppRule->geoTblItemNoVia() = 0;
    flightAppRule->vendor() = "ATP";
    flightAppRule->fltAppl() = FlightApplication::MUST;

    _flightApplication = _memHandle.create<FlightApplication>();
    const VendorCode vendor = "ATP";
    _flightApplication->initialize(flightAppRule, false, vendor, _trx);

    _pricingUnit = _memHandle.create<PricingUnit>();
  }

  void commonInOrOutBoundAddAirSegment(const CarrierCode carrier,
                                       const FlightNumber flightNumber,
                                       const LocCode origin,
                                       const LocCode destination,
                                       const bool isInBound)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = carrier;
    airSeg->flightNumber() = flightNumber;
    airSeg->origAirport() = origin;
    airSeg->destAirport() = destination;
    airSeg->origin() =
        _flightApplication->getDH().getLoc(origin, _flightApplication->getDH().ticketDate());
    airSeg->destination() =
        _flightApplication->getDH().getLoc(destination, _flightApplication->getDH().ticketDate());

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(airSeg);
    fareMarket->origin() = airSeg->origin();
    fareMarket->destination() = airSeg->destination();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, 0);

    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    paxTypeFare->initialize(fare, 0, fareMarket);

    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->inbound() = isInBound;
    fareUsage->paxTypeFare() = paxTypeFare;

    _pricingUnit->fareUsage().push_back(fareUsage);
  }

  void testInOrOutBoundFailSkip()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    //    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", false);
    //    commonInOrOutBoundAddAirSegment("CA", 168, "NRT", "PEK", true);
    //    commonInOrOutBoundAddAirSegment("CA", 975, "PEK", "SIN", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void testInOrOutBoundSkipFail()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("CA", 168, "NRT", "PEK", false);
    commonInOrOutBoundAddAirSegment("CA", 975, "PEK", "SIN", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void testInOrOutBoundPassSkip()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonInOrOutBoundAddAirSegment("CA", 168, "NRT", "PEK", true);
    commonInOrOutBoundAddAirSegment("CA", 975, "PEK", "SIN", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void testInOrOutBoundSkipPass()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("CA", 168, "NRT", "PEK", false);
    commonInOrOutBoundAddAirSegment("CA", 975, "PEK", "SIN", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void testInOrOutBoundSkipSkip()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("CA", 168, "NRT", "PEK", false);
    commonInOrOutBoundAddAirSegment("CA", 975, "PEK", "SIN", false);
    commonInOrOutBoundAddAirSegment("CA", 608, "SIN", "PEK", true);
    commonInOrOutBoundAddAirSegment("CA", 778, "PEK", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(SKIP, result);
  }

  void testInOrOutBoundFailFail()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(FAIL, result);
  }

  void testInOrOutBoundPassPass()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void testInOrOutBoundFailPass()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void testInOrOutBoundPassFail()
  {
    commonInOrOutBound();
    commonInOrOutBoundAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonInOrOutBoundAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonInOrOutBoundAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonInOrOutBoundAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

  void commonRTW(const Indicator dir, const Indicator fltAppl, bool rtwReq)
  {
    _memHandle.get(_trx);
    PricingRequest* request = _memHandle.create<PricingRequest>();
    _trx->setRequest(request);

    PricingOptions* pricOptions = _memHandle.create<PricingOptions>();
    pricOptions->setRtw(rtwReq);
    _trx->setOptions(pricOptions);

    _flightAppRule = _memHandle.create<FlightAppRule>();
    _flightAppRule->carrierFltTblItemNo1() = 27533;
    _flightAppRule->carrierFltTblItemNo2() = 0;
    _flightAppRule->flt1() = 0;
    _flightAppRule->flt2() = 0;
    _flightAppRule->flt3() = 0;
    _flightAppRule->fltRelational1() = FlightApplication::BLANK;
    _flightAppRule->fltRelational2() = FlightApplication::BLANK;
    _flightAppRule->equipAppl() = FlightApplication::BLANK;
    _flightAppRule->fltNonStop() = FlightApplication::BLANK;
    _flightAppRule->fltDirect() = FlightApplication::BLANK;
    _flightAppRule->fltMultiStop() = FlightApplication::BLANK;
    _flightAppRule->fltOneStop() = FlightApplication::BLANK;
    _flightAppRule->fltOnline() = FlightApplication::BLANK;
    _flightAppRule->fltInterline() = FlightApplication::BLANK;
    _flightAppRule->fltSame() = FlightApplication::BLANK;
    _flightAppRule->overrideDateTblItemNo() = 0;
    _flightAppRule->geoAppl() = FlightApplication::CONDITIONAL;
    _flightAppRule->locAppl() = FlightApplication::BETWEEN_AND;
    _flightAppRule->viaInd() = FlightApplication::BLANK;
    _flightAppRule->inOutInd() = dir;
    _flightAppRule->geoTblItemNoBetwVia() = 0;
    _flightAppRule->geoTblItemNoAndVia() = 0;
    _flightAppRule->geoTblItemNoVia() = 0;
    _flightAppRule->vendor() = "ATP";
    _flightAppRule->fltAppl() = fltAppl;
  }

  void commonRTWCreateFlightApplication()
  {
    _memHandle.get(_flightApplication);
    const VendorCode vendor = "ATP";
    _flightApplication->isForCppUnitTest() = true;
    _flightApplication->initialize(_flightAppRule, false, vendor, _trx);

    _memHandle.get(_pricingUnit);
  }

  void commonRTWCreateMockTable() { _memHandle.get(_table986); }

  void commonRTWaddToTable986(const CarrierCode marketingCxr,
                              const CarrierCode operatingCxr,
                              const FlightNumber flt1,
                              const FlightNumber flt2)
  {
    CarrierFlightSeg* table1item = _memHandle.create<CarrierFlightSeg>();
    table1item->marketingCarrier() = marketingCxr;
    table1item->operatingCarrier() = operatingCxr;
    table1item->flt1() = flt1;
    table1item->flt2() = flt2;
    _table986->segs.push_back(table1item);
  }

  void commonRTWAddTableToFlightAppRules()
  {
    _flightAppRule->carrierFltTblItemNo1() = MockTable986::putTable(_table986);
  }

  void commonRTWCreateFareMarket() { _fareMarket = _memHandle.create<FareMarket>(); }

  void commonRTWAddAirSegmentToFareMarket(const CarrierCode carrier,
                                          const CarrierCode operatingCarrier,
                                          const FlightNumber flightNumber,
                                          const LocCode origin,
                                          const LocCode destination)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(operatingCarrier);
    airSeg->flightNumber() = flightNumber;
    airSeg->origAirport() = origin;
    airSeg->destAirport() = destination;
    airSeg->origin() =
        _flightApplication->getDH().getLoc(origin, _flightApplication->getDH().ticketDate());
    airSeg->destination() =
        _flightApplication->getDH().getLoc(destination, _flightApplication->getDH().ticketDate());

    _fareMarket->travelSeg().push_back(airSeg);
    _fareMarket->origin() = _fareMarket->travelSeg().front()->origin();
    _fareMarket->destination() = _fareMarket->travelSeg().back()->destination();
  }

  void commonRTWCreatePTF()
  {
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fareInfo = _memHandle.create<FareInfo>();
    _fare = _memHandle.create<Fare>();

    _fareInfo->vendor() = "ATP";
    _fare->initialize(Fare::FS_International, _fareInfo, *_fareMarket, 0);
    _paxTypeFare->initialize(_fare, 0, _fareMarket);
  }

  void commonRTWAddAirSegment(const CarrierCode carrier,
                              const FlightNumber flightNumber,
                              const LocCode origin,
                              const LocCode destination,
                              const bool isInBound)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = carrier;
    airSeg->flightNumber() = flightNumber;
    airSeg->origAirport() = origin;
    airSeg->destAirport() = destination;
    airSeg->origin() =
        _flightApplication->getDH().getLoc(origin, _flightApplication->getDH().ticketDate());
    airSeg->destination() =
        _flightApplication->getDH().getLoc(destination, _flightApplication->getDH().ticketDate());

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(airSeg);
    fareMarket->origin() = airSeg->origin();
    fareMarket->destination() = airSeg->destination();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, 0);

    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    paxTypeFare->initialize(fare, 0, fareMarket);

    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->inbound() = isInBound;
    fareUsage->paxTypeFare() = paxTypeFare;

    _pricingUnit->fareUsage().push_back(fareUsage);
  }

  void commonRTWAddPaxTypeFare(const CarrierCode carrier,
                               const FlightNumber flightNumber,
                               const LocCode origin,
                               const LocCode destination,
                               const bool isInBound)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->carrier() = carrier;
    airSeg->flightNumber() = flightNumber;
    airSeg->origAirport() = origin;
    airSeg->destAirport() = destination;
    airSeg->origin() =
        _flightApplication->getDH().getLoc(origin, _flightApplication->getDH().ticketDate());
    airSeg->destination() =
        _flightApplication->getDH().getLoc(destination, _flightApplication->getDH().ticketDate());

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg().push_back(airSeg);
    fareMarket->origin() = airSeg->origin();
    fareMarket->destination() = airSeg->destination();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, 0);

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->initialize(fare, 0, fareMarket);
  }

  void testRTW1()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(INBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW2()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW3()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(IN_XOR_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW4()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(IN_OR_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW5()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(IN_AND_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW6()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    commonRTW(NOT_APPLY, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddAirSegment("MH", 608, "SIN", "KUL", false);
    commonRTWAddAirSegment("MH", 778, "KUL", "NRT", false);
    commonRTWAddAirSegment("MH", 608, "NRT", "KUL", true);
    commonRTWAddAirSegment("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 =
        _flightApplication->process(*_trx, *_pricingUnit, *(_pricingUnit->fareUsage().front()));

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW7()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 = _flightApplication->process(*_paxTypeFare, *_trx);

    commonRTW(INBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 = _flightApplication->process(*_paxTypeFare, *_trx);

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW8()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 = _flightApplication->process(*_paxTypeFare, *_trx);

    commonRTW(OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 = _flightApplication->process(*_paxTypeFare, *_trx);

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW9()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 = _flightApplication->process(*_paxTypeFare, *_trx);

    commonRTW(IN_XOR_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 = _flightApplication->process(*_paxTypeFare, *_trx);

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW10()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 = _flightApplication->process(*_paxTypeFare, *_trx);

    commonRTW(IN_OR_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 = _flightApplication->process(*_paxTypeFare, *_trx);

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testRTW11()
  {
    // When request type is RTW, direction indicator should be bypassed and treated as NOT APPLY

    commonRTW(NOT_APPLY, FlightApplication::MUST, false);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result1 = _flightApplication->process(*_paxTypeFare, *_trx);

    commonRTW(IN_AND_OUTBOUND, FlightApplication::MUST, true);
    commonRTWCreateFlightApplication();
    commonRTWAddPaxTypeFare("MH", 608, "SIN", "KUL", false);
    commonRTWAddPaxTypeFare("MH", 778, "KUL", "NRT", false);
    commonRTWAddPaxTypeFare("MH", 608, "NRT", "KUL", true);
    commonRTWAddPaxTypeFare("MH", 9778, "KUL", "NRT", true);

    Record3ReturnTypes result2 = _flightApplication->process(*_paxTypeFare, *_trx);

    CPPUNIT_ASSERT_EQUAL(result1, result2);
  }

  void testNegateReturnValue_PASS()
  {
    CPPUNIT_ASSERT_EQUAL(PASS,      FlightApplication::negateReturnValue(FAIL));
  }

  void testNegateReturnValue_FAIL()
  {
    CPPUNIT_ASSERT_EQUAL(FAIL,      FlightApplication::negateReturnValue(PASS));
  }

  void testNegateReturnValue_SOFTPASS()
  {
    CPPUNIT_ASSERT_EQUAL(SOFTPASS,  FlightApplication::negateReturnValue(SOFTPASS));
  }

  void testNegateReturnValue_SKIP()
  {
    CPPUNIT_ASSERT_EQUAL(SKIP,      FlightApplication::negateReturnValue(SKIP));
  }

  void testNegateReturnValue_STOP()
  {
    CPPUNIT_ASSERT_EQUAL(STOP,      FlightApplication::negateReturnValue(STOP));
  }

  void openSegOpenseg_must_Pass_EQPwithFlight_WQ_APO40449()
  {
    FlightAppRule record3;
    PricingRequest* request = _memHandle.create<PricingRequest>();
    NoPNRPricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(request);
    record3.carrierFltTblItemNo1() = 0;
    record3.carrierFltTblItemNo2() = 0;
    record3.flt1() = 0;
    record3.flt2() = 0;
    record3.flt3() = 0;
    record3.equipAppl() = FlightApplication::BLANK;
    record3.fltNonStop() = FlightApplication::BLANK;
    record3.fltDirect() = FlightApplication::BLANK;
    record3.fltMultiStop() = FlightApplication::BLANK;
    record3.fltOneStop() = FlightApplication::BLANK;
    record3.fltOnline() = FlightApplication::BLANK;
    record3.fltInterline() = FlightApplication::BLANK;
    record3.fltSame() = FlightApplication::BLANK;
    record3.overrideDateTblItemNo() = 0;
    record3.geoAppl() = FlightApplication::BLANK;
    record3.locAppl() = FlightApplication::BLANK;
    record3.viaInd() = FlightApplication::BLANK;
    record3.inOutInd() = FlightApplication::BLANK;
    record3.geoTblItemNoBetwVia() = 0;
    record3.geoTblItemNoAndVia() = 0;
    record3.geoTblItemNoVia() = 0;
    record3.vendor() = "ATP";
    record3.fltAppl() = FlightApplication::MUST;
    record3.flt1() = 100;
    record3.equipType() = "TRN";
    record3.equipAppl() = 0;
    MockTable986 table1;
    CarrierFlightSeg* table1item0 = _memHandle.create<CarrierFlightSeg>();
    table1item0->marketingCarrier() = "SR";
    table1item0->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item0->flt1() = RuleConst::ANY_FLIGHT;
    table1item0->flt2() = 0;
    table1.segs.push_back(table1item0);

    CarrierFlightSeg* table1item1 = _memHandle.create<CarrierFlightSeg>();
    table1item1->marketingCarrier() = "SN";
    table1item1->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item1->flt1() = RuleConst::ANY_FLIGHT;
    table1item1->flt2() = 0;
    table1.segs.push_back(table1item1);

    CarrierFlightSeg* table1item2 = _memHandle.create<CarrierFlightSeg>();
    table1item2->marketingCarrier() = "OS";
    table1item2->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item2->flt1() = RuleConst::ANY_FLIGHT;
    table1item2->flt2() = 0;
    table1.segs.push_back(table1item2);

    CarrierFlightSeg* table1item3 = _memHandle.create<CarrierFlightSeg>();
    table1item3->marketingCarrier() = "LX";
    table1item3->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item3->flt1() = RuleConst::ANY_FLIGHT;
    table1item3->flt2() = 0;
    table1.segs.push_back(table1item3);

    CarrierFlightSeg* table1item4 = _memHandle.create<CarrierFlightSeg>();
    table1item4->marketingCarrier() = "DL";
    table1item4->operatingCarrier() = FlightApplication::ANY_CARRIER;
    table1item4->flt1() = RuleConst::ANY_FLIGHT;
    table1item4->flt2() = 0;
    table1.segs.push_back(table1item4);

    record3.carrierFltTblItemNo1() = MockTable986::putTable(&table1);
    Record3ReturnTypes result;
    FlightApplication validator;
    validator.isForCppUnitTest() = true;

    const VendorCode vendor = "ATP";

    validator.initialize(&record3, false, vendor, trx);

#ifdef DEBUG
    validator.printTree();
#endif

    // ------
    std::vector<TravelSeg*> itin0;
    AirSeg* airSeg0 = 0;
    validator.getDH().get(airSeg0);
    airSeg0->carrier() = "DL";

    const Loc* airSeg0Orig = validator.getDH().getLoc("MSP", validator.getDH().ticketDate());
    airSeg0->origin() = airSeg0Orig;

    const Loc* airSeg0Dest = validator.getDH().getLoc("LON", validator.getDH().ticketDate());
    airSeg0->destination() = airSeg0Dest;

    airSeg0->segmentType() = Open;

    itin0.push_back(airSeg0);

    PaxTypeFare ptf0;
    FareMarket fm0;
    fm0.travelSeg().insert(fm0.travelSeg().end(), itin0.begin(), itin0.end());
    fm0.origin() = itin0.front()->origin();
    fm0.destination() = itin0.back()->destination();
    FareInfo fareInfo0;
    fareInfo0.vendor() = "ATP";
    Fare fare0;
    fare0.initialize(Fare::FS_International, &fareInfo0, fm0, 0);
    ptf0.initialize(&fare0, 0, &fm0);
    result = validator.process(ptf0, *trx);
    CPPUNIT_ASSERT_EQUAL(PASS, result);
  }

 protected:
  class FlightApplicationDataHandle : public DataHandleMock
  {
    TestMemHandle& _memHandle;

  public:
    explicit FlightApplicationDataHandle(TestMemHandle& dh) : _memHandle(dh) {}

    const std::vector<AirlineAllianceCarrierInfo*>&
    getAirlineAllianceCarrier(const CarrierCode& carrierCode)
    {
      std::vector<AirlineAllianceCarrierInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();

      if (carrierCode == "LH" || carrierCode == "LO")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*A";

        ret->push_back(cxrInfo);
        return *ret;
      }

      if (carrierCode == "AA" || carrierCode == "AB")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*O";

        ret->push_back(cxrInfo);
        return *ret;
      }

      if (carrierCode == "AF" || carrierCode == "AM")
      {
        AirlineAllianceCarrierInfo* cxrInfo = _memHandle.create<AirlineAllianceCarrierInfo>();
        cxrInfo->genericAllianceCode() = "*S";

        ret->push_back(cxrInfo);
        return *ret;
      }

      return *ret;
    }

    const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber)
    {
      std::vector<GeoRuleItem*>& ret = *_memHandle.create<std::vector<GeoRuleItem*> >();
      ret.push_back(_memHandle.create<GeoRuleItem>());
      if (itemNumber == 1)
      {
        ret.front()->loc1().locType() = 'P';
        ret.front()->loc1().loc() = "JFK";
        return ret;
      }
      else if (itemNumber == 122)
      {
        ret.front()->loc1().locType() = 'N';
        ret.front()->loc1().loc() = "US";
        return ret;
      }
      else if (itemNumber == 533)
      {
        ret.front()->loc1().locType() = 'N';
        ret.front()->loc1().loc() = "JP";
        return ret;
      }
      else if (itemNumber == 50)
      {
        ret.front()->loc1().locType() = 'C';
        ret.front()->loc1().loc() = "CHI";
        return ret;
      }
      else if (itemNumber == 55)
      {
        ret.front()->loc1().locType() = 'C';
        ret.front()->loc1().loc() = "MIA";
        return ret;
      }
      else if (itemNumber == 622)
      {
        ret.front()->loc1().locType() = 'C';
        ret.front()->loc1().loc() = "KUL";
        return ret;
      }
      else if (itemNumber == 3036)
      {
        ret.front()->loc1().locType() = 'Z';
        ret.front()->loc1().loc() = "00310";
        return ret;
      }
      else if (itemNumber == 4)
      {
        ret.front()->tsi() = 5;
        return ret;
      }
      return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
    }

    const TSIInfo* getTSI(int key)
    {
      if (key == 5)
      {
        TSIInfo* ret = _memHandle.create<TSIInfo>();
        ret->tsi() = key;
        ret->geoRequired() = ' ';
        ret->geoNotType() = ' ';
        ret->geoOut() = ' ';
        ret->geoItinPart() = ' ';
        ret->geoCheck() = 'D';
        ret->loopDirection() = 'B';
        ret->loopOffset() = 0;
        ret->loopToSet() = 1;
        ret->loopMatch() = 'F';
        ret->scope() = 'A';
        ret->type() = 'O';
        ret->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)'S');
        return ret;
      }
      return DataHandleMock::getTSI(key);
    }

    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "MSP")
        return "MSP";
      else if (locCode == "JFK")
        return "NYC";
      else if (locCode == "CHI")
        return "CHI";
      else if (locCode == "SIN")
        return "SIN";
      else if (locCode == "PEK")
        return "PEK";
      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }

    CarrierFlight* getCarrierFlight(const VendorCode& vendor, int itemNo)
    {
      CarrierFlightSeg* carrierFlightSeg = new CarrierFlightSeg; // it is component of CarrierFlight
      carrierFlightSeg->flt1() = 1;
      carrierFlightSeg->flt2() = 8999;
      carrierFlightSeg->marketingCarrier() = "MH";

      CarrierFlight* carrierFlight = _memHandle.create<CarrierFlight>();
      carrierFlight->segs().push_back(carrierFlightSeg);
      return carrierFlight;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FlightApplicationTest);

} // tse
