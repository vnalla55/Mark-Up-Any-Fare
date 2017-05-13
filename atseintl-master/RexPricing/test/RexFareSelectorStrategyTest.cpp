#include <boost/assign/std/vector.hpp>

#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "RexPricing/RexFareSelectorStrategy.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/MockPaxTypeFareWrapper.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class RexFareSelectorStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorStrategyTest);

  CPPUNIT_TEST(testSelectByFareBasis);
  CPPUNIT_TEST(testSelectByFareBasis_WithTktDesignator);
  CPPUNIT_TEST(testSelectByFareBasis_WithTktDesignatorAndTrimTo13Char);
  CPPUNIT_TEST(testSelectByFareBasis_LongFareBasis);
  CPPUNIT_TEST(testSelectByFareBasis_LongFareBasisWithTktDesignator);

  CPPUNIT_TEST(testPreSelect_ByValidCategory);
  CPPUNIT_TEST(testPreSelect_FareMarketDirectionality_Inbound);
  CPPUNIT_TEST(testPreSelect_FareMarketDirectionality_Outbound);
  CPPUNIT_TEST(testPreSelect_FareMarketDirectionality_Unknown);
  CPPUNIT_TEST(testPreSelect_SkipCAT35RemoveCurrency);
  CPPUNIT_TEST(testPreSelect_FareCalcCurrencyPreference_Pass);
  CPPUNIT_TEST(testPreSelect_FareCalcCurrencyPreference_Fail);
  CPPUNIT_TEST(testPreSelect_FareCalcCurrencyPreferenceNUC_Fail);

  CPPUNIT_TEST(testForcedSideTrip_Fail);
  CPPUNIT_TEST(testForcedSideTrip_Pass);

  CPPUNIT_TEST(testSelectByDirectionality_Inbound);
  CPPUNIT_TEST(testSelectByDirectionality_Outbound);
  CPPUNIT_TEST(testSelectByDirectionality_Unknown);

  CPPUNIT_TEST(testRuningMockSelector);

  CPPUNIT_TEST(testSelection_empty);
  CPPUNIT_TEST(testSelection_noMatch);
  CPPUNIT_TEST(testSelection_NoIndAtpSec);
  CPPUNIT_TEST(testSelection_NoIndAtpNoSec);
  CPPUNIT_TEST(testSelection_NoIndNoAtpSec);
  CPPUNIT_TEST(testSelection_NoIndNoAtpNoSec);
  CPPUNIT_TEST(testSelection_IndAtpSec);
  CPPUNIT_TEST(testSelection_IndAtpNoSec);
  CPPUNIT_TEST(testSelection_IndNoAtpSec);
  CPPUNIT_TEST(testSelection_IndNoAtpNoSec);

  CPPUNIT_TEST(testUpdateAmount_Published);
  CPPUNIT_TEST(testUpdateAmount_PublishedNegotiated);

  CPPUNIT_TEST_SUITE_END();

private:
  typedef RexFareSelectorStrategy::Iterator Iterator;
  static const VendorCode SITA, ATP;
  static const MoneyAmount NOT_AMOUNT;

  RexPricingTrx* _trx;
  ExcItin* _excItin;
  PaxType _pt;
  FareMarket* _fm;
  FareCompInfo* _fc;

  TestMemHandle _memH;

  RexFareSelectorStrategy* _selector;

public:
  enum
  {
    cat15on = 1,
    cat15off = 0,
    cat35on = 1,
    cat35off = 0,
    cat25on = 1,
    cat25off = 0,
    industryOn = 1,
    industryOff = 0
  };

  void setUp()
  {
    _memH.create<RootLoggerGetOff>();
    _memH.create<TestConfigInitializer>();

    _pt.paxType() = "ADT";
    _excItin = _memH.create<ExcItin>();
    _excItin->calculationCurrency() = USD;
    //y_excItin->calcCurrencyOverride() = USD;

    _trx = _memH.create<RexPricingTrx>();
    _trx->setRequest(_memH.create<RexPricingRequest>());
    _trx->setOptions(_memH.create<RexPricingOptions>());

    _trx->exchangeItin().push_back(_excItin);
    _trx->fareCalcConfig() = _memH.create<FareCalcConfig>();
    _trx->fareCalcConfig()->fareBasisTktDesLng() = ' ';
    _trx->exchangePaxType() = &_pt;
    _trx->paxType().push_back(&_pt);

    PaxTypeBucket ptc;
    ptc.requestedPaxType() = &_pt;

    _fm = _memH.create<FareMarket>();
    _fm->direction() = FMDirection::INBOUND;
    _fm->geoTravelType() = GeoTravelType::International;
    _fm->paxTypeCortege().push_back(ptc);
    _fm->travelSeg() += _memH.create<AirSeg>();

    _fc = _memH.create<FareCompInfo>();
    _fc->fareMarket() = _fm;
    _fc->fareBasisCode() = "CX";

    _selector = _memH.create<MockRexFareSelectorStrategy>((*_trx));
  }

  void tearDown() { _memH.clear(); }

  std::vector<PaxTypeFare*>& ptf() { return _fm->paxTypeCortege(&_pt)->paxTypeFare(); }

  void assertPreSelect(const std::vector<PaxTypeFareWrapper>& expect)
  {
    std::vector<PaxTypeFareWrapper> selected;
    _selector->preSelect(*_fc, selected);

    CPPUNIT_ASSERT_EQUAL(expect, selected);
  }

  void testSelectByFareBasis()
  {
    _fc->fareBasisCode() = "AB";

    FareClassCode fcc[] = { "AB", "CC", "AB", "ABCD" };
    populateFaresToFareMarket(fcc);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0], ptf()[2];

    assertPreSelect(expect);
  }

  void testSelectByFareBasis_LongFareBasis()
  {
    _fc->fareBasisCode() = "ABCDEFGHIJKL";

    FareClassCode init[] = { "ABCDEFGHIJKL", "ABCDEFGHIJK", "ABCDEFGHIJKLM",
                             "ABCDEFGH",     "ABCDEFG",     "ABCDEFGHI" };
    populateFaresToFareMarket(init);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0];

    assertPreSelect(expect);
  }

  void testSelectByFareBasis_LongFareBasisWithTktDesignator()
  {
    _fc->fareBasisCode() = "ABCDEFGHIJ/TD1";

    FareClassCode init[] = { "ABCDEFGHIJKL", "ABCDEFGHIJ/TD1", "ABCDEFGHIJ/TD2",
                             "ABCDEFGH/TD1", "ABCDEFGH",       "ABCDEFGHIJ/TD1" };
    populateFaresToFareMarket(init);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[1], ptf()[2], ptf()[5];

    assertPreSelect(expect);
  }

  void testSelectByFareBasis_WithTktDesignator()
  {
    _fc->fareBasisCode() = "CC/DP100";

    FareClassCode fcc[] = { "CX", "CC/DP100", "CX/DP100", "CCX/DP100" };
    populateFaresToFareMarket(fcc);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[1];

    assertPreSelect(expect);
  }

  void testSelectByFareBasis_WithTktDesignatorAndTrimTo13Char()
  {
    _fc->fareBasisCode() = "NLXNCGB8/TVL0";

    FareClassCode fcc[] = { "NLXNCGB8/TVL050", "NLXNCGB8",      "NLXNCGB8/TVL060",
                            "NLXNCGB8/TVL100", "NLXNCGB8/TVL0", "NLXNCGB8/TVL" };
    populateFaresToFareMarket(fcc);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0], ptf()[2], ptf()[4];

    assertPreSelect(expect);
  }

  void testPreSelect_ByValidCategory()
  {
    typedef std::pair<bool, bool> Pair;
    Pair init[] = { Pair(cat15on, cat35off), Pair(cat15on, cat35on), Pair(cat15off, cat35on) };

    populateFaresToFareMarket(init);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[1];

    assertPreSelect(expect);
  }

  void testPreSelect_FareMarketDirectionality_Inbound()
  {
    _fm->direction() = FMDirection::INBOUND;

    populateFaresToFareMarket();

    std::vector<PaxTypeFareWrapper> expect(ptf().begin(), ptf().end());

    assertPreSelect(expect);
  }

  void testPreSelect_FareMarketDirectionality_Outbound()
  {
    _fm->direction() = FMDirection::OUTBOUND;

    populateFaresToFareMarket();

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0], ptf()[1], ptf()[4], ptf()[5];

    assertPreSelect(expect);
  }

  void testPreSelect_FareMarketDirectionality_Unknown()
  {
    _fm->direction() = FMDirection::UNKNOWN;

    populateFaresToFareMarket();

    std::vector<PaxTypeFareWrapper> expect(ptf().begin(), ptf().end());

    assertPreSelect(expect);
  }

  void testPreSelect_SkipCAT35RemoveCurrency()
  {
    _fc->hasVCTR() = true;
    _trx->exchangeItin().front()->calcCurrencyOverride() = "USD";

    typedef std::pair<CurrencyCode, bool> Pair;
    Pair init[] = { Pair("USD", cat35off), Pair("JPY", cat35off), Pair("USD", cat35on) };
    populateFaresToFareMarket(init);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0], ptf()[2];

    assertPreSelect(expect);
    CPPUNIT_ASSERT(ptf()[0]->areAllCategoryValid());
  }

  void testPreSelect_FareCalcCurrencyPreference_Pass()
  {
    _trx->exchangeItin().front()->calcCurrencyOverride() = "USD";

    CurrencyCode cc[] = { "USD", "CAD", "NUC", "PLN", "USD", "JPY" };
    populateFaresToFareMarket(cc);

    std::vector<PaxTypeFareWrapper> expect;
    expect += ptf()[0], ptf()[4];

    assertPreSelect(expect);
  }

  void testPreSelect_FareCalcCurrencyPreference_Fail()
  {
    _trx->exchangeItin().front()->calcCurrencyOverride() = "CHF";

    CurrencyCode cc[] = { "USD", "CAD", "NUC", "PLN", "USD", "JPY" };
    populateFaresToFareMarket(cc);

    std::vector<PaxTypeFareWrapper> expect(ptf().begin(), ptf().end());

    assertPreSelect(expect);
  }

  void testPreSelect_FareCalcCurrencyPreferenceNUC_Fail()
  {
    _trx->exchangeItin().front()->calcCurrencyOverride() = "NUC";

    CurrencyCode cc[] = { "USD", "CAD", "NUC", "PLN", "USD", "JPY" };
    populateFaresToFareMarket(cc);

    std::vector<PaxTypeFareWrapper> expect(ptf().begin(), ptf().end());

    assertPreSelect(expect);
  }

  void testForcedSideTrip_Fail() { CPPUNIT_ASSERT(!_selector->checkForcedSideTrip(*_fm)); }

  void testForcedSideTrip_Pass()
  {
    _fm->travelSeg().front()->forcedSideTrip() = 'T';

    CPPUNIT_ASSERT(_selector->checkForcedSideTrip(*_fm));
  }

  void testSelectByDirectionality_Inbound()
  {
    Directionality dir[] = { FROM, TO, BETWEEN, WITHIN, BOTH, ORIGIN, TERMINATE };
    populateFaresToFareMarket(dir);

    _fm->direction() = FMDirection::INBOUND;

    std::vector<PaxTypeFare*> expect(ptf()),
        result(ptf().begin(), _selector->selectByDirectionality(*_fc, ptf().begin(), ptf().end()));

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelectByDirectionality_Outbound()
  {
    Directionality dir[] = { FROM, TO, BETWEEN, WITHIN, BOTH, ORIGIN, TERMINATE };
    populateFaresToFareMarket(dir);

    _fm->direction() = FMDirection::OUTBOUND;

    std::vector<PaxTypeFare*> expect(ptf()),
        result(ptf().begin(), _selector->selectByDirectionality(*_fc, ptf().begin(), ptf().end()));

    expect.erase(expect.begin() + 1);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelectByDirectionality_Unknown()
  {
    Directionality dir[] = { FROM, TO, BETWEEN, WITHIN, BOTH, ORIGIN, TERMINATE };
    populateFaresToFareMarket(dir);

    _fm->direction() = FMDirection::UNKNOWN;

    std::vector<PaxTypeFare*> expect(ptf()),
        result(ptf().begin(), _selector->selectByDirectionality(*_fc, ptf().begin(), ptf().end()));

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void populateWrapperAmount(std::vector<PaxTypeFareWrapper>& vec)
  {
    for (Iterator i = vec.begin(); i != vec.end(); ++i)
      i->setAmount(i->get()->fareAmount());
  }

  void testRuningMockSelector()
  {
    MoneyAmount amt[] = { 100.0, 200.0, 300.0 };
    populateFaresToFareMarket(amt);

    MockRexFareSelectorStrategy selector(*_trx);
    std::vector<PaxTypeFareWrapper> preSelected(ptf().begin(), ptf().end());

    Iterator begin = preSelected.begin();

    begin = selector.select(*_fc, begin, preSelected.end());
    CPPUNIT_ASSERT(begin != preSelected.end());
    CPPUNIT_ASSERT_EQUAL(ptf()[0], begin->get());

    begin = selector.select(*_fc, ++begin, preSelected.end());
    CPPUNIT_ASSERT(begin != preSelected.end());
    CPPUNIT_ASSERT_EQUAL(ptf()[1], begin->get());

    begin = selector.select(*_fc, ++begin, preSelected.end());
    CPPUNIT_ASSERT(begin != preSelected.end());
    CPPUNIT_ASSERT_EQUAL(ptf()[2], begin->get());
  }

  void testSelection_empty()
  {
    std::vector<PaxTypeFareWrapper> preSelected, result, expect;
    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelection_noMatch()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected, expect;
    populatePreSelected(preSelected);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelection_NoIndAtpSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 2, 18);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[0]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[2]);
  }

  void testSelection_NoIndAtpNoSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 6, 22);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[3]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[5]);
  }

  void testSelection_NoIndNoAtpSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 3, 19);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[6]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[8]);
  }

  void testSelection_NoIndNoAtpNoSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 7, 23);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[9]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[11]);
  }

  void testSelection_IndAtpSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 0, 16);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[12]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[14]);
  }

  void testSelection_IndAtpNoSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 4, 20);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[15]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[17]);
  }

  void testSelection_IndNoAtpSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 1, 17);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[18]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[20]);
  }

  void testSelection_IndNoAtpNoSec()
  {
    std::vector<PaxTypeFareWrapper> result, preSelected,
        expect = populatePreSelected(preSelected, 5, 21);

    _selector->selection(*_fc, preSelected, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(expect[0], preSelected[21]);
    CPPUNIT_ASSERT_EQUAL(expect[1], preSelected[23]);
  }

  typedef std::pair<MoneyAmount, MoneyAmount> MoneyAmountPair;

  template <std::vector<tse::PaxTypeFareWrapper>::size_type size>
  void assertUpdateAmount(MoneyAmountPair (&amt)[size],
                          const std::vector<MoneyAmount>& expect,
                          bool usePublishedCurrency, bool considerNetAmonut)
  {
    populateFaresToFareMarket(amt);
    std::vector<PaxTypeFareWrapper> preSelected(ptf().begin(), ptf().end());

    _selector->updateAmount(preSelected.begin(), preSelected.end(),
                            usePublishedCurrency, considerNetAmonut);

    std::vector<MoneyAmount> result;
    CPPUNIT_ASSERT_EQUAL(size, preSelected.size());
    for (const PaxTypeFareWrapper& wrp : preSelected)
      result.push_back(wrp.getAmount());
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  enum { USE_PUBLISHED_AMOUNT = 1, USE_CONVERTED_AMOUNT = 0 };
  enum { USE_NET_AMOUNT = 1, USE_SELL_AMOUNT = 0 };

  void testUpdateAmount_Published()
  {
    MoneyAmountPair amt[] = { MoneyAmountPair(100.0, 80.0),
        MoneyAmountPair(200.0, NOT_AMOUNT), MoneyAmountPair(300.0, 150.0) };
    std::vector<MoneyAmount> expect;
    expect += 100.0, 200.0, 300.0;
    assertUpdateAmount(amt, expect,  USE_PUBLISHED_AMOUNT, USE_SELL_AMOUNT);
  }

  void testUpdateAmount_PublishedNegotiated()
  {
    MoneyAmountPair amt[] = { MoneyAmountPair(100.0, 80.0),
        MoneyAmountPair(200.0, NOT_AMOUNT), MoneyAmountPair(300.0, 150.0) };
    std::vector<MoneyAmount> expect;
    expect += 80.0, 200.0, 150.0;
    assertUpdateAmount(amt, expect,  USE_PUBLISHED_AMOUNT, USE_NET_AMOUNT);
  }

  struct CreateFare
  {
    CreateFare(TestMemHandle& mh, DataHandle& dh) : _memH(mh), _dataH(dh) {}

    template <typename T>
    PaxTypeFare* operator()(const T& init)
    {
      return create(init);
    }

    template <typename T, typename U>
    PaxTypeFare* operator()(const std::pair<T, U>& init)
    {
      return create(init.first, init.second);
    }

    PaxTypeFare* create(const VendorCode& ven, bool industry, bool secure) const
    {
      return create("CX", ven, TO, secure, secure, industry, "USD");
    }

  private:
    TestMemHandle& _memH;
    DataHandle& _dataH;
    PaxTypeFare* create(const FareClassCode& fcc,
                        const VendorCode& ven,
                        Directionality dir = TO,
                        bool cat15 = true,
                        bool cat35 = true,
                        bool industry = false,
                        const CurrencyCode& curr = "USD",
                        bool cat25 = false,
                        Indicator owrt = ALL_WAYS,
                        const MoneyAmount& amt = 0.0,
                        const MoneyAmount& netAmt = NOT_AMOUNT) const
    {
      PaxTypeFare* ptf = _memH.create<PaxTypeFare>();

      Fare* fare = 0;
      FareInfo* fareInfo = 0;

      ptf->setFare(fare = _memH.create<Fare>());
      fare->setFareInfo(fareInfo = _memH.create<FareInfo>());

      fareInfo->directionality() = dir;
      fareInfo->fareClass() = fcc;
      fareInfo->vendor() = ven;
      fareInfo->currency() = curr;
      fareInfo->owrt() = owrt;
      fareInfo->fareAmount() = amt;

      fare->_status.set(Fare::FS_IndustryFare, industry);

      ptf->setCategoryValid(15, cat15);
      ptf->setCategoryValid(35, cat35);

      if (cat25)
      {
        PaxTypeFareRuleData* rd = _memH.create<FBRPaxTypeFareRuleData>();
        rd->ruleItemInfo() = _memH.create<FareByRuleItemInfo>();

        rd->baseFare() = create(fcc);

        ptf->setRuleData(25, _dataH, rd, true);
        ptf->status().set(PaxTypeFare::PTF_FareByRule);
      }

      if (netAmt > -EPSILON)
      {
        NegPaxTypeFareRuleData* data = _memH.create<NegPaxTypeFareRuleData>();
        data->netAmount() = netAmt;
        PaxTypeFare::PaxTypeFareAllRuleData* allData = _memH.create<PaxTypeFare::PaxTypeFareAllRuleData>();
        allData->fareRuleData = data;
        (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allData;
        ptf->status().set(PaxTypeFare::PTF_Negotiated);
      }
      return ptf;
    }

    PaxTypeFare* create(const FareClassCode& fcc) const
    {
      return create(fcc, ATP, TO, true, true, false, "USD");
    }

    PaxTypeFare* create(const VendorCode& ven) const
    {
      return create("CX", ven, TO, true, true, false, "USD");
    }

    PaxTypeFare* create(Directionality dir) const
    {
      return create("CX", ATP, dir, true, true, false, "USD");
    }

    PaxTypeFare* create(bool cat15, bool cat35) const
    {
      return create("CX", ATP, TO, cat15, cat35, false, "USD");
    }

    PaxTypeFare* create(bool industry) const
    {
      return create("CX", ATP, TO, true, true, industry, "USD");
    }

    PaxTypeFare* create(const CurrencyCode& curr) const
    {
      return create("CX", ATP, TO, true, true, false, curr);
    }

    PaxTypeFare* create(const CurrencyCode& curr, bool cat35) const
    {
      return create("CX", ATP, TO, true, cat35, false, curr);
    }

    PaxTypeFare* create(const VendorCode& ven, bool industry) const
    {
      return create("CX", ven, TO, true, true, industry, "USD");
    }

    PaxTypeFare* create(const VendorCode& ven, Directionality dir) const
    {
      return create("CX", ven, dir, true, true, false, "USD");
    }

    PaxTypeFare* create(const FareClassCode& fcc, bool cat25) const
    {
      return create(fcc, ATP, TO, true, true, false, "USD", cat25);
    }

    PaxTypeFare* create(Indicator owrt, const MoneyAmount& amt) const
    {
      return create("CX", ATP, TO, true, true, false, "USD", false, owrt, amt);
    }

    PaxTypeFare* create(const MoneyAmount& amt, const MoneyAmount& netAmt) const
    {
      return create("CX", ATP, TO, true, true, false, "USD", false, ALL_WAYS, amt, netAmt);
    }
  };

  template <typename T, int size>
  void populateFaresToFareMarket(T (&init)[size])
  {
    std::transform(
        init, init + size, std::back_inserter(ptf()), CreateFare(_memH, _trx->dataHandle()));
  }

  template <typename T, typename U, int size>
  void populateFaresToFareMarket(std::pair<T, U>(&init)[size])
  {
    std::transform(
        init, init + size, std::back_inserter(ptf()), CreateFare(_memH, _trx->dataHandle()));
  }

  void populateFaresToFareMarket()
  {
    typedef std::pair<VendorCode, Directionality> Pair;
    Pair init[] = { Pair(ATP, FROM), Pair(SITA, FROM), Pair(ATP, TO),
                    Pair(SITA, TO),  Pair(ATP, BOTH),  Pair(SITA, BOTH) };

    populateFaresToFareMarket(init);
  }

  std::vector<PaxTypeFareWrapper> populatePreSelected(std::vector<PaxTypeFareWrapper>& preselected,
                                                      unsigned one = 99,
                                                      unsigned two = 99)
  {
    CreateFare cf(_memH, _trx->dataHandle());
    preselected += cf.create(ATP, industryOn, cat35on), // 0.IndAtpSec
        cf.create(SITA, industryOn, cat35on), // 1.IndNoAtpSec
        cf.create(ATP, industryOff, cat35on), // 2.NoIndAtpSec
        cf.create(SITA, industryOff, cat35on), // 3.NoIndNoAtpSec
        cf.create(ATP, industryOn, cat35off), // 4.IndAtpNoSec
        cf.create(SITA, industryOn, cat35off), // 5.IndNoAtpNoSec
        cf.create(ATP, industryOff, cat35off), // 6.NoIndAtpNoSec
        cf.create(SITA, industryOff, cat35off), // 7.NoIndNoAtpNoSec
        cf.create(ATP, industryOn, cat35on), // 8.IndAtpSec
        cf.create(SITA, industryOn, cat35on), // 9.IndNoAtpSec
        cf.create(ATP, industryOff, cat35on), // 10.NoIndAtpSec
        cf.create(SITA, industryOff, cat35on), // 11.NoIndNoAtpSec
        cf.create(ATP, industryOn, cat35off), // 12.IndAtpNoSec
        cf.create(SITA, industryOn, cat35off), // 13.IndNoAtpNoSec
        cf.create(ATP, industryOff, cat35off), // 14.NoIndAtpNoSec
        cf.create(SITA, industryOff, cat35off), // 15.NoIndNoAtpNoSec
        cf.create(ATP, industryOn, cat35on), // 16.IndAtpSec
        cf.create(SITA, industryOn, cat35on), // 17.IndNoAtpSec
        cf.create(ATP, industryOff, cat35on), // 18.NoIndAtpSec
        cf.create(SITA, industryOff, cat35on), // 19.NoIndNoAtpSec
        cf.create(ATP, industryOn, cat35off), // 20.IndAtpNoSec
        cf.create(SITA, industryOn, cat35off), // 21.IndNoAtpNoSec
        cf.create(ATP, industryOff, cat35off), // 22.NoIndAtpNoSec
        cf.create(SITA, industryOff, cat35off); // 23.NoIndNoAtpNoSec
    populateWrapperAmount(preselected);

    std::vector<PaxTypeFareWrapper> expect;
    if (one != 99)
    {
      preselected[one].setAmount(MockRexFareSelectorStrategy::AMOUNT);
      expect += preselected[one];
    }
    if (two != 99)
    {
      preselected[two].setAmount(MockRexFareSelectorStrategy::AMOUNT);
      expect += preselected[two];
    }

    return expect;
  }

  class MockRexFareSelectorStrategy : public RexFareSelectorStrategy
  {
    friend class RexFareSelectorStrategyTest;

  public:
    MockRexFareSelectorStrategy(const RexPricingTrx& trx) : RexFareSelectorStrategy(trx) {}

    RexFareSelectorStrategy::Iterator select(FareCompInfo& fc, Iterator begin, Iterator end) const
    {
      return begin;
    }

    bool select(FareCompInfo& fc,
                Iterator begin,
                Iterator end,
                std::vector<PaxTypeFareWrapper>& selected) const
    {
      std::remove_copy_if(begin, end, std::back_inserter(selected), std::not1(IsValid()));
      return !selected.empty();
    }

    static const MoneyAmount AMOUNT;

  protected:
    const DateTime&
    getRuleApplicationDate(const RexPricingTrx& trx, const CarrierCode& govCarrier) const
    {
      return DateTime::emptyDate();
    }

    struct IsValid : std::unary_function<PaxTypeFareWrapper, bool>
    {
      bool operator()(const PaxTypeFareWrapper& wrp) const
      {
        return std::abs(wrp.getAmount() - AMOUNT) < EPSILON;
      }
    };
  };
};

const VendorCode
RexFareSelectorStrategyTest::SITA("SITA"),
    RexFareSelectorStrategyTest::ATP("ATP");

const MoneyAmount RexFareSelectorStrategyTest::NOT_AMOUNT = -1.0;

const MoneyAmount RexFareSelectorStrategyTest::MockRexFareSelectorStrategy::AMOUNT = 333.33;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorStrategyTest);

} // tse
