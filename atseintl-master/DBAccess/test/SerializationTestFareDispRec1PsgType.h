//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_FAREDISPREC1PSGTYPE_H
#define SERIALIZATION_TEST_FAREDISPREC1PSGTYPE_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/FareDispRec1PsgType.h"

namespace tse
{
class SerializationTestFareDispRec1PsgType : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestFareDispRec1PsgType);
  CPPUNIT_TEST(testInfoType<FareDispRec1PsgType>);
  CPPUNIT_TEST(testInfoVectorType<FareDispRec1PsgType>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_FAREDISPREC1PSGTYPE_H
