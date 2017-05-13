//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_PASSENGERAIRLINE_H
#define SERIALIZATION_TEST_PASSENGERAIRLINE_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/PassengerAirlineInfo.h"

namespace tse
{
class SerializationTestPassengerAirlineInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestPassengerAirlineInfo);
  CPPUNIT_TEST(testInfoType<PassengerAirlineInfo>);
  CPPUNIT_TEST(testInfoVectorType<PassengerAirlineInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif
