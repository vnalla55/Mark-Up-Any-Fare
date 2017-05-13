//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_SECTOR_DETAIL_INFO_H
#define SERIALIZATION_TEST_SECTOR_DETAIL_INFO_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/SectorDetailInfo.h"

namespace tse
{
class SerializationTestSectorDetailInfo : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestSectorDetailInfo);
  CPPUNIT_TEST(testInfoType<SectorDetailInfo>);
  CPPUNIT_TEST(testInfoVectorType<SectorDetailInfo>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_SECTOR_DETAIL_INFO_H
