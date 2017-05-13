#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/PaxTypeInfo.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

class PaxTypeFareMock : public PaxTypeFare
{
public:
  void addRuleData(int cat, PaxTypeFareAllRuleData* allRuleData)
  {
    (*_paxTypeFareRuleDataMap)[cat] = allRuleData;
  }
};

class PaxTypeFareComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeFareComparatorTest);

  CPPUNIT_TEST(testNullLeft);
  CPPUNIT_TEST(testNullRight);

  CPPUNIT_TEST(testNucAmountLeft);
  CPPUNIT_TEST(testNucAmountRight);

  CPPUNIT_TEST(testCabinLeft);
  CPPUNIT_TEST(testCabinRight);

  CPPUNIT_TEST(testMileageLeft);
  CPPUNIT_TEST(testMileageRight);

  CPPUNIT_TEST(testFareByRuleLeft);
  CPPUNIT_TEST(testFareByRuleRight);

  CPPUNIT_TEST(testPaxTypeNumLeft);
  CPPUNIT_TEST(testPaxTypeNumRight);

  CPPUNIT_TEST(testWebFareLeft);
  CPPUNIT_TEST(testWebFareRight);

  CPPUNIT_TEST(testRequestedPaxLeft);
  CPPUNIT_TEST(testRequestedPaxRight);
  CPPUNIT_TEST(testNotRequestedPaxLeft);
  CPPUNIT_TEST(testNotRequestedPaxRight);

  CPPUNIT_TEST(testIndustryCarrierLeft);
  CPPUNIT_TEST(testIndustryCarrierRight);

  CPPUNIT_TEST(testPaxTypeStatusLeft);
  CPPUNIT_TEST(testPaxTypeStatusRight);

  CPPUNIT_TEST(testFareClassLeft);
  CPPUNIT_TEST(testFareClassRight);

  CPPUNIT_TEST(testRuleLeft);
  CPPUNIT_TEST(testRuleRight);

  CPPUNIT_TEST(testNegotiatedLeft);
  CPPUNIT_TEST(testNegotiatedRight);

  CPPUNIT_TEST(testEqual);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _dh = _memHandle.create<DataHandleMock>();

    _pricingOptions = _memHandle.create<PricingOptions>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_pricingOptions);
    _fareMarket = _memHandle.create<FareMarket>();
    _fareMarket->paxTypeCortege().resize(1);
    _paxTypeCortege = &_fareMarket->paxTypeCortege()[0];

    _lFareInfo = _memHandle.create<FareInfo>();
    _lFare = _memHandle.create<Fare>();
    _lFare->setFareInfo(_lFareInfo);
    _lptf = _memHandle.create<PaxTypeFareMock>();
    _lptf->setFare(_lFare);
    _lptf->fareMarket() = _fareMarket;

    _rFareInfo = _memHandle.create<FareInfo>();
    _rFare = _memHandle.create<Fare>();
    _rFare->setFareInfo(_rFareInfo);
    _rptf = _memHandle.create<PaxTypeFareMock>();
    _rptf->setFare(_rFare);
    _rptf->fareMarket() = _fareMarket;
  }

  void tearDown() { _memHandle.clear(); }

  void testNullLeft()
  {
    setCortegePaxType(ADULT);
    _lptf = 0;
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testNullRight()
  {
    setCortegePaxType(ADULT);
    _rptf = 0;
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testNucAmountLeft()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    _lFare->nucFareAmount() = 10.0;
    _rFare->nucFareAmount() = 20.0;

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testNucAmountRight()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    _lFare->nucFareAmount() = 30.0;
    _rFare->nucFareAmount() = 20.0;

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testCabinLeft()
  {
    setCortegePaxType(ADULT);
    _pricingOptions->setZeroFareLogic(true);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    _lptf->cabin().setEconomyClass();
    _rptf->cabin().setBusinessClass();

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testCabinRight()
  {
    setCortegePaxType(ADULT);
    _pricingOptions->setZeroFareLogic(true);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    _lptf->cabin().setBusinessClass();
    _rptf->cabin().setEconomyClass();

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testMileageLeft()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    _lptf->mileage() = 100;
    _rptf->mileage() = 1000;

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testMileageRight()
  {
    setCortegePaxType(ADULT);
    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->setAltDates(true);
    _pricingOptions->setZeroFareLogic(true);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    _lptf->mileage() = 100;
    _rptf->mileage() = 10;

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testFareByRuleLeft()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    _lptf->status().set(PaxTypeFare::PTF_FareByRule);
    _rptf->status().clear(PaxTypeFare::PTF_FareByRule);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testFareByRuleRight()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    _lptf->status().clear(PaxTypeFare::PTF_FareByRule);
    _rptf->status().set(PaxTypeFare::PTF_FareByRule);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testPaxTypeNumLeft()
  {
    using namespace boost::assign;

    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();

    _lptf->actualPaxTypeItem().clear();
    _lptf->actualPaxTypeItem() += 2;
    _rptf->actualPaxTypeItem().clear();
    _rptf->actualPaxTypeItem() += 3;

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testPaxTypeNumRight()
  {
    using namespace boost::assign;

    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();

    _lptf->actualPaxTypeItem().clear();
    _lptf->actualPaxTypeItem() += 4;
    _rptf->actualPaxTypeItem().clear();
    _rptf->actualPaxTypeItem() += 3;

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testWebFareLeft()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();

    _lptf->setWebFare(true);
    _rptf->setWebFare(false);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testWebFareRight()
  {
    setCortegePaxType(ADULT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();

    _lptf->setWebFare(false);
    _rptf->setWebFare(true);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testRequestedPaxLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();

    setRequestedPax(CHILD, INFANT);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testRequestedPaxRight()
  {
    setCortegePaxType(INFANT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();

    setRequestedPax(CHILD, INFANT);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testNotRequestedPaxLeft()
  {
    setCortegePaxType(INFANT);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();

    setRequestedPax(CHILD, ADULT);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testNotRequestedPaxRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();

    setRequestedPax(ADULT, INFANT);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testIndustryCarrierLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();

    _lFareInfo->carrier() = CarrierCode("AA");
    _lptf->setFare(_lFare);

    _rFareInfo->carrier() = INDUSTRY_CARRIER;
    _rptf->setFare(_rFare);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testIndustryCarrierRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();

    _lFareInfo->carrier() = INDUSTRY_CARRIER;
    _lptf->setFare(_lFare);

    _rFareInfo->carrier() = CarrierCode("AA");
    _rptf->setFare(_rFare);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testPaxTypeStatusLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();

    setPaxTypeStatusChild(_lptf);
    setPaxTypeStatusAdult(_rptf);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testPaxTypeStatusRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();

    setPaxTypeStatusAdult(_lptf);
    setPaxTypeStatusChild(_rptf);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testFareClassLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();

    _lFareInfo->fareClass() = FareClassCode("C");
    _rFareInfo->fareClass() = FareClassCode("Y");

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testFareClassRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();

    _lFareInfo->fareClass() = FareClassCode("Y");
    _rFareInfo->fareClass() = FareClassCode("C");

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testRuleLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();
    setEqualFareClass();

    _lFareInfo->ruleNumber() = RuleNumber("0001");
    _lptf->setFare(_lFare);
    _rFareInfo->ruleNumber() = RuleNumber("0002");
    _rptf->setFare(_rFare);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testRuleRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();
    setEqualFareClass();

    _lFareInfo->ruleNumber() = RuleNumber("0002");
    _lptf->setFare(_lFare);
    _rFareInfo->ruleNumber() = RuleNumber("0001");
    _rptf->setFare(_rFare);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testNegotiatedLeft()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();
    setEqualFareClass();
    setEqualRule();

    _lptf->status().set(PaxTypeFare::PTF_Negotiated, true);
    _rptf->status().set(PaxTypeFare::PTF_Negotiated, true);

    setFareRuleData(_lptf, 35, 5);
    setFareRuleData(_rptf, 35, 7);

    CPPUNIT_ASSERT(comparator(_lptf, _rptf));
  }

  void testNegotiatedRight()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();
    setEqualFareClass();
    setEqualRule();

    _lptf->status().set(PaxTypeFare::PTF_Negotiated, true);
    _rptf->status().set(PaxTypeFare::PTF_Negotiated, true);

    setFareRuleData(_lptf, 35, 9);
    setFareRuleData(_rptf, 35, 7);

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
  }

  void testEqual()
  {
    setCortegePaxType(CHILD);
    PaxTypeFare::FareComparator comparator(
        *_paxTypeCortege, 0, _trx->getOptions()->isZeroFareLogic());

    setEqualAmounts();
    setEqualCabins();
    setEqualMileage();
    setEqualFareByRule();
    setEqualPaxTypeNum();
    setEqualWebFare();
    setEqualRequestedPax();
    setEqualCarrier();
    setEqualPaxTypeStatus();
    setEqualFareClass();
    setEqualRule();
    setEqualNegotiated();

    CPPUNIT_ASSERT(!comparator(_lptf, _rptf));
    CPPUNIT_ASSERT(!comparator(_rptf, _lptf));
  }

protected:
  void setEqualAmounts()
  {
    _lFare->nucFareAmount() = 20.0;
    _rFare->nucFareAmount() = 20.0;
  }

  void setEqualCabins()
  {
    _lptf->cabin().setEconomyClass();
    _rptf->cabin().setEconomyClass();
  }

  void setEqualMileage()
  {
    _lptf->mileage() = 0;
    _rptf->mileage() = 0;
  }

  void setEqualFareByRule()
  {
    _lptf->status().clear(PaxTypeFare::PTF_FareByRule);
    _rptf->status().clear(PaxTypeFare::PTF_FareByRule);
  }

  void setEqualPaxTypeNum()
  {
    using namespace boost::assign;

    _lptf->actualPaxTypeItem().clear();
    _lptf->actualPaxTypeItem() += 3;
    _rptf->actualPaxTypeItem().clear();
    _rptf->actualPaxTypeItem() += 3;
  }

  void setEqualWebFare()
  {
    _lptf->setWebFare(false);
    _rptf->setWebFare(false);
  }

  void setRequestedPax(PaxTypeCode l, PaxTypeCode r)
  {
    FareClassAppSegInfo* lSegInfo = _memHandle.create<FareClassAppSegInfo>();
    lSegInfo->_paxType = l;
    _lptf->fareClassAppSegInfo() = lSegInfo;

    FareClassAppSegInfo* rSegInfo = _memHandle.create<FareClassAppSegInfo>();
    rSegInfo->_paxType = r;
    _rptf->fareClassAppSegInfo() = rSegInfo;
  }

  void setEqualRequestedPax()
  {
    setRequestedPax(ADULT, ADULT);
  }

  void setEqualCarrier()
  {
    _lFareInfo->carrier() = CarrierCode("AA");
    _lptf->setFare(_lFare);

    _rFareInfo->carrier() = CarrierCode("AA");
    _rptf->setFare(_rFare);
  }

  void setPaxTypeStatus(PaxTypeFare* ptf, const PaxTypeCode& paxType)
  {
    PaxTypeInfo* ptInfo = _memHandle.create<PaxTypeInfo>();

    ptInfo->paxType() = paxType;
    ptInfo->initPsgType();

    ptf->paxType() = _memHandle.create<PaxType>();
    ptf->paxType()->paxTypeInfo() = ptInfo;
  }

  void setPaxTypeStatusChild(PaxTypeFare* ptf) { setPaxTypeStatus(ptf, JNN); }

  void setPaxTypeStatusAdult(PaxTypeFare* ptf) { setPaxTypeStatus(ptf, JCB); }

  void setEqualPaxTypeStatus()
  {
    setPaxTypeStatusAdult(_lptf);
    setPaxTypeStatusAdult(_rptf);
  }

  void setEqualFareClass()
  {
    _lFareInfo->fareClass() = FareClassCode("Y");
    _rFareInfo->fareClass() = FareClassCode("Y");
  }

  void setEqualRule()
  {
    _lFareInfo->ruleNumber() = RuleNumber("0001");
    _lptf->setFare(_lFare);
    _rFareInfo->ruleNumber() = RuleNumber("0001");
    _rptf->setFare(_rFare);
  }

  void setFareRuleData(PaxTypeFareMock* ptf, int cat, uint32_t itemNo)
  {
    CategoryRuleItemInfo* ruleItemInfo = _memHandle.create<CategoryRuleItemInfo>();
    ruleItemInfo->setItemNo(itemNo);

    PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
    ruleData->categoryRuleItemInfo() = ruleItemInfo;

    PaxTypeFare::PaxTypeFareAllRuleData* allRuleData =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRuleData->chkedRuleData = true;
    allRuleData->chkedGfrData = false;
    allRuleData->fareRuleData = ruleData;
    allRuleData->gfrRuleData = 0;

    ptf->addRuleData(cat, allRuleData);
  }

  void setEqualNegotiated()
  {
    _lptf->status().set(PaxTypeFare::PTF_Negotiated, true);
    _rptf->status().set(PaxTypeFare::PTF_Negotiated, true);

    setFareRuleData(_lptf, 35, 5);
    setFareRuleData(_rptf, 35, 5);
  }

  void setCortegePaxType(PaxTypeCode paxCode)
  {
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->number() = 1;
    paxType->paxType() = paxCode;
    _paxTypeCortege->requestedPaxType() = paxType;
  }

private:
  TestMemHandle _memHandle;
  DataHandleMock* _dh;

  PricingOptions* _pricingOptions;
  PricingTrx* _trx;
  FareMarket* _fareMarket;
  PaxTypeBucket* _paxTypeCortege;

  FareInfo* _lFareInfo;
  Fare* _lFare;
  PaxTypeFareMock* _lptf;

  FareInfo* _rFareInfo;
  Fare* _rFare;
  PaxTypeFareMock* _rptf;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeFareComparatorTest);

} // tse
