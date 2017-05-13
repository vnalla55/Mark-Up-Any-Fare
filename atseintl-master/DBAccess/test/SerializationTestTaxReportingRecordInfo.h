//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TAX_REPORTING_RECORD_INFO_H
#define SERIALIZATION_TEST_TAX_REPORTING_RECORD_INFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TaxReportingRecordInfo.h"

namespace tse
{
class SerializationTestTaxReportingRecordInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTaxReportingRecordInfo);
  CPPUNIT_TEST(testInfoType<TaxReportingRecordInfo>);
  CPPUNIT_TEST(testInfoVectorType<TaxReportingRecordInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TAX_REPORTING_RECORD_INFO_H
