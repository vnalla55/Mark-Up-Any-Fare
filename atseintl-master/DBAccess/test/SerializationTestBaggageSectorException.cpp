//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/BaggageSectorException.h"

namespace tse
{
class SerializationTestBaggageSectorException : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestBaggageSectorException);
  CPPUNIT_TEST(testInfoType<BaggageSectorException>);
  CPPUNIT_TEST(testInfoVectorType<BaggageSectorException>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestBaggageSectorException);

} // namespace tse
