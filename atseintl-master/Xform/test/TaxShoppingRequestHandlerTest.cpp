#include "test/include/CppUnitHelperMacros.h"
#include "Common/TaxShoppingConfig.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/TaxShoppingRequestHandler.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/TaxShoppingSchemaNames.h"

namespace tse
{
using namespace taxshopping;

class TaxShoppingConfigMock : public TaxShoppingConfig
{
  public:
    TaxShoppingConfigMock() {}
};

class TaxShoppingRequestHandlerFake : public TaxShoppingRequestHandler
{
public:
  TaxShoppingRequestHandlerFake(Trx*& trx, const TaxShoppingConfigMock& taxCfg)
    : TaxShoppingRequestHandler(trx, taxCfg, false)
  {}

  using TaxShoppingRequestHandler::getNationTransitTimes;
  using TaxShoppingRequestHandler::getTransitHoursChar;
  using TaxShoppingRequestHandler::getSameDayRuleChar;
  using TaxShoppingRequestHandler::isNextStopoverRestr;
  using TaxShoppingRequestHandler::getFirstTvlDateApplicableTaxes;
  using TaxShoppingRequestHandler::getFirstTvlDateRangeString;
  using TaxShoppingRequestHandler::clearNationsFirstTvlDates;
  using TaxShoppingRequestHandler::generateIntlKey;

  void collectNationTransitTimes(NationTransitTimes& transitTime,
                                 const NationCode& nation,
                                 const DateTime& tktDate) const
  {
    transitTime._transitHours.insert(_hours.begin(), _hours.end());
    transitTime._transitTotalMinutes.insert(_minutes.begin(), _minutes.end());
  }

  void addHours(long h)
  {
    _hours.insert(Hours(h));
    _minutes.insert(Minutes(h * 60));
  }
  void clearHours()
  {
    _hours.clear();
    _minutes.clear();
  }

  const FarePath* const createFarePath()
  {
    return &TaxShoppingRequestHandler::createFarePath(getCurrentFarePathInfo());
  }

  const std::string getCurrentFPKey()
  {
    std::string key;
    createFarePathKey(getCurrentFarePathInfo(), key);
    return key;
  }

  const TaxTrx* getTaxTrx() const { return _taxTrx; }

  void setGroupItins(bool val) { _taxTrx->setGroupItins(val); }
  void setApplyUSCAgrouping(bool val) { _applyUSCAgrouping = val; }

  void createFakeTransaction(TestMemHandle& memHandle)
  {
    _trx = _taxTrx = memHandle.create<TaxTrx>();

    _taxTrx->requestType() = OTA_REQUEST;
    _taxTrx->otaXmlVersion() = "2.0.0";
    _taxTrx->setShoppingPath(true);
    _taxTrx->otaRequestRootElementType() = "TRQ";

    TaxRequest* request = 0;
    _taxTrx->dataHandle().get(request);
    request->ticketingDT() = DateTime::localTime();
    _taxTrx->setRequest(request);

    PricingOptions* options = 0;
    _taxTrx->dataHandle().get(options);
    _taxTrx->setOptions(options);

    Agent* agent = _taxTrx->dataHandle().create<Agent>();
    _taxTrx->getRequest()->ticketingAgent() = agent;

    Billing* billing = _taxTrx->dataHandle().create<Billing>();
    _taxTrx->billing() = billing;
  }

  void addTravelSegmentToItin(TestMemHandle& memHandle)
  {
    TravelSeg* seg1 = memHandle.create<AirSeg>();
    _currentItin->travelSeg().push_back(seg1);
  }

  void createItinAndFarePathInfo(TestMemHandle& memHandle)

  {
    _currentItin = memHandle.create<Itin>();

    PaxType* paxType = memHandle.create<PaxType>();
    paxType->paxType() = "ADT";
    paxType->number() = 1;
    _currentFarePathInfos.emplace_back(FarePathInfo{*paxType, 1, 42.11});
  }

  void handleTax(const std::string& taxCode, const std::string& taxAmount,
                 const std::string& segStartId = {},
                 const std::string& segEndId = {})
  {
    addAttribute(_TRQ_BC0, taxCode);
    addAttribute(_TRQ_C6B, taxAmount);
    if (!segStartId.empty())
      addAttribute(_TRQ_C1S, segStartId);
    else
      removeAttribute(_TRQ_C1S);

    if (!segEndId.empty())
      addAttribute(_TRQ_C1E, segEndId);
    else
      removeAttribute(_TRQ_C1E);

    IAttributes attrs(_NoOfAttributeNames_, _attrValueArray);

    TaxShoppingRequestHandler::onStartTAX(attrs);
  }

  void parsePRO()
  {
    IAttributes attrs(_NoOfAttributeNames_, _attrValueArray);
    TaxShoppingRequestHandler::onStartPRO(attrs);
  }

  void callOnEndPXI()
  {
    TaxShoppingRequestHandler::onEndPXI();
  }

  void addAttribute(uint32_t attrId, const std::string& value)
  {
    _attrValueArray[attrId] = IValueString(value.c_str(), value.length());
  }

  void addNationWithStopover(const NationCode nation, const bool stopover)
  {
    _nationWithNextStopoverRestr.emplace(nation, stopover);
  }
  void addNationTaxFirstTravelDate(const NationCode nation, const boost::gregorian::date date)
  {
    _nationTaxFirstTravelDatesMap.emplace(nation, TaxFirstTravelDates()).first->second.emplace(date);
  }
  void removeAttribute(uint32_t attrId)
  {
    _attrValueArray[attrId] = IValueString{};
  }

  void addTravelSegment(TravelSeg* seg)
  {
    _currentItin->travelSeg().push_back(seg);
  }

  void clearSegments()
  {
    _currentItin->travelSeg().clear();
  }

private:
  std::set<Hours> _hours;
  std::set<Minutes> _minutes;
  IValueString _attrValueArray[_NoOfAttributeNames_];
};

class TaxShoppingRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxShoppingRequestHandlerTest);

  CPPUNIT_TEST(testEmptyNationTransitHoursMap);
  CPPUNIT_TEST(testGetTransitHoursCharEmpty);
  CPPUNIT_TEST(testGetTransitHoursChar);
  CPPUNIT_TEST(getSameDayRuleChar);
  CPPUNIT_TEST(onStartTAXTest);
  CPPUNIT_TEST(getNextStopoverRestrTest);
  CPPUNIT_TEST(getFirstTvlDateApplicableTaxesTest);
  CPPUNIT_TEST(getFirstTvlDateRangeStringTest);
  CPPUNIT_TEST(equipmentTypeExceptionTest);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  TaxShoppingConfigMock _taxShoppingConfig;
public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown() { _memHandle.clear(); }
  void testEmptyNationTransitHoursMap()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    CPPUNIT_ASSERT(handler._nationTransitTimes.empty());
    handler.getNationTransitTimes(NationCode("DE"), DateTime());
    CPPUNIT_ASSERT_EQUAL(handler._nationTransitTimes.size(), static_cast<size_t>(1));
  }

  void testGetTransitHoursCharEmpty()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    DateTime arrDate = DateTime();
    const char h =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(1), DateTime());
    CPPUNIT_ASSERT_EQUAL(h, 'a');
  }

  void testGetTransitHoursChar()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.addHours(4);
    handler.addHours(12);

    DateTime arrDate = DateTime();
    const char hA =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(1), DateTime());
    CPPUNIT_ASSERT_EQUAL(hA, 'a');
    const char hA2 =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(3), DateTime());
    CPPUNIT_ASSERT_EQUAL(hA2, 'a');
    const char hA3 =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(4), DateTime());
    CPPUNIT_ASSERT_EQUAL(hA3, 'a');
    const char hB =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(5), DateTime());
    CPPUNIT_ASSERT_EQUAL(hB, 'b');
    const char hC =
        handler.getTransitHoursChar(NationCode("DE"), arrDate, arrDate + Hours(13), DateTime());
    CPPUNIT_ASSERT_EQUAL(hC, 'c');
  }

  void getSameDayRuleChar()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);

    DateTime d1 = DateTime::localTime();
    std::vector<NationCode> nations;

    const char c1 = handler.getSameDayRuleChar(d1, d1, nations, NationCode("DE"));
    CPPUNIT_ASSERT_EQUAL('0', c1);

    DateTime d2 = d1 + Hours(48);
    const char c2 = handler.getSameDayRuleChar(d1, d2, nations, NationCode("DE"));
    CPPUNIT_ASSERT_EQUAL('0', c2);

    nations.push_back(NationCode("AR"));

    const char c3 = handler.getSameDayRuleChar(d1, d2, nations, NationCode("DE"));
    CPPUNIT_ASSERT_EQUAL('0', c3);

    nations.push_back(NationCode("DE"));

    const char c4 = handler.getSameDayRuleChar(d1, d2, nations, NationCode("DE"));
    CPPUNIT_ASSERT_EQUAL('1', c4);
  }

  void onStartTAXTest()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    handler.createItinAndFarePathInfo(_memHandle);
    AirSeg tvl1, tvl2, tvl3;
    handler.addTravelSegment(&tvl1);
    handler.addTravelSegment(&tvl2);
    handler.addTravelSegment(&tvl3);
    handler.setGroupItins(true);
    handler.setApplyUSCAgrouping(false);

    const FarePath* fp = handler.createFarePath();
    CPPUNIT_ASSERT_MESSAGE("FarePath initialization", fp);
    TaxCode yqi("YQI");
    TaxCode yqf("YQF");

    handler.handleTax(yqi, "0.00");
    fp = handler.createFarePath();
    CPPUNIT_ASSERT_EQUAL(fp->getExternalTaxes().size(), static_cast<size_t>(0));

    handler.handleTax(yqi, "1.00");
    fp = handler.createFarePath();
    CPPUNIT_ASSERT_EQUAL(fp->getExternalTaxes().size(), static_cast<size_t>(1));
    const TaxItem* item = fp->getExternalTaxes().back();
    CPPUNIT_ASSERT_EQUAL(item->taxCode(), yqi);
    CPPUNIT_ASSERT_EQUAL(item->taxAmount(), 1.00);

    handler.handleTax(yqf, "2.00", "1", "2");
    fp = handler.createFarePath();
    CPPUNIT_ASSERT_EQUAL(fp->getExternalTaxes().size(), static_cast<size_t>(2));
    item = fp->getExternalTaxes().back();
    CPPUNIT_ASSERT_EQUAL(item->taxCode(), yqf);
    CPPUNIT_ASSERT_EQUAL(item->taxAmount(), 2.00);

    handler.handleTax(yqi, "3.00", "0");
    fp = handler.createFarePath();
    CPPUNIT_ASSERT_EQUAL(fp->getExternalTaxes().size(), static_cast<size_t>(3));
    item = fp->getExternalTaxes().back();
    CPPUNIT_ASSERT_EQUAL(item->taxCode(), yqi);
    CPPUNIT_ASSERT_EQUAL(item->taxAmount(), 3.00);

    handler.callOnEndPXI();

    const std::string expectedFPKey("|ADT1|42.11|YQI1;1-2YQF2;0YQI3;");
    CPPUNIT_ASSERT_EQUAL(expectedFPKey, handler.getCurrentFPKey());
  }

  void onStartPROTest()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    const TaxTrx* taxTrx = handler.getTaxTrx();

    const std::string dateStr("2014-06-30");
    const std::string minutes("30");
    handler.addAttribute(_TRQ_D07, dateStr);
    handler.addAttribute(_TRQ_D54, minutes);

    handler.addAttribute(_TRQ_P0J, "Y");

    handler.addAttribute(_TRQ_PFC, "F");
    handler.addAttribute(_TRQ_AF0, "LON");//POS
    handler.addAttribute(_TRQ_AG0, "ROM");//POT

    handler.addAttribute(_TRQ_C48, "USD");

    // Check default values
    CPPUNIT_ASSERT_MESSAGE("ETicket default", !taxTrx->getRequest()->isElectronicTicket());
    CPPUNIT_ASSERT_MESSAGE("PFC default", taxTrx->getOptions()->getCalcPfc());
    CPPUNIT_ASSERT_MESSAGE("POT override default", taxTrx->getRequest()->ticketPointOverride().empty());
    CPPUNIT_ASSERT_MESSAGE("POS override default", taxTrx->getRequest()->salePointOverride().empty());
    CPPUNIT_ASSERT_MESSAGE("Currency override default", !taxTrx->getOptions()->isMOverride());
    CPPUNIT_ASSERT_MESSAGE("Currency override code default", taxTrx->getOptions()->currencyOverride().empty());

    // parse request PRO
    handler.parsePRO();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tkt date", taxTrx->getRequest()->ticketingDT(), DateTime(dateStr, 30));
    CPPUNIT_ASSERT_MESSAGE("ETicket", taxTrx->getRequest()->isElectronicTicket());
    CPPUNIT_ASSERT_MESSAGE("PFC", !taxTrx->getOptions()->getCalcPfc());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("POT override", taxTrx->getRequest()->ticketPointOverride(), LocCode("ROM"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("POS override", taxTrx->getRequest()->salePointOverride(), LocCode("LON"));
    CPPUNIT_ASSERT_MESSAGE("Currency override", taxTrx->getOptions()->isMOverride());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Currency override code", taxTrx->getOptions()->currencyOverride(), CurrencyCode("USD"));
  }

  void getNextStopoverRestrTest()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    std::vector<TravelSeg*> segments;
    AirSeg* seg = _memHandle.create<AirSeg>();
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = NationCode("BD");
    seg->origin() = loc;
    seg->destination() = loc;
    segments.push_back(seg);
    handler.addNationWithStopover(NationCode("BD"), true);

    CPPUNIT_ASSERT_MESSAGE("isNextStopoverRestr - pass", handler.isNextStopoverRestr(segments));

    loc->nation() = NationCode("PL");
    handler.addNationWithStopover(NationCode("PL"), false);

    CPPUNIT_ASSERT_MESSAGE("isNextStopoverRestr - fail", !handler.isNextStopoverRestr(segments));
  }

  void getFirstTvlDateApplicableTaxesTest()
  {
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    std::vector<TravelSeg*> segments;
    DateTime date1("2015-11-01", 0);
    DateTime date2("2015-11-10", 0);
    DateTime date3("2015-11-15", 0);
    AirSeg* seg1 = _memHandle.create<AirSeg>();
    AirSeg* seg2 = _memHandle.create<AirSeg>();
    Loc* loc1 = _memHandle.create<Loc>();
    Loc* loc2 = _memHandle.create<Loc>();
    loc1->nation() = NationCode("BD");
    loc2->nation() = NationCode("PL");
    seg1->origin() = loc1;
    seg1->destination() = loc2;
    seg1->departureDT() = date1;
    seg1->arrivalDT() = date2;
    seg2->origin() = loc2;
    seg2->destination() = loc1;
    seg2->departureDT() = date2;
    seg2->arrivalDT() = date3;
    segments.push_back(seg1);
    segments.push_back(seg2);
    handler.addNationTaxFirstTravelDate(NationCode("BD"), DateTime("2015-11-01", 0).date());
    handler.addNationTaxFirstTravelDate(NationCode("BD"), DateTime("2015-11-09", 0).date());
    handler.addNationTaxFirstTravelDate(NationCode("PL"), DateTime("2015-11-02", 0).date());
    handler.addNationTaxFirstTravelDate(NationCode("AU"), DateTime("2015-11-02", 0).date());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("nationTaxFirstTravelDate", handler.getFirstTvlDateApplicableTaxes(NationCode("BD"), segments, DateTime()), 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("nationTaxFirstTravelDate", handler.getFirstTvlDateApplicableTaxes(NationCode("PL"), segments, DateTime()), 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("nationTaxFirstTravelDate", handler.getFirstTvlDateApplicableTaxes(NationCode("DE"), segments, DateTime()), 0);
  }

  void getFirstTvlDateRangeStringTest()
  {

    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    std::vector<TravelSeg*> segments;
    std::vector<Loc*> locations;
    const NationCode N0("GB");
    const NationCode N1("DE");
    const NationCode N2("PL");
    const NationCode N3("AU");
    const NationCode N4("BD");
    const NationCode N5("AR");

    DateTime tvlDate("2016-03-02", 0);
    const size_t segmentsNo = 6u;
    for(size_t i = 0; i < segmentsNo; ++i)
    {
      AirSeg* seg = _memHandle.create<AirSeg>();
      Loc* loc = _memHandle.create<Loc>();
      loc->nation() = N0;
      locations.push_back(loc);
      seg->origin() = loc;
      seg->departureDT() = tvlDate.addDays(i * 5);
      seg->arrivalDT() = tvlDate.addDays(i * 5);
      segments.push_back(seg);
      if (i > 0)
        segments[i-1]->destination() = loc;
    }

    segments.back()->destination() = locations.back();
    // segments tvl dates: 2016-03-02, 2016-03-07, 2016-03-12, 2016-03-17, 2016-03-22, 2016-03-27

    CPPUNIT_ASSERT_MESSAGE("getFirstTvlDateRangeString - empty tax tvl dates", handler.getFirstTvlDateRangeString(segments, DateTime()).empty());

    handler.addNationTaxFirstTravelDate(N1, DateTime("2016-03-05", 0).date());
    handler.addNationTaxFirstTravelDate(N1, DateTime("2016-03-15", 0).date());
    handler.addNationTaxFirstTravelDate(N2, DateTime("2016-03-10", 0).date());
    handler.addNationTaxFirstTravelDate(N3, DateTime("2016-03-20", 0).date());
    handler.addNationTaxFirstTravelDate(N4, DateTime("2016-03-25", 0).date());

    // all segments within nation N1 - dates range {2016-03-05, 2016-03-15}
    for(Loc* loc : locations)
      loc->nation() = N1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - N1", std::string("abb"), handler.getFirstTvlDateRangeString(segments, DateTime()));

    // all segments within nation N2 - dates range {2016-03-10}
    for(Loc* loc : locations)
      loc->nation() = N2;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - N2", std::string("aa"), handler.getFirstTvlDateRangeString(segments, DateTime()));

    // nations N2, N2 - dates range {2016-03-10, 2016-03-20}
    locations[4]->nation() = N3;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - N2, N3", std::string("aabb"), handler.getFirstTvlDateRangeString(segments, DateTime()));

    // nations N2, N2 - dates range {2016-03-10, 2016-03-20, 2016-03-25}
    locations[5]->nation() = N4;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - N2, N3, N4", std::string("aabbc"), handler.getFirstTvlDateRangeString(segments, DateTime()));

    // all nations - dates range {2016-03-05, 2016-03-10, 2016-03-15, 2016-03-20, 2016-03-25}
    locations[0]->nation() = N1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - all nations", std::string("abcde"), handler.getFirstTvlDateRangeString(segments, DateTime()));

    // no date ranges for any nation
    for (Loc* const loc : locations)
      loc->nation() = N5;
    CPPUNIT_ASSERT_MESSAGE("getFirstTvlDateRangeString - no date ranges", handler.getFirstTvlDateRangeString(segments, DateTime()).empty());

    // all segments are within the same dates range
    handler.addNationTaxFirstTravelDate(N5, DateTime("2016-03-01", 0).date());
    CPPUNIT_ASSERT_MESSAGE("getFirstTvlDateRangeString - N5", handler.getFirstTvlDateRangeString(segments, DateTime()).empty());
    handler.addNationTaxFirstTravelDate(N5, DateTime("2016-03-30", 0).date());
    handler.clearNationsFirstTvlDates();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getFirstTvlDateRangeString - N5 (v2)", std::string("bbbbbb"), handler.getFirstTvlDateRangeString(segments, DateTime()));
  }

  void equipmentTypeExceptionTest()
  {
    std::string key;
    Trx* trx(0);
    TaxShoppingRequestHandlerFake handler(trx, _taxShoppingConfig);
    handler.createFakeTransaction(_memHandle);
    handler.createItinAndFarePathInfo(_memHandle);
    handler.setGroupItins(true);
    handler.setApplyUSCAgrouping(false);

    Loc loc1, loc2, loc3, loc4;
    loc1.nation() = loc2.nation() = loc3.nation() = loc4.nation() = "GB";
    AirSeg tvl1, tvl2, tvl3;
    tvl1.origin() = &loc1;
    tvl1.destination() = &loc2;
    tvl2.origin() = &loc2;
    tvl2.destination() = &loc3;
    tvl3.origin() = &loc3;
    tvl3.destination() = &loc4;

    handler.addTravelSegment(&tvl1);
    handler.addTravelSegment(&tvl2);
    handler.addTravelSegment(&tvl3);

    // Grouping key for this travel route: "-0--0--0-|aa|N|N|00|0|"
    // Char generated for special equipment type for GB taxes is at index 15
    const std::string groupingKeyValue("-0--0--0-|aa|N|N|00|0||ADT1|42.11");
    handler.generateIntlKey(key);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Grouping key check", groupingKeyValue, key);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Equipment type char", 'N', key[15]);

    for (const auto& type : std::vector<std::string>{"TRN","TGV","ICE"})
    {
      key.clear();
      tvl1.equipmentType() = type;
      handler.generateIntlKey(key);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Equipment type char - " + type + " on segment 1", 'Y', key[15]);
    }

    tvl1.equipmentType() = "";
    tvl2.equipmentType() = "TGV";
    key.clear();
    handler.generateIntlKey(key);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Equipment type char - TGV type on segment 2", 'Y', key[15]);

    tvl3.equipmentType() = "TGV";
    key.clear();
    handler.generateIntlKey(key);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Equipment type char - TGV type on segment 3", 'Y', key[15]);

    loc1.nation() = loc2.nation() = loc3.nation() = loc4.nation() = "DE";
    key.clear();
    handler.generateIntlKey(key);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Equipment type char - nation different than GB", 'N', key[15]);

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxShoppingRequestHandlerTest);
}
// namespace
