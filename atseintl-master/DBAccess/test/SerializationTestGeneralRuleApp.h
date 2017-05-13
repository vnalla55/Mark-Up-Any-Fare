//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_GENERALRULEAPP_H
#define SERIALIZATION_TEST_GENERALRULEAPP_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/GeneralRuleApp.h"

namespace tse
{
class SerializationTestGeneralRuleApp : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestGeneralRuleApp);
  CPPUNIT_TEST(testInfoType<GeneralRuleApp>);
  CPPUNIT_TEST(testInfoVectorType<GeneralRuleApp>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_GENERALRULEAPP_H
