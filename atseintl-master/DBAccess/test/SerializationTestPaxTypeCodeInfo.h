//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_PAX_TYPE_CODE_INFO_H
#define SERIALIZATION_TEST_PAX_TYPE_CODE_INFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/PaxTypeCodeInfo.h"

namespace tse
{
class SerializationTestPaxTypeCodeInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestPaxTypeCodeInfo);
  CPPUNIT_TEST(testInfoType<PaxTypeCodeInfo>);
  CPPUNIT_TEST(testInfoVectorType<PaxTypeCodeInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_PAX_TYPE_CODE_INFO_H
