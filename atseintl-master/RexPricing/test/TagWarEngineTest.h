//-------------------------------------------------------------------
//
//  File:        TagWarEngineTest.h
//  Created:     August 22, 2007
//  Authors:     Daniel Rolka
//
//  Updates:
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

#ifndef TAG_WAR_ENGINE_TEST_H
#define TAG_WAR_ENGINE_TEST_H

#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "RexPricing/TagWarEngine.h"
#include <iostream>

namespace tse
{

class TagWarEngineTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TagWarEngineTest);
  CPPUNIT_TEST(testMarkWithFareType_Tag1Tag1);
  CPPUNIT_TEST(testMarkWithFareType_Tag2Tag2);
  CPPUNIT_TEST(testMarkWithFareType_Tag1Tag2);
  CPPUNIT_TEST(testMarkWithFareType_Tag1Tag3);
  CPPUNIT_TEST(testMarkWithFareType_Tag2Tag3);
  CPPUNIT_TEST(testMarkWithFareType_Tag3Tag3);
  CPPUNIT_TEST(testMarkWithFareType_Tag1Tag4);
  CPPUNIT_TEST(testMarkWithFareType_Tag2Tag4);
  CPPUNIT_TEST(testMarkWithFareType_Tag3Tag4);
  CPPUNIT_TEST(testMarkWithFareType_Tag4Tag4);
  CPPUNIT_TEST(testMarkWithFareType_Tag5Tag5);
  CPPUNIT_TEST(testMarkWithFareType_Tag6Tag6_TravelCommenced);
  CPPUNIT_TEST(testMarkWithFareType_Tag6Tag6_TravelNotCommenced);
  CPPUNIT_TEST(testMarkWithFareType_Tag5Tag6);
  CPPUNIT_TEST(testMarkWithFareType_Tag7Tag7);
  CPPUNIT_TEST(testMarkWithFareType_Tag9Tag9);
  CPPUNIT_TEST(testMarkWithFareType_Tag9Tag10);
  CPPUNIT_TEST(testMarkWithFareType_Tag10Tag10);
  CPPUNIT_TEST(testMarkWithFareType_Tag10Tag11);
  CPPUNIT_TEST(testMarkWithFareType_Tag11Tag11);
  CPPUNIT_TEST(testMarkWithFareType_Tag1Tag2Tag3Tag4Tag5Tag6Tag9Tag10Tag11);
  CPPUNIT_TEST(testTagsNotSet);
  CPPUNIT_TEST(testZeroT988First);
  CPPUNIT_TEST(testZeroT988Between);
  CPPUNIT_TEST(testZeroT988Last);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testMarkWithFareType_Tag1Tag1();
  void testMarkWithFareType_Tag2Tag2();
  void testMarkWithFareType_Tag1Tag2();
  void testMarkWithFareType_Tag1Tag3();
  void testMarkWithFareType_Tag2Tag3();
  void testMarkWithFareType_Tag3Tag3();
  void testMarkWithFareType_Tag1Tag4();
  void testMarkWithFareType_Tag2Tag4();
  void testMarkWithFareType_Tag3Tag4();
  void testMarkWithFareType_Tag4Tag4();
  void testMarkWithFareType_Tag5Tag5();
  void testMarkWithFareType_Tag6Tag6_TravelCommenced();
  void testMarkWithFareType_Tag6Tag6_TravelNotCommenced();
  void testMarkWithFareType_Tag5Tag6();
  void testMarkWithFareType_Tag7Tag7();
  void testMarkWithFareType_Tag9Tag9();
  void testMarkWithFareType_Tag9Tag10();
  void testMarkWithFareType_Tag10Tag10();
  void testMarkWithFareType_Tag10Tag11();
  void testMarkWithFareType_Tag11Tag11();
  void testMarkWithFareType_Tag1Tag2Tag3Tag4Tag5Tag6Tag9Tag10Tag11();
  void testTagsNotSet();
  void testZeroT988First();
  void testZeroT988Between();
  void testZeroT988Last();

private:
  void AddProcessTag(ProcessTagPermutation& perm, int pt = 0);
  void ClearProcessTagPermutationData(ProcessTagPermutation& perm);
};

CPPUNIT_TEST_SUITE_REGISTRATION(TagWarEngineTest);

}
#endif
