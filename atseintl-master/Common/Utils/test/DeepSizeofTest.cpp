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
#include "Common/Utils/DeepSizeof.h"

#include <vector>

using namespace ::testing;

namespace tse
{

namespace tools
{

class DeepSizeofTest: public Test
{
public:
  void SetUp(){}
  void TearDown(){}
};

struct Member
{
  uint64_t a;
  uint64_t b;
  uint64_t c;
};

// Total size:
// 3 words
// 1 vpointer = 1 word
// 1 vector = 3 words
// N * (pointer + Member) = N * 4 words
// total: 7 + 4N words = 56 + 32N B
struct Composite
{
  uint64_t x;
  uint64_t y;
  std::vector<Member*> members;
  uint64_t z;
  virtual void foo(){};  // for vpointer
  virtual ~Composite()
  {
    for (Member* m: members)
    {
      delete m;
    }
  }
};



Composite* factory(size_t n)
{
  Composite* c = new Composite();
  for (size_t i = 0; i < n; ++i)
  {
    c->members.push_back(new Member());
  }
  return c;
}

size_t deep_sizeof_impl(const Composite& c)
{
  size_t sz = sizeof(c);
  for (auto m: c.members)
  {
    sz += deep_sizeof(m);
  }
  return sz;
}

size_t deep_sizeof_impl(const Member& m)
{
  return sizeof(m);
}

TEST_F(DeepSizeofTest, testCorrectResultForComposite)
{
  Composite* c = factory(3);
  // 56 + 32*3 = 152
  ASSERT_EQ(152, deep_sizeof(*c));
  delete c;
}

TEST_F(DeepSizeofTest, testCorrectResultForConstComposite)
{
  const Composite* c = factory(5);
  // 56 + 32*5 = 216
  ASSERT_EQ(216, deep_sizeof(*c));
  delete c;
}

TEST_F(DeepSizeofTest, testCorrectResultForCompositePtr)
{
  Composite* c = factory(7);
  // 56 + 32*7 + 8(ptr) = 288
  ASSERT_EQ(288, deep_sizeof(c));
  delete c;
}

TEST_F(DeepSizeofTest, testCorrectResultForConstCompositePtr)
{
  const Composite* c = factory(9);
  // 56 + 32*9 + 8(ptr) = 352
  ASSERT_EQ(352, deep_sizeof(c));
  delete c;
}

} // namespace tools

} // namespace tse

