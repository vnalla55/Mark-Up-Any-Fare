//----------------------------------------------------------------
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
//-----------------------------------------------------------------

#include "Util/NullTermVector.h"

#include <gtest/gtest.h>
#include <list>
#include <stdint.h>
#include <vector>
#include "test/include/GtestHelperMacros.h"

namespace tse
{
class NullTermVectorTest : public ::testing::Test
{
protected:
  template<class T>
  struct NullEquals5Policy
  {
    static T getNull() { return 5u; }
    static bool isNull(const T& v) { return v == 5u; }
  };
};

TEST_F(NullTermVectorTest, testEmpty)
{
  NullTermVector<int, 3> v, vcopy(v);
  EXPECT_TRUE(v.empty());
  EXPECT_TRUE(vcopy.empty());

  v[1] = v[2] = 1;
  EXPECT_TRUE(v.empty());

  v[0] = 1;
  EXPECT_FALSE(v.empty());
}

TEST_F(NullTermVectorTest, testSizeCapacity)
{
  NullTermVector<int, 5> v;
  EXPECT_EQ(size_t(0), v.size());
  EXPECT_EQ(size_t(5), v.capacity());

  v[0] = 1;
  EXPECT_EQ(size_t(1), v.size());
  v[2] = 3;
  EXPECT_EQ(size_t(1), v.size());
  v[1] = 2;
  EXPECT_EQ(size_t(3), v.size());

  v.fill(5);
  EXPECT_EQ(size_t(5), v.size());

  v.clear();
  EXPECT_EQ(size_t(0), v.size());
}

TEST_F(NullTermVectorTest, testIterators)
{
  NullTermVector<int, 5> v;
  v[0] = 1;
  v[1] = 2;
  v[3] = 4;
  EXPECT_EQ(int(1), *v.begin());
  EXPECT_EQ(int(2), *v.rbegin());
  EXPECT_TRUE(v.end() == v.begin() + 2);
  EXPECT_TRUE(v.rend() == v.rbegin() + 2);
}

TEST_F(NullTermVectorTest, testIteratorsConsistency)
{
  NullTermVector<int, 5> v;
  const NullTermVector<int, 5>& vconst = v;
  v[0] = 1;
  v[1] = 2;
  v[2] = 3;

  EXPECT_EQ(*v.begin(), *vconst.begin());
  EXPECT_EQ(*v.cbegin(), *vconst.begin());
  EXPECT_TRUE(v.cbegin() == vconst.begin());

  EXPECT_EQ(*v.end(), *vconst.end());
  EXPECT_EQ(*v.cend(), *vconst.end());
  EXPECT_TRUE(v.cend() == vconst.end());

  EXPECT_EQ(*v.rbegin(), *vconst.rbegin());
  EXPECT_EQ(*v.crbegin(), *vconst.rbegin());
  EXPECT_TRUE(v.crbegin() == vconst.rbegin());

  EXPECT_EQ(*v.rend(), *vconst.rend());
  EXPECT_EQ(*v.crend(), *vconst.rend());
  EXPECT_TRUE(v.crend() == vconst.rend());
}

TEST_F(NullTermVectorTest, testFrontBack)
{
  NullTermVector<int, 5> v;
  const NullTermVector<int, 5>& vconst = v;
  v.fill(100);
  v[0] = 1;
  v[2] = 3;
  v[3] = 0;
  EXPECT_EQ(int(1), v.front());
  EXPECT_EQ(int(1), vconst.front());
  EXPECT_EQ(int(3), v.back());
  EXPECT_EQ(int(3), vconst.back());
  v.back() = 4;
  EXPECT_EQ(int(4), vconst.back());
  v.front() = 0;
  EXPECT_TRUE(v.empty());
}

TEST_F(NullTermVectorTest, testAt)
{
  NullTermVector<int, 3> v;
  const NullTermVector<int, 3>& vconst = v;
  v.fill(100);
  v[0] = 1;
  v[1] = 0;

  EXPECT_EQ(int(100), v.at(2));
  EXPECT_EQ(int(100), vconst.at(2));
  EXPECT_EQ(int(0), v.at(1));
  EXPECT_EQ(int(0), vconst.at(1));
  EXPECT_EQ(int(1), v.at(0));
  EXPECT_EQ(int(1), vconst.at(0));
  v.at(2) = 200;
  EXPECT_EQ(int(200), vconst[2]);
  EXPECT_ANY_THROW(v.at(3));
}

TEST_F(NullTermVectorTest, testEquals)
{
  NullTermVector<int, 3> l, r;
  l.fill(100);
  r.fill(200);
  EXPECT_TRUE(l != r);
  EXPECT_FALSE(l == r);

  l[0] = r[0] = 300;
  l[1] = r[1] = 0;
  EXPECT_TRUE(l == r);

  l.fill(100);
  r.fill(200);
  r[2] = 0;
  EXPECT_FALSE(l == r);

  r = l;
  EXPECT_TRUE(l == r);
}

TEST_F(NullTermVectorTest, testCompare_Equal)
{
  NullTermVector<int, 3> l, r;
  l.fill(100);
  r.fill(100);

  EXPECT_FALSE(l < r);
  EXPECT_FALSE(l > r);
  EXPECT_TRUE(l <= r);
  EXPECT_TRUE(l >= r);
}

TEST_F(NullTermVectorTest, testCompare_Diff)
{
  NullTermVector<int, 3> l, r;
  l.fill(100);
  r.fill(100);
  r[2] = 0;

  EXPECT_FALSE(l < r);
  EXPECT_TRUE(l > r);
  EXPECT_FALSE(l <= r);
  EXPECT_TRUE(l >= r);

  l[1] = 99;

  EXPECT_TRUE(l < r);
  EXPECT_FALSE(l > r);
  EXPECT_TRUE(l <= r);
  EXPECT_FALSE(l >= r);
}

TEST_F(NullTermVectorTest, testOtherPolicy)
{
  NullTermVector<uint32_t, 5, NullEquals5Policy> v;
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(uint32_t(5), v.front());
  v[0] = 100;
  v[1] = 0;
  EXPECT_EQ(size_t(2), v.size());
  v.fill(0);
  EXPECT_EQ(size_t(5), v.size());
}

TEST_F(NullTermVectorTest, testConstructorVecRange)
{
  std::vector<size_t> dynv;
  dynv.push_back(1u);
  dynv.push_back(2u);
  dynv.push_back(3u);

  NullTermVector<size_t, 5> v(dynv);
  EXPECT_EQ(size_t(1), v[0]);
  EXPECT_EQ(size_t(2), v[1]);
  EXPECT_EQ(size_t(3), v[2]);
  EXPECT_EQ(size_t(0), v[3]);
  EXPECT_EQ(size_t(0), v[4]);

  std::list<size_t> l(v.begin(), v.end());
  NullTermVector<size_t, 5> v2(l.begin(), l.end());
  EXPECT_TRUE(v == v2);
}

TEST_F(NullTermVectorTest, testConstructorVal)
{
  NullTermVector<size_t, 3> v(10u);
  EXPECT_EQ(size_t(10), v[0]);
  EXPECT_EQ(size_t(10), v[1]);
  EXPECT_EQ(size_t(10), v[2]);

  NullTermVector<size_t, 4> v2(2u, 10u);
  EXPECT_EQ(size_t(10), v2[0]);
  EXPECT_EQ(size_t(10), v2[1]);
  EXPECT_EQ(size_t(0), v2[2]);
  EXPECT_EQ(size_t(0), v2[3]);
}

TEST_F(NullTermVectorTest, testAssignVal)
{
  NullTermVector<size_t, 4u> v;
  v.fill(10u);
  v.assign(2u, 5u);
  EXPECT_EQ(size_t(5), v[0]);
  EXPECT_EQ(size_t(5), v[1]);
  EXPECT_EQ(size_t(0), v[2]);
  EXPECT_EQ(size_t(0), v[3]);
  ASSERT_ANY_THROW(v.assign(5u, 5u));
}

TEST_F(NullTermVectorTest, testAssignRange)
{
  NullTermVector<size_t, 4u> v;
  v.fill(10u);
  std::list<size_t> l;
  l.push_back(1);
  l.push_back(2);
  v.assign(l.begin(), l.end());
  EXPECT_EQ(size_t(1), v[0]);
  EXPECT_EQ(size_t(2), v[1]);
  EXPECT_EQ(size_t(0), v[2]);
  EXPECT_EQ(size_t(0), v[3]);
  l.assign(5u, 5u);
  ASSERT_ANY_THROW(v.assign(l.begin(), l.end()));
}

}
