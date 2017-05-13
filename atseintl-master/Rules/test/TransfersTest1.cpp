#include <vector>
#include <set>

#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VCTR.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/SurfaceTransfersInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/TransfersInfoSeg1.h"
#include "DBAccess/TSIInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleItemCaller.h"
#include "Rules/RuleSetPreprocessor.h"
#include "Rules/Transfers1.h"
#include "Rules/TransfersInfoWrapper.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackAPO37838Record1EffDateCheck);

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  CarrierApplicationInfo* getCxrApp(CarrierCode cxr)
  {
    CarrierApplicationInfo* ret = _memHandle.create<CarrierApplicationInfo>();
    ret->carrier() = cxr;
    return ret;
  }

public:
  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    if (currency == "USD")
    {
      NUCInfo* ret = _memHandle.create<NUCInfo>();
      ret->_nucFactor = 1;
      ret->_roundingFactor = 1;
      ret->_nucFactorNodec = 8;
      return ret;
    }
    return DataHandleMock::getNUCFirst(currency, carrier, date);
  }

  const PaxTypeInfo* getPaxType(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
  {
    if (paxTypeCode == "ADT")
      return DataHandleMock::getPaxType("ADT", "ATP");
    return DataHandleMock::getPaxType(paxTypeCode, vendor);
  }
  const TSIInfo* getTSI(int key)
  {
    if (key == 18 || key == 28 || key == 29)
    {
      TSIInfo* ret = _memHandle.create<TSIInfo>();
      ret->tsi() = key;
      ret->geoRequired() = key == 18 ? ' ' : 'T';
      ret->geoNotType() = key == 18 ? ' ' : 'T';
      ret->geoOut() = ' ';
      ret->geoItinPart() = key == 18 ? ' ' : (key == 28 ? 'O' : 'T');
      ret->geoCheck() = 'B';
      ret->loopDirection() = 'F';
      ret->loopOffset() = 0;
      ret->loopToSet() = 0;
      ret->loopMatch() = 'A';
      ret->scope() = key == 18 ? 'A' : 'S';
      ret->type() = 'B';
      if (key == 18)
        ret->matchCriteria().push_back((TSIInfo::TSIMatchCriteria)'N');
      return ret;
    }
    return DataHandleMock::getTSI(key);
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "BKK")
      return "BKK";
    else if (locCode == "CUN")
      return "CUN";
    else if (locCode == "HKG")
      return "HKG";
    else if (locCode == "JFK")
      return "NYC";
    else if (locCode == "LAX")
      return "LAX";
    else if (locCode == "STT")
      return "STT";
    else if (locCode == "SFO")
      return "SFO";
    else if (locCode == "DFW")
      return "DFW";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }

  bool getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& tariffNumber,
                                   const RuleNumber& ruleNumber,
                                   CatNumber catNum,
                                   RuleNumber& ruleNumOut,
                                   TariffNumber& tariffNumOut)
  {
    if (carrier == "")
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRule(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut);
  }
  //msd
  bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& tariffNumber,
                                            const RuleNumber& ruleNumber,
                                            CatNumber catNum,
                                            RuleNumber& ruleNumOut,
                                            TariffNumber& tariffNumOut,
                                            const DateTime& tvlDate)
  {
    if (carrier == "")
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRuleByTvlDate(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut, tvlDate);
  }
  //msd


  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate = DateTime::emptyDate())
  {
    if (carrier == "")
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    return DataHandleMock::getGeneralFareRule(
        vendor, carrier, ruleTariff, rule, category, date, applDate);
  }
  const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor,
                        int itemNo,
                        const DateTime& applDate = DateTime::emptyDate())
  {
    std::vector<CarrierApplicationInfo*>& ret =
        *_memHandle.create<std::vector<CarrierApplicationInfo*> >();
    if (itemNo == 22)
    {
      ret.push_back(getCxrApp("AA"));
      ret.push_back(getCxrApp("AX"));
      return ret;
    }
    else if (itemNo == 3001)
      return ret;
    return DataHandleMock::getCarrierApplication(vendor, itemNo, applDate);
  }

  const std::vector<SurfaceTransfersInfo*>&
  getSurfaceTransfers(const VendorCode& vendor, int itemNo)
  {
    if (vendor == "ATP")
    {
      if (itemNo == 19 || itemNo == 30)
        return *_memHandle.create<std::vector<SurfaceTransfersInfo*> >();
      if (itemNo == 40)
      {
        SurfaceTransfersInfo* sti = _memHandle.create<SurfaceTransfersInfo>();
        sti->restriction() = Transfers1::SURFACE_TABLE_RESTR_EITHER;
        sti->originDest() = Transfers1::SURFACE_TABLE_OD_EITHER;
        sti->fareBrkEmbeddedLoc().locType() = 'A';
        sti->fareBrkEmbeddedLoc().loc() = "1";
        sti->surfaceLoc().locType() = 'A';
        sti->surfaceLoc().loc() = "2";
        std::vector<SurfaceTransfersInfo*>* vec =
            _memHandle.create<std::vector<SurfaceTransfersInfo*> >();
        vec->push_back(sti);
        return *vec;
      }
      if (itemNo == 41)
      {
        SurfaceTransfersInfo* sti = _memHandle.create<SurfaceTransfersInfo>();
        sti->restriction() = Transfers1::SURFACE_TABLE_RESTR_EITHER;
        sti->originDest() = Transfers1::SURFACE_TABLE_OD_ORIG;
        sti->fareBrkEmbeddedLoc().locType() = 'A';
        sti->fareBrkEmbeddedLoc().loc() = "1";
        sti->surfaceLoc().locType() = 'A';
        sti->surfaceLoc().loc() = "2";
        std::vector<SurfaceTransfersInfo*>* vec =
            _memHandle.create<std::vector<SurfaceTransfersInfo*> >();
        vec->push_back(sti);
        return *vec;
      }
    }
    return DataHandleMock::getSurfaceTransfers(vendor, itemNo);
  }

  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "LG" || carrier == "OS")
      return DataHandleMock::getCarrierPreference("", date);
    return DataHandleMock::getCarrierPreference(carrier, date);
  }

  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
  {
    if (city == "BKK" || city == "FRA" || city == "HKG" || city == "LON" || city == "NRT" ||
        city == "SIN" || city == "YQB" || city == "SFO" || city == "DFW" || city == "CBR")
      return *_memHandle.create<std::vector<MultiAirportCity*> >();
    return DataHandleMock::getMultiAirportCity(city);
  }

  const RuleItemInfo* getRuleItemInfo(const CategoryRuleInfo* rule,
                                      const CategoryRuleItemInfo* item,
                                      const DateTime& applDate = DateTime::emptyDate())
  {
    RuleItemInfoKey key(rule, item);
    if (_ruleItemInfo.count(key) > 0)
      return _ruleItemInfo[key];
    return DataHandleMock::getRuleItemInfo(rule, item, applDate);
  }

  void setRuleItemInfo(const CategoryRuleInfo* rule,
                       const CategoryRuleItemInfo* item,
                       const RuleItemInfo* ruleItemInfo)
  {
    _ruleItemInfo[RuleItemInfoKey(rule, item)] = ruleItemInfo;
  }

private:
  typedef std::pair<const CategoryRuleInfo*, const CategoryRuleItemInfo*> RuleItemInfoKey;
  std::map<RuleItemInfoKey, const RuleItemInfo*> _ruleItemInfo;
};
}

class Transfers1Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Transfers1Test);

  CPPUNIT_TEST(testValidateFare0);
  CPPUNIT_TEST(testValidateFare1);
  CPPUNIT_TEST(testValidateFare2);
  CPPUNIT_TEST(testValidateFare3);
  CPPUNIT_TEST(testValidateFare4);
  CPPUNIT_TEST(testValidateFare5);
  CPPUNIT_TEST(testValidateFare6);
  CPPUNIT_TEST(testValidateFare7);
  CPPUNIT_TEST(testValidateFare8);

  CPPUNIT_TEST(testValidatePU0);
  CPPUNIT_TEST(testValidatePU1);
  CPPUNIT_TEST(testValidatePU2);
  CPPUNIT_TEST(testValidatePU3);
  CPPUNIT_TEST(testValidatePU4);
  CPPUNIT_TEST(testValidatePU5);
  CPPUNIT_TEST(testValidatePU6);
  CPPUNIT_TEST(testValidatePU7);
  CPPUNIT_TEST(testValidatePU8);
  CPPUNIT_TEST(testValidatePU9);
  CPPUNIT_TEST(testValidatePU10);

  CPPUNIT_TEST(testValidatePU12);
  CPPUNIT_TEST(testValidatePU13);
  CPPUNIT_TEST(testValidatePU14);
  CPPUNIT_TEST(testValidatePU15);
  CPPUNIT_TEST(testValidatePU16);

  CPPUNIT_TEST(testValidatePU20);
  CPPUNIT_TEST(testValidatePU21);
  CPPUNIT_TEST(testValidatePU22);
  CPPUNIT_TEST(testValidatePU23);
  CPPUNIT_TEST(testValidatePU24);
  CPPUNIT_TEST(testValidatePU25);
  CPPUNIT_TEST(testValidatePU26);
  CPPUNIT_TEST(testValidatePU27);

  CPPUNIT_TEST(testValidatePU28);
  CPPUNIT_TEST(testValidatePU29);

  CPPUNIT_TEST(testValidatePU30);
  CPPUNIT_TEST(testValidatePU31);

  CPPUNIT_TEST(testValidatePU32);
  CPPUNIT_TEST(testValidatePU33);
  CPPUNIT_TEST(testValidatePU34);
  CPPUNIT_TEST(testValidatePU35);
  CPPUNIT_TEST(testValidatePU36);
  CPPUNIT_TEST(testValidatePU37);

  CPPUNIT_TEST(testValidate_RtwIgnoreInOutOr);
  CPPUNIT_TEST(testValidate_RtwMaxBlank);
  CPPUNIT_TEST(testValidate_RtwTransfersPerNation);
  CPPUNIT_TEST(testValidate_RtwTransfersPerNationPass);
  CPPUNIT_TEST(testValidate_RtwPermittedReachedSameCharge);
  CPPUNIT_TEST(testValidate_RtwPermittedReachedOtherCharge);
  CPPUNIT_TEST(testProcessSurfaceRestrictions_RtwMaxEmbeddedExceeded);
  CPPUNIT_TEST(testProcessSurfaceRestrictions_RtwEmbeddedLocNotSpecified);
  CPPUNIT_TEST(testProcessSurfaceRestrictions_RtwEmbeddedIgnoreOrigDestInd);

  CPPUNIT_TEST(testRegressionPL7907);
  CPPUNIT_TEST(testRegressionPL8118);
  CPPUNIT_TEST(testRegressionPL11648);
  CPPUNIT_TEST(testRegressionPL20281);
  CPPUNIT_SKIP_TEST(testRegressionPL20281WithRelationalIndAND);

  CPPUNIT_TEST(testValidateSurfaceRestrictions);

  CPPUNIT_TEST(testValidatePU99);
  CPPUNIT_TEST(testValidatePU6GW);
  CPPUNIT_TEST(testRegressionPL8118GW);

  CPPUNIT_TEST(testIsFCvsPUreturnTrue);
  CPPUNIT_TEST(testIsFCvsPUreturnFalseMAXisXX);
  CPPUNIT_TEST(testIsFCvsPUreturnFalseMAXisBlank);
  CPPUNIT_TEST(testIsFCvsPUreturnFalseSegsIsNotEmpty);

  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchOrigDest_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchOrigDest_Fail);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchOrig_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchOrig_Fail);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchDest_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_FirstSegMatchDest_Fail);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchOrigDest_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchOrigDest_Fail);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchOrig_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchOrig_Fail);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchDest_Pass);
  CPPUNIT_TEST(testMatchSurfaceGeoFareBreak_LastSegMatchDest_Fail);

  CPPUNIT_TEST(testApplySystemDefaultAssumptionSkipWhenNotUSCA);
  CPPUNIT_TEST(testApplySystemDefaultAssumptionSkipWhenAllAir);
  CPPUNIT_TEST(testApplySystemDefaultAssumptionFailWhenNoAir);

  CPPUNIT_TEST(testCheckInfoSegRestriction_Dom);
  CPPUNIT_TEST(testCheckInfoSegRestriction_Dom_USCA);
  CPPUNIT_TEST(testCheckInfoSegRestriction_Intl);
  CPPUNIT_TEST(testCheckInfoSegRestriction_Intl_XURU);
  CPPUNIT_TEST(testCheckInfoSegRestriction_DomIntl);

  CPPUNIT_TEST(testCheckInfoSegDirectionality_SegGeoApplNotPermitted_inOutOut);
  CPPUNIT_TEST(testCheckInfoSegDirectionality_SegGeoApplNotPermitted_inOutIn);
  CPPUNIT_TEST(testRegressionAPO34850_SurfaceSectorNotMatch);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<RootLoggerGetOff>();
    _trx = _memHandle.create<PricingTrx>();

    _trx->setOptions(_memHandle.create<PricingOptions>());

    // Create Locations
    //
    sfo = getLoc("SFO");
    dfw = getLoc("DFW");
    dal = getLoc("DAL");
    sjc = getLoc("SJC");
    jfk = getLoc("JFK");
    bos = getLoc("BOS");
    lga = getLoc("LGA");
    lax = getLoc("LAX");
    iah = getLoc("IAH");
    mel = getLoc("MEL");
    syd = getLoc("SYD");
    hkg = getLoc("HKG");
    nrt = getLoc("NRT");
    mia = getLoc("MIA");
    yyz = getLoc("YYZ");
    yvr = getLoc("YVR");
    lhr = getLoc("LHR");
    gig = getLoc("GIG");
    hnl = getLoc("HNL");
    stt = getLoc("STT");
    anc = getLoc("ANC");
    sju = getLoc("SJU");
    cdg = getLoc("CDG");
    mex = getLoc("MEX");
    lux = getLoc("LUX");
    bkk = getLoc("BKK");
    fuk = getLoc("FUK");
    cbr = getLoc("CBR");
    tia = getLoc("TIA");
    vie = getLoc("VIE");
    zrh = getLoc("ZRH");
    yul = getLoc("YUL");
    yqb = getLoc("YQB");
    cun = getLoc("CUN");
    sin = getLoc("SIN");
    svo = getLoc("/LocSVO");
    hta = getLoc("/LocHTA");

    _myDataHandle = _memHandle.create<MyDataHandle>();
    fallback::value::fallbackAPO37838Record1EffDateCheck.set(true);
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  void tearDown() { _memHandle.clear(); }

  AirSeg* createAirSeg(int pnrSegment,
                       const Loc*& origin,
                       const Loc*& destination,
                       bool stopOver,
                       std::string carrier,
                       FlightNumber flightNumber,
                       GeoTravelType travelType,
                       DateTime departure,
                       DateTime arrival)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->pnrSegment() = pnrSegment;
    airSeg->origin() = origin;
    airSeg->destination() = destination;
    airSeg->stopOver() = stopOver;
    airSeg->carrier() = carrier;
    airSeg->flightNumber() = flightNumber;
    airSeg->geoTravelType() = travelType;
    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    return airSeg;
  }

  SurfaceSeg* createSurfaceSeg(int pnrSegment,
                               const Loc*& origin,
                               const Loc*& destination,
                               bool stopOver,
                               GeoTravelType travelType)
  {
    SurfaceSeg* seg = _memHandle.create<SurfaceSeg>();
    seg->pnrSegment() = pnrSegment;
    seg->origin() = origin;
    seg->destination() = destination;
    seg->stopOver() = stopOver;
    seg->geoTravelType() = travelType;

    return seg;
  }

  FareMarket*
  createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier)
  {
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->origin() = origin;
    fm1->destination() = destination;
    fm1->governingCarrier() = goveringCarrier;
    return fm1;
  }

  Fare* createFare(FareMarket* fm1,
                   Fare::FareState state,
                   GlobalDirection gd,
                   Indicator owrt,
                   CurrencyCode currency)
  {
    Fare* f1 = _memHandle.create<Fare>();
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* tcri1 = _memHandle.create<TariffCrossRefInfo>();

    fi1->_globalDirection = gd;
    fi1->_owrt = owrt;
    fi1->_currency = currency;
    f1->initialize(state, fi1, *fm1, tcri1);
    return f1;
  }

  PaxTypeFare* createPaxTypeFare(
      Fare* f1, FareMarket& fm1, PaxTypeCode paxTypeCode, VendorCode vendorCode, Indicator adultInd)
  {
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxType* pt1 = _memHandle.create<PaxType>();
    PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();

    pt1->paxType() = paxTypeCode;
    pt1->vendorCode() = vendorCode;
    pti1->adultInd() = adultInd;
    pt1->paxTypeInfo() = pti1;
    ptf1->initialize(f1, pt1, &fm1);
    return ptf1;
  }

  Itin* createItin1(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   802,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 13, 25, 0), //  6:25AM PST;
                                   DateTime(2004, 8, 15, 17, 1, 0)); // 12:01PM CST;

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   806,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 22, 35, 0), //  6:25AM PST;
                                   DateTime(2004, 8, 16, 2, 16, 0)); // 12:01PM CST;

    AirSeg* jfk_dfw = createAirSeg(3,
                                   jfk,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   1295,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 21, 12, 28, 0), //  6:25AM PST;
                                   DateTime(2004, 8, 21, 16, 3, 0)); // 12:01PM CST;

    AirSeg* dfw_sfo = createAirSeg(4,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   1441,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 21, 16, 47, 0), //  6:25AM PST;
                                   DateTime(2004, 8, 21, 20, 21, 0)); // 12:01PM CST;

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

    // Create the FareUsages
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu3->travelSeg().push_back(jfk_dfw);
    fu4->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = true;
    fu4->inbound() = true;

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
    FareMarket* fm1 = createFareMarket(sfo, dfw, "AA");
    FareMarket* fm2 = createFareMarket(dfw, jfk, "AA");
    FareMarket* fm3 = createFareMarket(jfk, dfw, "AA");
    FareMarket* fm4 = createFareMarket(dfw, sfo, "AA");

    // Create the Fares
    Fare* f1 = createFare(fm1, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f2 = createFare(fm2, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f3 = createFare(fm3, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f4 = createFare(fm4, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm3->travelSeg().push_back(jfk_dfw);
    fm4->travelSeg().push_back(dfw_sfo);

    fm1->setGlobalDirection(GlobalDirection::US);
    fm2->setGlobalDirection(GlobalDirection::US);
    fm3->setGlobalDirection(GlobalDirection::US);
    fm4->setGlobalDirection(GlobalDirection::US);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);

    fp->paxType() = ptf1->actualPaxType();

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

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* mex_dfw = createAirSeg(1,
                                   mex,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   1066,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 15, 12, 0, 0), //  7:00AM CST
                                   DateTime(2004, 8, 15, 14, 41, 0)); //  9:41AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 15, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 16, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 16, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 30, 18, 35, 0), // 2:35PM EST
                                   DateTime(2004, 8, 30, 22, 8, 0)); // 5:08PM CST

    AirSeg* dfw_mex = createAirSeg(8,
                                   dfw,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   409,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 18, 35, 0), // 2:35PM EST
                                   DateTime(2004, 8, 30, 22, 8, 0)); // 5:08PM CST

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
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

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
    FareMarket* fm1 = createFareMarket(mex, dfw, "AA");
    FareMarket* fm2 = createFareMarket(dfw, lhr, "AA");
    FareMarket* fm3 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm4 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm5 = createFareMarket(lhr, dfw, "AA");
    FareMarket* fm6 = createFareMarket(dfw, mex, "AA");

    Fare* f1 = createFare(fm1, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f2 = createFare(fm2, Fare::FS_Domestic, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f3 = createFare(fm3, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f4 = createFare(fm4, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f5 = createFare(fm5, Fare::FS_Domestic, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f6 = createFare(fm6, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf5 = createPaxTypeFare(f5, *fm5, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf6 = createPaxTypeFare(f6, *fm6, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

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

    fp->paxType() = ptf1->actualPaxType();

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

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* mex_dfw = createAirSeg(1,
                                   mex,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   1066,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 15, 12, 0, 0), //  7:00AM CST
                                   DateTime(2004, 8, 15, 14, 41, 0)); //  9:41AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 15, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 16, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 16, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 30, 18, 35, 0), // 2:35PM EST
                                   DateTime(2004, 8, 30, 22, 8, 0)); // 5:08PM CST

    AirSeg* dfw_mex = createAirSeg(8,
                                   dfw,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   409,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 18, 35, 0), // 2:35PM EST
                                   DateTime(2004, 8, 30, 22, 8, 0)); // 5:08PM CST

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
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

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
    FareMarket* fm1 = createFareMarket(mex, dfw, "AA");
    FareMarket* fm2 = createFareMarket(dfw, lhr, "AA");
    FareMarket* fm3 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm4 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm5 = createFareMarket(lhr, dfw, "AA");
    FareMarket* fm6 = createFareMarket(dfw, mex, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f5 =
        createFare(fm5, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "");
    Fare* f6 =
        createFare(fm6, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf5 = createPaxTypeFare(f5, *fm5, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf6 = createPaxTypeFare(f6, *fm6, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

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

    fp->paxType() = ptf1->actualPaxType();

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

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AE"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;
    pu2->puType() = PricingUnit::Type::ROUNDTRIP;
    pu3->puType() = PricingUnit::Type::ROUNDTRIP;

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
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
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
    FareMarket* fm1 = createFareMarket(sfo, dfw, "AE");
    FareMarket* fm2 = createFareMarket(dfw, lhr, "AA");
    FareMarket* fm3 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm4 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm5 = createFareMarket(lhr, dfw, "AA");
    FareMarket* fm6 = createFareMarket(dfw, sfo, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f5 =
        createFare(fm5, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f6 =
        createFare(fm6, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf5 = createPaxTypeFare(f5, *fm5, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf6 = createPaxTypeFare(f6, *fm6, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

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

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin5(PricingTrx& trx, const std::string paxType)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AE"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

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
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
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
    FareMarket* fm1 = createFareMarket(sfo, dfw, "AE");
    FareMarket* fm2 = createFareMarket(dfw, lhr, "AA");
    FareMarket* fm3 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm4 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm5 = createFareMarket(lhr, dfw, "AA");
    FareMarket* fm6 = createFareMarket(dfw, sfo, "AA");

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::AT);
    fm6->setGlobalDirection(GlobalDirection::ZZ);

    fm1->geoTravelType() = GeoTravelType::Domestic;
    fm2->geoTravelType() = GeoTravelType::International;
    fm3->geoTravelType() = GeoTravelType::International;
    fm4->geoTravelType() = GeoTravelType::International;
    fm5->geoTravelType() = GeoTravelType::International;
    fm6->geoTravelType() = GeoTravelType::Domestic;

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf3 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf4 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf5 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf6 = _memHandle.create<PaxTypeFare>();

    Fare* f1 =
        createFare(fm1, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f5 =
        createFare(fm5, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f6 =
        createFare(fm6, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");

    f1->setCategoryValid(8);
    f2->setCategoryValid(8);
    f3->setCategoryValid(8);
    f4->setCategoryValid(8);
    f5->setCategoryValid(8);
    f6->setCategoryValid(8);

    PaxType* pt1 = _memHandle.create<PaxType>();
    PaxType* pt2 = _memHandle.create<PaxType>();
    PaxType* pt3 = _memHandle.create<PaxType>();
    PaxType* pt4 = _memHandle.create<PaxType>();
    PaxType* pt5 = _memHandle.create<PaxType>();
    PaxType* pt6 = _memHandle.create<PaxType>();

    PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti2 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti3 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti4 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti5 = _memHandle.create<PaxTypeInfo>();
    PaxTypeInfo* pti6 = _memHandle.create<PaxTypeInfo>();

    if (paxType == "CNN")
    {
      pt1->paxType() = "CNN";
      pt2->paxType() = "CNN";
      pt3->paxType() = "CNN";
      pt4->paxType() = "CNN";
      pt5->paxType() = "CNN";
      pt6->paxType() = "CNN";

      pt1->vendorCode() = "ATP";
      pt2->vendorCode() = "ATP";
      pt3->vendorCode() = "ATP";
      pt4->vendorCode() = "ATP";
      pt5->vendorCode() = "ATP";
      pt6->vendorCode() = "ATP";

      pti1->childInd() = 'Y';
      pti2->childInd() = 'Y';
      pti3->childInd() = 'Y';
      pti4->childInd() = 'Y';
      pti5->childInd() = 'Y';
      pti6->childInd() = 'Y';
    }
    else if (paxType == "INF")
    {
      pt1->paxType() = "INF";
      pt2->paxType() = "INF";
      pt3->paxType() = "INF";
      pt4->paxType() = "INF";
      pt5->paxType() = "INF";
      pt6->paxType() = "INF";

      pt1->vendorCode() = "ATP";
      pt2->vendorCode() = "ATP";
      pt3->vendorCode() = "ATP";
      pt4->vendorCode() = "ATP";
      pt5->vendorCode() = "ATP";
      pt6->vendorCode() = "ATP";

      pti1->infantInd() = 'Y';
      pti2->infantInd() = 'Y';
      pti3->infantInd() = 'Y';
      pti4->infantInd() = 'Y';
      pti5->infantInd() = 'Y';
      pti6->infantInd() = 'Y';
    }
    else
    {
      pt1->paxType() = "ADT";
      pt2->paxType() = "ADT";
      pt3->paxType() = "ADT";
      pt4->paxType() = "ADT";
      pt5->paxType() = "ADT";
      pt6->paxType() = "ADT";

      pt1->vendorCode() = "ATP";
      pt2->vendorCode() = "ATP";
      pt3->vendorCode() = "ATP";
      pt4->vendorCode() = "ATP";
      pt5->vendorCode() = "ATP";
      pt6->vendorCode() = "ATP";

      pti1->adultInd() = 'Y';
      pti2->adultInd() = 'Y';
      pti3->adultInd() = 'Y';
      pti4->adultInd() = 'Y';
      pti5->adultInd() = 'Y';
      pti6->adultInd() = 'Y';
    }

    pt1->paxTypeInfo() = pti1;
    pt2->paxTypeInfo() = pti2;
    pt3->paxTypeInfo() = pti3;
    pt4->paxTypeInfo() = pti4;
    pt5->paxTypeInfo() = pti5;
    pt6->paxTypeInfo() = pti6;

    ptf1->initialize(f1, pt1, fm1);
    ptf2->initialize(f2, pt2, fm2);
    ptf3->initialize(f3, pt3, fm3);
    ptf4->initialize(f4, pt4, fm4);
    ptf5->initialize(f5, pt5, fm5);
    ptf6->initialize(f6, pt6, fm6);

    FareClassAppInfo* fcai1 = _memHandle.create<FareClassAppInfo>();
    FareClassAppInfo* fcai2 = _memHandle.create<FareClassAppInfo>();
    FareClassAppInfo* fcai3 = _memHandle.create<FareClassAppInfo>();
    FareClassAppInfo* fcai4 = _memHandle.create<FareClassAppInfo>();
    FareClassAppInfo* fcai5 = _memHandle.create<FareClassAppInfo>();
    FareClassAppInfo* fcai6 = _memHandle.create<FareClassAppInfo>();

    ptf1->fareClassAppInfo() = fcai1;
    ptf2->fareClassAppInfo() = fcai2;
    ptf3->fareClassAppInfo() = fcai3;
    ptf4->fareClassAppInfo() = fcai4;
    ptf5->fareClassAppInfo() = fcai5;
    ptf6->fareClassAppInfo() = fcai6;

    FareClassAppSegInfo* fcasi1 = _memHandle.create<FareClassAppSegInfo>();
    FareClassAppSegInfo* fcasi2 = _memHandle.create<FareClassAppSegInfo>();
    FareClassAppSegInfo* fcasi3 = _memHandle.create<FareClassAppSegInfo>();
    FareClassAppSegInfo* fcasi4 = _memHandle.create<FareClassAppSegInfo>();
    FareClassAppSegInfo* fcasi5 = _memHandle.create<FareClassAppSegInfo>();
    FareClassAppSegInfo* fcasi6 = _memHandle.create<FareClassAppSegInfo>();

    fcasi1->_paxType = pt1->paxType();
    fcasi2->_paxType = pt2->paxType();
    fcasi3->_paxType = pt3->paxType();
    fcasi4->_paxType = pt4->paxType();
    fcasi5->_paxType = pt5->paxType();
    fcasi6->_paxType = pt6->paxType();

    ptf1->fareClassAppSegInfo() = fcasi1;
    ptf2->fareClassAppSegInfo() = fcasi2;
    ptf3->fareClassAppSegInfo() = fcasi3;
    ptf4->fareClassAppSegInfo() = fcasi4;
    ptf5->fareClassAppSegInfo() = fcasi5;
    ptf6->fareClassAppSegInfo() = fcasi6;

    if (paxType == "CNN" || paxType == "INF")
    {
      DiscountInfo* di1 = _memHandle.create<DiscountInfo>();
      DiscountInfo* di2 = _memHandle.create<DiscountInfo>();
      DiscountInfo* di3 = _memHandle.create<DiscountInfo>();
      DiscountInfo* di4 = _memHandle.create<DiscountInfo>();
      DiscountInfo* di5 = _memHandle.create<DiscountInfo>();
      DiscountInfo* di6 = _memHandle.create<DiscountInfo>();

      if (paxType == "CNN")
      {
        di1->discPercent() = 50.0;
        di2->discPercent() = 50.0;
        di3->discPercent() = 50.0;
        di4->discPercent() = 50.0;
        di5->discPercent() = 50.0;
        di6->discPercent() = 50.0;
      }
      else if (paxType == "INF")
      {
        di1->discPercent() = 10.0;
        di2->discPercent() = 10.0;
        di3->discPercent() = 10.0;
        di4->discPercent() = 10.0;
        di5->discPercent() = 10.0;
        di6->discPercent() = 10.0;
      }

      ptf1->status().set(PaxTypeFare::PTF_Discounted);
      ptf2->status().set(PaxTypeFare::PTF_Discounted);
      ptf3->status().set(PaxTypeFare::PTF_Discounted);
      ptf4->status().set(PaxTypeFare::PTF_Discounted);
      ptf5->status().set(PaxTypeFare::PTF_Discounted);
      ptf6->status().set(PaxTypeFare::PTF_Discounted);

      PaxTypeFareRuleData* ptfrd1 = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFareRuleData* ptfrd2 = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFareRuleData* ptfrd3 = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFareRuleData* ptfrd4 = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFareRuleData* ptfrd5 = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFareRuleData* ptfrd6 = _memHandle.create<PaxTypeFareRuleData>();

      ptfrd1->ruleItemInfo() = di1;
      ptfrd2->ruleItemInfo() = di2;
      ptfrd3->ruleItemInfo() = di3;
      ptfrd4->ruleItemInfo() = di4;
      ptfrd5->ruleItemInfo() = di5;
      ptfrd6->ruleItemInfo() = di6;

      ptf1->setRuleData(19, trx.dataHandle(), ptfrd1);
      ptf2->setRuleData(19, trx.dataHandle(), ptfrd2);
      ptf3->setRuleData(19, trx.dataHandle(), ptfrd3);
      ptf4->setRuleData(19, trx.dataHandle(), ptfrd4);
      ptf5->setRuleData(19, trx.dataHandle(), ptfrd5);
      ptf6->setRuleData(19, trx.dataHandle(), ptfrd6);
    }

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

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

    fp->paxType() = pt1;

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

    itin->calculationCurrency() = "NUC";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AE"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();
    FareUsage* fu6 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu2->travelSeg().push_back(dfw_jfk);
    fu2->travelSeg().push_back(jfk_lhr);
    fu3->travelSeg().push_back(lhr_cdg);
    fu4->travelSeg().push_back(cdg_lhr);
    fu5->travelSeg().push_back(lhr_jfk);
    fu5->travelSeg().push_back(jfk_dfw);
    fu6->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu5->inbound() = true;
    fu6->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;
    pu2->puType() = PricingUnit::Type::ROUNDTRIP;
    pu3->puType() = PricingUnit::Type::ROUNDTRIP;

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
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(jfk_lhr);
    pu2->travelSeg().push_back(lhr_jfk);
    pu3->travelSeg().push_back(lhr_cdg);
    pu3->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = jfk_dfw;
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
    FareMarket* fm1 = createFareMarket(sfo, dfw, "AE");
    FareMarket* fm2 = createFareMarket(dfw, lhr, "AA");
    FareMarket* fm3 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm4 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm5 = createFareMarket(lhr, dfw, "AA");
    FareMarket* fm6 = createFareMarket(dfw, sfo, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f5 =
        createFare(fm5, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f6 =
        createFare(fm6, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf5 = createPaxTypeFare(f5, *fm5, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf6 = createPaxTypeFare(f6, *fm6, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;
    fu6->paxTypeFare() = ptf6;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm2->travelSeg().push_back(dfw_jfk);
    fm2->travelSeg().push_back(jfk_lhr);
    fm3->travelSeg().push_back(lhr_cdg);
    fm4->travelSeg().push_back(cdg_lhr);
    fm5->travelSeg().push_back(lhr_jfk);
    fm5->travelSeg().push_back(jfk_dfw);
    fm6->travelSeg().push_back(dfw_sfo);

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

    fp->paxType() = ptf1->actualPaxType();

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

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AE"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    // dfw_jfk->carrier()       = "AA";
    // dfw_jfk->flightNumber()  = 1882;
    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string(""),
                                   0,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    // dfw_sfo->carrier()       = "AA";
    // dfw_sfo->flightNumber()  = 260>();
    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string(""),
                                   0,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu2->travelSeg().push_back(lhr_cdg);
    fu3->travelSeg().push_back(cdg_lhr);
    fu4->travelSeg().push_back(lhr_jfk);
    fu4->travelSeg().push_back(jfk_dfw);
    fu4->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = true;
    fu4->inbound() = true;

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
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);
    pu2->travelSeg().push_back(lhr_cdg);
    pu2->travelSeg().push_back(cdg_lhr);

    pu1->turnAroundPoint() = lhr_jfk;
    pu2->turnAroundPoint() = cdg_lhr;

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
    FareMarket* fm1 = createFareMarket(sfo, lhr, "AA");
    FareMarket* fm2 = createFareMarket(lhr, cdg, "BA");
    FareMarket* fm3 = createFareMarket(cdg, lhr, "BA");
    FareMarket* fm4 = createFareMarket(lhr, sfo, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_Domestic, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "GBP");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm2->travelSeg().push_back(lhr_cdg);
    fm3->travelSeg().push_back(cdg_lhr);
    fm4->travelSeg().push_back(lhr_jfk);
    fm4->travelSeg().push_back(jfk_dfw);
    fm4->travelSeg().push_back(dfw_sfo);

    fm1->setGlobalDirection(GlobalDirection::AT);
    fm2->setGlobalDirection(GlobalDirection::ZZ);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin8(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* dfw_jfk = createAirSeg(1,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 10, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(2,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 10, 17, 6, 20, 0)); //  6:20AM GMT

    // lhr_cdg->carrier()       = "BA";
    // lhr_cdg->flightNumber()  = 304;
    AirSeg* lhr_cdg = createAirSeg(3,
                                   lhr,
                                   cdg,
                                   false,
                                   std::string(""),
                                   0,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 10, 18, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(4,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 11, 29, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_lga = createAirSeg(5,
                                   lhr,
                                   lga,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 11, 29, 16, 30, 0)); // 12:30PM EST

    // lga_jfk->carrier()       = "AA";
    // lga_jfk->flightNumber()  = 10>();
    AirSeg* lga_jfk = createAirSeg(6,
                                   lga,
                                   jfk,
                                   false,
                                   std::string(""),
                                   0,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 11, 29, 16, 30, 0), // 12:30PM EST
                                   DateTime(2004, 11, 30, 22, 59, 0)); //  5:59PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 11, 30, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 12, 1, 3, 54, 0)); // 8:54PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_lga);
    itin->travelSeg().push_back(lga_jfk);
    itin->travelSeg().push_back(jfk_dfw);

    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_lga);
    trx.travelSeg().push_back(lga_jfk);
    trx.travelSeg().push_back(jfk_dfw);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_lga);
    fu2->travelSeg().push_back(lga_jfk);
    fu2->travelSeg().push_back(jfk_dfw);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_lga);
    pu1->travelSeg().push_back(lga_jfk);
    pu1->travelSeg().push_back(jfk_dfw);

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
    FareMarket* fm1 = createFareMarket(dfw, cdg, "AA");
    FareMarket* fm2 = createFareMarket(cdg, dfw, "BA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_lga);
    fm2->travelSeg().push_back(lga_jfk);
    fm2->travelSeg().push_back(jfk_dfw);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin9(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* dfw_jfk = createAirSeg(1,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(2,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(3,
                                   lhr,
                                   cdg,
                                   false,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(4,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(5,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(6,
                                   jfk,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);

    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);
    fu2->travelSeg().push_back(jfk_dfw);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_dfw);

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
    FareMarket* fm1 = createFareMarket(dfw, cdg, "AA");
    FareMarket* fm2 = createFareMarket(cdg, dfw, "BA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);
    fm2->travelSeg().push_back(jfk_dfw);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin10(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("AE"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AA"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("AF"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AA"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);
    fu2->travelSeg().push_back(jfk_dfw);
    fu2->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);

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
    FareMarket* fm1 = createFareMarket(sfo, cdg, "AA");
    FareMarket* fm2 = createFareMarket(cdg, sfo, "BA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);
    fm2->travelSeg().push_back(jfk_dfw);
    fm2->travelSeg().push_back(dfw_sfo);

    fm1->setGlobalDirection(GlobalDirection::AT);
    fm2->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin11(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* sfo_dfw = createAirSeg(1,
                                   sfo,
                                   dfw,
                                   false,
                                   std::string("DL"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 8, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* dfw_jfk = createAirSeg(2,
                                   dfw,
                                   jfk,
                                   true,
                                   std::string("AE"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 8, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 8, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("AX"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 8, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("AF"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 8, 30, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 8, 30, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 8, 30, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_dfw = createAirSeg(7,
                                   jfk,
                                   dfw,
                                   true,
                                   std::string("AE"),
                                   1345,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 8, 31, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 9, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* dfw_sfo = createAirSeg(8,
                                   dfw,
                                   sfo,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 9, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 9, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sfo_dfw);
    itin->travelSeg().push_back(dfw_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_dfw);
    itin->travelSeg().push_back(dfw_sfo);

    trx.travelSeg().push_back(sfo_dfw);
    trx.travelSeg().push_back(dfw_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_dfw);
    trx.travelSeg().push_back(dfw_sfo);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sfo_dfw);
    fu1->travelSeg().push_back(dfw_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);
    fu2->travelSeg().push_back(jfk_dfw);
    fu2->travelSeg().push_back(dfw_sfo);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sfo_dfw);
    pu1->travelSeg().push_back(dfw_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_dfw);
    pu1->travelSeg().push_back(dfw_sfo);

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
    FareMarket* fm1 = createFareMarket(sfo, cdg, "AA");
    FareMarket* fm2 = createFareMarket(cdg, sfo, "BA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sfo_dfw);
    fm1->travelSeg().push_back(dfw_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);
    fm2->travelSeg().push_back(jfk_dfw);
    fm2->travelSeg().push_back(dfw_sfo);

    fm1->setGlobalDirection(GlobalDirection::AT);
    fm2->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin12(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* mex_yyz = createAirSeg(1,
                                   mex,
                                   yyz,
                                   false,
                                   std::string("DL"),
                                   1590,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 15, 7, 25, 0), // 12:25AM PST
                                   DateTime(2004, 10, 15, 10, 44, 0)); //  5:44AM CST

    AirSeg* yyz_jfk = createAirSeg(2,
                                   yyz,
                                   jfk,
                                   true,
                                   std::string("AE"),
                                   1882,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 15, 17, 30, 0), // 12:30PM CST
                                   DateTime(2004, 10, 15, 21, 8, 0)); //  5:08PM EST

    AirSeg* jfk_lhr = createAirSeg(3,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 10, 17, 6, 20, 0)); //  6:20AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 10, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(5,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("AF"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 11, 29, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(6,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 11, 29, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_yyz = createAirSeg(7,
                                   jfk,
                                   yyz,
                                   true,
                                   std::string("AE"),
                                   1345,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 30, 22, 59, 0), // 5:59PM EST
                                   DateTime(2004, 12, 1, 3, 54, 0)); // 8:54PM CST

    AirSeg* yyz_mex = createAirSeg(8,
                                   yyz,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 12, 2, 2, 51, 0), //  9:51PM CST
                                   DateTime(2004, 12, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(mex_yyz);
    itin->travelSeg().push_back(yyz_jfk);
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);
    itin->travelSeg().push_back(jfk_yyz);
    itin->travelSeg().push_back(yyz_mex);

    trx.travelSeg().push_back(mex_yyz);
    trx.travelSeg().push_back(yyz_jfk);
    trx.travelSeg().push_back(jfk_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_lhr);
    trx.travelSeg().push_back(lhr_jfk);
    trx.travelSeg().push_back(jfk_yyz);
    trx.travelSeg().push_back(yyz_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_yyz);
    fu1->travelSeg().push_back(yyz_jfk);
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu2->travelSeg().push_back(cdg_lhr);
    fu2->travelSeg().push_back(lhr_jfk);
    fu2->travelSeg().push_back(jfk_yyz);
    fu2->travelSeg().push_back(yyz_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_yyz);
    pu1->travelSeg().push_back(yyz_jfk);
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);
    pu1->travelSeg().push_back(jfk_yyz);
    pu1->travelSeg().push_back(yyz_mex);

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
    FareMarket* fm1 = createFareMarket(mex, cdg, "AA");
    FareMarket* fm2 = createFareMarket(cdg, mex, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_yyz);
    fm1->travelSeg().push_back(yyz_jfk);
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm2->travelSeg().push_back(cdg_lhr);
    fm2->travelSeg().push_back(lhr_jfk);
    fm2->travelSeg().push_back(jfk_yyz);
    fm2->travelSeg().push_back(yyz_mex);

    fm1->setGlobalDirection(GlobalDirection::AT);
    fm2->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItin13(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* mex_yvr = createAirSeg(1,
                                   mex,
                                   yvr,
                                   true,
                                   std::string("AA"),
                                   1590,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 10, 8, 25, 0), // 12:25AM PST
                                   DateTime(2004, 10, 10, 15, 44, 0)); //  7:44AM PST

    AirSeg* yvr_yyz = createAirSeg(2,
                                   yvr,
                                   yyz,
                                   false,
                                   std::string("AC"),
                                   1290,
                                   GeoTravelType::ForeignDomestic,
                                   DateTime(2004, 10, 15, 18, 25, 0), // 10:25AM PST
                                   DateTime(2004, 10, 15, 21, 44, 0)); //  3:44PM CST

    AirSeg* yyz_lhr = createAirSeg(3,
                                   yyz,
                                   lhr,
                                   false,
                                   std::string("BA"),
                                   1882,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 15, 18, 30, 0), // 12:30PM CST
                                   DateTime(2004, 10, 16, 1, 8, 0)); //  1:08AM GMT

    AirSeg* lhr_cdg = createAirSeg(4,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 16, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 10, 16, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_jfk = createAirSeg(5,
                                   cdg,
                                   jfk,
                                   false,
                                   std::string("AA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 11, 29, 16, 30, 0)); // 12:30PM EST

    AirSeg* jfk_mex = createAirSeg(6,
                                   jfk,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 30, 22, 59, 0), //  5:59PM EST
                                   DateTime(2004, 12, 2, 6, 24, 0)); // 11:24PM PST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(mex_yvr);
    itin->travelSeg().push_back(yvr_yyz);
    itin->travelSeg().push_back(yyz_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_jfk);
    itin->travelSeg().push_back(jfk_mex);

    trx.travelSeg().push_back(mex_yvr);
    trx.travelSeg().push_back(yvr_yyz);
    trx.travelSeg().push_back(yyz_lhr);
    trx.travelSeg().push_back(lhr_cdg);
    trx.travelSeg().push_back(cdg_jfk);
    trx.travelSeg().push_back(jfk_mex);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mex_yvr);
    fu1->travelSeg().push_back(yvr_yyz);
    fu2->travelSeg().push_back(yyz_lhr);
    fu2->travelSeg().push_back(lhr_cdg);
    fu3->travelSeg().push_back(cdg_jfk);
    fu3->travelSeg().push_back(jfk_mex);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->puType() = PricingUnit::Type::CIRCLETRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);
    pu1->fareUsage().push_back(fu3);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mex_yvr);
    pu1->travelSeg().push_back(yvr_yyz);
    pu1->travelSeg().push_back(yyz_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_jfk);
    pu1->travelSeg().push_back(jfk_mex);

    pu1->turnAroundPoint() = cdg_jfk;

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
    FareMarket* fm1 = createFareMarket(mex, yyz, "AA");
    FareMarket* fm2 = createFareMarket(yyz, cdg, "BA");
    FareMarket* fm3 = createFareMarket(cdg, mex, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mex_yvr);
    fm1->travelSeg().push_back(yvr_yyz);
    fm2->travelSeg().push_back(yyz_lhr);
    fm2->travelSeg().push_back(lhr_cdg);
    fm3->travelSeg().push_back(cdg_jfk);
    fm3->travelSeg().push_back(jfk_mex);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItinRtw(PricingTrx& trx)
  {
    Itin* itin = _memHandle.create<Itin>();

    AirSeg* mex_yyz = createAirSeg(1,
                                   mex,
                                   yyz,
                                   false,
                                   std::string("AA"),
                                   1590,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 10, 8, 25, 0), // 12:25AM PST
                                   DateTime(2004, 10, 10, 15, 44, 0)); //  7:44AM PST
    SurfaceSeg* yyz_lhr = createSurfaceSeg(2, yyz, lhr, false, GeoTravelType::International);
    AirSeg* lhr_cdg = createAirSeg(3,
                                   lhr,
                                   cdg,
                                   true,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 11, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 10, 11, 8, 40, 0)); //  9:40AM
    SurfaceSeg* cdg_jfk = createSurfaceSeg(4, cdg, jfk, false, GeoTravelType::International);
    AirSeg* jfk_mex = createAirSeg(5,
                                   jfk,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   2601,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 12, 22, 59, 0), //  5:59PM EST
                                   DateTime(2004, 10, 13, 6, 24, 0)); // 11:24PM PST

    itin->travelSeg().push_back(mex_yyz);
    itin->travelSeg().push_back(yyz_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_jfk);
    itin->travelSeg().push_back(jfk_mex);
    itin->calculationCurrency() = "USD";

    FareMarket* fm = createFareMarket(mex, mex, "AA");
    fm->travelSeg() = itin->travelSeg();
    fm->setGlobalDirection(GlobalDirection::RW);

    Fare* fare = createFare(
        fm, Fare::FS_International, GlobalDirection::RW, ROUND_TRIP_MAYNOT_BE_HALVED, "USD");
    PaxTypeFare* ptf = createPaxTypeFare(fare, *fm, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->travelSeg() = itin->travelSeg();
    fu->inbound() = false;
    fu->paxTypeFare() = ptf;

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->puType() = PricingUnit::Type::ROUNDTHEWORLD_SFC;
    pu->fareUsage().push_back(fu);
    pu->travelSeg() = itin->travelSeg();
    pu->turnAroundPoint() = lhr_cdg;

    FarePath* fp = _memHandle.create<FarePath>();
    fp->paxType() = ptf->actualPaxType();
    fp->pricingUnit().push_back(pu);
    fp->itin() = itin;

    itin->farePath().push_back(fp);
    itin->fareMarket().push_back(fm);
    trx.itin().push_back(itin);
    trx.travelSeg() = itin->travelSeg();
    return itin;
  }

  Itin* createItinRtwLimitPerNation(PricingTrx& trx)
  {
    Itin* itin = _memHandle.create<Itin>();

    AirSeg* mex_yyz = createAirSeg(1,
                                   mex,
                                   yyz,
                                   false,
                                   std::string("AA"),
                                   123,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 10, 8, 00, 0),
                                   DateTime(2004, 10, 10, 15, 00, 0));
    AirSeg* yyz_yvr = createAirSeg(2,
                                   yyz,
                                   yvr,
                                   false,
                                   std::string("AA"),
                                   234,
                                   GeoTravelType::Domestic,
                                   DateTime(2004, 10, 10, 16, 00, 0),
                                   DateTime(2004, 10, 10, 18, 00, 0));
    AirSeg* yvr_lhr = createAirSeg(3,
                                   yvr,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   345,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 11, 6, 00, 0),
                                   DateTime(2004, 10, 11, 14, 00, 0));
    AirSeg* lhr_nrt = createAirSeg(4,
                                   lhr,
                                   nrt,
                                   false,
                                   std::string("AA"),
                                   456,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 11, 16, 00, 0),
                                   DateTime(2004, 10, 11, 23, 00, 0));
    AirSeg* nrt_mex = createAirSeg(4,
                                   nrt,
                                   mex,
                                   false,
                                   std::string("AA"),
                                   567,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 12, 6, 00, 0),
                                   DateTime(2004, 10, 12, 12, 00, 0));


    itin->travelSeg().push_back(mex_yyz);
    itin->travelSeg().push_back(yyz_yvr);
    itin->travelSeg().push_back(yvr_lhr);
    itin->travelSeg().push_back(lhr_nrt);
    itin->travelSeg().push_back(nrt_mex);
    itin->calculationCurrency() = "USD";

    FareMarket* fm = createFareMarket(mex, mex, "AA");
    fm->travelSeg() = itin->travelSeg();
    fm->setGlobalDirection(GlobalDirection::RW);

    Fare* fare = createFare(
        fm, Fare::FS_International, GlobalDirection::RW, ROUND_TRIP_MAYNOT_BE_HALVED, "USD");
    PaxTypeFare* ptf = createPaxTypeFare(fare, *fm, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->travelSeg() = itin->travelSeg();
    fu->inbound() = false;
    fu->paxTypeFare() = ptf;

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->puType() = PricingUnit::Type::ROUNDTHEWORLD_SFC;
    pu->fareUsage().push_back(fu);
    pu->travelSeg() = itin->travelSeg();
    pu->turnAroundPoint() = yvr_lhr;

    FarePath* fp = _memHandle.create<FarePath>();
    fp->paxType() = ptf->actualPaxType();
    fp->pricingUnit().push_back(pu);
    fp->itin() = itin;

    itin->farePath().push_back(fp);
    itin->fareMarket().push_back(fm);
    trx.itin().push_back(itin);
    trx.travelSeg() = itin->travelSeg();
    return itin;
  }

  Itin* createItinTestSurface(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments
    //
    AirSeg* mia_stt = createAirSeg(1,
                                   mia,
                                   stt,
                                   false,
                                   std::string("AA"),
                                   671,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 10, 15, 17, 5, 0), // 12:05PM EDT
                                   DateTime(2006, 10, 15, 19, 35, 0)); //  2:35PM EDT

    SurfaceSeg* stt_cun = _memHandle.create<SurfaceSeg>();
    stt_cun->pnrSegment() = 2;
    stt_cun->origin() = stt;
    stt_cun->destination() = cun;
    stt_cun->origAirport() = "STT";
    stt_cun->destAirport() = "CUN";
    stt_cun->stopOver() = false;
    stt_cun->geoTravelType() = GeoTravelType::International;
    stt_cun->departureDT() = DateTime(2006, 10, 15, 19, 35, 0); // 2:35PM EDT
    stt_cun->arrivalDT() = DateTime(2006, 10, 22, 13, 7, 0); // 7:07AM CDT

    AirSeg* cun_dfw = createAirSeg(3,
                                   cun,
                                   dfw,
                                   false,
                                   std::string("MX"),
                                   3080,
                                   GeoTravelType::International,
                                   DateTime(2006, 10, 22, 13, 7, 0), //  7:07AM CDT
                                   DateTime(2006, 10, 22, 15, 46, 0)); //  9:46AM CDT

    SurfaceSeg* dfw_lax = _memHandle.create<SurfaceSeg>();
    dfw_lax->pnrSegment() = 4;
    dfw_lax->origin() = dfw;
    dfw_lax->destination() = lax;
    dfw_lax->origAirport() = "DFW";
    dfw_lax->destAirport() = "LAX";
    dfw_lax->stopOver() = false;
    dfw_lax->geoTravelType() = GeoTravelType::Domestic;
    dfw_lax->departureDT() = DateTime(2006, 10, 22, 15, 46, 0); // 7:07AM CDT
    dfw_lax->arrivalDT() = DateTime(2006, 10, 27, 16, 30, 0); // 1:52PM PDT

    AirSeg* lax_hnl = createAirSeg(5,
                                   lax,
                                   hnl,
                                   false,
                                   std::string("AA"),
                                   31,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 10, 27, 16, 30, 0), //  8:30AM PDT
                                   DateTime(2006, 10, 27, 21, 15, 0)); // 11:15AM HST

    AirSeg* hnl_dfw = createAirSeg(6,
                                   hnl,
                                   dfw,
                                   false,
                                   std::string("AA"),
                                   270,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 11, 3, 18, 25, 0), // 8:25AM HST
                                   DateTime(2006, 11, 3, 21, 40, 0)); // 3:40PM CST

    AirSeg* dfw_mia = createAirSeg(7,
                                   dfw,
                                   mia,
                                   false,
                                   std::string("AA"),
                                   436,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 11, 3, 23, 50, 0), // 5:50PM CST
                                   DateTime(2006, 11, 3, 15, 35, 0)); // 9:35PM EST

    SurfaceSeg* mia_cun = _memHandle.create<SurfaceSeg>();
    mia_cun->pnrSegment() = 8;
    mia_cun->origin() = mia;
    mia_cun->destination() = cun;
    mia_cun->origAirport() = "MIA";
    mia_cun->destAirport() = "CUN";
    mia_cun->stopOver() = false;
    mia_cun->geoTravelType() = GeoTravelType::International;
    mia_cun->departureDT() = DateTime(2006, 11, 3, 15, 35, 0); // 9:35PM EST
    mia_cun->arrivalDT() = DateTime(2006, 11, 10, 13, 0, 0); // 7:00AM CST

    AirSeg* cun_mex = createAirSeg(9,
                                   cun,
                                   mex,
                                   true,
                                   std::string("MX"),
                                   340,
                                   GeoTravelType::ForeignDomestic,
                                   DateTime(2006, 11, 10, 13, 0, 0), // 7:00AM CST
                                   DateTime(2006, 11, 10, 15, 10, 0)); // 9:10AM CST

    AirSeg* mex_dfw = createAirSeg(10,
                                   mex,
                                   dfw,
                                   true,
                                   std::string("AM"),
                                   844,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 10, 18, 10, 0), // 12:10PM CST
                                   DateTime(2006, 11, 10, 20, 35, 0)); //  2:35PM CST

    SurfaceSeg* dfw_bos = _memHandle.create<SurfaceSeg>();
    dfw_bos->pnrSegment() = 11;
    dfw_bos->origin() = dfw;
    dfw_bos->destination() = bos;
    dfw_bos->origAirport() = "DFW";
    dfw_bos->destAirport() = "BOS";
    dfw_bos->stopOver() = true;
    dfw_bos->geoTravelType() = GeoTravelType::Domestic;
    dfw_bos->departureDT() = DateTime(2006, 11, 10, 20, 35, 0); // 2:35PM CST
    dfw_bos->arrivalDT() = DateTime(2006, 11, 15, 12, 25, 0); // 7:25AM EST

    AirSeg* bos_mia = createAirSeg(12,
                                   bos,
                                   mia,
                                   false,
                                   std::string("AA"),
                                   2607,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 11, 15, 12, 25, 0), //  7:25AM EST
                                   DateTime(2006, 11, 15, 15, 50, 0)); // 10:50AM EST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(mia_stt);
    itin->travelSeg().push_back(stt_cun);
    itin->travelSeg().push_back(cun_dfw);
    itin->travelSeg().push_back(dfw_lax);
    itin->travelSeg().push_back(lax_hnl);
    itin->travelSeg().push_back(hnl_dfw);
    itin->travelSeg().push_back(dfw_mia);
    itin->travelSeg().push_back(mia_cun);
    itin->travelSeg().push_back(cun_mex);
    itin->travelSeg().push_back(mex_dfw);
    itin->travelSeg().push_back(dfw_bos);
    itin->travelSeg().push_back(bos_mia);

    trx.travelSeg().push_back(mia_stt);
    trx.travelSeg().push_back(stt_cun);
    trx.travelSeg().push_back(cun_dfw);
    trx.travelSeg().push_back(dfw_lax);
    trx.travelSeg().push_back(lax_hnl);
    trx.travelSeg().push_back(hnl_dfw);
    trx.travelSeg().push_back(dfw_mia);
    trx.travelSeg().push_back(mia_cun);
    trx.travelSeg().push_back(cun_mex);
    trx.travelSeg().push_back(mex_dfw);
    trx.travelSeg().push_back(dfw_bos);
    trx.travelSeg().push_back(bos_mia);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();
    FareUsage* fu4 = _memHandle.create<FareUsage>();
    FareUsage* fu5 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(mia_stt);
    fu1->travelSeg().push_back(stt_cun);
    fu1->travelSeg().push_back(cun_dfw);
    fu2->travelSeg().push_back(dfw_lax);
    fu2->travelSeg().push_back(lax_hnl);
    fu3->travelSeg().push_back(hnl_dfw);
    fu3->travelSeg().push_back(dfw_mia);
    fu3->travelSeg().push_back(mia_cun);
    fu3->travelSeg().push_back(cun_mex);
    fu4->travelSeg().push_back(mex_dfw);
    fu4->travelSeg().push_back(dfw_bos);
    fu5->travelSeg().push_back(bos_mia);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = false;
    fu3->inbound() = false;
    fu4->inbound() = true;
    fu4->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    PricingUnit* pu2 = _memHandle.create<PricingUnit>();
    PricingUnit* pu3 = _memHandle.create<PricingUnit>();
    PricingUnit* pu4 = _memHandle.create<PricingUnit>();
    PricingUnit* pu5 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu2->fareUsage().push_back(fu2);
    pu3->fareUsage().push_back(fu3);
    pu4->fareUsage().push_back(fu4);
    pu5->fareUsage().push_back(fu5);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(mia_stt);
    pu1->travelSeg().push_back(stt_cun);
    pu1->travelSeg().push_back(cun_dfw);
    pu2->travelSeg().push_back(dfw_lax);
    pu2->travelSeg().push_back(lax_hnl);
    pu3->travelSeg().push_back(hnl_dfw);
    pu3->travelSeg().push_back(dfw_mia);
    pu3->travelSeg().push_back(mia_cun);
    pu3->travelSeg().push_back(cun_mex);
    pu4->travelSeg().push_back(mex_dfw);
    pu4->travelSeg().push_back(dfw_bos);
    pu5->travelSeg().push_back(bos_mia);

    // Create a fare path
    //
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    //
    fp->pricingUnit().push_back(pu1);
    fp->pricingUnit().push_back(pu2);
    fp->pricingUnit().push_back(pu3);
    fp->pricingUnit().push_back(pu4);
    fp->pricingUnit().push_back(pu5);

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

    fm1->origin() = mia;
    fm1->destination() = dfw;
    fm2->origin() = dfw;
    fm2->destination() = hnl;
    fm3->origin() = hnl;
    fm3->destination() = mex;
    fm4->origin() = mex;
    fm4->destination() = bos;
    fm5->origin() = bos;
    fm5->destination() = mia;

    fm1->governingCarrier() = "AA";
    fm2->governingCarrier() = "AA";
    fm3->governingCarrier() = "AA";
    fm4->governingCarrier() = "AA";
    fm5->governingCarrier() = "AA";

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f2 =
        createFare(fm2, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f4 =
        createFare(fm4, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f5 =
        createFare(fm5, Fare::FS_Domestic, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "USD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf4 = createPaxTypeFare(f4, *fm4, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf5 = createPaxTypeFare(f5, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;
    fu4->paxTypeFare() = ptf4;
    fu5->paxTypeFare() = ptf5;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(mia_stt);
    fm1->travelSeg().push_back(stt_cun);
    fm1->travelSeg().push_back(cun_dfw);
    fm2->travelSeg().push_back(dfw_lax);
    fm2->travelSeg().push_back(lax_hnl);
    fm3->travelSeg().push_back(hnl_dfw);
    fm3->travelSeg().push_back(dfw_mia);
    fm3->travelSeg().push_back(mia_cun);
    fm3->travelSeg().push_back(cun_mex);
    fm4->travelSeg().push_back(mex_dfw);
    fm4->travelSeg().push_back(dfw_bos);
    fm5->travelSeg().push_back(bos_mia);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);
    fm3->setGlobalDirection(GlobalDirection::ZZ);
    fm4->setGlobalDirection(GlobalDirection::ZZ);
    fm5->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    itin->fareMarket().push_back(fm3);
    itin->fareMarket().push_back(fm4);
    itin->fareMarket().push_back(fm5);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItinRegressionPL7907(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "JPY";

    // Create the travel segments
    //
    AirSeg* fuk_hkg = createAirSeg(1,
                                   fuk,
                                   hkg,
                                   false,
                                   std::string("CX"),
                                   511,
                                   GeoTravelType::International,
                                   DateTime(2006, 8, 2, 1, 50, 0), // 10:50AM GMT+9
                                   DateTime(2006, 8, 2, 7, 0, 0)); //  3:00PM GMT+8

    AirSeg* hkg_syd = createAirSeg(2,
                                   hkg,
                                   syd,
                                   false,
                                   std::string("CX"),
                                   111,
                                   GeoTravelType::International,
                                   DateTime(2006, 8, 2, 11, 10, 0), // 7:10PM GMT+8
                                   DateTime(2006, 8, 2, 19, 15, 0)); // 6:15AM GMT+11

    AirSeg* syd_cbr = createAirSeg(3,
                                   syd,
                                   cbr,
                                   true,
                                   std::string("QF"),
                                   1409,
                                   GeoTravelType::ForeignDomestic,
                                   DateTime(2006, 8, 2, 21, 45, 0), // 8:45AM GMT+11
                                   DateTime(2006, 8, 2, 22, 35, 0)); // 9:35AM GMT+11

    AirSeg* cbr_syd = createAirSeg(4,
                                   cbr,
                                   syd,
                                   true,
                                   std::string("QF"),
                                   1410,
                                   GeoTravelType::ForeignDomestic,
                                   DateTime(2006, 11, 12, 23, 0, 0), // 10:00AM GMT+11
                                   DateTime(2006, 11, 12, 23, 50, 0)); // 10:50AM GMT+11

    AirSeg* syd_hkg = createAirSeg(5,
                                   syd,
                                   hkg,
                                   false,
                                   std::string("CX"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 12, 5, 15, 0), // 4:15PM GMT+11
                                   DateTime(2006, 11, 12, 14, 25, 0)); // 10:25PM GMT+8

    AirSeg* hkg_fuk = createAirSeg(6,
                                   hkg,
                                   fuk,
                                   false,
                                   std::string("CX"),
                                   510,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 13, 7, 45, 0), // 3:15PM GMT+8
                                   DateTime(2006, 11, 13, 11, 55, 0)); // 8:55PM GMT+9

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(fuk_hkg);
    itin->travelSeg().push_back(hkg_syd);
    itin->travelSeg().push_back(syd_cbr);
    itin->travelSeg().push_back(cbr_syd);
    itin->travelSeg().push_back(syd_hkg);
    itin->travelSeg().push_back(hkg_fuk);

    trx.travelSeg().push_back(fuk_hkg);
    trx.travelSeg().push_back(hkg_syd);
    trx.travelSeg().push_back(syd_cbr);
    trx.travelSeg().push_back(cbr_syd);
    trx.travelSeg().push_back(syd_hkg);
    trx.travelSeg().push_back(hkg_fuk);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(fuk_hkg);
    fu1->travelSeg().push_back(hkg_syd);
    fu1->travelSeg().push_back(syd_cbr);
    fu2->travelSeg().push_back(cbr_syd);
    fu2->travelSeg().push_back(syd_hkg);
    fu2->travelSeg().push_back(hkg_fuk);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(fuk_hkg);
    pu1->travelSeg().push_back(hkg_syd);
    pu1->travelSeg().push_back(syd_cbr);
    pu1->travelSeg().push_back(cbr_syd);
    pu1->travelSeg().push_back(syd_hkg);
    pu1->travelSeg().push_back(hkg_fuk);

    pu1->turnAroundPoint() = cbr_syd;

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
    FareMarket* fm1 = createFareMarket(fuk, cbr, "CX");
    FareMarket* fm2 = createFareMarket(cbr, fuk, "CX");

    Fare* f1 = createFare(
        fm1, Fare::FS_International, GlobalDirection::ZZ, ROUND_TRIP_MAYNOT_BE_HALVED, "JPY");
    Fare* f2 = createFare(
        fm2, Fare::FS_International, GlobalDirection::ZZ, ROUND_TRIP_MAYNOT_BE_HALVED, "JPY");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(fuk_hkg);
    fm1->travelSeg().push_back(hkg_syd);
    fm1->travelSeg().push_back(syd_cbr);
    fm2->travelSeg().push_back(cbr_syd);
    fm2->travelSeg().push_back(syd_hkg);
    fm2->travelSeg().push_back(hkg_fuk);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItinRegressionPL8118(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "EUR";

    // Create the travel segments
    //
    AirSeg* tia_vie = createAirSeg(1,
                                   tia,
                                   vie,
                                   false,
                                   std::string("OS"),
                                   850,
                                   GeoTravelType::International,
                                   DateTime(2006, 9, 1, 4, 0, 0), // 5:00AM GMT+1
                                   DateTime(2006, 9, 1, 5, 35, 0)); // 6:35AM GMT+1

    AirSeg* vie_zrh = createAirSeg(2,
                                   vie,
                                   zrh,
                                   false,
                                   std::string("OS"),
                                   561,
                                   GeoTravelType::International,
                                   DateTime(2006, 9, 1, 6, 5, 0), // 7:05AM GMT+1
                                   DateTime(2006, 9, 1, 7, 40, 0)); // 8:40AM GMT+1

    AirSeg* zrh_yul = createAirSeg(3,
                                   zrh,
                                   yul,
                                   false,
                                   std::string("LX"),
                                   86,
                                   GeoTravelType::International,
                                   DateTime(2006, 9, 1, 11, 55, 0), // 12:55PM GMT+1
                                   DateTime(2006, 9, 1, 20, 5, 0)); // 3:05PM GMT-5

    AirSeg* yul_yqb = createAirSeg(4,
                                   yul,
                                   yqb,
                                   true,
                                   std::string("AC"),
                                   8722,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 9, 1, 22, 0, 0), // 5:00PM GMT-5
                                   DateTime(2006, 9, 1, 22, 48, 0)); // 5:48PM GMT-5

    AirSeg* yqb_yul = createAirSeg(5,
                                   yqb,
                                   yul,
                                   false,
                                   std::string("AC"),
                                   8717,
                                   GeoTravelType::Domestic,
                                   DateTime(2006, 11, 2, 19, 15, 0), // 2:15PM GMT-5
                                   DateTime(2006, 11, 2, 23, 25, 0)); // 6:25PM GMT-5

    AirSeg* yul_zrh = createAirSeg(6,
                                   yul,
                                   zrh,
                                   false,
                                   std::string("LX"),
                                   87,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 2, 22, 0, 0), // 5:00PM GMT-5
                                   DateTime(2006, 11, 3, 5, 25, 0)); // 6:25AM GMT+1

    AirSeg* zrh_vie = createAirSeg(7,
                                   zrh,
                                   vie,
                                   false,
                                   std::string("OS"),
                                   568,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 3, 6, 40, 0), // 7:40AM GMT+1
                                   DateTime(2006, 11, 3, 8, 10, 0)); // 9:10AM GMT+1

    AirSeg* vie_tia = createAirSeg(8,
                                   vie,
                                   tia,
                                   false,
                                   std::string("OS"),
                                   847,
                                   GeoTravelType::International,
                                   DateTime(2006, 11, 3, 9, 50, 0), // 10:50AM GMT+1
                                   DateTime(2006, 11, 3, 11, 45, 0)); // 12:45PM GMT+1

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(tia_vie);
    itin->travelSeg().push_back(vie_zrh);
    itin->travelSeg().push_back(zrh_yul);
    itin->travelSeg().push_back(yul_yqb);
    itin->travelSeg().push_back(yqb_yul);
    itin->travelSeg().push_back(yul_zrh);
    itin->travelSeg().push_back(zrh_vie);
    itin->travelSeg().push_back(vie_tia);

    trx.travelSeg().push_back(tia_vie);
    trx.travelSeg().push_back(vie_zrh);
    trx.travelSeg().push_back(zrh_yul);
    trx.travelSeg().push_back(yul_yqb);
    trx.travelSeg().push_back(yqb_yul);
    trx.travelSeg().push_back(yul_zrh);
    trx.travelSeg().push_back(zrh_vie);
    trx.travelSeg().push_back(vie_tia);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(tia_vie);
    fu1->travelSeg().push_back(vie_zrh);
    fu1->travelSeg().push_back(zrh_yul);
    fu1->travelSeg().push_back(yul_yqb);
    fu2->travelSeg().push_back(yqb_yul);
    fu2->travelSeg().push_back(yul_zrh);
    fu2->travelSeg().push_back(zrh_vie);
    fu2->travelSeg().push_back(vie_tia);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(tia_vie);
    pu1->travelSeg().push_back(vie_zrh);
    pu1->travelSeg().push_back(zrh_yul);
    pu1->travelSeg().push_back(yul_yqb);
    pu1->travelSeg().push_back(yqb_yul);
    pu1->travelSeg().push_back(yul_zrh);
    pu1->travelSeg().push_back(zrh_vie);
    pu1->travelSeg().push_back(vie_tia);

    pu1->turnAroundPoint() = yqb_yul;

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
    FareMarket* fm1 = createFareMarket(tia, yqb, "OS");
    FareMarket* fm2 = createFareMarket(yqb, tia, "OS");

    Fare* f1 = createFare(
        fm1, Fare::FS_International, GlobalDirection::ZZ, ROUND_TRIP_MAYNOT_BE_HALVED, "JPY");
    Fare* f2 = createFare(
        fm2, Fare::FS_International, GlobalDirection::ZZ, ROUND_TRIP_MAYNOT_BE_HALVED, "JPY");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(tia_vie);
    fm1->travelSeg().push_back(vie_zrh);
    fm1->travelSeg().push_back(zrh_yul);
    fm1->travelSeg().push_back(yul_yqb);
    fm2->travelSeg().push_back(yqb_yul);
    fm2->travelSeg().push_back(yul_zrh);
    fm2->travelSeg().push_back(zrh_vie);
    fm2->travelSeg().push_back(vie_tia);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItinRegressionPL11648(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "EUR";

    // Create the travel segments
    //
    AirSeg* lux_lhr = createAirSeg(1,
                                   lux,
                                   lhr,
                                   false,
                                   std::string("LG"),
                                   4403,
                                   GeoTravelType::International,
                                   DateTime(2006, 1, 27, 11, 50, 0), // 12:50PM GMT+1
                                   DateTime(2006, 1, 27, 13, 0, 0)); //  1:00PM GMT

    AirSeg* lhr_bkk = createAirSeg(2,
                                   lhr,
                                   bkk,
                                   true,
                                   std::string("QF"),
                                   302,
                                   GeoTravelType::International,
                                   DateTime(2006, 1, 27, 21, 25, 0), // 9:25PM GMT
                                   DateTime(2006, 1, 28, 8, 40, 0)); // 3:40PM GMT+7

    SurfaceSeg* bkk_hkg = _memHandle.create<SurfaceSeg>();
    bkk_hkg->pnrSegment() = 3;
    bkk_hkg->origin() = bkk;
    bkk_hkg->destination() = hkg;
    bkk_hkg->stopOver() = false;
    bkk_hkg->geoTravelType() = GeoTravelType::International;
    bkk_hkg->departureDT() = DateTime(2006, 1, 28, 8, 40, 0); //  3:40PM GMT+7
    bkk_hkg->arrivalDT() = DateTime(2006, 2, 10, 12, 30, 0); //  8:30PM GMT+8

    AirSeg* hkg_syd = createAirSeg(4,
                                   hkg,
                                   syd,
                                   true,
                                   std::string("QF"),
                                   128,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 10, 12, 30, 0), // 8:30PM GMT+8
                                   DateTime(2006, 2, 10, 21, 20, 0)); // 8:20AM GMT+11

    AirSeg* syd_hkg = createAirSeg(5,
                                   syd,
                                   hkg,
                                   true,
                                   std::string("QF"),
                                   29,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 16, 7, 0, 0), // 6:00PM GMT+11
                                   DateTime(2006, 2, 16, 15, 50, 0)); // 11:50PM GMT+8

    AirSeg* hkg_lhr = createAirSeg(6,
                                   hkg,
                                   lhr,
                                   true,
                                   std::string("BA"),
                                   32,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 18, 15, 45, 0), // 11:45PM GMT+8
                                   DateTime(2006, 2, 19, 5, 0, 0)); // 5:00AM GMT

    AirSeg* lhr_lux = createAirSeg(7,
                                   lhr,
                                   lux,
                                   false,
                                   std::string("LG"),
                                   4402,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 19, 8, 5, 0), // 8:05AM GMT
                                   DateTime(2006, 2, 19, 9, 15, 0)); // 10:15AM GMT+1

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(lux_lhr);
    itin->travelSeg().push_back(lhr_bkk);
    itin->travelSeg().push_back(bkk_hkg);
    itin->travelSeg().push_back(hkg_syd);
    itin->travelSeg().push_back(syd_hkg);
    itin->travelSeg().push_back(hkg_lhr);
    itin->travelSeg().push_back(lhr_lux);

    trx.travelSeg().push_back(lux_lhr);
    trx.travelSeg().push_back(lhr_bkk);
    trx.travelSeg().push_back(bkk_hkg);
    trx.travelSeg().push_back(hkg_syd);
    trx.travelSeg().push_back(syd_hkg);
    trx.travelSeg().push_back(hkg_lhr);
    trx.travelSeg().push_back(lhr_lux);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(lux_lhr);
    fu1->travelSeg().push_back(lhr_bkk);
    fu1->travelSeg().push_back(bkk_hkg);
    fu1->travelSeg().push_back(hkg_syd);
    fu2->travelSeg().push_back(syd_hkg);
    fu2->travelSeg().push_back(hkg_lhr);
    fu2->travelSeg().push_back(lhr_lux);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(lux_lhr);
    pu1->travelSeg().push_back(lhr_bkk);
    pu1->travelSeg().push_back(bkk_hkg);
    pu1->travelSeg().push_back(hkg_syd);
    pu1->travelSeg().push_back(syd_hkg);
    pu1->travelSeg().push_back(hkg_lhr);
    pu1->travelSeg().push_back(lhr_lux);

    pu1->turnAroundPoint() = syd_hkg;

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
    FareMarket* fm1 = createFareMarket(lux, syd, "LG");
    FareMarket* fm2 = createFareMarket(syd, lux, "LG");

    Fare* f1 = createFare(fm1, Fare::FS_International, ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");
    Fare* f2 = createFare(fm2, Fare::FS_International, ZZ, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(lux_lhr);
    fm1->travelSeg().push_back(lhr_bkk);
    fm1->travelSeg().push_back(bkk_hkg);
    fm1->travelSeg().push_back(hkg_syd);
    fm2->travelSeg().push_back(syd_hkg);
    fm2->travelSeg().push_back(hkg_lhr);
    fm2->travelSeg().push_back(lhr_lux);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  Itin* createItinRegressionPL20281(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "SGD";

    // Create the travel segments
    //
    AirSeg* sin_hkg = createAirSeg(1,
                                   sin,
                                   hkg,
                                   false,
                                   std::string("CX"),
                                   710,
                                   GeoTravelType::International,
                                   DateTime(2006, 1, 27, 11, 50, 0), // 12:50PM GMT+1
                                   DateTime(2006, 1, 27, 13, 0, 0)); //  1:00PM GMT

    AirSeg* hkg_kix = createAirSeg(2,
                                   hkg,
                                   nrt,
                                   false,
                                   std::string("JL"),
                                   702,
                                   GeoTravelType::International,
                                   DateTime(2006, 1, 27, 21, 25, 0), // 9:25PM GMT
                                   DateTime(2006, 1, 28, 8, 40, 0)); // 3:40PM GMT+7

    AirSeg* kix_fuk = createAirSeg(3,
                                   nrt,
                                   fuk,
                                   true,
                                   std::string("JL"),
                                   2567,
                                   GeoTravelType::International,
                                   DateTime(2006, 1, 28, 8, 40, 0), //  3:40PM GMT+7
                                   DateTime(2006, 2, 10, 12, 30, 0)); //  8:30PM GMT+8

    AirSeg* fuk_kix = createAirSeg(4,
                                   fuk,
                                   nrt,
                                   false,
                                   std::string("JL"),
                                   2560,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 10, 12, 30, 0), // 8:30PM GMT+8
                                   DateTime(2006, 2, 10, 21, 20, 0)); // 8:20AM GMT+11

    AirSeg* kix_hkg = createAirSeg(5,
                                   nrt,
                                   hkg,
                                   false,
                                   std::string("JL"),
                                   701,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 16, 7, 0, 0), // 6:00PM GMT+11
                                   DateTime(2006, 2, 16, 15, 50, 0)); // 11:50PM GMT+8

    AirSeg* hkg_sin = createAirSeg(6,
                                   hkg,
                                   sin,
                                   false,
                                   std::string("CX"),
                                   735,
                                   GeoTravelType::International,
                                   DateTime(2006, 2, 18, 15, 45, 0), // 11:45PM GMT+8
                                   DateTime(2006, 2, 19, 5, 0, 0)); // 5:00AM GMT

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(sin_hkg);
    itin->travelSeg().push_back(hkg_kix);
    itin->travelSeg().push_back(kix_fuk);
    itin->travelSeg().push_back(fuk_kix);
    itin->travelSeg().push_back(kix_hkg);
    itin->travelSeg().push_back(hkg_sin);

    trx.travelSeg().push_back(sin_hkg);
    trx.travelSeg().push_back(hkg_kix);
    trx.travelSeg().push_back(kix_fuk);
    trx.travelSeg().push_back(fuk_kix);
    trx.travelSeg().push_back(kix_hkg);
    trx.travelSeg().push_back(hkg_sin);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(sin_hkg);
    fu1->travelSeg().push_back(hkg_kix);
    fu1->travelSeg().push_back(kix_fuk);
    fu2->travelSeg().push_back(fuk_kix);
    fu2->travelSeg().push_back(kix_hkg);
    fu2->travelSeg().push_back(hkg_sin);

    // Set the directionality
    fu1->inbound() = false;
    fu2->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu2);

    // Attach the travel segs to the pricing units
    //
    pu1->travelSeg().push_back(sin_hkg);
    pu1->travelSeg().push_back(hkg_kix);
    pu1->travelSeg().push_back(kix_fuk);
    pu1->travelSeg().push_back(fuk_kix);
    pu1->travelSeg().push_back(kix_hkg);
    pu1->travelSeg().push_back(hkg_sin);

    pu1->turnAroundPoint() = fuk_kix;

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
    FareMarket* fm1 = createFareMarket(sin, fuk, "JL");
    FareMarket* fm2 = createFareMarket(fuk, sin, "JL");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "SGD");
    Fare* f2 =
        createFare(fm2, Fare::FS_International, GlobalDirection::ZZ, ONE_WAY_MAY_BE_DOUBLED, "SGD");

    // Create and initialize the PaxTypeFares
    //
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf2 = createPaxTypeFare(f2, *fm2, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    //
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;

    // Attach the travel segs to the fare markets
    //
    fm1->travelSeg().push_back(sin_hkg);
    fm1->travelSeg().push_back(hkg_kix);
    fm1->travelSeg().push_back(kix_fuk);
    fm2->travelSeg().push_back(fuk_kix);
    fm2->travelSeg().push_back(kix_hkg);
    fm2->travelSeg().push_back(hkg_sin);

    fm1->setGlobalDirection(GlobalDirection::ZZ);
    fm2->setGlobalDirection(GlobalDirection::ZZ);

    // Attach the fare markets to the itin
    //
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    //
    trx.itin().push_back(itin);

    return itin;
  }

  void createBaseTransfersInfo1(tse::TransfersInfo1& trInfo1)
  {
    trInfo1.vendor() = "ATP";
    trInfo1.itemNo() = 1;
    trInfo1.unavailTag() = ' ';
    trInfo1.noTransfersMin() = "";
    trInfo1.noTransfersMax() = "XX";
    trInfo1.primeCxrPrimeCxr() = ' ';
    trInfo1.primePrimeMaxTransfers() = "";
    trInfo1.sameCxrSameCxr() = ' ';
    trInfo1.sameSameMaxTransfers() = "";
    trInfo1.primeCxrOtherCxr() = ' ';
    trInfo1.primeOtherMaxTransfers() = "";
    trInfo1.otherCxrOtherCxr() = ' ';
    trInfo1.otherOtherMaxTransfers() = "";
    trInfo1.noOfTransfersOut() = "";
    trInfo1.noOfTransfersIn() = "";
    trInfo1.noOfTransfersAppl() = ' ';
    trInfo1.fareBreakSurfaceInd() = 'Y';
    trInfo1.fareBreakSurfaceTblItemNo() = 0;
    trInfo1.embeddedSurfaceInd() = 'Y';
    trInfo1.embeddedSurfaceTblItemNo() = 0;
    trInfo1.transfersChargeAppl() = ' ';
    trInfo1.maxNoTransfersCharge1() = "";
    trInfo1.maxNoTransfersCharge2() = "";
    trInfo1.charge1Cur1Amt() = 0;
    trInfo1.charge2Cur1Amt() = 0;
    trInfo1.cur1() = "";
    trInfo1.noDec1() = 0;
    trInfo1.charge1Cur2Amt() = 0;
    trInfo1.charge2Cur2Amt() = 0;
    trInfo1.cur2() = "";
    trInfo1.noDec2() = 0;
    trInfo1.inhibit() = ' ';
  }

  void createBaseTransfersInfoSeg1(TransfersInfoSeg1* trInfoSeg1)
  {
    trInfoSeg1->orderNo() = 1;
    trInfoSeg1->transferAppl() = ' ';
    trInfoSeg1->noTransfersPermitted() = "";
    trInfoSeg1->primeOnline() = ' ';
    trInfoSeg1->sameOnline() = ' ';
    trInfoSeg1->primeInterline() = ' ';
    trInfoSeg1->otherInterline() = ' ';
    trInfoSeg1->stopoverConnectInd() = ' ';
    trInfoSeg1->carrierAppl() = ' ';
    trInfoSeg1->carrierIn() = "";
    trInfoSeg1->carrierOut() = "";
    trInfoSeg1->inCarrierApplTblItemNo() = 0;
    trInfoSeg1->outCarrierApplTblItemNo() = 0;
    trInfoSeg1->tsi() = 0;
    trInfoSeg1->loc1().locType() = LOCTYPE_NONE;
    trInfoSeg1->loc1().loc() = "";
    trInfoSeg1->loc2().locType() = LOCTYPE_NONE;
    trInfoSeg1->loc2().loc() = "";
    trInfoSeg1->zoneTblItemNo() = 0;
    trInfoSeg1->betweenAppl() = ' ';
    trInfoSeg1->gateway() = ' ';
    trInfoSeg1->restriction() = ' ';
    trInfoSeg1->outInPortion() = ' ';
    trInfoSeg1->chargeAppl() = ' ';
  }

  void createBaseCategoryRuleInfo(CategoryRuleInfo& crInfo)
  {
    crInfo.categoryNumber() = RuleConst::TRANSFER_RULE;
    crInfo.vendorCode() = "ATP";
    crInfo.tariffNumber() = 1;
    crInfo.carrierCode() = "NW";
    crInfo.ruleNumber() = "0001";
    crInfo.sequenceNumber() = 1;
  }

  void initCategoryRuleInfo(CategoryRuleInfo& crInfo,
                            const TransfersInfo1& trInfo,
                            uint32_t relInd = CategoryRuleItemInfo::THEN)
  {
    if (crInfo.categoryRuleItemInfoSet().empty())
      crInfo.addItemInfoSetNosync(new CategoryRuleItemInfoSet);

    CategoryRuleItemInfoSet& criis = *crInfo.categoryRuleItemInfoSet().back();
    criis.push_back(CategoryRuleItemInfo());

    CategoryRuleItemInfo& crii = criis.back();
    crii.setItemcat(9);
    crii.setOrderNo(uint32_t(criis.size()));
    crii.setItemNo(trInfo.itemNo());
    crii.setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relInd));
    _myDataHandle->setRuleItemInfo(&crInfo, &crii, &trInfo);
  }

  TransfersInfoWrapper& createTrInfoWrapperRtw(const TransfersInfo1& trInfo, PricingUnit& pu)
  {
    CategoryRuleInfo& crInfo = *_memHandle.create<CategoryRuleInfo>();
    createBaseCategoryRuleInfo(crInfo);
    initCategoryRuleInfo(crInfo, trInfo);

    FareUsage* fu = pu.fareUsage().front();
    RuleSetPreprocessor& rsp = *_memHandle.create<RuleSetPreprocessor>();
    rsp.process(*_trx, &crInfo, pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper& trInfoWrapper = *_memHandle.create<TransfersInfoWrapper>(fu);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, pu);

    return trInfoWrapper;
  }


  class TransfersSubclass : public Transfers1
  {
  public:
    bool isSameVCTRSequence(const FareUsage* fu,
                            const FareUsage* ruleFu,
                            const TransfersInfoWrapper& trInfoWrapper)
    {
      return true;
    }
    bool isFCvsPU(const TransfersInfo1& trInfo) { return Transfers1::isFCvsPU(trInfo); }

    bool expediaSPRfixActive(PricingTrx& trx) { return true; }
  };

  class TransfersInfoWrapperSubclass : public TransfersInfoWrapper
  {
  public:
    TransfersInfoWrapperSubclass() : TransfersInfoWrapper(0) {}
    void buildTransfersInfo()
    {
      _trInfo = _memHandle.create<TransfersInfo1>();
      createBaseTransfersInfo(*_trInfo);
    }
    const RuleItemInfo* getRuleItemInfo(PricingTrx& trx, const CategoryRuleItemInfo* cri) const
    {
      return _trInfo;
    }
    void createBaseTransfersInfo(tse::TransfersInfo1& trInfo1)
    {
      trInfo1.vendor() = "ATP";
      trInfo1.itemNo() = 1;
      trInfo1.unavailTag() = ' ';
      trInfo1.noTransfersMin() = "";
      trInfo1.noTransfersMax() = "XX";
      trInfo1.primeCxrPrimeCxr() = ' ';
      trInfo1.primePrimeMaxTransfers() = "";
      trInfo1.sameCxrSameCxr() = ' ';
      trInfo1.sameSameMaxTransfers() = "";
      trInfo1.primeCxrOtherCxr() = ' ';
      trInfo1.primeOtherMaxTransfers() = "";
      trInfo1.otherCxrOtherCxr() = ' ';
      trInfo1.otherOtherMaxTransfers() = "";
      trInfo1.noOfTransfersOut() = "";
      trInfo1.noOfTransfersIn() = "";
      trInfo1.noOfTransfersAppl() = ' ';
      trInfo1.fareBreakSurfaceInd() = 'Y';
      trInfo1.fareBreakSurfaceTblItemNo() = 0;
      trInfo1.embeddedSurfaceInd() = 'Y';
      trInfo1.embeddedSurfaceTblItemNo() = 0;
      trInfo1.transfersChargeAppl() = ' ';
      trInfo1.maxNoTransfersCharge1() = "";
      trInfo1.maxNoTransfersCharge2() = "";
      trInfo1.charge1Cur1Amt() = 0;
      trInfo1.charge2Cur1Amt() = 0;
      trInfo1.cur1() = "";
      trInfo1.noDec1() = 0;
      trInfo1.charge1Cur2Amt() = 0;
      trInfo1.charge2Cur2Amt() = 0;
      trInfo1.cur2() = "";
      trInfo1.noDec2() = 0;
      trInfo1.inhibit() = ' ';
    }

  private:
    TestMemHandle _memHandle;
    TransfersInfo1* _trInfo;
  };

  void testValidateFare0()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FareMarket* fm = _itin->fareMarket()[0];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Test error condition.
    //

    const TransfersInfo1* trInfo = 0;

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, trInfo, *fm);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 0", ret == tse::FAIL);
  }

  void testValidateFare1()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FareMarket* fm = _itin->fareMarket()[0];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 1", finalPass);
  }

  void testValidateFare2()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FareMarket* fm = _itin->fareMarket()[1];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 2", finalPass);
  }

  void testValidateFare3()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FareMarket* fm = _itin->fareMarket()[2];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 3", finalPass);
  }

  void testValidateFare4()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FareMarket* fm = _itin->fareMarket()[3];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 4", finalPass);
  }

  void testValidateFare5()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FareMarket* fm = _itin->fareMarket()[0];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    PaxTypeFare* fare = pu->fareUsage()[0]->paxTypeFare();

    //
    // Minimum 1 transfer required
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMin() = "01";

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 5", !finalPass);
  }

  void testValidateFare6()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FareMarket* fm = _itin->fareMarket()[1];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Stopover record 3 not available for use.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.unavailTag() = 'X';
    trInfo.noTransfersMin() = "01";

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 6", !finalPass);
  }

  void testValidateFare7()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FareMarket* fm = _itin->fareMarket()[1];
    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    PaxTypeFare* fare = pu->fareUsage()[1]->paxTypeFare();

    //
    // Stopover record 3 for text purpose only.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.unavailTag() = 'Y';
    trInfo.noTransfersMin() = "01";

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, *fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, *fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 7", finalPass);
  }

  void testValidateFare8()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FareMarket* fm = _itin->fareMarket()[1];

    FareInfo fi;
    fi._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;

    Fare f;
    f.setFareInfo(&fi);

    PaxTypeFare fare;
    fare.initialize(&f, 0, 0);

    //
    // Round trip fare always passes fare validation.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMin() = "01";

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, *_itin, fare, &trInfoWrapper, *fm);

    bool finalPass = false;

    if ((ret == tse::PASS || ret == tse::SOFTPASS) && (trInfoWrapper.needToProcessResults()))
    {
      Record3ReturnTypes finalRet = trInfoWrapper.processResults(*_trx, fare);
      finalPass = (finalRet == tse::PASS || finalRet == tse::SOFTPASS);
    }
    else
    {
      finalPass = (ret == tse::PASS || ret == tse::SOFTPASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Fare Scope - Test 8", finalPass);
  }

  void testValidatePU0()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Test error condition.
    //

    const TransfersInfo1* trInfo = 0;

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, trInfo, *fp, *pu, *fu);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 0", ret == tse::FAIL);
  }

  void testValidatePU1()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 1", finalPass);
  }

  void testValidatePU2()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin1(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 2", finalPass);
  }

  void testValidatePU3()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 3", finalPass);
  }

  void testValidatePU4()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[2];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited transfers. Stop time >= 4 hours qualifies as a stopover.
    // Transfers permitted in NYC and Zone 1.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_ZONE;
    trInfoSeg2->loc1().loc() = "00001";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 4", finalPass);
  }

  void testValidatePU5()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[2];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // One transfer permitted in FRA, LON, MAD or PAR
    //
    // LHR (LON) is permitted
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "FRA";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc1().loc() = "MAD";
    trInfoSeg2->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc2().loc() = "PAR";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 5", finalPass);
  }

  void testValidatePU6()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin3(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // No transfer permitted in NYC
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "02";
    trInfo.noOfTransfersOut() = "01";
    trInfo.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->transferAppl() = 'N';
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(fu);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 6", !finalPass);
  }

  void testValidatePU6GW()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin3(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Transfer permitted in NYC at  GW
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "02";
    trInfo.noOfTransfersOut() = "01";
    trInfo.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "NYC";
    trInfoSeg1->gateway() = 'X';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 6GW", finalPass);
  }

  void testValidatePU7()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin6(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Maximum of one Primary/Primary transfer at stopover point.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "01";
    trInfo.primeCxrPrimeCxr() = 'X';
    trInfo.primePrimeMaxTransfers() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->stopoverConnectInd() = 'S';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret1 = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    bool finalPass1 = false;
    if ((ret1 == tse::PASS) && (trInfoWrapper.needToProcessResults()))
    {
      finalPass1 = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);
    }
    else
    {
      finalPass1 = (ret1 == tse::PASS);
    }

    bool finalPass2 = false;
    if (finalPass1)
    {
      trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);
      Record3ReturnTypes ret2 = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu2);

      if ((ret2 == tse::PASS) && (trInfoWrapper.needToProcessResults()))
      {
        finalPass2 = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);
      }
      else
      {
        finalPass2 = (ret2 == tse::PASS);
      }
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 7",
                           !(finalPass1 && finalPass2));
  }

  void testValidatePU8()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu2 = pu->fareUsage()[0];
    FareUsage* fu5 = pu->fareUsage()[1];

    //
    // One transfer permitted on either the outbound or inbound
    //  portion of travel, but not both.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "01";
    trInfo.noOfTransfersAppl() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    crInfo.categoryNumber() = RuleConst::TRANSFER_RULE;
    crInfo.vendorCode() = "ATP";
    crInfo.tariffNumber() = 1;
    crInfo.carrierCode() = "NW";
    crInfo.ruleNumber() = "0001";
    crInfo.sequenceNumber() = 1;

    rsp.process(*_trx, &crInfo, *pu, *fu2);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu5);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.crInfo(&crInfo);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 8", ret != tse::PASS);
  }

  void testValidatePU9()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Only 1 transfer permitted on the outbound portion of travel.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "XX";
    trInfo.noOfTransfersOut() = "01";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    crInfo.categoryNumber() = RuleConst::TRANSFER_RULE;
    crInfo.vendorCode() = "ATP";
    crInfo.tariffNumber() = 1;
    crInfo.carrierCode() = "NW";
    crInfo.ruleNumber() = "0001";
    crInfo.sequenceNumber() = 1;

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 9", finalPass);
  }

  void testValidatePU10()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted on outbound.
    // Transfers not permitted on inbound.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noOfTransfersOut() = "XX";
    trInfo.noOfTransfersIn() = "0";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";
    crInfo.sequenceNumber() = 1;

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu2);
    trInfoWrapper1.setCurrentTrInfo(&trInfo);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret1 = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    bool finalPass1 = false;
    bool finalPass2 = false;

    if ((ret1 == tse::PASS) && trInfoWrapper1.needToProcessResults())
    {
      finalPass1 = (trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);
    }
    else
    {
      finalPass1 = (ret1 == tse::PASS);
    }

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.setCurrentTrInfo(&trInfo);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);
    Record3ReturnTypes ret2 = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    if ((ret2 == tse::PASS) && trInfoWrapper2.needToProcessResults())
    {
      finalPass2 = (trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);
    }
    else
    {
      finalPass2 = (ret2 == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 10",
                           !finalPass1 && !finalPass2);
  }

  void testValidatePU12()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    //  Unlimited transfers permitted on inbound. Transfers not permitted
    //   on outbound.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noOfTransfersOut() = "0";
    trInfo.noOfTransfersIn() = "XX";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(fu);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 12", !finalPass);
  }

  void testValidatePU13()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Primary/Primary transfer only permitted at stopover points.
    //  Not connection points.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->primeOnline() = 'X';
    trInfoSeg1->stopoverConnectInd() = 'S';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->sameOnline() = 'X';
    trInfoSeg2->primeInterline() = 'X';
    trInfoSeg2->otherInterline() = 'X';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 13", finalPass);
  }

  void testValidatePU14()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Minimum of 3 transfers required.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMin() = "03";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 14", !finalPass);
  }

  void testValidatePU15()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Stopover record 3 not available for use.
    //

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.unavailTag() = 'X';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 15", !finalPass);
  }

  void testValidatePU16()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin2(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Stopover record 3 for text purpose only.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.unavailTag() = 'Y';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 16", finalPass);
  }

  void testValidatePU20()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu = pu->fareUsage()[0];

    //
    // Unlimited transfers. Primary/Primary permitted at connection points only.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->primeOnline() = 'X';
    trInfoSeg1->stopoverConnectInd() = 'C';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->sameOnline() = 'X';
    trInfoSeg2->primeInterline() = 'X';
    trInfoSeg2->otherInterline() = 'X';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(fu);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    bool finalPass = false;

    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 20A", ret == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 20B", !finalPass);
  }

  void testValidatePU21()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "01";
    trInfo.charge1Cur1Amt() = 100;
    trInfo.charge2Cur1Amt() = 50;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;
    trInfo.charge1Cur2Amt() = 60;
    trInfo.charge2Cur2Amt() = 30;
    trInfo.cur2() = "GBP";
    trInfo.noDec2() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = trInfoWrapper.needToProcessResults() &&
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 1 && fu2->transferSurcharges().size() == 0);

    CPPUNIT_ASSERT((*fu1->transferSurcharges().begin()).second->amount() == 100 &&
                   (*fu1->transferSurcharges().begin()).second->currencyCode() == "USD");
  }

  void testValidatePU22()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted. First transfer free. Second at 60GBP.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "XX";
    trInfo.charge2Cur1Amt() = 100;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;
    trInfo.charge1Cur2Amt() = 60;
    trInfo.cur2() = "GBP";
    trInfo.noDec2() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = !trInfoWrapper.needToProcessResults() ||
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 1 && fu2->transferSurcharges().size() == 0);

    CPPUNIT_ASSERT((*fu1->transferSurcharges().begin()).second->amount() == 0);
  }

  void testValidatePU23()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin5(*_trx, "CNN");

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted. Child discount 50%.
    // First transfer at 50USD, second at 30GBP
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.fareBreakSurfaceInd() = 'Y';
    trInfo.embeddedSurfaceInd() = 'Y';
    trInfo.transfersChargeAppl() = RuleConst::CHARGE_PAX_ADULT_CHILD_DISC;
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "XX";
    trInfo.charge1Cur1Amt() = 100;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;
    trInfo.charge2Cur2Amt() = 60;
    trInfo.cur2() = "GBP";
    trInfo.noDec2() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = trInfoWrapper.needToProcessResults() &&
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 1 && fu2->transferSurcharges().size() == 0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, (*fu1->transferSurcharges().begin()).second->unconvertedAmount(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"),
                         (*fu1->transferSurcharges().begin()).second->unconvertedCurrencyCode());
  }

  void testValidatePU24()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin5(*_trx, "INF");

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.transfersChargeAppl() = RuleConst::CHARGE_PAX_ADULT_CHILD_DISC;
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "XX";
    trInfo.charge1Cur1Amt() = 100;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;
    trInfo.charge1Cur2Amt() = 60;
    trInfo.cur2() = "GBP";
    trInfo.noDec2() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = trInfoWrapper.needToProcessResults() &&
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    // Infant is discounted, but the fare amount is zero so there is no
    //  stopover surcharge for any of the transfers.

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 0 && fu2->transferSurcharges().size() == 0);
  }

  void testValidatePU25()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin6(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // Unlimited transfers permitted.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.transfersChargeAppl() = RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC;
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "XX";
    trInfo.charge1Cur1Amt() = 100;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;
    trInfo.charge1Cur2Amt() = 60;
    trInfo.cur2() = "GBP";
    trInfo.noDec2() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = trInfoWrapper.needToProcessResults() &&
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 1 && fu2->transferSurcharges().size() == 0);

    CPPUNIT_ASSERT((*fu1->transferSurcharges().begin()).second->amount() == 100 &&
                   (*fu1->transferSurcharges().begin()).second->currencyCode() ==
                       "NUC" &&
                   !(*fu1->transferSurcharges().begin()).second->isSegmentSpecific());
  }

  void testValidatePU26()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin6(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Two transfers permitted within the US. First one free, second at 100USD
    // Unlimited transfers outside the US at 100USD
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "01";
    trInfo.maxNoTransfersCharge2() = "XX";
    trInfo.charge2Cur1Amt() = 100;
    trInfo.cur1() = "USD";
    trInfo.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->noTransfersPermitted() = "01";
    trInfoSeg1->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg1->loc1().loc() = "US";
    trInfoSeg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->noTransfersPermitted() = "XX";
    trInfoSeg2->chargeAppl() = '2';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1));

    const bool finalPass = !trInfoWrapper.needToProcessResults() ||
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 1 && fu2->transferSurcharges().size() == 0);

    CPPUNIT_ASSERT((*fu1->transferSurcharges().begin()).second->amount() == 0);
  }

  void testValidatePU27()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin6(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    // Two transfers permitted within the US. First one free, second at 100USD
    // Unlimited transfers outside the US at 100USD
    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "XX";
    trInfo1.charge2Cur1Amt() = 100;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;
    trInfo1.itemNo() = 382926;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "02";
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "XX";
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "02";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "XX";
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;
    trInfo1.itemNo() = 382926;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->noTransfersPermitted() = "02";
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "US";
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "XX";
    trInfo2Seg2->chargeAppl() = '2';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    CategoryRuleItemInfoSet* criis = new CategoryRuleItemInfoSet();
    crInfo.addItemInfoSetNosync(criis);

    CategoryRuleItemInfo crii1;
    crii1.setItemcat(9);
    crii1.setOrderNo(1);
    crii1.setRelationalInd(CategoryRuleItemInfo::THEN);
    crii1.setItemNo(382926);

    CategoryRuleItemInfo crii2;
    crii2.setItemcat(9);
    crii2.setOrderNo(2);
    crii2.setRelationalInd(CategoryRuleItemInfo::OR);
    crii2.setItemNo(382927);

    criis->push_back(crii1);
    criis->push_back(crii2);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapperSubclass trInfoWrapper;
    TransfersSubclass tr;

    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);
    trInfoWrapper.setCurrentTrInfo(&trInfo1);

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu2));

    trInfoWrapper.setCurrentTrInfo(&trInfo2);

    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu2));

    trInfoWrapper.buildTransfersInfo();
    const bool finalPass = trInfoWrapper.needToProcessResults() &&
                           trInfoWrapper.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS;

    CPPUNIT_ASSERT(finalPass);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 0 && fu2->transferSurcharges().size() == 1);

    CPPUNIT_ASSERT((*fu2->transferSurcharges().begin()).second->amount() == 0 &&
                   (*fu2->transferSurcharges().begin()).second->currencyCode() ==
                       "NUC" &&
                   (*fu2->transferSurcharges().begin()).second->isSegmentSpecific());
  }

  void testValidatePU28()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin9(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "XX";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->primeOnline() = 'X';
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "XX";
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->loc1().loc() = "US";
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "XX";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "XX";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->transferAppl() = 'N';
    trInfo2Seg1->primeInterline() = 'X';
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "02";
    trInfo2Seg2->primeInterline() = 'X';
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "GB";
    trInfo2Seg2->chargeAppl() = '2';

    TransfersInfoSeg1* trInfo2Seg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg3);
    trInfo2Seg3->orderNo() = 3;
    trInfo2Seg3->noTransfersPermitted() = "XX";
    trInfo2Seg3->primeInterline() = 'X';
    trInfo2Seg3->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segs().push_back(trInfo2Seg3);
    trInfo2.segCnt() = 3;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(0);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    const Record3ReturnTypes retFu1Final = trInfoWrapper1.needToProcessResults()
                                               ? trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1)
                                               : tse::SKIP;
    CPPUNIT_ASSERT(retFu1Final);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    const Record3ReturnTypes retFu2Final = trInfoWrapper2.needToProcessResults()
                                               ? trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2)
                                               : tse::SKIP;

    CPPUNIT_ASSERT(retFu2Final);

    CPPUNIT_ASSERT(fu1->transfers().size() == 2 && fu2->transfers().size() == 2);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 2 && fu2->transferSurcharges().size() == 2);

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() && (*iter).second->amount() == 20 &&
                   (*iter).second->currencyCode() == "USD" && (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() && (*iter).second->amount() == 100 &&
                   (*iter).second->currencyCode() == "USD" && (*iter).second->isSegmentSpecific());
  }

  void testValidatePU29()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin10(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->primeOnline() = 'X';
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->gateway() = 'X';
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "01";
    trInfo1Seg2->tsi() = 28;
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "XX";
    trInfo2.maxNoTransfersCharge1() = "02";
    trInfo2.maxNoTransfersCharge2() = "XX";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->transferAppl() = 'N';
    trInfo2Seg1->primeInterline() = 'X';
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "02";
    trInfo2Seg2->primeInterline() = 'X';
    trInfo2Seg2->tsi() = 0;
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "GB";
    trInfo2Seg2->chargeAppl() = '2';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg3);
    trInfo2Seg3->orderNo() = 3;
    trInfo2Seg3->noTransfersPermitted() = "XX";
    trInfo2Seg3->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segs().push_back(trInfo2Seg3);
    trInfo2.segCnt() = 3;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 3 && fu2->transfers().size() == 3);

    CPPUNIT_ASSERT(fu1->transferSurcharges().size() == 3 && fu2->transferSurcharges().size() == 3);

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                     (*iter).second->unconvertedAmount() == 50 &&
                     (*iter).second->unconvertedCurrencyCode() == "USD" &&
                     (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                     (*iter).second->unconvertedAmount() == 20 &&
                     (*iter).second->unconvertedCurrencyCode() == "USD" &&
                     (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                     (*iter).second->unconvertedAmount() == 100 &&
                     (*iter).second->unconvertedCurrencyCode() == "USD" &&
                     (*iter).second->isSegmentSpecific());
  }

  void testValidatePU30()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin11(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->primeInterline() = 'X';
    trInfo1Seg1->otherInterline() = 'X';
    trInfo1Seg1->carrierAppl() = 'X';
    trInfo1Seg1->inCarrierApplTblItemNo() = 22; // AA, AX
    trInfo1Seg1->outCarrierApplTblItemNo() = 22; // AA, AX
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "01";
    trInfo1Seg2->carrierIn() = "DL";
    trInfo1Seg2->carrierOut() = "AE";
    trInfo1Seg2->tsi() = 28;
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "04";
    trInfo2.maxNoTransfersCharge1() = "02";
    trInfo2.maxNoTransfersCharge2() = "02";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->transferAppl() = 'N';
    trInfo2Seg1->primeInterline() = 'X';
    trInfo2Seg1->carrierAppl() = 'X';
    trInfo2Seg1->inCarrierApplTblItemNo() = 3001; // AF, BA, etc...
    trInfo2Seg1->outCarrierApplTblItemNo() = 3001;
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "01";
    trInfo2Seg2->primeInterline() = 'X';
    trInfo2Seg2->carrierAppl() = 'X';
    trInfo2Seg2->inCarrierApplTblItemNo() = 3001;
    trInfo2Seg2->outCarrierApplTblItemNo() = 3001;
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "GB";
    trInfo2Seg2->chargeAppl() = '2';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg3);
    trInfo2Seg3->orderNo() = 3;
    trInfo2Seg3->noTransfersPermitted() = "01";
    trInfo2Seg3->primeInterline() = 'X';
    trInfo2Seg3->carrierAppl() = 'X';
    trInfo2Seg3->carrierIn() = "BA";
    trInfo2Seg3->carrierOut() = "AE";
    trInfo2Seg3->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg4 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg4);
    trInfo2Seg4->orderNo() = 4;
    trInfo2Seg4->noTransfersPermitted() = "02";
    trInfo2Seg4->carrierAppl() = 'X';
    trInfo2Seg4->carrierIn() = "AE";
    trInfo2Seg4->carrierOut() = "AA";
    trInfo2Seg4->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segs().push_back(trInfo2Seg3);
    trInfo2.segs().push_back(trInfo2Seg4);
    trInfo2.segCnt() = 4;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));
  }

  void testValidatePU31()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin12(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->primeInterline() = 'X';
    trInfo1Seg1->otherInterline() = 'X';
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "FR";
    trInfo1Seg1->zoneTblItemNo() = 51;
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "01";
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->loc1().loc() = "US";
    trInfo1Seg2->zoneTblItemNo() = 5;
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "04";
    trInfo2.maxNoTransfersCharge1() = "02";
    trInfo2.maxNoTransfersCharge2() = "02";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->transferAppl() = 'N';
    trInfo2Seg1->otherInterline() = 'X';
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "FR";
    trInfo2Seg1->zoneTblItemNo() = 51;
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "01";
    trInfo2Seg2->otherInterline() = 'X';
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "FR";
    trInfo2Seg2->zoneTblItemNo() = 51;
    trInfo2Seg2->chargeAppl() = '2';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg3);
    trInfo2Seg3->orderNo() = 3;
    trInfo2Seg3->noTransfersPermitted() = "01";
    trInfo2Seg3->zoneTblItemNo() = 5;
    trInfo2Seg3->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg4 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg4);
    trInfo2Seg4->orderNo() = 4;
    trInfo2Seg4->noTransfersPermitted() = "02";
    trInfo2Seg4->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg4->loc1().loc() = "US";
    trInfo2Seg4->zoneTblItemNo() = 2002;
    trInfo2Seg4->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segs().push_back(trInfo2Seg3);
    trInfo2.segs().push_back(trInfo2Seg4);
    trInfo2.segCnt() = 4;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) != tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 3 && fu2->transfers().size() == 3);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fu1->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu2->transferSurcharges().size());

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 50 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 75 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 20 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());
  }

  void testValidatePU32()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin12(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "03";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "02";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->primeInterline() = 'X';
    trInfo1Seg1->otherInterline() = 'X';
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->loc2().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc2().loc() = "FR";
    trInfo1Seg1->betweenAppl() = 'X';
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "02";
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->loc1().loc() = "MX";
    trInfo1Seg2->loc2().locType() = LOCTYPE_NATION;
    trInfo1Seg2->loc2().loc() = "GB";
    trInfo1Seg2->betweenAppl() = 'X';
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "03";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "02";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->noTransfersPermitted() = "02";
    trInfo2Seg1->otherInterline() = 'X';
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "FR";
    trInfo2Seg1->zoneTblItemNo() = 5;
    trInfo2Seg1->betweenAppl() = 'X';
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "01";
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "US";
    trInfo2Seg2->zoneTblItemNo() = 954;
    trInfo2Seg2->betweenAppl() = 'X';
    trInfo2Seg2->chargeAppl() = '2';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 3 && fu2->transfers().size() == 3);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fu1->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), fu2->transferSurcharges().size());

    FareUsage::TransferSurchargeMultiMapCI
        iter = fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 50 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 32I",
                           iter != fu1->transferSurcharges().end() &&
                               (*iter).second->unconvertedAmount() == 50 &&
                               (*iter).second->unconvertedCurrencyCode() == "USD" &&
                               (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 20 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());
  }

  void testValidatePU33()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin11(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "03";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "02";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->restriction() = 'B';
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "02";
    trInfo1Seg2->restriction() = 'D';
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "03";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "02";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->noTransfersPermitted() = "01";
    trInfo2Seg1->restriction() = 'B';
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "02";
    trInfo2Seg2->restriction() = 'I';
    trInfo2Seg2->chargeAppl() = '2';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 3 && fu2->transfers().size() == 3);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fu1->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), fu2->transferSurcharges().size());

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 50 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[1]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 20 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu1->transferSurcharges().find(fu1->travelSeg()[2]);
    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 100 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());
  }

  void testValidatePU34()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin13(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    FareUsage* fu3 = pu->fareUsage()[2];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->loc2().locType() = LOCTYPE_NONE;
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "01";
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->loc1().loc() = "CA";
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "01";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "00";
    trInfo2.charge1Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu3);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    RuleSetPreprocessor rsp2;
    CategoryRuleInfo crInfo2;

    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber() = 10;
    crInfo2.carrierCode() = "BA";
    crInfo2.ruleNumber() = "ABCD";

    rsp2.process(*_trx, &crInfo2, *pu, *fu2);
    rsp2.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp2, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper3(0);
    trInfoWrapper3.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper3.setCurrentTrInfo(&trInfo1);
    trInfoWrapper3.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper3, *fp, *pu, *fu3));

    trInfoWrapper3.setCurrentTrInfo(&trInfo2);
    trInfoWrapper3.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper3, *fp, *pu, *fu3));

    CPPUNIT_ASSERT(trInfoWrapper3.needToProcessResults() &&
                   trInfoWrapper3.processResults(*_trx, *fp, *pu, *fu3) == tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1 &&
                   fu3->transfers().size() == 1);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu3->transferSurcharges().size());

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 50 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu2->transferSurcharges().find(fu2->travelSeg()[0]);
    CPPUNIT_ASSERT(iter != fu2->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 100 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());
  }

  void testValidatePU35()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin13(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    FareUsage* fu3 = pu->fareUsage()[2];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "02";
    trInfo1Seg1->loc1().locType() = LOCTYPE_AREA;
    trInfo1Seg1->loc1().loc() = "1";
    trInfo1Seg1->chargeAppl() = '1';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segCnt() = 1;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "01";
    trInfo2.maxNoTransfersCharge1() = "01";
    trInfo2.maxNoTransfersCharge2() = "00";
    trInfo2.charge1Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu3);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    RuleSetPreprocessor rsp2;
    CategoryRuleInfo crInfo2;

    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber() = 10;
    crInfo2.carrierCode() = "BA";
    crInfo2.ruleNumber() = "ABCD";

    rsp2.process(*_trx, &crInfo2, *pu, *fu2);
    rsp2.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper2(0);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp2, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper3(0);
    trInfoWrapper3.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper3.setCurrentTrInfo(&trInfo1);
    trInfoWrapper3.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper3, *fp, *pu, *fu3));

    trInfoWrapper3.setCurrentTrInfo(&trInfo2);
    trInfoWrapper3.setNoTransfersMax(3);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper3, *fp, *pu, *fu3));

    CPPUNIT_ASSERT(trInfoWrapper3.needToProcessResults() &&
                   trInfoWrapper3.processResults(*_trx, *fp, *pu, *fu3) == tse::PASS);

    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1 &&
                   fu3->transfers().size() == 1);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu1->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu2->transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu3->transferSurcharges().size());

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);

    CPPUNIT_ASSERT(iter != fu1->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 20 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());

    iter = fu2->transferSurcharges().find(fu2->travelSeg()[0]);
    CPPUNIT_ASSERT(iter != fu2->transferSurcharges().end() &&
                   (*iter).second->unconvertedAmount() == 100 &&
                   (*iter).second->unconvertedCurrencyCode() == "USD" &&
                   (*iter).second->isSegmentSpecific());
  }

  void testValidatePU36()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin13(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    FareUsage* fu3 = pu->fareUsage()[2];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "XX";

    //
    // Transfers not permitted on the Fare Component
    //
    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noOfTransfersOut() = "00";
    trInfo2.noOfTransfersIn() = "00";

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "AA";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu3);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    RuleSetPreprocessor rsp2;
    CategoryRuleInfo crInfo2;

    createBaseCategoryRuleInfo(crInfo2);
    crInfo2.tariffNumber() = 10;
    crInfo2.carrierCode() = "BA";
    crInfo2.ruleNumber() = "ABCD";
    crInfo2.sequenceNumber() = 1;

    rsp2.process(*_trx, &crInfo2, *pu, *fu2);
    // rsp2.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp2, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) != tse::PASS);

    TransfersInfoWrapper trInfoWrapper3(fu3);
    trInfoWrapper3.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper3.setCurrentTrInfo(&trInfo1);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper3, *fp, *pu, *fu3));

    CPPUNIT_ASSERT(trInfoWrapper3.needToProcessResults() &&
                   trInfoWrapper3.processResults(*_trx, *fp, *pu, *fu3) == tse::PASS);


    CPPUNIT_ASSERT(fu1->transfers().size() == 1 && fu2->transfers().size() == 1 &&
                   fu3->transfers().size() == 1);
  }

  void testValidatePU37()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItin4(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[1];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    //
    // One transfer permitted on either the outbound or inbound
    //  portion of travel, but not both.
    //
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "01";
    trInfo.noOfTransfersAppl() = 'X';
    trInfo.fareBreakSurfaceInd() = 'Y';
    trInfo.embeddedSurfaceInd() = 'Y';

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 37", ret != tse::PASS);
  }

  void testValidate_RtwIgnoreInOutOr()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtw(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "10";
    trInfo.noOfTransfersIn() = "01";
    trInfo.noOfTransfersOut() = "01";
    trInfo.noOfTransfersAppl() = 'X';

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;
    tr.setRtw(true);
    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    CPPUNIT_ASSERT_EQUAL(tse::PASS, ret);
  }

  void testValidate_RtwMaxBlank()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtw(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "";
    trInfo.noOfTransfersIn() = "01";
    trInfo.noOfTransfersOut() = "01";

    CategoryRuleInfo crInfo;
    createBaseCategoryRuleInfo(crInfo);

    RuleSetPreprocessor rsp;
    rsp.process(*_trx, &crInfo, *pu, *fu);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;
    tr.setRtw(true);
    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);

    CPPUNIT_ASSERT_EQUAL(tse::SKIP, ret);
  }

  void testValidate_RtwTransfersPerNation()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtwLimitPerNation(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "XX";
    trInfo.noTransfersMax() = "XX";

    TransfersInfoSeg1* trInfoSeg = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg);
    trInfoSeg->noTransfersPermitted() = "01";
    trInfoSeg->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg->chargeAppl() = '1';
    trInfo.segs().push_back(trInfoSeg);
    trInfo.segCnt() = 1;

    TransfersInfoWrapper& trInfoWrapper = createTrInfoWrapperRtw(trInfo, *pu);
    TransfersSubclass tr;
    tr.setRtw(true);

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);
    if (ret == tse::PASS && trInfoWrapper.needToProcessResults())
      ret = trInfoWrapper.processResults(*_trx, *fp, *pu, *fu);
    CPPUNIT_ASSERT_EQUAL(tse::FAIL, ret);
  }

  void testValidate_RtwTransfersPerNationPass()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtwLimitPerNation(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "XX";
    trInfo.noTransfersMax() = "XX";

    TransfersInfoSeg1* trInfoSeg = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg);
    trInfoSeg->noTransfersPermitted() = "02";
    trInfoSeg->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg->chargeAppl() = '1';
    trInfo.segs().push_back(trInfoSeg);
    trInfo.segCnt() = 1;

    TransfersInfoWrapper& trInfoWrapper = createTrInfoWrapperRtw(trInfo, *pu);
    TransfersSubclass tr;
    tr.setRtw(true);

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);
    if (ret == tse::PASS && trInfoWrapper.needToProcessResults())
      ret = trInfoWrapper.processResults(*_trx, *fp, *pu, *fu);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, ret);
  }

  void testValidate_RtwPermittedReachedSameCharge()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtwLimitPerNation(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    // Two transfers in Canada on itinerary.
    // Therefore, we should fail even though the second rec. segment has unlimited
    // NO transfers permitted (since they have the same charge number)
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->noTransfersPermitted() = "01";
    trInfoSeg1->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg1->loc1().loc() = "CA";
    trInfoSeg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->noTransfersPermitted() = "XX";
    trInfoSeg2->chargeAppl() = '1';

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "XX";
    trInfo.noTransfersMax() = "XX";
    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper& trInfoWrapper = createTrInfoWrapperRtw(trInfo, *pu);
    TransfersSubclass tr;
    tr.setRtw(true);

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);
    if (ret == tse::PASS && trInfoWrapper.needToProcessResults())
      ret = trInfoWrapper.processResults(*_trx, *fp, *pu, *fu);
    CPPUNIT_ASSERT_EQUAL(tse::FAIL, ret);
  }

  void testValidate_RtwPermittedReachedOtherCharge()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtwLimitPerNation(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu = pu->fareUsage()[0];

    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->noTransfersPermitted() = "01";
    trInfoSeg1->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg1->loc1().loc() = "CA";
    trInfoSeg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();
    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->noTransfersPermitted() = "XX";
    trInfoSeg2->chargeAppl() = '2';

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.maxNoTransfersCharge1() = "XX";
    trInfo.noTransfersMax() = "XX";
    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segCnt() = 2;

    TransfersInfoWrapper& trInfoWrapper = createTrInfoWrapperRtw(trInfo, *pu);
    TransfersSubclass tr;
    tr.setRtw(true);

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu);
    if (ret == tse::PASS && trInfoWrapper.needToProcessResults())
      ret = trInfoWrapper.processResults(*_trx, *fp, *pu, *fu);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, ret);
  }

  void testProcessSurfaceRestrictions_RtwMaxEmbeddedExceeded()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtw(*_trx);

    const FareMarket& fm = *_itin->fareMarket()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.embeddedSurfaceInd() = '1';
    trInfo.embeddedSurfaceTblItemNo() = 40;

    TransfersSubclass tr;
    tr.setRtw(true);
    Transfers1::ProcessingResult ret = tr.processSurfaceSectorRestrictions(*_trx, trInfo, fm, 0);

    CPPUNIT_ASSERT_EQUAL(Transfers1::FAIL, ret);
  }

  void testProcessSurfaceRestrictions_RtwEmbeddedLocNotSpecified()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtw(*_trx);

    const FareMarket& fm = *_itin->fareMarket()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.embeddedSurfaceInd() = 'Y';
    trInfo.embeddedSurfaceTblItemNo() = 0;

    TransfersSubclass tr;
    tr.setRtw(true);
    Transfers1::ProcessingResult ret = tr.processSurfaceSectorRestrictions(*_trx, trInfo, fm, 0);

    CPPUNIT_ASSERT_EQUAL(Transfers1::FAIL, ret);
  }

  void testProcessSurfaceRestrictions_RtwEmbeddedIgnoreOrigDestInd()
  {
    PricingRequest req;
    req.ticketingDT() = DateTime(time(0));
    _trx->setRequest(&req);
    _itin = createItinRtw(*_trx);

    const FareMarket& fm = *_itin->fareMarket()[0];

    TransfersInfo1 trInfo;
    createBaseTransfersInfo1(trInfo);
    trInfo.embeddedSurfaceInd() = 'Y';
    trInfo.embeddedSurfaceTblItemNo() = 41;

    TransfersSubclass tr;
    tr.setRtw(true);
    Transfers1::ProcessingResult ret = tr.processSurfaceSectorRestrictions(*_trx, trInfo, fm, 0);

    CPPUNIT_ASSERT_EQUAL(Transfers1::CONTINUE, ret);
  }

  void testValidateSurfaceRestrictions()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinTestSurface(*_trx);

    FareMarket* fm1 = _itin->fareMarket()[0];
    FareMarket* fm2 = _itin->fareMarket()[1];
    FareMarket* fm3 = _itin->fareMarket()[2];

    FarePath* fp = _itin->farePath()[0];

    PaxTypeFare* ptf1 = fp->pricingUnit()[0]->fareUsage()[0]->paxTypeFare();
    PaxTypeFare* ptf2 = fp->pricingUnit()[1]->fareUsage()[0]->paxTypeFare();
    PaxTypeFare* ptf3 = fp->pricingUnit()[2]->fareUsage()[0]->paxTypeFare();

    Itin* itin2 = createItin8(*_trx);

    FareMarket* fm6 = itin2->fareMarket()[0];
    FarePath* fp2 = itin2->farePath()[0];
    PaxTypeFare* ptf6 = fp2->pricingUnit()[0]->fareUsage()[0]->paxTypeFare();

    // Surface sectors not permitted at fare breaks
    //
    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "XX";
    trInfo1.fareBreakSurfaceInd() = 'N';

    // Embedded Surface sectors not permitted
    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "XX";
    trInfo2.embeddedSurfaceInd() = 'N';

    // Fare Break Surface sectors permitted in Zone 220, or Zone 210
    //  at Fare destination, or between DEL and BOM.
    TransfersInfo1 trInfo3;

    createBaseTransfersInfo1(trInfo3);
    trInfo3.itemNo() = 3;
    trInfo3.noTransfersMax() = "XX";
    trInfo3.fareBreakSurfaceTblItemNo() = 30;

    // Fare Break Surface sectors NOT permitted in Zone 220, or Zone 210
    //  at Fare destination, or between DEL and BOM.
    TransfersInfo1 trInfo4;

    createBaseTransfersInfo1(trInfo4);
    trInfo4.itemNo() = 4;
    trInfo4.noTransfersMax() = "XX";
    trInfo4.fareBreakSurfaceInd() = 'N';
    trInfo4.fareBreakSurfaceTblItemNo() = 30;

    // Embedded Surface sectors permitted between US and Mexico.
    TransfersInfo1 trInfo7;

    createBaseTransfersInfo1(trInfo7);
    trInfo7.itemNo() = 7;
    trInfo7.noTransfersMax() = "XX";
    trInfo7.embeddedSurfaceTblItemNo() = 19;

    // Embedded Surface sectors NOT permitted between US and Mexico.
    TransfersInfo1 trInfo8;

    createBaseTransfersInfo1(trInfo8);
    trInfo8.itemNo() = 8;
    trInfo8.noTransfersMax() = "XX";
    trInfo8.embeddedSurfaceInd() = 'N';
    trInfo8.embeddedSurfaceTblItemNo() = 19;

    /* These two test cases will not work because the table 976
     is coded using an ATPCO reserved zone, but the rule vendor
     is coded as SABR. The SABR reserved zones do not cover the
     same geographic areas as the ATPCO reserved zones.
     I left these test cases here so that we can rework them later
     when(if) we have a valid table 976 with the correct zones coded.

     // Embedded surface sectors permitted between Zone 9 (Mexico) and
     //  Zone 7 (US Virgin Islands).
     TransfersInfo1 trInfo9;

     trInfo9.vendor()                    = "AF11"; //SABR
     trInfo9.itemNo()                    = 9;
     trInfo9.unavailTag()                = ' ';
     trInfo9.noTransfersMin()            = "";
     trInfo9.noTransfersMax()            = "XX";
     trInfo9.primeCxrPrimeCxr()          = ' ';
     trInfo9.primePrimeMaxTransfers()    = "";
     trInfo9.sameCxrSameCxr()            = ' ';
     trInfo9.sameSameMaxTransfers()      = "";
     trInfo9.primeCxrOtherCxr()          = ' ';
     trInfo9.primeOtherMaxTransfers()    = "";
     trInfo9.otherCxrOtherCxr()          = ' ';
     trInfo9.otherOtherMaxTransfers()    = "";
     trInfo9.noOfTransfersOut()          = "";
     trInfo9.noOfTransfersIn()           = "";
     trInfo9.noOfTransfersAppl()         = ' ';
     trInfo9.fareBreakSurfaceInd()       = 'Y';
     trInfo9.fareBreakSurfaceTblItemNo() = 0;
     trInfo9.embeddedSurfaceInd()        = 'Y';
     trInfo9.embeddedSurfaceTblItemNo()  = 4;
     trInfo9.transfersChargeAppl()       = ' ';
     trInfo9.maxNoTransfersCharge1()     = "";
     trInfo9.maxNoTransfersCharge2()     = "";
     trInfo9.charge1Cur1Amt()            = 0;
     trInfo9.charge2Cur1Amt()            = 0;
     trInfo9.cur1()                      = "";
     trInfo9.noDec1()                    = 0;
     trInfo9.charge1Cur2Amt()            = 0;
     trInfo9.charge2Cur2Amt()            = 0;
     trInfo9.cur2()                      = "";
     trInfo9.noDec2()                    = 0;
     trInfo9.inhibit()                   = ' ';

     // Embedded surface sectors NOT permitted between Zone 9 (Mexico) and
     //  Zone 7 (US Virgin Islands).
     TransfersInfo1 trInfo10;

     trInfo10.vendor()                    = "AF11"; //SABR
     trInfo10.itemNo()                    = 10;
     trInfo10.unavailTag()                = ' ';
     trInfo10.noTransfersMin()            = "";
     trInfo10.noTransfersMax()            = "XX";
     trInfo10.primeCxrPrimeCxr()          = ' ';
     trInfo10.primePrimeMaxTransfers()    = "";
     trInfo10.sameCxrSameCxr()            = ' ';
     trInfo10.sameSameMaxTransfers()      = "";
     trInfo10.primeCxrOtherCxr()          = ' ';
     trInfo10.primeOtherMaxTransfers()    = "";
     trInfo10.otherCxrOtherCxr()          = ' ';
     trInfo10.otherOtherMaxTransfers()    = "";
     trInfo10.noOfTransfersOut()          = "";
     trInfo10.noOfTransfersIn()           = "";
     trInfo10.noOfTransfersAppl()         = ' ';
     trInfo10.fareBreakSurfaceInd()       = 'Y';
     trInfo10.fareBreakSurfaceTblItemNo() = 0;
     trInfo10.embeddedSurfaceInd()        = 'N';
     trInfo10.embeddedSurfaceTblItemNo()  = 4;
     trInfo10.transfersChargeAppl()       = ' ';
     trInfo10.maxNoTransfersCharge1()     = "";
     trInfo10.maxNoTransfersCharge2()     = "";
     trInfo10.charge1Cur1Amt()            = 0;
     trInfo10.charge2Cur1Amt()            = 0;
     trInfo10.cur1()                      = "";
     trInfo10.noDec1()                    = 0;
     trInfo10.charge1Cur2Amt()            = 0;
     trInfo10.charge2Cur2Amt()            = 0;
     trInfo10.cur2()                      = "";
     trInfo10.noDec2()                    = 0;
     trInfo10.inhibit()                   = ' ';
     */

    TransfersInfoWrapper trInfoWrapper(0);
    TransfersSubclass tr;

    trInfoWrapper.setCurrentTrInfo(&trInfo1);

    Record3ReturnTypes ret1Fm1 = tr.validate(*_trx, *_itin, *ptf1, &trInfoWrapper, *fm1);
    Record3ReturnTypes ret1Fm2 = tr.validate(*_trx, *_itin, *ptf2, &trInfoWrapper, *fm2);

    trInfoWrapper.setCurrentTrInfo(&trInfo2);

    Record3ReturnTypes ret2Fm1 = tr.validate(*_trx, *_itin, *ptf1, &trInfoWrapper, *fm1);
    Record3ReturnTypes ret2Fm2 = tr.validate(*_trx, *_itin, *ptf2, &trInfoWrapper, *fm2);

    trInfoWrapper.setCurrentTrInfo(&trInfo3);

    Record3ReturnTypes ret3Fm6 = tr.validate(*_trx, *itin2, *ptf6, &trInfoWrapper, *fm6);

    trInfoWrapper.setCurrentTrInfo(&trInfo4);

    //  Record3ReturnTypes ret4Fm6 =
    //    tr.validate(*_trx, *itin2, *ptf6, &trInfoWrapper, *fm6);

    trInfoWrapper.setCurrentTrInfo(&trInfo7);

    Record3ReturnTypes ret7Fm1 = tr.validate(*_trx, *_itin, *ptf1, &trInfoWrapper, *fm1);
    Record3ReturnTypes ret7Fm3 = tr.validate(*_trx, *_itin, *ptf3, &trInfoWrapper, *fm3);

    trInfoWrapper.setCurrentTrInfo(&trInfo8);

    Record3ReturnTypes ret8Fm1 = tr.validate(*_trx, *_itin, *ptf1, &trInfoWrapper, *fm1);
    Record3ReturnTypes ret8Fm3 = tr.validate(*_trx, *_itin, *ptf3, &trInfoWrapper, *fm3);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions A", ret1Fm1 == tse::SOFTPASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions B", ret1Fm2 == tse::FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions C", ret2Fm1 == tse::FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions D", ret2Fm2 == tse::SOFTPASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions E", ret3Fm6 == tse::SOFTPASS);

    /*
     It's impossible to write some of these unit tests so that they
     will always work because the data in the database keeps changing.
     The only long-term solution is to mock the data...

     CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions F",
     ret4Fm6 == tse::FAIL);
     */

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions G", ret7Fm1 == tse::SOFTPASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions H", ret7Fm3 == tse::SOFTPASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions I", ret8Fm1 == tse::FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Surface Restrictions J", ret8Fm3 == tse::FAIL);
  }

  void testRegressionPL7907()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL7907(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.vendor() = "SITA";
    trInfo1.noTransfersMax() = "02";
    trInfo1.noOfTransfersOut() = "01";
    trInfo1.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->orderNo() = 2;
    trInfo1Seg1->tsi() = 18;

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segCnt() = 1;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.vendor() = "SITA";
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "02";
    trInfo2.noOfTransfersOut() = "01";
    trInfo2.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->orderNo() = 2;
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "JP";

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segCnt() = 1;

    TransfersInfo1 trInfo3;

    createBaseTransfersInfo1(trInfo3);
    trInfo3.vendor() = "SITA";
    trInfo3.itemNo() = 3;
    trInfo3.noTransfersMax() = "02";
    trInfo3.noOfTransfersOut() = "01";
    trInfo3.noOfTransfersIn() = "01";

    TransfersInfoSeg1* trInfo3Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo3Seg1);
    trInfo3Seg1->orderNo() = 2;
    trInfo3Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo3Seg1->loc1().loc() = "AU";

    trInfo3.segs().push_back(trInfo3Seg1);
    trInfo3.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.vendorCode() = "SITA";
    crInfo.tariffNumber() = 8;
    crInfo.carrierCode() = "CX";
    crInfo.ruleNumber() = "CX1W";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(fu1);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    trInfoWrapper1.setCurrentTrInfo(&trInfo3);
    trInfoWrapper1.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1));

    CPPUNIT_ASSERT(trInfoWrapper1.needToProcessResults() &&
                   trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    trInfoWrapper2.setCurrentTrInfo(&trInfo3);
    trInfoWrapper2.setNoTransfersMax(6);
    CPPUNIT_ASSERT_EQUAL(tse::PASS, tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2));

    CPPUNIT_ASSERT(trInfoWrapper2.needToProcessResults() &&
                   trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);
  }

  void testRegressionPL8118()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL8118(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "04";
    trInfo1.noOfTransfersOut() = "02";
    trInfo1.noOfTransfersIn() = "02";

    TransfersInfo1 trInfo2;

    // trInfo2.textTblItemNo()        = 1175464;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "02";
    trInfo2.noOfTransfersOut() = "01";
    trInfo2.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->tsi() = 28;
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->tsi() = 29;
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "YY";
    crInfo.ruleNumber() = "073D";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(0);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    bool finalPass1 = false;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    Record3ReturnTypes ret1Fu1 = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    Record3ReturnTypes ret2Fu1 = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    if ((ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS) && trInfoWrapper1.needToProcessResults())
    {
      finalPass1 = (trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS) ||
                   (trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::SKIP);
    }
    else
    {
      finalPass1 = (ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS);
    }

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    bool finalPass2 = false;

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    Record3ReturnTypes ret1Fu2 = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    Record3ReturnTypes ret2Fu2 = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    if ((ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS) && trInfoWrapper2.needToProcessResults())
    {
      finalPass2 = (trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);
    }
    else
    {
      finalPass2 = (ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test A",
                           (ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test B", finalPass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test C",
                           (ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test D", finalPass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test E",
                           fu1->transfers().size() == 3 && fu2->transfers().size() == 3);
  }

  void testRegressionPL11648()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL11648(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    // FareUsage*   fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "06";
    trInfo.noOfTransfersOut() = "03";
    trInfo.noOfTransfersIn() = "03";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->orderNo() = 2;
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "LON";
    trInfoSeg1->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc2().loc() = "FRA";
    trInfoSeg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc1().loc() = "SIN";
    trInfoSeg2->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc2().loc() = "BKK";
    trInfoSeg2->chargeAppl() = '1';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg3);
    trInfoSeg3->orderNo() = 3;
    trInfoSeg3->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg3->loc1().loc() = "HKG";
    trInfoSeg3->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg3->loc2().loc() = "TYO";
    trInfoSeg3->chargeAppl() = '1';

    TransfersInfoSeg1* trInfoSeg4 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg4);
    trInfoSeg4->orderNo() = 4;
    trInfoSeg4->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg4->loc1().loc() = "DPS";
    trInfoSeg4->chargeAppl() = '1';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg5 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg5);
    trInfoSeg5->orderNo() = 5;
    trInfoSeg5->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg5->loc1().loc() = "AU";
    trInfoSeg5->chargeAppl() = '1';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segs().push_back(trInfoSeg3);
    trInfo.segs().push_back(trInfoSeg4);
    trInfo.segs().push_back(trInfoSeg5);
    trInfo.segCnt() = 5;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 4;
    crInfo.carrierCode() = "QF";
    crInfo.ruleNumber() = "5148";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    bool finalPass = false;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);
    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalPass = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);
    }
    else
    {
      finalPass = (ret == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 11648 - Test A", ret == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 11648 - Test B", finalPass);
  }

  void testRegressionPL20281()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL20281(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "01";
    trInfo.charge1Cur1Amt() = 450;
    trInfo.cur1() = "SGD";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "HKG";
    trInfoSeg1->outInPortion() = 'E';
    trInfoSeg1->chargeAppl() = '1';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 8;
    crInfo.carrierCode() = "JL";
    crInfo.ruleNumber() = "4316";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    Record3ReturnTypes retFromWP = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    fu1->paxTypeFare()->fareMarket()->fareBasisCode() = "BRTSG2";
    Record3ReturnTypes retFromWPQ = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 20281 - Test A",
                           retFromWP == tse::FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 20281 - Test B",
                           retFromWPQ == tse::FAIL);
  }

  void initTrx(PricingTrx& trx)
  {
    time_t t = time(0);
    DateTime* tkDT = _memHandle.insert<DateTime>(new DateTime(t));
    PricingRequest* request;
    PricingOptions* options;
    trx.dataHandle().get(request);
    trx.dataHandle().get(options);
    request->ticketingDT() = *tkDT;
    trx.setRequest(request);
    trx.setOptions(options);

    trx.diagnostic().diagnosticType() = Diagnostic309;
    trx.diagnostic().activate();
  }

  void testRegressionPL20281WithRelationalIndAND()
  {
    initTrx(*_trx);
    _itin = createItinRegressionPL20281(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "01";
    trInfo1.charge1Cur1Amt() = 450;
    trInfo1.cur1() = "SGD";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "LON";
    trInfoSeg1->outInPortion() = 'E';
    trInfoSeg1->chargeAppl() = '1';

    trInfo1.segs().push_back(trInfoSeg1);
    trInfo1.segCnt() = 1;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "02";

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc1().loc() = "NYC";
    trInfoSeg2->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfoSeg2);
    trInfo2.segCnt() = 1;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 389;
    crInfo.carrierCode() = "BA";
    crInfo.ruleNumber() = "JP01";

    // Memory will be deleted by crInfo
    CategoryRuleItemInfoSet* criis = new CategoryRuleItemInfoSet();
    crInfo.addItemInfoSetNosync(criis);

    CategoryRuleItemInfo crii1;
    crii1.setItemcat(9);
    crii1.setOrderNo(1);
    crii1.setRelationalInd(CategoryRuleItemInfo::THEN);
    crii1.setItemNo(382926);

    CategoryRuleItemInfo crii2;
    crii2.setItemcat(9);
    crii2.setOrderNo(2);
    crii2.setRelationalInd(CategoryRuleItemInfo::AND);
    crii2.setItemNo(382927);

    criis->push_back(crii1);
    criis->push_back(crii2);

    rsp.process(*_trx, &crInfo, *pu, *fu1);

    int16_t numTransfersMax = atoi(trInfo1.noTransfersMax().c_str());
    numTransfersMax += atoi(trInfo2.noTransfersMax().c_str());
    rsp.leastRestrictiveTransfersPermitted() = numTransfersMax;

    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    fu1->paxTypeFare()->fareMarket()->fareBasisCode() = "Y";
    trInfoWrapper.setCurrentTrInfo(&trInfo1);
    trInfoWrapper.setNoTransfersMax(3);
    tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    trInfoWrapper.setCurrentTrInfo(&trInfo2);
    trInfoWrapper.setNoTransfersMax(3);
    tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);

    trInfoWrapper.needToProcessResults();

    // Set FC Scope to prevent trInfoWrapper.leastRestrictiveTransfersPermitted()
    // being cleared by TransfersInfoWrapper::collectMaxTransfersAllow()
    trInfoWrapper.transferFCscope() = true;
    trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 20281 - Test C",
                           pu->mostRestrictiveMaxTransfer() == numTransfersMax);
  }

  Itin* createItinNew1(PricingTrx& trx)
  {
    // Create the itin
    //
    Itin* itin = _memHandle.create<Itin>();

    itin->calculationCurrency() = "USD";

    // Create the travel segments

    AirSeg* jfk_lhr = createAirSeg(1,
                                   jfk,
                                   lhr,
                                   false,
                                   std::string("AA"),
                                   100,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 16, 22, 20, 0), //  6:20PM EST
                                   DateTime(2004, 10, 17, 6, 20, 0)); // 6:20 GMT

    AirSeg* lhr_cdg = createAirSeg(2,
                                   lhr,
                                   cdg,
                                   false,
                                   std::string("BA"),
                                   304,
                                   GeoTravelType::International,
                                   DateTime(2004, 10, 17, 7, 20, 0), //  7:20AM GMT
                                   DateTime(2004, 10, 17, 8, 40, 0)); //  9:40AM

    AirSeg* cdg_lhr = createAirSeg(3,
                                   cdg,
                                   lhr,
                                   false,
                                   std::string("AF"),
                                   303,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 6, 40, 0), //  7:40AM
                                   DateTime(2004, 11, 29, 8, 0, 0)); //  8:00AM GMT

    AirSeg* lhr_jfk = createAirSeg(4,
                                   lhr,
                                   jfk,
                                   false,
                                   std::string("BA"),
                                   101,
                                   GeoTravelType::International,
                                   DateTime(2004, 11, 29, 10, 0, 0), // 10:00AM GMT
                                   DateTime(2004, 11, 29, 16, 30, 0)); // 12:30PM EST

    // Attach the travel segments to the Itin and the Transaction
    //
    itin->travelSeg().push_back(jfk_lhr);
    itin->travelSeg().push_back(lhr_cdg);
    itin->travelSeg().push_back(cdg_lhr);
    itin->travelSeg().push_back(lhr_jfk);

    _trx->travelSeg().push_back(jfk_lhr);
    _trx->travelSeg().push_back(lhr_cdg);
    _trx->travelSeg().push_back(cdg_lhr);
    _trx->travelSeg().push_back(lhr_jfk);

    // Create the FareUsages, PaxTypeFares, Fares and FareInfos
    //
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();

    // Attach the travel segments to the fare usages
    //
    fu1->travelSeg().push_back(jfk_lhr);
    fu1->travelSeg().push_back(lhr_cdg);
    fu3->travelSeg().push_back(cdg_lhr);
    fu3->travelSeg().push_back(lhr_jfk);

    // Set the directionality
    fu1->inbound() = false;
    fu3->inbound() = true;

    // Create the pricing units
    //
    PricingUnit* pu1 = _memHandle.create<PricingUnit>();
    pu1->puType() = PricingUnit::Type::ROUNDTRIP;

    // Attach the fare usages to the pricing units
    //
    pu1->fareUsage().push_back(fu1);
    pu1->fareUsage().push_back(fu3);

    // Attach the travel segs to the pricing units
    pu1->travelSeg().push_back(jfk_lhr);
    pu1->travelSeg().push_back(lhr_cdg);
    pu1->travelSeg().push_back(cdg_lhr);
    pu1->travelSeg().push_back(lhr_jfk);

    pu1->turnAroundPoint() = cdg_lhr;

    // Create a fare path
    FarePath* fp = _memHandle.create<FarePath>();

    // Attach the pricing units to the fare path
    fp->pricingUnit().push_back(pu1);

    // Attach the fare path to the itin
    itin->farePath().push_back(fp);

    // Attach the itin to the fare path
    fp->itin() = itin;

    // Create the FareMarkets
    FareMarket* fm1 = createFareMarket(jfk, cdg, "AA");
    FareMarket* fm3 = createFareMarket(cdg, jfk, "AA");

    Fare* f1 =
        createFare(fm1, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "USD");
    Fare* f3 =
        createFare(fm3, Fare::FS_International, GlobalDirection::AT, ONE_WAY_MAY_BE_DOUBLED, "EUR");

    // Create and initialize the PaxTypeFares
    PaxTypeFare* ptf1 = createPaxTypeFare(f1, *fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    PaxTypeFare* ptf3 = createPaxTypeFare(f3, *fm3, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');

    // Attach the PaxTypeFares to the FareUsages
    fu1->paxTypeFare() = ptf1;
    fu3->paxTypeFare() = ptf3;

    // Attach the travel segs to the fare markets
    fm1->travelSeg().push_back(jfk_lhr);
    fm1->travelSeg().push_back(lhr_cdg);
    fm3->travelSeg().push_back(cdg_lhr);
    fm3->travelSeg().push_back(lhr_jfk);

    fm1->setGlobalDirection(GlobalDirection::AT);
    fm3->setGlobalDirection(GlobalDirection::AT);

    // Attach the fare markets to the itin
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm3);

    fp->paxType() = ptf1->actualPaxType();

    // Attach the itin to the transaction
    _trx->itin().push_back(itin);

    return itin;
  }

  void testValidatePU99()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinNew1(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];
    FareUsage* fu3 = pu->fareUsage()[2];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "02";
    trInfo1.maxNoTransfersCharge1() = "01";
    trInfo1.maxNoTransfersCharge2() = "01";
    trInfo1.charge1Cur1Amt() = 20;
    trInfo1.charge2Cur1Amt() = 50;
    trInfo1.cur1() = "USD";
    trInfo1.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo1Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg1);
    trInfo1Seg1->noTransfersPermitted() = "01";
    trInfo1Seg1->primeOnline() = 'X';
    trInfo1Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg1->loc1().loc() = "US";
    trInfo1Seg1->gateway() = 'X';
    trInfo1Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo1Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo1Seg2);
    trInfo1Seg2->orderNo() = 2;
    trInfo1Seg2->noTransfersPermitted() = "01";
    trInfo1Seg2->tsi() = 28;
    trInfo1Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo1Seg2->chargeAppl() = '2';

    trInfo1.segs().push_back(trInfo1Seg1);
    trInfo1.segs().push_back(trInfo1Seg2);
    trInfo1.segCnt() = 2;

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.maxNoTransfersCharge1() = "02";
    trInfo2.maxNoTransfersCharge2() = "XX";
    trInfo2.charge1Cur1Amt() = 75;
    trInfo2.charge2Cur1Amt() = 100;
    trInfo2.cur1() = "USD";
    trInfo2.noDec1() = 2;

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->transferAppl() = 'N';
    trInfo2Seg1->primeInterline() = 'X';
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->loc1().loc() = "GB";
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->noTransfersPermitted() = "02";
    trInfo2Seg2->primeInterline() = 'X';
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->loc1().loc() = "GB";
    trInfo2Seg2->chargeAppl() = '2';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg3);
    trInfo2Seg3->orderNo() = 3;
    trInfo2Seg3->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segs().push_back(trInfo2Seg3);
    trInfo2.segCnt() = 3;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu3);

    TransfersInfoWrapper trInfoWrapper1(0);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    Record3ReturnTypes retFu1A = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    Record3ReturnTypes retFu1B = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    Record3ReturnTypes retFu1Final = tse::SKIP;
    if ((retFu1A == tse::PASS) && (retFu1B == tse::PASS))
    {
      if (trInfoWrapper1.needToProcessResults())
      {
        retFu1Final = trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1);
      }
    }

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    Record3ReturnTypes retFu2A = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(RuleConst::MAX_NUMBER_XX);
    Record3ReturnTypes retFu2B = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    Record3ReturnTypes retFu2Final = tse::SKIP;
    if ((retFu2A == tse::PASS) && (retFu2B == tse::PASS))
    {
      if (trInfoWrapper2.needToProcessResults())
      {
        retFu2Final = trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2);
      }
    }

    bool finalPass1 = (retFu1Final == tse::PASS);
    bool finalPass2 = (retFu2Final == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99A",
                           retFu1A == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99B",
                           retFu1B == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99C",
                           retFu2A == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99D",
                           retFu2B == tse::PASS);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99E",
                           finalPass1 && finalPass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99F",
                           fu1->transfers().size() == 1 && fu2->transfers().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99G",
                           fu1->transferSurcharges().size() == 1 &&
                               fu2->transferSurcharges().size() == 1);

    FareUsage::TransferSurchargeMultiMapCI iter =
        fu1->transferSurcharges().find(fu1->travelSeg()[0]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99J",
                           iter != fu1->transferSurcharges().end() &&
                               (*iter).second->unconvertedAmount() == 100 &&
                               (*iter).second->unconvertedCurrencyCode() == "USD" &&
                               (*iter).second->isSegmentSpecific());

    iter = fu2->transferSurcharges().find(fu2->travelSeg()[0]);
    CPPUNIT_ASSERT_MESSAGE("Error in validate: Pricing Unit Scope - Test 99M",
                           iter != fu2->transferSurcharges().end());
  }

  void testRegressionPL8118GW()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL8118(*_trx);

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    FareUsage* fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo1;

    createBaseTransfersInfo1(trInfo1);
    trInfo1.noTransfersMax() = "04";
    trInfo1.noOfTransfersOut() = "02";
    trInfo1.noOfTransfersIn() = "02";

    TransfersInfo1 trInfo2;

    createBaseTransfersInfo1(trInfo2);
    trInfo2.itemNo() = 2;
    trInfo2.noTransfersMax() = "02";
    trInfo2.noOfTransfersOut() = "01";
    trInfo2.noOfTransfersIn() = "01";

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfo2Seg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg1);
    trInfo2Seg1->tsi() = 28;
    trInfo2Seg1->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg1->gateway() = 'X';
    trInfo2Seg1->chargeAppl() = '1';

    TransfersInfoSeg1* trInfo2Seg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfo2Seg2);
    trInfo2Seg2->orderNo() = 2;
    trInfo2Seg2->tsi() = 29;
    trInfo2Seg2->loc1().locType() = LOCTYPE_NATION;
    trInfo2Seg2->chargeAppl() = '1';

    trInfo2.segs().push_back(trInfo2Seg1);
    trInfo2.segs().push_back(trInfo2Seg2);
    trInfo2.segCnt() = 2;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.carrierCode() = "YY";
    crInfo.ruleNumber() = "073D";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu2);

    TransfersInfoWrapper trInfoWrapper1(0);
    trInfoWrapper1.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    bool finalPass1 = false;

    trInfoWrapper1.setCurrentTrInfo(&trInfo1);
    trInfoWrapper1.setNoTransfersMax(6);
    Record3ReturnTypes ret1Fu1 = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    trInfoWrapper1.setCurrentTrInfo(&trInfo2);
    trInfoWrapper1.setNoTransfersMax(6);
    Record3ReturnTypes ret2Fu1 = tr.validate(*_trx, &trInfoWrapper1, *fp, *pu, *fu1);

    if ((ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS) && trInfoWrapper1.needToProcessResults())
    {
      finalPass1 = (trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS) ||
                   (trInfoWrapper1.processResults(*_trx, *fp, *pu, *fu1) == tse::SKIP);
    }
    else
    {
      finalPass1 = (ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS);
    }

    TransfersInfoWrapper trInfoWrapper2(fu2);
    trInfoWrapper2.doRuleSetPreprocessing(*_trx, rsp, *pu);

    bool finalPass2 = false;

    trInfoWrapper2.setCurrentTrInfo(&trInfo1);
    trInfoWrapper2.setNoTransfersMax(6);
    Record3ReturnTypes ret1Fu2 = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    trInfoWrapper2.setCurrentTrInfo(&trInfo2);
    trInfoWrapper2.setNoTransfersMax(6);
    Record3ReturnTypes ret2Fu2 = tr.validate(*_trx, &trInfoWrapper2, *fp, *pu, *fu2);

    if ((ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS) && trInfoWrapper2.needToProcessResults())
    {
      finalPass2 = (trInfoWrapper2.processResults(*_trx, *fp, *pu, *fu2) == tse::PASS);
    }
    else
    {
      finalPass2 = (ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test A",
                           (ret1Fu1 == tse::PASS) && (ret2Fu1 == tse::PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test B", finalPass1);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test C",
                           (ret1Fu2 == tse::PASS) && (ret2Fu2 == tse::PASS));

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test D", finalPass2);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression PL 8118 - Test E",
                           fu1->transfers().size() == 3 && fu2->transfers().size() == 3);
  }

  void testIsFCvsPUreturnTrue()
  {
    TransfersSubclass tr;
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "02";
    trInfo.segs().clear();
    CPPUNIT_ASSERT(tr.isFCvsPU(trInfo));
  }

  void testIsFCvsPUreturnFalseMAXisXX()
  {
    TransfersSubclass tr;
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    CPPUNIT_ASSERT(!tr.isFCvsPU(trInfo));
  }

  void testIsFCvsPUreturnFalseMAXisBlank()
  {
    TransfersSubclass tr;
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "";
    CPPUNIT_ASSERT(!tr.isFCvsPU(trInfo));
  }

  void testIsFCvsPUreturnFalseSegsIsNotEmpty()
  {
    TransfersSubclass tr;
    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "04";
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segCnt() = 1;
    CPPUNIT_ASSERT(!tr.isFCvsPU(trInfo));
  }
  SurfaceTransfersInfo* createSTI(LocCode loc1, LocCode loc2)
  {
    SurfaceTransfersInfo* ret = _memHandle.create<SurfaceTransfersInfo>();
    ret->fareBrkEmbeddedLoc().loc() = loc1;
    ret->surfaceLoc().loc() = loc2;
    if (!loc1.empty())
      ret->fareBrkEmbeddedLoc().locType() = LOCTYPE_CITY;
    else
      ret->fareBrkEmbeddedLoc().locType() = LOCTYPE_NONE;
    if (!loc2.empty())
      ret->surfaceLoc().locType() = LOCTYPE_CITY;
    else
      ret->surfaceLoc().locType() = LOCTYPE_NONE;
    return ret;
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchOrigDest_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("SFO", "DFW"), true));
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchOrigDest_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("DFW", "SFO"), true));
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchOrig_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("SFO", ""), true));
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchOrig_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("DFW", ""), true));
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchDest_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("", "DFW"), true));
  }
  void testMatchSurfaceGeoFareBreak_FirstSegMatchDest_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("", "SFO"), true));
  }

  void testMatchSurfaceGeoFareBreak_LastSegMatchOrigDest_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("DFW", "SFO"), false));
  }
  void testMatchSurfaceGeoFareBreak_LastSegMatchOrigDest_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("SFO", "DFW"), false));
  }
  void testMatchSurfaceGeoFareBreak_LastSegMatchOrig_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("DFW", ""), false));
  }
  void testMatchSurfaceGeoFareBreak_LastSegMatchOrig_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("SFO", ""), false));
  }
  void testMatchSurfaceGeoFareBreak_LastSegMatchDest_Pass()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("", "SFO"), false));
  }
  void testMatchSurfaceGeoFareBreak_LastSegMatchDest_Fail()
  {
    Transfers1 tr;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    AirSeg* aSeg = createAirSeg(0,
                                sfo,
                                dfw,
                                false,
                                "AA",
                                0,
                                GeoTravelType::Domestic,
                                DateTime(2010, 1, 1, 1, 1, 0),
                                DateTime(2010, 1, 1, 1, 1, 0));
    CPPUNIT_ASSERT(!tr.matchSurfaceGeoFareBreak(*_trx, *aSeg, *createSTI("", "DFW"), false));
  }

  void testApplySystemDefaultAssumptionSkipWhenNotUSCA()
  {
    FareMarket fm;
    CPPUNIT_ASSERT_EQUAL(Transfers1::applySystemDefaultAssumption(*_itin, fm), tse::SKIP);
  }

  void testApplySystemDefaultAssumptionSkipWhenAllAir()
  {
    FareMarket fm;
    fm.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA);
    Itin itin;
    AirSeg airSeg;
    fm.travelSeg().push_back(&airSeg);
    CPPUNIT_ASSERT_EQUAL(Transfers1::applySystemDefaultAssumption(itin, fm), tse::SKIP);
  }

  void testApplySystemDefaultAssumptionFailWhenNoAir()
  {
    FareMarket fm;
    fm.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA);
    Itin itin;
    ArunkSeg arunkSeg;
    fm.travelSeg().push_back(&arunkSeg);
    CPPUNIT_ASSERT_EQUAL(Transfers1::applySystemDefaultAssumption(itin, fm), tse::FAIL);
  }

  void setupCheckInfoSegRestriction(Transfers1::TravelSegMarkup& tsm,
                                    const Loc* loc1,
                                    const Loc* loc2,
                                    const Loc* loc3)
  {
    AirSeg* ts1 = createAirSeg(0,
                               loc1,
                               loc2,
                               false,
                               "AA",
                               0,
                               GeoTravelType::UnknownGeoTravelType,
                               DateTime(2020, 1, 1, 10, 0, 0),
                               DateTime(2020, 1, 1, 12, 0, 0));
    AirSeg* ts2 = createAirSeg(1,
                               loc2,
                               loc3,
                               false,
                               "AA",
                               1,
                               GeoTravelType::UnknownGeoTravelType,
                               DateTime(2020, 1, 1, 14, 0, 0),
                               DateTime(2020, 1, 1, 16, 0, 0));

    tsm.travelSeg() = ts1;
    tsm.nextTravelSeg() = ts2;
  }

  void testCheckInfoSegRestriction_Dom()
  {
    Transfers1::TravelSegMarkup tsm;
    setupCheckInfoSegRestriction(tsm, dfw, bos, sfo);

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.restriction() = Transfers1::SEG_RESTRICTION_DOMESTIC;

    Transfers1 tr;
    CPPUNIT_ASSERT_EQUAL(Transfers1::MATCH, tr.checkInfoSegRestriction(tsm, trInfoSeg));
  }

  void testCheckInfoSegRestriction_Dom_USCA()
  {
    Transfers1::TravelSegMarkup tsm;
    setupCheckInfoSegRestriction(tsm, dfw, bos, yvr);

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.restriction() = Transfers1::SEG_RESTRICTION_DOMESTIC;

    Transfers1 tr;
    CPPUNIT_ASSERT_EQUAL(Transfers1::NOT_MATCH, tr.checkInfoSegRestriction(tsm, trInfoSeg));
  }

  void testCheckInfoSegRestriction_Intl()
  {
    Transfers1::TravelSegMarkup tsm;
    setupCheckInfoSegRestriction(tsm, mel, bos, hta);

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.restriction() = Transfers1::SEG_RESTRICTION_INTERNATIONAL;

    Transfers1 tr;
    CPPUNIT_ASSERT_EQUAL(Transfers1::MATCH, tr.checkInfoSegRestriction(tsm, trInfoSeg));
  }

  void testCheckInfoSegRestriction_Intl_XURU()
  {
    Transfers1::TravelSegMarkup tsm;
    setupCheckInfoSegRestriction(tsm, mel, svo, hta);

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.restriction() = Transfers1::SEG_RESTRICTION_INTERNATIONAL;

    Transfers1 tr;
    CPPUNIT_ASSERT_EQUAL(Transfers1::NOT_MATCH, tr.checkInfoSegRestriction(tsm, trInfoSeg));
  }

  void testCheckInfoSegRestriction_DomIntl()
  {
    Transfers1::TravelSegMarkup tsm;
    setupCheckInfoSegRestriction(tsm, mel, svo, hta);

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.restriction() = Transfers1::SEG_RESTRICTION_DOM_INTL;

    Transfers1 tr;
    CPPUNIT_ASSERT_EQUAL(Transfers1::MATCH, tr.checkInfoSegRestriction(tsm, trInfoSeg));
  }

  void testCheckInfoSegDirectionality_SegGeoApplNotPermitted_inOutOut()
  {
    Transfers1 tr;
    Transfers1::TravelSegMarkup tsm;
    Transfers1::TransfersInfoSegMarkup trISM;
    bool processNextInfoSeg;
    bool failed;
    std::string shortFailReason;
    std::string failReason;

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.noTransfersPermitted() = "01";
    trInfoSeg.outInPortion() = Transfers1::SEG_INOUT_OUT;
    trInfoSeg.transferAppl() = Transfers1::SEG_GEO_APPL_NOT_PERMITTED;

    trISM.initialize(&trInfoSeg);
    trISM.matchCountOut() = 1;

    CPPUNIT_ASSERT(tr.checkInfoSegDirectionality(tsm, trISM, processNextInfoSeg, failed, shortFailReason, failReason));
    CPPUNIT_ASSERT(processNextInfoSeg);
  }

  void testCheckInfoSegDirectionality_SegGeoApplNotPermitted_inOutIn()
  {
    Transfers1 tr;
    Transfers1::TravelSegMarkup tsm;
    Transfers1::TransfersInfoSegMarkup trISM;
    bool processNextInfoSeg;
    bool failed;
    std::string shortFailReason;
    std::string failReason;

    TransfersInfoSeg1 trInfoSeg;
    trInfoSeg.noTransfersPermitted() = "01";
    trInfoSeg.outInPortion() = Transfers1::SEG_INOUT_IN;
    trInfoSeg.transferAppl() = Transfers1::SEG_GEO_APPL_NOT_PERMITTED;

    trISM.initialize(&trInfoSeg);
    trISM.matchCountIn() = 1;

    CPPUNIT_ASSERT(tr.checkInfoSegDirectionality(tsm, trISM, processNextInfoSeg, failed, shortFailReason, failReason));
    CPPUNIT_ASSERT(processNextInfoSeg);
  }
  void testRegressionAPO34850_SurfaceSectorNotMatch()
  {
    time_t t = time(0);
    DateTime tkDT(t);
    PricingRequest req;
    req.ticketingDT() = tkDT;
    _trx->setRequest(&req);

    _trx->diagnostic().diagnosticType() = Diagnostic309;
    _trx->diagnostic().activate();

    _itin = createItinRegressionPL11648(*_trx);  //reuse itim from PL11648

    FarePath* fp = _itin->farePath()[0];
    PricingUnit* pu = fp->pricingUnit()[0];
    FareUsage* fu1 = pu->fareUsage()[0];
    // FareUsage*   fu2 = pu->fareUsage()[1];

    TransfersInfo1 trInfo;

    createBaseTransfersInfo1(trInfo);
    trInfo.noTransfersMax() = "";
    trInfo.noOfTransfersOut() = "02";
    trInfo.noOfTransfersIn() = "02";
    trInfo.fareBreakSurfaceInd() = 'N';
    trInfo.fareBreakSurfaceTblItemNo() = 0;
    trInfo.embeddedSurfaceInd() = 'Y';
    trInfo.embeddedSurfaceTblItemNo() = 0;


    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg1 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg1);
    trInfoSeg1->orderNo() = 2;
    trInfoSeg1->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc1().loc() = "LON";
    trInfoSeg1->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg1->loc2().loc() = "FRA";
    trInfoSeg1->chargeAppl() = '1';
    trInfoSeg1->outInPortion() = 'B';

    TransfersInfoSeg1* trInfoSeg2 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg2);
    trInfoSeg2->orderNo() = 2;
    trInfoSeg2->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc1().loc() = "SIN";
    trInfoSeg2->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg2->loc2().loc() = "BKK";
    trInfoSeg2->chargeAppl() = '1';
    trInfoSeg2->outInPortion() = 'B';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg3 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg3);
    trInfoSeg3->orderNo() = 3;
    trInfoSeg3->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg3->loc1().loc() = "PYY";
    trInfoSeg3->loc2().locType() = LOCTYPE_CITY;
    trInfoSeg3->loc2().loc() = "TYO";
    trInfoSeg3->chargeAppl() = '1';
    trInfoSeg3->outInPortion() = 'B';

    TransfersInfoSeg1* trInfoSeg4 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg4);
    trInfoSeg4->orderNo() = 4;
    trInfoSeg4->loc1().locType() = LOCTYPE_CITY;
    trInfoSeg4->loc1().loc() = "DPS";
    trInfoSeg4->chargeAppl() = '1';
    trInfoSeg4->outInPortion() = 'B';

    // Memory will be freed by trInfo
    TransfersInfoSeg1* trInfoSeg5 = new TransfersInfoSeg1();

    createBaseTransfersInfoSeg1(trInfoSeg5);
    trInfoSeg5->orderNo() = 5;
    trInfoSeg5->loc1().locType() = LOCTYPE_NATION;
    trInfoSeg5->loc1().loc() = "AU";
    trInfoSeg5->chargeAppl() = '1';
    trInfoSeg4->outInPortion() = 'B';

    trInfo.segs().push_back(trInfoSeg1);
    trInfo.segs().push_back(trInfoSeg2);
    trInfo.segs().push_back(trInfoSeg3);
    trInfo.segs().push_back(trInfoSeg4);
    trInfo.segs().push_back(trInfoSeg5);

    trInfo.segCnt() = 5;

    RuleSetPreprocessor rsp;

    CategoryRuleInfo crInfo;

    createBaseCategoryRuleInfo(crInfo);
    crInfo.tariffNumber() = 4;
    crInfo.carrierCode() = "QF";
    crInfo.ruleNumber() = "5148";

    rsp.process(*_trx, &crInfo, *pu, *fu1);
    rsp.fareUsagePricingUnitScopeForTransfers().insert(fu1);

    TransfersInfoWrapper trInfoWrapper(0);
    trInfoWrapper.setCurrentTrInfo(&trInfo);
    trInfoWrapper.doRuleSetPreprocessing(*_trx, rsp, *pu);

    TransfersSubclass tr;

    bool finalRESP = false;

    Record3ReturnTypes ret = tr.validate(*_trx, &trInfoWrapper, *fp, *pu, *fu1);
    if ((ret == tse::PASS) && trInfoWrapper.needToProcessResults())
    {
      finalRESP = (trInfoWrapper.processResults(*_trx, *fp, *pu, *fu1) == tse::PASS);
    }
    else
    {
      finalRESP = (ret == tse::FAIL);
    }

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression APO34850 - Test A", ret == tse::FAIL);

    CPPUNIT_ASSERT_MESSAGE("Error in validate: Regression APO34850 - Test B", finalRESP);
  }

private:
  PricingTrx* _trx;
  Itin* _itin;
  TestMemHandle _memHandle;
  MyDataHandle* _myDataHandle;

  // Locations
  //
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  const tse::Loc* dal; // Love Field
  const tse::Loc* sjc; // San Jose
  const tse::Loc* jfk;
  const tse::Loc* bos;
  const tse::Loc* lga;
  const tse::Loc* lax;
  const tse::Loc* iah; // Houston
  const tse::Loc* mel;
  const tse::Loc* syd;
  const tse::Loc* hkg; // Hong Kong
  const tse::Loc* nrt; // Tokyo, Narita
  const tse::Loc* mia;
  const tse::Loc* yyz; // Toronto, Canada
  const tse::Loc* yvr; // Vancouver, Canada
  const tse::Loc* lhr; // London
  const tse::Loc* gig; // Brazil
  const tse::Loc* hnl; // Honalulu (Hawaii)
  const tse::Loc* stt; // St Thomas (Virgin Islands)
  const tse::Loc* anc; // Anchorage, Alaska
  const tse::Loc* sju; // San Juan (Puerto Rico)
  const tse::Loc* cdg; // Paris
  const tse::Loc* mex;
  const tse::Loc* lux;
  const tse::Loc* bkk;
  const tse::Loc* fuk;
  const tse::Loc* cbr;
  const tse::Loc* tia;
  const tse::Loc* vie;
  const tse::Loc* zrh;
  const tse::Loc* yul;
  const tse::Loc* yqb;
  const tse::Loc* cun; // Cancun, Mexico
  const tse::Loc* sin;
  const Loc* svo;
  const Loc* hta;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Transfers1Test);

} // tse
