//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_INDUSTRYPRICINGAPPL_H
#define SERIALIZATION_TEST_INDUSTRYPRICINGAPPL_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/IndustryPricingAppl.h"

namespace tse
{
class SerializationTestIndustryPricingAppl : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestIndustryPricingAppl);
  CPPUNIT_TEST(testInfoType<IndustryPricingAppl>);
  CPPUNIT_TEST(testInfoVectorType<IndustryPricingAppl>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_INDUSTRYPRICINGAPPL_H
