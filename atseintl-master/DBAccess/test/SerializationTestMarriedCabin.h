//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_MARRIEDCABIN_H
#define SERIALIZATION_TEST_MARRIEDCABIN_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/MarriedCabin.h"

namespace tse
{
class SerializationTestMarriedCabin : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestMarriedCabin);
  CPPUNIT_TEST(testInfoType<MarriedCabin>);
  CPPUNIT_TEST(testInfoVectorType<MarriedCabin>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_MARRIEDCABIN_H
