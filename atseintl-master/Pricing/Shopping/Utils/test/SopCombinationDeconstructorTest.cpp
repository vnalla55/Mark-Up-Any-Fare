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
#include "Pricing/Shopping/Utils/SopCombinationDeconstructor.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

namespace tse
{

namespace utils
{

const unsigned int DUMMY_SOP_1 = 3;
const unsigned int DUMMY_SOP_2 = 2;
const unsigned int DUMMY_SOP_3 = 58;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class SopCombinationDeconstructorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SopCombinationDeconstructorTest);
  CPPUNIT_TEST(oneLegTest);
  CPPUNIT_TEST(twoLegTest);
  CPPUNIT_TEST(threeLegTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _dec = _memHandle.create<SopCombinationDeconstructor>();
    _c = _memHandle.create<SopCombination>();
    _tmp = _memHandle.create<SopCombinationDeconstructor::PartVector>();
  }

  void tearDown() { _memHandle.clear(); }

  void oneLegTest()
  {
    sopOnNextLeg(DUMMY_SOP_1);
    _dec->insertItem(*_c);
    TSE_ASSERT(*_tmp == _dec->getItemParts());
  }

  void twoLegTest()
  {
    sopOnNextLeg(DUMMY_SOP_1);
    sopOnNextLeg(DUMMY_SOP_2);
    _dec->insertItem(*_c);
    TSE_ASSERT(*_tmp == _dec->getItemParts());
  }

  void threeLegTest()
  {
    sopOnNextLeg(DUMMY_SOP_1);
    sopOnNextLeg(DUMMY_SOP_2);
    sopOnNextLeg(DUMMY_SOP_3);
    _dec->insertItem(*_c);
    TSE_ASSERT(*_tmp == _dec->getItemParts());
  }

private:
  void sopOnNextLeg(unsigned int sop)
  {
    _tmp->push_back(SopEntry(_c->size(), sop));
    _c->push_back(sop);
  }

  TestMemHandle _memHandle;

  SopCombinationDeconstructor* _dec;
  SopCombination* _c;
  SopCombinationDeconstructor::PartVector* _tmp;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SopCombinationDeconstructorTest);

} // namespace utils

} // namespace tse
