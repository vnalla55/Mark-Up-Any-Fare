//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "DBAccess/test/SerializationTestAddonFareContainer.h"
#include "DBAccess/AddonFareInfoFactory.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/DiskCache.h"

static const size_t NUM_OBJECTS = 10;
static const size_t NUM_ITEMS = 3000;
static const size_t POOL_SIZE = 5;

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestAddonFareContainer);

namespace tse
{
void
SerializationTestAddonFareContainer::insertNewItems(std::vector<AddonFareInfo*>& container,
                                                    size_t items_per)
{
  while (items_per--)
  {
    AddonFareInfo* info = new AddonFareInfo;
    AddonFareInfo::dummyData(*info);
    container.push_back(info);
  }
}

void
SerializationTestAddonFareContainer::clearContainer(std::vector<AddonFareInfo*>& container)
{
  for (std::vector<AddonFareInfo*>::iterator it = container.begin(); it != container.end(); ++it)
  {
    delete (*it);
  }
  container.clear();
}

const std::string&
SerializationTestAddonFareContainer::containerDescription()
{
  static const std::string s("std::vector<AddonFareInfo*>");
  return s;
}

void
SerializationTestAddonFareContainer::addonFareContainerTest()
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

  testContainer<std::vector<AddonFareInfo*>, SerializationTestAddonFareContainer>(
      num_objects, num_items, pool_size);
}

void
SerializationTestAddonFareContainer::mixedTypeTest()
{
  // std::cout << std::endl ;
  AddonFareInfo* fi(0);
  size_t limit(30);
  size_t i(0);
  Flattenizable::Archive archive;
  std::vector<AddonFareInfo*> original;
  std::vector<AddonFareInfo*> reassembled;

  for (i = 0; i < limit; ++i)
  {
    if (((i + 1) % 3) == 0)
    {
      fi = AddonFareInfoFactory::create(eAddonFareInfo);
    }
    else if (((i + 1) % 3) == 1)
    {
      fi = AddonFareInfoFactory::create(eSITAAddonFareInfo);
    }
    else
    {
      fi = AddonFareInfoFactory::create(eFDAddonFareInfo);
    }
    original.push_back(fi);
  }

  std::string ignore;
  FLATTENIZE_SAVE(archive, original, 0, ignore, ignore);

  FLATTENIZE_RESTORE(archive, reassembled, NULL, 0);

  CPPUNIT_ASSERT(tse::objectsIdentical(original, reassembled));

  for (i = 0; i < limit; ++i)
  {
    AddonFareInfo* basePtr = reassembled[i];
    SITAAddonFareInfo* sitaPtr = dynamic_cast<SITAAddonFareInfo*>(basePtr);
    FDAddOnFareInfo* fdPtr = dynamic_cast<FDAddOnFareInfo*>(basePtr);

    if (((i + 1) % 3) == 0)
    {
      CPPUNIT_ASSERT(basePtr->objectType() == eAddonFareInfo);
      CPPUNIT_ASSERT(sitaPtr == 0);
      CPPUNIT_ASSERT(fdPtr == 0);
    }
    else if (((i + 1) % 3) == 1)
    {
      CPPUNIT_ASSERT(basePtr->objectType() == eSITAAddonFareInfo);
      CPPUNIT_ASSERT(sitaPtr != 0);
      CPPUNIT_ASSERT(fdPtr == 0);
    }
    else
    {
      CPPUNIT_ASSERT(basePtr->objectType() == eFDAddonFareInfo);
      CPPUNIT_ASSERT(sitaPtr != 0);
      CPPUNIT_ASSERT(fdPtr != 0);
    }
  }

  clearContainer(original);
  clearContainer(reassembled);
}
}
