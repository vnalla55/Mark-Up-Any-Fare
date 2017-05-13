#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Util/CartesianProduct.h"
#include <boost/assign/std/vector.hpp>

namespace tse
{

using namespace boost::assign;

class CartesianProductTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CartesianProductTest);

  CPPUNIT_TEST(testCartesianProductAllSetsNotEmpty1);
  CPPUNIT_TEST(testCartesianProductAllSetsNotEmpty2);
  CPPUNIT_TEST(testCartesianProductAllSetsEmpty);
  CPPUNIT_TEST(testCartesianProductOneSetEmptyThreeNotEmpty);
  CPPUNIT_TEST(testCartesianProductOnlyOneSetNotEmpty);
  CPPUNIT_TEST(testCartesianProductOnlyOneSetAndEmpty);
  CPPUNIT_TEST(testCartesianProductCheckAllPermutations);
  CPPUNIT_TEST(testCartesianProductNoSets);

  CPPUNIT_TEST_SUITE_END();

  CartesianProduct<std::vector<char> >* _cartProd;
  TestMemHandle _memH;

public:
  void setUp() { _cartProd = _memH.insert(new CartesianProduct<std::vector<char> >); }

  void tearDown() { _memH.clear(); }

  void testCartesianProductAllSetsNotEmpty1()
  {
    std::vector<char> v1;
    std::vector<char> v2;
    std::vector<char> v3;

    v1 += 'A', 'B';
    v2 += 'X', 'Y';
    v3 += '1', '2';

    _cartProd->addSet(v1);
    _cartProd->addSet(v2);
    _cartProd->addSet(v3);

    CPPUNIT_ASSERT_EQUAL(size_t(8), _cartProd->size());
  }

  void testCartesianProductAllSetsNotEmpty2()
  {
    std::vector<char> v1;
    std::vector<char> v2;
    std::vector<char> v3;
    std::vector<char> v4;

    v1 += 'A', 'B';
    v2 += 'X', 'Y';
    v3 += '1', '2';
    v4 += '3', '4';

    _cartProd->addSet(v1);
    _cartProd->addSet(v2);
    _cartProd->addSet(v3);
    _cartProd->addSet(v4);

    CPPUNIT_ASSERT_EQUAL(size_t(16), _cartProd->size());
  }

  void testCartesianProductAllSetsEmpty()
  {
    std::vector<char> v1;
    std::vector<char> v2;
    std::vector<char> v3;
    std::vector<char> v4;

    _cartProd->addSet(v1);
    _cartProd->addSet(v2);
    _cartProd->addSet(v3);
    _cartProd->addSet(v4);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _cartProd->size());
  }

  void testCartesianProductOneSetEmptyThreeNotEmpty()
  {
    std::vector<char> v1_empty;
    std::vector<char> v2;
    std::vector<char> v3;
    std::vector<char> v4;

    v2 += 'X', 'Y';
    v3 += '1', '2';
    v4 += '3', '4';

    _cartProd->addSet(v1_empty);
    _cartProd->addSet(v2);
    _cartProd->addSet(v3);
    _cartProd->addSet(v4);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _cartProd->size());
  }

  void testCartesianProductOnlyOneSetNotEmpty()
  {
    std::vector<char> v2;

    v2.push_back('X');
    v2.push_back('Y');
    v2.push_back('1');

    _cartProd->addSet(v2);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _cartProd->size());
  }

  void testCartesianProductOnlyOneSetAndEmpty()
  {
    std::vector<char> v2;
    _cartProd->addSet(v2);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _cartProd->size());
    CPPUNIT_ASSERT(_cartProd->getNext().empty());
  }

  void testCartesianProductCheckAllPermutations()
  {
    CartesianProduct<std::vector<int> > cartProd;
    std::vector<int> v1;
    std::vector<int> v2;
    std::vector<int> v3;

    v1 += 1, 2;
    v2 += 3, 4;
    v3 += 5, 6;

    cartProd.addSet(v1);
    cartProd.addSet(v2);
    cartProd.addSet(v3);

    CPPUNIT_ASSERT_EQUAL(size_t(8), cartProd.size());

    CartesianProduct<std::vector<int> >::ProductType p = cartProd.getNext();
    int counter = 0;
    while (!p.empty())
    {
      counter++;
      int resp = 0;

      typedef CartesianProduct<std::vector<int> >::ProductType::iterator It;
      for (It i = p.begin(); i != p.end(); ++i)
      {
        resp = resp * 10 + (*i);
      }

      switch (counter)
      {
      case 1:
        CPPUNIT_ASSERT_EQUAL(135, resp);
        break;
      case 2:
        CPPUNIT_ASSERT_EQUAL(136, resp);
        break;
      case 3:
        CPPUNIT_ASSERT_EQUAL(145, resp);
        break;
      case 4:
        CPPUNIT_ASSERT_EQUAL(146, resp);
        break;
      case 5:
        CPPUNIT_ASSERT_EQUAL(235, resp);
        break;
      case 6:
        CPPUNIT_ASSERT_EQUAL(236, resp);
        break;
      case 7:
        CPPUNIT_ASSERT_EQUAL(245, resp);
        break;
      case 8:
        CPPUNIT_ASSERT_EQUAL(246, resp);
        break;
      default:
        break;
      }

      p = cartProd.getNext();
    }
  }

  void testCartesianProductNoSets()
  {
    CPPUNIT_ASSERT_EQUAL(size_t(0), _cartProd->size());
    CPPUNIT_ASSERT(_cartProd->getNext().empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CartesianProductTest);
}
