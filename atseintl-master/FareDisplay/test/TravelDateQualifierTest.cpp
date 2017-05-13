//-------------------------------------------------------------------
//
//  File:        TravelDateQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 20, 2006
//  Description: This class does unit tests of the
//               TraveldateQualifier class.
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
#include "FareDisplay/TravelDateQualifier.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "DataModel/AirSeg.h"

using namespace std;

namespace tse
{
class TravelDateQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TravelDateQualifierTest);
  CPPUNIT_TEST(testQualifyTravelDate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testQualifyTravelDate()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareDisplayRequest request;
    FareDisplayPref fdPref;
    TravelDateQualifier tdq;
    Itin itin;

    AirSeg seg1;
    AirSeg seg2;
    AirSeg seg3;
    AirSeg seg4;
    AirSeg seg5;

    DateTime dt1(2006, 9, 12, 12, 0, 0);
    DateTime dt2(2006, 9, 14, 12, 0, 0);
    DateTime dt3(2006, 9, 15, 12, 0, 0);
    DateTime dt4(2006, 9, 17, 12, 0, 0);

    seg1.departureDT() = dt1;
    seg2.departureDT() = dt1;
    seg3.departureDT() = dt1;
    seg4.departureDT() = dt1;
    seg5.departureDT() = dt2;

    fm.travelSeg().push_back(&seg1);
    fm.travelSeg().push_back(&seg2);
    fm.travelSeg().push_back(&seg3);
    fm.travelSeg().push_back(&seg4);

    fareInfo.effDate() = dt2;
    fareInfo.discDate() = dt3;

    fdPref.returnDateValidation() = 'Y';
    options.fareDisplayPref() = &fdPref;
    fdTrx.setOptions(&options);

    fare.setFareInfo(&fareInfo);
    ptFare.setFare(&fare);

    itin.fareMarket().push_back(&fm);

    fdTrx.itin().push_back(&itin);

    {
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }
    {
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt1;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }

    {
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt2;
      fareInfo.effDate() = dt1;
      fareInfo.discDate() = dt1;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }

    {
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt2;
      request.dateRangeUpper() = dt4;
      fareInfo.effDate() = dt1;
      fareInfo.discDate() = dt3;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }

    {
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt2;
      request.dateRangeUpper() = dt2;
      fdTrx.setTravelDate(dt4);
      fareInfo.effDate() = dt1;
      fareInfo.discDate() = dt3;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }

    {
      fdPref.returnDateValidation() = 'N';
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt2;
      request.dateRangeUpper() = dt2;
      request.returnDate() = dt4;
      fdTrx.setTravelDate(dt2);
      fareInfo.effDate() = dt1;
      fareInfo.discDate() = dt3;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Eff_Disc_Dates);
    }

    {
      fdTrx.setRequest(&request);
      request.dateRangeLower() = dt2;
      request.dateRangeUpper() = dt2;
      request.returnDate() = dt2;
      fdTrx.setTravelDate(dt2);
      fareInfo.effDate() = dt1;
      fareInfo.discDate() = dt3;
      const tse::PaxTypeFare::FareDisplayState ret = tdq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TravelDateQualifierTest);
}
