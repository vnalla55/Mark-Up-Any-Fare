//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_CURRENCY_H
#define SERIALIZATION_TEST_CURRENCY_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/Currency.h"

namespace tse
{
class SerializationTestCurrency : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestCurrency);
  CPPUNIT_TEST(testInfoType<Currency>);
  CPPUNIT_TEST(testInfoVectorType<Currency>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_CURRENCY_H
