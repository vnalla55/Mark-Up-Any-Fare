//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
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

#include <string>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"
#include "Common/Utils/Pprint.h"

#include <iostream>
#include <vector>
#include <boost/regex.hpp>

using namespace ::testing;

namespace tse
{

namespace tools
{

class PprintTest: public Test
{
public:
  void SetUp(){}
  void TearDown(){}
};

struct Foo
{
  int n;
};

void pprint_impl(std::ostream& out, const Foo& foo)
{
  out << "This is Foo: " << foo.n;
}

TEST_F(PprintTest, testPrintsInt)
{
  ASSERT_EQ("5", pformat(int(5)));
}

TEST_F(PprintTest, testPrintsFloat)
{
  ASSERT_EQ("3.1415", pformat(float(3.1415)));
}

TEST_F(PprintTest, testPrintsIntPtr)
{
  int* p = new int(5);
  ASSERT_TRUE(boost::regex_match(pformat(p), boost::regex("0x[[:xdigit:]]+ -> 5")));
  delete p;
}

TEST_F(PprintTest, testPrintsVector)
{
  ASSERT_EQ("[1, 2, 3]", pformat(std::vector<int>{1, 2, 3}));
}

TEST_F(PprintTest, testPrintsVectorOfPtrs)
{
  std::vector<float*> v = {new float(3.1), new float(4.15), new float(9.2)};
  ASSERT_TRUE(boost::regex_match(
      pformat(v),
      boost::regex("\\[0x[[:xdigit:]]+ -> 3.1, 0x[[:xdigit:]]+ -> 4.15, 0x[[:xdigit:]]+ -> 9.2\\]")));
  for (auto p: v)
  {
    delete p;
  }
}

TEST_F(PprintTest, testPrintsClassWithPprintImpl)
{
  ASSERT_EQ("This is Foo: 5", pformat(Foo{5}));
}

} // namespace tools

} // namespace tse

