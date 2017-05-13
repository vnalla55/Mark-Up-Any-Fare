//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_NEUTRALVALIDATINGAIRLINEINFO_H
#define SERIALIZATION_TEST_NEUTRALVALIDATINGAIRLINEINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"

namespace tse
{
class SerializationTestNeutralValidatingAirlineInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestNeutralValidatingAirlineInfo);
  CPPUNIT_TEST(testInfoType<NeutralValidatingAirlineInfo>);
  CPPUNIT_TEST(testInfoVectorType<NeutralValidatingAirlineInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif
