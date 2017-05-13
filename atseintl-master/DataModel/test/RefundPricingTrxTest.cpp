#include <boost/assign/std/vector.hpp>
#include "DataModel/Agent.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/ExcItin.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingOptions.h"
#include "test/include/TestConfigInitializer.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> vec)
{
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, ", "));
  return os;
}

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
using namespace boost::assign;

std::ostream& operator<<(std::ostream& os, const FareMarket::RetrievalInfo& ri)
{
  return os << '{' << ri._flag << ';' << ri._date << '}';
}

bool operator==(const FareMarket::RetrievalInfo& lhs, const FareMarket::RetrievalInfo& rhs)
{
  return (lhs._flag == rhs._flag) && (lhs._date == rhs._date);
}

std::ostream& operator<<(std::ostream& os, const FareMarket& fm)
{
  return os << '{' << fm.boardMultiCity() << ';' << fm.offMultiCity() << ';'
            << fm.getGlobalDirection() << ';' << static_cast<int>(fm.geoTravelType()) << '}';
}

// bool
// operator== (const FareMarket& lhs, const FareMarket& rhs)
// {
//     return lhs.boardMultiCity() = rhs.boardMultiCity() &&
//         lhs.offMultiCity() = rhs.offMultiCity() &&
//         lhs.globalDirection() = rhs.globalDirection() &&
//         lhs.geoTravelType() = rhs.geoTravelType;

// }

class RefundPricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RefundPricingTrxTest);

  CPPUNIT_TEST(testSetAnalyzeExchangeItinWhenExchangeItin);
  CPPUNIT_TEST(testSetAnalyzeExchangeItinWhenFlownItin);

  CPPUNIT_TEST(testSetUpRexSkipSecurity_Airline);
  CPPUNIT_TEST(testSetUpRexSkipSecurity_Subscriber);

  CPPUNIT_TEST(testGetPermutationsRetrievalInfo_HistoricalTicketBasedOnly);
  CPPUNIT_TEST(testGetPermutationsRetrievalInfo_HistoricalTravelCommenBasedOnly);
  CPPUNIT_TEST(testGetPermutationsRetrievalInfo_Mixed);
  CPPUNIT_TEST(testGetPermutationsRetrievalInfo_EmptyPermutations);

  CPPUNIT_TEST(testPrepareNewFareMarkets_InHistoricalTicketBasedOnly);
  CPPUNIT_TEST(testPrepareNewFareMarkets_InHistoricalTravelCommenBasedOnly);
  CPPUNIT_TEST(testPrepareNewFareMarkets_InMixed);

  CPPUNIT_TEST(testSkipRulesOnExcItin_allForRefund);
  CPPUNIT_TEST(testSkipRulesOnExcItin_fromBaseForRefund);
  CPPUNIT_TEST(testSkipRulesOnExcItin_noneForRefund);

  CPPUNIT_TEST_SUITE_END();

protected:
  RefundPricingTrx* _refundTrx;
  ExcItin* _excItin;
  Itin* _itin;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _refundTrx = _memHandle.insert(new RefundPricingTrx());
    _memHandle.create<TestConfigInitializer>();
    _excItin = _memHandle.insert(new ExcItin());
    _itin = _memHandle.insert(new Itin());

    _refundTrx->exchangeItin().push_back(_excItin);
    _refundTrx->newItin().push_back(_itin);

    _refundTrx->setOriginalTktIssueDT() = DateTime(2008, 10, 3);
    _excItin->travelCommenceDate() = DateTime(2008, 10, 7);
  }

  void tearDown() { _memHandle.clear(); }

  void testSetAnalyzeExchangeItinWhenExchangeItin()
  {
    _refundTrx->setOptions(_memHandle.insert(new RexPricingOptions));
    RexBaseTrx& baseTrx = *_refundTrx;

    baseTrx.setAnalyzingExcItin(true);

    CPPUNIT_ASSERT_EQUAL(_excItin, baseTrx.exchangeItin().front());
    CPPUNIT_ASSERT(baseTrx.isAnalyzingExcItin());
    CPPUNIT_ASSERT(baseTrx.getOptions()->fbcSelected());
  }

  void testSetAnalyzeExchangeItinWhenFlownItin()
  {
    _refundTrx->setOptions(_memHandle.insert(new RexPricingOptions));
    RexBaseTrx& baseTrx = *_refundTrx;

    baseTrx.setAnalyzingExcItin(false);

    CPPUNIT_ASSERT_EQUAL(_itin, baseTrx.newItin().front());
    CPPUNIT_ASSERT(!baseTrx.isAnalyzingExcItin());
    CPPUNIT_ASSERT(!baseTrx.getOptions()->fbcSelected());
  }

  void helperSetUpRexSkipSecurity(const std::string& emptyMeansAirline)
  {
    _refundTrx->setRequest(_memHandle.insert(new PricingRequest));
    _refundTrx->getRequest()->ticketingAgent() = _memHandle.insert(new Agent);
    _refundTrx->getRequest()->ticketingAgent()->tvlAgencyPCC() = emptyMeansAirline;
  }

  void assertRexSkipSecurity()
  {
    CPPUNIT_ASSERT(!_refundTrx->skipSecurityForExcItin());
    _refundTrx->setUpSkipSecurityForExcItin();
    CPPUNIT_ASSERT(_refundTrx->skipSecurityForExcItin());
  }

  void testSetUpRexSkipSecurity_Airline()
  {
    TestConfigInitializer::setValue(
        "SKIP_CAT_33_AIRLINE_SECURITY_CHECK", "Y", "REX_FARE_SELECTOR_SVC");
    helperSetUpRexSkipSecurity("");
    assertRexSkipSecurity();
  }

  void testSetUpRexSkipSecurity_Subscriber()
  {
    TestConfigInitializer::setValue(
        "SKIP_CAT_33_SUBSCRIBER_SECURITY_CHECK", "Y", "REX_FARE_SELECTOR_SVC");
    helperSetUpRexSkipSecurity("ZYW");
    assertRexSkipSecurity();
  }

  RefundPermutation* createPermutation(Indicator repriceInd)
  {
    RefundPermutation* perm = _memHandle.insert(new RefundPermutation);
    perm->repriceIndicator() = repriceInd;
    return perm;
  }

  FareMarket::RetrievalInfo* createRetrievalInfo(FareMarket::FareRetrievalFlags flag)
  {
    FareMarket::RetrievalInfo* info = _memHandle.insert(new FareMarket::RetrievalInfo);
    info->_flag = flag;
    switch (flag)
    {
    case FareMarket::RetrievHistorical:
      info->_date = _refundTrx->originalTktIssueDT();
      break;
    case FareMarket::RetrievTvlCommence:
      info->_date = _excItin->travelCommenceDate();
      break;
    default:
      info->_date = DateTime::emptyDate();
    }
    return info;
  }

  struct IsEqual
  {
    bool operator()(FareMarket::RetrievalInfo* lhs, FareMarket::RetrievalInfo* rhs) const
    {
      return (*lhs) == (*rhs);
    }
  } equal;

  typedef std::vector<FareMarket::RetrievalInfo*> RetrievalInfoVec;

  void testGetPermutationsRetrievalInfo_EmptyPermutations()
  {
    RetrievalInfoVec result = _refundTrx->getPermutationsRetrievalInfo();
    CPPUNIT_ASSERT(result.empty());
  }

  void testGetPermutationsRetrievalInfo_HistoricalTicketBasedOnly()
  {
    _refundTrx->_permutations += createPermutation(RefundPermutation::HISTORICAL_TICKET_BASED);

    RetrievalInfoVec expect, result = _refundTrx->getPermutationsRetrievalInfo();

    expect += createRetrievalInfo(FareMarket::RetrievHistorical);

    CPPUNIT_ASSERT(std::equal(expect.begin(), expect.end(), result.begin(), equal));
  }

  void testGetPermutationsRetrievalInfo_HistoricalTravelCommenBasedOnly()
  {
    _refundTrx->_permutations +=
        createPermutation(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED);

    RetrievalInfoVec expect, result = _refundTrx->getPermutationsRetrievalInfo();

    expect += createRetrievalInfo(FareMarket::RetrievTvlCommence);

    CPPUNIT_ASSERT(std::equal(expect.begin(), expect.end(), result.begin(), equal));
  }

  void testGetPermutationsRetrievalInfo_Mixed()
  {
    _refundTrx->_permutations += createPermutation(RefundPermutation::HISTORICAL_TICKET_BASED),
        createPermutation(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED);

    RetrievalInfoVec expect, result = _refundTrx->getPermutationsRetrievalInfo();

    expect += createRetrievalInfo(FareMarket::RetrievHistorical),
        createRetrievalInfo(FareMarket::RetrievTvlCommence);

    CPPUNIT_ASSERT(std::equal(expect.begin(), expect.end(), result.begin(), equal));
  }

  FareMarket* createFareMarket(const LocCode& boardMultiCity = "",
                               const LocCode& offMultiCity = "",
                               GlobalDirection globalDirection = GlobalDirection::NO_DIR,
                               GeoTravelType geoTravelType = GeoTravelType::UnknownGeoTravelType)
  {
    FareMarket* fm = _memHandle.insert(new FareMarket);
    fm->boardMultiCity() = boardMultiCity;
    fm->offMultiCity() = offMultiCity;
    fm->setGlobalDirection(globalDirection);
    fm->geoTravelType() = geoTravelType;
    return fm;
  }

  void testPrepareNewFareMarkets_InHistoricalTicketBasedOnly()
  {
    _refundTrx->_permutations += createPermutation(RefundPermutation::HISTORICAL_TICKET_BASED);

    _itin->fareMarket() += createFareMarket(), createFareMarket();

    _refundTrx->prepareNewFareMarkets();

    CPPUNIT_ASSERT(_refundTrx->needRetrieveHistoricalFare());

    FareMarket::RetrievalInfo* expect = createRetrievalInfo(FareMarket::RetrievHistorical);

    CPPUNIT_ASSERT_EQUAL(*expect, *_itin->fareMarket()[0]->retrievalInfo());
    CPPUNIT_ASSERT_EQUAL(*expect, *_itin->fareMarket()[1]->retrievalInfo());
  }

  void testPrepareNewFareMarkets_InHistoricalTravelCommenBasedOnly()
  {
    _refundTrx->_permutations +=
        createPermutation(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED);

    _itin->fareMarket() += createFareMarket(), createFareMarket();

    _refundTrx->prepareNewFareMarkets();

    CPPUNIT_ASSERT(_refundTrx->needRetrieveTvlCommenceFare());

    FareMarket::RetrievalInfo* expect = createRetrievalInfo(FareMarket::RetrievTvlCommence);

    CPPUNIT_ASSERT_EQUAL(*expect, *_itin->fareMarket()[0]->retrievalInfo());
    CPPUNIT_ASSERT_EQUAL(*expect, *_itin->fareMarket()[1]->retrievalInfo());
  }

  void testPrepareNewFareMarkets_InMixed()
  {
    _refundTrx->_permutations += createPermutation(RefundPermutation::HISTORICAL_TICKET_BASED),
        createPermutation(RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED);

    _itin->fareMarket() +=
        createFareMarket("KRK", "DFW", GlobalDirection::AT, GeoTravelType::International),
        createFareMarket("DFW", "LAX", GlobalDirection::US, GeoTravelType::Domestic);

    _refundTrx->prepareNewFareMarkets();

    CPPUNIT_ASSERT(_refundTrx->needRetrieveHistoricalFare());
    CPPUNIT_ASSERT(_refundTrx->needRetrieveTvlCommenceFare());

    FareMarket::RetrievalInfo* expect[] = {createRetrievalInfo(FareMarket::RetrievHistorical),
                                           createRetrievalInfo(FareMarket::RetrievTvlCommence)};

    CPPUNIT_ASSERT_EQUAL(size_t(4), _itin->fareMarket().size());

    CPPUNIT_ASSERT_EQUAL(*expect[0], *_itin->fareMarket()[0]->retrievalInfo());
    CPPUNIT_ASSERT_EQUAL(*expect[0], *_itin->fareMarket()[1]->retrievalInfo());
    CPPUNIT_ASSERT_EQUAL(*expect[1], *_itin->fareMarket()[2]->retrievalInfo());
    CPPUNIT_ASSERT_EQUAL(*expect[1], *_itin->fareMarket()[3]->retrievalInfo());

    CPPUNIT_ASSERT_EQUAL(*_itin->fareMarket()[0], *_itin->fareMarket()[2]);
    CPPUNIT_ASSERT_EQUAL(*_itin->fareMarket()[1], *_itin->fareMarket()[3]);
  }

  typedef std::vector<uint16_t> Rules;

  Rules& getRulesVector()
  {
    Rules& rules = generateRules1to25();
    _refundTrx->skipRulesOnExcItin(rules);
    return rules;
  }

  Rules& generateRules1to25()
  {
    Rules* rules = _memHandle.insert(new Rules());
    for (int i = 1; i < 26; ++i)
      *rules += i;

    return *rules;
  }

  void testSkipRulesOnExcItin_allForRefund()
  {
    _refundTrx->arePenaltiesAndFCsEqualToSumFromFareCalc() = true;
    Rules& categories = getRulesVector();
    Rules expected;
    expected += 1, 5, 10, 15, 16, 24, 25;

    CPPUNIT_ASSERT_EQUAL(expected, categories);
  }

  void testSkipRulesOnExcItin_fromBaseForRefund()
  {
    Rules& categories = getRulesVector();
    Rules expected;
    expected += 1, 5, 8, 9, 10, 12, 15, 16, 24, 25;

    CPPUNIT_ASSERT_EQUAL(expected, categories);
  }

  void testSkipRulesOnExcItin_noneForRefund()
  {
    _refundTrx->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    Rules& categories = getRulesVector();
    Rules expected = generateRules1to25();

    CPPUNIT_ASSERT_EQUAL(expected, categories);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RefundPricingTrxTest);
}
