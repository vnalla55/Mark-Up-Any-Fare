//-------------------------------------------------------------------
//
//  File:        FareDisplayTypeQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 13, 2016
//  Description: This class does unit tests of the FareDisplayTypeQualifier,
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
#include "FareDisplay/FareDisplayTypeQualifier.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class FareDisplayTypeQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayTypeQualifierTest);
  CPPUNIT_TEST(testQualifyFareDisplayType);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFareDisplayType()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareDisplayTypeQualifier fdtq;
    FareClassAppInfo* _fca = _memHandle.create<FareClassAppInfo>();
    fdTrx.setOptions(&options);

    {
      options.frrDisplayType() = 'C';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      _fca->_displayCatType = 'C';
      ptFare.fareClassAppInfo() = _fca;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fdtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrDisplayType() = ' ';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      _fca->_displayCatType = 'C';
      ptFare.fareClassAppInfo() = _fca;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fdtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }

    {
      options.frrDisplayType() = 'C';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      _fca->_displayCatType = 'D';
      ptFare.fareClassAppInfo() = _fca;
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fdtq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Display_Type);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayTypeQualifierTest);
}
