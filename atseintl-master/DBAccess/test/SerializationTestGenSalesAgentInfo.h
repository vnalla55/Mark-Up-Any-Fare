//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#ifndef SERIALIATION_TEST_GENSALESAGENTINFO_H
#define SERIALIATION_TEST_GENSALESAGENTINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/GenSalesAgentInfo.h"

namespace tse
{
class SerializationTestGenSalesAgentInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestGenSalesAgentInfo);
  CPPUNIT_TEST(testInfoVectorType<GenSalesAgentInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}
#endif
