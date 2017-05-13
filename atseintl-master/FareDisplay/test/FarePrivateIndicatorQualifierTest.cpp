//-------------------------------------------------------------------
//
//  File:        FarePrivateIndicatorQualifierTest.cpp
//  Author:      Lifu Liu
//  Created:     April 18, 2016
//  Description: This class does unit tests of the FarePrivateIndicatorQualifier,
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
#include "FareDisplay/FarePrivateIndicatorQualifier.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/FareByRuleApp.h"


using namespace std;

namespace tse
{

class FarePrivateIndicatorQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FarePrivateIndicatorQualifierTest);
  CPPUNIT_TEST(testQualifyFarePrivateIndicatorForNotMatch);
  CPPUNIT_TEST(testQualifyFarePrivateIndicatorForEmpty);
  CPPUNIT_TEST(testQualifyFareOldPrivateIndicatorForXTKTC35);
  CPPUNIT_TEST(testQualifyFareOldPrivateIndicatorForCORPID);
  CPPUNIT_TEST(testQualifyFareOldPrivateIndicatorForPRIVATE);
  CPPUNIT_TEST(testQualifyFareNewPrivateIndicatorForCAT35);
  CPPUNIT_TEST(testQualifyFareNewPrivateIndicatorForXTKTC35);


  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testQualifyFarePrivateIndicatorForNotMatch()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = 'X';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Private_Indicator);
  }

  void testQualifyFarePrivateIndicatorForEmpty()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = ' ';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
  }

  void testQualifyFareOldPrivateIndicatorForPRIVATE()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = '@';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
  }

  void testQualifyFareNewPrivateIndicatorForCAT35()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = '/';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;

    const FareClassAppInfo* fcai = ptFare.fareClassAppInfo();
    const_cast<FareClassAppInfo*>(fcai)->_displayCatType = 'C';

    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setMatchedCorpID(false);
    ptFare.fareDisplayCat35Type() = 'M';
    ptFare.setFare(&fare);

    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
  }

  void testQualifyFareOldPrivateIndicatorForXTKTC35()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = 'X';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
    NegPaxTypeFareRuleData* negPTFareRuleData(_memHandle(new NegPaxTypeFareRuleData));
    negPTFareRuleData->tktIndicator() = 'N';
    ptFare.setRuleData(RuleConst::NEGOTIATED_RULE, fdTrx.dataHandle(), negPTFareRuleData);
    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
  }

  void testQualifyFareNewPrivateIndicatorForXTKTC35()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FarePrivateIndicatorQualifier fpiq;

    fdTrx.setOptions(&options);

    options.frrPrivateIndicator() = 'X';
    tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
    const FareClassAppInfo* fcai = ptFare.fareClassAppInfo();
    const_cast<FareClassAppInfo*>(fcai)->_displayCatType = 'C';
    NegPaxTypeFareRuleData* negPTFareRuleData(_memHandle(new NegPaxTypeFareRuleData));
    negPTFareRuleData->tktIndicator() = 'N';
    ptFare.setRuleData(RuleConst::NEGOTIATED_RULE, fdTrx.dataHandle(), negPTFareRuleData);
    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
    CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
  }

  void testQualifyFareOldPrivateIndicatorForCORPID()
    {
      FareDisplayTrx fdTrx;
      PaxTypeFare ptFare;
      FareInfo fareInfo;
      Fare fare;
      FareMarket fm;
      TariffCrossRefInfo tcrInfo;
      FareDisplayOptions options;
      FarePrivateIndicatorQualifier fpiq;

      fdTrx.setOptions(&options);

      options.frrPrivateIndicator() = '*';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setMatchedCorpID();
      ptFare.setCat15SecurityValid(true);

      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = fpiq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FarePrivateIndicatorQualifierTest);
}
