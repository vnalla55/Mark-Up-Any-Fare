//-------------------------------------------------------------------
//
//  File:        FareRuleTariffQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 8, 2016
//  Description: This class does unit tests of the FareRuleTariffQualifier,
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
#include "FareDisplay/FareRuleTariffQualifier.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class FareRuleTariffQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareRuleTariffQualifierTest);
  CPPUNIT_TEST(testQualifyFareRuleTariff);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareRuleTariff()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareRuleTariffQualifier frtq;

    fdTrx.setOptions(&options);

    {
      options.frrTariffNumber() = 20;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      tcrInfo.ruleTariff() = 20;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrTariffNumber() = 0;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      tcrInfo.ruleTariff() = 30;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrTariffNumber() = 20;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      tcrInfo.ruleTariff() = 30;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = frtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Rule_Tariff);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareRuleTariffQualifierTest);
}
