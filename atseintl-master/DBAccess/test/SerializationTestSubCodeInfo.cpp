//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SubCodeInfo.h"

namespace tse
{
class SerializationTestSubCodeInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSubCodeInfo);
  CPPUNIT_TEST(testInfoType<SubCodeInfo>);
  CPPUNIT_TEST(testInfoVectorType<SubCodeInfo>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestSubCodeInfo);
} // namespace tse
