//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_FARE_CONTAINER_H
#define SERIALIZATION_TEST_FARE_CONTAINER_H

#include "DBAccess/test/SerializationTestBase.h"

namespace tse
{
class SerializationTestFareContainer : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestFareContainer);
  CPPUNIT_TEST(fareContainerTest);
  CPPUNIT_TEST(mixedTypeTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void fareContainerTest();
  void mixedTypeTest();

  static void insertNewItems(std::vector<const FareInfo*>& container, size_t items_per);
  static void clearContainer(std::vector<const FareInfo*>& container);
  static const std::string& containerDescription();
};

} // namespace tse

#endif // SERIALIZATION_TEST_FARE_CONTAINER_H
