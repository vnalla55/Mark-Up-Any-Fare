// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/TseConsts.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/FarePath.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/CarryOnBaggageAllowanceValidator.h"
#include "FreeBagService/test/S7Builder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class CarryOnBaggageAllowanceValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarryOnBaggageAllowanceValidatorTest);
  CPPUNIT_TEST(testCheckSectorPortionInd_S);
  CPPUNIT_TEST(testCheckSectorPortionInd_NOT_S);
  CPPUNIT_TEST(testCheckSectorPortionInd_BLANC_loc1_BLANK);
  CPPUNIT_TEST(testCheckSectorPortionInd_BLANC_loc2_BLANK);
  CPPUNIT_TEST(testCheckSectorPortionInd_BLANC_loc3_BLANK);
  CPPUNIT_TEST(testCheckSectorPortionInd_BLANC_loc1_BLANK_loc2_BLANK_loc3_BLANK);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_F);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NOT_F);

  CPPUNIT_TEST(testCheckIntermediatePoint_VIA_LOC_NULL);
  CPPUNIT_TEST(testCheckStopCnxDestInd_empty);

  CPPUNIT_TEST(testValidateCnxOrStopover_empty);
  CPPUNIT_TEST(testValidateCnxOrStopover_NOT_empty);

  CPPUNIT_TEST(testCheckOccurrenceBlankBlank_NOT_BLANK_NOT_BLANK);
  CPPUNIT_TEST(testCheckOccurrenceBlankBlank_NOT_BLANK_BLANK);
  CPPUNIT_TEST(testCheckOccurrenceBlankBlank_BLANK_NOT_BLANK);
  CPPUNIT_TEST(testCheckOccurrenceBlankBlank_BLANK_BLANK);

  CPPUNIT_TEST(testCheckFeeApplication_BLANK);
  CPPUNIT_TEST(testCheckFeeApplication_NOT_BLANK);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  CarryOnBaggageAllowanceValidator* _validator;
  BaggageTravel* _baggageTravel;

public:
  void setUp()
  {
    _baggageTravel = _memHandle.create<BaggageTravel>();
    _memHandle.create<TestConfigInitializer>();
    _validator = createCarryOnBaggageAllowanceValidator();
  }

  void tearDown() { _memHandle.clear(); }

  CarryOnBaggageAllowanceValidator* createCarryOnBaggageAllowanceValidator()
  {
    Itin* itin = _memHandle.create<Itin>();
    itin->setBaggageTripType(BaggageTripType::TO_FROM_US);
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = itin;

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());

    _baggageTravel->setupTravelData(*_farePath);
    _baggageTravel->_trx = _trx;

    CheckedPoint* checkedPoint = _memHandle.create<CheckedPoint>();
    Ts2ss* ts2ss = _memHandle.create<Ts2ss>();
    BagValidationOpt opt(*_baggageTravel, *checkedPoint, *ts2ss, false, nullptr);
    return _memHandle.insert(new CarryOnBaggageAllowanceValidator(opt));
  }

  void testCheckSectorPortionInd_S()
  {
    CPPUNIT_ASSERT(_validator->checkSectorPortionInd(
        S7Builder(&_memHandle).withSectorPortion('S').buildRef()));
  }

  void testCheckSectorPortionInd_NOT_S()
  {
    CPPUNIT_ASSERT(!_validator->checkSectorPortionInd(
        S7Builder(&_memHandle).withSectorPortion('P').buildRef()));
  }

  void testCheckSectorPortionInd_BLANC_loc1_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkSectorPortionInd(
        S7Builder(&_memHandle)
            .withSectorPortion(' ')
            .withLocations(EMPTY_STRING(), IATA_AREA1, IATA_AREA2)
            .buildRef()));
  }

  void testCheckSectorPortionInd_BLANC_loc2_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkSectorPortionInd(
        S7Builder(&_memHandle)
            .withSectorPortion(' ')
            .withLocations(IATA_AREA1, EMPTY_STRING(), IATA_AREA2)
            .buildRef()));
  }

  void testCheckSectorPortionInd_BLANC_loc3_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkSectorPortionInd(
        S7Builder(&_memHandle)
            .withSectorPortion(' ')
            .withLocations(IATA_AREA1, IATA_AREA2, EMPTY_STRING())
            .buildRef()));
  }

  void testCheckSectorPortionInd_BLANC_loc1_BLANK_loc2_BLANK_loc3_BLANK()
  {
    CPPUNIT_ASSERT(_validator->checkSectorPortionInd(
        S7Builder(&_memHandle)
            .withSectorPortion(' ')
            .withLocations(EMPTY_STRING(), EMPTY_STRING(), EMPTY_STRING())
            .buildRef()));
  }

  void testCheckServiceNotAvailNoCharge_F()
  {
    OCFees fees;

    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
                .buildRef(),
            fees));
  }

  void testCheckServiceNotAvailNoCharge_NOT_F()
  {
    OCFees fees;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_EMD_ISSUED)
                .buildRef(),
            fees));
  }

  void testCheckIntermediatePoint_VIA_LOC_NULL()
  {
    LocKey emptyLocKey;
    std::vector<TravelSeg*> travelSegments;

    CPPUNIT_ASSERT(_validator->checkIntermediatePoint(
        S7Builder(&_memHandle).withViaLoc(emptyLocKey).buildRef(), travelSegments));
  }

  void testCheckStopCnxDestInd_empty()
  {
    std::vector<TravelSeg*> travelSegments;

    CPPUNIT_ASSERT(_validator->checkStopCnxDestInd(
        S7Builder(&_memHandle).withStopCnxDest(' ').buildRef(), travelSegments));
  }

  void testCheckStopCnxDestInd_NOT_empty()
  {
    std::vector<TravelSeg*> travelSegments;

    CPPUNIT_ASSERT(!_validator->checkStopCnxDestInd(
        S7Builder(&_memHandle).withStopCnxDest('P').buildRef(), travelSegments));
  }

  void testValidateCnxOrStopover_empty()
  {
    std::vector<TravelSeg*> travelSegments;

    CPPUNIT_ASSERT(_validator->validateCnxOrStopover(
        S7Builder(&_memHandle).withStopoverTime("").buildRef(), travelSegments, false));
  }

  void testValidateCnxOrStopover_NOT_empty()
  {
    std::vector<TravelSeg*> travelSegments;

    CPPUNIT_ASSERT(!_validator->validateCnxOrStopover(
        S7Builder(&_memHandle).withStopoverTime("012").buildRef(), travelSegments, false));
  }

  void testCheckOccurrenceBlankBlank_NOT_BLANK_NOT_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrenceBlankBlank(
        S7Builder(&_memHandle).withBaggageOccurrence(1, 2).buildRef()));
  }

  void testCheckOccurrenceBlankBlank_NOT_BLANK_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrenceBlankBlank(
        S7Builder(&_memHandle).withBaggageOccurrence(1, 0).buildRef()));
  }

  void testCheckOccurrenceBlankBlank_BLANK_NOT_BLANK()
  {
    CPPUNIT_ASSERT(!_validator->checkOccurrenceBlankBlank(
        S7Builder(&_memHandle).withBaggageOccurrence(0, 2).buildRef()));
  }

  void testCheckOccurrenceBlankBlank_BLANK_BLANK()
  {
    CPPUNIT_ASSERT(_validator->checkOccurrenceBlankBlank(
        S7Builder(&_memHandle).withBaggageOccurrence(0, 0).buildRef()));
  }

  void testCheckFeeApplication_BLANK()
  {
    CPPUNIT_ASSERT(
        _validator->checkFeeApplication(S7Builder(&_memHandle).withApplication(' ').buildRef()));
  }

  void testCheckFeeApplication_NOT_BLANK()
  {
    CPPUNIT_ASSERT(
        !_validator->checkFeeApplication(S7Builder(&_memHandle).withApplication('3').buildRef()));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnBaggageAllowanceValidatorTest);
}
