//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include "DBAccess/test/SerializationTestAddonCombFareContainer.h"

static const size_t NUM_OBJECTS = 10;
static const size_t NUM_ITEMS = 3000;
static const size_t POOL_SIZE = 5;

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestAddonCombFareContainer);

namespace tse
{

const std::string&
SerializationTestAddonCombFareContainer::containerDescription()
{
  static const std::string s("AddonFareClassCombMultiMap");
  return s;
}

void
SerializationTestAddonCombFareContainer::fareContainerTest()
{
  static size_t num_objects(NUM_OBJECTS);
  static size_t num_items(NUM_ITEMS);
  static size_t pool_size(POOL_SIZE);
  static char compare_with_boost(' ');

  static bool input_read(false);

  if (!input_read)
  {
    std::ifstream input("./input.txt");
    if (input.is_open())
    {
      input >> num_objects >> num_items >> pool_size >> compare_with_boost;
      input.close();
    }
    input_read = true;
  }

  testContainer<AddonFareClassCombMultiMap, SerializationTestAddonCombFareContainer>(
      num_objects, num_items, pool_size);
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
SerializationTestAddonCombFareContainer::insertNewItems(AddonFareClassCombMultiMap& container,
                                                        size_t items_per)
{
  while (items_per--)
  {
    AddonCombFareClassInfo* info = new AddonCombFareClassInfo();
    AddonCombFareClassInfo::dummyData(*info);
    AddonCombFareClassSpecifiedKey key(info->fareClass(), info->owrt());
    std::vector<AddonCombFareClassInfo*> combs(1, info);
    container.emplace(key, std::move(combs));
  }
}

void
SerializationTestAddonCombFareContainer::clearContainer(AddonFareClassCombMultiMap& container)
{
  for (AddonFareClassCombMultiMap::iterator it = container.begin(); it != container.end(); ++it)
  {
    for (auto c: it->second)
      delete c;
  }
  container.clear();
}

#else

void
SerializationTestAddonCombFareContainer::insertNewItems(AddonFareClassCombMultiMap& container,
                                                        size_t items_per)
{
  while (items_per--)
  {
    AddonCombFareClassInfo* info = new AddonCombFareClassInfo();
    AddonCombFareClassInfo::dummyData(*info);
    AddonCombFareClassInfoKey key(
        info->addonFareClass(), info->geoAppl(), info->owrt(), info->fareClass());
    container.insert(AddonFareClassCombMultiMap::value_type(key, info));
  }
}

void
SerializationTestAddonCombFareContainer::clearContainer(AddonFareClassCombMultiMap& container)
{
  for (AddonFareClassCombMultiMap::iterator it = container.begin(); it != container.end(); ++it)
  {
    delete (*it).second;
  }
  container.clear();
}

#endif

}
