#include "Common/AltPricingUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NoPNRPricingOptions.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareCalcConfig.h"
#include "Server/TseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

#include <iostream>

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;
using namespace std;
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

const string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const string LOC_DEN = "/vobs/atseintl/test/testdata/data/LocDEN.xml";
const string LOC_CHI = "/vobs/atseintl/test/testdata/data/LocCHI.xml";
const string LOC_NYC1 = "/vobs/atseintl/test/testdata/data/LocNYC.xml";
const string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const string LOC_FRA = "/vobs/atseintl/test/testdata/data/LocFRA.xml";

class TseServerMock : public TseServer
{
public:
  static tse::ConfigMan* getConfig() { return Global::_configMan; }
  static void setConfig(tse::ConfigMan* configMan) { Global::_configMan = configMan; }
};

class AltPricingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AltPricingUtilTest);
  CPPUNIT_TEST(testIgnoreAvailCheckWhenMipTrx);
  CPPUNIT_TEST(testIgnoreAvailCheckWhenWPNC);
  CPPUNIT_TEST(testIgnoreAvailCheckWhenNoFareCalcConfig);
  CPPUNIT_TEST(testIgnoreAvailCheckWhenFCCnoMatchAvailIsNO);
  CPPUNIT_TEST(testIgnoreAvailCheckWhenFCCnoMatchAvailIsYES);
  CPPUNIT_SKIP_TEST(testValidateFarePathBookingCodeNoPNRPricingTrue);
  CPPUNIT_TEST(testValidateFarePathBookingCodeWPPricingTrue);
  CPPUNIT_TEST(testValidateFarePathBookingCodeWPANotXMrequestTrue);
  CPPUNIT_TEST(testValidateFarePathBookingCodeTrue);
  CPPUNIT_TEST(testValidateFarePathBookingCodeFalse);
  CPPUNIT_TEST(testValidateFarePathBookingCodeDiffFalse);
  CPPUNIT_TEST(testValidateFarePathBookingCodeDiffTrue);
  CPPUNIT_TEST_SUITE_END();

public:
  //-----------------------------------------------
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TseServerMock::setConfig(&_config);
    _trx = new PricingTrx();
    _request = new PricingRequest();
    _trx->setRequest(_request);

    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;
  }

  //-----------------------------------------------
  void tearDown()
  {
    _memHandle.clear();
    delete _request;
    delete _trx;
  }

  //-----------------------------------------------
  void testIgnoreAvailCheckWhenMipTrx()
  {
    setTrxAsMip();

    CPPUNIT_ASSERT(!AltPricingUtil::ignoreAvail(*_trx));
  }

  //-----------------------------------------------
  void testIgnoreAvailCheckWhenWPNC()
  {
    setTrxAsPricing();
    setRequestAsWPNC();
    CPPUNIT_ASSERT(!AltPricingUtil::ignoreAvail(*_trx));
  }

  //-----------------------------------------------
  void testIgnoreAvailCheckWhenNoFareCalcConfig()
  {
    setTrxAsPricing();
    setRequestAsWP();
    _trx->fareCalcConfig() = new FareCalcConfig();
    CPPUNIT_ASSERT(!AltPricingUtil::ignoreAvail(*_trx));
  }

  //-----------------------------------------------
  void testIgnoreAvailCheckWhenFCCnoMatchAvailIsNO()
  {
    setTrxAsPricing();
    setRequestAsWP();
    _trx->fareCalcConfig() = new FareCalcConfig();
    _trx->fareCalcConfig()->noMatchAvail() = 'N';
    CPPUNIT_ASSERT(!AltPricingUtil::ignoreAvail(*_trx));
  }

  //-----------------------------------------------
  void testIgnoreAvailCheckWhenFCCnoMatchAvailIsYES()
  {
    setTrxAsPricing();
    setRequestAsWP();
    _trx->fareCalcConfig() = new FareCalcConfig();
    _trx->fareCalcConfig()->noMatchAvail() = 'Y';
    CPPUNIT_ASSERT(AltPricingUtil::ignoreAvail(*_trx));
  }

  //-----------------------------------------------
  //-----------------------------------------------
  void testValidateFarePathBookingCodeNoPNRPricingTrue()
  {
    NoPNRPricingTrx nTrx;

    nTrx.setRequest(_request);
    NoPNRPricingOptions options;
    nTrx.setOptions(&options);
    nTrx.noPNRPricing() = 'T';
    setRequestAsWPNC();
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatus();
    AltPricingUtil::prepareToProcess(nTrx);
    CPPUNIT_ASSERT(AltPricingUtil::validateFarePathBookingCode(nTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeWPPricingTrue()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WP;
    setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatus();
    AltPricingUtil::prepareToProcess(aTrx);
    CPPUNIT_ASSERT(AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeWPANotXMrequestTrue()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WPA;
    //  setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatus();
    AltPricingUtil::prepareToProcess(aTrx);
    CPPUNIT_ASSERT(AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeTrue()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WPA;
    setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatus();
    AltPricingUtil::prepareToProcess(aTrx);
    CPPUNIT_ASSERT(AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeFalse()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WPA;
    setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatusPASS();
    AltPricingUtil::prepareToProcess(aTrx);

    CPPUNIT_ASSERT(!AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeDiffFalse()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WPA;
    setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatusPASS();
    createDifferentialDataFailed();
    AltPricingUtil::prepareToProcess(aTrx);
    CPPUNIT_ASSERT(!AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

  void testValidateFarePathBookingCodeDiffTrue()
  {
    AltPricingTrx aTrx;

    aTrx.altTrxType() = PricingTrx::WPA;
    setRequestAsWPNC();
    aTrx.setRequest(_trx->getRequest());
    aTrx.setOptions(_trx->getOptions());
    createBasicPricingUnitAndItin();
    setUpBkgCodeStatusPASS();
    createDifferentialDataPass();
    AltPricingUtil::prepareToProcess(aTrx);
    CPPUNIT_ASSERT(AltPricingUtil::validateFarePathBookingCode(aTrx, *_farePath));
  }

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  tse::ConfigMan _config;
  Itin* _itin;
  FarePath* _farePath;
  TestMemHandle _memHandle;

  //-----------------------------------------------
  void setTrxAsMip()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    return;
  }

  //-----------------------------------------------
  void setTrxAsPricing()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    return;
  }

  //-----------------------------------------------
  void setRequestAsWPNC()
  {
    _request->lowFareRequested() = 'Y';
    return;
  }

  //-----------------------------------------------
  void setRequestAsWP()
  {
    _request->lowFareRequested() = 'N';
    return;
  }

  void createBasicPricingUnitAndItin()
  {
    AirSeg* ts1 = _memHandle.create<AirSeg>();
    AirSeg* ts2 = _memHandle.create<AirSeg>();
    AirSeg* ts3 = _memHandle.create<AirSeg>();

    ts1->origin() = TestLocFactory::create(LOC_DFW);
    ts1->destination() = TestLocFactory::create(LOC_DEN);
    ts2->origin() = TestLocFactory::create(LOC_CHI);
    ts2->destination() = TestLocFactory::create(LOC_NYC1);
    ts3->origin() = TestLocFactory::create(LOC_DEN);
    ts3->destination() = TestLocFactory::create(LOC_LON);

    ts1->segmentType() = Air;
    ts2->segmentType() = Air;
    ts3->segmentType() = Air;

    _itin->travelSeg() += ts1, ts2, ts3;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    fu1->travelSeg() += ts1, ts3;
    fu2->travelSeg() += ts2;
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage() += fu1;
    pu->fareUsage() += fu2;
    _farePath->pricingUnit() += pu;

    PaxTypeFare::SegmentStatus segS1;
    segS1._bkgCodeSegStatus.set(PaxTypeFare::BKSS_NOT_YET_PROCESSED, true);
    fu1->segmentStatus().clear();
    fu1->segmentStatus().push_back(segS1);
    fu1->segmentStatus().push_back(segS1);

    fu2->segmentStatus().clear();
    fu2->segmentStatus().push_back(segS1);
  }
  void setUpBkgCodeStatus()
  {
    vector<PricingUnit*>::iterator iter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator iterEnd = _farePath->pricingUnit().end();

    for (; iter != iterEnd; ++iter)
    {
      PricingUnit* pu = *iter;

      vector<FareUsage*>::iterator fuIter = pu->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = pu->fareUsage().end();

      for (; fuIter != fuIterEnd; ++fuIter)
      {
        FareUsage* fu = *fuIter;
        std::vector<TravelSeg*>::iterator iterTvl = fu->travelSeg().begin();
        std::vector<TravelSeg*>::iterator iterTvlEnd = fu->travelSeg().end();

        for (int i = 0; iterTvl != iterTvlEnd; iterTvl++, i++) // Initialize
        {
          PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[i];
          segStat._bkgCodeSegStatus.setNull();
          if (i)
          {
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
          }
          else
          {
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
            segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
          }
        }
      }
    }
  }

  void setUpBkgCodeStatusPASS()
  {
    vector<PricingUnit*>::iterator iter = _farePath->pricingUnit().begin();
    vector<PricingUnit*>::iterator iterEnd = _farePath->pricingUnit().end();

    for (; iter != iterEnd; ++iter)
    {
      PricingUnit* pu = *iter;

      vector<FareUsage*>::iterator fuIter = pu->fareUsage().begin();
      vector<FareUsage*>::iterator fuIterEnd = pu->fareUsage().end();

      for (; fuIter != fuIterEnd; ++fuIter)
      {
        FareUsage* fu = *fuIter;
        std::vector<TravelSeg*>::iterator iterTvl = fu->travelSeg().begin();
        std::vector<TravelSeg*>::iterator iterTvlEnd = fu->travelSeg().end();

        for (int i = 0; iterTvl != iterTvlEnd; iterTvl++, i++) // Initialize
        {
          PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[i];
          segStat._bkgCodeSegStatus.setNull();
          segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
        }
      }
    }
  }

  void createDifferentialDataFailed()
  {
    DifferentialData dd;
    dd.status() = DifferentialData::SC_FAILED;
    _farePath->pricingUnit().front()->fareUsage().front()->differentialPlusUp().push_back(&dd);
  }

  void createDifferentialDataPass()
  {
    DifferentialData* dd = _memHandle.create<DifferentialData>();
    ;
    dd->status() = DifferentialData::SC_PASSED;
    PaxTypeFare* fare =
        TestPaxTypeFareFactory::create("/vobs/atseintl/test/testdata/data/PaxTypeFare.xml");
    dd->fareHigh() = fare;

    _farePath->pricingUnit().front()->fareUsage().front()->differentialPlusUp().push_back(dd);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AltPricingUtilTest);
}
