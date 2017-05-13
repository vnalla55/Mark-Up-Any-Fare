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
#include "DBAccess/test/SerializationTestFareContainer.h"
#include "DBAccess/FareInfoFactory.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/DiskCache.h"

static const size_t NUM_OBJECTS = 10;
static const size_t NUM_ITEMS = 3000;
static const size_t POOL_SIZE = 5;

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestFareContainer);

namespace tse
{
void
SerializationTestFareContainer::insertNewItems(std::vector<const FareInfo*>& container,
                                               size_t items_per)
{
  while (items_per--)
  {
    FareInfo* info = new FareInfo;
    FareInfo::dummyData(*info);
    container.push_back(info);
  }
}

void
SerializationTestFareContainer::clearContainer(std::vector<const FareInfo*>& container)
{
  for (std::vector<const FareInfo*>::iterator it = container.begin(); it != container.end(); ++it)
  {
    delete (*it);
  }
  container.clear();
}

const std::string&
SerializationTestFareContainer::containerDescription()
{
  static const std::string s("std::vector<const FareInfo*>");
  return s;
}

void
SerializationTestFareContainer::fareContainerTest()
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

  testContainer<std::vector<const FareInfo*>, SerializationTestFareContainer>(
      num_objects, num_items, pool_size);
}

void
SerializationTestFareContainer::mixedTypeTest()
{
  // std::cout << std::endl ;
  FareInfo* fi(0);
  size_t limit(20);
  size_t i(0);
  Flattenizable::Archive archive;
  std::vector<const FareInfo*> original;
  std::vector<const FareInfo*> reassembled;

  for (i = 0; i < limit; ++i)
  {
    if (((i + 1) % 2) == 0)
    {
      fi = FareInfoFactory::create(eFareInfo);
    }
    else
    {
      fi = FareInfoFactory::create(eSITAFareInfo);
    }
    original.push_back(fi);
  }

  std::string ignore;
  FLATTENIZE_SAVE(archive, original, 0, ignore, ignore);

  FLATTENIZE_RESTORE(archive, reassembled, NULL, 0);

  CPPUNIT_ASSERT(tse::objectsIdentical(original, reassembled));

  for (i = 0; i < limit; ++i)
  {
    const FareInfo* basePtr = reassembled[i];
    const SITAFareInfo* sitaPtr = dynamic_cast<const SITAFareInfo*>(basePtr);

    if (((i + 1) % 2) == 0)
    {
      CPPUNIT_ASSERT(basePtr->objectType() == eFareInfo);
      CPPUNIT_ASSERT(sitaPtr == 0);
    }
    else
    {
      CPPUNIT_ASSERT(basePtr->objectType() == eSITAFareInfo);
      CPPUNIT_ASSERT(sitaPtr != 0);
    }
  }

  clearContainer(original);
  clearContainer(reassembled);
}
}
