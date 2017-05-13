//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_STATE_H
#define SERIALIZATION_TEST_STATE_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/State.h"

namespace tse
{
class SerializationTestState : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestState);
  CPPUNIT_TEST(testInfoType<State>);
  CPPUNIT_TEST(testInfoVectorType<State>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_STATE_H
