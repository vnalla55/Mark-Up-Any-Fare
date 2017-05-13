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

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DataModel/FarePath.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/EmbargoesValidator.h"
#include "FreeBagService/test/S7Builder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class EmbargoesValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EmbargoesValidatorTest);

  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_X);
  CPPUNIT_TEST(testCheckServiceNotAvailNoCharge_NotX);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  EmbargoesValidator* _validator;
  BaggageTravel* _baggageTravel;

public:
  void setUp()
  {
    _memHandle(new TestConfigInitializer);
    _baggageTravel = _memHandle.create<BaggageTravel>();
    _validator = createEmbargoesValidator();
  }

  void tearDown() { _memHandle.clear(); }

  EmbargoesValidator* createEmbargoesValidator()
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
    return _memHandle.insert(new EmbargoesValidator(opt));
  }

  void testCheckServiceNotAvailNoCharge_X()
  {
    OCFees fees;
    CPPUNIT_ASSERT_EQUAL(
        PASS_S7,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_NOT_AVAILABLE)
                .buildRef(),
            fees));
  }

  void testCheckServiceNotAvailNoCharge_NotX()
  {
    OCFees fees;
    OptionalServicesInfo s7;

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED)
                .buildRef(),
            fees));

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_EMD_ISSUED)
                .buildRef(),
            fees));

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_BOOK_NO_EMD)
                .buildRef(),
            fees));

    CPPUNIT_ASSERT_EQUAL(
        FAIL_S7_NOT_AVAIL_NO_CHANGE,
        _validator->checkServiceNotAvailNoCharge(
            S7Builder(&_memHandle)
                .withNotAvailNoCharge(OptionalServicesValidator::SERVICE_FREE_NO_BOOK_EMD_ISSUED)
                .buildRef(),
            fees));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EmbargoesValidatorTest);
}
