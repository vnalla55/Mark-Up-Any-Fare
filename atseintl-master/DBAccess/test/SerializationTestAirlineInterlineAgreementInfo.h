//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_AIRLINEINTERLINEAGREEMENT_H
#define SERIALIZATION_TEST_AIRLINEINTERLINEAGREEMENT_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"

namespace tse
{
class SerializationTestAirlineInterlineAgreementInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAirlineInterlineAgreementInfo);
  CPPUNIT_TEST(testInfoType<AirlineInterlineAgreementInfo>);
  CPPUNIT_TEST(testInfoVectorType<AirlineInterlineAgreementInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif
