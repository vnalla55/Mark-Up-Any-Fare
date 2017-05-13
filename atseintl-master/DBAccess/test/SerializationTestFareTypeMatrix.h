//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_FARETYPEMATRIX_H
#define SERIALIZATION_TEST_FARETYPEMATRIX_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/FareTypeMatrix.h"

namespace tse
{
class SerializationTestFareTypeMatrix : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestFareTypeMatrix);
  CPPUNIT_TEST(testInfoType<FareTypeMatrix>);
  CPPUNIT_TEST(testInfoVectorType<FareTypeMatrix>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_FARETYPEMATRIX_H
