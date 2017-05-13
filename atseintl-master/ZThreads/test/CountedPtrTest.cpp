#include "ZThreads/zthread/CountedPtr.h"


#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

namespace
{

struct Setter
{
  Setter(): value(0) {}
  virtual ~Setter() {}
  virtual void set(int v) { value = v; }

  int value;
};

struct DerivedSetter: public Setter
{
  virtual ~DerivedSetter() {}
  virtual void set(int v) { value = 2 * v; }
};

} // unnamed namespace

class CountedPtrTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CountedPtrTest);
    CPPUNIT_TEST(testCountedPtr);
  CPPUNIT_TEST_SUITE_END();

  void testCountedPtr()
  {
    {
      ZThread::CountedPtr<int> ptr;
      CPPUNIT_ASSERT_EQUAL(false, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(true, !ptr);

      ZThread::CountedPtr<int> copy = ptr;
      CPPUNIT_ASSERT_EQUAL(true, copy == ptr);
      CPPUNIT_ASSERT_EQUAL(false, copy != ptr);
    }

    {
      ZThread::CountedPtr<int> ptr(new int(1));
      CPPUNIT_ASSERT_EQUAL(true, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(false, !ptr);

      ZThread::CountedPtr<int> copy = ptr;
      CPPUNIT_ASSERT_EQUAL(true, copy == ptr);
      CPPUNIT_ASSERT_EQUAL(false, copy != ptr);
    }

    {
      ZThread::CountedPtr<int> ptr(new int(1));
      CPPUNIT_ASSERT_EQUAL(true, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(false, !ptr);

      ZThread::CountedPtr<int> copy = ptr;
      CPPUNIT_ASSERT_EQUAL(true, copy == ptr);
      CPPUNIT_ASSERT_EQUAL(false, copy != ptr);

      copy = ZThread::CountedPtr<int>(new int(1));
      CPPUNIT_ASSERT_EQUAL(false, copy == ptr);
      CPPUNIT_ASSERT_EQUAL(true, copy != ptr);
    }

    {
      ZThread::CountedPtr<int> ptr(new int(1));
      CPPUNIT_ASSERT_EQUAL(true, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(false, !ptr);

      ptr.reset();
      CPPUNIT_ASSERT_EQUAL(false, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(true, !ptr);
    }

    {
      ZThread::CountedPtr<Setter> ptr(new Setter);
      CPPUNIT_ASSERT_EQUAL(true, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(false, !ptr);

      ptr->set(10);
      CPPUNIT_ASSERT_EQUAL(10, ptr->value);
      CPPUNIT_ASSERT_EQUAL(10, (*ptr).value);

      const ZThread::CountedPtr<Setter> constPtr = ptr;
      CPPUNIT_ASSERT_EQUAL(10, constPtr->value);
      CPPUNIT_ASSERT_EQUAL(10, (*constPtr).value);
    }

    {
      ZThread::CountedPtr<Setter> ptr(new DerivedSetter);
      CPPUNIT_ASSERT_EQUAL(true, bool(ptr));
      CPPUNIT_ASSERT_EQUAL(false, !ptr);

      ptr->set(10);
      CPPUNIT_ASSERT_EQUAL(20, ptr->value);
      CPPUNIT_ASSERT_EQUAL(20, (*ptr).value);

      const ZThread::CountedPtr<Setter> constPtr = ptr;
      CPPUNIT_ASSERT_EQUAL(20, constPtr->value);
      CPPUNIT_ASSERT_EQUAL(20, (*constPtr).value);
    }

    {
      ZThread::CountedPtr<DerivedSetter> derived(new DerivedSetter);
      ZThread::CountedPtr<Setter> ptr(derived);

      ptr->set(21);
      CPPUNIT_ASSERT_EQUAL(42, ptr->value);
      CPPUNIT_ASSERT_EQUAL(42, (*ptr).value);

      ptr.reset();

      ptr = derived;

      ptr->set(11);
      CPPUNIT_ASSERT_EQUAL(22, ptr->value);
      CPPUNIT_ASSERT_EQUAL(22, (*ptr).value);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CountedPtrTest);

} // namespace tse
