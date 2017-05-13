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
#include "Common/Utils/PtrCollections.h"

using namespace ::testing;

namespace tse
{

namespace tools
{

class PtrHashMapTest: public Test
{
public:

  void insertPair(int* p, std::string s, bool expected_insertion,
                  int* expected_ptr, int expected_ptr_deref,
                  std::string expected_value)
  {
    auto iter_flag = _map->insert(std::make_pair(p, s));
    ASSERT_EQ(expected_insertion, iter_flag.second);
    ASSERT_EQ(expected_ptr, iter_flag.first->first);
    ASSERT_EQ(expected_ptr_deref, *iter_flag.first->first);
    ASSERT_EQ(expected_value, iter_flag.first->second);
  }


  typedef PtrHashMap<int*, std::string> PointerMap;
  void SetUp(){ _map.reset(new PointerMap()); }
  void TearDown()
  {
    for (auto& ptr_val: *_map)
    {
      delete ptr_val.first;
    }
  }
  std::shared_ptr<PointerMap> _map;
};

TEST_F(PtrHashMapTest, testCorrectlyAcceptsOneElement)
{
  int* p = new int(5);
  insertPair(p, "foo", true, p, 5, "foo");
  ASSERT_EQ(1, _map->size());
}

TEST_F(PtrHashMapTest, testTwoElementsAddedIfDifferentDereferencedKey)
{
  int* p = new int(5);
  insertPair(p, "foo", true, p, 5, "foo");
  int* q = new int(10);
  insertPair(q, "bar", true, q, 10, "bar");
  ASSERT_EQ(2, _map->size());
}


TEST_F(PtrHashMapTest, testElementDroppedIfSameDereferencedKey)
{
  int* p = new int(5);
  insertPair(p, "foo", true, p, 5, "foo");
  int* q = new int(5);
  insertPair(q, "bar", false, p, 5, "foo");
  ASSERT_EQ(1, _map->size());
}

TEST_F(PtrHashMapTest, testElementDroppedIfSamePointer)
{
  int* p = new int(5);
  insertPair(p, "foo", true, p, 5, "foo");
  insertPair(p, "bar", false, p, 5, "foo");
  ASSERT_EQ(1, _map->size());
}

} // namespace tools

} // namespace tse
