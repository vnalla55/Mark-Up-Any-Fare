//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"

#include "DBAccess/ValueCodeAlgorithm.h"

namespace tse
{
class SerializationTestValueCodeAlgorithm : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestValueCodeAlgorithm);
  CPPUNIT_TEST(testInfoType<ValueCodeAlgorithm>);
  CPPUNIT_TEST(testInfoVectorType<ValueCodeAlgorithm>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestValueCodeAlgorithm);
} // namespace tse
