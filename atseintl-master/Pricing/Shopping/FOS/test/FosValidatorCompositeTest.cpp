// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include <sstream>

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/BaseValidator.h"
#include "Pricing/Shopping/FOS/FosValidatorComposite.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{

class FosBaseGenerator;
class FosStatistic;

class MockValidator : public BaseValidator
{
public:
  MockValidator(ShoppingTrx& trx,
                FosBaseGenerator& generator,
                FosStatistic& stats,
                ValidatorType vt,
                ValidationResult validres,
                bool throwAway)
    : BaseValidator(trx, generator, stats),
      _vt(vt),
      _validationResult(validres),
      _throwAway(throwAway)
  {
  }

  virtual ValidatorType getType() const { return _vt; }
  virtual ValidationResult validate(const SopIdVec& combination) const { return _validationResult; }
  virtual bool isThrowAway(const SopIdVec& combination, ValidatorBitMask validBitMask) const
  {
    return _throwAway;
  }

private:
  ValidatorType _vt;
  ValidationResult _validationResult;
  bool _throwAway;
};

// ==================================
// TEST CLASS
// ==================================

class FosValidatorCompositeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FosValidatorCompositeTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testIsThrowAway);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidate()
  {
    FosBaseGenerator* generator = 0;
    FosStatistic* statistic = 0;
    FosValidatorComposite validatorComposite;

    ValidatorBitMask validBitMask = 1234;
    ValidatorBitMask deferredBitMask = 4321;
    ValidatorBitMask invalidSopDetailsBitMask = 5678;

    validatorComposite.validate(
        SopIdVec(), validBitMask, deferredBitMask, invalidSopDetailsBitMask);

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), validBitMask);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), deferredBitMask);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), invalidSopDetailsBitMask);

    MockValidator mv1(*_trx, *generator, *statistic, VALIDATOR_ONLINE, BaseValidator::VALID, true);
    MockValidator mv2(*_trx,
                      *generator,
                      *statistic,
                      VALIDATOR_TRIANGLE,
                      BaseValidator::INVALID_SOP_DETAILS,
                      true);
    MockValidator mv3(
        *_trx, *generator, *statistic, VALIDATOR_CUSTOM, BaseValidator::DEFERRED, true);

    validatorComposite.addValidator(mv1);
    validatorComposite.addValidator(mv2);
    validatorComposite.addValidator(mv3);
    validatorComposite.validate(
        SopIdVec(), validBitMask, deferredBitMask, invalidSopDetailsBitMask);

    CPPUNIT_ASSERT_EQUAL(validatorBitMask(VALIDATOR_ONLINE), validBitMask);
    CPPUNIT_ASSERT_EQUAL(validatorBitMask(VALIDATOR_CUSTOM), deferredBitMask);
    CPPUNIT_ASSERT_EQUAL(validatorBitMask(VALIDATOR_TRIANGLE), invalidSopDetailsBitMask);
  }

  void testIsThrowAway()
  {
    FosBaseGenerator* generator = 0;
    FosStatistic* statistic = 0;
    FosValidatorComposite validatorComposite;

    CPPUNIT_ASSERT(!validatorComposite.isThrowAway(SopIdVec(), 0));

    MockValidator mv1(*_trx, *generator, *statistic, VALIDATOR_ONLINE, BaseValidator::VALID, false);
    MockValidator mv2(
        *_trx, *generator, *statistic, VALIDATOR_CUSTOM, BaseValidator::DEFERRED, true);

    validatorComposite.addValidator(mv1);
    validatorComposite.addValidator(mv2);
    CPPUNIT_ASSERT(validatorComposite.isThrowAway(SopIdVec(), 0));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FosValidatorCompositeTest);

} // fos
} // tse
