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
#include "Common/Utils/SharingSet.h"

#include <boost/core/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <string>
#include <sstream>

using namespace ::testing;
using ::testing::_;
using ::testing::StrictMock;

namespace tse
{

namespace tools
{

template <class T>
struct Integer: private boost::noncopyable
{
  Integer(T& a_t, int a_n): t(a_t), n(a_n)
  {
    ++t.population;
  }

  ~Integer()
  {
    --t.population;
  }

  bool operator==(const Integer& rhs) const
  {
    return n == rhs.n;
  }

  T& t;
  int n;
};

static constexpr size_t DUMMY_SIZEOF = 100;

template <class T>
size_t deep_sizeof_impl(const Integer<T>& i)
{
  return DUMMY_SIZEOF;
}

template <class T>
std::ostream& pprint_impl(std::ostream& out, const Integer<T>& i)
{
  return out << "<" << i.n << ">";
}

template <class T>
inline std::size_t hash_value(const Integer<T>& i)
{
  return boost::hash_value(i.n);
}

class SharingSetTest: public Test
{
public:
  typedef Integer<SharingSetTest> Int;
  typedef SharingSet<Int> ShSet;


  Int* produce(int n)
  {
    return new Int(*this, n);
  }

  Int* insert(int n)
  {
    Int* p = _set->insert(produce(n));
    integers.push_back(p);
    return p;
  }

  void insertPtr(Int* p, Int* expected_return, size_t expected_uc,
                       size_t expected_total_uc, size_t expected_size,
                       size_t expected_population)
  {
    Int* q = _set->insert(p);
    integers.push_back(q);
    ASSERT_EQ(expected_return, q);
    // p may be deallocated now: do not use
    assureStatus(q, expected_uc, expected_total_uc, expected_size,
                 expected_population);
  }

  void eraseNumbers()
  {
    for (auto* ptr: integers)
    {
      const bool deleted = _set->erase(ptr);
      // all pointers here should be owned by the set
      ASSERT_TRUE(deleted);
    }
  }

  void assureStatus(Int* p, size_t expected_uc, size_t expected_total_uc,
                    size_t expected_size, size_t expected_population)
  {
    ASSERT_EQ(expected_uc, _set->usageCount(p));
    ASSERT_EQ(expected_total_uc, _set->totalUsageCount());
    ASSERT_EQ(expected_size, _set->size());
    ASSERT_EQ(expected_population, population);
  }

  void assureEmpty()
  {
    ASSERT_EQ(0, _set->totalUsageCount());
    ASSERT_EQ(0, _set->size());
  }

  void SetUp()
  {
    population = 0;
    _set.reset(new ShSet());
  }


  void TearDown()
  {
    eraseNumbers();
    _set.reset();
    ASSERT_EQ(0, population);
  }

  // a little common scenario
  void oneElementInsertion()
  {
    _ptr = produce(5);
    insertPtr(_ptr, _ptr, 1, 1, 1, 1);
  }

  Int* _ptr;
  std::shared_ptr<ShSet> _set;
  size_t population;
  std::vector<Int*> integers;
};


TEST_F(SharingSetTest, testSize)
{
  insert(5);
  insert(10);
  insert(10);
  insert(15);
  ASSERT_EQ(3, _set->size());
}

TEST_F(SharingSetTest, testUsageCount)
{
  Int* p = insert(5);
  Int* q = insert(10);
  insert(10);
  Int* r = insert(15);
  ASSERT_EQ(1, _set->usageCount(p));
  ASSERT_EQ(2, _set->usageCount(q));
  ASSERT_EQ(1, _set->usageCount(r));
}

TEST_F(SharingSetTest, testTotalUsageCount)
{
  insert(5);
  insert(10);
  insert(10);
  insert(15);
  ASSERT_EQ(4, _set->totalUsageCount());
}

TEST_F(SharingSetTest, testStats)
{
  // We lose: 8(ptr) + 8(ctr) = 16
  insert(5);

  // We save: (5(usage) - 1) * 100(deep_sizeof) - (8(ptr) + 8(ctr)) = 384
  insert(10);
  insert(10);
  insert(10);
  insert(10);
  insert(10);

  SharingSetMemStats stats = _set->stats();
  ASSERT_EQ(384, stats.bytes_saved);
  ASSERT_EQ(16, stats.bytes_lost);
  ASSERT_EQ(232, stats.actual_mem_used); // (100(deep_sizeof) + 8(ptr) + 8(ctr)) * 2(elems)
  ASSERT_EQ(600, stats.hypothetical_mem_used); // 100(deep_sizeof) * 6(elems)
}


TEST_F(SharingSetTest, testSummary)
{
  insert(5);
  insert(10);
  insert(10);
  insert(10);
  insert(10);
  insert(10);
  std::ostringstream out;
  _set->print_summary(out);
  ASSERT_EQ("SHARING SET SUMMARY\n"
      "Total 2 key/item pairs\n"
      "Total usage count 6\n"
      "Saved   384 B memory\n"
      "Lost    16 B memory\n"
      "Balance 368 B memory\n"
      "Actual memory 232 B (not including map internals)\n"
      "Hypothetical memory 600 B\n"
      "END OF SET SUMMARY\n", out.str());
}


TEST_F(SharingSetTest, testOneElemInsertion)
{
  oneElementInsertion();
}

TEST_F(SharingSetTest, testDifferentElemInsertion)
{
  oneElementInsertion();
  Int* q = produce(10);
  insertPtr(q, q, 1, 2, 2, 2);
}

TEST_F(SharingSetTest, testSameElemInsertion)
{
  oneElementInsertion();
  insertPtr(produce(5), integers[0], 2, 2, 1, 1);
}

TEST_F(SharingSetTest, testSamePointerInsertion)
{
  oneElementInsertion();
  insertPtr(integers[0], integers[0], 2, 2, 1, 1);
}

TEST_F(SharingSetTest, testUsageZeroIfNoSuchValue)
{
  oneElementInsertion();
  Int* absent = produce(7);
  ASSERT_EQ(0, _set->usageCount(absent));
  ASSERT_EQ(2, population); // nothing deleted so far...
  delete absent;
}


TEST_F(SharingSetTest, testCorrectUsageCountCalcFromDifferentPointer)
{
  oneElementInsertion();
  Int* x = produce(5);
  ASSERT_EQ(1, _set->usageCount(x));
  ASSERT_EQ(2, population); // nothing deleted so far...
  delete x;
  ASSERT_EQ(1, population);
}


TEST_F(SharingSetTest, testOneElemErasure)
{
  oneElementInsertion();
  const bool deleted = _set->erase(integers.back());
  integers.pop_back();
  ASSERT_TRUE(deleted);
  assureEmpty();
  ASSERT_EQ(0, population);
  ASSERT_TRUE(integers.empty());
}


TEST_F(SharingSetTest, testNoDeletionIfUnknownValue)
{
  oneElementInsertion();
  Int* x = produce(752);
  const bool deleted = _set->erase(x);
  ASSERT_FALSE(deleted);
  ASSERT_EQ(1, _set->totalUsageCount());
  ASSERT_EQ(1, _set->size());
  ASSERT_EQ(2, population);
  delete x;
  ASSERT_EQ(1, population);
}


TEST_F(SharingSetTest, testNoDeletionIfUnknownPointer)
{
  oneElementInsertion();
  Int* x = produce(5);
  const bool deleted = _set->erase(x);
  ASSERT_FALSE(deleted);
  ASSERT_EQ(1, _set->totalUsageCount());
  ASSERT_EQ(1, _set->size());
  ASSERT_EQ(2, population);
  delete x;
  ASSERT_EQ(1, population);
}

TEST_F(SharingSetTest, testTwoUsagesErasure)
{
  oneElementInsertion();
  insertPtr(produce(5), _ptr, 2, 2, 1, 1);
  ASSERT_TRUE(_set->erase(_ptr));
  assureStatus(_ptr, 1, 1, 1, 1);
  ASSERT_TRUE(_set->erase(_ptr));
  integers.pop_back();
  integers.pop_back();
  assureEmpty();
  ASSERT_EQ(0, population);
}

TEST_F(SharingSetTest, testTwoElemsErasure)
{
  oneElementInsertion();
  Int* q = produce(6);
  insertPtr(q, q, 1, 2, 2, 2);
  ASSERT_TRUE(_set->erase(_ptr));
  ASSERT_TRUE(_set->erase(q));
  integers.pop_back();
  integers.pop_back();
  assureEmpty();
  ASSERT_EQ(0, population);
}


} // namespace tools

} // namespace tse
