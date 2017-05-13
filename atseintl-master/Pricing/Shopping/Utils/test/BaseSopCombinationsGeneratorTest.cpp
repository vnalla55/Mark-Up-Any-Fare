//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Utils/SopCombinationsGenerator.h"

#include <memory>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace utils
{

class MockBaseSopCombinationsGenerator :
    public BaseSopCombinationsGenerator {
 public:
  MOCK_METHOD0(initialize,
      void());
  MOCK_METHOD0(nextElement,
      SopCombination());
};

class BaseSopCombinationsGeneratorTest: public Test
{
public:
  void SetUp()
  {
    _generator.reset(new MockBaseSopCombinationsGenerator());
  }

  void TearDown(){}

  void testInit()
  {
    EXPECT_CALL(*_generator, initialize())
        .Times(1);
    _generator->manualInit();
  }

  std::shared_ptr<MockBaseSopCombinationsGenerator> _generator;

  SopCombination DUMMY_NEXT_ELEMENT;
};


TEST_F(BaseSopCombinationsGeneratorTest, callsOnceInitializeThenAlwaysNextElement)
{
  {
    InSequence dummy;
    EXPECT_CALL(*_generator, initialize())
      .Times(1);
    EXPECT_CALL(*_generator, nextElement())
      .Times(5)
      .WillRepeatedly(Return(DUMMY_NEXT_ELEMENT));
  }
  _generator->next();
  _generator->next();
  _generator->next();
  _generator->next();
  _generator->next();
}


TEST_F(BaseSopCombinationsGeneratorTest, manualInitInitializes)
{
  EXPECT_CALL(*_generator, initialize())
      .Times(1);
  _generator->manualInit();
}


TEST_F(BaseSopCombinationsGeneratorTest, manualInitOnlyInitializes)
{
  testInit();
}

TEST_F(BaseSopCombinationsGeneratorTest, afterManualInitOnlyNextElementCalled)
{
  testInit();
  EXPECT_CALL(*_generator, nextElement())
    .Times(1)
    .WillRepeatedly(Return(DUMMY_NEXT_ELEMENT));
  _generator->next();
}


} // namespace utils

} // namespace tse

