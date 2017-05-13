//-----------------------------------------------------------------------------
//
//  File:     BookingCodeExceptionValidatorTest.cpp
//            **This is NOT a C++ Exception **
//
//  Author :  Kul Shekhar
//            Linda Dillahunty
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#include <string>
#include <time.h>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/CabinType.h"
#include "BookingCode/test/BookingCodeExceptionValidatorTest.h"
#include "BookingCode/BookingCodeExceptionValidator.h"
#include "Diagnostic/Diag405Collector.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxType.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DataModel/Itin.h"
#include "Common/ClassOfService.h"
#include "DataModel/FarePath.h"
#include "Common/LocUtil.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/CarrierPreference.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Billing.h"
#include "Common/AltPricingUtil.h"
#include "Common/StopWatch.h"
#include "DataModel/Itin.h"
#include "test/include/TestConfigInitializer.h"
#include "Common/Config/ConfigMan.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;
using namespace tse;

const BookingCode BookingCodeExceptionValidatorTest::AVAILABLE_BOOKING_CODE1 = "Y";
const BookingCode BookingCodeExceptionValidatorTest::AVAILABLE_BOOKING_CODE2 = "F";
const BookingCode BookingCodeExceptionValidatorTest::BOOKING_CODE1_NOT_AVAILABLE = "A";
const BookingCode BookingCodeExceptionValidatorTest::BOOKING_CODE2_NOT_AVAILABLE = "B";
const BookingCode BookingCodeExceptionValidatorTest::BOOKED_BOOKING_CODE = "Q";

CPPUNIT_TEST_SUITE_REGISTRATION(BookingCodeExceptionValidatorTest);

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _trx = _memHandle.create<PricingTrx>();
  _itin = _memHandle.create<Itin>();
  _request = _memHandle.create<PricingRequest>();

  _pop = _memHandle.create<PricingOptions>();
  _trx->setOptions(_pop);

  _agent = _memHandle.create<Agent>();
  _cust = _memHandle.create<Customer>();
  _agent->agentTJR() = _cust;
  _request->ticketingAgent() = _agent;

  _billing = _memHandle.create<Billing>();
  _trx->setRequest(_request);
  _trx->billing() = _billing;

  _bceSegment = new BookingCodeExceptionSegment;
  _airSeg = _memHandle.create<AirSeg>();
  initializeTvlSegAvail();
  _paxTypeFare = _memHandle.create<PaxTypeFare>();
  _fareInfo = _memHandle.create<FareInfo>();
  _cxrPref = _memHandle.create<CarrierPreference>();
  initializePaxTypeFare();

  _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
  _trx->fareCalcConfig()->noMatchAvail() = 'N';

  _paxType = _memHandle.create<PaxType>();
  _pInfo = _memHandle.create<PaxTypeInfo>();
  _paxType->paxTypeInfo() = _pInfo;
  _trx->paxType().push_back(_paxType);
  setSegStatusVec();
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::tearDown()
{
  _memHandle.clear();
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testTagFlightNOMATCH()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  bceVal._fltResultVector.resize(1);
  int airIndex = 0;

  bceVal.tagFltNOMATCH(_segStatVec, airIndex);
  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED));
  CPPUNIT_ASSERT(bceVal._fltResultVector[0].first ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  CPPUNIT_ASSERT(bceVal._fltResultVector[0].second ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
}

//----------------------------------------------
void
BookingCodeExceptionValidatorTest::testTagFlightFail()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._fltResultVector.resize(1);
  int airIndex = 0;

  bceVal.tagFltFAIL(_segStatVec, airIndex, BOOKING_CODE_NOT_OFFERED);
  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED));
  CPPUNIT_ASSERT(bceVal._fltResultVector[0].second ==
                 BookingCodeExceptionValidator::BCE_FLT_FAILED);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testCheckBookedClassAvailWhenEntryNotWPNC()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  // TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  setRequestAsWP();
  CPPUNIT_ASSERT(!bceVal.checkBookedClassAvail(*_trx, 0));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testCheckBookedClassAvailWhenUpSellEntry()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  // TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  setRequestAsWPNC();
  _request->upSellEntry() = true;
  CPPUNIT_ASSERT(bceVal.checkBookedClassAvail(*_trx, 0));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testCheckBookedClassAvailWhenMipTrx()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  // TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  setRequestAsWPNC();
  _request->upSellEntry() = false;
  setTrxAsMip();
  _billing->actionCode() = "JR.";
  CPPUNIT_ASSERT(bceVal.checkBookedClassAvail(*_trx, 0));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testCheckBookedClassAvailWhenMipTrxWPNI()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  // TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  setRequestAsWPNC();
  _request->upSellEntry() = false;
  setTrxAsMip();
  _billing->actionCode() = "WPNI.C";
  CPPUNIT_ASSERT(!bceVal.checkBookedClassAvail(*_trx, 0));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPsizeProblem()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  CPPUNIT_ASSERT(
      !bceVal.validateWP(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPPass()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  setRequestAsWP();

  uint16_t airIndex = 0;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  tvlSeg->setBookingCode(AVAILABLE_BOOKING_CODE1);
  _bceSegment->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  CPPUNIT_ASSERT(
      bceVal.validateWP(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));

  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPFail()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  setRequestAsWP();

  uint16_t airIndex = 0;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  tvlSeg->setBookingCode(AVAILABLE_BOOKING_CODE1);
  _bceSegment->bookingCode1() = BOOKING_CODE1_NOT_AVAILABLE;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  CPPUNIT_ASSERT(
      !bceVal.validateWP(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testGetAvailabilityIfNoCarrierPref()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, tvlSeg, _segStatVec[0], *_paxTypeFare, 0) == 0);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testGetAvailabilityForSolo()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = YES;

  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, tvlSeg, _segStatVec[0], *_paxTypeFare, 0) == 0);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testGetAvailabilityValid()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = NO;

  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, tvlSeg, _segStatVec[0], *_paxTypeFare, 0) ==
                 &(_airSeg->classOfService()));
  // CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, tvlSeg, _segStatVec[0], *_paxTypeFare) == 0);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPNCFailSizeProblem()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  //_airSeg->carrierPref() = _cxrPref;
  //_cxrPref->availabilityApplyrul2st() = NO;

  uint16_t airIndex = 2;
  CPPUNIT_ASSERT_MESSAGE("will only fail if airIndex too large", airIndex >= _segStatVec.size());

  CPPUNIT_ASSERT(
      !bceVal.validateWPNC(*_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, *_trx));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPNCFailBookingCode()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = NO;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  tvlSeg->carrierPref() = const_cast<CarrierPreference*>(_cxrPref);

  uint16_t airIndex = 0;

  _bceSegment->bookingCode1() = BOOKING_CODE1_NOT_AVAILABLE;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  CPPUNIT_ASSERT(
      !bceVal.validateWPNC(*_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, *_trx));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPNCPassBookingCode()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = NO;

  uint16_t airIndex = 0;

  tvlSeg->setBookingCode(BOOKED_BOOKING_CODE);
  tvlSeg->bookedCabin().setEconomyClass();
  _bceSegment->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  CPPUNIT_ASSERT(
      bceVal.validateWPNC(*_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, *_trx));

  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED));
  CPPUNIT_ASSERT(!_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH));
  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeReBook == AVAILABLE_BOOKING_CODE1);
  CPPUNIT_ASSERT(_segStatVec[0]._reBookCabin.isEconomyClass());
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPNCFailNumSeats()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = NO;

  uint16_t airIndex = 0;

  tvlSeg->setBookingCode(BOOKED_BOOKING_CODE);
  tvlSeg->bookedCabin().setEconomyClass();
  _bceSegment->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  _pInfo->numberSeatsReq() = 1;
  int tooManySeatsRequested = 3;
  _paxType->number() = tooManySeatsRequested;

  CPPUNIT_ASSERT(
      !bceVal.validateWPNC(*_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, *_trx));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateWPNCPassIgnoreAvail()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = NO;

  uint16_t airIndex = 0;

  tvlSeg->setBookingCode(BOOKED_BOOKING_CODE);
  tvlSeg->bookedCabin().setEconomyClass();
  _bceSegment->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  _bceSegment->bookingCode2() = BOOKING_CODE2_NOT_AVAILABLE;

  _pInfo->numberSeatsReq() = 1;
  int tooManySeatsRequested = 3;
  _paxType->number() = tooManySeatsRequested;
  _trx->fareCalcConfig()->noMatchAvail() = 'Y';

  CPPUNIT_ASSERT(
      bceVal.validateWPNC(*_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, *_trx));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateBCForWPReturnTrue()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWP();
  bceValStub._validateShouldReturnWP = true;
  bceValStub._validateShouldReturnWPNC = false;
  bceValStub._lastAirIndex = 0;

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(
      bceValStub.validateBC(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateBCForWPReturnFalseRule1()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWP();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;
  bceValStub._validateShouldReturnWP = false;
  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(
      !bceValStub.validateBC(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateBCForWPReturnFalseRule2AsBooked()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWP();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1_AS_BOOKED;

  bceValStub._validateShouldReturnWP = false;
  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(
      !bceValStub.validateBC(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateBCForWPNCReturnFalse()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWP = false;
  bceValStub._validateShouldReturnWPNC = false;
  bceValStub._lastAirIndex = 0;

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(
      !bceValStub.validateBC(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testValidateBCForWPNCReturnTrue()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWP = false;
  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  uint16_t airIndex = 2;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  CPPUNIT_ASSERT(
      bceValStub.validateBC(*_trx, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare));
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagP()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;

  uint16_t airIndex = 0;
  int16_t iFlt = 0;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed == BookingCodeExceptionValidator::BCE_PERMITTED);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagR()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED;

  uint16_t airIndex = 0;
  int16_t iFlt = 0;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed == BookingCodeExceptionValidator::BCE_REQUIRED);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagB_RBD1_NotAvail()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;
  bceValStub._fltResultVector.resize(1);

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;

  uint16_t airIndex = 0;
  int16_t iFlt = -1;
  uint32_t itemNo = 1;
  PaxTypeFare* pfare = createPTF("ADT", "AA");
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  tvlSeg->setBookingCode("Y");
  _bceSegment->bookingCode1() = "B";
  _bceSegment->bookingCode2() = "Y";
  pfare->fareMarket()->travelSeg().push_back(tvlSeg);

  pfare->fareMarket()->classOfServiceVec().push_back(&(tvlSeg->classOfService()));

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *pfare, iFlt);

  CPPUNIT_ASSERT(bceValStub._fltResultVector[airIndex].first == -1 ); //BCE_FLT_NOMATCH
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagB_RBD1_Avail_RBD2_Match()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;
  bceValStub._fltResultVector.resize(1);

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;

  uint16_t airIndex = 0;
  int16_t iFlt = -1;
  uint32_t itemNo = 1;
  PaxTypeFare* pfare = createPTF("ADT", "AA");
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  tvlSeg->setBookingCode("F");
  _bceSegment->bookingCode1() = "Y";
  _bceSegment->bookingCode2() = "F";
  pfare->fareMarket()->travelSeg().push_back(tvlSeg);

  pfare->fareMarket()->classOfServiceVec().push_back(&(tvlSeg->classOfService()));

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *pfare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed =
                 BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagD()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;

  uint16_t airIndex = 0;
  int16_t iFlt = -1;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  PaxTypeFare* pfare = createPTF("ADT", "AA");
  pfare->fareMarket()->travelSeg().push_back(tvlSeg);

  pfare->fareMarket()->classOfServiceVec().push_back(&(tvlSeg->classOfService()));

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *pfare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed ==
                 BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE);
}
//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagD_R1T999Cat23R3()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._rec1T999Cat25R3 = true; // T999 from the Cat25 Rec3
  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;

  uint16_t airIndex = 0;
  int16_t iFlt = -1;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  PaxTypeFare* pfare = createPTF("ADT", "AA");
  pfare->fareMarket()->travelSeg().push_back(tvlSeg);

  pfare->fareMarket()->classOfServiceVec().push_back(&(tvlSeg->classOfService()));

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *pfare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed !=
                 BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE);
}
//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagO()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWPNC();
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWPNC = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_OFFER;

  uint16_t airIndex = 0;
  int16_t iFlt = 0;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed ==
                 BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_OFFER);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::testProcessRestrictionTagOasTagPforignoreAvail()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  setRequestAsWP();
  _trx->fareCalcConfig()->noMatchAvail() = 'Y';
  bceValStub._statusType = BookingCodeExceptionValidator::STATUS_RULE1;

  bceValStub._validateShouldReturnWP = true;
  bceValStub._lastAirIndex = 0;

  _bceSegment->restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_OFFER;

  uint16_t airIndex = 0;
  int16_t iFlt = 0;
  uint32_t itemNo = 1;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;

  bceValStub.processRestrictionTag(
      *_trx, itemNo, *_bceSegment, tvlSeg, _segStatVec, airIndex, *_paxTypeFare, iFlt);

  CPPUNIT_ASSERT(bceValStub._restTagProcessed == BookingCodeExceptionValidator::BCE_PERMITTED);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setRequestAsWP()
{
  _request->lowFareRequested() = 'N';
  return;
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setRequestAsWPNC()
{
  _request->lowFareRequested() = 'Y';
  return;
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setTrxAsMip()
{
  _trx->setTrxType(PricingTrx::MIP_TRX);
  return;
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setTrxAsPricing()
{
  _trx->setTrxType(PricingTrx::PRICING_TRX);
  return;
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::initializePaxTypeFare()
{
  Fare fare;
  TariffCrossRefInfo tariffRefInfo;
  FareMarket fareMarket;
  // pCxrPref->availabilityApplyrul2st() = YES;
  fareMarket.governingCarrierPref() = _cxrPref;

  fare.initialize(Fare::FS_International, _fareInfo, fareMarket, &tariffRefInfo);
  PaxType paxType;
  PaxTypeCode paxTypeCode = "ADT";
  paxType.paxType() = paxTypeCode;
  _paxTypeFare->initialize(&fare, &paxType, &fareMarket);
  return;
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::initializeTvlSegAvail()
{
  ClassOfService* pCos = _memHandle.create<ClassOfService>();
  pCos->bookingCode() = AVAILABLE_BOOKING_CODE1;
  pCos->numSeats() = 1;
  pCos->cabin().setEconomyClass();

  _airSeg->classOfService().push_back(pCos);

  pCos = _memHandle.create<ClassOfService>();
  pCos->bookingCode() = AVAILABLE_BOOKING_CODE2;
  pCos->numSeats() = 1;
  pCos->cabin().setFirstClass();

  _airSeg->classOfService().push_back(pCos);
}

//-----------------------------------------------
void
BookingCodeExceptionValidatorTest::setSegStatusVec()
{
  _segStatVec.clear();
  PaxTypeFare::SegmentStatus segStat;
  _segStatVec.push_back(segStat);
  return;
}

//-----------------------------------------------
BookingCodeExceptionValidatorStubValidateBC::BookingCodeExceptionValidatorStubValidateBC(
    Itin& itin, bool partOfLocalJny, bool skipFlownSegCat31)
  : BookingCodeExceptionValidator(itin, partOfLocalJny, skipFlownSegCat31, false)
{
}

//-----------------------------------------------
bool
BookingCodeExceptionValidatorStubValidateBC::validateWP(
    PricingTrx& trx,
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    const uint16_t airIndex,
    PaxTypeFare& paxTypeFare,
    bool rbd2Only)
{
  _lastAirIndex = airIndex;
  return _validateShouldReturnWP;
}

//-----------------------------------------------
bool
BookingCodeExceptionValidatorStubValidateBC::validateWPNC(
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    const uint16_t airIndex,
    PaxTypeFare& paxTypeFare,
    PricingTrx& trx,
    bool rbd2Only)
{
  _lastAirIndex = airIndex;
  return _validateShouldReturnWPNC;
}

BookingCodeValidationStatus
BookingCodeExceptionValidatorStubValidateBC::validateWPNC_NEW(
    const BookingCodeExceptionSegment& bceSegment,
    const TravelSeg* tvlSeg,
    PaxTypeFare::SegmentStatusVec& segStatVec,
    const uint16_t airIndex,
    PaxTypeFare& paxTypeFare,
    PricingTrx& trx,
    bool rbd2Only)
{
  _lastAirIndex = airIndex;
  if (_validateShouldReturnWPNC)
    return BOOKING_CODE_PASSED;
  else
    return BOOKING_CODE_NOT_OFFERED;
}

PaxTypeFare*
BookingCodeExceptionValidatorTest::createPTF(std::string paxTypeCode, std::string carrier)
{
  PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();

  Fare* fare = _memHandle.create<Fare>();
  TariffCrossRefInfo tariffRefInfo;
  FareMarket* fareMarket = _memHandle.create<FareMarket>();

  fareMarket->governingCarrierPref() = _cxrPref;

  fare->initialize(Fare::FS_International, _fareInfo, *fareMarket, &tariffRefInfo);
  PaxType* paxType = _memHandle.create<PaxType>();
  PaxTypeCode ptc = paxTypeCode;
  paxType->paxType() = ptc;

  paxTypeFare->initialize(fare, paxType, fareMarket);

  return paxTypeFare;
}

void
BookingCodeExceptionValidatorTest::testValidateCarrier1()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  PaxTypeFare* ptf = createPTF("ADT", "AA");
  Fare* fare = ptf->fare();
  AirSeg aSeg;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_DOLLARDOLLARCARRIER;
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_ANYCARRIER;
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  sequence.ifTag() = BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT;
  segment.segNo() = 1;

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_XDOLLARCARRIER;
  fare->status().set(Fare::FS_IndustryFare, true);
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  fare->status().set(Fare::FS_IndustryFare, false);
  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  segment.viaCarrier() = INDUSTRY_CARRIER;
  fare->status().set(Fare::FS_IndustryFare, true);
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  fare->status().set(Fare::FS_IndustryFare, false);
  CarrierCode cc = CarrierCode("AA");
  const_cast<CarrierCode&>(fare->carrier()) = segment.viaCarrier() = cc;
  ptf->setFare(fare);
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  segment.viaCarrier() = INDUSTRY_CARRIER;
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg) == false);

  segment.segNo() = 0;

  fare->status().set(Fare::FS_IndustryFare, true);
  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_XDOLLARCARRIER;
  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  fare->status().set(Fare::FS_IndustryFare, false);
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  aSeg.carrier() = "AA";
  segment.viaCarrier() = "AA";
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg));

  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT(bceVal.validateCarrier(sequence, segment, *ptf, aSeg) == false);
}

void
BookingCodeExceptionValidatorTest::testValidateCarrier2()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  PaxTypeFare* ptf = createPTF("ADT", "AA");
  Fare* fare = ptf->fare();
  AirSeg aSeg;
  int iFlt = 0;
  int fltIndex = 1;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_DOLLARDOLLARCARRIER;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_ANYCARRIER;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  sequence.ifTag() = BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT;
  segment.segNo() = 1;

  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_XDOLLARCARRIER;
  fare->status().set(Fare::FS_IndustryFare, true);
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  fare->status().set(Fare::FS_IndustryFare, false);
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  fltIndex = 0;
  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  iFlt = -1;
  segment.viaCarrier() = INDUSTRY_CARRIER;
  fare->status().set(Fare::FS_IndustryFare, true);
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  fare->status().set(Fare::FS_IndustryFare, false);
  CarrierCode cc = CarrierCode("AA");
  const_cast<CarrierCode&>(fare->carrier()) = segment.viaCarrier() = cc;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  segment.viaCarrier() = INDUSTRY_CARRIER;
  fare->status().set(Fare::FS_IndustryFare, true);
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  aSeg.carrier() = "AA";
  segment.viaCarrier() = "AA";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  aSeg.carrier() = "BA";
  segment.viaCarrier() = ANY_CARRIER;
  fare->status().set(Fare::FS_IndustryFare, false);
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  segment.segNo() = 0;
  segment.viaCarrier() = BookingCodeExceptionValidator::BCE_XDOLLARCARRIER;
  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  fare->status().set(Fare::FS_IndustryFare, false);
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  aSeg.carrier() = "AA";
  segment.viaCarrier() = "AA";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));

  aSeg.carrier() = "BA";
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT,
                       bceVal.validateCarrier(sequence, segment, *fare, aSeg, iFlt, fltIndex));
}

void
BookingCodeExceptionValidatorTest::testValidatePrimarySecondary()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;
  Fare fare;

  DataHandle dh;

  AirSeg aSeg1;
  AirSeg aSeg2;

  TravelSeg* primarySector = &aSeg1;
  TravelSeg* travelSeg = &aSeg2;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_CHAR_BLANK;
  CPPUNIT_ASSERT_EQUAL(
      BCE_PASS, bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  fare.status().set(Fare::FS_Domestic, true);
  fare.status().set(Fare::FS_Transborder, true);
  fare.status().set(Fare::FS_ForeignDomestic, true);

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(
      BCE_NEXT_SEQUENCE,
      bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_SECONDARY;
  CPPUNIT_ASSERT_EQUAL(
      BCE_NEXT_SEQUENCE,
      bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_FROMTO_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(
      BCE_NEXT_SEQUENCE,
      bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  fare.status().set(Fare::FS_Domestic, false);
  fare.status().set(Fare::FS_Transborder, false);
  fare.status().set(Fare::FS_ForeignDomestic, false);
  primarySector = travelSeg;
  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_PRIMARY;
  CPPUNIT_ASSERT(bceVal.validatePrimarySecondary(
                     sequence, segment, fare, primarySector, travelSeg) == BCE_PASS);
  CPPUNIT_ASSERT_EQUAL(
      BCE_PASS, bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_FROMTO_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(
      BCE_NEXT_FLT,
      bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  primarySector = &aSeg1;
  CPPUNIT_ASSERT_EQUAL(
      BCE_FROMTO_PRI,
      bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  segment.primarySecondary() = BookingCodeExceptionValidator::BCE_SEG_SECONDARY;
  CPPUNIT_ASSERT_EQUAL(
      BCE_PASS, bceVal.validatePrimarySecondary(sequence, segment, fare, primarySector, travelSeg));

  primarySector = travelSeg;
  CPPUNIT_ASSERT(bceVal.validatePrimarySecondary(
                     sequence, segment, fare, primarySector, travelSeg) == BCE_NEXT_FLT);
}

void
BookingCodeExceptionValidatorTest::testValidateFlights()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  AirSeg aSeg;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.flight1() = -1;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateFlights(sequence, segment, aSeg));

  segment.fltRangeAppl() = BookingCodeExceptionValidator::BCE_FLTINDIVIDUAL;
  aSeg.flightNumber() = 1;
  segment.flight1() = 2;
  segment.flight2() = 3;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateFlights(sequence, segment, aSeg));

  segment.flight1() = 1;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateFlights(sequence, segment, aSeg));

  segment.flight1() = 2;
  segment.flight2() = 1;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateFlights(sequence, segment, aSeg));

  segment.fltRangeAppl() = BookingCodeExceptionValidator::BCE_FLTRANGE;
  aSeg.flightNumber() = 2;
  segment.flight1() = 3;
  segment.flight2() = 1;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateFlights(sequence, segment, aSeg));

  aSeg.flightNumber() = 1;
  segment.flight1() = 1;
  segment.flight2() = 2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateFlights(sequence, segment, aSeg));

  segment.fltRangeAppl() = BookingCodeExceptionValidator::BCE_PRIME; // default in switch statement.
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateFlights(sequence, segment, aSeg));
}

void
BookingCodeExceptionValidatorTest::testValidateEquipment()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  AirSeg aSeg;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.equipType() = BookingCodeExceptionValidator::BCE_EQUIPBLANK;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateEquipment(sequence, segment, aSeg));

  segment.equipType() = "";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateEquipment(sequence, segment, aSeg));

  segment.equipType() = aSeg.equipmentType() = TRAIN;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateEquipment(sequence, segment, aSeg));

  aSeg.equipmentType() = BookingCodeExceptionValidator::BCE_EQUIPBLANK;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateEquipment(sequence, segment, aSeg));
}

void
BookingCodeExceptionValidatorTest::testValidatePortionOfTravel()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  AirSeg aSeg;
  Fare fare;
  FareInfo fareInfo;

  Loc loc1;
  Loc loc2;

  aSeg.origin() = &loc1;
  aSeg.destination() = &loc2;

  fare.setFareInfo(&fareInfo);

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.tvlPortion() = "AT";
  const_cast<FareInfo*>(fare.fareInfo())->globalDirection() = GlobalDirection::AT;
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA1;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA2;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "CA";
  const_cast<NationCode&>(aSeg.origin()->nation()) = CANADA;
  const_cast<NationCode&>(aSeg.destination()->nation()) = CANADA;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "CA";
  const_cast<NationCode&>(aSeg.origin()->nation()) = CANADA;
  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "CO";
  const_cast<NationCode&>(aSeg.origin()->nation()) = CANADA;
  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA1;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "CO";
  const_cast<NationCode&>(aSeg.origin()->nation()) = UNITED_STATES;
  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA1;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "DO";
  Loc loc;
  const_cast<NationCode&>(loc.nation()) = UNITED_STATES;

  aSeg.origin() = &loc;
  aSeg.destination() = &loc;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<NationCode&>(loc2.nation()) = PERU;
  aSeg.destination() = &loc2;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "EH";
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA2;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA2;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA3;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA3;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "FD";
  const_cast<NationCode&>(aSeg.origin()->nation()) = PERU;
  const_cast<NationCode&>(aSeg.origin()->nation()) = PERU;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "FE";
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA3;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA3;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "PA";
  const_cast<FareInfo*>(fare.fareInfo())->globalDirection() = GlobalDirection::PA;
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA3;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "TB";
  const_cast<NationCode&>(aSeg.origin()->nation()) = CANADA;
  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<NationCode&>(aSeg.destination()->nation()) = CANADA;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "TM";
  const_cast<NationCode&>(aSeg.origin()->nation()) = UNITED_STATES;
  const_cast<NationCode&>(aSeg.destination()->nation()) = MEXICO;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "US";
  const_cast<NationCode&>(aSeg.origin()->nation()) = UNITED_STATES;
  const_cast<NationCode&>(aSeg.destination()->nation()) = UNITED_STATES;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<NationCode&>(aSeg.destination()->nation()) = PERU;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "WH";
  const_cast<IATAAreaCode&>(aSeg.origin()->area()) = IATA_AREA1;
  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA1;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  const_cast<IATAAreaCode&>(aSeg.destination()->area()) = IATA_AREA2;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));

  segment.tvlPortion() = "";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePortionOfTravel(sequence, segment, fare, aSeg));
}

void
BookingCodeExceptionValidatorTest::testValidatePointOfSale()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  AirSeg aSeg;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.posLoc() = "";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePointOfSale(sequence, segment, *_trx, aSeg));

  segment.posLoc() = "\0";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validatePointOfSale(sequence, segment, *_trx, aSeg));
}

void
BookingCodeExceptionValidatorTest::testValidateSoldTag()
{
  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.soldInOutInd() = '1';
  _itin->intlSalesIndicator() = Itin::SITI;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));

  _itin->intlSalesIndicator() = Itin::SOTO;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateSoldTag(sequence, segment, *_trx));

  segment.soldInOutInd() = '2';
  _itin->intlSalesIndicator() = Itin::SOTO;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));

  _itin->intlSalesIndicator() = Itin::SOTI;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateSoldTag(sequence, segment, *_trx));

  segment.soldInOutInd() = '3';
  _itin->intlSalesIndicator() = Itin::SITO;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));

  _itin->intlSalesIndicator() = Itin::SOTO;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateSoldTag(sequence, segment, *_trx));

  segment.soldInOutInd() = '4';
  _itin->intlSalesIndicator() = Itin::SOTI;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));

  _itin->intlSalesIndicator() = Itin::SOTO;
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_FLT, bceVal.validateSoldTag(sequence, segment, *_trx));

  segment.soldInOutInd() = '5';
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));

  Itin* itinKeep = _itin;
  _itin = 0;
  CPPUNIT_ASSERT_EQUAL(BCE_PASS, bceVal.validateSoldTag(sequence, segment, *_trx));
  _itin = itinKeep;
}

void
BookingCodeExceptionValidatorTest::testValidateFareclassType()
{

  BookingCodeExceptionSequence sequence;
  BookingCodeExceptionSegment segment;
  PaxTypeFare paxTypeFare;
  FareClassAppInfo fareClassAppInfo;
  Fare fare;
  FareInfo fareInfo;

  fare.setFareInfo(&fareInfo);
  paxTypeFare.setFare(&fare);
  const_cast<FareClassAppInfo*&>(paxTypeFare.fareClassAppInfo()) = &fareClassAppInfo;

  AirSeg aSeg;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.fareclass() = "";
  segment.fareclassType() = 'T';
  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "A";
  const_cast<FareType&>(paxTypeFare.fcaFareType()) = "A";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  segment.fareclass() = "A";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  const_cast<FareType&>(paxTypeFare.fcaFareType()) = "B";
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  segment.fareclassType() = 'F';
  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "A";
  segment.fareclass() = "A";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "B";
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  segment.fareclassType() = 'M';
  segment.fareclass() = "A";
  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "A";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "B";
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  segment.fareclassType() = 'A';
  segment.fareclass() = "AA";
  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "AB";
  CPPUNIT_ASSERT_EQUAL(BCE_PASS,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  const_cast<FareClassCode&>(paxTypeFare.fareClass()) = "BA";
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));

  segment.fareclassType() = 'X';
  CPPUNIT_ASSERT_EQUAL(BCE_NEXT_SEQUENCE,
                       bceVal.validateFareclassType(sequence, segment, paxTypeFare, aSeg));
}

void
BookingCodeExceptionValidatorTest::testGetSegment()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  BookingCodeExceptionSequence sequence;

  BookingCodeExceptionSegment* segment1 = new BookingCodeExceptionSegment;
  BookingCodeExceptionSegment* segment2 = new BookingCodeExceptionSegment;

  segment1->segNo() = 1;
  segment2->segNo() = 2;

  sequence.segmentVector().push_back(segment1);
  sequence.segmentVector().push_back(segment2);

  CPPUNIT_ASSERT(bceVal.getSegment(sequence, 1) != 0);
  CPPUNIT_ASSERT(bceVal.getSegment(sequence, 1) != 0);
  CPPUNIT_ASSERT(bceVal.getSegment(sequence, 3) == 0);
}

void
BookingCodeExceptionValidatorTest::testAllFltsDone()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  TravelSegResult result1(BookingCodeExceptionValidator::BCE_FLT_FAILED, 0);
  TravelSegResult result2(BookingCodeExceptionValidator::BCE_FLT_FAILED, 0);

  bceVal._fltResultVector.push_back(result1);
  bceVal._fltResultVector.push_back(result2);

  CPPUNIT_ASSERT(bceVal.allFltsDone(-1));
  CPPUNIT_ASSERT(bceVal.allFltsDone(1));

  bceVal._fltResultVector[0].first = BookingCodeExceptionValidator::BCE_FLT_NOMATCH;
  CPPUNIT_ASSERT(!bceVal.allFltsDone(-1));

  bceVal._fltResultVector[0].first = BookingCodeExceptionValidator::BCE_FLT_FAILED;
  bceVal._fltResultVector[1].first = BookingCodeExceptionValidator::BCE_FLT_NOMATCH;
  CPPUNIT_ASSERT(!bceVal.allFltsDone(1));
}

void
BookingCodeExceptionValidatorTest::testResetFltResult()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  TravelSegResult result1(BookingCodeExceptionValidator::BCE_FLT_FAILED, 0);
  TravelSegResult result2(BookingCodeExceptionValidator::BCE_FLT_FAILED, 0);

  bceVal._fltResultVector.push_back(result1);
  bceVal._fltResultVector.push_back(result2);

  bceVal.resetFltResult(BookingCodeExceptionValidator::BCE_ALL_SEQUENCES, 2);

  CPPUNIT_ASSERT(bceVal._fltResultVector[0].first ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  CPPUNIT_ASSERT(bceVal._fltResultVector[0].second ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  CPPUNIT_ASSERT(bceVal._fltResultVector[1].first ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  CPPUNIT_ASSERT(bceVal._fltResultVector[1].second ==
                 BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
}

void
BookingCodeExceptionValidatorTest::testResetFirstMatchingSeqs()
{
  PaxTypeFare paxTypeFare;
  FareMarket fareMarket;

  AirSeg airSeg1;
  AirSeg airSeg2;

  fareMarket.travelSeg().push_back(&airSeg1);
  fareMarket.travelSeg().push_back(&airSeg1);

  paxTypeFare.fareMarket() = &fareMarket;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  bceVal.resetFirstMatchingSeqs(paxTypeFare);

  CPPUNIT_ASSERT(bceVal._firstMatchingSeqs.size() == 2);
  CPPUNIT_ASSERT(bceVal._firstMatchingSeqs[0] == BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  CPPUNIT_ASSERT(bceVal._firstMatchingSeqs[1] == BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
}

void
BookingCodeExceptionValidatorTest::testGetSegStatusVec()
{
  PaxTypeFare paxTypeFare;
  FareUsage fu;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  bceVal._fu = &fu;

  PaxTypeFare::SegmentStatus ss1;
  PaxTypeFare::SegmentStatus ss2;

  CabinType ct1;
  CabinType ct2;

  ct1.setFirstClass();
  ct2.setBusinessClass();

  ss1._reBookCabin = ct1;
  ss2._reBookCabin = ct2;

  fu.segmentStatus().push_back(ss1);
  fu.segmentStatus().push_back(ss2);

  paxTypeFare.segmentStatus().push_back(ss2);
  paxTypeFare.segmentStatus().push_back(ss2);

  paxTypeFare.segmentStatusRule2().push_back(ss2);
  paxTypeFare.segmentStatusRule2().push_back(ss1);

  bceVal._asBookedStatus.push_back(ss1);
  bceVal._asBookedStatus.push_back(ss1);

  std::vector<PaxTypeFare::SegmentStatus>& res1 = bceVal.getSegStatusVec(paxTypeFare);
  std::vector<PaxTypeFare::SegmentStatus>& res2 = fu.segmentStatus();
  CPPUNIT_ASSERT(res1[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res1[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._fu = 0;
  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE1;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = paxTypeFare.segmentStatus();
  CPPUNIT_ASSERT(res1[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res1[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE1_AS_BOOKED;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = bceVal._asBookedStatus;
  CPPUNIT_ASSERT(res1[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res1[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = paxTypeFare.segmentStatusRule2();
  CPPUNIT_ASSERT(res2[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res2[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2_AS_BOOKED;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = bceVal._asBookedStatus;
  CPPUNIT_ASSERT(res1[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res1[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_JORNY;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = paxTypeFare.segmentStatusRule2();
  CPPUNIT_ASSERT(res2[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res2[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_JORNY_AS_BOOKED;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = bceVal._asBookedStatus;
  CPPUNIT_ASSERT(res2[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res2[1]._reBookCabin == res2[1]._reBookCabin);

  bceVal._statusType = (BookingCodeExceptionValidator::StatusType)999;
  res1 = bceVal.getSegStatusVec(paxTypeFare);
  res2 = bceVal._asBookedStatus;
  CPPUNIT_ASSERT(res2[0]._reBookCabin == res2[0]._reBookCabin);
  CPPUNIT_ASSERT(res2[1]._reBookCabin == res2[1]._reBookCabin);
}

void
BookingCodeExceptionValidatorTest::testAnalyzeSeg()
{
  PaxTypeFare paxTypeFare;
  BookingCodeExceptionSegment segment;

  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  segment.fltRangeAppl() = BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.fltRangeAppl() = BookingCodeExceptionValidator::BCE_CHAR_BLANK;
  segment.flight1() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.flight1() = 0;
  segment.equipType() = TRAIN;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.equipType() = "";
  segment.tvlPortion() = "In";
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlPortion() = "";
  segment.posTsi() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.posTsi() = 0;
  segment.posLoc() = "KRK";
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.posLoc() = "";
  segment.posLocType() = BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.posLocType() = BookingCodeExceptionValidator::BCE_CHAR_BLANK;
  segment.soldInOutInd() = BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.soldInOutInd() = BookingCodeExceptionValidator::BCE_CHAR_BLANK;
  segment.tvlEffYear() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlEffYear() = 0;
  segment.tvlEffMonth() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlEffMonth() = 0;
  segment.tvlEffDay() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlEffDay() = 0;
  segment.tvlDiscYear() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlDiscYear() = 0;
  segment.tvlDiscMonth() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlDiscMonth() = 0;
  segment.tvlDiscDay() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlDiscDay() = 0;
  segment.daysOfWeek() = "123";
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.daysOfWeek() = "";
  segment.tvlStartTime() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlStartTime() = 0;
  segment.tvlEndTime() = 1;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.tvlEndTime() = 0;
  segment.arbZoneNo() = "1";
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.arbZoneNo() = "";
  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_STANDBY;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_NOT_PERMITTED;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_OFFER;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_AVAIL;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED_IF_PRIME_NOT_OFFER;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED_IF_PRIME_NOT_AVAIL;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_ADDITIONAL_DATA_APPLIES;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_DOES_NOT_EXIST;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED_WHEN_OFFERED;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));

  segment.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED_WHEN_AVAIL;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.analyzeSeg(paxTypeFare, segment));
}

void
BookingCodeExceptionValidatorTest::testFlowJourneyCarrier()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  _airSeg->carrierPref() = _cxrPref;
  const_cast<CarrierPreference*>(_airSeg->carrierPref())->flowMktJourneyType() = YES;

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  _trx->getOptions()->journeyActivatedForPricing() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.flowJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->getOptions()->journeyActivatedForShopping() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.flowJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
  _trx->getOptions()->applyJourneyLogic() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.flowJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  _trx->getOptions()->journeyActivatedForPricing() = true;
  _trx->getOptions()->journeyActivatedForShopping() = true;
  _trx->getOptions()->applyJourneyLogic() = true;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.flowJourneyCarrier(*_trx, _airSeg));

  const_cast<CarrierPreference*>(_airSeg->carrierPref())->flowMktJourneyType() = NO;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.flowJourneyCarrier(*_trx, _airSeg));

  CPPUNIT_ASSERT_EQUAL(false, bceVal.flowJourneyCarrier(*_trx, _airSeg));
}

void
BookingCodeExceptionValidatorTest::testLocalJourneyCarrier()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  _airSeg->carrierPref() = _cxrPref;
  const_cast<CarrierPreference*>(_airSeg->carrierPref())->localMktJourneyType() = YES;

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  _trx->getOptions()->journeyActivatedForPricing() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->getOptions()->journeyActivatedForShopping() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
  _trx->getOptions()->applyJourneyLogic() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  _trx->getOptions()->journeyActivatedForPricing() = true;
  _trx->getOptions()->journeyActivatedForShopping() = true;
  _trx->getOptions()->applyJourneyLogic() = true;
  bceVal._partOfLocalJny = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));

  bceVal._partOfLocalJny = true;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.localJourneyCarrier(*_trx, _airSeg));

  const_cast<CarrierPreference*>(_airSeg->carrierPref())->localMktJourneyType() = NO;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));

  CPPUNIT_ASSERT_EQUAL(false, bceVal.localJourneyCarrier(*_trx, _airSeg));
}

void
BookingCodeExceptionValidatorTest::testTryLocalWithFlowAvail()
{
  PaxTypeFare paxTypeFare;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  _trx->getOptions()->journeyActivatedForPricing() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->getOptions()->journeyActivatedForPricing() = true;
  _trx->getOptions()->applyJourneyLogic() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->getOptions()->applyJourneyLogic() = true;
  bceVal._partOfLocalJny = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->getOptions()->journeyActivatedForShopping() = false;
  bceVal._partOfLocalJny = true;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->getOptions()->journeyActivatedForShopping() = true;
  _trx->getOptions()->applyJourneyLogic() = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->getOptions()->applyJourneyLogic() = true;
  bceVal._partOfLocalJny = false;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  bceVal._partOfLocalJny = true;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
  CPPUNIT_ASSERT_EQUAL(false, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));

  _trx->setTrxType(PricingTrx::PRICING_TRX);
  CPPUNIT_ASSERT_EQUAL(true, bceVal.tryLocalWithFlowAvail(*_trx, paxTypeFare));
}

void
BookingCodeExceptionValidatorTest::testValidateStartDate()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  DateTime startDate(2008, 11, 25);

  CPPUNIT_ASSERT_EQUAL(true, bceVal.validateStartDate(startDate, 2008, 11, 25));
  CPPUNIT_ASSERT_EQUAL(true, bceVal.validateStartDate(startDate, 2008, 11, 24));
  CPPUNIT_ASSERT_EQUAL(false, bceVal.validateStartDate(startDate, 2008, 11, 26));
}

void
BookingCodeExceptionValidatorTest::testValidateStopDate()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  DateTime startDate(2008, 11, 25);

  CPPUNIT_ASSERT_EQUAL(true, bceVal.validateStopDate(startDate, 2008, 11, 25));
  CPPUNIT_ASSERT_EQUAL(false, bceVal.validateStopDate(startDate, 2008, 11, 24));
  CPPUNIT_ASSERT_EQUAL(true, bceVal.validateStopDate(startDate, 2008, 11, 26));
}

void
BookingCodeExceptionValidatorTest::testPartOfJourney()
{
  FareMarket fareMarket;
  AirSeg aSeg;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  fareMarket.setFlowMarket(false);
  _itin->fareMarket().push_back(&fareMarket);
  CPPUNIT_ASSERT_EQUAL(false, bceVal.partOfJourney(*_trx, &aSeg));

  _itin->segmentOAndDMarket()[&aSeg] = _memHandle.create<OAndDMarket>();
  CPPUNIT_ASSERT_EQUAL(true, bceVal.partOfJourney(*_trx, &aSeg));
}

void
BookingCodeExceptionValidatorTest::testJourneyExistInItin()
{
  FareMarket fareMarket;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  fareMarket.setFlowMarket(false);
  _itin->fareMarket().push_back(&fareMarket);
  CPPUNIT_ASSERT_EQUAL(false, bceVal.journeyExistInItin(*_trx));

  OAndDMarket odFm;
  _itin->oAndDMarkets().push_back(&odFm);
  CPPUNIT_ASSERT_EQUAL(true, bceVal.journeyExistInItin(*_trx));
}

void
BookingCodeExceptionValidatorTest::testStatusToAsBooked()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE1;
  bceVal.statusToAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_RULE1_AS_BOOKED);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2;
  bceVal.statusToAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_RULE2_AS_BOOKED);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_JORNY;
  bceVal.statusToAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_JORNY_AS_BOOKED);
}

void
BookingCodeExceptionValidatorTest::testStatusFromAsBooked()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE1_AS_BOOKED;
  bceVal.statusFromAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_RULE1);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2_AS_BOOKED;
  bceVal.statusFromAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_RULE2);

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_JORNY_AS_BOOKED;
  bceVal.statusFromAsBooked();
  CPPUNIT_ASSERT(bceVal._statusType == BookingCodeExceptionValidator::STATUS_JORNY);
}

void
BookingCodeExceptionValidatorTest::testCheckBookedClassAvail()
{
  AirSeg aSeg;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  _trx->getRequest()->lowFareRequested() = 'N';
  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  _trx->getRequest()->lowFareRequested() = 'Y';
  _trx->getRequest()->upSellEntry() = true;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  _trx->getRequest()->upSellEntry() = false;
  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->billing()->actionCode() = "WPNI.A";
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  _trx->setTrxType(PricingTrx::FF_TRX);
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->billing()->actionCode() = "WPNI.C";
  aSeg.realResStatus() = QF_RES_STATUS;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  aSeg.realResStatus() = CONFIRM_RES_STATUS;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassAvail(*_trx, &aSeg));

  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassAvail(*_trx, 0));
}

void
BookingCodeExceptionValidatorTest::testCheckBookedClassOffer()
{
  AirSeg aSeg;
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);

  _trx->getRequest()->lowFareRequested() = 'N';
  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassOffer(*_trx, &aSeg));

  _trx->getRequest()->lowFareRequested() = 'Y';
  _trx->getRequest()->upSellEntry() = false;
  _trx->setTrxType(PricingTrx::MIP_TRX);
  _trx->billing()->actionCode() = "WPNI.A";
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassOffer(*_trx, &aSeg));

  _trx->billing()->actionCode() = "WPNI.C";
  aSeg.realResStatus() = QF_RES_STATUS;
  CPPUNIT_ASSERT_EQUAL(true, bceVal.checkBookedClassOffer(*_trx, &aSeg));

  aSeg.realResStatus() = CONFIRM_RES_STATUS;
  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassOffer(*_trx, &aSeg));

  CPPUNIT_ASSERT_EQUAL(false, bceVal.checkBookedClassOffer(*_trx, 0));
}

void
BookingCodeExceptionValidatorTest::testGetAvailability()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  PaxTypeFare::SegmentStatus status;
  PaxTypeFare paxTypeFare;
  ClassOfService* cos;

  cos = _memHandle.create<ClassOfService>();
  std::vector<ClassOfService*> cosVec;

  cosVec.push_back(cos);
  _airSeg->classOfService() = cosVec;
  _airSeg->carrierPref() = _cxrPref;

  bceVal._statusType = BookingCodeExceptionValidator::STATUS_RULE2_AS_BOOKED;
  _airSeg->carrierPref() = 0;
  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, _airSeg, status, paxTypeFare, 0) == 0);

  _airSeg->carrierPref() = _cxrPref;
  _cxrPref->availabilityApplyrul2st() = YES;
  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, _airSeg, status, paxTypeFare, 0) == 0);

  _cxrPref->availabilityApplyrul2st() = NO;
  _cxrPref->localMktJourneyType() = YES;
  CPPUNIT_ASSERT(bceVal.getAvailability(*_trx, _airSeg, status, paxTypeFare, 0) == 0);

  _cxrPref->localMktJourneyType() = NO;
  _cxrPref->flowMktJourneyType() = NO;
  CPPUNIT_ASSERT((*bceVal.getAvailability(*_trx, _airSeg, status, paxTypeFare, 0)) == cosVec);
}

void
BookingCodeExceptionValidatorTest::testTagFltFAILTagN()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  bceVal._fltResultVector.resize(1);
  int airIndex = 0;

  bceVal.tagFltFAILTagN(*_paxTypeFare, _segStatVec, airIndex, *_trx);

  CPPUNIT_ASSERT(_paxTypeFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_TAG_N));
  CPPUNIT_ASSERT(_segStatVec[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_TAG_N));
}
void
BookingCodeExceptionValidatorTest::testValidateCabinForDifferential()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  bceVal._fltResultVector.resize(1);
  setRequestAsWPNC();
  PaxTypeFare paxTypeFare;
  paxTypeFare.cabin().setBusinessClass();
  tvlSeg->bookedCabin().setEconomyClass();
  bceVal.validateCabinForDifferential(*_trx, tvlSeg, paxTypeFare);

  CPPUNIT_ASSERT(paxTypeFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_REQ_LOWER_CABIN));
}

// RBD Enhancement Project
void
BookingCodeExceptionValidatorTest::testIsSegmentNoMatched_Yes()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t segSize = 3;
  int16_t segMatch[segSize];
  for (uint16_t i = 0; i < segSize; ++i)
    segMatch[i] = BookingCodeExceptionValidator::BCE_FLT_NOMATCH;
  CPPUNIT_ASSERT(bceVal.isSegmentNoMatched(segMatch, segSize));
}
void
BookingCodeExceptionValidatorTest::testIsSegmentNoMatched_No()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t segSize = 3;
  int16_t segMatch[segSize];
  for (uint16_t i = 0; i < segSize; ++i)
    segMatch[i] = 0;
  CPPUNIT_ASSERT(!bceVal.isSegmentNoMatched(segMatch, segSize));
}
void
BookingCodeExceptionValidatorTest::testIsAllFlightsSkipped_Yes()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t fltSize = 3;
  int16_t fltMatch[fltSize];
  for (uint16_t i = 0; i < fltSize; ++i)
    fltMatch[i] = BookingCodeExceptionValidator::BCE_FLT_SKIPPED;
  CPPUNIT_ASSERT(bceVal.isAllFlightsSkipped(fltMatch, fltSize));
}
void
BookingCodeExceptionValidatorTest::testIsAllFlightsSkipped_No()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t fltSize = 3;
  int16_t fltMatch[fltSize];
  for (uint16_t i = 0; i < fltSize; ++i)
    fltMatch[i] = BookingCodeExceptionValidator::BCE_FLT_SKIPPED;
  fltMatch[fltSize - 1] = BookingCodeExceptionValidator::BCE_FLT_NOMATCH;
  CPPUNIT_ASSERT(!bceVal.isAllFlightsSkipped(fltMatch, fltSize));
}
void
BookingCodeExceptionValidatorTest::testIsAllFlightsMatched_Yes()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t fltSize = 3;
  int16_t fltMatch[fltSize];
  fltMatch[0] = 0;
  for (uint16_t i = 1; i < fltSize; ++i)
    fltMatch[i] = i + 1;

  CPPUNIT_ASSERT(bceVal.isAllFlightsMatched(fltMatch, fltSize));
}
void
BookingCodeExceptionValidatorTest::testIsAllFlightsMatched_No()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  const uint16_t fltSize = 3;
  int16_t fltMatch[fltSize];
  fltMatch[0] = 0;
  for (uint16_t i = 1; i < fltSize; ++i)
    fltMatch[i] = i + 1;

  fltMatch[fltSize - 1] = BookingCodeExceptionValidator::BCE_FLT_NOMATCH;
  CPPUNIT_ASSERT(!bceVal.isAllFlightsMatched(fltMatch, fltSize));

  fltMatch[fltSize - 1] = BookingCodeExceptionValidator::BCE_FLT_SKIPPED;
  CPPUNIT_ASSERT(!bceVal.isAllFlightsMatched(fltMatch, fltSize));
}

void
BookingCodeExceptionValidatorTest::testValidateSequenceForBothRBDs_False()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  BookingCodeExceptionSequence sequence;

  BookingCodeExceptionSegment* segment1 = new BookingCodeExceptionSegment;
  BookingCodeExceptionSegment* segment2 = new BookingCodeExceptionSegment;

  segment1->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  segment2->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;

  sequence.segmentVector().push_back(segment1);
  sequence.segmentVector().push_back(segment2);

  CPPUNIT_ASSERT(!bceVal.validateSequenceForBothRBDs(sequence));
}

void
BookingCodeExceptionValidatorTest::testValidateSequenceForBothRBDs_True()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  BookingCodeExceptionSequence sequence;

  BookingCodeExceptionSegment* segment1 = new BookingCodeExceptionSegment;
  BookingCodeExceptionSegment* segment2 = new BookingCodeExceptionSegment;

  segment1->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  segment2->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;
  segment1->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  segment1->bookingCode2() = AVAILABLE_BOOKING_CODE2;
  segment2->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  segment2->bookingCode2() = AVAILABLE_BOOKING_CODE2;

  sequence.segmentVector().push_back(segment1);
  sequence.segmentVector().push_back(segment2);

  CPPUNIT_ASSERT(bceVal.validateSequenceForBothRBDs(sequence));
}

void
BookingCodeExceptionValidatorTest::testValidateSequenceForBothRBDs_False_S2RBD2Empty()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  BookingCodeExceptionSequence sequence;

  BookingCodeExceptionSegment* segment1 = new BookingCodeExceptionSegment;
  BookingCodeExceptionSegment* segment2 = new BookingCodeExceptionSegment;

  segment1->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  segment2->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;
  segment1->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  segment1->bookingCode2() = AVAILABLE_BOOKING_CODE2;
  segment2->bookingCode1() = AVAILABLE_BOOKING_CODE1;

  sequence.segmentVector().push_back(segment1);
  sequence.segmentVector().push_back(segment2);

  CPPUNIT_ASSERT(!bceVal.validateSequenceForBothRBDs(sequence));
}
void
BookingCodeExceptionValidatorTest::testValidateSequenceForBothRBDs_False_S1RBD2Empty()
{
  BookingCodeExceptionValidator bceVal(*_itin, false, false, false);
  BookingCodeExceptionSequence sequence;

  BookingCodeExceptionSegment* segment1 = new BookingCodeExceptionSegment;
  BookingCodeExceptionSegment* segment2 = new BookingCodeExceptionSegment;

  segment1->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  segment2->restrictionTag() = BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;
  segment1->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  segment2->bookingCode1() = AVAILABLE_BOOKING_CODE1;
  segment2->bookingCode2() = AVAILABLE_BOOKING_CODE2;

  sequence.segmentVector().push_back(segment1);
  sequence.segmentVector().push_back(segment2);

  CPPUNIT_ASSERT(!bceVal.validateSequenceForBothRBDs(sequence));
}

void
BookingCodeExceptionValidatorTest::testValidateWPNCS()
{
  _request->lowFareNoAvailability() = 'Y';
  _request->setjumpUpCabinAllowed();
  _trx->getOptions()->cabin().setFirstClass();
  _segStatVec[0]._reBookCabin.setFirstClass();
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);
  bceValStub._fltResultVector.resize(1);
  uint16_t airIndex = 0;
  _airSeg->rbdReplaced() = true;
  TravelSeg* tvlSeg = (TravelSeg*)_airSeg;
  tvlSeg->bookedCabin().setBusinessClass();
  bceValStub.checkPriceByCabin(*_trx, _segStatVec, airIndex, tvlSeg, *_paxTypeFare);
  CPPUNIT_ASSERT(_segStatVec[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL));
}

void
BookingCodeExceptionValidatorTest::testAllSegsStatusQF()
{
  _request->lowFareNoAvailability() = 'Y';
  _request->lowFareRequested() = 'Y';
  _trx->setTrxType(PricingTrx::PRICING_TRX);
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);

  AirSeg* pAirSegConv1 = _memHandle.create<AirSeg>();
  pAirSegConv1->realResStatus() = QF_RES_STATUS;
  CPPUNIT_ASSERT(bceValStub.allSegsStatusQF(*_trx, *_paxTypeFare, pAirSegConv1));

  AirSeg* airSeg1 = _memHandle.create<AirSeg>();
  airSeg1->realResStatus() = QF_RES_STATUS;
  AirSeg* airSeg2 = _memHandle.create<AirSeg>();
  airSeg2->realResStatus() = QF_RES_STATUS;
  AirSeg* airSeg3 = _memHandle.create<AirSeg>();
  airSeg3->realResStatus() = RQ_RES_STATUS;
  _paxTypeFare->fareMarket()->travelSeg() = {airSeg1, airSeg2};
  CPPUNIT_ASSERT(bceValStub.allSegsStatusQF(*_trx, *_paxTypeFare, nullptr));
  _paxTypeFare->fareMarket()->travelSeg().push_back(airSeg3);
  CPPUNIT_ASSERT(!bceValStub.allSegsStatusQF(*_trx, *_paxTypeFare, nullptr));
}

void
BookingCodeExceptionValidatorTest::testSkipCat31Flown()
{
  ArunkSeg* tvlSeg = _memHandle.create<ArunkSeg>();
  RexPricingTrx rexTrx;

  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, true);
  CPPUNIT_ASSERT(!bceValStub.skipCat31Flown(rexTrx, static_cast<TravelSeg*>(tvlSeg)));
  rexTrx.setExcTrxType(PricingTrx::AR_EXC_TRX);
  rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  CPPUNIT_ASSERT(!bceValStub.skipCat31Flown(rexTrx, static_cast<TravelSeg*>(tvlSeg)));
  _airSeg->unflown() = false;

  CPPUNIT_ASSERT(bceValStub.skipCat31Flown(rexTrx, _airSeg));
}

void
BookingCodeExceptionValidatorTest::testValidateWPNCSFromPrimeRBD()
{
  BookingCodeExceptionValidatorStubValidateBC bceValStub(*_itin, false, false);

  BookingCodeExceptionSegment* segment = _memHandle.create<BookingCodeExceptionSegment>();

  segment->restrictionTag() =  BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_OFFER;
  CPPUNIT_ASSERT(bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_PERMITTED_IF_PRIME_NOT_AVAIL;
  CPPUNIT_ASSERT(bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_REQUIRED_IF_PRIME_NOT_OFFER;
  CPPUNIT_ASSERT(bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_REQUIRED_IF_PRIME_NOT_AVAIL;
  CPPUNIT_ASSERT(bceValStub.validateWPNCSFromPrimeRBD(*segment));

  segment->restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_REQUIRED_WHEN_OFFERED;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_REQUIRED_WHEN_AVAIL;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() =
      BookingCodeExceptionValidator::BCE_ADDITIONAL_DATA_APPLIES;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() = BookingCodeExceptionValidator::BCE_STANDBY;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() = BookingCodeExceptionValidator::BCE_NOT_PERMITTED;
  CPPUNIT_ASSERT(!bceValStub.validateWPNCSFromPrimeRBD(*segment));
  segment->restrictionTag() = BookingCodeExceptionValidator::BCE_DOES_NOT_EXIST;
}

void
BookingCodeExceptionValidatorTest::testResetAsBooked()
{
}

