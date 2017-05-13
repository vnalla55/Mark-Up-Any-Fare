//-------------------------------------------------------------------
//
//  File:        FareCatSelectQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 15, 2016
//  Description: This class does unit tests of the FareCatSelectQualifier,
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
#include "FareDisplay/FareCatSelectQualifier.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class FareCatSelectQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCatSelectQualifierTest);
  CPPUNIT_TEST(testQualifyFareCatSelect);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareCatSelect()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareCatSelectQualifier fcsq;

    fdTrx.setOptions(&options);

    {
      options.frrSelectCat25Fares() = true;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.status().set(PaxTypeFare::PTF_FareByRule);
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fcsq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrSelectCat35Fares() = true;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.status().set(PaxTypeFare::PTF_Negotiated);
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fcsq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrSelectCat15Fares() = true;
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.status().set(PaxTypeFare::PTF_Negotiated);
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fcsq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Sales_Restriction_Select);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCatSelectQualifierTest);
}
