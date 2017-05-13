//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TPDPSR_H
#define SERIALIZATION_TEST_TPDPSR_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TpdPsr.h"

namespace tse
{
class SerializationTestTpdPsr : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTpdPsr);
  CPPUNIT_TEST(testInfoType<TpdPsr>);
  CPPUNIT_TEST(testInfoVectorType<TpdPsr>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TPDPSR_H
