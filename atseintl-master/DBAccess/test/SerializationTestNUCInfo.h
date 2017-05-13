//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_NUCINFO_H
#define SERIALIZATION_TEST_NUCINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/NUCInfo.h"

namespace tse
{
class SerializationTestNUCInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestNUCInfo);
  CPPUNIT_TEST(testInfoType<NUCInfo>);
  CPPUNIT_TEST(testInfoVectorType<NUCInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_NUCINFO_H
