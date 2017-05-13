//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_SVCFEESFAREIDINFO_H
#define SERIALIZATION_TEST_SVCFEESFAREIDINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SvcFeesFareIdInfo.h"

namespace tse
{
class SerializationTestSvcFeesFareIdInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSvcFeesFareIdInfo);
  CPPUNIT_TEST(testInfoType<SvcFeesFareIdInfo>);
  CPPUNIT_TEST(testInfoVectorType<SvcFeesFareIdInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_SVCFEESFAREIDINFO_H
