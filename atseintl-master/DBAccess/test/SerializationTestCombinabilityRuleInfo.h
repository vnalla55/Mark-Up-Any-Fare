//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_COMBINABILITYRULEINFO_H
#define SERIALIZATION_TEST_COMBINABILITYRULEINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/CombinabilityRuleInfo.h"

namespace tse
{
class SerializationTestCombinabilityRuleInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestCombinabilityRuleInfo);
  CPPUNIT_TEST(testInfoType<CombinabilityRuleInfo>);
  CPPUNIT_TEST(testInfoVectorType<CombinabilityRuleInfo>);
  CPPUNIT_TEST(testDummyDataValues);
  CPPUNIT_TEST_SUITE_END();

  void testDummyDataValues();

private:
  void testDummyData(const CombinabilityRuleInfo& obj);
};

} // namespace tse

#endif // SERIALIZATION_TEST_COMBINABILITYRULEINFO_H
