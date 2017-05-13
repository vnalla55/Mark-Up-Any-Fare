//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TSIINFO_H
#define SERIALIZATION_TEST_TSIINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TSIInfo.h"

namespace tse
{
class SerializationTestTSIInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTSIInfo);
  CPPUNIT_TEST(testInfoType<TSIInfo>);
  CPPUNIT_TEST(testInfoVectorType<TSIInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TSIINFO_H
