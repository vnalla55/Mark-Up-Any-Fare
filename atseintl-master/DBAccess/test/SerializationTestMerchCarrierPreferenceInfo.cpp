//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/MerchCarrierPreferenceInfo.h"

namespace tse
{
class SerializationTestMerchCarrierPreferenceInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestMerchCarrierPreferenceInfo);
  CPPUNIT_TEST(testInfoType<MerchCarrierPreferenceInfo>);
  CPPUNIT_TEST(testInfoVectorType<MerchCarrierPreferenceInfo>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestMerchCarrierPreferenceInfo);
} // namespace tse
