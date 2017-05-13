//-------------------------------------------------------------------
//
//  File:        FareClassAppUnavailTagQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 9, 2006
//  Description: This class does unit tests of the FareClassAppUnavailTagQualifier,
//               class.
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//--------------------------------------------------------------------
#include "Common/Logger.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/FareClassAppUnavailTagQualifier.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <memory>

#include <log4cxx/propertyconfigurator.h>

using namespace std;
namespace tse
{
class FareClassAppUnavailTagQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareClassAppUnavailTagQualifierTest);
  CPPUNIT_TEST(testQualifyFareClassAppUnavailTag);
  CPPUNIT_TEST_SUITE_END();

  static log4cxx::LoggerPtr _logger;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareClassAppUnavailTag()
  {
    FareDisplayTrx fdTrx;
    FareDisplayRequest request;
    PaxTypeFare ptFare;
    FareClassAppInfo fcaInfo;
    FareClassAppUnavailTagQualifier fcq;

    fdTrx.setRequest(&request);
    ptFare.fareClassAppInfo() = &fcaInfo;

    {
      request.requestedInclusionCode() = ASEAN_ADDON_FARES;
      const tse::PaxTypeFare::FareDisplayState ret = fcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      request.requestedInclusionCode() = ALL_INCLUSION_CODE;
      fcaInfo._unavailTag = 'Z'; // FARE_NOT_TO_BE_DISPLAYED;
      const tse::PaxTypeFare::FareDisplayState ret = fcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_FareClassApp_Unavailable);
    }
    {
      request.requestedInclusionCode() = ALL_INCLUSION_CODE;
      fcaInfo._unavailTag = 'Z' - 1; // FARE_NOT_TO_BE_DISPLAYED -1
      const tse::PaxTypeFare::FareDisplayState ret = fcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareClassAppUnavailTagQualifierTest);

log4cxx::LoggerPtr
FareClassAppUnavailTagQualifierTest::_logger(
    log4cxx::Logger::getLogger("atseintl.FareDisplay.test.FareClassAppUnavailTagQualifierTest"));
}
