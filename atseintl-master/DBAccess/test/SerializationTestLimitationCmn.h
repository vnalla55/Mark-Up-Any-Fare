//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_LIMITATIONCMN_H
#define SERIALIZATION_TEST_LIMITATIONCMN_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/LimitationCmn.h"

namespace tse
{
class SerializationTestLimitationCmn : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestLimitationCmn);
  CPPUNIT_TEST(testInfoType<LimitationCmn>);
  CPPUNIT_TEST(testInfoVectorType<LimitationCmn>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_LIMITATIONCMN_H
