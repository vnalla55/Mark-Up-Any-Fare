//----------------------------------------------------------------------------
//	File: FDMaxStayApplicationTest.cpp
//
//	Author			: Partha Kumar Chakraborti
//  Created			:      03/30/2005
//  Description	:  This is a unit test class for FDMaxStayApplication.cpp
//
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

#include "Rules/FDMaxStayApplication.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/FareDisplayUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/MinStayRestriction.h"
#include "DataModel/FareDisplayOptions.h"

using namespace boost;

namespace tse
{
class FDMaxStayApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDMaxStayApplicationTest);
  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_FailDate);
  CPPUNIT_TEST(testValidate_FailNoFDTrx);
  CPPUNIT_TEST(testValidate_SkipNoRule);
  CPPUNIT_TEST(testValidate_SkipNoMaxRestriction);
  CPPUNIT_TEST(testValidate_FailExcludeNoOneYeear);
  CPPUNIT_TEST(testValidate_PassExcludeOneYeear);
  CPPUNIT_TEST(testValidate_FailNoFareDisplayInfo);
  CPPUNIT_TEST(testValidate_PassNoReturnDate);
  CPPUNIT_TEST(testValidate_NotProcessedDataUnavail);
  CPPUNIT_TEST(testValidate_NotProcessedNoValidateRules);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdMaxStayApplication = _memHandle.create<FDMaxStayApplication>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _itin = _memHandle.create<Itin>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    _fareMarket = _memHandle.create<FareMarket>();
    _fareDisplayRequest = _memHandle.create<FareDisplayRequest>();
    _maxStayRestrction = _memHandle.create<MaxStayRestriction>();
    _fdOptions = _memHandle.create<FareDisplayOptions>();
    _fareDisplayInfo = _memHandle.create<FareDisplayInfo>();
    _aSeg = _memHandle.create<AirSeg>();

    _fdTrx->setRequest(_fareDisplayRequest);
    _fdTrx->setOptions(_fdOptions);
    _paxTypeFare->setFare(fare);
    fare->setFareInfo(fi);
    fi->fareClass() = "FARECLSS";
    _paxTypeFare->fareDisplayInfo() = _fareDisplayInfo;
    _fareDisplayRequest->returnDate() = DateTime(2010, 7, 15);
    _fareMarket->direction() = FMDirection::OUTBOUND;
    _fareMarket->travelSeg().push_back(_aSeg);
    fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _aSeg->latestDepartureDT() = DateTime(2010, 7, 1);
    _maxStayRestrction->maxStay() = "1";
    _maxStayRestrction->maxStayUnit() = "M";
  }
  void tearDown() { _memHandle.clear(); }

  void testValidate_Pass()
  {
    CPPUNIT_ASSERT_EQUAL(
        PASS,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_FailDate()
  {
    _fareDisplayRequest->returnDate() = DateTime(2010, 8, 15);
    CPPUNIT_ASSERT_EQUAL(
        FAIL,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_FailNoFDTrx()
  {
    PricingTrx pricingTrx1;
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _fdMaxStayApplication->validate(
                             pricingTrx1, *_itin, *_paxTypeFare, NULL, *_fareMarket, false));
  }
  void testValidate_SkipNoRule()
  {
    CPPUNIT_ASSERT_EQUAL(
        SKIP,
        _fdMaxStayApplication->validate(*_fdTrx, *_itin, *_paxTypeFare, NULL, *_fareMarket, false));
  }
  void testValidate_SkipNoMaxRestriction()
  {
    MinStayRestriction minRest;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         _fdMaxStayApplication->validate(
                             *_fdTrx, *_itin, *_paxTypeFare, &minRest, *_fareMarket, false));
  }
  void testValidate_FailExcludeNoOneYeear()
  {
    _fdOptions->excludeMinMaxStayFares() = 'Y';
    CPPUNIT_ASSERT_EQUAL(
        FAIL,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_PassExcludeOneYeear()
  {
    _fdOptions->excludeMinMaxStayFares() = 'Y';
    _maxStayRestrction->maxStay() = "12";
    CPPUNIT_ASSERT_EQUAL(
        PASS,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_FailNoFareDisplayInfo()
  {
    _paxTypeFare->fareDisplayInfo() = 0;
    CPPUNIT_ASSERT_EQUAL(
        FAIL,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_PassNoReturnDate()
  {
    _fareDisplayRequest->returnDate() = DateTime::emptyDate();
    CPPUNIT_ASSERT_EQUAL(
        PASS,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_NotProcessedDataUnavail()
  {
    _maxStayRestrction->unavailTag() = 'X';
    CPPUNIT_ASSERT_EQUAL(
        NOTPROCESSED,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }
  void testValidate_NotProcessedNoValidateRules()
  {
    _fdOptions->validateRules() = 'N';
    CPPUNIT_ASSERT_EQUAL(
        NOTPROCESSED,
        _fdMaxStayApplication->validate(
            *_fdTrx, *_itin, *_paxTypeFare, _maxStayRestrction, *_fareMarket, false));
  }

private:
  TestMemHandle _memHandle;
  FDMaxStayApplication* _fdMaxStayApplication;
  FareDisplayTrx* _fdTrx;
  Itin* _itin;
  PaxTypeFare* _paxTypeFare;
  FareMarket* _fareMarket;
  FareDisplayRequest* _fareDisplayRequest;
  MaxStayRestriction* _maxStayRestrction;
  FareDisplayOptions* _fdOptions;
  FareDisplayInfo* _fareDisplayInfo;
  AirSeg* _aSeg;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FDMaxStayApplicationTest);
}
