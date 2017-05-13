// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#ifndef _GLIBCXX_DEBUG

#include "Util/Vector.h"

#include <stdexcept>
#include <tr1/unordered_set>

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
class CheckedObject;

class Snapshot
{
public:
  void recordCtor(CheckedObject* obj)
  {
    if (_objects.find(obj) != _objects.end())
      throw std::runtime_error("Constructor called twice on same address");
    _objects.insert(obj);
  }

  void recordDtor(CheckedObject* obj)
  {
    if (_objects.find(obj) == _objects.end())
      throw std::runtime_error("Destructor called twice on same address");
    _objects.erase(obj);
  }

  bool isEmpty() { return _objects.empty(); }

  friend bool operator==(const Snapshot& l, const Snapshot& r)
  {
    if (l._objects.size() != r._objects.size())
      return false;

    for (CheckedObject* obj : l._objects)
    {
      if (r._objects.find(obj) == r._objects.end())
        return false;
    }

    return true;
  }

  friend bool operator!=(const Snapshot& l, const Snapshot& r) { return !(l == r); }

private:
  std::tr1::unordered_set<CheckedObject*> _objects;
};

class CheckedObject
{
public:
  CheckedObject(int v = 0) : value(v) { snapshot.recordCtor(this); }
  CheckedObject(const CheckedObject& o) : value(o.value) { snapshot.recordCtor(this); }
  ~CheckedObject() { snapshot.recordDtor(this); }

  int value;

  friend bool operator==(const CheckedObject& l, const CheckedObject& r)
  {
    return l.value == r.value;
  }

  friend bool operator<(const CheckedObject& l, const CheckedObject& r)
  {
    return l.value < r.value;
  }

  static Snapshot snapshot;
};

Snapshot CheckedObject::snapshot;

class AssertNothingChanged
{
public:
  AssertNothingChanged() : _snapshot(CheckedObject::snapshot) {}
  ~AssertNothingChanged()
  {
    if (_snapshot != CheckedObject::snapshot)
      throw std::runtime_error("The number of objects changed");
  }

private:
  Snapshot _snapshot;
};

template <typename TypeParam>
class VectorTest : public ::testing::Test
{
public:
  typedef Vector<TypeParam> VectorT;
  typedef std::vector<TypeParam> VectorStd;

  VectorT* vector;

  void SetUp() { vector = 0; }
  void TearDown()
  {
    delete vector;
    ASSERT_TRUE(CheckedObject::snapshot.isEmpty());
  }

  static int value(const TypeParam& v);
  static void setValue(TypeParam& v, int value);
};

template <>
int
VectorTest<int>::value(const int& v)
{
  return v;
}

template <>
void
VectorTest<int>::setValue(int& v, int value)
{
  v = value;
}

template <>
int
VectorTest<CheckedObject>::value(const CheckedObject& v)
{
  return v.value;
}

template <>
void
VectorTest<CheckedObject>::setValue(CheckedObject& v, int value)
{
  v = value;
}

template <>
int
VectorTest<std::vector<int> >::value(const std::vector<int>& v)
{
  return v.size();
}

template <>
void
VectorTest<std::vector<int> >::setValue(std::vector<int>& v, int value)
{
  v.resize(value);
}

template <>
int
VectorTest<std::vector<CheckedObject> >::value(const std::vector<CheckedObject>& v)
{
  return v.size();
}

template <>
void
VectorTest<std::vector<CheckedObject> >::setValue(std::vector<CheckedObject>& v, int value)
{
  v.resize(value);
}

template <>
int
VectorTest<Vector<int> >::value(const Vector<int>& v)
{
  return v.size();
}

template <>
void
VectorTest<Vector<int> >::setValue(Vector<int>& v, int value)
{
  v.resize(value);
}

template <>
int
VectorTest<Vector<CheckedObject> >::value(const Vector<CheckedObject>& v)
{
  return v.size();
}

template <>
void
VectorTest<Vector<CheckedObject> >::setValue(Vector<CheckedObject>& v, int value)
{
  v.resize(value);
}

typedef ::testing::Types<
  int, CheckedObject,
  std::vector<int>, std::vector<CheckedObject>,
  Vector<int>, Vector<CheckedObject> > VectorTestTypes;
TYPED_TEST_CASE(VectorTest, VectorTestTypes);

TYPED_TEST(VectorTest, testSizeIsOk)
{
  ASSERT_EQ(size_t(16), sizeof(*this->vector));
}

TYPED_TEST(VectorTest, testConstructorDefault)
{
  this->vector = new Vector<TypeParam>;

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testConstructorFillZero)
{
  this->vector = new Vector<TypeParam>(0);

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testConstructorFillOne)
{
  this->vector = new Vector<TypeParam>(1);

  ASSERT_EQ(1, this->vector->size());
  ASSERT_EQ(0, this->value(this->vector->at(0)));
}

TYPED_TEST(VectorTest, testConstructorFillMany)
{
  this->vector = new Vector<TypeParam>(123);

  ASSERT_EQ(123, this->vector->size());
  ASSERT_EQ(0, this->value(this->vector->back()));
}

TYPED_TEST(VectorTest, testConstructorFillCopyZero)
{
  this->vector = new Vector<TypeParam>(0, TypeParam(1));

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testConstructorFillCopyOne)
{
  this->vector = new Vector<TypeParam>(1, TypeParam(2));

  ASSERT_EQ(1, this->vector->size());
  ASSERT_EQ(2, this->value(this->vector->back()));
}

TYPED_TEST(VectorTest, testConstructorFillCopyMany)
{
  this->vector = new Vector<TypeParam>(123, TypeParam(3));

  ASSERT_EQ(123, this->vector->size());
  ASSERT_EQ(3, this->value(this->vector->at(100)));
}

TYPED_TEST(VectorTest, testConstructorRangeEmpty)
{
  TypeParam tab[0];
  this->vector = new Vector<TypeParam>(tab + 0, tab + 0);

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testConstructorRangeNotEmpty)
{
  TypeParam tab[3] = { TypeParam(1), TypeParam(2), TypeParam(3) };
  this->vector = new Vector<TypeParam>(tab + 0, tab + 3);

  ASSERT_EQ(3, this->vector->size());
  ASSERT_EQ(1, this->value(this->vector->front()));
}

TYPED_TEST(VectorTest, testConstructorCopy)
{
  this->vector = new Vector<TypeParam>(Vector<TypeParam>(3, TypeParam(7)));

  ASSERT_EQ(3, this->vector->size());
  ASSERT_EQ(7, this->value(this->vector->at(1)));
}

TYPED_TEST(VectorTest, testConstructorCopyStdVector)
{
  this->vector = new Vector<TypeParam>(std::vector<TypeParam>(2, TypeParam(3)));

  ASSERT_EQ(2, this->vector->size());
  ASSERT_EQ(3, this->value(this->vector->front()));
}

TYPED_TEST(VectorTest, testAssignment)
{
  this->vector = new Vector<TypeParam>(8, TypeParam(1));

  *this->vector = Vector<TypeParam>(4);

  ASSERT_EQ(4, this->vector->size());
  ASSERT_EQ(0, this->value(this->vector->back()));
}

TYPED_TEST(VectorTest, testAssignFillZero)
{
  this->vector = new Vector<TypeParam>(13);

  this->vector->assign(0, TypeParam(1));

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testAssignFillOne)
{
  this->vector = new Vector<TypeParam>(13);

  this->vector->assign(1, TypeParam(2));

  ASSERT_EQ(1, this->vector->size());
  ASSERT_EQ(2, this->value(this->vector->back()));
}

TYPED_TEST(VectorTest, testAssignFillMany)
{
  this->vector = new Vector<TypeParam>(13);

  this->vector->assign(123, TypeParam(3));

  ASSERT_EQ(123, this->vector->size());
  ASSERT_EQ(3, this->value(this->vector->at(100)));
}

TYPED_TEST(VectorTest, testAssignRangeEmpty)
{
  this->vector = new Vector<TypeParam>(13);

  TypeParam tab[0];
  this->vector->assign(tab + 0, tab + 0);

  ASSERT_EQ(0, this->vector->size());
}

TYPED_TEST(VectorTest, testAssignRangeNotEmpty)
{
  this->vector = new Vector<TypeParam>(13);

  TypeParam tab[3] = { TypeParam(1), TypeParam(2), TypeParam(3) };
  this->vector->assign(tab + 0, tab + 3);

  ASSERT_EQ(3, this->vector->size());
  ASSERT_EQ(1, this->value(this->vector->front()));
}

TYPED_TEST(VectorTest, testResizeEqual)
{
  this->vector = new Vector<TypeParam>(13);

  {
    AssertNothingChanged nc;
    this->vector->resize(13);
  }

  ASSERT_EQ(13, this->vector->size());
}

TYPED_TEST(VectorTest, testResizeSmaller)
{
  this->vector = new Vector<TypeParam>(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->resize(8);

  ASSERT_EQ(8, this->vector->size());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
}

TYPED_TEST(VectorTest, testResizeBarelyBigger)
{
  this->vector = new Vector<TypeParam>;
  this->vector->reserve(14);
  this->vector->resize(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->resize(14);

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
  ASSERT_EQ(0, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testResizeBigger)
{
  this->vector = new Vector<TypeParam>(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->resize(30);

  ASSERT_EQ(30, this->vector->size());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
  ASSERT_EQ(0, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testResizeBiggerCopy)
{
  this->vector = new Vector<TypeParam>(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->resize(30, TypeParam(23));

  ASSERT_EQ(30, this->vector->size());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
  ASSERT_EQ(23, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testReserveEqual)
{
  this->vector = new Vector<TypeParam>(13);
  ASSERT_EQ(13, this->vector->capacity());

  {
    AssertNothingChanged nc;
    this->vector->reserve(13);
  }
}

TYPED_TEST(VectorTest, testReserveSmaller)
{
  this->vector = new Vector<TypeParam>(13);

  this->vector->reserve(28);
  ASSERT_EQ(28, this->vector->capacity());

  {
    AssertNothingChanged nc;
    this->vector->reserve(20);
  }
}

TYPED_TEST(VectorTest, testReserveSmallerThanSize)
{
  this->vector = new Vector<TypeParam>(13);
  ASSERT_EQ(13, this->vector->capacity());

  {
    AssertNothingChanged nc;
    this->vector->reserve(7);
  }
}

TYPED_TEST(VectorTest, testReserveBarelyBigger)
{
  this->vector = new Vector<TypeParam>(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->reserve(14);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
  ASSERT_EQ(0, this->value(this->vector->at(12)));
}

TYPED_TEST(VectorTest, testReserveBigger)
{
  this->vector = new Vector<TypeParam>(13);

  this->setValue(this->vector->at(0), 7);
  this->vector->reserve(30);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(30, this->vector->capacity());
  ASSERT_EQ(7, this->value(this->vector->at(0)));
  ASSERT_EQ(0, this->value(this->vector->at(12)));
}

TYPED_TEST(VectorTest, testShrinkToFit)
{
  this->vector = new Vector<TypeParam>(13);

  this->vector->push_back(TypeParam());
  this->vector->shrink_to_fit();

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());
}

TYPED_TEST(VectorTest, testClear)
{
  this->vector = new Vector<TypeParam>(13);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->clear();

  ASSERT_EQ(0, this->vector->size());
  ASSERT_EQ(0, this->vector->capacity());
}

TYPED_TEST(VectorTest, testPushBackWithSpare)
{
  this->vector = new Vector<TypeParam>(13);
  this->vector->reserve(14);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  this->vector->push_back(TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  ASSERT_EQ(0, this->value(this->vector->at(0)));
  ASSERT_EQ(0, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testPushBackNoSpare)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(123));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->push_back(TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_LE(14, this->vector->capacity());

  ASSERT_EQ(123, this->value(this->vector->at(0)));
  ASSERT_EQ(123, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testPopBackNoSpare)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(567));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->pop_back();

  ASSERT_EQ(12, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(567, this->value(this->vector->at(0)));
  ASSERT_EQ(567, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testPopBackBigSpare)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(1337));
  this->vector->reserve(50);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(50, this->vector->capacity());

  this->vector->pop_back();

  ASSERT_EQ(12, this->vector->size());
  ASSERT_GT(50, this->vector->capacity());
  ASSERT_LE(12, this->vector->capacity());

  ASSERT_EQ(1337, this->value(this->vector->at(0)));
  ASSERT_EQ(1337, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testInsertWithSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(23));
  this->vector->reserve(14);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  this->vector->insert(this->vector->end(), TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  ASSERT_EQ(23, this->value(this->vector->at(0)));
  ASSERT_EQ(23, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertWithSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(23));
  this->vector->reserve(14);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  this->vector->insert(this->vector->begin(), TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  ASSERT_EQ(42, this->value(this->vector->at(0)));
  ASSERT_EQ(23, this->value(this->vector->at(12)));
  ASSERT_EQ(23, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertWithSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(23));
  this->vector->reserve(14);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  this->vector->insert(this->vector->begin() + 7, TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_EQ(14, this->vector->capacity());

  ASSERT_EQ(23, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(7)));
  ASSERT_EQ(23, this->value(this->vector->at(12)));
  ASSERT_EQ(23, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertNoSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(234));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->end(), TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_LE(14, this->vector->capacity());

  ASSERT_EQ(234, this->value(this->vector->at(0)));
  ASSERT_EQ(234, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertNoSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(234));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->begin(), TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_LE(14, this->vector->capacity());

  ASSERT_EQ(42, this->value(this->vector->at(0)));
  ASSERT_EQ(234, this->value(this->vector->at(12)));
  ASSERT_EQ(234, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertNoSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(234));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->begin() + 7, TypeParam(42));

  ASSERT_EQ(14, this->vector->size());
  ASSERT_LE(14, this->vector->capacity());

  ASSERT_EQ(234, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(7)));
  ASSERT_EQ(234, this->value(this->vector->at(12)));
  ASSERT_EQ(234, this->value(this->vector->at(13)));
}

TYPED_TEST(VectorTest, testInsertManyWithSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(6745));
  this->vector->reserve(18);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  this->vector->insert(this->vector->end(), 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  ASSERT_EQ(6745, this->value(this->vector->at(0)));
  ASSERT_EQ(6745, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
  ASSERT_EQ(42, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testInsertManyWithSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(6745));
  this->vector->reserve(18);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  this->vector->insert(this->vector->begin(), 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  ASSERT_EQ(42, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(4)));
  ASSERT_EQ(6745, this->value(this->vector->at(5)));
  ASSERT_EQ(6745, this->value(this->vector->at(12)));
  ASSERT_EQ(6745, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testInsertManyWithSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(6745));
  this->vector->reserve(18);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  this->vector->insert(this->vector->begin() + 7, 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_EQ(18, this->vector->capacity());

  ASSERT_EQ(6745, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(7)));
  ASSERT_EQ(42, this->value(this->vector->at(11)));
  ASSERT_EQ(6745, this->value(this->vector->at(12)));
  ASSERT_EQ(6745, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testInsertManyNoSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(9654));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->end(), 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_LE(18, this->vector->capacity());

  ASSERT_EQ(9654, this->value(this->vector->at(0)));
  ASSERT_EQ(9654, this->value(this->vector->at(12)));
  ASSERT_EQ(42, this->value(this->vector->at(13)));
  ASSERT_EQ(42, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testInsertManyNoSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(9654));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->begin(), 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_LE(18, this->vector->capacity());

  ASSERT_EQ(42, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(4)));
  ASSERT_EQ(9654, this->value(this->vector->at(5)));
  ASSERT_EQ(9654, this->value(this->vector->at(12)));
  ASSERT_EQ(9654, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testInsertManyNoSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(9654));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->insert(this->vector->begin() + 7, 5, TypeParam(42));

  ASSERT_EQ(18, this->vector->size());
  ASSERT_LE(18, this->vector->capacity());

  ASSERT_EQ(9654, this->value(this->vector->at(0)));
  ASSERT_EQ(42, this->value(this->vector->at(7)));
  ASSERT_EQ(42, this->value(this->vector->at(11)));
  ASSERT_EQ(9654, this->value(this->vector->at(12)));
  ASSERT_EQ(9654, this->value(this->vector->at(17)));
}

TYPED_TEST(VectorTest, testEraseNoSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(635));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->end() - 1);

  ASSERT_EQ(12, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(635, this->value(this->vector->at(0)));
  ASSERT_EQ(635, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseNoSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(635));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->begin());

  ASSERT_EQ(12, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(635, this->value(this->vector->at(0)));
  ASSERT_EQ(635, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseNoSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(635));

  this->setValue(this->vector->at(6), 123);
  this->setValue(this->vector->at(7), 0);
  this->setValue(this->vector->at(8), 321);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->begin() + 7);

  ASSERT_EQ(12, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(635, this->value(this->vector->at(0)));
  ASSERT_EQ(635, this->value(this->vector->at(5)));
  ASSERT_EQ(123, this->value(this->vector->at(6)));
  ASSERT_EQ(321, this->value(this->vector->at(7)));
  ASSERT_EQ(635, this->value(this->vector->at(8)));
  ASSERT_EQ(635, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseBigSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(875));
  this->vector->reserve(52);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(52, this->vector->capacity());

  this->vector->erase(this->vector->end() - 1);

  ASSERT_EQ(12, this->vector->size());
  ASSERT_LE(12, this->vector->capacity());
  ASSERT_GT(52, this->vector->capacity());

  ASSERT_EQ(875, this->value(this->vector->at(0)));
  ASSERT_EQ(875, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseBigSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(875));
  this->vector->reserve(52);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(52, this->vector->capacity());

  this->vector->erase(this->vector->begin());

  ASSERT_EQ(12, this->vector->size());
  ASSERT_LE(12, this->vector->capacity());
  ASSERT_GT(52, this->vector->capacity());

  ASSERT_EQ(875, this->value(this->vector->at(0)));
  ASSERT_EQ(875, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseBigSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(875));
  this->vector->reserve(52);

  this->setValue(this->vector->at(6), 567);
  this->setValue(this->vector->at(7), 0);
  this->setValue(this->vector->at(8), 789);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(52, this->vector->capacity());

  this->vector->erase(this->vector->begin() + 7);

  ASSERT_EQ(12, this->vector->size());
  ASSERT_LE(12, this->vector->capacity());
  ASSERT_GT(52, this->vector->capacity());

  ASSERT_EQ(875, this->value(this->vector->at(0)));
  ASSERT_EQ(875, this->value(this->vector->at(5)));
  ASSERT_EQ(567, this->value(this->vector->at(6)));
  ASSERT_EQ(789, this->value(this->vector->at(7)));
  ASSERT_EQ(875, this->value(this->vector->at(8)));
  ASSERT_EQ(875, this->value(this->vector->at(11)));
}

TYPED_TEST(VectorTest, testEraseManyNoSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(7805));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->end() - 2, this->vector->end());

  ASSERT_EQ(11, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(7805, this->value(this->vector->at(0)));
  ASSERT_EQ(7805, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseManyNoSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(7805));

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->begin(), this->vector->begin() + 2);

  ASSERT_EQ(11, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(7805, this->value(this->vector->at(0)));
  ASSERT_EQ(7805, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseManyNoSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(7805));

  this->setValue(this->vector->at(6), 67985);
  this->setValue(this->vector->at(7), 0);
  this->setValue(this->vector->at(8), 0);
  this->setValue(this->vector->at(9), 4576);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  this->vector->erase(this->vector->begin() + 7, this->vector->begin() + 9);

  ASSERT_EQ(11, this->vector->size());
  ASSERT_EQ(13, this->vector->capacity());

  ASSERT_EQ(7805, this->value(this->vector->at(0)));
  ASSERT_EQ(7805, this->value(this->vector->at(5)));
  ASSERT_EQ(67985, this->value(this->vector->at(6)));
  ASSERT_EQ(4576, this->value(this->vector->at(7)));
  ASSERT_EQ(7805, this->value(this->vector->at(8)));
  ASSERT_EQ(7805, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseManyBigSpareEnd)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(3867));
  this->vector->reserve(53);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(53, this->vector->capacity());

  this->vector->erase(this->vector->end() - 2, this->vector->end());

  ASSERT_EQ(11, this->vector->size());
  ASSERT_LE(11, this->vector->capacity());
  ASSERT_GT(53, this->vector->capacity());

  ASSERT_EQ(3867, this->value(this->vector->at(0)));
  ASSERT_EQ(3867, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseManyBigSpareBegin)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(3867));
  this->vector->reserve(53);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(53, this->vector->capacity());

  this->vector->erase(this->vector->begin(), this->vector->begin() + 2);

  ASSERT_EQ(11, this->vector->size());
  ASSERT_LE(11, this->vector->capacity());
  ASSERT_GT(53, this->vector->capacity());

  ASSERT_EQ(3867, this->value(this->vector->at(0)));
  ASSERT_EQ(3867, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseManyBigSpareMiddle)
{
  this->vector = new Vector<TypeParam>(13, TypeParam(3867));
  this->vector->reserve(53);

  this->setValue(this->vector->at(6), 745);
  this->setValue(this->vector->at(7), 0);
  this->setValue(this->vector->at(8), 0);
  this->setValue(this->vector->at(9), 4);

  ASSERT_EQ(13, this->vector->size());
  ASSERT_EQ(53, this->vector->capacity());

  this->vector->erase(this->vector->begin() + 7, this->vector->begin() + 9);

  ASSERT_EQ(11, this->vector->size());
  ASSERT_LE(11, this->vector->capacity());
  ASSERT_GT(53, this->vector->capacity());

  ASSERT_EQ(3867, this->value(this->vector->at(0)));
  ASSERT_EQ(3867, this->value(this->vector->at(5)));
  ASSERT_EQ(745, this->value(this->vector->at(6)));
  ASSERT_EQ(4, this->value(this->vector->at(7)));
  ASSERT_EQ(3867, this->value(this->vector->at(8)));
  ASSERT_EQ(3867, this->value(this->vector->at(10)));
}

TYPED_TEST(VectorTest, testEraseAll)
{
  this->vector = new Vector<TypeParam>(11, TypeParam(463));

  ASSERT_EQ(11, this->vector->size());

  this->vector->erase(this->vector->begin(), this->vector->end());

  ASSERT_EQ(0, this->vector->size());
  ASSERT_EQ(0, this->vector->capacity());
}

TYPED_TEST(VectorTest, testSwapEmpty)
{
  this->vector = new Vector<TypeParam>();
  Vector<TypeParam> v2;

  {
    AssertNothingChanged nc;
    this->vector->swap(v2);
  }

  ASSERT_EQ(0, this->vector->size());
  ASSERT_EQ(0, this->vector->capacity());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(0, v2.capacity());
}

TYPED_TEST(VectorTest, testSwapWithEmpty)
{
  this->vector = new Vector<TypeParam>(7);
  Vector<TypeParam> v2;

  {
    AssertNothingChanged nc;
    this->vector->swap(v2);
  }

  ASSERT_EQ(0, this->vector->size());
  ASSERT_EQ(0, this->vector->capacity());
  ASSERT_EQ(7, v2.size());
  ASSERT_EQ(7, v2.capacity());
}

TYPED_TEST(VectorTest, testSwapWithEmpty2)
{
  this->vector = new Vector<TypeParam>();
  Vector<TypeParam> v2(7);

  {
    AssertNothingChanged nc;
    this->vector->swap(v2);
  }

  ASSERT_EQ(7, this->vector->size());
  ASSERT_EQ(7, this->vector->capacity());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(0, v2.capacity());
}

TYPED_TEST(VectorTest, testSwapFull)
{
  this->vector = new Vector<TypeParam>(7, TypeParam(42));
  Vector<TypeParam> v2(9, TypeParam(1337));

  {
    AssertNothingChanged nc;
    this->vector->swap(v2);
  }

  ASSERT_EQ(9, this->vector->size());
  ASSERT_EQ(9, this->vector->capacity());
  ASSERT_EQ(1337, this->value(this->vector->front()));
  ASSERT_EQ(7, v2.size());
  ASSERT_EQ(7, v2.capacity());
  ASSERT_EQ(42, this->value(v2.front()));
}

TYPED_TEST(VectorTest, testComparisonWhenEqual)
{
  this->vector = new Vector<TypeParam>(7, TypeParam(678));
  Vector<TypeParam> v2(7, TypeParam(678));

  ASSERT_TRUE(*this->vector == v2);
  ASSERT_TRUE(v2 == *this->vector);

  ASSERT_FALSE(*this->vector != v2);
  ASSERT_FALSE(v2 != *this->vector);

  ASSERT_FALSE(*this->vector < v2);
  ASSERT_FALSE(v2 < *this->vector);

  ASSERT_TRUE(*this->vector <= v2);
  ASSERT_TRUE(v2 <= *this->vector);

  ASSERT_FALSE(*this->vector > v2);
  ASSERT_FALSE(v2 > *this->vector);

  ASSERT_TRUE(*this->vector >= v2);
  ASSERT_TRUE(v2 >= *this->vector);
}

TYPED_TEST(VectorTest, testComparisonWhenBigger)
{
  this->vector = new Vector<TypeParam>(7, TypeParam(678));
  Vector<TypeParam> v2(7, TypeParam(1000));

  ASSERT_FALSE(*this->vector == v2);
  ASSERT_FALSE(v2 == *this->vector);

  ASSERT_TRUE(*this->vector != v2);
  ASSERT_TRUE(v2 != *this->vector);

  ASSERT_TRUE(*this->vector < v2);
  ASSERT_FALSE(v2 < *this->vector);

  ASSERT_TRUE(*this->vector <= v2);
  ASSERT_FALSE(v2 <= *this->vector);

  ASSERT_FALSE(*this->vector > v2);
  ASSERT_TRUE(v2 > *this->vector);

  ASSERT_FALSE(*this->vector >= v2);
  ASSERT_TRUE(v2 >= *this->vector);
}

TYPED_TEST(VectorTest, testComparisonWhenBiggerInSize)
{
  this->vector = new Vector<TypeParam>(7, TypeParam(678));
  Vector<TypeParam> v2(8, TypeParam(678));

  ASSERT_FALSE(*this->vector == v2);
  ASSERT_FALSE(v2 == *this->vector);

  ASSERT_TRUE(*this->vector != v2);
  ASSERT_TRUE(v2 != *this->vector);

  ASSERT_TRUE(*this->vector < v2);
  ASSERT_FALSE(v2 < *this->vector);

  ASSERT_TRUE(*this->vector <= v2);
  ASSERT_FALSE(v2 <= *this->vector);

  ASSERT_FALSE(*this->vector > v2);
  ASSERT_TRUE(v2 > *this->vector);

  ASSERT_FALSE(*this->vector >= v2);
  ASSERT_TRUE(v2 >= *this->vector);
}

TYPED_TEST(VectorTest, testComparisonWhenBiggerInSizeEvenIfLastLower)
{
  this->vector = new Vector<TypeParam>(7, TypeParam(678));
  Vector<TypeParam> v2(7, TypeParam(678));
  v2.resize(8);

  ASSERT_FALSE(*this->vector == v2);
  ASSERT_FALSE(v2 == *this->vector);

  ASSERT_TRUE(*this->vector != v2);
  ASSERT_TRUE(v2 != *this->vector);

  ASSERT_TRUE(*this->vector < v2);
  ASSERT_FALSE(v2 < *this->vector);

  ASSERT_TRUE(*this->vector <= v2);
  ASSERT_FALSE(v2 <= *this->vector);

  ASSERT_FALSE(*this->vector > v2);
  ASSERT_TRUE(v2 > *this->vector);

  ASSERT_FALSE(*this->vector >= v2);
  ASSERT_TRUE(v2 >= *this->vector);
}
}

#endif
