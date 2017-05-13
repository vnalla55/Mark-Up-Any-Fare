//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/BaggageSectorCarrierApp.h"

namespace tse
{
class SerializationTestBaggageSectorCarrierApp : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestBaggageSectorCarrierApp);
  CPPUNIT_TEST(testInfoType<BaggageSectorCarrierApp>);
  CPPUNIT_TEST(testInfoVectorType<BaggageSectorCarrierApp>);
  CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationTestBaggageSectorCarrierApp);

} // namespace tse
