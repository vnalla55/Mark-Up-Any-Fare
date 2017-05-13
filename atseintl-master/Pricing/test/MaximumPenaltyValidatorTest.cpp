//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Common/ErrorResponseException.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/test/PricingMockDataBuilder.h"
#include "Rules/Penalties.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MaximumPenaltyValidatorTest : public CppUnit::TestFixture
{
private:
  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<GeneralFareRuleInfo*>& getGeneralFareRule(const VendorCode& vendor,
                                                                const CarrierCode& carrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& rule,
                                                                const CatNumber& category,
                                                                const DateTime& date,
                                                                const DateTime& applDate)
    {
      static std::vector<GeneralFareRuleInfo*> ret;
      return ret;
    }
    bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
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
    const Loc* getLoc(const LocCode& locCode, const DateTime& date) { return nullptr; }
  };

  CPPUNIT_TEST_SUITE(MaximumPenaltyValidatorTest);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsEmpty);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNonChangeable);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNonRefundable);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNonChangeableOrNonRefundable);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNonChangeableAndNonRefundable);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNoFaresWithChangePenalty);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNoFaresWithRefundPenalty);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNoFaresBothOr);
  CPPUNIT_TEST(testGetFailedFaresDiagnosticsNoFaresBothAnd);
  CPPUNIT_TEST(testValidateFarePathInfoMode);
  CPPUNIT_SKIP_TEST(testValidateFarePathMissingDataChangeFilter);
  CPPUNIT_SKIP_TEST(testValidateFarePathMissingDataRefundFilter);
  CPPUNIT_TEST(testGetPenaltyCurrency);
  CPPUNIT_TEST(testValidateQueryApplicablePass);
  CPPUNIT_TEST(testValidateQueryApplicableFail);
  CPPUNIT_TEST(testValidateQueryNotApplicablePass);
  CPPUNIT_TEST(testValidateQueryNotApplicableFail);
  CPPUNIT_TEST(testValidateAmountLowerPass);
  CPPUNIT_TEST(testValidateAmountEqualsPass);
  CPPUNIT_TEST(testValidateAmountHigherFail);
  CPPUNIT_TEST(testValidateAmountNonApplicableFail);
  CPPUNIT_TEST(testValidateFilterAmountPass);
  CPPUNIT_TEST(testValidateFilterQueryPass);
  CPPUNIT_TEST(testValidateFilterAmountFail);
  CPPUNIT_TEST(testValidateFilterQueryFail);
  CPPUNIT_TEST_SUITE_END();

  using ValidationResponse = std::pair<bool, std::string>;
  using Fee = MaxPenaltyResponse::Fee;
  using Fees = MaxPenaltyResponse::Fees;
  using Departure = smp::RecordApplication;
  using Filter = MaxPenaltyInfo::Filter;
  using Query = smp::ChangeQuery;

public:
  MaximumPenaltyValidatorTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = PricingMockDataBuilder::getPricingTrx();
    _memHandle.create<MyDataHandle>();
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DISPLAY_DETAIL, "MAXPEN"));
    TestConfigInitializer::setValue("MAX_PENALTY_FAILED_FARES_THRESHOLD", 100, "PRICING_SVC");
    _maxPenaltyValidator = _memHandle.create<MaximumPenaltyValidator>(*_trx);

    auto itin = PricingMockDataBuilder::addItin(*_trx);
    auto paxType = PricingMockDataBuilder::addPaxType(*_trx, "NIL");
    auto origin = PricingMockDataBuilder::getLoc(*_trx, "NIL");
    auto dest = PricingMockDataBuilder::getLoc(*_trx, "NIL");
    auto fareMarket =
        PricingMockDataBuilder::addFareMarket(*_trx, *itin, "TEST_CARRIER", origin, dest);
    auto pricingUnit = _memHandle.create<PricingUnit>();
    auto fareUsage = _memHandle.create<FareUsage>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    auto fare = _memHandle.create<Fare>();
    auto fareInfo = _memHandle.create<FareInfo>();
    fare->initialize(Fare::FS_ConstructedFare,
                     fareInfo,
                     *fareMarket,
                     _memHandle.create<TariffCrossRefInfo>(),
                     _memHandle.create<ConstructedFareInfo>());
    paxType->maxPenaltyInfo() = _memHandle.create<MaxPenaltyInfo>();
    _paxTypeFare->initialize(fare, paxType, fareMarket);
    _paxTypeFare->initializeMaxPenaltyStructure(_trx->dataHandle());
    fareUsage->paxTypeFare() = _paxTypeFare;
    pricingUnit->fareUsage().push_back(fareUsage);
    _farePath = _memHandle.create<FarePath>();
    _farePath->paxType() = paxType;
    _farePath->pricingUnit().push_back(pricingUnit);
    _farePath->itin() = itin;
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _trx;
    _trx = nullptr;
  }

  bool validateFilter(const Filter& filter, Fees& fees)
  {
    return _maxPenaltyValidator->validateFilter(filter, fees).first;
  }

  bool validateQuery(const Query& query, const Departure& departure, Fees& fees)
  {
    return _maxPenaltyValidator->validateQuery(query, departure, fees);
  }

  bool validateAmount(const Money& maxPenalty, const Departure& departure, Fees& fees)
  {
    return _maxPenaltyValidator->validateAmount(maxPenalty, departure, fees);
  }

  void testGetFailedFaresDiagnosticsEmpty()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string(),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::AND, {smp::BOTH, {}, {}}, {smp::BOTH, {}, {}}}, {{false, 0.0}, {false, 0.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNonChangeable()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO NONCHANGEABLE FARES"),
                         MaximumPenaltyValidator::getFailedFaresDiagnostics(
                             {smp::AND, {smp::BOTH, smp::NONCHANGEABLE, {}}, {smp::BOTH, {}, {}}},
                             {{true, 0.0}, {false, 0.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNonRefundable()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO NONREFUNDABLE FARES"),
                         MaximumPenaltyValidator::getFailedFaresDiagnostics(
                             {smp::AND, {smp::BOTH, {}, {}}, {smp::BOTH, smp::NONCHANGEABLE, {}}},
                             {{false, 0.0}, {true, 0.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNonChangeableOrNonRefundable()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "MAXIMUM PENALTY IS TOO RESTRICTIVE. NO NONCHANGEABLE FARES OR NO NONREFUNDABLE FARES"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::AND, {smp::BOTH, smp::NONCHANGEABLE, {}}, {smp::BOTH, smp::NONCHANGEABLE, {}}},
            {{true, 0.0}, {true, 0.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNonChangeableAndNonRefundable()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO NONCHANGEABLE FARES AND NO "
                    "NONREFUNDABLE FARES"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::OR, {smp::BOTH, smp::NONCHANGEABLE, {}}, {smp::BOTH, smp::NONCHANGEABLE, {}}},
            {{true, 0.0}, {true, 0.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNoFaresWithChangePenalty()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO FARES WITH WITH CHANGE PENALTY LESS "
                    "THAN 200.00 USD"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::AND, {smp::BOTH, {}, Money(100.0, USD)}, {smp::BOTH, smp::NONCHANGEABLE, {}}},
            {{false, 200.0}, {true, 0.0}}));
  }

  void testGetFailedFaresDiagnosticsNoFaresWithRefundPenalty()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO FARES WITH WITH REFUND PENALTY LESS "
                    "THAN 200.00 USD"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::AND, {smp::BOTH, smp::NONCHANGEABLE, {}}, {smp::BOTH, {}, Money(100.0, USD)}},
            {{true, 0.0}, {false, 200.0}, 0}));
  }

  void testGetFailedFaresDiagnosticsNoFaresBothOr()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO FARES WITH WITH CHANGE PENALTY LESS "
                    "THAN 200.00 USD OR NO FARES WITH WITH REFUND PENALTY LESS THAN 199.99 USD"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::AND, {smp::BOTH, {}, Money(100.0, USD)}, {smp::BOTH, {}, Money(100.0, USD)}},
            {{false, 200.0}, {false, 199.99}, 0}));
  }

  void testGetFailedFaresDiagnosticsNoFaresBothAnd()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("MAXIMUM PENALTY IS TOO RESTRICTIVE. NO FARES WITH WITH CHANGE PENALTY LESS "
                    "THAN 200.00 USD AND NO FARES WITH WITH REFUND PENALTY LESS THAN 199.99 USD"),
        MaximumPenaltyValidator::getFailedFaresDiagnostics(
            {smp::OR, {smp::BOTH, {}, Money(100.0, USD)}, {smp::BOTH, {}, Money(100.0, USD)}},
            {{false, 200.0}, {false, 199.99}, 0}));
  }

  void testValidateFarePathInfoMode()
  {
    MaxPenaltyInfo info{smp::INFO, {smp::BOTH, {}, {}}, {smp::BOTH, {}, {}}};
    _farePath->paxType()->maxPenaltyInfo() = &info;

    ValidationResponse response = _maxPenaltyValidator->validateFarePath(*_farePath);

    CPPUNIT_ASSERT_EQUAL(true, response.first);
    CPPUNIT_ASSERT_EQUAL(std::string(""), response.second);
  }

  void testValidateFarePathMissingDataChangeFilter()
  {
    MaxPenaltyInfo info{
        smp::OR, {smp::BOTH, smp::CHANGEABLE, Money{100.0, USD}}, {smp::BOTH, {}, {}}};
    _farePath->paxType()->maxPenaltyInfo() = &info;

    PenaltyInfo record;
    record.noRefundInd() = Penalties::RESERVATIONS_CANNOT_BE_CHANGED;
    _paxTypeFare->addPenaltyInfo(&record);

    ValidationResponse response = _maxPenaltyValidator->validateFarePath(*_farePath);
    CPPUNIT_ASSERT_EQUAL(true, response.first);
    CPPUNIT_ASSERT_EQUAL(std::string(""), response.second);
  }

  void testValidateFarePathMissingDataRefundFilter()
  {
    MaxPenaltyInfo info{
        smp::AND, {smp::BOTH, {}, {}}, {smp::BOTH, smp::CHANGEABLE, Money{100.0, USD}}};
    _farePath->paxType()->maxPenaltyInfo() = &info;

    PenaltyInfo record;
    record.noRefundInd() = Penalties::TICKET_NON_REFUNDABLE;
    _paxTypeFare->addPenaltyInfo(&record);

    ValidationResponse response = _maxPenaltyValidator->validateFarePath(*_farePath);
    CPPUNIT_ASSERT_EQUAL(true, response.first);
    CPPUNIT_ASSERT_EQUAL(std::string(""), response.second);
  }

  void testGetPenaltyCurrency()
  {
    _trx->setRequest(_memHandle.create<PricingRequest>());
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    _trx->setOptions(pricingOptions);
    Agent* ticketingAgent = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent() = ticketingAgent;

    Filter filter{};
    CPPUNIT_ASSERT_THROW(_maxPenaltyValidator->getPenaltyCurrency(filter), ErrorResponseException);

    _trx->getOptions()->currencyOverride() = NUC;
    CPPUNIT_ASSERT(_maxPenaltyValidator->getPenaltyCurrency(filter) == NUC);

    ticketingAgent->currencyCodeAgent() = "GBP";
    //override has precedence
    CPPUNIT_ASSERT(_maxPenaltyValidator->getPenaltyCurrency(filter) == NUC);

    filter = {smp::BOTH, smp::CHANGEABLE, Money(100.0, USD)};
    CPPUNIT_ASSERT(_maxPenaltyValidator->getPenaltyCurrency(filter) == USD);
  }

  void testValidateQueryApplicablePass()
  {
    Query query = smp::CHANGEABLE;
    Fees fees{Fee(Money(100.0, USD), false), Fee()};
    Departure departure = smp::BEFORE;
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));

    fees = {Fee(), Fee(Money(0., USD))};
    departure = smp::AFTER;
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));

    departure = smp::BOTH;
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));

    fees = {Fee(Money(0., USD)), Fee()};
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));
  }

  void testValidateQueryApplicableFail()
  {
    Query query = smp::CHANGEABLE;
    Fees fees{Fee(), Fee(Money(0., USD))};
    Departure departure = smp::BEFORE;
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));

    fees = {Fee(Money(0., USD)), Fee()};
    departure = smp::AFTER;
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));

    fees = {Fee(), Fee()};
    departure = smp::BOTH;
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));
  }

  void testValidateQueryNotApplicablePass()
  {
    Query query = smp::NONCHANGEABLE;
    Fees fees{Fee(), Fee()};
    Departure departure = smp::BOTH;
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(), Fee(Money(0., USD))};
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(Money(100.0, USD)), Fee()};
    CPPUNIT_ASSERT(validateQuery(query, departure, fees));
  }

  void testValidateQueryNotApplicableFail()
  {
    Query query = smp::NONCHANGEABLE;
    Fees fees{Fee(Money(0., USD)), Fee(Money(10., USD))};
    Departure departure = smp::BOTH;
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(Money(0., USD)), Fee()};
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(), Fee(Money(0., USD))};
    CPPUNIT_ASSERT(!validateQuery(query, departure, fees));
  }

  void testValidateAmountEqualsPass()
  {
    const Money maxPenalty = Money(100.0, USD);
    Departure departure = smp::BOTH;
    Fees fees{Fee(Money(100.0, USD)), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(Money(100.0, USD)), Fee()};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));
  }

  void testValidateAmountLowerPass()
  {
    const Money maxPenalty = Money(150.0, USD);
    Departure departure = smp::BOTH;
    Fees fees{Fee(Money(130.0, USD)), Fee(Money(149.99, USD))};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(Money(0.01, USD)), Fee()};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(), Fee(Money(0.0, USD))};
    CPPUNIT_ASSERT(validateAmount(maxPenalty, departure, fees));
  }

  void testValidateAmountHigherFail()
  {
    const Money maxPenalty = Money(50.0, USD);
    Departure departure = smp::BOTH;
    Fees fees{Fee(Money(100.0, USD)), Fee(Money(100.01, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(10.0, USD)), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(50.01, USD)), Fee(Money(0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(Money(50.01, USD)), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(3000, USD)), Fee(Money(10.0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(), Fee(Money(1000.0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(10.0, USD)), Fee(Money(50.01, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
  }

  void testValidateAmountNonApplicableFail()
  {
    const Money maxPenalty = Money(150.0, USD);
    Departure departure = smp::BOTH;
    Fees fees{Fee(), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(149.99, USD)), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));

    departure = smp::BEFORE;
    fees = {Fee(), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));

    departure = smp::AFTER;
    fees = {Fee(), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
    fees = {Fee(Money(1000, USD)), Fee()};
    CPPUNIT_ASSERT(!validateAmount(maxPenalty, departure, fees));
  }

  void testValidateFilterAmountPass()
  {
    Filter filter{smp::BOTH, {}, Money(100.0, USD)};
    Fees fees{Fee(Money(99.9, USD)), Fee(Money(0.01, USD), true)};
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::BEFORE;
    fees._after = Fee();
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._after = Fee(Money(100.01, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    fees._after = Fee(Money(100.00, USD), true);
    fees._before = Fee();
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._before = Fee(Money(100000, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));
  }

  void testValidateFilterQueryPass()
  {
    Filter filter{smp::BOTH, smp::CHANGEABLE, {}};
    Fees fees{Fee(Money(0.0, USD)), Fee(Money(100000, USD))};
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::BEFORE;
    fees._after = Fee();
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._before = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    fees._before = Fee();
    fees._after = Fee(Money(0, USD));
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._after = Fee(Money(10000, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._query = smp::NONCHANGEABLE;
    fees = {Fee(), Fee()};
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::BEFORE;
    fees._after = Fee(Money(0, USD));
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._after = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    fees._after = Fee();
    fees._before = Fee(Money(0, USD));
    CPPUNIT_ASSERT(validateFilter(filter, fees));
    fees._before = Fee(Money(10000, USD), true);
    CPPUNIT_ASSERT(validateFilter(filter, fees));
  }

  void testValidateFilterAmountFail()
  {
    Filter filter{smp::BOTH, {}, Money(100.0, USD)};
    Fees fees{Fee(Money(99.9, USD)), Fee(Money(100.01, USD), true)};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees = {Fee(Money(100.01, USD)), Fee(Money(0, USD))};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));

    filter._departure = smp::BEFORE;
    fees._after = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._after = Fee(Money(100.01, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    fees._before = Fee(Money(100.00, USD), true);
    fees._after = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee(Money(100000, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
  }

  void testValidateFilterQueryFail()
  {
    Filter filter{smp::BOTH, smp::CHANGEABLE, {}};
    Fees fees{Fee(), Fee()};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    /* AND LOGIC
    fees._before = Fee(Money(0.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee();
    fees._after = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    */

    filter._departure = smp::BEFORE;
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._after = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    fees._after = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));

    filter._query = smp::NONCHANGEABLE;
    fees = {Fee(Money(100.0, USD)), Fee(Money(100.0, USD))};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees = {{{}, false}, {{}, false}};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    /* AND LOGIC
    fees._before = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee(Money(0.0, USD);
    fees._after = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    */

    filter._departure = smp::BEFORE;
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._after = Fee();
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee{{}, false};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._after = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));

    filter._departure = smp::AFTER;
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee(Money(0, USD));
    fees._after = Fee{{}, false};
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._before = Fee(Money(10000, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
    fees._after = Fee(Money(100.0, USD), true);
    CPPUNIT_ASSERT(!validateFilter(filter, fees));
  }

private:

  TestMemHandle _memHandle;
  MaximumPenaltyValidator* _maxPenaltyValidator;
  PricingTrx* _trx = nullptr;
  FarePath* _farePath;
  PaxTypeFare* _paxTypeFare;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MaximumPenaltyValidatorTest);
} //tse
