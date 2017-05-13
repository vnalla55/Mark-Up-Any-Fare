//-------------------------------------------------------------------
//
//  File:        DirectionalityQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 9, 2006
//  Description: This class does unit tests of the DirectionalityQualifier,
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
#include "FareDisplay/DirectionalityQualifier.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class DirectionalityQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DirectionalityQualifierTest);
  CPPUNIT_TEST(testQualifyDirectionality);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void getLoc(FareDisplayTrx& trx, Loc& loc, LocCode& code)
  {
    DateTime date = DateTime::localTime();
    const Loc* lc = trx.dataHandle().getLoc(code, date);
    loc = *(const_cast<Loc*>(lc));
  }

  void testQualifyDirectionality()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareInfo fareInfo;
    Fare fare;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    DirectionalityQualifier dirq;
    AirSeg seg1;

    DateTime dt1(2005, 9, 12, 12, 0, 0);

    fdTrx.setOptions(&options);

    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);

    seg1.departureDT() = dt1;
    fdTrx.travelSeg().push_back(&seg1);

    Loc origin;
    Loc destination;
    LocCode orig = "DFW";
    LocCode dest = "LON";

    getLoc(fdTrx, origin, orig);
    getLoc(fdTrx, destination, dest);

    Itin itin;

    fm.origin() = &origin;
    fm.destination() = &destination;
    fm.governingCarrier() = "BA";

    fm.boardMultiCity() = orig;
    fm.offMultiCity() = dest;

    itin.fareMarket().push_back(&fm);
    fdTrx.itin().push_back(&itin);
    fdTrx.fareMarket().push_back(&fm);

    {
      fareInfo.directionality() = TO;
      const tse::PaxTypeFare::FareDisplayState ret = dirq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Not_Valid_For_Direction);
    }
    {
      fareInfo.directionality() = FROM;
      const tse::PaxTypeFare::FareDisplayState ret = dirq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      fm.offMultiCity() = orig;
      const tse::PaxTypeFare::FareDisplayState ret = dirq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DirectionalityQualifierTest);
}
