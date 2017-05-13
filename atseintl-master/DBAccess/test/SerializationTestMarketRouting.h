//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_MARKETROUTING_H
#define SERIALIZATION_TEST_MARKETROUTING_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/MarketRoutingInfo.h"

namespace tse
{
class SerializationTestMarketRouting : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestMarketRouting);
  CPPUNIT_TEST(testInfoType<MarketRouting>);
  CPPUNIT_TEST(testInfoVectorType<MarketRouting>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_MARKETROUTING_H
