//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_ADDON_COMB_FARE_CONTAINER_H
#define SERIALIZATION_TEST_ADDON_COMBFARE_CONTAINER_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/AddonCombFareClassInfo.h"

namespace tse
{
class SerializationTestAddonCombFareContainer : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAddonCombFareContainer);
  CPPUNIT_TEST(fareContainerTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void fareContainerTest();

  static void insertNewItems(AddonFareClassCombMultiMap& container, size_t items_per);
  static void clearContainer(AddonFareClassCombMultiMap& container);

  static const std::string& containerDescription();
};

} // namespace tse

#endif // SERIALIZATION_TEST_ADDON_COMB_FARE_CONTAINER_H
