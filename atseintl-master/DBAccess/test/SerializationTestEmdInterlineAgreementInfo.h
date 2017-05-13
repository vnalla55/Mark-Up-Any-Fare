//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_EMDINTERLINEAGREEMENT_H
#define SERIALIZATION_TEST_EMDINTERLINEAGREEMENT_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"

namespace tse
{
class SerializationTestEmdInterlineAgreementInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestEmdInterlineAgreementInfo);
  CPPUNIT_TEST(testInfoType<EmdInterlineAgreementInfo>);
  CPPUNIT_TEST(testInfoVectorType<EmdInterlineAgreementInfo>);
  CPPUNIT_TEST_SUITE_END();
};
} // tse

#endif // SERIALIZATION_TEST_EMDINTERLINEAGREEMENT_H
