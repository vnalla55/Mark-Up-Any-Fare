//----------------------------------------------------------------------------
//  File:        MultiPriorityQueueTest.cpp
//  Created:     2011-00-01
//
//  Description: Unit tests for the universal priority queue
//
//  Updates:
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/MultiDimensionalPQ.h"

namespace
{
class Value
{
public:
  static Value* create(tse::TestMemHandle& mh, double v)
  {
    Value* value = 0;
    mh.get(value);
    value->_v = v;
    return value;
  }

  Value() : _v(0) {}

  double queueRank() const { return _v; }

private:
  double _v;
};
}

class MultiDimensionalPQTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(MultiDimensionalPQTest);

  CPPUNIT_TEST(testTwoDimensions);
  CPPUNIT_TEST(testThreeDimensions);
  CPPUNIT_TEST(testThreeDimensionsOneEmpty);
  CPPUNIT_TEST(testFourDimensions);

  CPPUNIT_TEST_SUITE_END();

public:
  MultiDimensionalPQTest();
  virtual ~MultiDimensionalPQTest();
  void setUp();
  void tearDown();

private:
  void testTwoDimensions();
  void testThreeDimensions();
  void testThreeDimensionsOneEmpty();
  void testFourDimensions();

  void checkValue(const std::vector<Value*>& v, size_t dim, double sum, int item);

  tse::TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MultiDimensionalPQTest);

MultiDimensionalPQTest::MultiDimensionalPQTest() {}

MultiDimensionalPQTest::~MultiDimensionalPQTest() {}

void
MultiDimensionalPQTest::setUp()
{
}

void
MultiDimensionalPQTest::tearDown()
{
  _memHandle.clear();
}

void
MultiDimensionalPQTest::testTwoDimensions()
{
  std::vector<std::vector<Value*>*> input;
  input.resize(2);
  for (size_t i = 0; i < input.size(); ++i)
    _memHandle.get(input[i]);

  input[0]->push_back(Value::create(_memHandle, 1));
  input[0]->push_back(Value::create(_memHandle, 2));
  input[0]->push_back(Value::create(_memHandle, 3));
  input[0]->push_back(Value::create(_memHandle, 4));
  input[1]->push_back(Value::create(_memHandle, 1));
  input[1]->push_back(Value::create(_memHandle, 3));
  input[1]->push_back(Value::create(_memHandle, 5));

  tse::MultiDimensionalPQ<Value*, double> multiPriorityQueue(input);
  std::vector<double> expected;
  for (size_t i = 0; i < input[0]->size(); ++i)
    for (size_t j = 0; j < input[1]->size(); ++j)
      expected.push_back((*input[0])[i]->queueRank() + (*input[1])[j]->queueRank());
  std::sort(expected.begin(), expected.end());
  for (size_t i = 0; i < expected.size(); ++i)
    checkValue(multiPriorityQueue.next(), 2, expected[i], i);
  CPPUNIT_ASSERT(multiPriorityQueue.next().empty());
}

void
MultiDimensionalPQTest::testThreeDimensions()
{
  std::vector<std::vector<Value*>*> input;
  input.resize(3);
  for (size_t i = 0; i < input.size(); ++i)
    _memHandle.get(input[i]);

  for (size_t i = 0; i < 4; ++i)
    input[0]->push_back(Value::create(_memHandle, i + 1));
  for (size_t i = 0; i < 5; i += 2)
    input[1]->push_back(Value::create(_memHandle, i + 1));
  for (size_t i = 0; i < 4; ++i)
    input[2]->push_back(Value::create(_memHandle, i + 2));

  tse::MultiDimensionalPQ<Value*, double> multiPriorityQueue(input);
  std::vector<double> expected;
  for (size_t i = 0; i < input[0]->size(); ++i)
    for (size_t j = 0; j < input[1]->size(); ++j)
      for (size_t k = 0; k < input[2]->size(); ++k)
        expected.push_back((*input[0])[i]->queueRank() + (*input[1])[j]->queueRank() +
                           (*input[2])[k]->queueRank());
  std::sort(expected.begin(), expected.end());
  for (size_t i = 0; i < expected.size(); ++i)
    checkValue(multiPriorityQueue.next(), 3, expected[i], i);
  CPPUNIT_ASSERT(multiPriorityQueue.next().empty());
}

void
MultiDimensionalPQTest::testThreeDimensionsOneEmpty()
{
  std::vector<std::vector<Value*>*> input;
  input.resize(3);
  for (size_t i = 0; i < input.size(); ++i)
    _memHandle.get(input[i]);

  for (size_t i = 0; i < 4; ++i)
    input[0]->push_back(Value::create(_memHandle, i + 1));
  for (size_t i = 0; i < 4; ++i)
    input[2]->push_back(Value::create(_memHandle, i + 2));

  tse::MultiDimensionalPQ<Value*, double> multiPriorityQueue(input);
  CPPUNIT_ASSERT(multiPriorityQueue.next().empty());
}

void
MultiDimensionalPQTest::testFourDimensions()
{
  std::vector<std::vector<Value*>*> input;
  input.resize(4);
  for (size_t i = 0; i < input.size(); ++i)
    _memHandle.get(input[i]);

  for (size_t i = 0; i < 4; ++i)
    input[0]->push_back(Value::create(_memHandle, i + 1));
  for (size_t i = 0; i < 5; i += 2)
    input[1]->push_back(Value::create(_memHandle, i + 1));
  for (size_t i = 0; i < 4; ++i)
    input[2]->push_back(Value::create(_memHandle, 4));
  for (size_t i = 0; i < 2; i += 2)
    input[1]->push_back(Value::create(_memHandle, i + 1));

  tse::MultiDimensionalPQ<Value*, double> multiPriorityQueue(input);
  std::vector<double> expected;
  for (size_t i = 0; i < input[0]->size(); ++i)
    for (size_t j = 0; j < input[1]->size(); ++j)
      for (size_t k = 0; k < input[2]->size(); ++k)
        for (size_t m = 0; m < input[3]->size(); ++m)
        {
          expected.push_back((*input[0])[i]->queueRank() + (*input[1])[j]->queueRank() +
                             (*input[2])[k]->queueRank() + (*input[3])[m]->queueRank());
        }
  std::sort(expected.begin(), expected.end());
  for (size_t i = 0; i < expected.size(); ++i)
    checkValue(multiPriorityQueue.next(), 4, expected[i], i);
  CPPUNIT_ASSERT(multiPriorityQueue.next().empty());
}

void
MultiDimensionalPQTest::checkValue(const std::vector<Value*>& v, size_t dim, double sum, int item)
{
  CPPUNIT_ASSERT_EQUAL(dim, v.size());
  double total = 0;
  for (size_t i = 0; i < v.size(); ++i)
    total += v[i]->queueRank();
  std::ostringstream oss;
  oss << "At item " << item;
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(oss.str(), sum, total, 0.01);
}
