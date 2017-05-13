//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/OptionalServicesSeg.h"

namespace tse
{
class SerializationTestOptionalServicesSeg : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestOptionalServicesSeg);
  CPPUNIT_TEST(testInfoType<OptionalServicesSeg>);
  CPPUNIT_TEST(testInfoVectorType<OptionalServicesSeg>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestOptionalServicesSeg);
} // namespace tse
