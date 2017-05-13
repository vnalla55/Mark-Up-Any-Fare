// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include "Util/TaggedTuple.h"

#include <memory>
#include <string>

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

namespace tse
{
class TaggedTupleTest : public ::testing::Test
{
protected:
  struct Name : Tag<std::string> {};
  struct Age : Tag<int> {};
  struct Weight : Tag<double> {};
  struct PointerTag : Tag<std::unique_ptr<int>> {};
};

TEST_F(TaggedTupleTest, testGet_ConstRef)
{
  const TaggedTuple<Name, Age, Weight> t("John", 20, 80.0);
  EXPECT_EQ(&std::get<0>(t), &t.get<Name>());
  EXPECT_EQ(&std::get<1>(t), &t.get<Age>());
  EXPECT_EQ(&std::get<2>(t), &t.get<Weight>());
  EXPECT_EQ(std::string("John"), t.get<Name>());
  EXPECT_EQ(20, t.get<Age>());
  EXPECT_EQ(80.0, t.get<Weight>());
}

TEST_F(TaggedTupleTest, testGet_Ref)
{
  TaggedTuple<Name, Age, Weight> t("John", 20, 80.0);
  EXPECT_EQ(&std::get<0>(t), &t.get<Name>());
  EXPECT_EQ(&std::get<1>(t), &t.get<Age>());
  EXPECT_EQ(&std::get<2>(t), &t.get<Weight>());
  t.get<Name>() = "David";
  EXPECT_EQ(std::string("David"), t.get<Name>());
  EXPECT_EQ(20, t.get<Age>());
  EXPECT_EQ(80.0, t.get<Weight>());
}

TEST_F(TaggedTupleTest, testGet_Order)
{
  using SomeTuple = TaggedTuple<Name, Age>;
  EXPECT_TRUE(SomeTuple("AAA", 10) == SomeTuple("AAA", 10));
  EXPECT_TRUE(SomeTuple("AAA", 10) != SomeTuple("AAZ", 10));
  EXPECT_TRUE(SomeTuple("AAA", 10) <= SomeTuple("AAA", 10));
  EXPECT_TRUE(SomeTuple("AAA", 10) >= SomeTuple("AAA", 10));
  EXPECT_TRUE(SomeTuple("AAA", 10) < SomeTuple("AAA", 11));
  EXPECT_TRUE(SomeTuple("AAA", 10) <= SomeTuple("AAA", 11));
  EXPECT_TRUE(SomeTuple("AAZ", 10) > SomeTuple("AAA", 10));
  EXPECT_TRUE(SomeTuple("AAZ", 10) >= SomeTuple("AAA", 10));
}
}
