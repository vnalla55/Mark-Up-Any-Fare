//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/OptionalServicesInfo.h"

namespace tse
{
class SerializationTestOptionalServicesInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestOptionalServicesInfo);
  CPPUNIT_TEST(testInfoType<OptionalServicesInfo>);
  CPPUNIT_TEST(testInfoVectorType<OptionalServicesInfo>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestOptionalServicesInfo);
} // namespace tse
