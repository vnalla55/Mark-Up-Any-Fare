#include <vector>
#include "Common/DateTime.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"


namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate)
  {
    return _generalFareRule;
  }

  bool
  getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const TariffNumber& tariffNumber,
                                       const RuleNumber& ruleNumber,
                                       CatNumber catNum,
                                       RuleNumber& ruleNumOut,
                                       TariffNumber& tariffNumOut,
                                       DateTime tvlDate)
  {
    return false;
  }
  const std::vector<GeneralFareRuleInfo*> _generalFareRule;
};
}

class PaxTypeFareMock : public PaxTypeFare
{
public:
  PaxTypeFareMock()
  {
     _fareInfo._vendor = "ATP";
     _fareInfo._footnote1 = "";
     _fareInfo._footnote2 = "";
     _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _market);
     setFare(&_fare);
     _fareMarket = &_market;
  }
private:
  Fare _fare;
  FareInfo _fareInfo;
  FareMarket _market;
};

class ShoppingAltDateUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingAltDateUtilTest);
  CPPUNIT_TEST(testAltGetTravelDateRange);
  CPPUNIT_TEST(testGetDuration);
  CPPUNIT_TEST(testGetNoDays);
  CPPUNIT_TEST(testValidateAltDatePair_Duration);
  CPPUNIT_TEST(testCleanUpProcessedCategoriesForFareByRule);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = createTrx("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testAltGetTravelDateRange()
  {
    DatePair dateRecieved, dateSet(DateTime(2008, 5, 12), DateTime(2008, 5, 28));
    ShoppingTrx* shoppingTrx(0);
    _memHandle.get(shoppingTrx);
    setDepartureDates(shoppingTrx);
    dateRecieved = ShoppingAltDateUtil::getTravelDateRange(*shoppingTrx);

    if (dateRecieved == dateSet)
      CPPUNIT_ASSERT(true);
    else
      CPPUNIT_ASSERT(false);
  }

  void testGetDuration()
  {
    setTransactionData();

    SopIdVec sops(2);
    sops[0] = 1;
    sops[1] = 1;

    uint64_t expected(7);
    uint64_t duration = ShoppingAltDateUtil::getDuration(*_trx, sops);

    uint16_t lastNoDays = ((duration / 1000000) / HOURS_PER_DAY) / SECONDS_PER_HOUR;

    CPPUNIT_ASSERT(lastNoDays == expected);
  }

  void testGetNoDays()
  {
    setTransactionData();

    SopIdVec sops(2);
    sops[0] = 1;
    sops[1] = 1;

    uint64_t expected(7);
    uint64_t duration = ShoppingAltDateUtil::getDuration(*_trx, sops);
    uint16_t lastNoDays = ShoppingAltDateUtil::getNoOfDays(duration);

    CPPUNIT_ASSERT(lastNoDays == expected);
  }

  void testValidateAltDatePair_Duration()
  {
    setTransactionData();

    DatePair valid1(DateTime(2014, 4, 25), DateTime(2014, 4, 28));
    _trx->minDuration() = 0;
    _trx->maxDuration() = 7;
    CPPUNIT_ASSERT(ShoppingAltDateUtil::validateAltDatePair(*_trx, valid1));

    DatePair valid2(DateTime(2014, 4, 20), DateTime(2014, 4, 23));
    _trx->minDuration() = 3;
    _trx->maxDuration() = 3;
    CPPUNIT_ASSERT(ShoppingAltDateUtil::validateAltDatePair(*_trx, valid2));

    DatePair invalid1(DateTime(2014, 4, 25), DateTime(2014, 5, 1));
    _trx->minDuration() = 0;
    _trx->maxDuration() = 5;
    CPPUNIT_ASSERT(!ShoppingAltDateUtil::validateAltDatePair(*_trx, invalid1));

    DatePair invalid2(DateTime(2014, 4, 20), DateTime(2014, 4, 22));
    _trx->minDuration() = 3;
    _trx->maxDuration() = 8;
    CPPUNIT_ASSERT(!ShoppingAltDateUtil::validateAltDatePair(*_trx, invalid2));
  }

  void testCleanUpProcessedCategoriesForFareByRule()
  {
    TestMemHandle memH;
    memH.create<MyDataHandle>();
    std::vector<uint16_t> categorySequence = {RuleConst::DAY_TIME_RULE, RuleConst::DAY_TIME_RULE, RuleConst::SEASONAL_RULE,
                                              RuleConst::ADVANCE_RESERVATION_RULE, RuleConst::MINIMUM_STAY_RULE, RuleConst::MAXIMUM_STAY_RULE,
                                              RuleConst::BLACKOUTS_RULE, RuleConst::TRAVEL_RESTRICTIONS_RULE, RuleConst::SALE_RESTRICTIONS_RULE};

    PaxTypeFareMock paxTypeFare;
    PaxTypeFareMock baseFare;
    FBRPaxTypeFareRuleData rd;
    FareByRuleItemInfo ruleInfo;
    rd.ruleItemInfo() = &ruleInfo;
    rd.baseFare() = &baseFare;
    paxTypeFare.setRuleData(RuleConst::FARE_BY_RULE, _trx->dataHandle(), &rd, true);
    Itin itin;
    AirSeg airSeg;
    itin.travelSeg().push_back(&airSeg);
    baseFare.fareMarket()->travelSeg().push_back(&airSeg);
    paxTypeFare.fareMarket()->travelSeg().push_back(&airSeg);

    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule, true);

    for (uint16_t category : categorySequence)
    {
      CPPUNIT_ASSERT(!paxTypeFare.isCategoryProcessed(category));
      CPPUNIT_ASSERT(!paxTypeFare.getBaseFare(RuleConst::FARE_BY_RULE)->isCategoryProcessed(category));
    }

    FareMarketRuleController fareMarketRuleController;
    fareMarketRuleController.categorySequence() = categorySequence;

    fareMarketRuleController.validate(*_trx, itin, baseFare);
    fareMarketRuleController.validate(*_trx, itin, paxTypeFare);

    for (uint16_t category : categorySequence)
    {
      CPPUNIT_ASSERT(paxTypeFare.isCategoryProcessed(category));
      CPPUNIT_ASSERT(paxTypeFare.getBaseFare(RuleConst::FARE_BY_RULE)->isCategoryProcessed(category));
    }

    ShoppingAltDateUtil::cleanUpCategoryProcess(&paxTypeFare);

    for (uint16_t category : categorySequence)
    {
      CPPUNIT_ASSERT(!paxTypeFare.isCategoryProcessed(category));
      CPPUNIT_ASSERT(!paxTypeFare.getBaseFare(RuleConst::FARE_BY_RULE)->isCategoryProcessed(category));
    }

    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule, false);

    fareMarketRuleController.validate(*_trx, itin, baseFare);
    fareMarketRuleController.validate(*_trx, itin, paxTypeFare);

    for (uint16_t category : categorySequence)
    {
      CPPUNIT_ASSERT(paxTypeFare.isCategoryProcessed(category));
      CPPUNIT_ASSERT(paxTypeFare.getBaseFare(RuleConst::FARE_BY_RULE)->isCategoryProcessed(category));
    }

    ShoppingAltDateUtil::cleanUpCategoryProcess(&paxTypeFare);

    for (uint16_t category : categorySequence)
    {
      CPPUNIT_ASSERT(!paxTypeFare.isCategoryProcessed(category));
      CPPUNIT_ASSERT(paxTypeFare.getBaseFare(RuleConst::FARE_BY_RULE)->isCategoryProcessed(category));
    }
  }

protected:
  AirSeg* buildSegment(DateTime departure)
  {
    AirSeg* airSeg;
    _memHandle.get(airSeg);
    airSeg->departureDT() = departure;
    return airSeg;
  }

  void setDepartureDates(ShoppingTrx* shoppingTrx)
  {
    AirSeg* airSeg1 = buildSegment(DateTime(2008, 5, 21));
    AirSeg* airSeg2 = buildSegment(DateTime(2008, 5, 16));
    AirSeg* airSeg3 = buildSegment(DateTime(2008, 5, 18));
    AirSeg* airSeg4 = buildSegment(DateTime(2008, 5, 20));
    AirSeg* airSeg5 = buildSegment(DateTime(2008, 5, 12));
    AirSeg* airSeg6 = buildSegment(DateTime(2008, 5, 24));
    AirSeg* airSeg7 = buildSegment(DateTime(2008, 5, 17));
    AirSeg* airSeg8 = buildSegment(DateTime(2008, 5, 28));
    Itin* itineraryOb1;
    Itin* itineraryOb2;
    Itin* itineraryOb3;
    Itin* itineraryOb4;
    Itin* itineraryOb5;
    Itin* itineraryOb6;
    Itin* itineraryOb7;
    Itin* itineraryOb8;
    _memHandle.get(itineraryOb1);
    _memHandle.get(itineraryOb2);
    _memHandle.get(itineraryOb3);
    _memHandle.get(itineraryOb4);
    _memHandle.get(itineraryOb5);
    _memHandle.get(itineraryOb6);
    _memHandle.get(itineraryOb7);
    _memHandle.get(itineraryOb8);
    itineraryOb1->travelSeg().push_back(airSeg1);
    itineraryOb2->travelSeg().push_back(airSeg2);
    itineraryOb3->travelSeg().push_back(airSeg3);
    itineraryOb4->travelSeg().push_back(airSeg4);
    itineraryOb5->travelSeg().push_back(airSeg5);
    itineraryOb6->travelSeg().push_back(airSeg6);
    itineraryOb7->travelSeg().push_back(airSeg7);
    itineraryOb8->travelSeg().push_back(airSeg8);
    shoppingTrx->legs().push_back(ShoppingTrx::Leg());
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb1, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb2, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb3, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb5, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb6, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb7, 1, true));
    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb8, 1, true));
  }

  void setTransactionData()
  {
    AirSeg* airSeg1 = buildSegment(DateTime(2008, 5, 15));
    AirSeg* airSeg2 = buildSegment(DateTime(2008, 5, 16));
    AirSeg* airSeg3 = buildSegment(DateTime(2008, 5, 17));
    AirSeg* airSeg4 = buildSegment(DateTime(2008, 5, 18));
    AirSeg* airSeg5 = buildSegment(DateTime(2008, 5, 22));
    AirSeg* airSeg6 = buildSegment(DateTime(2008, 5, 23));
    AirSeg* airSeg7 = buildSegment(DateTime(2008, 5, 24));
    AirSeg* airSeg8 = buildSegment(DateTime(2008, 5, 25));
    Itin* itineraryOb1(0);
    Itin* itineraryOb2(0);
    Itin* itineraryOb3(0);
    Itin* itineraryOb4(0);
    Itin* itineraryOb5(0);
    Itin* itineraryOb6(0);
    Itin* itineraryOb7(0);
    Itin* itineraryOb8(0);
    _memHandle.get(itineraryOb1);
    _memHandle.get(itineraryOb2);
    _memHandle.get(itineraryOb3);
    _memHandle.get(itineraryOb4);
    _memHandle.get(itineraryOb5);
    _memHandle.get(itineraryOb6);
    _memHandle.get(itineraryOb7);
    _memHandle.get(itineraryOb8);

    itineraryOb1->travelSeg().push_back(airSeg1);
    itineraryOb2->travelSeg().push_back(airSeg2);
    itineraryOb3->travelSeg().push_back(airSeg3);
    itineraryOb4->travelSeg().push_back(airSeg4);
    itineraryOb5->travelSeg().push_back(airSeg5);
    itineraryOb6->travelSeg().push_back(airSeg6);
    itineraryOb7->travelSeg().push_back(airSeg7);
    itineraryOb8->travelSeg().push_back(airSeg8);

    _trx->legs().clear();

    _trx->legs().push_back(ShoppingTrx::Leg());
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb1, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb2, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb3, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb4, 1, true));

    _trx->legs().push_back(ShoppingTrx::Leg());
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb5, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb6, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb7, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb8, 1, true));

    DatePair dp1(airSeg1->departureDT(), airSeg5->departureDT());
    DatePair dp2(airSeg1->departureDT(), airSeg6->departureDT());
    DatePair dp3(airSeg1->departureDT(), airSeg7->departureDT());
    DatePair dp4(airSeg1->departureDT(), airSeg8->departureDT());

    addDurationAltDatePairs(dp1);
    addDurationAltDatePairs(dp2);
    addDurationAltDatePairs(dp3);
    addDurationAltDatePairs(dp4);
  }

  void addDurationAltDatePairs(DatePair& dp)
  {
    std::pair<DatePair, PricingTrx::AltDateInfo*> altDatePairElem(dp, 0);
    uint64_t duration = dp.second.get64BitRepDateOnly() - dp.first.get64BitRepDateOnly();
    PricingTrx::DurationAltDatePairs::iterator foundElemIt =
        _trx->durationAltDatePairs().find(duration);

    if (foundElemIt != _trx->durationAltDatePairs().end())
    {
      foundElemIt->second.insert(altDatePairElem);
    }
    else
    {
      PricingTrx::AltDatePairs datePairsMap;
      datePairsMap.insert(altDatePairElem);
      _trx->durationAltDatePairs()[duration] = datePairsMap;
    }
  }

private:
  ShoppingTrx* createTrx(std::string filename)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(filename);
    ShoppingTrx::Leg* pLeg1 = 0;
    ShoppingTrx::Leg* pLeg2 = 0;
    _memHandle.get(pLeg1);
    _memHandle.get(pLeg2);
    trx->legs().clear();
    trx->legs().push_back(*pLeg1);
    trx->legs().push_back(*pLeg2);
    PaxType* paxType = 0;
    _memHandle.get(paxType);
    trx->paxType().push_back(paxType);
    pLeg1->sop().reserve(10);
    pLeg2->sop().reserve(10);
    trx->flightMatrix().clear();

    return trx;
  }

  ShoppingTrx* _trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingAltDateUtilTest);
}
