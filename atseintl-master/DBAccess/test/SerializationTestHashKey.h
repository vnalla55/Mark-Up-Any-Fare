//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_HASHKEY_H
#define SERIALIZATION_TEST_HASHKEY_H

#include "DBAccess/test/SerializationTestBase.h"

namespace tse
{
class SerializationTestHashKey : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestHashKey);
  CPPUNIT_TEST(testObjectToHash);
  CPPUNIT_TEST(testToAndFromString);
  CPPUNIT_TEST_SUITE_END();

public:
  void testObjectToHash();
  void testToAndFromString();
};

} // namespace tse

#endif // SERIALIZATION_TEST_HASHKEY_H
