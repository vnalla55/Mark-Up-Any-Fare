//-------------------------------------------------------------------------------
// Copyright 2016, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma pack

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/TaxExemption.h"

namespace tse
{
class SerializationTestTaxExemption : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestTaxExemption);
  CPPUNIT_TEST(testInfoType<TaxExemption>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

