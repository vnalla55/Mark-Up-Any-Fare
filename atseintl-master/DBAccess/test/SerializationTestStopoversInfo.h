//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_STOPOVERSINFO_H
#define SERIALIZATION_TEST_STOPOVERSINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/StopoversInfo.h"

namespace tse
{
class SerializationTestStopoversInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestStopoversInfo);
  CPPUNIT_TEST(testInfoType<StopoversInfo>);
  CPPUNIT_TEST(testInfoVectorType<StopoversInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_STOPOVERSINFO_H
