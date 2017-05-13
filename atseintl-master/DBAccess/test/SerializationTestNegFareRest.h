//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_NEGFAREREST_H
#define SERIALIZATION_TEST_NEGFAREREST_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/NegFareRest.h"

namespace tse
{
class SerializationTestNegFareRest : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestNegFareRest);
  CPPUNIT_TEST(testInfoType<NegFareRest>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_NEGFAREREST_H
