// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Processor/TopologicalOrderer.h"

#include <memory>
#include <set>

#include <gtest/gtest.h>

#include "test/include/CppUnitHelperMacros.h"

namespace tax
{
class TopologicalOrdererTest : public CppUnit::TestFixture
{
  typedef int Key;
  typedef int Value;
  typedef TopologicalOrderer<Key, Value> Orderer;
  typedef typename boost::optional<const Orderer::Values&> TestValues;

  CPPUNIT_TEST_SUITE(TopologicalOrdererTest);

  CPPUNIT_TEST(testGetNextEmpty);
  CPPUNIT_TEST(testGetNextOneValue);
  CPPUNIT_TEST(testCommit);
  CPPUNIT_TEST(testGetNextOneEdge);
  CPPUNIT_TEST(testGetNextManyEdges);
  CPPUNIT_TEST(testGetNextManyEdgesNoNode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _orderer.reset(new Orderer());
    _keyMatcher.reset(new KeyMatcher());
  }

  void tearDown() {}

  void testGetNextEmpty() { ASSERT_TRUE(!_orderer->getNext()); }

  void testGetNextOneValue()
  {
    _orderer->addValue(1, 2);
    _orderer->commit<KeyMatcher>(*_keyMatcher);
    TestValues values = _orderer->getNext();

    ASSERT_EQ(1, values.get().size());
    ASSERT_EQ(2, *values.get().begin());
    ASSERT_TRUE(!_orderer->getNext());
  }

  void testCommit()
  {
    ASSERT_TRUE(_orderer->addEdge(1, 2));
    ASSERT_TRUE(_orderer->addValue(1, 3));
    _orderer->commit<KeyMatcher>(*_keyMatcher);
    ASSERT_FALSE(_orderer->addEdge(2, 3));
    ASSERT_FALSE(_orderer->addValue(2, 3));
  }

  void testGetNextOneEdge()
  {
    _orderer->addValue(1, 4);
    _orderer->addValue(2, 5);
    _orderer->addEdge(1, 2);
    _orderer->commit<KeyMatcher>(*_keyMatcher);

    TestValues values = _orderer->getNext();
    ASSERT_EQ(1, values.get().size());
    ASSERT_EQ(5, *values.get().begin());
    values = _orderer->getNext();
    ASSERT_EQ(1, values.get().size());
    ASSERT_EQ(4, *values.get().begin());
    ASSERT_TRUE(!_orderer->getNext());
  }

  /* Edges are from lower number to higher
   *     2-6
   *    / \
   *  1 -3-5
   *    \ /
   *     4
   */
  void testGetNextManyEdges()
  {
    _orderer->addValue(1, 1);
    _orderer->addValue(2, 2);
    _orderer->addValue(3, 3);
    _orderer->addValue(4, 4);
    _orderer->addValue(5, 5);
    _orderer->addValue(6, 6);

    _orderer->addEdge(1, 2);
    _orderer->addEdge(1, 30);
    _orderer->addEdge(1, 40);
    _orderer->addEdge(2, 5);
    _orderer->addEdge(2, 6);
    _orderer->addEdge(3, 5);
    _orderer->addEdge(4, 5);

    _orderer->commit(*_keyMatcher);
    std::set<Value> result;
    TestValues nextResult;
    for (int i = 0; i < 2; ++i)
    {
      nextResult = _orderer->getNext(); // 6 or 5
      ASSERT_TRUE(nextResult.is_initialized());
      ASSERT_EQ(1, nextResult->size());
      result.insert(*nextResult->begin());
    }
    ASSERT_NE(result.end(), result.find(5));
    ASSERT_NE(result.end(), result.find(6));

    result.clear();
    for (int i = 0; i < 3; ++i)
    {
      nextResult = _orderer->getNext(); // 2 or 3 or 4
      ASSERT_EQ(1, nextResult.get().size());
      result.insert(*nextResult.get().begin());
    }
    ASSERT_NE(result.end(), result.find(2));
    ASSERT_NE(result.end(), result.find(3));
    ASSERT_NE(result.end(), result.find(4));

    nextResult = _orderer->getNext(); // 2 or 3 or 4
    ASSERT_EQ(1, nextResult.get().size());
    ASSERT_EQ(1, *nextResult.get().begin());
    ASSERT_TRUE(!_orderer->getNext());
  }

  /* Edges are from lower number to higher
   *     2-6
   *    / \
   *  1 -3-5
   *    \ /
   *     4
   *
   * Not existing node: 7 with edges
   * 1 - 7
   * 3 - 7
   */
  void testGetNextManyEdgesNoNode()
  {
    _orderer->addValue(1, 1);
    _orderer->addValue(2, 2);
    _orderer->addValue(3, 3);
    _orderer->addValue(4, 4);
    _orderer->addValue(5, 5);
    _orderer->addValue(6, 6);

    _orderer->addEdge(1, 2);
    _orderer->addEdge(1, 3);
    _orderer->addEdge(1, 4);
    _orderer->addEdge(1, 7);
    _orderer->addEdge(2, 50);
    _orderer->addEdge(2, 6);
    _orderer->addEdge(3, 50);
    _orderer->addEdge(3, 7);
    _orderer->addEdge(4, 5);

    _orderer->commit(*_keyMatcher);
    std::set<Value> result;
    TestValues nextResult;
    for (int i = 0; i < 2; ++i)
    {
      nextResult = _orderer->getNext(); // 6 or 5
      ASSERT_TRUE(nextResult.is_initialized());
      ASSERT_EQ(1, nextResult->size());
      result.insert(*nextResult->begin());
    }
    ASSERT_NE(result.end(), result.find(5));
    ASSERT_NE(result.end(), result.find(6));

    result.clear();
    for (int i = 0; i < 3; ++i)
    {
      nextResult = _orderer->getNext(); // 2 or 3 or 4
      ASSERT_EQ(1, nextResult.get().size());
      result.insert(*nextResult.get().begin());
    }
    ASSERT_NE(result.end(), result.find(2));
    ASSERT_NE(result.end(), result.find(3));
    ASSERT_NE(result.end(), result.find(4));

    nextResult = _orderer->getNext(); // 2 or 3 or 4
    ASSERT_EQ(1, nextResult.get().size());
    ASSERT_EQ(1, *nextResult.get().begin());
    ASSERT_TRUE(!_orderer->getNext());
  }

private:
  struct KeyMatcher
  {
    bool isSimple(const Key& a) const { return a < 10; }

    bool operator()(const Key& a, const Key& b) const
    {
      Key a_ = isSimple(a) ? a : (a / 10);
      Key b_ = isSimple(b) ? b : (b / 10);
      return a_ == b_;
    }
  };

  std::unique_ptr<Orderer> _orderer;
  std::set<std::pair<Key, Key> > _edges;
  std::unique_ptr<KeyMatcher> _keyMatcher;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TopologicalOrdererTest);
}
