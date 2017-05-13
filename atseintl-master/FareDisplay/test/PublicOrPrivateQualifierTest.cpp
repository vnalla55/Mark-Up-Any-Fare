//-------------------------------------------------------------------
//
//  File:        PublicOrPrivateQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 9, 2006
//  Description: This class does unit tests of the PublicOrPrivateQualifier,
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
#include "FareDisplay/PublicOrPrivateQualifier.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"

using namespace std;

namespace tse
{
class PublicOrPrivateQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PublicOrPrivateQualifierTest);
  CPPUNIT_TEST(testQualifyPublicOrPrivate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testQualifyPublicOrPrivate()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    PublicOrPrivateQualifier ppq;

    fdTrx.setOptions(&options);

    {
      options.publicFares() = 'Y';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = ppq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Public);
    }

    {
      options.privateFares() = 'Y';
      tcrInfo.tariffCat() = RuleConst::PRIVATE_TARIFF + 1;
      fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
      ptFare.setFare(&fare);
      const tse::PaxTypeFare::FareDisplayState ret = ppq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Private);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PublicOrPrivateQualifierTest);
}
