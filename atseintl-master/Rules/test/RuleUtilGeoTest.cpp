#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Rules/RuleUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include <boost/tokenizer.hpp>
#include <iostream>
#include <set>
#include <vector>
#include <time.h>

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  GeoRuleItem* getGeo(int tsi, LocTypeCode lt, LocCode lc, LocTypeCode lt2 = ' ', LocCode lc2 = "")
  {
    GeoRuleItem* ret = _memHandle.create<GeoRuleItem>();
    ret->tsi() = tsi;
    ret->loc1().locType() = lt;
    ret->loc1().loc() = lc;
    ret->loc2().locType() = lt2;
    ret->loc2().loc() = lc2;
    return ret;
  }

public:
  const TSIInfo* getTSI(int key)
  {
    tse::ConfigMan config;
    config.read("tsi.ini");
    std::vector<tse::ConfigMan::NameValue> recs;
    config.getValues(recs);
    std::vector<tse::ConfigMan::NameValue>::iterator it = recs.begin();
    std::vector<tse::ConfigMan::NameValue>::iterator ie = recs.end();
    for (; it != ie; it++)
    {
      int tsi = atoi(it->name.c_str());
      if (tsi == key)
      {
        boost::char_separator<char> sep("|");
        boost::tokenizer<boost::char_separator<char> > tok(it->value, sep);
        boost::tokenizer<boost::char_separator<char> >::iterator i = tok.begin();
        TSIInfo* tsiInfo = _memHandle.create<TSIInfo>();
        tsiInfo->tsi() = tsi;
        tsiInfo->description() = (*i);
        i++;
        tsiInfo->geoRequired() = (*i)[0];
        i++;
        tsiInfo->geoNotType() = (*i)[0];
        i++;
        tsiInfo->geoOut() = (*i)[0];
        i++;
        tsiInfo->geoItinPart() = (*i)[0];
        i++;
        tsiInfo->geoCheck() = (*i)[0];
        i++;
        tsiInfo->loopDirection() = (*i)[0];
        i++;
        tsiInfo->loopOffset() = atoi((*i).c_str());
        i++;
        tsiInfo->loopToSet() = atoi((*i).c_str());
        i++;
        tsiInfo->loopMatch() = (*i)[0];
        i++;
        tsiInfo->scope() = (*i)[0];
        i++;
        tsiInfo->type() = (*i)[0];
        i++;
        for (; i != tok.end(); i++)
        {
          char c = (*i)[0];
          if (c == 0)
            c = ' ';
          tsiInfo->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)c);
        }
        return tsiInfo;
      }
    }
    return DataHandleMock::getTSI(key);
  }
  const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemno)
  {
    std::vector<GeoRuleItem*>* ret = _memHandle.create<std::vector<GeoRuleItem*> >();
    if (vendor == "ATP")
    {
      if (itemno == 1)
      {
        ret->push_back(getGeo(0, 'P', "JFK"));
        return *ret;
      }
      else if (itemno == 19)
      {
        ret->push_back(getGeo(0, 'C', "ATL"));
        return *ret;
      }
      else if (itemno == 20)
      {
        return *ret;
      }
      else if (itemno == 28)
      {
        ret->push_back(getGeo(0, 'C', "SFO"));
        return *ret;
      }
      else if (itemno == 123)
      {
        ret->push_back(getGeo(6, ' ', ""));
        return *ret;
      }
      else if (itemno == 219)
      {
        ret->push_back(getGeo(0, 'A', "1"));
        return *ret;
      }
      else if (itemno == 3558)
      {
        ret->push_back(getGeo(0, 'C', "ATH"));
        return *ret;
      }
      else if (itemno == 3894)
      {
        ret->push_back(getGeo(0, 'C', "ATH", 'C', "VIE"));
        return *ret;
      }
    }

    return DataHandleMock::getGeoRuleItem(vendor, itemno);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "JFK")
      return "NYC";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
};
}

class RuleUtilGeoTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleUtilGeoTest);

  CPPUNIT_TEST(testValidateGeoRuleItem1);
  CPPUNIT_TEST(testValidateGeoRuleItem2);
  CPPUNIT_TEST(testValidateGeoRuleItem3);
  CPPUNIT_TEST(testValidateGeoRuleItem4);
  CPPUNIT_TEST(testValidateGeoRuleItem5);
  CPPUNIT_TEST(testValidateGeoRuleItem6);
  CPPUNIT_TEST(testValidateGeoRuleItem7);
  CPPUNIT_TEST(testValidateGeoRuleItem8);
  CPPUNIT_TEST(testValidateGeoRuleItem9);
  CPPUNIT_TEST(testValidateGeoRuleItem10);
  CPPUNIT_TEST(testValidateGeoRuleItem11);
  CPPUNIT_TEST(testValidateGeoRuleItem12);
  CPPUNIT_TEST(testValidateGeoRuleItem13);
  CPPUNIT_TEST(testValidateGeoRuleItem14);
  CPPUNIT_TEST(testValidateGeoRuleItem15);
  CPPUNIT_TEST(testValidateGeoRuleItem16);
  CPPUNIT_TEST(testValidateGeoRuleItem17);

  CPPUNIT_TEST(testGetTSIScopeFromGeoRuleItem1);

  CPPUNIT_TEST(testTable995WhollyWithin1);
  CPPUNIT_TEST(testTable995WhollyWithin2);

  CPPUNIT_TEST(testCheckGeoNotType1);
  CPPUNIT_TEST(testCheckGeoNotType2);
  CPPUNIT_TEST(testCheckGeoNotType3);
  CPPUNIT_TEST(testCheckGeoNotType4);
  CPPUNIT_TEST(testCheckGeoNotType5);
  CPPUNIT_TEST(testCheckGeoNotType6);
  CPPUNIT_TEST(testCheckGeoNotType7);
  CPPUNIT_TEST(testCheckGeoNotType8);
  CPPUNIT_TEST(testCheckGeoNotType9);
  CPPUNIT_TEST(testCheckGeoNotType10);
  CPPUNIT_TEST(testCheckGeoNotType11);
  CPPUNIT_TEST(testCheckGeoNotType12);
  CPPUNIT_TEST(testCheckGeoNotType13);
  CPPUNIT_TEST(testCheckGeoNotType14);
  CPPUNIT_TEST(testCheckGeoNotType15);
  CPPUNIT_TEST(testCheckGeoNotType16);
  CPPUNIT_TEST(testCheckGeoNotType17);
  CPPUNIT_TEST(testCheckGeoNotType18);
  CPPUNIT_TEST(testCheckGeoNotType19);
  CPPUNIT_TEST(testCheckGeoNotType20);
  CPPUNIT_TEST(testCheckGeoNotType21);
  CPPUNIT_TEST(testCheckGeoNotType22);
  CPPUNIT_TEST(testCheckGeoNotType23);
  CPPUNIT_TEST(testCheckGeoNotType24);
  CPPUNIT_TEST(testCheckGeoNotType25);
  CPPUNIT_TEST(testCheckGeoNotType26);
  CPPUNIT_TEST(testCheckGeoNotType27);
  CPPUNIT_TEST(testCheckGeoNotType28);
  CPPUNIT_TEST(testCheckGeoNotType29);
  CPPUNIT_TEST(testCheckGeoNotType30);

  CPPUNIT_TEST(testGetGeoData1);
  CPPUNIT_TEST(testGetGeoData2);
  CPPUNIT_TEST(testGetGeoData3);
  CPPUNIT_TEST(testGetGeoData4);
  CPPUNIT_TEST(testGetGeoData5);
  CPPUNIT_TEST(testGetGeoData6);
  CPPUNIT_TEST(testGetGeoData7);
  CPPUNIT_TEST(testGetGeoData8);
  CPPUNIT_TEST(testGetGeoData9);

  CPPUNIT_TEST(testCheckGeoData1);
  CPPUNIT_TEST(testCheckGeoData2);
  CPPUNIT_TEST(testCheckGeoData3);
  CPPUNIT_TEST(testCheckGeoData4);
  CPPUNIT_TEST(testCheckGeoData5);
  CPPUNIT_TEST(testCheckGeoData6);
  CPPUNIT_TEST(testCheckGeoData7);
  CPPUNIT_TEST(testCheckGeoData8);
  CPPUNIT_TEST(testCheckGeoData9);
  CPPUNIT_TEST(testCheckGeoData10);
  CPPUNIT_TEST(testCheckGeoData11);
  CPPUNIT_TEST(testCheckGeoData12);
  CPPUNIT_TEST(testCheckGeoData13);
  CPPUNIT_TEST(testCheckGeoData14);
  CPPUNIT_TEST(testCheckGeoData15);
  CPPUNIT_TEST(testCheckGeoData16);
  CPPUNIT_TEST(testCheckGeoData17);
  CPPUNIT_TEST(testCheckGeoData18);
  CPPUNIT_TEST(testCheckGeoData19);

  CPPUNIT_TEST(testGetGeoLocaleFromItin1);
  CPPUNIT_TEST(testGetGeoLocaleFromItin2);
  CPPUNIT_TEST(testGetGeoLocaleFromItin3);
  CPPUNIT_TEST(testGetGeoLocaleFromItin4);
  CPPUNIT_TEST(testGetGeoLocaleFromItin5);
  CPPUNIT_TEST(testGetGeoLocaleFromItin6);
  CPPUNIT_TEST(testGetGeoLocaleFromItin7);
  CPPUNIT_TEST(testGetGeoLocaleFromItin8);
  CPPUNIT_TEST(testGetGeoLocaleFromItin9);
  CPPUNIT_TEST(testGetGeoLocaleFromItin10);
  CPPUNIT_TEST(testGetGeoLocaleFromItin11);
  CPPUNIT_TEST(testGetGeoLocaleFromItin12);

  CPPUNIT_TEST(testScopeTSIGeo1);
  CPPUNIT_TEST(testScopeTSIGeo3);
  CPPUNIT_TEST(testScopeTSIGeo5);
  CPPUNIT_TEST(testScopeTSIGeo6);
  CPPUNIT_TEST(testScopeTSIGeo7);
  CPPUNIT_TEST(testScopeTSIGeo8);
  CPPUNIT_TEST(testScopeTSIGeo9);
  CPPUNIT_TEST(testScopeTSIGeo10);
  CPPUNIT_TEST(testScopeTSIGeo11);
  CPPUNIT_TEST(testScopeTSIGeo12);
  CPPUNIT_TEST(testScopeTSIGeo13);
  CPPUNIT_TEST(testScopeTSIGeo17);
  CPPUNIT_TEST(testScopeTSIGeo18);
  CPPUNIT_TEST(testScopeTSIGeo19);
  CPPUNIT_TEST(testScopeTSIGeo20);
  CPPUNIT_TEST(testScopeTSIGeo21);

  CPPUNIT_TEST(testScopeTSIGeo24);
  CPPUNIT_TEST(testScopeTSIGeo25);
  CPPUNIT_TEST(testScopeTSIGeo26);
  CPPUNIT_TEST(testScopeTSIGeo27);
  CPPUNIT_TEST(testScopeTSIGeo28);
  CPPUNIT_TEST(testScopeTSIGeo29);
  CPPUNIT_TEST(testScopeTSIGeo31);
  CPPUNIT_TEST(testScopeTSIGeo32);
  CPPUNIT_TEST(testScopeTSIGeo33);
  CPPUNIT_TEST(testScopeTSIGeo34);
  CPPUNIT_TEST(testScopeTSIGeo37);
  CPPUNIT_TEST(testScopeTSIGeo39);
  CPPUNIT_TEST(testScopeTSIGeo40);
  CPPUNIT_TEST(testScopeTSIGeo41);
  CPPUNIT_TEST(testScopeTSIGeo46);
  CPPUNIT_TEST(testScopeTSIGeo47);
  CPPUNIT_TEST(testScopeTSIGeo55);
  CPPUNIT_TEST(testScopeTSIGeo56);
  CPPUNIT_TEST(testScopeTSIGeo59);
  CPPUNIT_TEST(testScopeTSIGeo60);
  CPPUNIT_TEST(testScopeTSIGeo61);
  CPPUNIT_TEST(testScopeTSIGeo63);
  CPPUNIT_TEST(testScopeTSIGeo64);
  CPPUNIT_TEST(testScopeTSIGeo66);
  CPPUNIT_TEST(testScopeTSIGeo73);
  CPPUNIT_TEST(testScopeTSIGeo75);
  CPPUNIT_TEST(testScopeTSIGeo82);
  CPPUNIT_TEST(testScopeTSIGeo83);
  CPPUNIT_TEST(testScopeTSIGeo85);
  CPPUNIT_TEST(testScopeTSIGeo86);
  CPPUNIT_TEST(testScopeTSIGeo87);
  CPPUNIT_TEST(testScopeTSIGeo88);
  CPPUNIT_TEST(testScopeTSIGeo89);

  CPPUNIT_TEST(testScopeTSIGeo_CategoryScopeOverrides);

  CPPUNIT_TEST(testFindBtwTvlSegs);

  CPPUNIT_TEST(testMatchGeo1);
  CPPUNIT_TEST(testMatchGeo2);
  CPPUNIT_TEST(testMatchGeo3);

  CPPUNIT_TEST(
      testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginEqualToDestination);
  CPPUNIT_TEST(
      testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginNotEqualToDestinationPricingUnitOpenJaw);
  CPPUNIT_TEST(
      testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginNotEqualToDestinationPricingUnitNotOpenJaw);
  CPPUNIT_TEST(testSetupTravelSegMarkupSubJourneyScopeNoTurnAroundPoint);
  CPPUNIT_TEST(testSetupTravelSegMarkupJourneyScope);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getRequest()->ticketingDT() = DateTime::localTime();

    // Create Locations
    //
    sfo = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    dal = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDAL.xml");
    sjc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJC.xml");
    jfk = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    nyc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    bos = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    lga = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLGA.xml");
    lax = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    iah = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocIAH.xml");
    mel = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    syd = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    hkg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    nrt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    yyz = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYYZ.xml");
    yvr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    lhr = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");
    gig = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");
    hnl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHNL.xml");
    stt = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSTT.xml");
    anc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocANC.xml");
    sju = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSJU.xml");
    cdg = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");
    mex = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    lon = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    tul = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
    man = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMAN.xml");
    pap = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAP.xml");
    yvi = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVI.xml");
    hav = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHAV.xml");
    dus = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUS.xml");
    fra = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    sin = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSIN.xml");
    atl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    jnb = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJNB.xml");
  }

  void tearDown() { _memHandle.clear(); }

  // Helper methods
  AirSeg* createAirSeg(int16_t pnrSegment,
                       int16_t segmentOrder,
                       const Loc* origin,
                       const Loc* destination,
                       bool stopOver)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->pnrSegment() = pnrSegment;
    airSeg->segmentOrder() = segmentOrder;
    airSeg->origin() = origin;
    airSeg->destination() = destination;
    airSeg->stopOver() = stopOver;
    return airSeg;
  }

  AirSeg* createAirSeg(int16_t pnrSegment,
                       int16_t segmentOrder,
                       const Loc* origin,
                       const Loc* destination,
                       bool stopOver,
                       std::string carrier,
                       int16_t flightNumber)
  {
    AirSeg* airSeg = createAirSeg(pnrSegment, segmentOrder, origin, destination, stopOver);
    airSeg->carrier() = carrier;
    airSeg->flightNumber() = flightNumber;
    return airSeg;
  }

  FareUsage* buildBasicFareUsage(GlobalDirection direction, bool isInBound)
  {
    FareUsage* fu = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();

    fareInfo->_globalDirection = direction;
    fu->inbound() = isInBound;

    fare->setFareInfo(fareInfo);
    ptf->setFare(fare);
    fu->paxTypeFare() = ptf;
    return fu;
  }

  Itin* createItin1(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1, 1, sfo, dfw, false);
    AirSeg* dfw_jfk = createAirSeg(2, 2, dfw, jfk, false);
    AirSeg* jfk_dfw = createAirSeg(3, 3, jfk, dfw, false);
    AirSeg* dfw_sfo = createAirSeg(4, 4, dfw, sfo, false);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::US, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::US, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::US, true);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::US, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu3->travelSeg().push_back(jfk_dfw);
    fu4->travelSeg().push_back(dfw_sfo);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu4);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu3);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(dfw_jfk);
    pu2->travelSeg().push_back(jfk_dfw);

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_dfw);
    fm2->travelSeg().push_back(dfw_sfo);

    fm1->setGlobalDirection(GlobalDirection::US);
    fm2->setGlobalDirection(GlobalDirection::US);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin2(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    AirSeg* mex_dfw = createAirSeg(1, 1, mex, dfw, false);
    AirSeg* dfw_jfk = createAirSeg(2, 2, dfw, jfk, false);
    AirSeg* jfk_lhr = createAirSeg(3, 3, jfk, lhr, false);
    AirSeg* lhr_cdg = createAirSeg(4, 4, lhr, cdg, true);
    AirSeg* cdg_lhr = createAirSeg(5, 5, cdg, lhr, true);
    AirSeg* lhr_jfk = createAirSeg(6, 6, lhr, jfk, true);
    AirSeg* jfk_dfw = createAirSeg(7, 7, jfk, dfw, false);
    AirSeg* dfw_mex = createAirSeg(8, 8, dfw, mex, false);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(mex_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_mex);

    trx.travelSeg().push_back(mex_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::US, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::AT, false);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu5 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu6 = buildBasicFareUsage(GlobalDirection::AT, true);
    FareUsage* fu7 = buildBasicFareUsage(GlobalDirection::US, true);
    FareUsage* fu8 = buildBasicFareUsage(GlobalDirection::ZZ, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(mex_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu3->travelSeg().push_back(jfk_lhr);
    fu4->travelSeg().push_back(lhr_cdg);
    fu5->travelSeg().push_back(cdg_lhr);
    fu6->travelSeg().push_back(lhr_jfk);
    fu7->travelSeg().push_back(jfk_dfw);
    fu8->travelSeg().push_back(dfw_mex);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu8);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu3);
    pu2->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu7);
    pu3->fareUsage().push_back(fu4);
    pu3->fareUsage().push_back(fu5);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_dfw);
    pu1->travelSeg().push_back(dfw_mex);
    pu2->travelSeg().push_back(dfw_jfk);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(jfk_dfw);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = dfw_mex;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);
    fp->pricingUnit().push_back(pu3);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    FareMarket* fm3 = _memHandle.create<FareMarket>();
    FareMarket* fm4 = _memHandle.create<FareMarket>();
    FareMarket* fm5 = _memHandle.create<FareMarket>();
    FareMarket* fm6 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_mex);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::AT);
    fm6->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);
    itin->fareMarket().push_back(fm5);
    itin->fareMarket().push_back(fm6);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin3(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    // Create the travel segments
    //
    AirSeg* stt_mia = createAirSeg(1, 1, stt, mia, false, "AA", 123);
    AirSeg* mia_yyz = createAirSeg(2, 2, mia, yyz, false, "AA", 234);
    AirSeg* yyz_jfk = createAirSeg(3, 3, yyz, jfk, false, "AC", 555);
    AirSeg* jfk_lhr = createAirSeg(4, 4, jfk, lhr, true, "AA", 789);
    AirSeg* lhr_cdg = createAirSeg(5, 5, lhr, cdg, true, "BA", 888);
    AirSeg* cdg_lhr = createAirSeg(6, 6, cdg, lhr, true, "AF", 456);
    AirSeg* lhr_jfk = createAirSeg(7, 7, lhr, jfk, true, "AA", 987);
    AirSeg* jfk_dfw = createAirSeg(8, 8, jfk, dfw, true, "AA", 222);
    AirSeg* dfw_mia = createAirSeg(9, 9, dfw, mia, false, "AA", 1212);
    AirSeg* mia_stt = createAirSeg(10, 10, mia, stt, false, "AA", 3434);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(stt_mia);
    itin->travelSeg().push_back(mia_yyz);
    itin->travelSeg().push_back(yyz_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_mia);
    itin->travelSeg().push_back(mia_stt);

    // Set the turn around point for the Journey
    //
    itin->furthestPointSegmentOrder() = 6;

    trx.travelSeg().push_back(stt_mia);
    trx.travelSeg().push_back(mia_yyz);
    trx.travelSeg().push_back(yyz_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_mia);
    trx.travelSeg().push_back(mia_stt);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::AT, false);
    FareUsage* fu5 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu6 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu7 = buildBasicFareUsage(GlobalDirection::AT, true);
    FareUsage* fu8 = buildBasicFareUsage(GlobalDirection::US, true);
    FareUsage* fu9 = buildBasicFareUsage(GlobalDirection::US, true);
    FareUsage* fu10 = buildBasicFareUsage(GlobalDirection::PV, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(stt_mia);
    fu2->travelSeg().push_back(mia_yyz);
    fu3->travelSeg().push_back(yyz_jfk);
    fu4->travelSeg().push_back(jfk_lhr);
    fu5->travelSeg().push_back(lhr_cdg);
    fu6->travelSeg().push_back(cdg_lhr);
    fu7->travelSeg().push_back(lhr_jfk);
    fu8->travelSeg().push_back(jfk_dfw);
    fu9->travelSeg().push_back(dfw_mia);
    fu10->travelSeg().push_back(mia_stt);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu10);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu3);
    pu2->fareUsage().push_back(fu4);
    pu2->fareUsage().push_back(fu7);
    pu2->fareUsage().push_back(fu8);
    pu2->fareUsage().push_back(fu9);
    pu3->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu6);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(stt_mia);
    pu1->travelSeg().push_back(mia_stt);

    pu2->travelSeg().push_back(mia_yyz);
    pu2->travelSeg().push_back(yyz_jfk);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(jfk_dfw);
    pu2->travelSeg().push_back(dfw_mia);

    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = mia_stt;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);
    fp->pricingUnit().push_back(pu3);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    FareMarket* fm3 = _memHandle.create<FareMarket>();
    FareMarket* fm4 = _memHandle.create<FareMarket>();
    FareMarket* fm5 = _memHandle.create<FareMarket>();
    FareMarket* fm6 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(stt_mia);
    fm2->travelSeg().push_back(mia_yyz);
    fm2->travelSeg().push_back(yyz_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm5->travelSeg().push_back(dfw_mia);
    fm6->travelSeg().push_back(mia_stt);

    fm1->setGlobalDirection(GlobalDirection::PV);
    fm2->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::AT);
    fm6->setGlobalDirection(GlobalDirection::PV);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);
    itin->fareMarket().push_back(fm5);
    itin->fareMarket().push_back(fm6);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin4(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    // Create the travel segments
    //
    AirSeg* stt_mia = createAirSeg(1, 1, stt, mia, false);
    AirSeg* mia_yyz = createAirSeg(2, 2, mia, yyz, true);
    AirSeg* yyz_sfo = createAirSeg(3, 3, yyz, sfo, true);
    AirSeg* sfo_hkg = createAirSeg(4, 4, sfo, hkg, true);
    AirSeg* hkg_nrt = createAirSeg(5, 5, hkg, nrt, true);
    AirSeg* nrt_yvr = createAirSeg(6, 6, nrt, yvr, true);
    AirSeg* yvr_yyz = createAirSeg(7, 7, yvr, yyz, true);
    AirSeg* yyz_dfw = createAirSeg(8, 8, yyz, dfw, false);
    AirSeg* dfw_mia = createAirSeg(9, 9, dfw, mia, false);
    AirSeg* mia_stt = createAirSeg(10, 10, mia, stt, false);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(stt_mia);
    itin->travelSeg().push_back(mia_yyz);
    itin->travelSeg().push_back(yyz_sfo);
    itin->travelSeg().push_back(sfo_hkg);
    itin->travelSeg().push_back(hkg_nrt);
    itin->travelSeg().push_back(nrt_yvr);
    itin->travelSeg().push_back(yvr_yyz);
    itin->travelSeg().push_back(yyz_dfw);
    itin->travelSeg().push_back(dfw_mia);
    itin->travelSeg().push_back(mia_stt);

    // Set the turn around point for the Journey
    //
    itin->furthestPointSegmentOrder() = 5;

    trx.travelSeg().push_back(stt_mia);
    trx.travelSeg().push_back(mia_yyz);
    trx.travelSeg().push_back(yyz_sfo);
    trx.travelSeg().push_back(sfo_hkg);
    trx.travelSeg().push_back(hkg_nrt);
    trx.travelSeg().push_back(nrt_yvr);
    trx.travelSeg().push_back(yvr_yyz);
    trx.travelSeg().push_back(yyz_dfw);
    trx.travelSeg().push_back(dfw_mia);
    trx.travelSeg().push_back(mia_stt);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::NP, false);
    FareUsage* fu5 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu6 = buildBasicFareUsage(GlobalDirection::NP, true);
    FareUsage* fu7 = buildBasicFareUsage(GlobalDirection::CA, true);
    FareUsage* fu8 = buildBasicFareUsage(GlobalDirection::PV, true);
    FareUsage* fu9 = buildBasicFareUsage(GlobalDirection::US, true);
    FareUsage* fu10 = buildBasicFareUsage(GlobalDirection::PV, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(stt_mia);
    fu2->travelSeg().push_back(mia_yyz);
    fu3->travelSeg().push_back(yyz_sfo);
    fu4->travelSeg().push_back(sfo_hkg);
    fu5->travelSeg().push_back(hkg_nrt);
    fu6->travelSeg().push_back(nrt_yvr);
    fu7->travelSeg().push_back(yvr_yyz);
    fu8->travelSeg().push_back(yyz_dfw);
    fu9->travelSeg().push_back(dfw_mia);
    fu10->travelSeg().push_back(mia_stt);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->fareUsage().push_back(fu8);
    pu1->fareUsage().push_back(fu9);
    pu1->fareUsage().push_back(fu10);

    pu2->fareUsage().push_back(fu3);
    pu2->fareUsage().push_back(fu4);
    pu2->fareUsage().push_back(fu5);
    pu2->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu7);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(stt_mia);
    pu1->travelSeg().push_back(mia_yyz);
    pu1->travelSeg().push_back(yyz_dfw);
    pu1->travelSeg().push_back(dfw_mia);
    pu1->travelSeg().push_back(mia_stt);

    pu2->travelSeg().push_back(yyz_sfo);
    pu2->travelSeg().push_back(sfo_hkg);
    pu2->travelSeg().push_back(hkg_nrt);
    pu2->travelSeg().push_back(nrt_yvr);
    pu2->travelSeg().push_back(yvr_yyz);

    pu1->turnAroundPoint() = yyz_dfw;
    pu2->turnAroundPoint() = hkg_nrt;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    FareMarket* fm3 = _memHandle.create<FareMarket>();
    FareMarket* fm4 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(stt_mia);
    fm1->travelSeg().push_back(mia_yyz);
    fm2->travelSeg().push_back(yyz_dfw);
    fm2->travelSeg().push_back(dfw_mia);
    fm2->travelSeg().push_back(mia_stt);
    fm3->travelSeg().push_back(yyz_sfo);
    fm3->travelSeg().push_back(sfo_hkg);
    fm4->travelSeg().push_back(hkg_nrt);
    fm4->travelSeg().push_back(nrt_yvr);
    fm4->travelSeg().push_back(yvr_yyz);

    fm1->setGlobalDirection(GlobalDirection::PV);
    fm2->setGlobalDirection(GlobalDirection::PV);
    fm3->setGlobalDirection(GlobalDirection::NP);
    fm4->setGlobalDirection(GlobalDirection::NP);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin5(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    // Create the travel segments
    //
    AirSeg* jfk_lhr = createAirSeg(1, 1, jfk, lhr, true);
    AirSeg* lhr_cdg = createAirSeg(2, 2, lhr, cdg, true);
    AirSeg* cdg_lhr = createAirSeg(3, 3, cdg, lhr, true);
    AirSeg* lhr_jfk = createAirSeg(4, 4, lhr, jfk, false);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);

    // Set the turn around point for the Journey
    //
    itin->furthestPointSegmentOrder() = 3;

    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::ZZ, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);

    pu1->turnAroundPoint() = cdg_lhr;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);

    fm1->setGlobalDirection(ZZ);
    fm2->setGlobalDirection(ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin6(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    // Create the travel segments
    //
    AirSeg* stt_mia = createAirSeg(1, 1, stt, mia, false, "AA", 1234);
    AirSeg* mia_yyz = createAirSeg(2, 2, mia, yyz, false, "AA", 5678);
    AirSeg* yyz_jfk = createAirSeg(3, 3, yyz, jfk, false, "AC", 1111);
    AirSeg* jfk_lhr = createAirSeg(4, 4, jfk, lhr, false, "AA", 2222);
    AirSeg* lhr_cdg = createAirSeg(5, 5, lhr, cdg, false, "BA", 5555);
    AirSeg* cdg_lhr = createAirSeg(6, 6, cdg, lhr, false, "AF", 4444);
    AirSeg* lhr_jfk = createAirSeg(7, 7, lhr, jfk, false, "BA", 6666);
    AirSeg* jfk_dfw = createAirSeg(8, 8, jfk, dfw, false, "AA", 7777);
    AirSeg* dfw_mia = createAirSeg(9, 9, dfw, mia, false, "AA", 7777);
    AirSeg* mia_stt = createAirSeg(10, 10, mia, stt, false, "AA", 8888);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(stt_mia);
    itin->travelSeg().push_back(mia_yyz);
    itin->travelSeg().push_back(yyz_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_mia);
    itin->travelSeg().push_back(mia_stt);

    // Set the turn around point for the Journey
    //
    itin->furthestPointSegmentOrder() = 6;

    trx.travelSeg().push_back(stt_mia);
    trx.travelSeg().push_back(mia_yyz);
    trx.travelSeg().push_back(yyz_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_mia);
    trx.travelSeg().push_back(mia_stt);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::PV, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::AT, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu5 = buildBasicFareUsage(GlobalDirection::AT, true);
    FareUsage* fu6 = buildBasicFareUsage(GlobalDirection::PV, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(stt_mia);
    fu2->travelSeg().push_back(mia_yyz);
    fu2->travelSeg().push_back(yyz_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu5->travelSeg().push_back(dfw_mia);
    fu6->travelSeg().push_back(mia_stt);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu6);
    pu2->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu3);
    pu3->fareUsage().push_back(fu4);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(stt_mia);
    pu1->travelSeg().push_back(mia_stt);

    pu2->travelSeg().push_back(mia_yyz);
    pu2->travelSeg().push_back(yyz_jfk);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu2->travelSeg().push_back(jfk_dfw);
    pu2->travelSeg().push_back(dfw_mia);

    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = mia_stt;
    pu2->turnAroundPoint() = lhr_jfk;
    pu3->turnAroundPoint() = cdg_lhr;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);
    fp->pricingUnit().push_back(pu3);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    FareMarket* fm3 = _memHandle.create<FareMarket>();
    FareMarket* fm4 = _memHandle.create<FareMarket>();
    FareMarket* fm5 = _memHandle.create<FareMarket>();
    FareMarket* fm6 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(stt_mia);
    fm2->travelSeg().push_back(mia_yyz);
    fm2->travelSeg().push_back(yyz_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm5->travelSeg().push_back(dfw_mia);
    fm6->travelSeg().push_back(mia_stt);

    fm1->setGlobalDirection(GlobalDirection::PV);
    fm2->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::AT);
    fm6->setGlobalDirection(GlobalDirection::PV);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);
    itin->fareMarket().push_back(fm5);
    itin->fareMarket().push_back(fm6);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin7(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    // Create the travel segments
    //
    // Create the travel segments
    //
    AirSeg* stt_mia = createAirSeg(1, 1, stt, mia, false, "AA", 1234);
    AirSeg* mia_yyz = createAirSeg(2, 2, mia, yyz, false, "AA", 5678);
    AirSeg* yyz_anc = createAirSeg(3, 3, yyz, anc, false, "AC", 1111);
    AirSeg* anc_dfw = createAirSeg(4, 4, anc, dfw, false, "AA", 2222);
    AirSeg* dfw_mex = createAirSeg(5, 5, dfw, mex, false, "BA", 5555);
    AirSeg* mex_pap = createAirSeg(6, 6, mex, pap, false, "AA", 4444);
    AirSeg* pap_yvi = createAirSeg(7, 7, pap, yvi, false, "BA", 6666);
    AirSeg* yvi_gig = createAirSeg(8, 8, yvi, gig, false, "AA", 7777);
    AirSeg* gig_hav = createAirSeg(9, 9, gig, hav, false, "AA", 7777);
    AirSeg* hav_stt = createAirSeg(10, 10, hav, stt, false, "AA", 8888);

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(stt_mia);
    itin->travelSeg().push_back(mia_yyz);
    itin->travelSeg().push_back(yyz_anc);
    itin->travelSeg().push_back(anc_dfw);
    itin->travelSeg().push_back(dfw_mex);
    itin->travelSeg().push_back(mex_pap);
    itin->travelSeg().push_back(pap_yvi);
    itin->travelSeg().push_back(yvi_gig);
    itin->travelSeg().push_back(gig_hav);
    itin->travelSeg().push_back(hav_stt);

    // Set the turn around point for the Journey
    //
    itin->furthestPointSegmentOrder() = 8;

    trx.travelSeg().push_back(stt_mia);
    trx.travelSeg().push_back(mia_yyz);
    trx.travelSeg().push_back(yyz_anc);
    trx.travelSeg().push_back(anc_dfw);
    trx.travelSeg().push_back(dfw_mex);
    trx.travelSeg().push_back(mex_pap);
    trx.travelSeg().push_back(pap_yvi);
    trx.travelSeg().push_back(yvi_gig);
    trx.travelSeg().push_back(gig_hav);
    trx.travelSeg().push_back(hav_stt);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu2 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu3 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu4 = buildBasicFareUsage(GlobalDirection::ZZ, false);
    FareUsage* fu5 = buildBasicFareUsage(GlobalDirection::ZZ, true);
    FareUsage* fu6 = buildBasicFareUsage(GlobalDirection::ZZ, true);

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(stt_mia);
    fu1->travelSeg().push_back(mia_yyz);
    fu1->travelSeg().push_back(yyz_anc);
    fu2->travelSeg().push_back(anc_dfw);
    fu2->travelSeg().push_back(dfw_mex);
    fu3->travelSeg().push_back(mex_pap);
    fu3->travelSeg().push_back(pap_yvi);
    fu4->travelSeg().push_back(yvi_gig);
    fu5->travelSeg().push_back(gig_hav);
    fu6->travelSeg().push_back(hav_stt);

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu2->fareUsage().push_back(fu3);
    pu2->fareUsage().push_back(fu4);
    pu3->fareUsage().push_back(fu5);
    pu3->fareUsage().push_back(fu6);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(stt_mia);
    pu1->travelSeg().push_back(mia_yyz);
    pu1->travelSeg().push_back(yyz_anc);
    pu1->travelSeg().push_back(anc_dfw);
    pu1->travelSeg().push_back(dfw_mex);

    pu2->travelSeg().push_back(mex_pap);
    pu2->travelSeg().push_back(pap_yvi);
    pu2->travelSeg().push_back(yvi_gig);

    pu3->travelSeg().push_back(gig_hav);
    pu3->travelSeg().push_back(hav_stt);

    pu1->turnAroundPoint() = anc_dfw;
    pu2->turnAroundPoint() = yvi_gig;
    pu3->turnAroundPoint() = hav_stt;

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);
    fp->pricingUnit().push_back(pu3);

    // Attach the fare path to the itin
    //
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    //
    fp->itin() = itin;

    // Create the FareMarkets
    //
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    FareMarket* fm3 = _memHandle.create<FareMarket>();
    FareMarket* fm4 = _memHandle.create<FareMarket>();
    FareMarket* fm5 = _memHandle.create<FareMarket>();
    FareMarket* fm6 = _memHandle.create<FareMarket>();

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(stt_mia);
    fm1->travelSeg().push_back(mia_yyz);
    fm1->travelSeg().push_back(yyz_anc);
    fm2->travelSeg().push_back(anc_dfw);
    fm2->travelSeg().push_back(dfw_mex);
    fm3->travelSeg().push_back(mex_pap);
    fm3->travelSeg().push_back(pap_yvi);
    fm4->travelSeg().push_back(yvi_gig);
    fm5->travelSeg().push_back(gig_hav);
    fm6->travelSeg().push_back(hav_stt);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::ZZ);
    fm6->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);
    itin->fareMarket().push_back(fm5);
    itin->fareMarket().push_back(fm6);

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  PricingUnit* createOpenJawPricingUnit()
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    pu->fareUsage().push_back(fu1);
    pu->fareUsage().push_back(fu2);

    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    fi1->_globalDirection = GlobalDirection::US;
    Fare* f1 = _memHandle.create<Fare>();
    f1->setFareInfo(fi1);
    ptf1->setFare(f1);
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf1;

    AirSeg* tsMiaLax = createAirSeg(1, 1, mia, lax, false);
    AirSeg* tsDfwMia = createAirSeg(2, 2, dfw, mia, false);

    pu->travelSeg().push_back(tsMiaLax);
    pu->travelSeg().push_back(tsDfwMia);
    pu->puType() = PricingUnit::Type::OPENJAW;
    fu1->travelSeg().push_back(tsMiaLax);
    fu2->travelSeg().push_back(tsDfwMia);

    pu->turnAroundPoint() = tsDfwMia;

    return pu;
  }

  RuleUtil::TSITravelSegMarkup
  runSetupTravelSegMarkupSubJourney(PricingUnit::Type pricingUnitType, bool isOriginEqualToDestination)
  {
    PricingUnit* pu = createOpenJawPricingUnit(); // MIALAX - DFWMIA
    if (isOriginEqualToDestination)
      pu->travelSeg().back()->origin() = lax;
    pu->puType() = pricingUnitType; // PricingUnit::Type::ROUNDTRIP;

    if (pricingUnitType == PricingUnit::Type::ONEWAY)
      pu->turnAroundPoint() = NULL;

    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;
    tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD;
    tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);

    // Create a TSIData object.
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, ATPCO_VENDOR_CODE, 0, 0, pu, 0);
    RuleUtil::TSITravelSegMarkupContainer tsMarkup;
    RuleUtil::setupTravelSegMarkup(*_trx, tsiData, 0, tsMarkup);
    return tsMarkup.front();
  }

  RuleUtil::TSITravelSegMarkup runSetupTravelSegMarkupJourney()
  {
    Itin* itin = createItin4(*_trx);
    FarePath* fp = itin->farePath()[0];

    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;
    tsiInfo.loopDirection() = RuleConst::TSI_LOOP_BACKWARD;
    tsiInfo.matchCriteria().push_back(TSIInfo::OVER_WATER);
    tsiInfo.matchCriteria().push_back(TSIInfo::INTERNATIONAL);

    // Create a TSIData object.
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, ATPCO_VENDOR_CODE, fp, itin, 0, 0);
    RuleUtil::TSITravelSegMarkupContainer tsMarkup;
    RuleUtil::setupTravelSegMarkup(*_trx, tsiData, 0, tsMarkup);
    return tsMarkup.front();
  }

  void testValidateGeoRuleItem1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().activate();

    const uint32_t itemNo = 1;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = sfo;
    const Loc* loc2 = dfw;
    const Loc* loc3 = jfk;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    AirSeg air2;
    air2.pnrSegment() = 2;
    air2.segmentOrder() = 2;
    air2.origin() = loc2;
    air2.destination() = loc3;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);
    _trx->travelSeg().push_back(&air2);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    fm.origin() = loc1;
    fm.destination() = loc3;

    Itin itin;
    itin.travelSeg().push_back(&air1);
    itin.travelSeg().push_back(&air2);

    FarePath fp;
    fp.itin() = &itin;

    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 1",
                           applTravelSegment.size() == 1 && applTravelSegment[0]->destMatch() &&
                               applTravelSegment[0]->travelSeg()->destination()->loc() == "JFK");
  }

  void testValidateGeoRuleItem2()
  {
    const uint32_t itemNo = 1;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 2",
                           applTravelSegment.size() == 1 && applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->travelSeg()->origin()->loc() == "JFK");
  }

  void testValidateGeoRuleItem3()
  {
    const uint32_t itemNo = 1;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 3", !applTravelSegment.empty());
  }

  void testValidateGeoRuleItem4()
  {
    const uint32_t itemNo = 1;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 4", applTravelSegment.empty());
  }

  void testValidateGeoRuleItem5()
  {
    const uint32_t itemNo = 28;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 5", applTravelSegment.empty());
  }

  void testValidateGeoRuleItem6()
  {
    const uint32_t itemNo = 28;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 6", applTravelSegment.empty());
  }

  void testValidateGeoRuleItem7()
  {
    const uint32_t itemNo = 28;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 7", applTravelSegment.empty());
  }

  void testValidateGeoRuleItem8()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = nyc;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 8", !applTravelSegment.empty());
  }

  void testValidateGeoRuleItem9()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = nyc;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 9", !applTravelSegment.empty());
  }

  void testValidateGeoRuleItem10()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = nyc;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 10", !applTravelSegment.empty());
  }

  void testValidateGeoRuleItem11()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 11",
                           applTravelSegment.size() == 1 && applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->travelSeg()->origin()->loc() == "JFK");
  }

  void testValidateGeoRuleItem12()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = false;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 12", !applTravelSegment.empty());
  }

  void testValidateGeoRuleItem13()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  itin.farePath()[0],
                                  0,
                                  &pu,
                                  &fm,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 13", applTravelSegment.empty());
  }

  void testValidateGeoRuleItem14()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    bool result = RuleUtil::validateGeoRuleItem(itemNo,
                                                vendorCode,
                                                RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                                false,
                                                false,
                                                false,
                                                *_trx,
                                                0,
                                                0,
                                                &pu,
                                                &fm,
                                                ticketingDate,
                                                applTravelSegment,
                                                origCheck,
                                                destCheck,
                                                fltStopCheck,
                                                tsiReturn,
                                                loc1Return,
                                                loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 14", !result);
  }

  void testValidateGeoRuleItem15()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    bool result = RuleUtil::validateGeoRuleItem(itemNo,
                                                vendorCode,
                                                RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                                false,
                                                false,
                                                false,
                                                *_trx,
                                                itin.farePath()[0],
                                                0,
                                                0,
                                                &fm,
                                                ticketingDate,
                                                applTravelSegment,
                                                origCheck,
                                                destCheck,
                                                fltStopCheck,
                                                tsiReturn,
                                                loc1Return,
                                                loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 15", !result);
  }

  void testValidateGeoRuleItem16()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = false;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    // call buildFareMarket
    //
    //    TrxUtil::buildFareMarket(trx, trx.travelSeg());

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    //

    bool result = RuleUtil::validateGeoRuleItem(itemNo,
                                                vendorCode,
                                                RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                                false,
                                                false,
                                                false,
                                                *_trx,
                                                itin.farePath()[0],
                                                0,
                                                &pu,
                                                0,
                                                ticketingDate,
                                                applTravelSegment,
                                                origCheck,
                                                destCheck,
                                                fltStopCheck,
                                                tsiReturn,
                                                loc1Return,
                                                loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 16", !result);
  }

  void testValidateGeoRuleItem17()
  {
    const uint32_t itemNo = 219;
    VendorCode vendorCode = "ATP";
    const PricingUnit pu;
    const FareMarket fm;
    const DateTime ticketingDate;
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    bool origCheck = true;
    bool destCheck = true;
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey loc1Return;
    LocKey loc2Return;

    // create the Locations
    //
    const Loc* loc1 = jfk;
    const Loc* loc2 = lhr;

    // create the travel segments
    //
    AirSeg air1;
    air1.pnrSegment() = 1;
    air1.segmentOrder() = 1;
    air1.origin() = loc1;
    air1.destination() = loc2;

    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&air1);

    Itin itin;
    itin.travelSeg().push_back(&air1);

    FarePath fp;
    fp.itin() = &itin;
    itin.farePath().push_back(&fp);

    _trx->itin().push_back(&itin);

    RuleUtil::validateGeoRuleItem(itemNo,
                                  vendorCode,
                                  RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                  false,
                                  false,
                                  false,
                                  *_trx,
                                  0,
                                  &itin,
                                  0,
                                  0,
                                  ticketingDate,
                                  applTravelSegment,
                                  origCheck,
                                  destCheck,
                                  fltStopCheck,
                                  tsiReturn,
                                  loc1Return,
                                  loc2Return);

    // test to see that we have a valid Geo Rule Item
    //
    CPPUNIT_ASSERT_MESSAGE("Error in validateGeoRuleItem 17",
                           applTravelSegment.size() == 1 && applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->travelSeg()->origin()->loc() == "JFK");
  }

  void testGetTSIScopeFromGeoRuleItem1()
  {
    RuleConst::TSIScopeType scope;
    bool result = RuleUtil::getTSIScopeFromGeoRuleItem(123, "ATP", *_trx, scope);
    CPPUNIT_ASSERT_MESSAGE("Error in testGetTSIScopeFromGeoRuleItem 1", result);
  }

  void testTable995WhollyWithin1()
  {
    CPPUNIT_ASSERT_MESSAGE("Error in table995WhollyWithin: Test 1",
                           RuleUtil::table995WhollyWithin(*_trx, 3558, 3894, "ATP"));
  }

  void testTable995WhollyWithin2()
  {
    CPPUNIT_ASSERT_MESSAGE("Error in table995WhollyWithin: Test 2",
                           !RuleUtil::table995WhollyWithin(*_trx, 19, 20, "ATP"));
  }

  void testCheckGeoNotType1()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 1", !result);
  }

  void testCheckGeoNotType2()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 2", !result);
  }

  void testCheckGeoNotType3()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 3", !result);
  }

  void testCheckGeoNotType4()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 4", result);
  }

  void testCheckGeoNotType5()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_AREA;
    tsiData.locType2() = LOCTYPE_SUBAREA;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 5", result);
  }

  void testCheckGeoNotType6()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_NATION;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 6", result);
  }

  void testCheckGeoNotType7()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check CITY
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_CITY;

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_ZONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 7", result);
  }

  void testCheckGeoNotType8()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 8", !result);
  }

  void testCheckGeoNotType9()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 9", !result);
  }

  void testCheckGeoNotType10()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 10", !result);
  }

  void testCheckGeoNotType11()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 11", !result);
  }

  void testCheckGeoNotType12()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 12", result);
  }

  void testCheckGeoNotType13()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_ZONE;
    tsiData.locType2() = LOCTYPE_AREA;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 13", result);
  }

  void testCheckGeoNotType14()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_SUBAREA;
    tsiData.locType2() = LOCTYPE_NATION;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 14", result);
  }

  void testCheckGeoNotType15()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check BOTH (CITY and AIRPORT)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_BOTH;

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 15", result);
  }

  void testCheckGeoNotType16()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 16", !result);
  }

  void testCheckGeoNotType17()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_ZONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 17", !result);
  }

  void testCheckGeoNotType18()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 18", !result);
  }

  void testCheckGeoNotType19()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_CITY;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 19", !result);
  }

  void testCheckGeoNotType20()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 20", !result);
  }

  void testCheckGeoNotType21()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_ZONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 21", !result);
  }

  void testCheckGeoNotType22()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 22", !result);
  }

  void testCheckGeoNotType23()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_AIRPORT;
    tsiData.locType2() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 23", !result);
  }

  void testCheckGeoNotType24()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_ZONE;
    tsiData.locType2() = LOCTYPE_CITY;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 24", !result);
  }

  void testCheckGeoNotType25()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_ZONE;
    tsiData.locType2() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 25", !result);
  }

  void testCheckGeoNotType26()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_ZONE;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 26", !result);
  }

  void testCheckGeoNotType27()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_ZONE;
    tsiData.locType2() = LOCTYPE_ZONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 27", !result);
  }

  void testCheckGeoNotType28()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_AREA;
    tsiData.locType2() = LOCTYPE_SUBAREA;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 28", result);
  }

  void testCheckGeoNotType29()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NATION;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 29", result);
  }

  void testCheckGeoNotType30()
  {
    // Create a dummy TSIInfo object
    TSIInfo tsiInfo;

    // Create a TSIData object.
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    // We only need to set the GeoNotType field of TSIInfo for this test
    //

    //
    // Check THREE (CITY, AIRPORT and ZONE)
    //
    tsiInfo.geoNotType() = RuleConst::TSI_GEO_NOT_TYPE_THREE;

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    bool result = RuleUtil::checkGeoNotType(tsiData);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoNotType: Test 30", result);
  }

  void testGetGeoData1()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Req" (GeoType and GeoLocale both required)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    //
    // Test error condition... GeoType blank and GeoLocale blank
    //

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 1", !result);
  }

  void testGetGeoData2()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Req" (GeoType and GeoLocale both required)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    //
    // Test error condition... GeoType blank and GeoLocale not blank
    //

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;
    locKey1.loc() = "SFO";
    locKey2.loc() = "DFW";

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 2", !result);
  }

  void testGetGeoData3()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Req" (GeoType and GeoLocale both required)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    //
    // Test error condition... GeoType not blank and GeoLocale blank
    //

    locKey1.locType() = LOCTYPE_AIRPORT;
    locKey2.locType() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 3", !result);
  }

  void testGetGeoData4()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Req" (GeoType and GeoLocale both required)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_BOTH_REQUIRED;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_AIRPORT;
    locKey2.locType() = LOCTYPE_AIRPORT;
    locKey1.loc() = "SFO";
    locKey2.loc() = "DFW";

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 4A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 4B", tsiData.locType1() == locKey1.locType());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 4C", tsiData.locType2() == locKey2.locType());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 4D", tsiData.locCode1() == locKey1.loc());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 4E", tsiData.locCode2() == locKey2.loc());
  }

  void testGetGeoData5()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Type" (GeoType required, GeoLocale is ignored)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_GEOTYPE_REQUIRED;
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    //
    // Test error condition... GeoType blank
    //

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 5", !result);
  }

  void testGetGeoData6()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Type" (GeoType required, GeoLocale is ignored)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_GEOTYPE_REQUIRED;
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    //
    // Test error condition... GeoType blank and GeoLocale not blank
    //

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;
    locKey1.loc() = "SFO";
    locKey2.loc() = "DFW";

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 6", !result);
  }

  void testGetGeoData7()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);

    // Create the fare usages
    FareUsage fu1;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);

    // Set the directionality
    fu1.inbound() = false;

    // Create the pricing units
    PricingUnit pu1;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Type" (GeoType required, GeoLocale ignored)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_GEOTYPE_REQUIRED;
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, &pu1, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_AIRPORT;
    locKey2.locType() = LOCTYPE_AIRPORT;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 7A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 7B",
                           tsiData.locType1() == locKey1.locType() ||
                               tsiData.locType1() == locKey2.locType());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 7C", tsiData.locCode1() == sfo->loc());
  }

  void testGetGeoData8()
  {
    // Create the travel segments
    //
    ArunkSeg sjc_sfo;
    sjc_sfo.pnrSegment() = 1;
    sjc_sfo.segmentOrder() = 1;
    sjc_sfo.origin() = sjc;
    sjc_sfo.destination() = sfo;

    AirSeg sfo_dal;
    sfo_dal.pnrSegment() = 2;
    sfo_dal.segmentOrder() = 2;
    sfo_dal.origin() = sfo;
    sfo_dal.destination() = dal;

    ArunkSeg dal_dfw;
    dal_dfw.pnrSegment() = 3;
    dal_dfw.segmentOrder() = 3;
    dal_dfw.origin() = dal;
    dal_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 4;
    dfw_jfk.segmentOrder() = 4;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sjc_sfo);
    _trx->travelSeg().push_back(&sfo_dal);
    _trx->travelSeg().push_back(&dal_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sjc_sfo);
    fu1.travelSeg().push_back(&sfo_dal);
    fu1.travelSeg().push_back(&dal_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu2);
    pu1.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sjc_sfo);
    pu1.travelSeg().push_back(&sfo_dal);
    pu1.travelSeg().push_back(&dal_dfw);
    pu1.travelSeg().push_back(&dfw_jfk);
    pu1.travelSeg().push_back(&jfk_dfw);

    // Set the turn around point of the pricing unit
    pu1.turnAroundPoint() = &dfw_jfk;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "Get" (Ignore GeoType, get GeoLocale from itinerary)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_GET_FROM_ITIN;
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_TURNAROUND;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, &pu1, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 8A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 8B", LOCTYPE_NATION == tsiData.locType1());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 8C", tsiData.locCode1() == jfk->nation());
  }

  void testGetGeoData9()
  {
    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test GeoRequired = "" (Blank, GeoData not required)
    //
    tsiInfo.geoRequired() = RuleConst::TSI_GEO_BLANK;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NONE;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    bool result = RuleUtil::getGeoData(*_trx, tsiData, locKey1, locKey2);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 9A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 9B", LOCTYPE_NONE == tsiData.locType1());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 9C", LOCTYPE_NONE == tsiData.locType2());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 9D", tsiData.locCode1().empty());

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoData: Test 9E", tsiData.locCode2().empty());
  }

  void testGetGeoLocaleFromItin1()
  {
    Itin itin;

    // Create the travel segments
    //
    ArunkSeg sjc_sfo;
    sjc_sfo.pnrSegment() = 1;
    sjc_sfo.segmentOrder() = 1;
    sjc_sfo.origin() = sjc;
    sjc_sfo.destination() = sfo;

    AirSeg sfo_dal;
    sfo_dal.pnrSegment() = 2;
    sfo_dal.segmentOrder() = 2;
    sfo_dal.origin() = sfo;
    sfo_dal.destination() = dal;

    ArunkSeg dal_dfw;
    dal_dfw.pnrSegment() = 3;
    dal_dfw.segmentOrder() = 3;
    dal_dfw.origin() = dal;
    dal_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 4;
    dfw_jfk.segmentOrder() = 4;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sjc;
    dfw_sjc.pnrSegment() = 6;
    dfw_sjc.segmentOrder() = 6;
    dfw_sjc.origin() = dfw;
    dfw_sjc.destination() = sjc;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sjc_sfo);
    _trx->travelSeg().push_back(&sfo_dal);
    _trx->travelSeg().push_back(&dal_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sjc);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sjc_sfo);
    fu1.travelSeg().push_back(&sfo_dal);
    fu1.travelSeg().push_back(&dal_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sjc);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sjc_sfo);
    pu1.travelSeg().push_back(&sfo_dal);
    pu1.travelSeg().push_back(&dal_dfw);
    pu1.travelSeg().push_back(&dfw_sjc);

    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sjc;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sjc_sfo);
    itin.travelSeg().push_back(&sfo_dal);
    itin.travelSeg().push_back(&dal_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sjc);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 5;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, &fp, &itin, 0, 0);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 1A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 1B", loc->loc() == sjc->loc());

    // Same test but using only the Itin
    //
    RuleUtil::TSIData tsiData1(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, &itin, 0, 0);
    loc = 0;

    result = RuleUtil::getGeoLocaleFromItin(tsiData1, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 1C", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 1D", loc->loc() == sjc->loc());
  }

  void testGetGeoLocaleFromItin2()
  {
    // Create the travel segments
    //
    ArunkSeg sjc_sfo;
    sjc_sfo.pnrSegment() = 1;
    sjc_sfo.segmentOrder() = 1;
    sjc_sfo.origin() = sjc;
    sjc_sfo.destination() = sfo;

    AirSeg sfo_dal;
    sfo_dal.pnrSegment() = 2;
    sfo_dal.segmentOrder() = 2;
    sfo_dal.origin() = sfo;
    sfo_dal.destination() = dal;

    ArunkSeg dal_dfw;
    dal_dfw.pnrSegment() = 3;
    dal_dfw.segmentOrder() = 3;
    dal_dfw.origin() = dal;
    dal_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 4;
    dfw_jfk.segmentOrder() = 4;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sjc;
    dfw_sjc.pnrSegment() = 6;
    dfw_sjc.segmentOrder() = 6;
    dfw_sjc.origin() = dfw;
    dfw_sjc.destination() = sjc;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sjc_sfo);
    _trx->travelSeg().push_back(&sfo_dal);
    _trx->travelSeg().push_back(&dal_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sjc);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sjc_sfo);
    fu1.travelSeg().push_back(&sfo_dal);
    fu1.travelSeg().push_back(&dal_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sjc);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sjc_sfo);
    pu1.travelSeg().push_back(&sfo_dal);
    pu1.travelSeg().push_back(&dal_dfw);
    pu1.travelSeg().push_back(&dfw_sjc);

    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sjc;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sjc_sfo);
    itin.travelSeg().push_back(&sfo_dal);
    itin.travelSeg().push_back(&dal_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sjc);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 5;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, &fp, &itin, 0, 0);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_DEST;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 2A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 2B", loc->loc() == sjc->loc());

    // Same test but using only the Itin
    //
    RuleUtil::TSIData tsiData1(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, &itin, 0, 0);
    loc = 0;

    result = RuleUtil::getGeoLocaleFromItin(tsiData1, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 2C", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 2D", loc->loc() == sjc->loc());
  }

  void testGetGeoLocaleFromItin3()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    ArunkSeg jfk_bos;
    jfk_bos.pnrSegment() = 3;
    jfk_bos.segmentOrder() = 3;
    jfk_bos.origin() = jfk;
    jfk_bos.destination() = bos;

    AirSeg bos_dfw;
    bos_dfw.pnrSegment() = 3;
    bos_dfw.segmentOrder() = 3;
    bos_dfw.origin() = bos;
    bos_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_bos);
    _trx->travelSeg().push_back(&bos_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu2.travelSeg().push_back(&jfk_bos);
    fu3.travelSeg().push_back(&bos_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_bos);
    pu2.travelSeg().push_back(&bos_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &bos_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_bos);
    itin.travelSeg().push_back(&bos_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 4;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, &fp, &itin, 0, /* &pu2*, */ 0);

    //
    // This is no longer an error condition for Journey scope
    //
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_TURNAROUND;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 3A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 3B", loc->loc() == bos->loc());

    // Same test but using only the Itin
    //
    RuleUtil::TSIData tsiData1(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, &itin, 0, 0);
    loc = 0;

    result = RuleUtil::getGeoLocaleFromItin(tsiData1, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 3C", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 3D", loc->loc() == bos->loc());
  }

  void testGetGeoLocaleFromItin4()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 3;
    jfk_dfw.segmentOrder() = 3;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, &fp, &itin, 0, 0);

    //
    // Test error condition... Unrecognized GeoItinPart
    //
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 4", !result);
  }

  void testGetGeoLocaleFromItin5()
  {
    // Create the travel segments
    //
    ArunkSeg sjc_sfo;
    sjc_sfo.pnrSegment() = 1;
    sjc_sfo.segmentOrder() = 1;
    sjc_sfo.origin() = sjc;
    sjc_sfo.destination() = sfo;

    AirSeg sfo_dal;
    sfo_dal.pnrSegment() = 2;
    sfo_dal.segmentOrder() = 2;
    sfo_dal.origin() = sfo;
    sfo_dal.destination() = dal;

    ArunkSeg dal_dfw;
    dal_dfw.pnrSegment() = 3;
    dal_dfw.segmentOrder() = 3;
    dal_dfw.origin() = dal;
    dal_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 4;
    dfw_jfk.segmentOrder() = 4;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 6;
    dfw_sfo.segmentOrder() = 6;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    ArunkSeg sfo_sjc;
    sfo_sjc.pnrSegment() = 7;
    sfo_sjc.segmentOrder() = 7;
    sfo_sjc.origin() = sfo;
    sfo_sjc.destination() = sjc;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sjc_sfo);
    _trx->travelSeg().push_back(&sfo_dal);
    _trx->travelSeg().push_back(&dal_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);
    _trx->travelSeg().push_back(&sfo_sjc);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sjc_sfo);
    fu1.travelSeg().push_back(&sfo_dal);
    fu1.travelSeg().push_back(&dal_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);
    fu4.travelSeg().push_back(&sfo_sjc);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sjc_sfo);
    pu1.travelSeg().push_back(&sfo_dal);
    pu1.travelSeg().push_back(&dal_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu1.travelSeg().push_back(&sfo_sjc);

    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sjc_sfo);
    itin.travelSeg().push_back(&sfo_dal);
    itin.travelSeg().push_back(&dal_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);
    itin.travelSeg().push_back(&sfo_sjc);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 5;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Sub-Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, &fp, &itin, &pu1, 0);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 5A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 5B", loc->loc() == sjc->loc());
  }

  void testGetGeoLocaleFromItin6()
  {
    // Create the travel segments
    //
    AirSeg sfo_dal;
    sfo_dal.pnrSegment() = 1;
    sfo_dal.segmentOrder() = 1;
    sfo_dal.origin() = sfo;
    sfo_dal.destination() = dal;

    ArunkSeg dal_dfw;
    dal_dfw.pnrSegment() = 2;
    dal_dfw.segmentOrder() = 2;
    dal_dfw.origin() = dal;
    dal_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 3;
    dfw_jfk.segmentOrder() = 3;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 4;
    jfk_dfw.segmentOrder() = 4;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    ArunkSeg dfw_dal;
    dfw_dal.pnrSegment() = 5;
    dfw_dal.segmentOrder() = 5;
    dfw_dal.origin() = dfw;
    dfw_dal.destination() = dal;

    AirSeg dal_sfo;
    dal_sfo.pnrSegment() = 6;
    dal_sfo.segmentOrder() = 6;
    dal_sfo.origin() = dal;
    dal_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dal);
    _trx->travelSeg().push_back(&dal_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_dal);
    _trx->travelSeg().push_back(&dal_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dal);
    fu1.travelSeg().push_back(&dal_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_dal);
    fu4.travelSeg().push_back(&dal_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dal);
    pu1.travelSeg().push_back(&dal_dfw);
    pu1.travelSeg().push_back(&dfw_dal);
    pu1.travelSeg().push_back(&dal_sfo);

    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_dal;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dal);
    itin.travelSeg().push_back(&dal_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_dal);
    itin.travelSeg().push_back(&dal_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 4;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Sub-Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, &fp, &itin, &pu2, 0);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_DEST;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 6A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 6B", loc->loc() == dfw->loc());
  }

  void testGetGeoLocaleFromItin7()
  {
    // Create the travel segments
    //
    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 1;
    dfw_jfk.segmentOrder() = 1;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    ArunkSeg jfk_bos;
    jfk_bos.pnrSegment() = 2;
    jfk_bos.segmentOrder() = 2;
    jfk_bos.origin() = jfk;
    jfk_bos.destination() = bos;

    AirSeg bos_lga;
    bos_lga.pnrSegment() = 3;
    bos_lga.segmentOrder() = 3;
    bos_lga.origin() = bos;
    bos_lga.destination() = lga;

    ArunkSeg lga_jfk;
    lga_jfk.pnrSegment() = 4;
    lga_jfk.segmentOrder() = 4;
    lga_jfk.origin() = lga;
    lga_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 5;
    jfk_dfw.segmentOrder() = 5;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_bos);
    _trx->travelSeg().push_back(&bos_lga);
    _trx->travelSeg().push_back(&lga_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&dfw_jfk);
    fu2.travelSeg().push_back(&jfk_bos);
    fu2.travelSeg().push_back(&bos_lga);
    fu2.travelSeg().push_back(&lga_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&dfw_jfk);
    pu1.travelSeg().push_back(&jfk_dfw);

    pu2.travelSeg().push_back(&jfk_bos);
    pu2.travelSeg().push_back(&bos_lga);
    pu2.travelSeg().push_back(&lga_jfk);

    // Set the turn around point for the pricing unit
    pu1.turnAroundPoint() = &jfk_dfw;
    pu2.turnAroundPoint() = &lga_jfk;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_bos);
    itin.travelSeg().push_back(&bos_lga);
    itin.travelSeg().push_back(&lga_jfk);
    itin.travelSeg().push_back(&jfk_dfw);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Sub-Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, &fp, &itin, &pu2, 0);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_TURNAROUND;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 7A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 7B", loc->loc() == lga->loc());
  }

  void testGetGeoLocaleFromItin8()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 3;
    jfk_dfw.segmentOrder() = 3;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Sub-Journey scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_SUB_JOURNEY, vendor, &fp, &itin, &pu1, 0);

    //
    // Test error condition... Unrecognized GeoItinPart
    //
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 8", !result);
  }

  void testGetGeoLocaleFromItin9()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 3;
    jfk_dfw.segmentOrder() = 3;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Create a fare market
    FareMarket fm;

    // Attach the travel segments to the fare market
    fm.travelSeg().push_back(&sfo_dfw);
    fm.travelSeg().push_back(&dfw_jfk);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Fare Component scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, &fp, &itin, &pu1, &fm);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_ORIG;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 9A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 9B", loc->loc() == sfo->loc());
  }

  void testGetGeoLocaleFromItin10()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    ArunkSeg jfk_bos;
    jfk_bos.pnrSegment() = 3;
    jfk_bos.segmentOrder() = 3;
    jfk_bos.origin() = jfk;
    jfk_bos.destination() = bos;

    AirSeg bos_dfw;
    bos_dfw.pnrSegment() = 4;
    bos_dfw.segmentOrder() = 4;
    bos_dfw.origin() = bos;
    bos_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 5;
    dfw_sfo.segmentOrder() = 5;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_bos);
    _trx->travelSeg().push_back(&bos_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu2.travelSeg().push_back(&jfk_bos);
    fu3.travelSeg().push_back(&bos_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_bos);
    pu2.travelSeg().push_back(&bos_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &bos_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_bos);
    itin.travelSeg().push_back(&bos_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 4;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Create a fare market
    FareMarket fm;

    // Attach the travel segments to the fare market
    fm.travelSeg().push_back(&sfo_dfw);
    fm.travelSeg().push_back(&dfw_jfk);
    fm.travelSeg().push_back(&jfk_bos);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Fare Component scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, &fp, &itin, &pu1, &fm);

    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_DEST;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 10A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 10B", loc->loc() == bos->loc());
  }

  void testGetGeoLocaleFromItin11()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 3;
    jfk_dfw.segmentOrder() = 3;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Create a fare market
    FareMarket fm;

    // Attach the travel segments to the fare market
    fm.travelSeg().push_back(&sfo_dfw);
    fm.travelSeg().push_back(&dfw_jfk);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Fare Component scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, &fp, &itin, &pu1, &fm);

    //
    // This is no longer an error condition for Fare Component scope.
    //
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_TURNAROUND;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 11A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 11B", loc->loc() == jfk->loc());
  }

  void testGetGeoLocaleFromItin12()
  {
    // Create the travel segments
    //
    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dfw;
    jfk_dfw.pnrSegment() = 3;
    jfk_dfw.segmentOrder() = 3;
    jfk_dfw.origin() = jfk;
    jfk_dfw.destination() = dfw;

    AirSeg dfw_sfo;
    dfw_sfo.pnrSegment() = 4;
    dfw_sfo.segmentOrder() = 4;
    dfw_sfo.origin() = dfw;
    dfw_sfo.destination() = sfo;

    // Create an itin
    Itin itin;

    // Attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(&sfo_dfw);
    _trx->travelSeg().push_back(&dfw_jfk);
    _trx->travelSeg().push_back(&jfk_dfw);
    _trx->travelSeg().push_back(&dfw_sfo);

    // Create the fare usages
    FareUsage fu1;
    FareUsage fu2;
    FareUsage fu3;
    FareUsage fu4;

    // Attach the travel segments to the fare usages
    fu1.travelSeg().push_back(&sfo_dfw);
    fu2.travelSeg().push_back(&dfw_jfk);
    fu3.travelSeg().push_back(&jfk_dfw);
    fu4.travelSeg().push_back(&dfw_sfo);

    // Set the directionality
    fu1.inbound() = false;
    fu2.inbound() = false;
    fu3.inbound() = true;
    fu4.inbound() = true;

    // Create the pricing units
    PricingUnit pu1;
    PricingUnit pu2;

    // Attach the fare usages to the pricing units
    pu1.fareUsage().push_back(&fu1);
    pu1.fareUsage().push_back(&fu4);
    pu2.fareUsage().push_back(&fu2);
    pu2.fareUsage().push_back(&fu3);

    // Attch the travel segments to the pricing unit
    pu1.travelSeg().push_back(&sfo_dfw);
    pu1.travelSeg().push_back(&dfw_sfo);
    pu2.travelSeg().push_back(&dfw_jfk);
    pu2.travelSeg().push_back(&jfk_dfw);

    pu1.turnAroundPoint() = &dfw_sfo;
    pu2.turnAroundPoint() = &jfk_dfw;

    // Create a fare path
    FarePath fp;

    // Attach the pricing units to the fare path
    fp.pricingUnit().push_back(&pu1);
    fp.pricingUnit().push_back(&pu2);

    // Create an itin
    // Itin itin;

    // Attach the travel segments to the itin
    itin.travelSeg().push_back(&sfo_dfw);
    itin.travelSeg().push_back(&dfw_jfk);
    itin.travelSeg().push_back(&jfk_dfw);
    itin.travelSeg().push_back(&dfw_sfo);

    // Set the turn around point for the Journey
    itin.furthestPointSegmentOrder() = 3;

    // Attach the fare path to the itin
    itin.farePath().push_back(&fp);

    // Attach the itin to the fare path
    fp.itin() = &itin;

    // Attach the itin to the transaction
    _trx->itin().push_back(&itin);

    // Create a fare market
    FareMarket fm;

    // Attach the travel segments to the fare market
    fm.travelSeg().push_back(&sfo_dfw);
    fm.travelSeg().push_back(&dfw_jfk);

    // Pointer to the return loc
    const Loc* loc = 0;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    //
    // Test Fare Component scope
    //
    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(
        tsiInfo, RuleConst::TSI_SCOPE_FARE_COMPONENT, vendor, &fp, &itin, &pu1, &fm);

    //
    // Test error condition... Unrecognized GeoItinPart
    //
    tsiInfo.geoItinPart() = RuleConst::TSI_GEO_ITIN_PART_BLANK;

    bool result = RuleUtil::getGeoLocaleFromItin(tsiData, loc);

    CPPUNIT_ASSERT_MESSAGE("Error in getGeoLocaleFromItin: Test 12", !result);
  }

  void testCheckGeoData1()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 1A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 1B",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 1C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 1D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 1E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));
  }

  void testCheckGeoData2()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 2A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 2B",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 2C", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 2D",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 2E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));
  }

  void testCheckGeoData3()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3B", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3F",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 3G", orig && (!dest));
  }

  void testCheckGeoData4()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4C",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4D",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4E", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4F",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 4G", (!orig) && dest);
  }

  void testCheckGeoData5()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5B", orig && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5E",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5F", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5G",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5H", orig && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 5I",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));
  }
  void testCheckGeoData6()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    //
    // Do test 1 again, but use the second set of Type/Loc variables.
    // The results should be exactly the same.
    //

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NATION;
    tsiData.locCode1() = EMPTY_STRING();
    tsiData.locCode2() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 6A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 6B",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 6C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 6D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 6E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));
  }

  void testCheckGeoData7()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    //
    // Do test 2 again, but use the second set of Type/Loc variables.
    // The results should be exactly the same.
    //

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NATION;
    tsiData.locCode1() = EMPTY_STRING();
    tsiData.locCode2() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 7A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 7B",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 7C", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 7E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 7F",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));
  }

  void testCheckGeoData8()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    //
    // Do test 3 again, but use the second set of Type/Loc variables.
    // The results should be exactly the same.
    //

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NATION;
    tsiData.locCode1() = EMPTY_STRING();
    tsiData.locCode2() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8B", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8F",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 8G", orig && (!dest));
  }

  void testCheckGeoData9()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    //
    // Do test 4 again, but use the second set of Type/Loc variables.
    // The results should be exactly the same.
    //

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NATION;
    tsiData.locCode1() = EMPTY_STRING();
    tsiData.locCode2() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9C",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9D",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9E", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9F",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 9G", (!orig) && dest);
  }

  void testCheckGeoData10()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    //
    // Do test 5 again, but use the second set of Type/Loc variables.
    // The results should be exactly the same.
    //

    tsiData.locType1() = LOCTYPE_NONE;
    tsiData.locType2() = LOCTYPE_NATION;
    tsiData.locCode1() = EMPTY_STRING();
    tsiData.locCode2() = "AU";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10B", orig && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10E",
                           RuleUtil::checkGeoData(*_trx, tsiData, &hkg_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10F", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10G",
                           RuleUtil::checkGeoData(*_trx, tsiData, &syd_mel, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10H", orig && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 10I",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));
  }

  void testCheckGeoData11()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 11A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 11B", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 11C",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));
  }

  void testCheckGeoData12()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 12A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 12B",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 12C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 12D", (!orig) && dest);
  }

  void testCheckGeoData13()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_EXCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13B", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13D", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13E",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13F", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 13G",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &dfw_iah, orig, dest));
  }

  void testCheckGeoData14()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 14A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 14B",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 14C", orig && (!dest));
  }

  void testCheckGeoData15()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 15A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 15B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 15C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 15D", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 15E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));
  }

  void testCheckGeoData16()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_INCLUDE;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16D", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16E",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16F", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16G",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 16H",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));
  }

  void testCheckGeoData17()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    //
    // GeoOut == BLANK is the same as INCLUDE
    //
    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_BLANK;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 17A",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 17B",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 17C", orig && (!dest));
  }

  void testCheckGeoData18()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    //
    // GeoOut == BLANK is the same as INCLUDE
    //
    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_BLANK;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 18A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 18B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 18C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 18D", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 18E",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));
  }

  void testCheckGeoData19()
  {
    // Create the travel segments
    //
    AirSeg mel_syd;
    mel_syd.pnrSegment() = 1;
    mel_syd.segmentOrder() = 1;
    mel_syd.origin() = mel;
    mel_syd.destination() = syd;

    AirSeg syd_hkg;
    syd_hkg.pnrSegment() = 2;
    syd_hkg.segmentOrder() = 2;
    syd_hkg.origin() = syd;
    syd_hkg.destination() = hkg;

    AirSeg hkg_syd;
    hkg_syd.pnrSegment() = 3;
    hkg_syd.segmentOrder() = 3;
    hkg_syd.origin() = hkg;
    hkg_syd.destination() = syd;

    AirSeg syd_mel;
    syd_mel.pnrSegment() = 4;
    syd_mel.segmentOrder() = 4;
    syd_mel.origin() = syd;
    syd_mel.destination() = mel;

    AirSeg sfo_dfw;
    sfo_dfw.pnrSegment() = 1;
    sfo_dfw.segmentOrder() = 1;
    sfo_dfw.origin() = sfo;
    sfo_dfw.destination() = dfw;

    AirSeg dfw_jfk;
    dfw_jfk.pnrSegment() = 2;
    dfw_jfk.segmentOrder() = 2;
    dfw_jfk.origin() = dfw;
    dfw_jfk.destination() = jfk;

    AirSeg jfk_dal;
    jfk_dal.pnrSegment() = 3;
    jfk_dal.segmentOrder() = 3;
    jfk_dal.origin() = jfk;
    jfk_dal.destination() = dal;

    AirSeg dfw_iah;
    dfw_iah.pnrSegment() = 1;
    dfw_iah.segmentOrder() = 1;
    dfw_iah.origin() = dfw;
    dfw_iah.destination() = iah;

    // Create a dummy TSIInfo object. We will only populate the fields
    //  we need for the purposes of this unit test.
    TSIInfo tsiInfo;
    tsiInfo.tsi() = -1;

    VendorCode vendor = "ATP";
    RuleUtil::TSIData tsiData(tsiInfo, RuleConst::TSI_SCOPE_JOURNEY, vendor, 0, 0, 0, 0);

    tsiData.locType1() = LOCTYPE_STATE;
    tsiData.locType2() = LOCTYPE_NONE;
    tsiData.locCode1() = "USTX";

    //
    // GeoOut == BLANK is the same as INCLUDE
    //
    tsiInfo.geoOut() = RuleConst::TSI_GEO_OUT_BLANK;
    tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG_DEST;

    bool orig = false;
    bool dest = false;

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19A",
                           RuleUtil::checkGeoData(*_trx, tsiData, &sfo_dfw, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19B", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19C",
                           RuleUtil::checkGeoData(*_trx, tsiData, &jfk_dal, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19D", (!orig) && dest);

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19E",
                           RuleUtil::checkGeoData(*_trx, tsiData, &dfw_jfk, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19F", orig && (!dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19G",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &mel_syd, orig, dest));

    CPPUNIT_ASSERT_MESSAGE("Error in checkGeoData: Test 19H",
                           !RuleUtil::checkGeoData(*_trx, tsiData, &syd_hkg, orig, dest));
  }

  //
  // Test TSI 1: Departure from Fare Origin
  //
  void testScopeTSIGeo1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 1;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin1(*_trx);

    FarePath* fp = *(itin->farePath().begin());
    PricingUnit* pu1 = *(fp->pricingUnit().begin());

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO is the departure from the Fare Origin
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 1A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 1B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 1C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo);
  }

  //
  // Test TSI 3: Departure of Each Trip
  //
  void testScopeTSIGeo3()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 3;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin1(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm2 = itin->fareMarket()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        fm2,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK is the departure of the second fare component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 3A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 3B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 3C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk);
  }

  //
  // Test TSI 5: Departure From Last Point of Stopover
  //
  void testScopeTSIGeo5()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 5;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // DFW-MIA is the departure from the last point of stopover for
    //  the second pricing unit
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *mia);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // DFW-MIA is the departure from the last point of stopover for the
    //  Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 5F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *mia);
  }

  //
  // Test TSI 6: Departure From Origin Gateway
  //
  void testScopeTSIGeo6()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 6;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK is the departure from the origin gateway for the second pricing unit
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR is the departure from the gateway of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 6F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr);
  }

  //
  // Test TSI 7: Departure of Overwater Segment
  //
  void testScopeTSIGeo7()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 7;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR and LHR-JFK are the overwater segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR is the departure of the overwater segment of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 7G",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr);
  }

  //
  // Test TSI 8: Departure of Last Segment
  //
  void testScopeTSIGeo8()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 8;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm2 = itin->fareMarket()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-DFW is the departure of the last segment of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm2,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // JFK is the departure of the last segment of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 8F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 9: Departure From First Point of Stopover
  //
  void testScopeTSIGeo9()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 9;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR-JFK is the departure from the first stopover of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // JFK-DFW is the departure from the first stopover of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 9F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);
  }

  //
  // Test TSI 10: Arrival at Fare Destination
  //
  void testScopeTSIGeo10()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 10;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // DFW is the arrival at the destination of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 10A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 10B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 10C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);
  }

  //
  // Test TSI 11: Arrival at First Point of Stopover
  //
  void testScopeTSIGeo11()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 11;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR is the arrival at the first stopover of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK is the arrival at the first stopover of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 11F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 12: Arrival at Destination Gateway
  //
  void testScopeTSIGeo12()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 12;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    //  pu2->travelSeg().push_back(dfw_jfk);
    //  pu2->travelSeg().push_back(jfk_lhr);
    //  pu2->travelSeg().push_back(lhr_jfk);
    //  pu2->travelSeg().push_back(jfk_dfw);
    // jfk is the destination gateway of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // JFK is the destination gateway of the Fare Component.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 12F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 13: Arrival of Overwater Segment
  //
  void testScopeTSIGeo13()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 13;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin2(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR and LHR-JFK are the overwater segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13D",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK is the overwater segment of the Fare Component.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 13G",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 17: Departure From Outward Destination
  //
  void testScopeTSIGeo17()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 17;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR-JFK is the departure from the outward destination of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17B", applTravelSegment.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    itin = createItin6(*_trx);

    fp = itin->farePath()[0];
    pu2 = fp->pricingUnit()[1];

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK is the departure from the outward destination of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 17F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 18: All International Sectors
  //
  void testScopeTSIGeo18()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 18;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm2 = itin->fareMarket()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // MIA-YYZ-JFK-LHR-JFK are the international segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18B", applTravelSegment.size() == 4);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18C",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18D",
                           applTravelSegment[3]->origMatch() && applTravelSegment[3]->destMatch() &&
                               *(applTravelSegment[3]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[3]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm2,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // MIA-YYZ-JFK-LHR is the international segment of the Fare Component.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18F", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18G",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18G",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 18G",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 19: All Domestic Sectors
  //
  void testScopeTSIGeo19()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 19;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-DFW and DFW-MIA are the domestic segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *mia);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // JFK-DFW and DFW-MIA are the domestic segments of the Fare Component.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19F", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19G",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 19H",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *mia);
  }

  //
  // Test TSI 20: International Domestic Transfer points
  //
  void testScopeTSIGeo20()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 20;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin6(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // YYZ-JFK and LHR-JFK are the international domestic transfer
    //  segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20D",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK is the international domestic transfer segments
    //  of the Fare Component.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 20G",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 21: Departure From Stopover
  //
  void testScopeTSIGeo21()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 21;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR-JFK, JFK-DFW and DFW-MIA are departures from the stopover
    //  segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *dfw);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21E",
                           applTravelSegment[2]->origMatch() &&
                               !applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *mia);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // JFK-DFW and DFW-MIA are the departures from the stopover segments
    //  of the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21F", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21G", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21H",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *dfw);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21I",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *mia);

    itin = createItin6(*_trx);

    fp = itin->farePath()[0];
    pu2 = fp->pricingUnit()[1];

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK, JFK-DFW and DFW-MIA are departures from the stopover
    //  segments of the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21J", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21K", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 21L",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }
  //
  // Test TSI 24: Arrival at First Point of Stopover Outside the
  //              Geographic Locale
  //
  void testScopeTSIGeo24()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 24;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NONE;

    locKey1.loc() = "US";

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR is the arrival at the first stopover outside of US for the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    locKey1.locType() = LOCTYPE_STATE;
    locKey1.loc() = "USTX";

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR-JFK is the first stopover outside TX in the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 24F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 25: All Segments
  //
  void testScopeTSIGeo25()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 25;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm5 = itin->fareMarket()[4];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 6 segments in the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25B", applTravelSegment.size() == 6);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There are 3 segments in Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25C", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25D", applTravelSegment.size() == 3);

    locKey1.locType() = LOCTYPE_NATION;
    locKey1.loc() = "CA";

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There are 2 segments with a board/off point Canada in the PU
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25F", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25G",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25H",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    locKey1.loc() = "GB";

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm5,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There is 1 segment with a board/off point in Great Britain in
    //  the Fare Component
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25I", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25J", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 25K",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 26: Outside Geographic Type of Fare Origin
  //
  void testScopeTSIGeo26()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 26;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 4 segments with a board/off point outside the nation
    //  of the origin (US) in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26B", applTravelSegment.size() == 4);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26E",
                           !applTravelSegment[2]->origMatch() &&
                               applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26F",
                           applTravelSegment[3]->origMatch() &&
                               !applTravelSegment[3]->destMatch() &&
                               *(applTravelSegment[3]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[3]->travelSeg()->destination()) == *jfk);

    locKey1.locType() = LOCTYPE_AREA;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There are 2 segments with a board/off point outside the area
    //  of the origin (IATA Area 1) in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26G", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26H", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26I",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 26J",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 27: Outside Geographic Type of Turnaround Point
  //
  void testScopeTSIGeo27()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 27;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NATION;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    PricingUnit* pu3 = fp->pricingUnit()[2];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR is the turnaround point.
    // There are 6 segments with a board/off point outside the nation
    //  of the turnaround point (GB) in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27B", applTravelSegment.size() == 6);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27E",
                           applTravelSegment[2]->origMatch() &&
                               !applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27F",
                           !applTravelSegment[3]->origMatch() &&
                               applTravelSegment[3]->destMatch() &&
                               *(applTravelSegment[3]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[3]->travelSeg()->destination()) == *jfk);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27G",
                           applTravelSegment[4]->origMatch() && applTravelSegment[4]->destMatch() &&
                               *(applTravelSegment[4]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[4]->travelSeg()->destination()) == *dfw);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27H",
                           applTravelSegment[5]->origMatch() && applTravelSegment[5]->destMatch() &&
                               *(applTravelSegment[5]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[5]->travelSeg()->destination()) == *mia);

    locKey2.locType() = LOCTYPE_AREA;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu3,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR is the turnaround point.
    // There are no segments with a board/off point outside the area
    //  of the turnaround point (IATA Area 2) in PU3
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27I", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 27J", applTravelSegment.empty());
  }

  //
  // Test TSI 28: Within Geographic Type of Fare Origin
  //
  void testScopeTSIGeo28()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 28;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_STATE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    PricingUnit* pu3 = fp->pricingUnit()[2];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // MIA is the fare origin.
    // There are 2 segments with a board/off point within the state
    //  of the fare origin (Florida) in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28D",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *mia);

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NATION;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu3,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // MIA is the fare origin.
    // There are 2 segments with a board/off point within the nation
    //  of the fare origin (GB) in PU3
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28F", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28G",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *cdg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 28H",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *cdg &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 29: Within Geographic Type of Turnaround Point
  //
  void testScopeTSIGeo29()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 29;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NATION;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    PricingUnit* pu3 = fp->pricingUnit()[2];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR is the turnaround point.
    // There are 2 segments with a board/off point within the nation
    //  of the turnaround point (GB) in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    locKey2.locType() = LOCTYPE_AREA;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu3,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // LHR is the turnaround point.
    // There are 2 segments with a board/off point within the area
    //  of the turnaround point (IATA Area 2) in PU3
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29F", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29G",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *cdg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 29H",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *cdg &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 31: Transatlantic Sectors
  //
  void testScopeTSIGeo31()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 31;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm2 = itin->fareMarket()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 2 trans-atlantic segments in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *jfk);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm2,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There is 1 trans-atlantic segment in FM2
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 31G",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 32: Transpacific Sectors
  //
  void testScopeTSIGeo32()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 32;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 2 trans-pacific segments in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There is 1 trans-pacific segment in FM4
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 32G",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 33: Outside the Following Geographic Locale
  //
  void testScopeTSIGeo33()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 33;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_AREA;
    locKey2.locType() = LOCTYPE_NONE;

    locKey1.loc() = "1"; // IATA Area 1

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 3 segments with a board/off point outside of Area 1 in PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *nrt);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33E",
                           applTravelSegment[2]->origMatch() &&
                               !applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *yvr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There are 2 segments with a board/off point outside of Area 1 in FM4
    //

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33F", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33G", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33H",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *nrt);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 33I",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 34: Arrival at Point of Turnaround
  //
  void testScopeTSIGeo34()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 34;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu1 = fp->pricingUnit()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // YYZ is the turnaround point of PU1
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // HKG is the turnaround point of PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 34F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);
  }

  //
  // Test TSI 37: Departure of Transoceanic Sector(s)
  //
  void testScopeTSIGeo37()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 37;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO-HKG and NRT-YVR are the transoceanic segments of PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37B", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   0,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // NRT-YVR is the transoceanic segment of FM4
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37E", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37F", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 37G",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 39: Within Any One Country
  //
  void testScopeTSIGeo39()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 39;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu1 = fp->pricingUnit()[0];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // STT-MIA, DFW-MIA and MIA-STT are all within one country
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *stt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *mia);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *dfw &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *mia);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39E",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *stt);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   0,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // YVR-YYZ is within one country
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39F", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39G", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 39H",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *yvr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);
  }

  //
  // Test TSI 40: Departure of Outbound Transatlantic Sector
  //
  void testScopeTSIGeo40()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 40;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR is the outbound trans-atlantic segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 40A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 40B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 40C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 41: Departure of Outbound Transpacific Sector
  //
  void testScopeTSIGeo41()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 41;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO-HKG is the outbound trans-pacific segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 41A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 41B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 41C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);
  }

  //
  // Test TSI 46: Departure from Last Stopover Outside Geographic Type
  //              of Fare Origin
  //
  void testScopeTSIGeo46()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 46;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // NRT-YVR is the departure from the last stopover outside the country
    //  of origin (Canada)
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 46A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 46B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 46C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 47: Overwater segments
  //
  void testScopeTSIGeo47()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 47;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin7(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu1 = fp->pricingUnit()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    PricingUnit* pu3 = fp->pricingUnit()[2];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *stt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *mia);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *anc);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47E",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *anc &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *dfw);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47F", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47G", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47H",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mex &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *pap);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47I",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *pap &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvi);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu3,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47J", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47K", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47L",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *gig &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hav);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 47M",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *hav &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *stt);
  }

  //
  // Test TSI 55: Departure of Inbound Transpacific Sector
  //
  void testScopeTSIGeo55()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 55;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // NRT-YVR is the inbound trans-pacific segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 55A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 55B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 55C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 56: Departure of Inbound Transatlantic Sector
  //
  void testScopeTSIGeo56()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 56;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR-JFK is the inbound trans-atlantic segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 56A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 56B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 56C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 59: Departure of Journey Origin
  //
  void testScopeTSIGeo59()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 59;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        0,
                                        itin,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // STT-MIA is the journey origin
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 59A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 59B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 59C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *stt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *mia);
  }

  //
  // Test TSI 60: Arrival at Journey Destination
  //
  void testScopeTSIGeo60()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 60;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        0,
                                        itin,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // MIA-STT is the journey destination
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 60A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 60B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 60C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *stt);
  }

  //
  // Test TSI 61: First Arrival at Geographic Type of the Turnaround Point
  //
  void testScopeTSIGeo61()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 61;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NATION;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu1 = fp->pricingUnit()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // YYZ is first arrival in the nation (Canada) of the turnaround point of PU1
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);

    locKey1.locType() = LOCTYPE_AREA;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // HKG is the first arrival in the area (Area 1) of the turnaround point
    //  of PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 61F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);
  }

  //
  // Test TSI 63: Departure of First International Sector of the Journey
  //
  void testScopeTSIGeo63()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 63;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        0,
                                        itin,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR is the first international sector of the journey
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 63A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 63B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 63C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);
  }

  //
  // Test TSI 64: Arrival at the First Stopover of Geographic Type
  //              of the Turnaround Point
  //
  void testScopeTSIGeo64()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 64;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_AREA;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin1 = createItin5(*_trx);
    FarePath* fp1 = itin1->farePath()[0];
    PricingUnit* pu1 = fp1->pricingUnit()[0];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp1,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR is arrival at the first stopover (LHR) in the area (IATA Area2)
    //  of the turnaround point of PU1
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    Itin* itin2 = createItin4(*_trx);
    FarePath* fp2 = itin2->farePath()[0];
    PricingUnit* pu2 = fp2->pricingUnit()[1];

    locKey1.locType() = LOCTYPE_AREA;

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp2,
                                   0,
                                   pu2,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // SFO-HKG is the arrival of the first stopover (HKG) in the area (Area 1)
    //  of the turnaround point of PU2.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64E", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 64F",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);
  }

  //
  // Test TSI 66: Departure of Last International Sector of the Journey
  //
  void testScopeTSIGeo66()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 66;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        0,
                                        itin,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR-JFK is the last international sector of the journey
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 66A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 66B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 66C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 73: Departure From the Last Stopover Before The Inbound
  //              Transatlantic Sector of Journey
  //
  void testScopeTSIGeo73()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 73;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    FarePath* fp = itin->farePath()[0];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // LHR is last stopover before the inbound transatlantic sector (LHR-JFK)
    // LHR-JFK is the departure from the stopover.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);

    itin = createItin6(*_trx);
    fp = itin->farePath()[0];

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_JOURNEY,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   0,
                                   0,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // There is no stopover in this FarePath, so the fare-break points
    //  are used instead of stopover points.
    // LHR is last fare-break point before the inbound transatlantic
    //  sector (LHR-JFK). The origin of LHR-JFK is the departure from the fare-break.
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73D", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73E", applTravelSegment.size() == 0);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 73F",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 75: All International Sectors of the Journey
  //
  void testScopeTSIGeo75()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 75;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin3(*_trx);

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        0,
                                        itin,
                                        0,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // JFK-LHR, LHR-CDG, CDG-LHR and LHR-JFK are all international sectors
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75B", applTravelSegment.size() == 4);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *jfk &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *cdg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75E",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *cdg &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *lhr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 75F",
                           applTravelSegment[3]->origMatch() && applTravelSegment[3]->destMatch() &&
                               *(applTravelSegment[3]->travelSeg()->origin()) == *lhr &&
                               *(applTravelSegment[3]->travelSeg()->destination()) == *jfk);
  }

  //
  // Test TSI 82: Departure From Gateway
  //
  void testScopeTSIGeo82()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 82;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO and HKG are the outbound gateways, NRT and YVR are the
    //  inbound gateways for PU2
    //
    // **NOTE: It may appear that HKG-NRT departs the destination gateway (HKG)
    //          but since HGK is the turn-around point, HKG-NRT is part of
    //          the return portion of flight and is considered INBOUND.
    //          Therefore, HKG-NRT is considered the arrival at the origin
    //          gateway which does not match this TSI definition.
    //
    //  pu2->travelSeg().push_back(yyz_sfo);
    //  pu2->travelSeg().push_back(sfo_hkg);
    //  pu2->travelSeg().push_back(hkg_nrt);
    //  pu2->travelSeg().push_back(nrt_yvr);
    //  pu2->travelSeg().push_back(yvr_yyz);
    //
    //  So we should have sfo at outbound, nrt, yvr at inbound match departure
    //  of gateway
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82D",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82E",
                           applTravelSegment[2]->origMatch() &&
                               !applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *yvr &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *yyz);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // NRT and YVR are the gateways for FM4
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82F", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82G", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82H",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 82I",
                           applTravelSegment[1]->origMatch() &&
                               !applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *yvr &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yyz);
  }

  //
  // Test TSI 83: Arrival at Gateway
  //
  void testScopeTSIGeo83()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 83;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO and HKG are the outbound gateways, NRT and YVR are the
    //  inbound gateways for PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83B", applTravelSegment.size() == 4);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *yyz &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *sfo);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83D",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83E",
                           !applTravelSegment[2]->origMatch() &&
                               applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *nrt);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83F",
                           !applTravelSegment[3]->origMatch() &&
                               applTravelSegment[3]->destMatch() &&
                               *(applTravelSegment[3]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[3]->travelSeg()->destination()) == *yvr);

    result = RuleUtil::scopeTSIGeo(tsi,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   false,
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    // NRT and YVR are the gateways for FM4
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83G", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83H", applTravelSegment.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83I",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *nrt);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 83J",
                           !applTravelSegment[1]->origMatch() &&
                               applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 85: From Last Fare Break Outside Country of Fare Origin
  //
  void testScopeTSIGeo85()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 85;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // NRT-YVR is the departure from the last fare break outside the country
    //  of origin (Canada) for PU2
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 85A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 85B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 85C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yvr);
  }

  //
  // Test TSI 86: Arrival at First Fare Break Outside Country of Fare Origin
  //
  void testScopeTSIGeo86()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 86;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu1 = fp->pricingUnit()[0];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu1,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // MIA-YYZ is the arrival at the first fare break outside the country
    //  of origin (US) for PU1
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 86A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 86B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 86C",
                           !applTravelSegment[0]->origMatch() &&
                               applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *mia &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *yyz);
  }
  //
  // Test TSI 87: Departure of First Outbound Intercontinental
  //
  void testScopeTSIGeo87()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 87;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // SFO-HKG is the departure of the first outbound intercontinental segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 87A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 87B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 87C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);
  }

  //
  // Test TSI 88: Departure of First Inbound Intercontinental
  //
  void testScopeTSIGeo88()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 88;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // NRT-YVR is the departure of the first inbound intercontinental segment
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 88A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 88B", applTravelSegment.size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 88C",
                           applTravelSegment[0]->origMatch() &&
                               !applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *nrt);
  }

  //
  // Test TSI 89: On Each Intercontinental Sector
  //
  void testScopeTSIGeo89()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const int16_t tsi = 89;
    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];

    bool result = RuleUtil::scopeTSIGeo(tsi,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                        false,
                                        false,
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        0,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    // There are 2 intercontinental segments
    //
    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 89A", result);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 89B", applTravelSegment.size() == 3);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 89C",
                           applTravelSegment[0]->origMatch() && applTravelSegment[0]->destMatch() &&
                               *(applTravelSegment[0]->travelSeg()->origin()) == *sfo &&
                               *(applTravelSegment[0]->travelSeg()->destination()) == *hkg);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 89D",
                           applTravelSegment[1]->origMatch() && applTravelSegment[1]->destMatch() &&
                               *(applTravelSegment[1]->travelSeg()->origin()) == *hkg &&
                               *(applTravelSegment[1]->travelSeg()->destination()) == *nrt);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test 89E",
                           applTravelSegment[2]->origMatch() && applTravelSegment[2]->destMatch() &&
                               *(applTravelSegment[2]->travelSeg()->origin()) == *nrt &&
                               *(applTravelSegment[2]->travelSeg()->destination()) == *yvr);
  }

  void testScopeTSIGeo_CategoryScopeOverrides()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic300;
    _trx->diagnostic().diagParamMap()[Diagnostic::DISPLAY_DETAIL] =
        RuleConst::DIAGNOSTIC_INCLUDE_GEO;
    _trx->diagnostic().activate();

    const DateTime ticketingDate;

    RuleUtil::TravelSegWrapperVector applTravelSegment;

    LocKey locKey1;
    LocKey locKey2;

    locKey1.locType() = LOCTYPE_NONE;
    locKey2.locType() = LOCTYPE_NONE;

    Itin* itin = createItin4(*_trx);

    FarePath* fp = itin->farePath()[0];
    PricingUnit* pu2 = fp->pricingUnit()[1];
    FareMarket* fm4 = itin->fareMarket()[3];

    bool result = RuleUtil::scopeTSIGeo(1,
                                        locKey1,
                                        locKey2,
                                        RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                        false,
                                        true, // Allow PU-FC scope override
                                        false,
                                        *_trx,
                                        fp,
                                        0,
                                        pu2,
                                        fm4,
                                        ticketingDate,
                                        applTravelSegment,
                                        Diagnostic300);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test scope override 1", result);

    result = RuleUtil::scopeTSIGeo(3,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY,
                                   false,
                                   false,
                                   true, // Allow FC-PU scope override
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test scope override 2", result);

    result = RuleUtil::scopeTSIGeo(59,
                                   locKey1,
                                   locKey2,
                                   RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                                   true, // Allow J-FC scope override
                                   false,
                                   false,
                                   *_trx,
                                   fp,
                                   0,
                                   pu2,
                                   fm4,
                                   ticketingDate,
                                   applTravelSegment,
                                   Diagnostic300);

    CPPUNIT_ASSERT_MESSAGE("Error in scopeTSIGeo: Test scope override 3", result);
  }

  void testFindBtwTvlSegs()
  {
    AirSeg tvlSeg1, tvlSeg2, tvlSeg3, tvlSeg4, tvlSeg5, tvlSeg6;

    tvlSeg1.departureDT() = DateTime(2008, 8, 8, 8, 0, 0);
    tvlSeg2.departureDT() = DateTime(2008, 8, 9, 8, 0, 0);
    tvlSeg3.departureDT() = DateTime(2008, 8, 10, 8, 0, 0);
    tvlSeg4.departureDT() = DateTime(2008, 8, 11, 8, 0, 0);
    tvlSeg5.departureDT() = DateTime(2008, 8, 12, 8, 0, 0);
    tvlSeg6.departureDT() = DateTime(2008, 8, 13, 8, 0, 0);

    RuleUtil::TravelSegWrapper tsw1, tsw2, tsw3, tsw4, tsw5, tsw6;
    tsw1.travelSeg() = &tvlSeg1;
    tsw2.travelSeg() = &tvlSeg2;
    tsw3.travelSeg() = &tvlSeg3;
    tsw4.travelSeg() = &tvlSeg4;
    tsw5.travelSeg() = &tvlSeg5;
    tsw6.travelSeg() = &tvlSeg6;

    std::vector<TravelSeg*> allTvlSegs;
    allTvlSegs.push_back(&tvlSeg1);
    allTvlSegs.push_back(&tvlSeg2);
    allTvlSegs.push_back(&tvlSeg3);
    allTvlSegs.push_back(&tvlSeg4);
    allTvlSegs.push_back(&tvlSeg5);
    allTvlSegs.push_back(&tvlSeg6);

    RuleUtil::TravelSegWrapperVector applBtwTvlSegs;
    RuleUtil::TravelSegWrapperVector applAndTvlSegs;
    std::vector<TravelSeg*> matchedTvlSegs;

    // test 1
    applBtwTvlSegs.push_back(&tsw2);
    applAndTvlSegs.push_back(&tsw3);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));
    CPPUNIT_ASSERT(matchedTvlSegs.size() == 2);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg2);
    CPPUNIT_ASSERT(matchedTvlSegs.back() == &tvlSeg3);

    // test 2
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw1);
    applAndTvlSegs.push_back(&tsw1);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));
    CPPUNIT_ASSERT(matchedTvlSegs.size() == 1);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg1);

    // test 3
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw2);
    applAndTvlSegs.push_back(&tsw2);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));
    CPPUNIT_ASSERT(matchedTvlSegs.size() == 1);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg2);

    // test 4
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw1);
    applBtwTvlSegs.push_back(&tsw2);
    applBtwTvlSegs.push_back(&tsw3);
    applAndTvlSegs.push_back(&tsw2);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));
    CPPUNIT_ASSERT(matchedTvlSegs.size() == 1);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg2);

    // test 5
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw1);
    applBtwTvlSegs.push_back(&tsw3);
    applAndTvlSegs.push_back(&tsw2);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));

    CPPUNIT_ASSERT(matchedTvlSegs.size() == 2);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg1);
    CPPUNIT_ASSERT(matchedTvlSegs.back() == &tvlSeg2);

    // test 6
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw3);
    applAndTvlSegs.push_back(&tsw2);

    CPPUNIT_ASSERT_EQUAL(
        false,
        RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));

    CPPUNIT_ASSERT(matchedTvlSegs.size() == 0);

    // test 7
    applBtwTvlSegs.clear();
    applAndTvlSegs.clear();
    matchedTvlSegs.clear();
    applBtwTvlSegs.push_back(&tsw3);
    applAndTvlSegs.push_back(&tsw2);
    applAndTvlSegs.push_back(&tsw4);

    CPPUNIT_ASSERT_EQUAL(
        true, RuleUtil::findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, matchedTvlSegs));

    CPPUNIT_ASSERT(matchedTvlSegs.size() == 2);
    CPPUNIT_ASSERT(matchedTvlSegs.front() == &tvlSeg3);
    CPPUNIT_ASSERT(matchedTvlSegs.back() == &tvlSeg4);
  }

  void testMatchGeo1()
  {
    LocKey loc1FromRule;
    LocKey loc2FromRule;
    LocCode originCode = "LON";
    LocCode destinationCode;
    Loc origin;
    Loc destination;
    VendorCode vendorCode;
    GeoTravelType geoTvlType = GeoTravelType::UnknownGeoTravelType;

    loc1FromRule.locType() = LOCTYPE_CITY;
    loc1FromRule.loc() = "DFW";

    bool ret = RuleUtil::matchGeo(*_trx,
                                  loc1FromRule,
                                  loc2FromRule,
                                  originCode,
                                  destinationCode,
                                  origin,
                                  destination,
                                  vendorCode,
                                  geoTvlType);
    CPPUNIT_ASSERT(!ret);
  }

  void testMatchGeo2()
  {
    LocKey loc1FromRule;
    LocKey loc2FromRule;
    LocCode originCode = "DFW";
    LocCode destinationCode = "NYC";
    Loc origin;
    Loc destination;
    VendorCode vendorCode;
    GeoTravelType geoTvlType = GeoTravelType::UnknownGeoTravelType;

    loc1FromRule.locType() = LOCTYPE_AIRPORT;
    loc1FromRule.loc() = "DFW";
    loc2FromRule.locType() = LOCTYPE_CITY;
    loc2FromRule.loc() = "NYC";

    bool ret = RuleUtil::matchGeo(*_trx,
                                  loc1FromRule,
                                  loc2FromRule,
                                  originCode,
                                  destinationCode,
                                  origin,
                                  destination,
                                  vendorCode,
                                  geoTvlType);
    CPPUNIT_ASSERT(ret);
  }

  void testMatchGeo3()
  {
    LocKey loc1FromRule;
    LocKey loc2FromRule;
    LocCode originCode = "DFW";
    LocCode destinationCode = "NYC";
    Loc origin;
    Loc destination;
    VendorCode vendorCode;
    GeoTravelType geoTvlType = GeoTravelType::UnknownGeoTravelType;

    loc1FromRule.locType() = LOCTYPE_AIRPORT;
    loc1FromRule.loc() = "DFW";
    loc2FromRule.locType() = LOCTYPE_AIRPORT;
    loc2FromRule.loc() = "LON";

    bool ret = RuleUtil::matchGeo(*_trx,
                                  loc1FromRule,
                                  loc2FromRule,
                                  originCode,
                                  destinationCode,
                                  origin,
                                  destination,
                                  vendorCode,
                                  geoTvlType);
    CPPUNIT_ASSERT(!ret);
  }

  void testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginEqualToDestination()
  {
    RuleUtil::TSITravelSegMarkup tsm = runSetupTravelSegMarkupSubJourney(
        PricingUnit::Type::ROUNDTRIP, /*isOriginEqualToDestinaton*/ true);
    CPPUNIT_ASSERT(tsm.destIsTurnAroundPoint());
  }

  void
  testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginNotEqualToDestinationPricingUnitOpenJaw()
  {
    RuleUtil::TSITravelSegMarkup tsm =
        runSetupTravelSegMarkupSubJourney(PricingUnit::Type::OPENJAW, false);
    CPPUNIT_ASSERT(tsm.destIsTurnAroundPoint());
  }

  void
  testSetupTravelSegMarkupSetTurnAroundPointAtDestForSubJourneyScopeOriginNotEqualToDestinationPricingUnitNotOpenJaw()
  {
    RuleUtil::TSITravelSegMarkup tsm =
        runSetupTravelSegMarkupSubJourney(PricingUnit::Type::CIRCLETRIP, false);
    CPPUNIT_ASSERT(!tsm.destIsTurnAroundPoint());
  }

  void testSetupTravelSegMarkupSubJourneyScopeNoTurnAroundPoint()
  {
    RuleUtil::TSITravelSegMarkup tsm = runSetupTravelSegMarkupSubJourney(PricingUnit::Type::ONEWAY, true);
    CPPUNIT_ASSERT(!tsm.destIsTurnAroundPoint());
  }

  void testSetupTravelSegMarkupJourneyScope()
  {
    RuleUtil::TSITravelSegMarkup tsm = runSetupTravelSegMarkupJourney();
    CPPUNIT_ASSERT(!tsm.destIsTurnAroundPoint());
  }

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  TestMemHandle _memHandle;

  // Locations
  //
  const Loc* anc; // Anchorage, Alaska
  const Loc* atl; // Atlanta, Georgia
  const Loc* bos;
  const Loc* cdg; // Paris
  const Loc* dal; // Love Field
  const Loc* dfw;
  const Loc* dus; // Dusseldorf, Germany
  const Loc* fra; // Frankfurt, Germany
  const Loc* gig; // Brazil
  const Loc* hav; // Havana, Cuba
  const Loc* hkg; // Hong Kong
  const Loc* hnl; // Honalulu (Hawaii)
  const Loc* iah; // Houston
  const Loc* jfk;
  const Loc* jnb; // Johannesburg, Sout Africa
  const Loc* lax;
  const Loc* lga;
  const Loc* lhr; // London
  const Loc* lon; // London
  const Loc* man; // Manchester
  const Loc* mel;
  const Loc* mex;
  const Loc* mia;
  const Loc* nrt; // Tokyo, Narita
  const Loc* nyc;
  const Loc* pap; // Port Au Prince, Haiti
  const Loc* sfo;
  const Loc* sin; // Singapore, Singapore
  const Loc* sjc; // San Jose
  const Loc* sju; // San Juan (Puerto Rico)
  const Loc* stt; // St Thomas (Virgin Islands)
  const Loc* syd;
  const Loc* tul; // Tulsa
  const Loc* yvi; // San Jose,Costa Rica
  const Loc* yvr; // Vancouver, Canada
  const Loc* yyz; // Toronto, Canada

};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilGeoTest);
} //tse
