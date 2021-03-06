//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/RoundTripRuleItem.h"

namespace tse
{
class SerializationTestRoundTripRuleItem : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestRoundTripRuleItem);
  CPPUNIT_TEST(testInfoType<RoundTripRuleItem>);
  CPPUNIT_TEST(testInfoVectorType<RoundTripRuleItem>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestRoundTripRuleItem);
} // namespace tse
