//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"

namespace tse
{
class SerializationTestSeatCabinCharacteristicInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSeatCabinCharacteristicInfo);
  CPPUNIT_TEST(testInfoType<SeatCabinCharacteristicInfo>);
  CPPUNIT_TEST(testInfoVectorType<SeatCabinCharacteristicInfo>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestSeatCabinCharacteristicInfo);
} // namespace tse
