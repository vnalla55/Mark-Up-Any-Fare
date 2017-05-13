//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_AIRLINECOUNTRYSETTLEMENTPLAN_H
#define SERIALIZATION_TEST_AIRLINECOUNTRYSETTLEMENTPLAN_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"

namespace tse
{
class SerializationTestAirlineCountrySettlementPlan : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestAirlineCountrySettlementPlan);
  CPPUNIT_TEST(testInfoType<AirlineCountrySettlementPlanInfo>);
  CPPUNIT_TEST(testInfoVectorType<AirlineCountrySettlementPlanInfo>);
  CPPUNIT_TEST_SUITE_END();
};
}

#endif
