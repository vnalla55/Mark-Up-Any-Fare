//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
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

#include "Common/TseConsts.h"
#include "Common/TNBrands/TNBrandsUtils.h"

#include <memory>
#include <string>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class TNBrandsUtilsTest: public Test
{
public:
  void SetUp(){}

  void TearDown(){}
};

class StringWithDefaultName: public std::string
{
public:
  static const std::string DEFAULT_NAME;
  StringWithDefaultName(const std::string& text = ""):
    std::string(text)
  {
    if (this->empty())
    {
      (*this) = DEFAULT_NAME;
    }
  }
};

const std::string StringWithDefaultName::DEFAULT_NAME = "Beethoven";

TEST_F(TNBrandsUtilsTest, testAssignsObjectDirectlyIfNonZero)
{
  std::shared_ptr<StringWithDefaultName> smartPointer;
  StringWithDefaultName* dummyTextPointer = new StringWithDefaultName("Mozart");

  assignValidObject(dummyTextPointer, smartPointer);

  ASSERT_EQ(dummyTextPointer, smartPointer.get());
}

TEST_F(TNBrandsUtilsTest, testCreatesNewObjectIfZero)
{
  std::shared_ptr<StringWithDefaultName> smartPointer;
  assignValidObject(static_cast<StringWithDefaultName*>(0), smartPointer);

  ASSERT_NE(static_cast<StringWithDefaultName*>(0), smartPointer.get());
  ASSERT_EQ(StringWithDefaultName::DEFAULT_NAME, *smartPointer);
}

} // namespace skipper

} // namespace tse
