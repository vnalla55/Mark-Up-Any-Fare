//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TRANSFERSINFO1_H
#define SERIALIZATION_TEST_TRANSFERSINFO1_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TransfersInfo1.h"

namespace tse
{
class SerializationTestTransfersInfo1 : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTransfersInfo1);
  CPPUNIT_TEST(testInfoType<TransfersInfo1>);
  CPPUNIT_TEST(testInfoVectorType<TransfersInfo1>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TRANSFERSINFO1_H
