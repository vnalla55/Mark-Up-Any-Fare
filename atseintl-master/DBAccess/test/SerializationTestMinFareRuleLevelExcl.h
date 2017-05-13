//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_MINFARERULELEVELEXCL_H
#define SERIALIZATION_TEST_MINFARERULELEVELEXCL_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/MinFareRuleLevelExcl.h"

namespace tse
{
class SerializationTestMinFareRuleLevelExcl : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestMinFareRuleLevelExcl);
  CPPUNIT_TEST(testInfoType<MinFareRuleLevelExcl>);
  CPPUNIT_TEST(testInfoVectorType<MinFareRuleLevelExcl>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_MINFARERULELEVELEXCL_H
