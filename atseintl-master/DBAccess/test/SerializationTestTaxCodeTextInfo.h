//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_TAX_CODE_TEXT_INFO_H
#define SERIALIZATION_TEST_TAX_CODE_TEXT_INFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TaxCodeTextInfo.h"

namespace tse
{
class SerializationTestTaxCodeTextInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTaxCodeTextInfo);
  CPPUNIT_TEST(testInfoType<TaxCodeTextInfo>);
  CPPUNIT_TEST(testInfoVectorType<TaxCodeTextInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_TAX_CODE_TEXT_INFO_H
