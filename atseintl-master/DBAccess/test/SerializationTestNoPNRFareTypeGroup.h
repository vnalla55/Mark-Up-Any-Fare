//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_NOPNRFARETYPEGROUP_H
#define SERIALIZATION_TEST_NOPNRFARETYPEGROUP_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/NoPNRFareTypeGroup.h"

namespace tse
{
class SerializationTestNoPNRFareTypeGroup : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestNoPNRFareTypeGroup);
  CPPUNIT_TEST(testInfoType<NoPNRFareTypeGroup>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_NOPNRFARETYPEGROUP_H
