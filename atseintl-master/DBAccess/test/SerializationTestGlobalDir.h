//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_GLOBALDIR_H
#define SERIALIZATION_TEST_GLOBALDIR_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/GlobalDir.h"

namespace tse
{
class SerializationTestGlobalDir : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestGlobalDir);
  CPPUNIT_TEST(testInfoType<GlobalDir>);
  CPPUNIT_TEST(testInfoVectorType<GlobalDir>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_GLOBALDIR_H
