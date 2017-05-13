//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/Shopping/Utils/StatusAccumulator.h"

#include <memory>

namespace tse
{

namespace utils
{

namespace
{
enum DummyEnum
{
  FIRST_VALUE = 0,
  SECOND_VALUE,
  THIRD_VALUE
};
}

struct AlwaysOverwrite
{
  static DummyEnum filter(const DummyEnum& oldStatus, const DummyEnum& newStatus)
  {
    return newStatus;
  }
};

struct NeverOverwrite
{
  static DummyEnum filter(const DummyEnum& oldStatus, const DummyEnum& newStatus)
  {
    return oldStatus;
  }
};

class StatusAccumulatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StatusAccumulatorTest);
  CPPUNIT_TEST(initialStatusTest);
  CPPUNIT_TEST(updateAndOverwriteTest);
  CPPUNIT_TEST(updateAndNoOverwriteTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _acc.reset(new Accumulator());
    _alwaysAcc.reset(new UpdatingAccumulator());
    _neverAcc.reset(new FrozenAccumulator());
  }

  void tearDown()
  {
    _acc.reset();
    _alwaysAcc.reset();
    _neverAcc.reset();
  }

  void initialStatusTest() { CPPUNIT_ASSERT(_acc->getStatus() == FIRST_VALUE); }

  void updateAndOverwriteTest()
  {
    _alwaysAcc->updateStatus(THIRD_VALUE);
    CPPUNIT_ASSERT(_alwaysAcc->getStatus() == THIRD_VALUE);
  }

  void updateAndNoOverwriteTest()
  {
    _neverAcc->updateStatus(THIRD_VALUE);
    CPPUNIT_ASSERT(_neverAcc->getStatus() == FIRST_VALUE);
  }

private:
  typedef StatusAccumulator<DummyEnum> Accumulator;
  typedef StatusAccumulator<DummyEnum, ZeroIntegerInitPolicy, AlwaysOverwrite> UpdatingAccumulator;
  typedef StatusAccumulator<DummyEnum, ZeroIntegerInitPolicy, NeverOverwrite> FrozenAccumulator;

  std::shared_ptr<Accumulator> _acc;
  std::shared_ptr<UpdatingAccumulator> _alwaysAcc;
  std::shared_ptr<FrozenAccumulator> _neverAcc;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StatusAccumulatorTest);

} // namespace utils

} // namespace tse
