#include "AddonConstruction/test/ConstructionJobMock.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/SmfGatewayPair.h"
#include "AddonConstruction/VendorSmf.h"
#include "Common/DateTime.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/DataHandle.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{
#define MINUS_ONE_HOUR_TIME_SHFT -1
class SmfGatewayPairTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SmfGatewayPairTest);

  CPPUNIT_TEST(matchAddonAndSpecifiedIsHistoricalFail);
  CPPUNIT_TEST(matchAddonAndSpecifiedIsNotHistoricalFail);
  CPPUNIT_TEST(matchAddonAndSpecifiedOWRTFail);

  CPPUNIT_TEST_SUITE_END();

  ConstructionJobMock* _cJob;
  PricingTrx* _trx;
  LocCode _orig;
  LocCode _dest;
  LocCode _boardCity;
  LocCode _offCity;
  AddonFareCortege _afc;
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  SpecifiedFare _sf;
#else
  AtpcoConstructedFare _cf;
#endif
  SmfGatewayPair* _smfChecker;
  VendorSmf* _smfVendor;
  bool _isHistorical;
  TestMemHandle _memHandle;

  class MyDataHandle : public DataHandleMock
  {
  public:
    const bool isHistorical() { return false; }
  };

public:
  void tearDown() { _memHandle.clear(); }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  void setUpSmfGatewayPair(int timeShift = 0)
  {
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _cJob = _memHandle.insert(new ConstructionJobMock(*_trx, _isHistorical));

    _cJob->setVendorCode("SMF");
    _cJob->carrier() = "AA";

    _smfChecker = _memHandle.create<SmfGatewayPair>();
    _smfVendor = _memHandle.create<VendorSmf>();
    _smfChecker->initialize(_cJob, _smfVendor);

    FareInfo* fi = _memHandle.create<FareInfo>();
    AddonFareInfo* afi = _memHandle.create<AddonFareInfo>();
    fi->effInterval().effDate() = DateTime::localTime();
    fi->effInterval().expireDate() = DateTime::localTime();
    _afc.effInterval().effDate() = DateTime::localTime();
    _afc.effInterval().expireDate() = DateTime::localTime();
    _afc.effInterval().discDate() = DateTime::localTime() + Hours(timeShift);

    afi->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _sf._specFare = fi;
    _afc.addonFare() = afi;
  }

  void matchAddonAndSpecifiedIsHistoricalFail()
  {
    AtpcoFareDateInterval cfi;
    _isHistorical = true;
    setUpSmfGatewayPair(MINUS_ONE_HOUR_TIME_SHFT);
    CPPUNIT_ASSERT_EQUAL(FM_DATE_INTERVAL_MISMATCH,
                         _smfChecker->matchAddonAndSpecified(_afc, _sf, false, true/*originAddon*/, cfi));
  }

  void matchAddonAndSpecifiedIsNotHistoricalFail()
  {
    AtpcoFareDateInterval cfi;
    _isHistorical = false;
    setUpSmfGatewayPair(MINUS_ONE_HOUR_TIME_SHFT);
    CPPUNIT_ASSERT_EQUAL(FM_DATE_INTERVAL_MISMATCH,
                         _smfChecker->matchAddonAndSpecified(_afc, _sf, false, true/*originAddon*/, cfi));
  }

  void matchAddonAndSpecifiedOWRTFail()
  {
    AtpcoFareDateInterval cfi;
    setUpSmfGatewayPair();
    CPPUNIT_ASSERT_EQUAL(FM_OWRT, _smfChecker->matchAddonAndSpecified(_afc, _sf, true, true/*originAddon*/, cfi));
  }

#else

  void setUpSmfGatewayPair(int timeShift = 0)
  {
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _cJob = _memHandle.insert(new ConstructionJobMock(*_trx, _isHistorical));

    _cJob->setVendorCode("SMF");
    _cJob->carrier() = "AA";

    _smfChecker = _memHandle.create<SmfGatewayPair>();
    _smfVendor = _memHandle.create<VendorSmf>();
    _smfChecker->initialize(_cJob, _smfVendor);

    FareInfo* fi = _memHandle.create<FareInfo>();
    AddonFareInfo* afi = _memHandle.create<AddonFareInfo>();
    _cf.prevEffInterval().effDate() = DateTime::localTime();
    _cf.prevEffInterval().expireDate() = DateTime::localTime();
    _afc.effInterval().effDate() = DateTime::localTime();
    _afc.effInterval().expireDate() = DateTime::localTime();
    _afc.effInterval().discDate() = DateTime::localTime() + Hours(timeShift);

    afi->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    fi->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _cf.specifiedFare() = fi;
    _afc.addonFare() = afi;
  }

  void matchAddonAndSpecifiedIsHistoricalFail()
  {
    std::vector<DateIntervalBase*> cfi;
    _isHistorical = true;
    setUpSmfGatewayPair(MINUS_ONE_HOUR_TIME_SHFT);
    CPPUNIT_ASSERT_EQUAL(FM_DATE_INTERVAL_MISMATCH,
                         _smfChecker->matchAddonAndSpecified(_afc, _cf, false, cfi));
  }

  void matchAddonAndSpecifiedIsNotHistoricalFail()
  {
    std::vector<DateIntervalBase*> cfi;
    _isHistorical = false;
    setUpSmfGatewayPair(MINUS_ONE_HOUR_TIME_SHFT);
    CPPUNIT_ASSERT_EQUAL(FM_DATE_INTERVAL_MISMATCH,
                         _smfChecker->matchAddonAndSpecified(_afc, _cf, false, cfi));
  }

  void matchAddonAndSpecifiedOWRTFail()
  {
    std::vector<DateIntervalBase*> cfi;
    setUpSmfGatewayPair();
    CPPUNIT_ASSERT_EQUAL(FM_OWRT, _smfChecker->matchAddonAndSpecified(_afc, _cf, true, cfi));
  }

#endif

};
CPPUNIT_TEST_SUITE_REGISTRATION(SmfGatewayPairTest);
}
