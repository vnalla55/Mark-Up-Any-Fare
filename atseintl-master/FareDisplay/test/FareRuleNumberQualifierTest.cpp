//-------------------------------------------------------------------
//
//  File:        FareRuleNumberQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 12, 2016
//  Description: This class does unit tests of the FareRuleNumberQualifier,
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
#include "FareDisplay/FareRuleNumberQualifier.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class FareRuleNumberQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareRuleNumberQualifierTest);
  CPPUNIT_TEST(testQualifyFareRuleNumber);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareRuleNumber()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareRuleNumberQualifier frtq;

    fdTrx.setOptions(&options);

    {
      options.frrRuleNumber() = "AB";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<RuleNumber&>(ptFare.ruleNumber()) = "AB";
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrRuleNumber() = "";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<RuleNumber&>(ptFare.ruleNumber()) = "AB";
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrRuleNumber() = "BC";
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const_cast<RuleNumber&>(ptFare.ruleNumber()) = "BD";
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Rule_Number);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareRuleNumberQualifierTest);
}
