
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Utils/SopCartesianGenerator.h"

#include <vector>
#include <sstream>
#include <iostream>

using namespace tse::utils;

namespace tse
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

// transforms [1, 3], [1, 7], [2, 3], [2, 7]
// into: [13, 17, 23, 27]
std::vector<int>
tuplesToNumbers(const std::vector<SopCombination>& cartesian)
{
  std::vector<int> result;
  for (unsigned int i = 0; i < cartesian.size(); ++i)
  {
    const SopCombination& comb = cartesian[i];
    std::stringstream out;
    for (unsigned int j = 0; j < comb.size(); ++j)
    {
      // Append a single digit as sting
      std::ostringstream tmp;
      tmp << comb[j];
      CPPUNIT_ASSERT(tmp.str().size() == 1);
      out << tmp.str();
    }
    int n;
    out >> n;
    result.push_back(n);
  }
  return result;
}

class SopCartesianGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SopCartesianGeneratorTest);
  CPPUNIT_TEST(noInputSopsGeneration);
  CPPUNIT_TEST(legZeroGapTest);
  CPPUNIT_TEST(trailingLegGapTest);
  CPPUNIT_TEST(legOneGapTest);
  CPPUNIT_TEST(multipleGapTest);
  CPPUNIT_TEST(oneLegGeneration);
  CPPUNIT_TEST(twoLegsGeneration);
  CPPUNIT_TEST(threeLegsGeneration);
  CPPUNIT_TEST(contentInspectionTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _gen = _memHandle.create<SopCartesianGenerator>(); }

  void tearDown() { _memHandle.clear(); }

  void noInputSopsGeneration() { checkGeneratorExhausted(); }

  void legZeroGapTest()
  {
    _gen->setNumberOfLegs(2);
    _gen->addSop(1, 2);
    _gen->addSop(1, 4);
    _gen->addSop(1, 8);
    checkGeneratorExhausted();
  }

  void trailingLegGapTest()
  {
    _gen->setNumberOfLegs(4);
    _gen->addSop(0, 2);
    _gen->addSop(1, 4);
    _gen->addSop(2, 8);
    _gen->addSop(2, 2);
    checkGeneratorExhausted();
  }

  void legOneGapTest()
  {
    _gen->setNumberOfLegs(4);
    _gen->addSop(0, 5);
    _gen->addSop(0, 11);
    _gen->addSop(2, 8);
    _gen->addSop(2, 53);
    _gen->addSop(3, 6);
    checkGeneratorExhausted();
  }

  void multipleGapTest()
  {
    _gen->setNumberOfLegs(6);
    _gen->addSop(0, 5);
    _gen->addSop(0, 1);
    _gen->addSop(2, 85);
    _gen->addSop(2, 5);
    _gen->addSop(4, 11);
    checkGeneratorExhausted();
  }

  void oneLegGeneration()
  {
    _gen->setNumberOfLegs(1);
    _gen->addSop(0, 2);
    _gen->addSop(0, 4);
    _gen->addSop(0, 8);

    std::vector<SopCombination> cart;

    SopCombination comb = _gen->next();
    while (!comb.empty())
    {
      cart.push_back(comb);
      comb = _gen->next();
    }
    CPPUNIT_ASSERT(cart.size() == 3);

    std::vector<int> answer;
    answer.push_back(2);
    answer.push_back(4);
    answer.push_back(8);

    CPPUNIT_ASSERT(answer == tuplesToNumbers(cart));

    checkGeneratorExhausted();
  }

  // Leg
  //    0    1
  // ---------
  //    2    3
  //    4    5
  //    8    9
  void twoLegsGeneration()
  {
    _gen->setNumberOfLegs(2);
    _gen->addSop(0, 2);
    _gen->addSop(0, 4);
    _gen->addSop(0, 8);

    _gen->addSop(1, 3);
    _gen->addSop(1, 5);
    _gen->addSop(1, 9);

    std::vector<SopCombination> cart;
    SopCombination comb = _gen->next();
    while (!comb.empty())
    {
      cart.push_back(comb);
      comb = _gen->next();
    }
    CPPUNIT_ASSERT(cart.size() == 9);

    std::vector<int> answer;
    answer.push_back(23);
    answer.push_back(25);
    answer.push_back(29);
    answer.push_back(43);
    answer.push_back(45);
    answer.push_back(49);
    answer.push_back(83);
    answer.push_back(85);
    answer.push_back(89);

    CPPUNIT_ASSERT(answer == tuplesToNumbers(cart));

    checkGeneratorExhausted();
  }

  // Leg
  //    0    1    2
  // --------------
  //    1    4    8
  //    2    5    9
  //         6
  void threeLegsGeneration()
  {
    _gen->setNumberOfLegs(3);
    _gen->addSop(0, 1);
    _gen->addSop(0, 2);

    _gen->addSop(1, 4);
    _gen->addSop(1, 5);
    _gen->addSop(1, 6);

    _gen->addSop(2, 8);
    _gen->addSop(2, 9);

    std::vector<SopCombination> cart;
    SopCombination comb = _gen->next();
    while (!comb.empty())
    {
      cart.push_back(comb);
      comb = _gen->next();
    }

    CPPUNIT_ASSERT(cart.size() == 12);

    std::vector<int> answer;

    answer.push_back(148);
    answer.push_back(149);
    answer.push_back(158);
    answer.push_back(159);
    answer.push_back(168);
    answer.push_back(169);

    answer.push_back(248);
    answer.push_back(249);
    answer.push_back(258);
    answer.push_back(259);
    answer.push_back(268);
    answer.push_back(269);

    CPPUNIT_ASSERT(answer == tuplesToNumbers(cart));

    checkGeneratorExhausted();
  }

  // Leg
  //    0    1
  // ---------
  //    2    3
  //    4    5
  //    8
  void contentInspectionTest()
  {
    _gen->setNumberOfLegs(2);
    _gen->addSop(0, 2);
    _gen->addSop(0, 4);
    _gen->addSop(0, 8);

    _gen->addSop(1, 3);
    _gen->addSop(1, 5);

    CPPUNIT_ASSERT_EQUAL(2u, _gen->getNumberOfLegs());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _gen->getSopsOnLeg(0).size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _gen->getSopsOnLeg(1).size());

    SopCombination comb;
    comb.push_back(2);
    comb.push_back(4);
    comb.push_back(8);
    CPPUNIT_ASSERT(comb == _gen->getSopsOnLeg(0));
    comb.clear();
    comb.push_back(3);
    comb.push_back(5);
    CPPUNIT_ASSERT(comb == _gen->getSopsOnLeg(1));
  }

private:
  void checkGeneratorExhausted()
  {
    for (int i = 0; i < 100; ++i)
    {
      const SopCombination comb = _gen->next();
      CPPUNIT_ASSERT(comb.empty());
    }
  }

  TestMemHandle _memHandle;
  SopCartesianGenerator* _gen;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SopCartesianGeneratorTest);

} // namespace tse
