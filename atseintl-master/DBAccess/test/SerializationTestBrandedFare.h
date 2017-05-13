//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_BRANDEDFARE_H
#define SERIALIZATION_TEST_BRANDEDFARE_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/BrandedFare.h"

namespace tse
{
class SerializationTestBrandedFare : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestBrandedFare);
  CPPUNIT_TEST(testInfoType<BrandedFare>);
  CPPUNIT_TEST(testInfoVectorType<BrandedFare>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_BRANDEDFARE_H
