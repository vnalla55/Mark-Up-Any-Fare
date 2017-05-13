//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_COPCARRIER_H
#define SERIALIZATION_TEST_COPCARRIER_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/CopCarrier.h"

namespace tse
{
class SerializationTestCopCarrier : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestCopCarrier);
  CPPUNIT_TEST(testInfoType<CopCarrier>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_COPCARRIER_H
