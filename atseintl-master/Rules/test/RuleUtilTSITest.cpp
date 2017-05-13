#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/ZoneInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtilTSI.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class RuleUtilTSITest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle()
    {
      _aliance.genericAllianceCode() = ONE_WORLD_ALLIANCE;
      _aaCxr.push_back(&_aliance);

      _continent1.locCode() = "0000000";
      _continent1.locType() = ZONE;
      _continent2.locCode() = "0000210";
      _continent1.locType() = ZONE;

      _aaCont.push_back(&_continent1);
      _aaCont.push_back(&_continent2);
    }

    const std::vector<AirlineAllianceCarrierInfo*>&
    getAirlineAllianceCarrier(const CarrierCode& carrierCode)
    {
      return _aaCxr;
    }

    const std::vector<AirlineAllianceContinentInfo*>&
    getAirlineAllianceContinent(const GenericAllianceCode& genericAllianceCode,
                                bool reduceTemporaryVectorsFallback)
    {
      return _aaCont;
    }


    const ZoneInfo*
    getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
    {
      return DataHandleMock::getZone(Vendor::ATPCO, zone, RESERVED, date);
    }

  protected:
    std::vector<AirlineAllianceCarrierInfo*> _aaCxr;
    std::vector<AirlineAllianceContinentInfo*> _aaCont;
    AirlineAllianceCarrierInfo _aliance;
    AirlineAllianceContinentInfo _continent1, _continent2;
  };

  CPPUNIT_TEST_SUITE(RuleUtilTSITest);
  CPPUNIT_TEST(testFDIsIntercontinentalNotRWPass);
  CPPUNIT_TEST(testFDIsIntercontinentalRWPass);
  CPPUNIT_TEST(testFDIsNotIntercontinental);
  CPPUNIT_TEST(testRtwIsIntercontinental);
  CPPUNIT_TEST(testRtwIsNotIntercontinental);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFDIsIntercontinentalNotRWPass()
  {
    CPPUNIT_ASSERT(isIntercontinental(*createFD(), true, false));
  }

  void testFDIsIntercontinentalRWPass()
  {
    CPPUNIT_ASSERT(isIntercontinental(*createFD(), false, true));
  }

  void testFDIsNotIntercontinental()
  {
    CPPUNIT_ASSERT(!isIntercontinental(*createFD(), false, false));
  }

  void testRtwIsIntercontinental()
  {
    CPPUNIT_ASSERT(isIntercontinental(*createRTW(), true, false));
  }

  void testRtwIsNotIntercontinental()
  {
    CPPUNIT_ASSERT(!isIntercontinental(*createRTW(), false, false));
  }

private:
  bool isIntercontinental(PricingTrx& trx, bool isInt, bool isRW)
  {
    TSIInfo tsiInfo;
    RuleConst::TSIScopeType scope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    RuleUtilTSI::TSIData tsiData =
        RuleUtilTSI::TSIData(tsiInfo, scope, ATPCO_VENDOR_CODE, 0, 0, 0, 0);
    RuleUtilTSI::TSITravelSegMarkup tsm;
    AirSeg travelSeg;
    Loc orig, dest;
    std::string noMatchReason;

    orig.loc() = "FRA";
    orig.city() = "FRA";
    orig.nation() = "DE";

    if (isInt)
    {
      dest.loc() = "DFW";
      dest.city() = "DFW";
      dest.nation() = "US";
    }
    else
    {
      dest.loc() = "FRA";
      dest.city() = "FRA";
      dest.nation() = "DE";
    }

    travelSeg.origin() = &orig;
    travelSeg.destination() = &dest;

    tsm.travelSeg() = &travelSeg;
    if (isRW)
      tsm.globalDirection() = GlobalDirection::RW;

    RuleUtilTSI::TSIMatchCriteria tsiMC(trx, tsiData, tsm, noMatchReason);

    return tsiMC.process(TSIInfo::INTERCONTINENTAL);
  }

  FareDisplayTrx* createFD()
  {
    FareDisplayTrx* trx = _memHandle.create<FareDisplayTrx>();
    FareDisplayOptions* opt = _memHandle.create<FareDisplayOptions>();
    trx->setOptions(opt);

    return trx;
  }

  PricingTrx* createRTW()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingOptions* opt = _memHandle.create<PricingOptions>();
    trx->setOptions(opt);
    opt->setRtw(true);

    Itin* itin = _memHandle.create<Itin>();
    FareMarket* fm = _memHandle.create<FareMarket>();

    trx->itin().push_back(itin);
    itin->fareMarket().push_back(fm);

    return trx;
  }

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest);
}
