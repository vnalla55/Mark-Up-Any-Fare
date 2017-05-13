#include "Rules/CategoryGeoCommon.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class CategoryGeoCommonTest : public CppUnit::TestFixture
{

  class MyDataHandle : public DataHandleMock
  {
  public:
    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "NYC")
        return "NYC";
      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }
    const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemNumber)
    {
      if (itemNumber == 0)
        return _geoRetVec;
      return DataHandleMock::getGeoRuleItem(vendor, itemNumber);
    }
    std::vector<GeoRuleItem*> _geoRetVec;
  };
  class TestCategoryGeoCommon : public CategoryGeoCommon
  {
    friend class CategoryGeoCommonTest;

  public:
    GeoBools _gb;
    void setPaxTypeFare(PaxTypeFare* ptf) { _paxTypeFare = ptf; }
    void setTrx(PricingTrx* trx) { _trx = trx; }
  };

  CPPUNIT_TEST_SUITE(CategoryGeoCommonTest);
  CPPUNIT_TEST(testGetTvlSegs_ViaPass);
  CPPUNIT_TEST(testGetTvlSegs_ViaNoMatchOrigin);
  CPPUNIT_TEST(testGetTvlSegs_ViaNoMatchDestination);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ptf = _memHandle.create<PaxTypeFare>();
    _fm = _ptf->fareMarket() = _memHandle.create<FareMarket>();
    Fare* f = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    _ptf->setFare(f);
    f->setFareInfo(fi);
    fi->vendor() = "ATP";
    _fm->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _fm->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    a1->origin() = _fm->origin();
    a1->destination() = a2->origin() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    a2->destination() = _fm->destination();
    _fm->travelSeg().push_back(a1);
    _fm->travelSeg().push_back(a2);
    _cg = _memHandle.create<TestCategoryGeoCommon>();
    _cg->setPaxTypeFare(_ptf);
    _checkOrig = false;
    _checkDest = false;
    _locKey.locType() = 'C';
    _locKey.loc() = "NYC";
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    trx->setRequest(req);
    _cg->setTrx(trx);
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }
  void tearDown() { _memHandle.clear(); }

  void testGetTvlSegs_ViaPass()
  {
    _cg->_gb.via = true;
    CPPUNIT_ASSERT(_cg->getTvlSegs(0, _locKey, _retTsVec, _cg->_gb, _checkOrig, _checkDest));
  }
  void testGetTvlSegs_ViaNoMatchOrigin()
  {
    _cg->_gb.via = true;
    _locKey.loc() = "DFW";
    CPPUNIT_ASSERT(!_cg->getTvlSegs(0, _locKey, _retTsVec, _cg->_gb, _checkOrig, _checkDest));
  }
  void testGetTvlSegs_ViaNoMatchDestination()
  {
    _cg->_gb.via = true;
    _locKey.loc() = "LON";
    CPPUNIT_ASSERT(!_cg->getTvlSegs(0, _locKey, _retTsVec, _cg->_gb, _checkOrig, _checkDest));
  }

private:
  TestMemHandle _memHandle;
  PaxTypeFare* _ptf;
  FareMarket* _fm;
  TestCategoryGeoCommon* _cg;
  bool _checkOrig;
  std::vector<TravelSeg*> _retTsVec;
  bool _checkDest;
  LocKey _locKey;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CategoryGeoCommonTest);
}
