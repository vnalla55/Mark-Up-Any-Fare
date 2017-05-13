#include "BookingCode/test/FareDisplayBookingCodeExceptionTest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestFactoryManager.h"

using namespace tse;

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayBookingCodeExceptionTest);

void
FareDisplayBookingCodeExceptionTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _trx = _memHandle.create<FareDisplayTrx>();
  _req = _memHandle.create<FareDisplayRequest>();
  _req->requestType() = "RB";
  _trx->setRequest(_req);
  _rb = _memHandle.create<RBData>();
  _rb->setSecondaryCityPairSameAsPrime(false);

  _ptfFBR = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Constr.xml");
  _ptfInd = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
  _airSeg = TestAirSegFactory::create("testdata/BCE1/SecondarySeg.xml");
  _fbExc = _memHandle.create<FareDisplayBookingCodeExceptionMock>(_trx, _ptfFBR, _rb);
  _fbExcInd = _memHandle.create<FareDisplayBookingCodeExceptionMock>(_trx, _ptfInd, _rb);

  _seq = _memHandle.create<BookingCodeExceptionSequence>();
  _seg = _memHandle.create<BookingCodeExceptionSegment>();
  initSequence(_seq);
  initSegment(_seg);
  restoreDirections();
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::tearDown()
{
  _memHandle.clear();
  TestFactoryManager::instance()->destroyAll();
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testcanRemoveSegmentCondMatch()
{
  // can remove segment when segments have the same conditional values
  BookingCodeExceptionSequence seq1, seq2;
  BookingCodeExceptionSegment seg1, seg2;
  initSequence(&seq1);
  initSequence(&seq2);
  initSegment(&seg1);
  initSegment(&seg2);
  seq1.segCnt() = 2;
  seq2.segCnt() = 2;
  seg1.flight1() = 1;
  seg2.flight1() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->canRemoveSegment(seg1, seg2, seq1, seq2));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testcanRemoveSegmentCondNotMatch()
{
  // can't remove segment when segments have the different conditional values
  BookingCodeExceptionSequence seq1, seq2;
  BookingCodeExceptionSegment seg1, seg2;
  initSequence(&seq1);
  initSequence(&seq2);
  initSegment(&seg1);
  initSegment(&seg2);
  seq1.segCnt() = 2;
  seq2.segCnt() = 2;
  seg1.flight1() = 1;
  seg2.flight1() = 2;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->canRemoveSegment(seg1, seg2, seq1, seq2));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testcanRemoveSegmentRestrictionNotMatch()
{
  // can't remove segment when segments have restriction tag different then required
  BookingCodeExceptionSequence seq1, seq2;
  BookingCodeExceptionSegment seg1, seg2;
  initSequence(&seq1);
  initSequence(&seq2);
  initSegment(&seg1);
  initSegment(&seg2);
  seq1.segCnt() = 2;
  seq2.segCnt() = 2;
  seg1.flight1() = 1;
  seg2.flight1() = 1;
  seg1.segNo() = 2;
  seg1.restrictionTag() = FareDisplayBookingCodeException::BCE_PERMITTED;
  seg2.restrictionTag() = FareDisplayBookingCodeException::BCE_PERMITTED;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->canRemoveSegment(seg1, seg2, seq1, seq2));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceSpecifiedBlank()
{
  // always pass for blank
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isValidSequence(*_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceSpecifiedSpecified()
{
  // fare specified, tag specified - pass
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_FARE_SPECIFIED;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isValidSequence(*_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceSpecifiedConstructed()
{
  // fare specified, tag constructed - fail
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_FARE_CONSTRUCTED;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isValidSequence(*_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceConstructedBlank()
{
  // always pass for blank
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, true);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isValidSequence(*_seq));
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceConstructedSpecified()
{
  // fare constructed, tag speciified - fail
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, true);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_FARE_SPECIFIED;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isValidSequence(*_seq));
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisValidSequenceConstructedConstructed()
{
  // fare constructed, tag constructed - passs
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, true);
  _seq->constructSpecified() = FareDisplayBookingCodeException::BCE_FARE_CONSTRUCTED;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isValidSequence(*_seq));
  _ptfFBR->fare()->status().set(Fare::FS_ConstructedFare, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalfltRangeAppl()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->fltRangeAppl() = 'A';
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalflight1()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->flight1() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalflight2()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->flight2() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalposTsi()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->posTsi() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalposLocType()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->posLocType() = '1';
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalsellTktInd()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->sellTktInd() = '1';
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlEffYear()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlEffYear() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlEffMonth()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlEffMonth() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlEffDay()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlEffDay() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlDiscYear()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlDiscYear() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlDiscMonth()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlDiscMonth() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlDiscDay()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlDiscDay() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlStartTime()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlStartTime() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionaltvlEndTime()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->tvlEndTime() = 1;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalrestrictionTag()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_PERMITTED;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalrestrictionTagB_R6C1()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  _fbExc->_convention2 = false;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testisRBSegmentConditionalrestrictionTagB_R6C2()
{
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->isRBSegmentConditional(*_seg));
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE;
  _fbExc->_convention2 = true;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->isRBSegmentConditional(*_seg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierBlank()
{
  // always match on blank
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierAnyCarrier()
{
  // match on any carrier
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_ANYCARRIER;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierCarrierEY()
{
  // match on carrier
  _seg->viaCarrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierCarrierYY()
{
  // fail for industry for FBR
  _seg->viaCarrier() = INDUSTRY_CARRIER;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierCarrierXDollarCarrierEY()
{
  // match when carriers are different
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierCarrierXDollarCarrierYY()
{
  // match when carriers are different
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "YY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierAirSegCarrierRB()
{
  // not match on ptf, check secondary segment
  _airSeg->carrier() = "IB";
  _seg->viaCarrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierAirSegCarrierFQ()
{
  // not match on ptf, check secondary segment
  _airSeg->carrier() = "IB";
  _seg->viaCarrier() = "IB";
  _req->requestType() = "FQ";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierConditXDollarCarrierEY()
{
  // not conditional segment check for $
  _seg->segNo() = 2;
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierConditXDollarCarrierIB()
{
  // not conditional segment check for $
  _seg->segNo() = 2;
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierConditCarrierEY()
{
  // match carrier must match on air ser carrier
  _seg->segNo() = 2;
  _seg->viaCarrier() = "EY";
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierConditCarrierIB()
{
  // match carrier must match on air ser carrier
  _seg->segNo() = 2;
  _seg->viaCarrier() = "EY";
  _airSeg->carrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierConditAirSegCarrier()
{
  // match on secondary segment carrier
  _seg->segNo() = 2;
  _seg->viaCarrier() = "IB";
  _airSeg->carrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryBlank()
{
  // always match on blank
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryAnyCarrier()
{
  // match on any carrier
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_ANYCARRIER;
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryCarrierEY()
{
  // no match on carrier EY
  _seg->viaCarrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(false, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryCarrierYY()
{
  // match on Industry fare
  _seg->viaCarrier() = INDUSTRY_CARRIER;
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryXDollarCarrierEY()
{
  // match on industry fare
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryXDollarCarrierYY()
{
  // match on industry fare
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "YY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryAirSegCarrierRB()
{
  // not match on ptf, check secondary segment
  _airSeg->carrier() = "IB";
  _seg->viaCarrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryAirSegCarrierFQ()
{
  // not match on ptf, check secondary segment
  _airSeg->carrier() = "IB";
  _seg->viaCarrier() = "IB";
  _req->requestType() = "FQ";
  CPPUNIT_ASSERT_EQUAL(false, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryConditXDollarCarrierEY()
{
  // not conditional segment check for $ - match on industry
  _seg->segNo() = 2;
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryConditXDollarCarrierIB()
{
  // not conditional segment check for $ - match on industry
  _seg->segNo() = 2;
  _seg->viaCarrier() = FareDisplayBookingCodeException::BCE_XDOLLARCARRIER;
  _airSeg->carrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
/////////////////////////////////////////////////////////////////////k///////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryConditCarrierEY()
{
  // match carrier must match on air ser carrier
  _seg->segNo() = 2;
  _seg->viaCarrier() = "EY";
  _airSeg->carrier() = "EY";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryConditCarrierIB()
{
  // match carrier must match on air ser carrier
  _seg->segNo() = 2;
  _seg->viaCarrier() = "EY";
  _airSeg->carrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(false, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
/////////////////////////////////////////////////////////////////////k///////////
void
FareDisplayBookingCodeExceptionTest::testvalidateCarrierIndustryConditAirSegCarrier()
{
  // match on secondary segment carrier
  _seg->segNo() = 2;
  _airSeg->carrier() = "IB";
  _seg->viaCarrier() = "IB";
  CPPUNIT_ASSERT_EQUAL(true, _fbExcInd->validateCarrier(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim11()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim12()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim13()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim21()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim22()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim23()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim31()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim32()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Prim33()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::AT);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec11()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec12()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec13()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec21()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec22()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec23()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec31()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec32()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelAT_Sec33()
{
  _seg->tvlPortion() = "AT"; // via Atlantic, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCA_PLCA()
{
  _seg->tvlPortion() = "CA"; // in canada
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCA_CAPL()
{
  _seg->tvlPortion() = "CA"; // in canada
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCA_CACA()
{
  _seg->tvlPortion() = "CA"; // in canada
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_11()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion (different iata or nations
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_12()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion (different iata or nations
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_21()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion (different iata or nations
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_CACA()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_CAPL()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelCO_PLCA()
{
  _seg->tvlPortion() = "CO"; // Controlling Portion
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_USUS()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_USCA()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_CAUS()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_PLPL()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_USPL()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelDO_PLUS()
{
  _seg->tvlPortion() = "DO"; // domestic or foreiggn domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_11()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_12()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_13()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_21()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_22()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_23()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_31()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_32()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelEH_33()
{
  _seg->tvlPortion() = "EH"; // between iata 2 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFD_USUS()
{
  _seg->tvlPortion() = "FD"; // foreign domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFD_USPL()
{
  _seg->tvlPortion() = "FD"; // foreign domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFD_PLUS()
{
  _seg->tvlPortion() = "FD"; // foreign domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFD_PLPL()
{
  _seg->tvlPortion() = "FD"; // foreign domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_11()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_12()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_13()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_21()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_22()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_23()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_31()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_32()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelFE_33()
{
  _seg->tvlPortion() = "FE"; // far east
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri11()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri12()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri13()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri21()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri22()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri23()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri31()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri32()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Pri33()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  setDirectionality(_ptfFBR, GlobalDirection::PA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec11()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec12()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec13()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "1");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec21()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec22()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec23()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec31()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec32()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelPA_Sec33()
{
  _seg->tvlPortion() = "PA"; // via Pacific, etween iata1 and 2 or 3
  // secondary segment validation
  _seg->segNo() = 2;
  setIATA(_airSeg->origin(), "3");
  setIATA(_airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelTB_USUS()
{
  _seg->tvlPortion() = "TB"; // Transborder
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelTB_USCA()
{
  _seg->tvlPortion() = "TB"; // Transborder
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelTB_CAUS()
{
  _seg->tvlPortion() = "TB"; // Transborder
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelTB_PLCA()
{
  _seg->tvlPortion() = "TB"; // Transborder
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), "PL");
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelTB_CAPL()
{
  _seg->tvlPortion() = "TB"; // Transborder
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), "PL");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelUS_USUS()
{
  _seg->tvlPortion() = "US"; // US Domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelUS_USCA()
{
  _seg->tvlPortion() = "US"; // US Domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), UNITED_STATES);
  setNation(airSeg->destination(), CANADA);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelUS_CAUS()
{
  _seg->tvlPortion() = "US"; // US Domestic
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setNation(airSeg->origin(), CANADA);
  setNation(airSeg->destination(), UNITED_STATES);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_11()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_12()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_13()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "1");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_21()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_22()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_23()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_31()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "1");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_32()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePortionOfTravelWH_33()
{
  _seg->tvlPortion() = "WH"; // Western Hemisphere
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "3");
  setIATA(airSeg->destination(), "3");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePortionOfTravel(*_seg, *_seq, _airSeg));
  // no distinguish between primary and secondary
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryDomesticPrimary()
{
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryDomesticSecondary()
{
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_SECONDARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryDomesticFromTo()
{
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_FROMTO_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Domestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryTransborderPrimary()
{
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryTransborderSecondary()
{
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_SECONDARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryTransborderFromTo()
{
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_FROMTO_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_Transborder, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryForeignDomesticPrimary()
{
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryForeignDomesticSecondary()
{
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_SECONDARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryForeignDomesticFromTo()
{
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, true);
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_FROMTO_PRIMARY;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _ptfFBR->fare()->status().set(Fare::FS_ForeignDomestic, false);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryBlank()
{
  // always pass for blank
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondaryPrimary()
{
  // primary check
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_PRIMARY;
  AirSeg* airSeg = dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front());
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePrimarySecondary(*_seg, *_seq, airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePrimarySecondarySecondary()
{
  // secondary check
  _seg->primarySecondary() = FareDisplayBookingCodeException::BCE_SEG_SECONDARY;
  _rb->setSecondary(false);
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
  _rb->setSecondary(true);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validatePrimarySecondary(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateTSIFail()
{
  // real TSI validation is done in RuleUtil, thsi just check segment checking
  Itin it;
  it.calculationCurrency() = "PLN";
  it.travelSeg().push_back(_ptfFBR->fareMarket()->travelSeg().front());
  _trx->itin().push_back(&it);
  _seg->tsi() = 1;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateTSI(*_seg, *_seq, _airSeg));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateTSIPassEmpty()
{
  // real TSI validation is done in RuleUtil, thsi just check segment checking
  Itin it;
  it.calculationCurrency() = "PLN";
  it.travelSeg().push_back(_ptfFBR->fareMarket()->travelSeg().front());
  _trx->itin().push_back(&it);
  _seg->tsi() = 2;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateTSI(*_seg, *_seq, _airSeg));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateTSIPassNotEmpty()
{
  // real TSI validation is done in RuleUtil, thsi just check segment checking
  Itin it;
  it.calculationCurrency() = "PLN";
  it.travelSeg().push_back(_ptfFBR->fareMarket()->travelSeg().front());
  _trx->itin().push_back(&it);
  _seg->tsi() = 3;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateTSI(*_seg, *_seq, _airSeg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriLUXABC()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriABCAUH()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriLUXAUH()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriAUHLUX()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_PriAUHLUX_RT()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  // pass if fare is round trip
  setOWRT(_ptfFBR, ROUND_TRIP_MAYNOT_BE_HALVED);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  setOWRT(_ptfFBR, ONE_WAY_MAYNOT_BE_DOUBLED);
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_SecBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  _seg->segNo() = 2;
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_SecLONABC()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_SecABCMAD()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_SecLONMAD()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir1_SecMADLON()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "MAD";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriLUXABC()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriABCAUH()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriLUXAUH()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriLUXAUH_RT()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  // pass if fare is round trip
  setOWRT(_ptfFBR, ROUND_TRIP_MAYNOT_BE_HALVED);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  setOWRT(_ptfFBR, ONE_WAY_MAYNOT_BE_DOUBLED);
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_PriAUHLUX()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_SecBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  _seg->segNo() = 2;
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_SecLONABC()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_SecABCMAD()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_SecLONMAD()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir2_SecMADLON()
{
  _seg->directionInd() = '2'; // Travel from Loc2 to Loc1
  // secondary sector
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "MAD";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '3'; // Fares originating Loc1
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriLUXABC()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriABCAUH()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriLUXAUH()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriAUHLUX()
{
  _seg->directionInd() = '1'; // Travel from Loc1 to Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_PriAUHLUX_INBOUND()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  // pas if dare is inbound
  setMarketDirection(_ptfFBR, FMDirection::INBOUND);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_SecBlank()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  _seg->segNo() = 2;
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_SecLONMAD()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  // secondary sector aways fail
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir3_SecMADLON()
{
  _seg->directionInd() = '3'; // Fares originating Loc1
  // secondary sector always fail
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "MAD";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '4'; // Fares originating Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriLUXABC()
{
  _seg->directionInd() = '4'; // Fares originating Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriABCAUH()
{
  _seg->directionInd() = '4'; // Fares originating Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriLUXAUH()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '4'; // Fares originating Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriLUXAUH_INBOUND()
{
  _seg->directionInd() = '4'; // Fares originating Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  // pass if fare is INBOUND
  setMarketDirection(_ptfFBR, FMDirection::INBOUND);
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_PriAUHLUX()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '4'; // Fares originating Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_SecBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = '4'; // Fares originating Loc2
  _seg->segNo() = 2;
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_SecLONMAD()
{
  _seg->directionInd() = '4'; // Fares originating Loc2
  // secondary sector always fail
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LON";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDir4_SecMADLON()
{
  _seg->directionInd() = '4'; // Fares originating Loc2
  // secondary sector always fail
  _seg->segNo() = 2;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'C';
  _seg->loc1() = "MAD";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_PriBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Pri12()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'A';
  _seg->loc1() = "1";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "2";
  //    setIATA(_rb->airSeg()->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Pri21()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'A';
  _seg->loc1() = "2";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "1";
  //    setIATA(_rb->airSeg()->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Pri22()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'A';
  _seg->loc1() = "2";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "2";
  //    setIATA(_rb->airSeg()->destination(), "2");
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_PriLUXABC()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "ABC";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_PriABCAUH()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "ABC";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_PriLUXAUH()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "LUX";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "AUH";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_PriAUHLUX()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _rb->setAirSeg(
      dynamic_cast<AirSeg*>(_ptfFBR->fareMarket()->travelSeg().front())); // primary sector
  _seg->loc1Type() = 'C';
  _seg->loc1() = "AUH";
  _seg->loc2Type() = 'C';
  _seg->loc2() = "LUX";
  // match on reversed locations
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_SecBlank()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  // if no loc then pass
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Sec12()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'A';
  _seg->loc1() = "1";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "2";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Sec21()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'A';
  _seg->loc1() = "2";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "1";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_Sec22()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1Type() = 'A';
  _seg->loc1() = "2";
  _seg->loc2Type() = 'A';
  _seg->loc2() = "2";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_SecLONABC()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1() = "LON";
  _seg->loc1Type() = 'C';
  _seg->loc2() = "ABC";
  _seg->loc2Type() = 'C';
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_SecABCMAD()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1() = "ABC";
  _seg->loc1Type() = 'C';
  _seg->loc2() = "MAD";
  _seg->loc2Type() = 'C';
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
///////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_SecLONMAD()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1() = "LON";
  _seg->loc1Type() = 'C';
  _seg->loc2() = "MAD";
  _seg->loc2Type() = 'C';
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlank_SecMADLON()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  // secondary sector
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->loc1() = "MAD";
  _seg->loc1Type() = 'C';
  _seg->loc2() = "LON";
  _seg->loc2Type() = 'C';
  // match on reversed locations
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}

////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlankConditional_KRKWAW_WAWLON()
{
  // real location validation in LocUtil, thi is just segment checking validation
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->segNo() = 1;
  _seq->segCnt() = 2;
  // first sequence filled with blank and segment 1 markets KRK-WAW should return true
  // (conditionally)
  // second sequence filled with blank and segment 2 markets WAW-LONS should return false (no
  // matching record)
  _seg->loc1() = "KRK";
  _seg->loc2() = "WAW";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _seg->segNo() = 2;
  _seg->loc1() = "WAW";
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlankConditional_WAWLON_LONMAD()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->segNo() = 1;
  _seq->segCnt() = 2;
  // first sequence filled with blank and segment 1 markets KRK-WAW should return true
  // (conditionally)
  // second sequence filled with blank and segment 2 markets LON-MAD should return true (match)
  _seg->loc1() = "WAW";
  _seg->loc2() = "LON";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _seg->segNo() = 2;
  _seg->loc1() = "LON";
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateLocationDirBlankConditional_LONMAD_MADWAW()
{
  _seg->directionInd() = ' '; // Travel/fare between Loc 1 and Loc2
  _seq->ifTag() = FareDisplayBookingCodeException::BCE_CHAR_BLANK;
  _rb->setAirSeg(_airSeg);
  _seg->segNo() = 1;
  _seq->segCnt() = 2;
  // first sequence filled with blank and segment 1 markets LON_MAD should return true (match)
  // second sequence filled with blank and segment 2 markets MAD-WAW should return true (previous
  // segment matched)
  _seg->loc1() = "LON";
  _seg->loc2() = "MAD";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
  _seg->segNo() = 2;
  _seg->loc1() = "MAD";
  _seg->loc2() = "WAW";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateLocation(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidatePointOfSale()
{
  CPPUNIT_ASSERT_EQUAL(true,
                       _fbExc->validatePointOfSale(*_seg, *_seq)); // no validation at this point
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateSoldTag()
{
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateSoldTag(*_seg, *_seq)); // no validation at this point
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateDateTimeDOW()
{
  CPPUNIT_ASSERT_EQUAL(true,
                       _fbExc->validateDateTimeDOW(*_seg, *_seq)); // no validation at this point
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_T_FR()
{
  _seg->fareclassType() = 'T';
  _seg->fareclass() = "FR";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_T_PIT()
{
  _seg->fareclassType() = 'T';
  _seg->fareclass() = "PIT";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_F_Y()
{
  _seg->fareclassType() = 'F';
  _seg->fareclass() = "Y";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_Y_AOWLU()
{
  _seg->fareclassType() = 'F';
  _seg->fareclass() = "AOWLU";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_M_ABA()
{
  _seg->fareclassType() = 'M';
  _seg->fareclass() = "A-BA";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_M_AWLU()
{
  _seg->fareclassType() = 'M';
  _seg->fareclass() = "A-WLU";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_A_B()
{
  _seg->fareclassType() = 'A';
  _seg->fareclass() = "B";
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testvalidateFareClassType_A_A()
{
  _seg->fareclassType() = 'A';
  _seg->fareclass() = "A";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->validateFareClassType(*_seg, *_seq));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testchangeFareBasisCodeFailNotPermitted()
{
  BookingCode bkg = "A";
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_NOT_PERMITTED;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->changeFareBasisCode(*_seg, bkg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testchangeFareBasisCodeFailConditional()
{
  BookingCode bkg = "A";
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_REQUIRED;
  _seg->flight1() = 1;
  CPPUNIT_ASSERT_EQUAL(false, _fbExc->changeFareBasisCode(*_seg, bkg));
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testchangeFareBasisCodeFailRestrictionTag()
{
  BookingCode bkg = "A";
  _ptfFBR->setChangeFareBasisBkgCode(bkg);
  // tor this restriction tag don't have change booking code
  _seg->restrictionTag() = FareDisplayBookingCodeException::BCE_ADDITIONAL_DATA_APPLIES;
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->changeFareBasisCode(*_seg, bkg));
  CPPUNIT_ASSERT_EQUAL(BookingCode(""), _ptfFBR->getChangeFareBasisBkgCode());
}
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::testchangeFareBasisCodePass()
{
  BookingCode bkg = "A";
  CPPUNIT_ASSERT_EQUAL(true, _fbExc->changeFareBasisCode(*_seg, bkg));
  CPPUNIT_ASSERT_EQUAL(bkg, _ptfFBR->getChangeFareBasisBkgCode());
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////                      Helping functions                                 ////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
FareDisplayBookingCodeExceptionTest::restoreDirections()
{
  TravelSeg* airSeg = _ptfFBR->fareMarket()->travelSeg().front();
  setIATA(airSeg->origin(), "2");
  setIATA(airSeg->destination(), "2");
  setIATA(_airSeg->origin(), "2");
  setIATA(_airSeg->destination(), "2");
  setNation(airSeg->origin(), "LU");
  setNation(airSeg->destination(), "AE");
  setNation(_airSeg->origin(), "GB");
  setNation(_airSeg->destination(), "ES");
  setDirectionality(_ptfFBR, GlobalDirection::EH);
  setOWRT(_ptfFBR, ONE_WAY_MAYNOT_BE_DOUBLED);
  setMarketDirection(_ptfFBR, FMDirection::OUTBOUND);
}
void
FareDisplayBookingCodeExceptionTest::setDirectionality(const PaxTypeFare* ptf, GlobalDirection gd)
{
  (const_cast<FareInfo*>(_ptfFBR->fare()->fareInfo()))->globalDirection() = gd;
}
void
FareDisplayBookingCodeExceptionTest::setIATA(const Loc* loc, std::string iata)
{
  (const_cast<Loc*>(loc))->area() = iata;
}
void
FareDisplayBookingCodeExceptionTest::setNation(const Loc* loc, NationCode nation)
{
  (const_cast<Loc*>(loc))->nation() = nation;
}
void
FareDisplayBookingCodeExceptionTest::setOWRT(PaxTypeFare* ptf, Indicator owrt)
{
  (const_cast<FareInfo*>(ptf->fare()->fareInfo()))->owrt() = owrt;
}
void
FareDisplayBookingCodeExceptionTest::setMarketDirection(PaxTypeFare* ptf, FMDirection dir)
{
  (const_cast<FareMarket*>(ptf->fareMarket()))->direction() = dir;
}
void
FareDisplayBookingCodeExceptionTest::initSegment(BookingCodeExceptionSegment* seg)
{
  seg->segNo() = 1;
  seg->fltRangeAppl() = ' ';
  seg->soldInOutInd() = ' ';
  seg->sellTktInd() = ' ';
  seg->flight1() = 0;
  seg->flight2() = 0;
  seg->posTsi() = 0;
  seg->posLocType() = ' ';
  seg->sellTktInd() = ' ';
  seg->tvlEffYear() = 0;
  seg->tvlEffMonth() = 0;
  seg->tvlEffDay() = 0;
  seg->tvlDiscYear() = 0;
  seg->tvlDiscMonth() = 0;
  seg->tvlDiscDay() = 0;
  seg->tvlStartTime() = 0;
  seg->tvlEndTime() = 0;
  seg->restrictionTag() = FareDisplayBookingCodeException::BCE_REQUIRED;
  seg->bookingCode1() = "A";
  seg->loc1Type() = ' ';
  seg->loc2Type() = ' ';
}
void
FareDisplayBookingCodeExceptionTest::initSequence(BookingCodeExceptionSequence* seq)
{
  seq->ifTag() = FareDisplayBookingCodeException::BCE_IF_FARECOMPONENT;
  seq->itemNo() = 1;
  seq->primeInd() = ' ';
  seq->tableType() = ' ';
  seq->seqNo() = 0;
  seq->constructSpecified() = ' ';
  seq->segCnt() = 1;
}
