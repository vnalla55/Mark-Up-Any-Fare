//
// Copyright Sabre 2011-12-05
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"

class AssertTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AssertTest);
  CPPUNIT_TEST(testPassingAssertionDoesNothing);
  CPPUNIT_TEST(testFailingAssertionThrows);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPassingAssertionDoesNothing() { CPPUNIT_ASSERT_NO_THROW(TSE_ASSERT(true)); }

  void testFailingAssertionThrows()
  {
    try
    {
      TSE_ASSERT(false);
      CPPUNIT_FAIL("assertion did not throw");
    }
    catch (const tse::ErrorResponseException& e) {}
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AssertTest);
