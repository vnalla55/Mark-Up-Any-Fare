//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_FAREBYRULEAPP_H
#define SERIALIZATION_TEST_FAREBYRULEAPP_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/FareByRuleApp.h"

namespace tse
{
class SerializationTestFareByRuleApp : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestFareByRuleApp);
  CPPUNIT_TEST(testInfoType<FareByRuleApp>);
  CPPUNIT_TEST(testInfoVectorType<FareByRuleApp>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif
