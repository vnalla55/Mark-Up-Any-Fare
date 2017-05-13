//-----------------------------------------------------------------------------
//
//  File:     BookingCodeExceptionValidatorTest.h
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

#ifndef BOOKING_CODE_EXCEPTION_VALIDATOR_TEST_H
#define BOOKING_CODE_EXCEPTION_VALIDATOR_TEST_H

#include <string>

#include "BookingCode/BookingCodeExceptionValidator.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Fare.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "Diagnostic/Diag405Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class Itin;
class PricingTrx;
class PricingRequest;
class Billing;
class BookingCodeExceptionSegment;
class AirSeg;
class PaxTypeFare;
class FareInfo;
class CarrierPreference;
class PaxType;
class PaxTypeInfo;
class Agent;
class Customer;
class PricingOptions;
class TravelSeg;

class BookingCodeExceptionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BookingCodeExceptionValidatorTest);
  CPPUNIT_TEST(testTagFlightNOMATCH);
  CPPUNIT_TEST(testTagFlightFail);
  CPPUNIT_TEST(testCheckBookedClassAvailWhenEntryNotWPNC);
  CPPUNIT_TEST(testCheckBookedClassAvailWhenUpSellEntry);
  CPPUNIT_TEST(testCheckBookedClassAvailWhenMipTrx);
  CPPUNIT_TEST(testCheckBookedClassAvailWhenMipTrxWPNI);
  CPPUNIT_TEST(testValidateWPsizeProblem);
  CPPUNIT_TEST(testValidateWPPass);
  CPPUNIT_TEST(testValidateWPFail);
  CPPUNIT_TEST(testGetAvailabilityIfNoCarrierPref);
  CPPUNIT_SKIP_TEST(testGetAvailabilityForSolo);
  CPPUNIT_SKIP_TEST(testGetAvailabilityValid);
  CPPUNIT_TEST(testValidateWPNCFailSizeProblem);
  CPPUNIT_SKIP_TEST(testValidateWPNCFailBookingCode);
  CPPUNIT_SKIP_TEST(testValidateWPNCPassBookingCode);
  CPPUNIT_SKIP_TEST(testValidateWPNCFailNumSeats);
  CPPUNIT_SKIP_TEST(testValidateWPNCPassIgnoreAvail);
  CPPUNIT_TEST(testValidateBCForWPReturnTrue);
  CPPUNIT_TEST(testValidateBCForWPReturnFalseRule1);
  CPPUNIT_TEST(testValidateBCForWPReturnFalseRule2AsBooked);
  CPPUNIT_TEST(testValidateBCForWPNCReturnFalse);
  CPPUNIT_TEST(testValidateBCForWPNCReturnTrue);
  CPPUNIT_TEST(testProcessRestrictionTagP);
  CPPUNIT_TEST(testProcessRestrictionTagR);
  CPPUNIT_TEST(testProcessRestrictionTagB_RBD1_NotAvail);
  CPPUNIT_TEST(testProcessRestrictionTagB_RBD1_Avail_RBD2_Match);
  CPPUNIT_TEST(testProcessRestrictionTagD);
  CPPUNIT_TEST(testProcessRestrictionTagD_R1T999Cat23R3);
  CPPUNIT_SKIP_TEST(testProcessRestrictionTagO);
  CPPUNIT_SKIP_TEST(testProcessRestrictionTagOasTagPforignoreAvail);

  CPPUNIT_TEST(testValidateCarrier1);
  CPPUNIT_TEST(testValidateCarrier2);
  CPPUNIT_TEST(testValidatePrimarySecondary);
  CPPUNIT_TEST(testValidateFlights);
  CPPUNIT_TEST(testValidateEquipment);
  CPPUNIT_TEST(testValidatePortionOfTravel);
  CPPUNIT_TEST(testValidatePointOfSale);
  CPPUNIT_TEST(testValidateSoldTag);
  CPPUNIT_TEST(testValidateFareclassType);
  CPPUNIT_TEST(testGetSegment);
  CPPUNIT_TEST(testAllFltsDone);
  CPPUNIT_TEST(testResetFltResult);
  CPPUNIT_TEST(testResetFirstMatchingSeqs);
  CPPUNIT_TEST(testGetSegStatusVec);
  CPPUNIT_TEST(testAnalyzeSeg);
  CPPUNIT_TEST(testFlowJourneyCarrier);
  CPPUNIT_TEST(testLocalJourneyCarrier);
  CPPUNIT_TEST(testTryLocalWithFlowAvail);
  CPPUNIT_TEST(testValidateStartDate);
  CPPUNIT_TEST(testValidateStopDate);
  CPPUNIT_TEST(testPartOfJourney);
  CPPUNIT_TEST(testJourneyExistInItin);
  CPPUNIT_TEST(testStatusToAsBooked);
  CPPUNIT_TEST(testStatusFromAsBooked);
  CPPUNIT_TEST(testCheckBookedClassAvail);
  CPPUNIT_TEST(testCheckBookedClassOffer);
  CPPUNIT_TEST(testGetAvailability);
  CPPUNIT_TEST(testTagFltFAILTagN);
  CPPUNIT_TEST(testValidateCabinForDifferential);
  CPPUNIT_TEST(testIsSegmentNoMatched_Yes);
  CPPUNIT_TEST(testIsSegmentNoMatched_No);
  CPPUNIT_TEST(testValidateSequenceForBothRBDs_False);
  CPPUNIT_TEST(testValidateSequenceForBothRBDs_True);
  CPPUNIT_TEST(testValidateSequenceForBothRBDs_False_S2RBD2Empty);
  CPPUNIT_TEST(testValidateSequenceForBothRBDs_False_S1RBD2Empty);
  CPPUNIT_TEST(testValidateWPNCS);
  CPPUNIT_TEST(testAllSegsStatusQF);
  CPPUNIT_TEST(testSkipCat31Flown);
  CPPUNIT_TEST(testValidateWPNCSFromPrimeRBD);
  CPPUNIT_TEST(testResetAsBooked);
  CPPUNIT_TEST_SUITE_END();

public:
  void testValidateFunction();

  void setUp();
  void tearDown();
  void testTagFlightNOMATCH();
  void testTagFlightFail();
  void testCheckBookedClassAvailWhenEntryNotWPNC();
  void testCheckBookedClassAvailWhenUpSellEntry();
  void testCheckBookedClassAvailWhenMipTrx();
  void testCheckBookedClassAvailWhenMipTrxWPNI();
  void testValidateWPsizeProblem();
  void testValidateWPPass();
  void testValidateWPFail();
  void testGetAvailabilityIfNoCarrierPref();
  void testGetAvailabilityForSolo();
  void testGetAvailabilityValid();
  void testValidateWPNCFailSizeProblem();
  void testValidateWPNCFailBookingCode();
  void testValidateWPNCPassBookingCode();
  void testValidateWPNCFailNumSeats();
  void testValidateWPNCPassIgnoreAvail();
  void testValidateBCForWPReturnTrue();
  void testValidateBCForWPReturnFalseRule1();
  void testValidateBCForWPReturnFalseRule2AsBooked();
  void testValidateBCForWPNCReturnFalse();
  void testValidateBCForWPNCReturnTrue();
  void testProcessRestrictionTagP();
  void testProcessRestrictionTagR();
  void testProcessRestrictionTagB_RBD1_NotAvail();
  void testProcessRestrictionTagB_RBD1_Avail_RBD2_Match();
  void testProcessRestrictionTagD();
  void testProcessRestrictionTagD_R1T999Cat23R3();
  void testProcessRestrictionTagO();
  void testProcessRestrictionTagOasTagPforignoreAvail();

  void testValidateCarrier1();
  void testValidateCarrier2();
  void testValidatePrimarySecondary();
  void testValidateFlights();
  void testValidateEquipment();
  void testValidatePortionOfTravel();
  void testValidatePointOfSale();
  void testValidateSoldTag();
  void testValidateFareclassType();
  void testGetSegment();
  void testAllFltsDone();
  void testResetFltResult();
  void testResetFirstMatchingSeqs();
  void testGetSegStatusVec();
  void testAnalyzeSeg();
  void testFlowJourneyCarrier();
  void testLocalJourneyCarrier();
  void testTryLocalWithFlowAvail();
  void testValidateStartDate();
  void testValidateStopDate();
  void testPartOfJourney();
  void testJourneyExistInItin();
  void testStatusToAsBooked();
  void testStatusFromAsBooked();
  void testCheckBookedClassAvail();
  void testCheckBookedClassOffer();
  void testGetAvailability();
  void testTagFltFAILTagN();
  void testValidateCabinForDifferential();
  void testIsSegmentNoMatched_Yes();
  void testIsSegmentNoMatched_No();
  void testIsAllFlightsSkipped_Yes();
  void testIsAllFlightsSkipped_No();
  void testIsAllFlightsMatched_Yes();
  void testIsAllFlightsMatched_No();
  void testValidateSequenceForBothRBDs_False();
  void testValidateSequenceForBothRBDs_True();
  void testValidateSequenceForBothRBDs_False_S2RBD2Empty();
  void testValidateSequenceForBothRBDs_False_S1RBD2Empty();
  void testValidateWPNCS();
  void testAllSegsStatusQF();
  void testSkipCat31Flown();
  void testValidateWPNCSFromPrimeRBD();
  void testResetAsBooked();

  static const BookingCode AVAILABLE_BOOKING_CODE1;
  static const BookingCode AVAILABLE_BOOKING_CODE2;
  static const BookingCode BOOKING_CODE1_NOT_AVAILABLE;
  static const BookingCode BOOKING_CODE2_NOT_AVAILABLE;
  static const BookingCode BOOKED_BOOKING_CODE;

private:
  PaxTypeFare* createPTF(std::string paxTypeCode, std::string carrier);

  void setRequestAsWP();
  void setRequestAsWPNC();
  void setTrxAsMip();
  void setTrxAsPricing();
  void initializePaxTypeFare();
  void initializeTvlSegAvail();
  void setSegStatusVec();

  std::string _expected;
  Itin* _itin;
  PricingTrx* _trx;
  PricingRequest* _request;
  Billing* _billing;
  BookingCodeExceptionSegment* _bceSegment;
  AirSeg* _airSeg;
  PaxTypeFare* _paxTypeFare;
  FareInfo* _fareInfo;
  CarrierPreference* _cxrPref;
  PaxType* _paxType;
  PaxTypeInfo* _pInfo;
  PaxTypeFare::SegmentStatusVec _segStatVec;
  Agent* _agent;
  Customer* _cust;
  PricingOptions* _pop;
  TestMemHandle _memHandle;
};

class BookingCodeExceptionValidatorStubValidateBC : public BookingCodeExceptionValidator
{
public:
  BookingCodeExceptionValidatorStubValidateBC(Itin& itin,
                                              bool partOfLocalJny,
                                              bool skipFlownSegCat31);
  bool _validateShouldReturnWP;
  bool _validateShouldReturnWPNC;
  uint16_t _lastAirIndex;

  bool validateWP(PricingTrx& trx,
                  const BookingCodeExceptionSegment& bceSegment,
                  const TravelSeg* tvlSeg,
                  PaxTypeFare::SegmentStatusVec& segStatVec,
                  const uint16_t airIndex,
                  PaxTypeFare& paxTypeFare,
                  bool rbd2Only = false);

  bool validateWPNC(const BookingCodeExceptionSegment& bceSegment,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    const uint16_t airIndex,
                    PaxTypeFare& paxTypeFare,
                    PricingTrx& trx,
                    bool rbd2Only = false);

  BookingCodeValidationStatus validateWPNC_NEW(const BookingCodeExceptionSegment& bceSegment,
                                               const TravelSeg* tvlSeg,
                                               PaxTypeFare::SegmentStatusVec& segStatVec,
                                               const uint16_t airIndex,
                                               PaxTypeFare& paxTypeFare,
                                               PricingTrx& trx,
                                               bool rbd2Only = false);

};
} // end Namespace tse

#endif // BOOKING_CODE_EXCEPTION_VALIDATOR_TEST_H
