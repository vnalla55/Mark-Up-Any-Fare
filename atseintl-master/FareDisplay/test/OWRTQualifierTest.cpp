//-------------------------------------------------------------------
//
//  File:        OWRTQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 20, 2006
//  Description: This class does unit tests of the
//               OWRTQualifier class.
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
#include "FareDisplay/OWRTQualifier.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"

#include <log4cxx/propertyconfigurator.h>

using namespace std;

namespace tse
{
class OWRTQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OWRTQualifierTest);
  CPPUNIT_TEST(testQualifyOWRT);
  CPPUNIT_TEST_SUITE_END();

  void testQualifyOWRT()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    FareClassAppInfo fca;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareDisplayRequest request;
    FareDisplayPref fdPref;
    OWRTQualifier owq;
    Itin itin;

    options.fareDisplayPref() = &fdPref;
    fdTrx.setOptions(&options);
    fdTrx.setRequest(&request);

    fare.setFareInfo(&fareInfo);
    ptFare.setFare(&fare);
    ptFare.fareClassAppInfo() = &fca;

    itin.fareMarket().push_back(&fm);

    fdTrx.itin().push_back(&itin);

    {
      options.oneWayFare() = 'Y';
      fareInfo.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
      const tse::PaxTypeFare::FareDisplayState ret = owq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_OW_RT);
    }
    {
      options.oneWayFare() = 'N';
      options.roundTripFare() = 'Y';
      fareInfo.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
      const tse::PaxTypeFare::FareDisplayState ret = owq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_OW_RT);
    }
    {
      options.oneWayFare() = 'N';
      options.roundTripFare() = 'N';
      options.halfRoundTripFare() = 'Y';
      fareInfo.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
      const tse::PaxTypeFare::FareDisplayState ret = owq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_OW_RT);
    }
    {
      options.oneWayFare() = 'N';
      options.roundTripFare() = 'N';
      options.halfRoundTripFare() = 'N';
      const tse::PaxTypeFare::FareDisplayState ret = owq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(OWRTQualifierTest);
}
