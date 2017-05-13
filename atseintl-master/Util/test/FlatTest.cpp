// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "Util/FlatMap.h"
#include "Util/FlatSet.h"

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
template <typename ValueT>
ValueT
createElement(int i);

template <>
int
createElement<int>(int i)
{
  return i;
}

template <>
std::pair<int, int>
createElement<std::pair<int, int> >(int i)
{
  return std::make_pair(i, i);
}

template <typename ValueT>
ValueT
createElement(int i, int j);

template <>
int
createElement<int>(int i, int)
{
  return i;
}

template <>
std::pair<int, int>
createElement<std::pair<int, int> >(int i, int j)
{
  return std::make_pair(i, j);
}

template <typename KeyT, typename ValueT>
KeyT
valueToKey(ValueT value);

template <>
int
valueToKey(int value)
{
  return value;
}

template <>
int
valueToKey(std::pair<int, int> value)
{
  return value.first;
}

template <typename MappedT, typename ValueT>
MappedT
valueToMapped(ValueT value);

template <>
int
valueToMapped(int value)
{
  return value;
}

template <>
int
valueToMapped(std::pair<int, int> value)
{
  return value.second;
}

template <typename Container>
class FlatTestCommon : public ::testing::Test
{
public:
  typedef Container FlatT;
  typedef typename FlatT::key_type KeyT;
  typedef typename FlatT::value_type ValueT;
  typedef typename FlatT::mapped_type MappedT;
  typedef typename FlatT::value_compare CompareT;
  typedef typename FlatT::iterator IteratorT;
  typedef typename FlatT::const_iterator ConstIteratorT;

  static const bool IsSet = FlatT::IsSet;
  static const bool IsUnique = FlatT::IsUnique;
  static const bool IsLess = boost::is_same<std::less<KeyT>, typename FlatT::key_compare>::value;

  FlatT* flat;

  void SetUp() { flat = 0; }
  void TearDown() { delete flat; }

  ValueT element(int i) { return createElement<ValueT>(i); }
  ValueT element(int i, int j) { return createElement<ValueT>(i, j); }
  ValueT choose(int ifLess, int ifGreater) { return IsLess ? element(ifLess) : element(ifGreater); }
  IteratorT chooseBeginEnd() { return IsLess ? flat->begin() : flat->end(); }
  IteratorT chooseEndBegin() { return IsLess ? flat->end() : flat->begin(); }
  IteratorT chooseIt(int ifLess)
  {
    return IsLess ? flat->begin() + ifLess : flat->end() - ifLess - 1;
  }
  IteratorT chooseItNext(int ifLess)
  {
    return IsLess ? flat->begin() + ifLess + 1 : flat->end() - ifLess - 1;
  }
  KeyT key(ValueT value) { return valueToKey<KeyT>(value); }
  MappedT mapped(ValueT value) { return valueToMapped<MappedT>(value); }
  CompareT getCompare() { return flat->value_comp(); }

  FlatT* sized(int size, int bias = 0)
  {
    FlatT* const flat = new FlatT;
    for (int i = 0; i < size; ++i)
      flat->insert(element(i + bias));
    flat->shrink_to_fit();
    return flat;
  }
};

template <typename Container>
class FlatTestUnique : public FlatTestCommon<Container>
{
};

template <typename Container>
class FlatTestMulti : public FlatTestCommon<Container>
{
};

template <typename Container>
class FlatMapTest : public FlatTestCommon<Container>
{
};

template <typename Container>
class FlatMultiMapTest : public FlatTestCommon<Container>
{
};

typedef ::testing::Types<FlatSet<int>,
                         FlatSet<int, std::greater<int> >,
                         FlatMultiSet<int>,
                         FlatMultiSet<int, std::greater<int> >,
                         FlatMap<int, int>,
                         FlatMap<int, int, std::greater<int> >,
                         FlatMultiMap<int, int>,
                         FlatMultiMap<int, int, std::greater<int> > > FlatTestCommonTypes;
TYPED_TEST_CASE(FlatTestCommon, FlatTestCommonTypes);

typedef ::testing::Types<FlatSet<int>,
                         FlatSet<int, std::greater<int> >,
                         FlatMap<int, int>,
                         FlatMap<int, int, std::greater<int> > > FlatTestUniqueTypes;
TYPED_TEST_CASE(FlatTestUnique, FlatTestUniqueTypes);

typedef ::testing::Types<FlatMultiSet<int>,
                         FlatMultiSet<int, std::greater<int> >,
                         FlatMultiMap<int, int>,
                         FlatMultiMap<int, int, std::greater<int> > > FlatTestMultiTypes;
TYPED_TEST_CASE(FlatTestMulti, FlatTestMultiTypes);

typedef ::testing::Types<FlatMap<int, int>, FlatMap<int, int, std::greater<int> > >
FlatMapTestTypes;
TYPED_TEST_CASE(FlatMapTest, FlatMapTestTypes);

typedef ::testing::Types<FlatMultiMap<int, int>, FlatMultiMap<int, int, std::greater<int> > >
FlatMultiMapTestTypes;
TYPED_TEST_CASE(FlatMultiMapTest, FlatMultiMapTestTypes);

#define IMPORT_DEFINES                                                                             \
  typedef FlatTestCommon<TypeParam> Base;                                                          \
  typedef typename Base::FlatT FlatT __attribute__((unused));                                      \
  typedef typename Base::KeyT KeyT __attribute__((unused));                                        \
  typedef typename Base::ValueT ValueT __attribute__((unused));                                    \
  typedef typename Base::MappedT MappedT __attribute__((unused));                                  \
  typedef typename Base::CompareT CompareT __attribute__((unused));                                \
  typedef typename Base::IteratorT IteratorT __attribute__((unused));                              \
  typedef typename Base::ConstIteratorT ConstIteratorT __attribute__((unused));

TYPED_TEST(FlatTestCommon, testSizeIsOk)
{
  ASSERT_EQ(size_t(16), sizeof(*this->flat));
}

TYPED_TEST(FlatTestCommon, testConstructorDefault)
{
  IMPORT_DEFINES

  this->flat = new FlatT;

  ASSERT_EQ(0, this->flat->size());
  ASSERT_EQ(0, this->flat->capacity());
}

TYPED_TEST(FlatTestCommon, testConstructorRangeEmpty)
{
  IMPORT_DEFINES

  ValueT tab[0];
  this->flat = new FlatT(tab + 0, tab + 0);

  ASSERT_EQ(0, this->flat->size());
}

TYPED_TEST(FlatTestCommon, testConstructorRangeNotEmpty)
{
  IMPORT_DEFINES

  ValueT tab[3] = { this->element(3), this->element(2), this->element(1) };
  this->flat = new FlatT(tab + 0, tab + 3);

  ASSERT_EQ(3, this->flat->size());
  ASSERT_EQ(this->choose(1, 3), *this->flat->begin());
}

TYPED_TEST(FlatTestCommon, testConstructorRangeNotEmptyDuplicates)
{
  IMPORT_DEFINES

  ValueT tab[5] = {
    this->element(3), this->element(3), this->element(2), this->element(1), this->element(2)
  };
  this->flat = new FlatT(tab + 0, tab + 5);

  if (this->IsUnique)
    ASSERT_EQ(3, this->flat->size());
  else
    ASSERT_EQ(5, this->flat->size());

  ASSERT_EQ(this->choose(1, 3), *this->flat->begin());
}

TYPED_TEST(FlatTestCommon, testConstructorCopy)
{
  IMPORT_DEFINES

  ValueT tab[3] = { this->element(7), this->element(14), this->element(1) };
  this->flat = new FlatT(FlatT(tab + 0, tab + 3));

  ASSERT_EQ(3, this->flat->size());
  ASSERT_EQ(this->element(7), *(this->flat->begin() + 1));
}

TYPED_TEST(FlatTestCommon, testAssignment)
{
  IMPORT_DEFINES

  ValueT tab[6] = { this->element(37), this->element(3), this->element(66),
                    this->element(16), this->element(2), this->element(0) };
  this->flat = this->sized(3);

  *this->flat = FlatT(tab + 0, tab + 6);

  ASSERT_EQ(6, this->flat->size());
  ASSERT_EQ(this->choose(0, 66), *this->flat->begin());
}

TYPED_TEST(FlatTestCommon, testReserve)
{
  IMPORT_DEFINES

  this->flat = this->sized(13);
  ASSERT_EQ(13, this->flat->capacity());

  this->flat->reserve(17);
  ASSERT_EQ(17, this->flat->capacity());

  this->flat->reserve(14);
  ASSERT_EQ(17, this->flat->capacity());

  this->flat->reserve(8);
  ASSERT_EQ(17, this->flat->capacity());
}

TYPED_TEST(FlatTestCommon, testShrinkToFit)
{
  IMPORT_DEFINES

  this->flat = this->sized(13);

  this->flat->insert(this->element(13));
  this->flat->shrink_to_fit();

  ASSERT_EQ(14, this->flat->size());
  ASSERT_EQ(14, this->flat->capacity());
}

TYPED_TEST(FlatTestCommon, testClear)
{
  IMPORT_DEFINES

  this->flat = this->sized(13);

  ASSERT_EQ(13, this->flat->size());
  ASSERT_EQ(13, this->flat->capacity());

  this->flat->clear();

  ASSERT_EQ(0, this->flat->size());
  ASSERT_EQ(0, this->flat->capacity());
}

TYPED_TEST(FlatTestCommon, testElementsAreSorted)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT sorted[10] = { this->element(0),  this->element(1),  this->element(8),  this->element(11),
                        this->element(13), this->element(15), this->element(34), this->element(56),
                        this->element(66), this->element(75) };

  if (Base::IsLess)
    ASSERT_TRUE(std::equal(this->flat->begin(), this->flat->end(), sorted));
  else
    ASSERT_TRUE(std::equal(
        this->flat->begin(), this->flat->end(), std::reverse_iterator<ValueT*>(sorted + 10)));
}

TYPED_TEST(FlatTestCommon, testFindExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  for (int i = 0; i < 10; ++i)
  {
    ValueT value = tab[i];
    KeyT key = this->key(value);
    IteratorT findIt = this->flat->find(key);
    IteratorT lowerIt = this->flat->lower_bound(key);
    IteratorT upperIt = this->flat->upper_bound(key);
    std::pair<IteratorT, IteratorT> equalIt = this->flat->equal_range(key);
    int count = this->flat->count(key);

    ASSERT_NE(this->flat->end(), findIt);
    ASSERT_EQ(1, count);
    ASSERT_EQ(value, *findIt);
    ASSERT_EQ(findIt, lowerIt);
    ASSERT_EQ(findIt + 1, upperIt);
    ASSERT_EQ(std::make_pair(lowerIt, upperIt), equalIt);
  }
}

TYPED_TEST(FlatTestCommon, testFindExistingMulti)
{
  IMPORT_DEFINES

  ValueT tab[20] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34), this->element(8),  this->element(8),
                     this->element(8),  this->element(11), this->element(11), this->element(11),
                     this->element(0),  this->element(1),  this->element(34), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 20);

  for (int i = 0; i < 20; ++i)
  {
    ASSERT_EQ(tab[i], *this->flat->find(this->key(tab[i])));
    ASSERT_LE(1, this->flat->count(this->key(tab[i])));

    ValueT value = tab[i];
    KeyT key = this->key(value);
    IteratorT findIt = this->flat->find(key);
    IteratorT lowerIt = this->flat->lower_bound(key);
    IteratorT upperIt = this->flat->upper_bound(key);
    std::pair<IteratorT, IteratorT> equalIt = this->flat->equal_range(key);
    int count = this->flat->count(key);

    ASSERT_NE(this->flat->end(), findIt);
    ASSERT_LE(1, count);
    ASSERT_EQ(value, *findIt);
    ASSERT_EQ(findIt, lowerIt);
    ASSERT_LT(findIt, upperIt);
    ASSERT_EQ(value, *(upperIt - 1));
    ASSERT_EQ(std::make_pair(lowerIt, upperIt), equalIt);
    ASSERT_EQ(upperIt - lowerIt, count);
  }

  if (this->IsUnique)
    return;

  ASSERT_EQ(1, this->flat->count(13));
  ASSERT_EQ(3, this->flat->count(34));
  ASSERT_EQ(2, this->flat->count(1));
  ASSERT_EQ(4, this->flat->count(8));
}

TYPED_TEST(FlatTestCommon, testFindNonExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ASSERT_EQ(this->flat->end(), this->flat->find(50));
  ASSERT_EQ(0, this->flat->count(50));
  ASSERT_EQ(this->choose(56, 34), *this->flat->lower_bound(50));
  ASSERT_EQ(this->choose(56, 34), *this->flat->upper_bound(50));

  ASSERT_EQ(this->flat->end(), this->flat->find(12));
  ASSERT_EQ(0, this->flat->count(12));
  ASSERT_EQ(this->choose(13, 11), *this->flat->lower_bound(12));
  ASSERT_EQ(this->choose(13, 11), *this->flat->upper_bound(12));

  ASSERT_EQ(this->flat->end(), this->flat->find(7));
  ASSERT_EQ(0, this->flat->count(7));
  ASSERT_EQ(this->choose(8, 1), *this->flat->lower_bound(7));
  ASSERT_EQ(this->choose(8, 1), *this->flat->upper_bound(7));

  ASSERT_EQ(this->flat->end(), this->flat->find(-5));
  ASSERT_EQ(0, this->flat->count(-5));
  ASSERT_EQ(this->chooseBeginEnd(), this->flat->lower_bound(-5));
  ASSERT_EQ(this->chooseBeginEnd(), this->flat->upper_bound(-5));

  ASSERT_EQ(this->flat->end(), this->flat->find(99));
  ASSERT_EQ(0, this->flat->count(99));
  ASSERT_EQ(this->chooseEndBegin(), this->flat->lower_bound(99));
  ASSERT_EQ(this->chooseEndBegin(), this->flat->upper_bound(99));
}

TYPED_TEST(FlatTestUnique, testInsertNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  std::pair<IteratorT, bool> pair;

  pair = this->flat->insert(this->element(3));
  ASSERT_EQ(std::make_pair(this->chooseIt(2), true), pair);

  pair = this->flat->insert(this->element(20));
  ASSERT_EQ(std::make_pair(this->chooseIt(7), true), pair);

  pair = this->flat->insert(this->element(-42));
  ASSERT_EQ(std::make_pair(this->chooseIt(0), true), pair);

  pair = this->flat->insert(this->element(100));
  ASSERT_EQ(std::make_pair(this->chooseIt(13), true), pair);

  pair = this->flat->insert(this->element(50));
  ASSERT_EQ(std::make_pair(this->chooseIt(10), true), pair);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestMulti, testInsertNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->insert(this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->insert(this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->insert(this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->insert(this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testInsertExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  std::pair<IteratorT, bool> pair;

  pair = this->flat->insert(this->element(13, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(4), false), pair);

  pair = this->flat->insert(this->element(8, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(2), false), pair);

  pair = this->flat->insert(this->element(75, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(9), false), pair);

  pair = this->flat->insert(this->element(0, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(0), false), pair);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testInsertExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->insert(this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->insert(this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->insert(this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestUnique, testEmplaceNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  std::pair<IteratorT, bool> pair;

  pair = this->flat->emplace(this->element(3));
  ASSERT_EQ(std::make_pair(this->chooseIt(2), true), pair);

  pair = this->flat->emplace(this->element(20));
  ASSERT_EQ(std::make_pair(this->chooseIt(7), true), pair);

  pair = this->flat->emplace(this->element(-42));
  ASSERT_EQ(std::make_pair(this->chooseIt(0), true), pair);

  pair = this->flat->emplace(this->element(100));
  ASSERT_EQ(std::make_pair(this->chooseIt(13), true), pair);

  pair = this->flat->emplace(this->element(50));
  ASSERT_EQ(std::make_pair(this->chooseIt(10), true), pair);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestMulti, testEmplaceNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace(this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->emplace(this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->emplace(this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->emplace(this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->emplace(this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testEmplaceExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  std::pair<IteratorT, bool> pair;

  pair = this->flat->emplace(this->element(13, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(4), false), pair);

  pair = this->flat->emplace(this->element(8, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(2), false), pair);

  pair = this->flat->emplace(this->element(75, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(9), false), pair);

  pair = this->flat->emplace(this->element(0, -1));
  ASSERT_EQ(std::make_pair(this->chooseIt(0), false), pair);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testEmplaceExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace(this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->emplace(this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->emplace(this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->emplace(this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestCommon, testInsertWithGoodHintNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(1), this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->insert(this->chooseItNext(6), this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->insert(this->chooseItNext(-1), this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->insert(this->chooseItNext(12), this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->insert(this->chooseItNext(9), this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testInsertWithGoodHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(4), this->element(13, -1));
  ASSERT_EQ(this->chooseIt(4), it);

  it = this->flat->insert(this->chooseItNext(2), this->element(8, -1));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->insert(this->chooseItNext(9), this->element(75, -1));
  ASSERT_EQ(this->chooseIt(9), it);

  it = this->flat->insert(this->chooseItNext(0), this->element(0, -1));
  ASSERT_EQ(this->chooseIt(0), it);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testInsertWithGoodHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(4), this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->insert(this->chooseItNext(2), this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->insert(this->chooseItNext(11), this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->insert(this->chooseItNext(0), this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestCommon, testInsertWithBadHintNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(0), this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->insert(this->chooseItNext(3), this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->insert(this->chooseItNext(12), this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->insert(this->chooseItNext(7), this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->insert(this->chooseItNext(10), this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testInsertWithBadHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(2), this->element(13, -1));
  ASSERT_EQ(this->chooseIt(4), it);

  it = this->flat->insert(this->chooseItNext(5), this->element(8, -1));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->insert(this->chooseItNext(8), this->element(75, -1));
  ASSERT_EQ(this->chooseIt(9), it);

  it = this->flat->insert(this->chooseItNext(1), this->element(0, -1));
  ASSERT_EQ(this->chooseIt(0), it);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testInsertWithBadHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->insert(this->chooseItNext(5), this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->insert(this->chooseItNext(0), this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->insert(this->chooseItNext(8), this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->insert(this->chooseItNext(3), this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestCommon, testEmplaceWithGoodHintNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(1), this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->emplace_hint(this->chooseItNext(6), this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->emplace_hint(this->chooseItNext(-1), this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->emplace_hint(this->chooseItNext(12), this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->emplace_hint(this->chooseItNext(9), this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testEmplaceWithGoodHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(4), this->element(13, -1));
  ASSERT_EQ(this->chooseIt(4), it);

  it = this->flat->emplace_hint(this->chooseItNext(2), this->element(8, -1));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->emplace_hint(this->chooseItNext(9), this->element(75, -1));
  ASSERT_EQ(this->chooseIt(9), it);

  it = this->flat->emplace_hint(this->chooseItNext(0), this->element(0, -1));
  ASSERT_EQ(this->chooseIt(0), it);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testEmplaceWithGoodHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(4), this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(2), this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(11), this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(0), this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestCommon, testEmplaceWithBadHintNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(0), this->element(3));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->emplace_hint(this->chooseItNext(3), this->element(20));
  ASSERT_EQ(this->chooseIt(7), it);

  it = this->flat->emplace_hint(this->chooseItNext(12), this->element(-42));
  ASSERT_EQ(this->chooseIt(0), it);

  it = this->flat->emplace_hint(this->chooseItNext(7), this->element(100));
  ASSERT_EQ(this->chooseIt(13), it);

  it = this->flat->emplace_hint(this->chooseItNext(10), this->element(50));
  ASSERT_EQ(this->chooseIt(10), it);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testEmplaceWithBadHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(2), this->element(13, -1));
  ASSERT_EQ(this->chooseIt(4), it);

  it = this->flat->emplace_hint(this->chooseItNext(5), this->element(8, -1));
  ASSERT_EQ(this->chooseIt(2), it);

  it = this->flat->emplace_hint(this->chooseItNext(8), this->element(75, -1));
  ASSERT_EQ(this->chooseIt(9), it);

  it = this->flat->emplace_hint(this->chooseItNext(1), this->element(0, -1));
  ASSERT_EQ(this->chooseIt(0), it);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testEmplaceWithBadHintExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  it = this->flat->emplace_hint(this->chooseItNext(5), this->element(13));
  ASSERT_EQ(13, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(0), this->element(8));
  ASSERT_EQ(8, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(8), this->element(75));
  ASSERT_EQ(75, this->key(*it));

  it = this->flat->emplace_hint(this->chooseItNext(3), this->element(0));
  ASSERT_EQ(0, this->key(*it));

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestCommon, testInsertManyNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[5] = {
    this->element(3), this->element(20), this->element(-42), this->element(100), this->element(50)
  };
  this->flat->insert(insert + 0, insert + 5);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testInsertManyExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[4] = {
    this->element(13, -1), this->element(8, -1), this->element(75, -1), this->element(0, -1)
  };
  this->flat->insert(insert + 0, insert + 4);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testInsertManyExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[4] = { this->element(13), this->element(8), this->element(75), this->element(0) };
  this->flat->insert(insert + 0, insert + 4);

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestUnique, testInsertManyMixed)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[9] = { this->element(3),     this->element(75, -1), this->element(20),
                       this->element(100),   this->element(13, -1), this->element(50),
                       this->element(8, -1), this->element(0, -1),  this->element(-42) };
  this->flat->insert(insert + 0, insert + 9);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestMulti, testInsertManyMixed)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[9] = { this->element(3),   this->element(75), this->element(20),
                       this->element(100), this->element(13), this->element(50),
                       this->element(8),   this->element(0),  this->element(-42) };
  this->flat->insert(insert + 0, insert + 9);

  ASSERT_EQ(19, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(4), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(11), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(13), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(18), this->flat->find(100));
}

TYPED_TEST(FlatMultiMapTest, testInsertManyPreservesOrder)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[4] = { this->element(13, 1), this->element(13, 2),
                       this->element(13, 3), this->element(13, 4) };
  this->flat->insert(insert + 0, insert + 4);

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(5, this->flat->count(13));

  const ConstIteratorT it = this->flat->lower_bound(13);
  ASSERT_EQ(13, (it+0)->second);
  ASSERT_EQ(1, (it+1)->second);
  ASSERT_EQ(2, (it+2)->second);
  ASSERT_EQ(3, (it+3)->second);
  ASSERT_EQ(4, (it+4)->second);
}

TYPED_TEST(FlatTestCommon, testUnsafeInsertNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_insert(this->element(3));
  this->flat->unsafe_insert(this->element(20));
  this->flat->unsafe_insert(this->element(-42));
  this->flat->unsafe_insert(this->element(100));
  this->flat->unsafe_insert(this->element(50));

  this->flat->order();

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testUnsafeInsertExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_insert(this->element(1, -1));
  this->flat->unsafe_insert(this->element(11, -1));
  this->flat->unsafe_insert(this->element(13, -1));
  this->flat->unsafe_insert(this->element(66, -1));

  this->flat->order();

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(1, this->mapped(*this->flat->find(1)));
  ASSERT_EQ(11, this->mapped(*this->flat->find(11)));
  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(66, this->mapped(*this->flat->find(66)));
}

TYPED_TEST(FlatTestMulti, testUnsafeInsertExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_insert(this->element(1));
  this->flat->unsafe_insert(this->element(11));
  this->flat->unsafe_insert(this->element(13));
  this->flat->unsafe_insert(this->element(66));

  this->flat->order();

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(1));
  ASSERT_EQ(2, this->flat->count(11));
  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(66));
}

TYPED_TEST(FlatTestCommon, testUnsafeEmplaceNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_emplace(this->element(3));
  this->flat->unsafe_emplace(this->element(20));
  this->flat->unsafe_emplace(this->element(-42));
  this->flat->unsafe_emplace(this->element(100));
  this->flat->unsafe_emplace(this->element(50));

  this->flat->order();

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testUnsafeEmplaceExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_emplace(this->element(1, -1));
  this->flat->unsafe_emplace(this->element(11, -1));
  this->flat->unsafe_emplace(this->element(13, -1));
  this->flat->unsafe_emplace(this->element(66, -1));

  this->flat->order();

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(1, this->mapped(*this->flat->find(1)));
  ASSERT_EQ(11, this->mapped(*this->flat->find(11)));
  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(66, this->mapped(*this->flat->find(66)));
}

TYPED_TEST(FlatTestMulti, testUnsafeEmplaceExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  this->flat->unsafe_emplace(this->element(1));
  this->flat->unsafe_emplace(this->element(11));
  this->flat->unsafe_emplace(this->element(13));
  this->flat->unsafe_emplace(this->element(66));

  this->flat->order();

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(1));
  ASSERT_EQ(2, this->flat->count(11));
  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(66));
}

TYPED_TEST(FlatTestCommon, testUnsafeInsertManyNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[5] = {
    this->element(3), this->element(20), this->element(-42), this->element(100), this->element(50)
  };
  this->flat->unsafe_insert(insert + 0, insert + 5);

  this->flat->order();

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestUnique, testUnsafeInsertManyExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[4] = {
    this->element(13, -1), this->element(8, -1), this->element(75, -1), this->element(0, -1)
  };
  this->flat->unsafe_insert(insert + 0, insert + 4);

  this->flat->order();

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatTestMulti, testUnsafeInsertManyExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[4] = { this->element(13), this->element(8), this->element(75), this->element(0) };
  this->flat->unsafe_insert(insert + 0, insert + 4);

  this->flat->order();

  ASSERT_EQ(14, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));
}

TYPED_TEST(FlatTestUnique, testUnsafeInsertManyMixed)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[9] = { this->element(3),     this->element(75, -1), this->element(20),
                       this->element(100),   this->element(13, -1), this->element(50),
                       this->element(8, -1), this->element(0, -1),  this->element(-42) };
  this->flat->unsafe_insert(insert + 0, insert + 9);

  this->flat->order();

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatTestMulti, testUnsafeInsertManyMixed)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ValueT insert[9] = { this->element(3),   this->element(75), this->element(20),
                       this->element(100), this->element(13), this->element(50),
                       this->element(8),   this->element(0),  this->element(-42) };
  this->flat->unsafe_insert(insert + 0, insert + 9);

  this->flat->order();

  ASSERT_EQ(19, this->flat->size());

  ASSERT_EQ(2, this->flat->count(13));
  ASSERT_EQ(2, this->flat->count(8));
  ASSERT_EQ(2, this->flat->count(75));
  ASSERT_EQ(2, this->flat->count(0));

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(4), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(11), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(13), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(18), this->flat->find(100));
}

TYPED_TEST(FlatTestCommon, testEraseIterator)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  if (Base::IsLess)
  {
    it = this->flat->erase(this->flat->begin() + 4);
    ASSERT_EQ(this->flat->begin() + 4, it);

    it = this->flat->erase(this->flat->begin() + 2);
    ASSERT_EQ(this->flat->begin() + 2, it);

    it = this->flat->erase(this->flat->begin() + 7);
    ASSERT_EQ(this->flat->begin() + 7, it);

    it = this->flat->erase(this->flat->begin() + 0);
    ASSERT_EQ(this->flat->begin() + 0, it);
  }
  else
  {
    it = this->flat->erase(this->flat->begin() + 5);
    ASSERT_EQ(this->flat->begin() + 5, it);

    it = this->flat->erase(this->flat->begin() + 6);
    ASSERT_EQ(this->flat->begin() + 6, it);

    it = this->flat->erase(this->flat->begin() + 0);
    ASSERT_EQ(this->flat->begin() + 0, it);

    it = this->flat->erase(this->flat->begin() + 6);
    ASSERT_EQ(this->flat->begin() + 6, it);
  }

  ASSERT_EQ(6, this->flat->size());

  ASSERT_EQ(0, this->flat->count(13));
  ASSERT_EQ(0, this->flat->count(8));
  ASSERT_EQ(0, this->flat->count(75));
  ASSERT_EQ(0, this->flat->count(0));
  ASSERT_EQ(1, this->flat->count(15));
  ASSERT_EQ(1, this->flat->count(1));
  ASSERT_EQ(1, this->flat->count(34));
  ASSERT_EQ(1, this->flat->count(56));
}

TYPED_TEST(FlatTestCommon, testEraseRange)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  IteratorT it;

  if (Base::IsLess)
  {
    it = this->flat->erase(this->flat->begin() + 2, this->flat->begin() + 4);
    ASSERT_EQ(this->flat->begin() + 2, it);
  }
  else
  {
    it = this->flat->erase(this->flat->begin() + 6, this->flat->begin() + 8);
    ASSERT_EQ(this->flat->begin() + 6, it);
  }

  ASSERT_EQ(8, this->flat->size());

  ASSERT_EQ(0, this->flat->count(11));
  ASSERT_EQ(0, this->flat->count(8));
  ASSERT_EQ(1, this->flat->count(75));
  ASSERT_EQ(1, this->flat->count(0));
  ASSERT_EQ(1, this->flat->count(15));
  ASSERT_EQ(1, this->flat->count(1));
  ASSERT_EQ(1, this->flat->count(34));
  ASSERT_EQ(1, this->flat->count(56));
}

TYPED_TEST(FlatTestCommon, testEraseNonExistingKey)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ASSERT_EQ(0, this->flat->erase(14));
  ASSERT_EQ(0, this->flat->erase(5));
  ASSERT_EQ(0, this->flat->erase(20));
  ASSERT_EQ(0, this->flat->erase(9));

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(1, this->flat->count(13));
  ASSERT_EQ(1, this->flat->count(8));
  ASSERT_EQ(1, this->flat->count(75));
  ASSERT_EQ(1, this->flat->count(0));
  ASSERT_EQ(1, this->flat->count(15));
  ASSERT_EQ(1, this->flat->count(1));
  ASSERT_EQ(1, this->flat->count(34));
  ASSERT_EQ(1, this->flat->count(56));
}

TYPED_TEST(FlatTestCommon, testEraseOnlyKey)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  ASSERT_EQ(1, this->flat->erase(13));
  ASSERT_EQ(1, this->flat->erase(8));
  ASSERT_EQ(1, this->flat->erase(75));
  ASSERT_EQ(1, this->flat->erase(0));

  ASSERT_EQ(6, this->flat->size());

  ASSERT_EQ(0, this->flat->count(13));
  ASSERT_EQ(0, this->flat->count(8));
  ASSERT_EQ(0, this->flat->count(75));
  ASSERT_EQ(0, this->flat->count(0));
  ASSERT_EQ(1, this->flat->count(15));
  ASSERT_EQ(1, this->flat->count(1));
  ASSERT_EQ(1, this->flat->count(34));
  ASSERT_EQ(1, this->flat->count(56));
}

TYPED_TEST(FlatTestMulti, testEraseMultipleKey)
{
  IMPORT_DEFINES

  ValueT tab[20] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34), this->element(8),  this->element(8),
                     this->element(8),  this->element(11), this->element(11), this->element(11),
                     this->element(0),  this->element(1),  this->element(34), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 20);

  ASSERT_EQ(1, this->flat->erase(13));
  ASSERT_EQ(4, this->flat->erase(8));
  ASSERT_EQ(1, this->flat->erase(75));
  ASSERT_EQ(2, this->flat->erase(0));

  ASSERT_EQ(12, this->flat->size());

  ASSERT_EQ(0, this->flat->count(13));
  ASSERT_EQ(0, this->flat->count(8));
  ASSERT_EQ(0, this->flat->count(75));
  ASSERT_EQ(0, this->flat->count(0));
  ASSERT_EQ(1, this->flat->count(15));
  ASSERT_EQ(2, this->flat->count(1));
  ASSERT_EQ(3, this->flat->count(34));
  ASSERT_EQ(1, this->flat->count(56));
}

TYPED_TEST(FlatTestCommon, testSwapEmpty)
{
  IMPORT_DEFINES

  this->flat = new FlatT();
  FlatT v2;

  this->flat->swap(v2);

  ASSERT_EQ(0, this->flat->size());
  ASSERT_EQ(0, this->flat->capacity());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(0, v2.capacity());
}

TYPED_TEST(FlatTestCommon, testSwapWithEmpty)
{
  IMPORT_DEFINES

  this->flat = this->sized(7);
  FlatT v2;

  this->flat->swap(v2);

  ASSERT_EQ(0, this->flat->size());
  ASSERT_EQ(0, this->flat->capacity());
  ASSERT_EQ(7, v2.size());
  ASSERT_EQ(7, v2.capacity());
}

TYPED_TEST(FlatTestCommon, testSwapWithEmpty2)
{
  IMPORT_DEFINES

  this->flat = new FlatT();
  FlatT* v2 = this->sized(7);

  this->flat->swap(*v2);

  ASSERT_EQ(7, this->flat->size());
  ASSERT_EQ(7, this->flat->capacity());
  ASSERT_EQ(0, v2->size());
  ASSERT_EQ(0, v2->capacity());

  delete v2;
}

TYPED_TEST(FlatTestCommon, testSwapFull)
{
  IMPORT_DEFINES

  this->flat = this->sized(7);
  FlatT* v2 = this->sized(9);

  this->flat->swap(*v2);

  ASSERT_EQ(9, this->flat->size());
  ASSERT_EQ(9, this->flat->capacity());
  ASSERT_EQ(1, this->flat->count(8));
  ASSERT_EQ(7, v2->size());
  ASSERT_EQ(7, v2->capacity());
  ASSERT_EQ(0, v2->count(8));

  delete v2;
}

TYPED_TEST(FlatTestCommon, testComparisonWhenEqual)
{
  IMPORT_DEFINES

  this->flat = this->sized(7);
  FlatT* v2 = this->sized(7);

  ASSERT_TRUE(*this->flat == *v2);
  ASSERT_TRUE(*v2 == *this->flat);

  ASSERT_FALSE(*this->flat != *v2);
  ASSERT_FALSE(*v2 != *this->flat);

  ASSERT_FALSE(*this->flat < *v2);
  ASSERT_FALSE(*v2 < *this->flat);

  ASSERT_TRUE(*this->flat <= *v2);
  ASSERT_TRUE(*v2 <= *this->flat);

  ASSERT_FALSE(*this->flat > *v2);
  ASSERT_FALSE(*v2 > *this->flat);

  ASSERT_TRUE(*this->flat >= *v2);
  ASSERT_TRUE(*v2 >= *this->flat);

  delete v2;
}

TYPED_TEST(FlatTestCommon, testComparisonWhenBigger)
{
  IMPORT_DEFINES

  this->flat = this->sized(7);
  FlatT* v2 = this->sized(7, 1);

  ASSERT_FALSE(*this->flat == *v2);
  ASSERT_FALSE(*v2 == *this->flat);

  ASSERT_TRUE(*this->flat != *v2);
  ASSERT_TRUE(*v2 != *this->flat);

  ASSERT_TRUE(*this->flat < *v2);
  ASSERT_FALSE(*v2 < *this->flat);

  ASSERT_TRUE(*this->flat <= *v2);
  ASSERT_FALSE(*v2 <= *this->flat);

  ASSERT_FALSE(*this->flat > *v2);
  ASSERT_TRUE(*v2 > *this->flat);

  ASSERT_FALSE(*this->flat >= *v2);
  ASSERT_TRUE(*v2 >= *this->flat);

  delete v2;
}

TYPED_TEST(FlatTestMulti, testComparisonWhenBiggerInSize)
{
  IMPORT_DEFINES

  this->flat = this->sized(7);
  FlatT* v2 = this->sized(7);
  v2->insert(this->element(6));

  ASSERT_FALSE(*this->flat == *v2);
  ASSERT_FALSE(*v2 == *this->flat);

  ASSERT_TRUE(*this->flat != *v2);
  ASSERT_TRUE(*v2 != *this->flat);

  ASSERT_TRUE(*this->flat < *v2);
  ASSERT_FALSE(*v2 < *this->flat);

  ASSERT_TRUE(*this->flat <= *v2);
  ASSERT_FALSE(*v2 <= *this->flat);

  ASSERT_FALSE(*this->flat > *v2);
  ASSERT_TRUE(*v2 > *this->flat);

  ASSERT_FALSE(*this->flat >= *v2);
  ASSERT_TRUE(*v2 >= *this->flat);

  delete v2;
}

TYPED_TEST(FlatMapTest, testOperatorSubscriptNew)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  int* ptr;

  ptr = &(*this->flat)[3];
  ASSERT_EQ(&this->chooseIt(2)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ptr = &(*this->flat)[20];
  ASSERT_EQ(&this->chooseIt(7)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ptr = &(*this->flat)[-42];
  ASSERT_EQ(&this->chooseIt(0)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ptr = &(*this->flat)[100];
  ASSERT_EQ(&this->chooseIt(13)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ptr = &(*this->flat)[50];
  ASSERT_EQ(&this->chooseIt(10)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ASSERT_EQ(15, this->flat->size());

  ASSERT_EQ(this->chooseIt(0), this->flat->find(-42));
  ASSERT_EQ(this->chooseIt(3), this->flat->find(3));
  ASSERT_EQ(this->chooseIt(8), this->flat->find(20));
  ASSERT_EQ(this->chooseIt(10), this->flat->find(50));
  ASSERT_EQ(this->chooseIt(14), this->flat->find(100));
}

TYPED_TEST(FlatMapTest, testOperatorSubscriptExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  int* ptr;

  ptr = &(*this->flat)[13];
  ASSERT_EQ(&this->chooseIt(4)->second, ptr);
  ASSERT_EQ(13, *ptr);

  ptr = &(*this->flat)[8];
  ASSERT_EQ(&this->chooseIt(2)->second, ptr);
  ASSERT_EQ(8, *ptr);

  ptr = &(*this->flat)[75];
  ASSERT_EQ(&this->chooseIt(9)->second, ptr);
  ASSERT_EQ(75, *ptr);

  ptr = &(*this->flat)[0];
  ASSERT_EQ(&this->chooseIt(0)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}

TYPED_TEST(FlatMapTest, testAtExisting)
{
  IMPORT_DEFINES

  ValueT tab[10] = { this->element(13), this->element(8),  this->element(66), this->element(56),
                     this->element(11), this->element(15), this->element(0),  this->element(1),
                     this->element(75), this->element(34) };
  this->flat = new FlatT(tab + 0, tab + 10);

  int* ptr;

  ptr = &this->flat->at(13);
  ASSERT_EQ(&this->chooseIt(4)->second, ptr);
  ASSERT_EQ(13, *ptr);

  ptr = &this->flat->at(8);
  ASSERT_EQ(&this->chooseIt(2)->second, ptr);
  ASSERT_EQ(8, *ptr);

  ptr = &this->flat->at(75);
  ASSERT_EQ(&this->chooseIt(9)->second, ptr);
  ASSERT_EQ(75, *ptr);

  ptr = &this->flat->at(0);
  ASSERT_EQ(&this->chooseIt(0)->second, ptr);
  ASSERT_EQ(0, *ptr);

  ASSERT_EQ(10, this->flat->size());

  ASSERT_EQ(13, this->mapped(*this->flat->find(13)));
  ASSERT_EQ(8, this->mapped(*this->flat->find(8)));
  ASSERT_EQ(75, this->mapped(*this->flat->find(75)));
  ASSERT_EQ(0, this->mapped(*this->flat->find(0)));
}
}
