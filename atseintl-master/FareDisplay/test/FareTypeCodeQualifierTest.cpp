//-------------------------------------------------------------------
//
//  File:        FareTypeCodeQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 5, 2016
//  Description: This class does unit tests of the FareTypeCodeQualifier,
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

#include <memory>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "FareDisplay/FareTypeCodeQualifier.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class FareTypeCodeQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareTypeCodeQualifierTest);
  CPPUNIT_TEST(testQualifyFareTypeCode);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareTypeCode()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareTypeCodeQualifier ftcq;

    fdTrx.setOptions(&options);

    {
      options.frrFareTypeCode() = "EU";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<FareType&>(ptFare.fcaFareType()) = "EU";
      const tse::PaxTypeFare::FareDisplayState ret = ftcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrFareTypeCode() = "";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<FareType&>(ptFare.fcaFareType()) = "EU";
      const tse::PaxTypeFare::FareDisplayState ret = ftcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrFareTypeCode() = "EU";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<FareType&>(ptFare.fcaFareType()) = "WU";
      const tse::PaxTypeFare::FareDisplayState ret = ftcq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Fare_Type_Code);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareTypeCodeQualifierTest);
}
