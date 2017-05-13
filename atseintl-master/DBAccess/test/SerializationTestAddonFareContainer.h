//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_ADDONFARE_CONTAINER_H
#define SERIALIZATION_TEST_ADDONFARE_CONTAINER_H

#include "DBAccess/test/SerializationTestBase.h"

namespace tse
{
class SerializationTestAddonFareContainer : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAddonFareContainer);
  CPPUNIT_TEST(addonFareContainerTest);
  CPPUNIT_TEST(mixedTypeTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void addonFareContainerTest();
  void mixedTypeTest();

  static void insertNewItems(std::vector<AddonFareInfo*>& container, size_t items_per);
  static void clearContainer(std::vector<AddonFareInfo*>& container);
  static const std::string& containerDescription();
};

} // namespace tse

#endif // SERIALIZATION_TEST_ADDONFARE_CONTAINER_H
