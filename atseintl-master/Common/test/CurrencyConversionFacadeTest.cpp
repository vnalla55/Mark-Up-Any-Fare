#include "test/include/CppUnitHelperMacros.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class CurrencyConversionFacadeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyConversionFacadeTest);
  CPPUNIT_TEST(dcConvertWithConversionCurrency);
  CPPUNIT_TEST(taxesConvertWithConversionCurrency);

  CPPUNIT_TEST_SUITE_END();

public:
  static const double EPSILON;
  static constexpr bool NO_INTERNATIONAL_ROUNDING = false;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    createMembers();
    setMembersRelations();
  }

  void tearDown() { _memHandle.clear(); }

  void dcConvertWithConversionCurrency()
  {
    initConvertWithConversionCurrency();

    const bool result = _ccf->convert(*_target,
                                      *_source,
                                      *_trx,
                                      NO_INTERNATIONAL_ROUNDING,
                                      CurrencyConversionRequest::DC_CONVERT);
    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      CPPUNIT_ASSERT( result );
      CPPUNIT_ASSERT_DOUBLES_EQUAL( 279876854184200, _target->value(), EPSILON );
    }
    else
    {
      CPPUNIT_ASSERT( result );
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1215323.462007, _target->value(), EPSILON);
    }
  }

  void taxesConvertWithConversionCurrency()
  {
    initConvertWithConversionCurrency();

    const bool result = _ccf->convert(*_target,
                                      *_source,
                                      *_trx,
                                      NO_INTERNATIONAL_ROUNDING,
                                      CurrencyConversionRequest::TAXES);
    if ( TrxUtil::isIcerActivated( *_trx ) )
    {
      CPPUNIT_ASSERT( result );
      CPPUNIT_ASSERT_DOUBLES_EQUAL( 279876854184200, _target->value(), EPSILON );
    }
    else
    {
      CPPUNIT_ASSERT( result );
      CPPUNIT_ASSERT_DOUBLES_EQUAL( 1057400.007761, _target->value(), EPSILON );
    }
  }

protected:
  void createMembers()
  {
    _memHandle.create<LocalDataHandleMock>();
    _agentLocation = _memHandle.create<Loc>();
    _agentTJR = _memHandle.create<Customer>();
    _agent = _memHandle.create<Agent>();
    _request = _memHandle.create<PricingRequest>();
    _options = _memHandle.create<PricingOptions>();
    _trx = _memHandle.create<PricingTrx>();

    _ccf = _memHandle.create<CurrencyConversionFacade>();
    _target = _memHandle.create<Money>(NUC);
    _source = _memHandle.create<Money>(NUC);
  }

  void setMembersRelations()
  {
    _agent->agentLocation() = _agentLocation;
    _agent->agentTJR() = _agentTJR;
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _trx->setOptions(_options);
  }

  void initConvertWithConversionCurrency()
  {
    _target->setCode(CurrencyCode("IRR"));
    _source->setCode(CurrencyCode("USD"));
    _source->value() = 100.0;
    _agentLocation->nation() = NationCode("RU"); // agent from RU
    _agentTJR->defaultCur() = CurrencyCode("IRR"); // logged into IR agency
  }

  class LocalDataHandleMock : public DataHandleMock
  {
  public:
    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* rates = _memHandle.create<std::vector<tse::BankerSellRate*> >();
      if (primeCur == "IRR" && cur == "USD")
        rates->push_back(getBSR(primeCur, cur, 0.00009457159, 11));
      else if (primeCur == "EUR" && cur == "USD")
        rates->push_back(getBSR(primeCur, cur, 1.2305, 4));
      else if (primeCur == "EUR" && cur == "IRR")
        rates->push_back(getBSR(primeCur, cur, 14954.5552, 4));
      else if (primeCur == "IRR" && cur == "IRR")
        rates->push_back(getBSR(primeCur, cur, 1.00000, 5));
      else if (primeCur == "USD" && cur == "IRR")
        rates->push_back(getBSR(primeCur, cur, 2798768541842, 8));
      return *rates;
    }

    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      return _memHandle.create<NUCInfo>();
    }

    const Currency* getCurrency(const CurrencyCode& currencyCode)
    {
      return _memHandle.create<Currency>();
    }

    const Currency* getCurrency(const CurrencyCode& currencyCode, const DateTime& date)
    {
      return _memHandle.create<Currency>();
    }

  protected:
    TestMemHandle _memHandle;

    BankerSellRate* getBSR(const CurrencyCode& primeCur,
                           const CurrencyCode& cur,
                           const ExchRate& rate,
                           const CurrencyNoDec& noDec,
                           const Indicator& rateType = 'B',
                           const std::string& agentSine = "FXR")
    {
      BankerSellRate* sellRate = _memHandle.create<BankerSellRate>();
      sellRate->primeCur() = primeCur;
      sellRate->cur() = cur;
      sellRate->rate() = rate;
      sellRate->rateNodec() = noDec;
      sellRate->rateType() = rateType;
      sellRate->agentSine() = agentSine;
      return sellRate;
    }
  };

  CurrencyConversionFacade* _ccf;
  Money* _target;
  Money* _source;

  PricingTrx* _trx;
  Agent* _agent;
  Loc* _agentLocation;
  Customer* _agentTJR;
  PricingRequest* _request;
  PricingOptions* _options;

  TestMemHandle _memHandle;
  RootLoggerGetOff _loggerOff;
};

const double CurrencyConversionFacadeTest::EPSILON = 0.000001;

CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyConversionFacadeTest);

}; // namespace
