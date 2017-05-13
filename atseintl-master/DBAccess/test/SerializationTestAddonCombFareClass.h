//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef SERIALIZATION_TEST_ADDONCOMBFARECLASS_H
#define SERIALIZATION_TEST_ADDONCOMBFARECLASS_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/AddonCombFareClassInfo.h"

namespace tse
{
class SerializationTestAddonCombFareClass : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAddonCombFareClass);
  CPPUNIT_TEST(testInfoTypeWithFlattenize<AddonCombFareClassInfo>);
  CPPUNIT_TEST(testInfoTypeWithFlattenize<AddonFareClassCombMultiMap>);
  CPPUNIT_TEST(testDataBlob);
  CPPUNIT_TEST_SUITE_END();

public:
  void testDataBlob();
};

} // namespace tse

#endif // SERIALIZATION_TEST_ADDONCOMBFARECLASS_H
