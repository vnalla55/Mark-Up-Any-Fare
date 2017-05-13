#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "FareDisplay/FareSelector.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class FareSelectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareSelectorTest);
  CPPUNIT_TEST(testSelectFares);
  CPPUNIT_TEST(testCheckInhibitIndicator);
  CPPUNIT_TEST(testQualifyOutboundCurrency);
  CPPUNIT_TEST_SUITE_END();

  void getLoc(FareDisplayTrx& trx, Loc& loc, LocCode& code)
  {
    DateTime date = DateTime::localTime();
    const Loc* lc = trx.dataHandle().getLoc(code, date);
    loc = *(const_cast<Loc*>(lc));
  }

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  void testQualifyOutboundCurrency()
  {
    FareInfo fareInfo1, fareInfo2;
    Fare fare1, fare2;
    PaxTypeFare ptFare1, ptFare2;
    FareMarket fm;
    FareDisplayResponse fdResponse;
    FareDisplayTrx fdTrx;
    PaxTypeBucket ptCortege;
    CurrencyCode cur1 = "USD";
    CurrencyCode cur2 = "PLN";

    fdTrx.fdResponse() = &fdResponse;
    fdTrx.itin().push_back(_memHandle.create<Itin>());

    fareInfo1.currency() = cur1;
    fareInfo2.currency() = cur2;

    fare1.setFareInfo(&fareInfo1);
    fare2.setFareInfo(&fareInfo2);

    ptFare1.setFare(&fare1);
    ptFare2.setFare(&fare2);

    ptFare1.setIsShoppingFare();
    ptFare2.setIsShoppingFare();

    ptCortege.paxTypeFare().push_back(&ptFare1);
    ptCortege.paxTypeFare().push_back(&ptFare2);

    fm.paxTypeCortege().push_back(ptCortege);

    FareSelector fs;

    fs.qualifyOutboundCurrency(fdTrx, fm, cur1);

    CPPUNIT_ASSERT(!ptFare1.fareDisplayStatus().isSet(PaxTypeFare::FD_Outbound_Currency));
    CPPUNIT_ASSERT(ptFare2.fareDisplayStatus().isSet(PaxTypeFare::FD_Outbound_Currency));
  }

  void testCheckInhibitIndicator()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFare ptFare;
    FareDisplayInfo fdInfo;
    FareInfo fareInfo;
    Fare fare;

    fareInfo.inhibit() = RuleConst::FARE_FOR_DISPLAY_ONLY;

    fare.setFareInfo(&fareInfo);
    ptFare.setFare(&fare);
    ptFare.fareDisplayInfo() = &fdInfo;

    FareSelector fs;

    fs.checkInhibitIndicator(fdTrx, ptFare);

    FareDisplayInfo* pInfo = ptFare.fareDisplayInfo();
    CPPUNIT_ASSERT(pInfo != NULL);
    CPPUNIT_ASSERT(pInfo->displayOnly() == true);
  }

  void testSelectFares()
  {
    FareDisplayTrx trx;
    FareSelector fs;
    CPPUNIT_ASSERT(!fs.selectFares(trx));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareSelectorTest);
}
