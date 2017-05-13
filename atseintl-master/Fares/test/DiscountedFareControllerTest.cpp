#include "Common/Global.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Fares/DiscountedFareController.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestContainerFactory.h"
#include "test/testdata/TestDiscountInfoFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestGeneralFareRuleInfoFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestRuleCPPUnitHelper.h"

#include <boost/unordered_map.hpp>

namespace tse
{
FALLBACKVALUE_DECL(fallbackAPO37838Record1EffDateCheck);

class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(FareMarket& fareMarket,
                  Fare& fare,
                  FareClassAppInfo& fareClassAppInfo,
                  FareClassAppSegInfo& fareClassAppSegInfo,
                  PaxType& paxType)
  {
    _actualPaxType = &paxType;
    _fareMarket = &fareMarket;
    _fare = &fare;
    _fareClassAppInfo = &fareClassAppInfo;
    _fareClassAppSegInfo = &fareClassAppSegInfo;
  }

  ~MockPaxTypeFare() {}

  PaxTypeFareRuleData* ruleData()
  {
    PaxTypeFareRuleData* ret = paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
    if (ret == 0)
      ret = _memHandle.create<PaxTypeFareRuleData>();
    return ret;
  }

  void setDiscountInfo(const DiscountInfo* p) { ruleData()->ruleItemInfo() = p; }

  void setGeneralFareRuleInfo(const GeneralFareRuleInfo* p) { ruleData()->categoryRuleInfo() = p; }

  void setCategoryRuleItemInfoSet(std::vector<CategoryRuleItemInfo>& p)
  {
    ruleData()->categoryRuleItemInfoVec() = &p;
  }

  void setCategoryRuleItemInfo(CategoryRuleItemInfo* p) { ruleData()->categoryRuleItemInfo() = p; }

private:
  TestMemHandle _memHandle;
};

class MockDiscountedFareController : public DiscountedFareController
{
public:
  MockDiscountedFareController(PricingTrx& trx,
                               Itin& itin,
                               FareMarket& fareMarket)
    : DiscountedFareController(trx, itin, fareMarket), _Mock_validate(true) {};

  virtual ~MockDiscountedFareController() {};

  virtual bool validate(DiagCollector& diag,
                        PaxTypeFare& ptFare,
                        const CategoryRuleInfo& ruleInfo,
                        const DiscountInfo& discountInfo,
                        const CategoryRuleItemInfoSet& catRuleItemInfoSet)
  {
    if (_Mock_validate)
      return true;
    return DiscountedFareController::validate(diag, ptFare, ruleInfo, discountInfo, catRuleItemInfoSet);
  }

  void setRealValidate() {_Mock_validate = false;};

  virtual const DiscountInfo*
  getDiscountInfo(const CategoryRuleInfo* ruleInfo, const CategoryRuleItemInfo* ruleItem)
  {
    DiscountInfo* discInfo = 0;

    auto dMapIt = _discInfoMap.find(*ruleItem);
    if (dMapIt == _discInfoMap.end())
    {
      _trx.dataHandle().get(discInfo);
      _discInfoMap.insert(std::make_pair(*ruleItem, discInfo));
    }
    else
    {
      discInfo = dMapIt->second;
    }
    return discInfo;
  }

private:
  bool _Mock_validate;
  boost::unordered_map<CategoryRuleItemInfo, DiscountInfo*> _discInfoMap;
};

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const bool isHistorical() { return false; }
  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate = DateTime::emptyDate())
  {
    if (vendor == "ATP" && carrier == "BA" && ruleTariff == 1 && rule == "2002" && category == 20)
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    else if (vendor == "ATP" && carrier == "BA" && ruleTariff == 1 && rule == "2002" &&
             category == 21)
      return *TestVectorFactory<TestGeneralFareRuleInfoFactory, GeneralFareRuleInfo>::create(
                 "/vobs/atseintl/Fares/test/data/DiscountedFareController/"
                 "GenFareRule_ATP_BA_1_2002_21.xml");
    else if (vendor == "ATP" && carrier == "BA" && ruleTariff == 1 && rule == "2002" &&
             category == 22)
      return *TestVectorFactory<TestGeneralFareRuleInfoFactory, GeneralFareRuleInfo>::create(
                 "/vobs/atseintl/Fares/test/data/DiscountedFareController/"
                 "GenFareRule_ATP_BA_1_2002_22.xml");
    else if (vendor == "ATP" && carrier == "BA" && ruleTariff == 60 && rule == "0210" &&
             category == 20)
      return *TestVectorFactory<TestGeneralFareRuleInfoFactory, GeneralFareRuleInfo>::create(
                 "/vobs/atseintl/Fares/test/data/DiscountedFareController/"
                 "GenFareRule_ATP_BA_60_0210_20.xml");
    else if (vendor == "ATP" && carrier == "BA" && ruleTariff == 60 && rule == "0210" &&
             category == 21)
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    else if (vendor == "ATP" && carrier == "BA" && ruleTariff == 0 && rule == "0000" &&
             category == 21)
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();

    return DataHandleMock::getGeneralFareRule(
        vendor, carrier, ruleTariff, rule, category, date, applDate);
  }

  const std::vector<GeneralFareRuleInfo*>& getGeneralFareRule(const VendorCode& vendor,
                                                              const CarrierCode& carrier,
                                                              const TariffNumber& ruleTariff,
                                                              const RuleNumber& rule,
                                                              const CatNumber& category,
                                                              const TvlDatesSet& tvlDates)
  {
    return getGeneralFareRule(vendor, carrier, ruleTariff, rule, category, *tvlDates.begin());
  }

  bool getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& tariffNumber,
                                   const RuleNumber& ruleNumber,
                                   CatNumber catNum,
                                   RuleNumber& ruleNumOut,
                                   TariffNumber& tariffNumOut)
  {
    if (vendor == "ATP" && carrier == "BA" && tariffNumber == 1 && ruleNumber == "0000" &&
        catNum == 20)
    {
      ruleNumOut = "0210";
      tariffNumOut = 60;
      return true;
    }
    return DataHandleMock::getGeneralRuleAppTariffRule(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut);
  }
  GeneralRuleApp* getGeneralRuleApp(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& tariffNumber,
                                    const RuleNumber& ruleNumber,
                                    CatNumber catNum)
  {
    if (vendor == "ATP" && carrier == "BA" && tariffNumber == 1 && ruleNumber == "0000" &&
        catNum == 22)
      return 0;

    return DataHandleMock::getGeneralRuleApp(vendor, carrier, tariffNumber, ruleNumber, catNum);
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
    if (vendor == "ATP" && carrier == "BA" && tariffNumber == 1 && ruleNumber == "0000" &&
        catNum == 20)
    {
      ruleNumOut = "0210";
      tariffNumOut = 60;
      return true;
    }
    return DataHandleMock::getGeneralRuleAppTariffRuleByTvlDate(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut, tvlDate);
  }

  GeneralRuleApp* getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const TariffNumber& tariffNumber,
                                             const RuleNumber& ruleNumber,
                                             CatNumber catNum,
                                             const DateTime& tvlDate)
  {
    if (vendor == "ATP" && carrier == "BA" && tariffNumber == 1 && ruleNumber == "0000" &&
        catNum == 22)
      return 0;

    return DataHandleMock::getGeneralRuleAppByTvlDate(vendor, carrier, tariffNumber, ruleNumber, catNum, tvlDate);
  }
  //msd

  const DiscountInfo* getDiscount(const VendorCode& vendor, int itemNo, int category)
  {
    if (vendor == "ATP" && itemNo == 208 && category == 20)
      return TestDiscountInfoFactory::create(
          "/vobs/atseintl/Fares/test/data/DiscountedFareController/DiscountInfo_ATP_208_20.xml");
    else if (vendor == "ATP" && itemNo == 8141 && category == 21)
      return TestDiscountInfoFactory::create(
          "/vobs/atseintl/Fares/test/data/DiscountedFareController/DiscountInfo_ATP_8141_21.xml");
    else if (vendor == "ATP" && itemNo == 70475 && category == 22)
      return TestDiscountInfoFactory::create(
          "/vobs/atseintl/Fares/test/data/DiscountedFareController/DiscountInfo_ATP_70475_22.xml");
    else if (vendor == "SITA" && itemNo == 9 && category == 19)
      return TestDiscountInfoFactory::create(
          "/vobs/atseintl/Fares/test/data/DiscountedFareController/DiscountInfo_SITA_9_19.xml");
    else if (vendor == "CUR")
    {
      DiscountInfo *di = _memHandle.create<DiscountInfo>();
      di->cur1() = "CUR";
      return di;
    }
    return DataHandleMock::getDiscount(vendor, itemNo, category);
  }
};
}
class DiscountedFareControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiscountedFareControllerTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessRule);
  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_Accomp);
  CPPUNIT_TEST(testValidate_Geo);
  CPPUNIT_TEST(testValidate_Cur1);
  CPPUNIT_TEST(testCalcAmount);
  CPPUNIT_TEST(testCalcPercentage);
  CPPUNIT_TEST(testWriteDiagnostics);
  CPPUNIT_TEST(testWriteDiag219);
  CPPUNIT_TEST(testWriteDiag319);
  CPPUNIT_TEST(testWriteBaseFaresDiag319);
  CPPUNIT_TEST(testgetDiscInfoFromRule_SoftPassWhenDir3SoftPass_IsDirectionPass);
  CPPUNIT_TEST(testgetDiscInfoFromRule_PassWhenDir3Pass_IsDirectionPass);
  CPPUNIT_TEST(testgetDiscInfoFromRule_FailWhenDir3Fail_IsDirectionPass);
  CPPUNIT_TEST(testgetDiscInfoFromRule_FailCNNWhenPreviousRec3NotAllowCNNDiscMatch994);
  CPPUNIT_TEST(testgetDiscInfoFromRule_PassCNNWhenPreviousRec3NotAllowCNNDiscSoftPass);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  PricingOptions* _options;
  Itin* _itin;
  MyDataHandle* _mdh;
  PaxType *_adtPT;
  FareMarket* _lonDfwFm;
  CategoryRuleItemInfoSet* _ruleSet;
  MockDiscountedFareController* _discFc;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _mdh = _memHandle.create<MyDataHandle>();
    _ruleSet = _memHandle.create<CategoryRuleItemInfoSet>();
    _trx->itin().push_back(_itin);
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _adtPT = TestPaxTypeFactory::create("/vobs/atseintl/test/testdata/data/PaxType.xml");
    _lonDfwFm = TestFareMarketFactory::create("/vobs/atseintl/test/testdata/data/FareMarket.xml");

    _trx->diagnostic().diagnosticType() = Diagnostic219;
    _trx->diagnostic().activate();
    _trx->paxType().push_back(_adtPT);

    _discFc = _memHandle.insert(new MockDiscountedFareController(*_trx, *_itin, *_lonDfwFm));
    fallback::value::fallbackAPO37838Record1EffDateCheck.set(true);
  }

  void tearDown() { _memHandle.clear(); }

  void testProcess()
  {
    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcais;

    TariffCrossRefInfo tcri;

    fare.initialize(Fare::FS_International, &fi, *_lonDfwFm, &tcri);

    MockPaxTypeFare ptFare(*_lonDfwFm, fare, fcai, fcais, *_adtPT);

    fi._fareClass = "B2";
    fi._fareAmount = 100;
    fi._currency = "USD";
    fi._carrier = "BA";
    fi._market1 = "DFW";
    fi._market2 = "MAN";
    fare.setFareInfo(&fi);
    ptFare.setFare(&fare);

    DiscountInfo di;
    di.paxType() = "ADT";
    di.minAge() = 18;
    di.maxAge() = 200;
    di.tktCodeModifier() = "TV1";

    ptFare.setDiscountInfo(&di);

    GeneralFareRuleInfo genInfo;
    genInfo.vendorCode() = "ATP";
    genInfo.carrierCode() = "AA";
    genInfo.tariffNumber() = 777;
    genInfo.ruleNumber() = "2";
    genInfo.sequenceNumber() = 1;

    ptFare.setGeneralFareRuleInfo(&genInfo);

    CategoryRuleItemInfo qual1;
    qual1.setRelationalInd(CategoryRuleItemInfo::IF);
    qual1.setItemcat(1);
    qual1.setItemNo(1111);

    CategoryRuleItemInfo qual2;
    qual2.setRelationalInd(CategoryRuleItemInfo::AND);
    qual2.setItemcat(2);
    qual2.setItemNo(2222);

    std::vector<CategoryRuleItemInfo> set;
    set.push_back(qual1);
    set.push_back(qual2);

    ptFare.setCategoryRuleItemInfoSet(set);

    fi._ruleNumber = "2002";
    fi._fareClass = "B2";
    fi._carrier = "BA";
    fi._vendor = "ATP";
    tcri._ruleTariff = 1;

    CPPUNIT_ASSERT(_discFc->process());
  }

  void testProcessRule()
  {
    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcais;

    TariffCrossRefInfo tcri;

    fare.initialize(Fare::FS_International, &fi, *_lonDfwFm, &tcri);

    MockPaxTypeFare ptFare(*_lonDfwFm, fare, fcai, fcais, *_adtPT);
    PaxTypeFare* newPTF;

    fi._fareClass = "1";
    fi._fareAmount = 100;
    fi._carrier = "AA";
    fi._currency = "USD";
    fi._market1 = "DFW";
    fi._market2 = "MAN";

    fare.setFareInfo(&fi);
    ptFare.setFare(&fare);

    DiscountInfo di;
    di.paxType() = "ADT";
    di.minAge() = 18;
    di.maxAge() = 200;
    di.tktCodeModifier() = "TV1";

    ptFare.setDiscountInfo(&di);

    CategoryRuleItemInfo itemInfo1;
    CategoryRuleItemInfo itemInfo2;
    CategoryRuleItemInfo itemInfo3;
    CategoryRuleItemInfo itemInfo4;
    CategoryRuleItemInfo itemInfo5;
    CategoryRuleItemInfo itemInfo6;
    CategoryRuleItemInfo itemInfo7;
    CategoryRuleItemInfo itemInfo8;
    CategoryRuleItemInfo itemInfo9;

    itemInfo1.setRelationalInd(CategoryRuleItemInfo::THEN);
    itemInfo2.setRelationalInd(CategoryRuleItemInfo::OR);
    itemInfo3.setRelationalInd(CategoryRuleItemInfo::OR);
    itemInfo4.setRelationalInd(CategoryRuleItemInfo::IF);
    itemInfo5.setRelationalInd(CategoryRuleItemInfo::OR);
    itemInfo6.setRelationalInd(CategoryRuleItemInfo::OR);
    itemInfo7.setRelationalInd(CategoryRuleItemInfo::AND);
    itemInfo8.setRelationalInd(CategoryRuleItemInfo::OR);
    itemInfo9.setRelationalInd(CategoryRuleItemInfo::AND);

    itemInfo1.setItemNo(1);
    itemInfo2.setItemNo(2);
    itemInfo3.setItemNo(3);
    itemInfo4.setItemNo(4);
    itemInfo5.setItemNo(5);
    itemInfo6.setItemNo(6);
    itemInfo7.setItemNo(7);
    itemInfo8.setItemNo(8);
    itemInfo9.setItemNo(9);

    itemInfo1.setItemcat(19);
    itemInfo2.setItemcat(19);
    itemInfo3.setItemcat(19);
    itemInfo4.setItemcat(19);
    itemInfo5.setItemcat(19);
    itemInfo6.setItemcat(19);
    itemInfo7.setItemcat(19);
    itemInfo8.setItemcat(19);
    itemInfo9.setItemcat(19);

    CategoryRuleItemInfoSet* ruleSet = new CategoryRuleItemInfoSet();
    ruleSet->push_back(itemInfo1);
    ruleSet->push_back(itemInfo2);
    ruleSet->push_back(itemInfo3);
    ruleSet->push_back(itemInfo4);
    ruleSet->push_back(itemInfo5);
    ruleSet->push_back(itemInfo6);
    ruleSet->push_back(itemInfo7);
    ruleSet->push_back(itemInfo8);
    ruleSet->push_back(itemInfo9);

    GeneralFareRuleInfo ruleInfo;
    ruleInfo.vendorCode() = "ATP";
    ruleInfo.addItemInfoSetNosync(ruleSet);

    PaxTypeFareRuleData rd;
    rd.ruleItemInfo() = &di;
    rd.categoryRuleInfo() = &ruleInfo;

    DiscountedFareController::DiscRule dr = DiscountedFareController::DiscRule(&di, &rd);
    _discFc->_calcMoney.getFromPTF(ptFare);

    newPTF = _discFc->makeFare(&ptFare, dr, _discFc->_calcMoney);
    CPPUNIT_ASSERT(newPTF);
  }

  MockPaxTypeFare* createFareDfwLon()
  {
    Fare *fare = _memHandle.create<Fare>();
    FareInfo *fi = _memHandle.create<FareInfo>();
    FareClassAppInfo *fcai = _memHandle.create<FareClassAppInfo>();
    FareClassAppSegInfo *fcais = _memHandle.create<FareClassAppSegInfo>();

    fi->_fareClass  = "1";
    fi->_fareAmount = 100;
    fi->_carrier    = "AA";
    fi->_market1 = "DFW";
    fi->_market2 = "LON";

    fare->setFareInfo(fi);

    MockPaxTypeFare *ptFare = new MockPaxTypeFare(*_lonDfwFm, *fare, *fcai, *fcais, *_adtPT);

    ptFare->setFare(fare);

    return ptFare;
  }

  DiscountInfo* createDiscount(MockPaxTypeFare* ptf)
  {
    DiscountInfo *di = _memHandle.create<DiscountInfo>();
    di->paxType() = "ADT";
    di->minAge() = 18;
    di->maxAge() = 200;
    di->tktCodeModifier() = "TV1";

    ptf->setDiscountInfo(di);

    return di;
  }

  GeneralFareRuleInfo* createGfr(MockPaxTypeFare* ptf)
  {
    GeneralFareRuleInfo *genInfo = _memHandle.create<GeneralFareRuleInfo>();
    genInfo->vendorCode()     = "ATP";
    genInfo->carrierCode()    = "AA";
    genInfo->tariffNumber()   = 777;
    genInfo->ruleNumber()     = "2";
    genInfo->sequenceNumber() = 1;

    ptf->setGeneralFareRuleInfo(genInfo);

    return genInfo;
  }

  void testValidate_Pass()
  {
    _discFc->setRealValidate();

    DCFactory *factory = DCFactory::instance();
    DiagCollector* dc  = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    DiscountInfo *di = createDiscount(ptFare);
    GeneralFareRuleInfo *genInfo = createGfr(ptFare);

    CPPUNIT_ASSERT(_discFc->validate(*dc, *ptFare, *genInfo, *di, *_ruleSet));
  }

  void testValidate_Accomp()
  {
    _discFc->setRealValidate();

    DCFactory *factory = DCFactory::instance();
    DiagCollector* dc  = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    DiscountInfo *di = createDiscount(ptFare);
    GeneralFareRuleInfo *genInfo = createGfr(ptFare);

    di->accInd() = DiscountedFareController::REQ_ACC_TVL;
    DiscountSegInfo* dsi = new DiscountSegInfo();
    dsi->accPsgType1() = "XXX";
    di->segs().push_back(dsi);

    CPPUNIT_ASSERT(!_discFc->validate(*dc, *ptFare, *genInfo, *di, *_ruleSet));
  }

  void testValidate_Geo()
  {
    _discFc->setRealValidate();

    DCFactory *factory = DCFactory::instance();
    DiagCollector* dc  = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    DiscountInfo *di = createDiscount(ptFare);
    GeneralFareRuleInfo *genInfo = createGfr(ptFare);

    di->geoTblItemNo() = 1234;

    CPPUNIT_ASSERT(!_discFc->validate(*dc, *ptFare, *genInfo, *di, *_ruleSet));
  }

  void testValidate_Cur1()
  {
    _discFc->setRealValidate();

    DCFactory *factory = DCFactory::instance();
    DiagCollector* dc  = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    DiscountInfo *di = createDiscount(ptFare);
    GeneralFareRuleInfo *genInfo = createGfr(ptFare);

    di->fareAmt1() = 100;
    di->vendor() = "CUR";
    di->cur1() = "XXX";
    _ruleSet->push_back(CategoryRuleItemInfo());

    CPPUNIT_ASSERT(!_discFc->validate(*dc, *ptFare, *genInfo, *di, *_ruleSet));
  }

  void testCalcAmount()
  {
    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcais;

    TariffCrossRefInfo tcri;

    fare.initialize(Fare::FS_International, &fi, *_lonDfwFm, &tcri);

    MockPaxTypeFare ptFare(*_lonDfwFm, fare, fcai, fcais, *_adtPT);
    DiscountInfo *di = createDiscount(&ptFare);

    di->farecalcInd() = DiscountedFareController::SPECIFIED;
    di->fareAmt1() = 500;

    _discFc->calcAmount(ptFare, *di);
    CPPUNIT_ASSERT(_discFc->_calcMoney.nucValue() == 500);
  }

  void testCalcPercentage()
  {
    MoneyAmount fareAmt = 100;
    Percent percent = 50;
    MoneyAmount result = 0;

    result = DiscountedFareController::calcPercentage(fareAmt, percent);
    CPPUNIT_ASSERT(result == 50);

    fareAmt = 100;
    percent = 25;
    result = 0;

    result = DiscountedFareController::calcPercentage(fareAmt, percent);
    CPPUNIT_ASSERT(result == 25);

    fareAmt = 100;
    percent = 75;
    result = 0;

    result = DiscountedFareController::calcPercentage(fareAmt, percent);
    CPPUNIT_ASSERT(result == 75);
  }

  void testWriteDiag219()
  {
    _discFc->_diag219On = true;

    DCFactory* factory = DCFactory::instance();
    DiagCollector* dc = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    createDiscount(ptFare);
    createGfr(ptFare);

    _discFc->writeDiag219(*dc, *ptFare);

    CPPUNIT_ASSERT(!dc->bad());
  }

  void testWriteDiag319()
  {
    _discFc->_diag319On = true;

    DCFactory* factory = DCFactory::instance();
    DiagCollector* dc = factory->create(*_trx);

    MockPaxTypeFare* ptFare = createFareDfwLon();
    DiscountInfo *di = createDiscount(ptFare);
    GeneralFareRuleInfo *genInfo = createGfr(ptFare);

    _discFc->writeDiag319(*dc, *ptFare, *genInfo, *di);

    CPPUNIT_ASSERT(!dc->bad());
  }

  void testWriteBaseFaresDiag319()
  {
    _discFc->_diag319On = true;

    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;
    FareClassAppSegInfo fcais;

    TariffCrossRefInfo tcri;

    fare.initialize(Fare::FS_International, &fi, *_lonDfwFm, &tcri);

    MockPaxTypeFare ptFare(*_lonDfwFm, fare, fcai, fcais, *_adtPT);

    fare.status().set(Fare::FS_PublishedFare);

    fi._fareClass = "1";
    fi._fareAmount = 100;
    fi._carrier = "AA";
    fi._market1 = "DFW";
    fi._market2 = "MAN";

    fcai._owrt = 'O';

    ptFare.setFare(&fare);
    ptFare.fareClassAppInfo() = &fcai;

    std::vector<PaxTypeFare*> fares;
    fares.push_back(&ptFare);

    DCFactory* factory = DCFactory::instance();
    DiagCollector* dc = factory->create(*_trx);
    createDiscount(&ptFare);

    _discFc->writeBaseFaresDiag319(*dc, fares);

    CPPUNIT_ASSERT(!dc->bad());
  }

  void testWriteDiagnostics()
  {
    _discFc->_diag219On = true;
    _discFc->_diag319On = true;

    DCFactory* factory = DCFactory::instance();
    DiagCollector* dc = factory->create(*_trx);

    _discFc->writeDiagnostics();

    CPPUNIT_ASSERT(!dc->bad());
  }

  void testgetDiscInfoFromRule_SoftPassWhenDir3SoftPass_IsDirectionPass()
  {
    PaxTypeFare* ptFare = buildPtfForGetDiscInfoFromRuleTest();

    bool isLocationSwapped;

    CategoryRuleInfo* ruleInfo = buildRuleForGetDiscInforFromRuleTest(
        *_trx, *ptFare, *_itin, SOFTPASS, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2, isLocationSwapped);

    MockDiscountedFareController fc(*_trx, *_itin, *ptFare->fareMarket());

    const bool allowDup = true;
    CPPUNIT_ASSERT_EQUAL(true,
                         fc.getDiscInfoFromRule(*ptFare, ruleInfo, isLocationSwapped, allowDup));
    CPPUNIT_ASSERT_EQUAL(1, (int)fc._discRules.size());
    CPPUNIT_ASSERT_EQUAL(true, (*(fc._discRules.begin())).second->isSoftPassDiscount());
  }

  void testgetDiscInfoFromRule_PassWhenDir3Pass_IsDirectionPass()
  {
    PaxTypeFare* ptFare = buildPtfForGetDiscInfoFromRuleTest();

    bool isLocationSwapped;

    CategoryRuleInfo* ruleInfo = buildRuleForGetDiscInforFromRuleTest(
        *_trx, *ptFare, *_itin, PASS, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2, isLocationSwapped);

    MockDiscountedFareController fc(*_trx, *_itin, *ptFare->fareMarket());
    const bool allowDup = true;
    CPPUNIT_ASSERT_EQUAL(true,
                         fc.getDiscInfoFromRule(*ptFare, ruleInfo, isLocationSwapped, allowDup));
    CPPUNIT_ASSERT_EQUAL(1, (int)fc._discRules.size());
    CPPUNIT_ASSERT_EQUAL(false, (*(fc._discRules.begin())).second->isSoftPassDiscount());
  }

  void testgetDiscInfoFromRule_FailWhenDir3Fail_IsDirectionPass()
  {
    PaxTypeFare* ptFare = buildPtfForGetDiscInfoFromRuleTest();

    bool isLocationSwapped;

    CategoryRuleInfo* ruleInfo = buildRuleForGetDiscInforFromRuleTest(
        *_trx, *ptFare, *_itin, FAIL, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2, isLocationSwapped);

    MockDiscountedFareController fc(*_trx, *_itin, *ptFare->fareMarket());
    const bool allowDup = true;
    CPPUNIT_ASSERT_EQUAL(false,
                         fc.getDiscInfoFromRule(*ptFare, ruleInfo, isLocationSwapped, allowDup));
    CPPUNIT_ASSERT_EQUAL(true, fc._discRules.empty());
  }

  void testgetDiscInfoFromRule_FailCNNWhenPreviousRec3NotAllowCNNDiscMatch994()
  {
    PaxTypeFare* ptFare = buildPtfForGetDiscInfoFromRuleTest();

    bool isLocationSwapped;

    CategoryRuleInfo* ruleInfo = buildRuleForGetDiscInforFromRuleTest(
        *_trx, *ptFare, *_itin, PASS, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2, isLocationSwapped);

    MockDiscountedFareController fc(*_trx, *_itin, *ptFare->fareMarket());
    chgRuleInfoWithItem1NoDiscItem2Disc(fc, *ruleInfo);
    const bool allowDup = true;

    // should only have on discRule which indicates discount not apply
    CPPUNIT_ASSERT_EQUAL(true,
                         fc.getDiscInfoFromRule(*ptFare, ruleInfo, isLocationSwapped, allowDup));
    CPPUNIT_ASSERT_EQUAL(1, (int)fc._discRules.size());
    const DiscountInfo* discInfo = fc._discRules.begin()->first;
    CPPUNIT_ASSERT_EQUAL('X', discInfo->discAppl());
  }

  void testgetDiscInfoFromRule_PassCNNWhenPreviousRec3NotAllowCNNDiscSoftPass()
  {
    PaxTypeFare* ptFare = buildPtfForGetDiscInfoFromRuleTest();

    bool isLocationSwapped;

    CategoryRuleInfo* ruleInfo = buildRuleForGetDiscInforFromRuleTest(
        *_trx, *ptFare, *_itin, SOFTPASS, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2, isLocationSwapped);

    MockDiscountedFareController fc(*_trx, *_itin, *ptFare->fareMarket());
    chgRuleInfoWithItem1NoDiscItem2Disc(fc, *ruleInfo);
    const bool allowDup = true;

    // should only have on discRule which indicates discount not apply
    CPPUNIT_ASSERT_EQUAL(true,
                         fc.getDiscInfoFromRule(*ptFare, ruleInfo, isLocationSwapped, allowDup));
    CPPUNIT_ASSERT_EQUAL(2, (int)fc._discRules.size());
    const DiscountInfo* discInfo = fc._discRules.begin()->first;
    CPPUNIT_ASSERT_EQUAL('X', discInfo->discAppl());
    discInfo = (fc._discRules.begin() + 1)->first;
    CPPUNIT_ASSERT_EQUAL(' ', discInfo->discAppl());
  }

  PaxTypeFare* buildPtfForGetDiscInfoFromRuleTest()
  {
    PaxTypeFare* ptFare = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    ptFare->fareMarket() = fareMarket;
    fareInfo->directionality() = BOTH;
    fareInfo->carrier() = "AA";
    fare->setFareInfo(fareInfo);
    ptFare->setFare(fare);

    AirSeg* tvlSeg1 = _memHandle.create<AirSeg>();
    tvlSeg1->origAirport() = "OG1";
    tvlSeg1->destAirport() = "DS1";
    tvlSeg1->boardMultiCity() = "OG1";
    tvlSeg1->offMultiCity() = "DS1";
    fareMarket->travelSeg().push_back(tvlSeg1);

    return ptFare;
  }

  CategoryRuleInfo* buildRuleForGetDiscInforFromRuleTest(PricingTrx& trx,
                                                         PaxTypeFare& ptFare,
                                                         Itin& itin,
                                                         const Record3ReturnTypes& isDirPassRtn,
                                                         const char R2Directionality,
                                                         bool& isLocationSwapped)
  {
    CategoryRuleInfo* ruleInfo = _memHandle.create<CategoryRuleInfo>();
    CategoryRuleItemInfoSet* ruleItemInfoSet = new CategoryRuleItemInfoSet();
    CategoryRuleItemInfo catRuleItemInfo;

    catRuleItemInfo.setItemcat(19);
    catRuleItemInfo.setItemNo(9);
    catRuleItemInfo.setInOutInd(RuleConst::ALWAYS_APPLIES);
    catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::THEN);
    ruleInfo->vendorCode() = "SITA";

    const DiscountInfo* discInfo = _trx->dataHandle().getDiscount(
        ruleInfo->vendorCode(), catRuleItemInfo.itemNo(), catRuleItemInfo.itemcat());
    CPPUNIT_ASSERT_EQUAL(true, discInfo != 0);
    CPPUNIT_ASSERT_EQUAL(
        true,
        TestRuleCPPUnitHelper::buildTestItinAndRuleForIsDirectionPass(
            catRuleItemInfo, isDirPassRtn, R2Directionality, ptFare, itin, isLocationSwapped));
    ruleItemInfoSet->push_back(catRuleItemInfo);
    ruleInfo->addItemInfoSetNosync(ruleItemInfoSet);
    return ruleInfo;
  }

  void chgRuleInfoWithItem1NoDiscItem2Disc(DiscountedFareController& discController,
                                           CategoryRuleInfo& ruleInfo)
  {
    MockDiscountedFareController& mockDiscController =
        static_cast<MockDiscountedFareController&>(discController);
    DiscountInfo* disc1 = const_cast<DiscountInfo*>(mockDiscController.getDiscountInfo(
        &ruleInfo, &ruleInfo.categoryRuleItemInfoSet().front()->front()));

    disc1->paxType() = "CNN";
    disc1->overrideDateTblItemNo() = 1111;
    disc1->discAppl() = 'X';

    CategoryRuleItemInfo catRuleItem2;
    catRuleItem2.setItemcat(19);
    catRuleItem2.setItemNo(10);
    catRuleItem2.setInOutInd(RuleConst::ALWAYS_APPLIES);
    catRuleItem2.setRelationalInd(CategoryRuleItemInfo::OR);

    ruleInfo.categoryRuleItemInfoSet().front()->push_back(catRuleItem2);

    DiscountInfo* disc2 =
        const_cast<DiscountInfo*>(mockDiscController.getDiscountInfo(&ruleInfo, &catRuleItem2));
    disc2->paxType() = "CNN";
    disc2->overrideDateTblItemNo() = 0;
    disc2->discAppl() = ' ';
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DiscountedFareControllerTest);
}
