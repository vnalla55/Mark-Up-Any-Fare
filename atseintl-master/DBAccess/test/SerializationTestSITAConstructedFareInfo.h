//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_SITACONSTRUCTEDFAREINFO_H
#define SERIALIZATION_TEST_SITACONSTRUCTEDFAREINFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SITAConstructedFareInfo.h"

namespace tse
{
class SerializationTestSITAConstructedFareInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSITAConstructedFareInfo);
  CPPUNIT_TEST(testInfoType<SITAConstructedFareInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_SITACONSTRUCTEDFAREINFO_H
