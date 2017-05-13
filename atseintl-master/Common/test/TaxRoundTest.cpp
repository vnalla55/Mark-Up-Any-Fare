#include <string>
#include <vector>

#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Money.h"
#include "Common/StopWatch.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxNation.h"
#include "Common/TaxRound.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }
  const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date)
  {
    if (nation == "BR")
    {
      TaxNation* ret = _memHandle.create<TaxNation>();
      ret->roundingRule() = NONE;
      ret->taxCollectionInd() = 'A';
      ret->taxFarequoteInd() = 'N';
      ret->taxCodeOrder().push_back("BR1");
      ret->taxCodeOrder().push_back("BR2");
      ret->taxCodeOrder().push_back("BR3");
      ret->taxCodeOrder().push_back("DU");
      return ret;
    }
    return DataHandleMock::getTaxNation(nation, date);
  }
};
}
class TaxRoundTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRoundTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testRetrieveRoundingData);
  CPPUNIT_TEST(testSpecialRounding);
  CPPUNIT_TEST(testSpecialRoundingSmallFareAmt1);
  CPPUNIT_TEST(testSpecialRoundingSmallFareAmt2);
  CPPUNIT_TEST(testSpecialRoundingConvert);
  CPPUNIT_TEST(testTrxNoGetOptions);
  CPPUNIT_TEST_SUITE_END();

public:
  TaxRoundTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      TaxRound taxRound;
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testRetrieveRoundingData()
  {
    TaxRound taxRound;
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    RoundingFactor roundingUnit = .1;
    CurrencyNoDec roundingNoDec = 2;
    RoundingRule roundingRule = UP;

    NationCode nation = "BR";

    PricingRequest request;

    request.ticketingDT() = time(0);

    Agent agent;
    Loc loc;

    loc.nation() = "PE";

    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingAgent()->agentLocation() = &loc;

    //   cout << "\n" << trx.getRequest()->ticketingDT() << "\n\n";

    taxRound.retrieveNationRoundingSpecifications(
        trx, nation, roundingUnit, roundingNoDec, roundingRule);
  }

  void testSpecialRounding()
  {
    TaxRound taxRound;
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);

    MoneyAmount fareAmount = 1000;
    MoneyAmount taxAmount = 10;
    MoneyAmount wrkAmount;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";
    CPPUNIT_ASSERT_EQUAL(10.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .01;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .02;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";

    fareAmount = 10;
    taxAmount = 10.02;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";

    CPPUNIT_ASSERT_EQUAL(10.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .98;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";
    CPPUNIT_ASSERT_EQUAL(1.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .99;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    // cout << "Round Amount = " << wrkAmount << "\n\n";
    CPPUNIT_ASSERT_EQUAL(1.0, wrkAmount);

    fareAmount = 10.091;
    taxAmount = .909;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(.91, wrkAmount, 1e-4);


    fareAmount = 10;
    taxAmount = .51;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);
    CPPUNIT_ASSERT_EQUAL(0.5, wrkAmount);

    fareAmount = 10;
    taxAmount = .51;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount, 10);
    CPPUNIT_ASSERT_EQUAL(0.5, wrkAmount);

    fareAmount = 10;
    taxAmount = .32;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .32;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount, 10);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .73;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .73;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount, 10);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .88;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);

    fareAmount = 10;
    taxAmount = .88;
    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount, 10);
    CPPUNIT_ASSERT_EQUAL(0.0, wrkAmount);
  }

  void testSpecialRoundingConvert()
  {
    TaxRound taxRound;

    MoneyAmount taxAmount = 10.243444;
    CurrencyCode paymentCurrency = "USD";
    MoneyAmount wrkAmount;
    RoundingFactor roundingUnit = .1;

    RoundingRule roundRule = DOWN;

    wrkAmount = taxRound.applyTaxRound(taxAmount, paymentCurrency, roundingUnit, roundRule);

    // std::cout << "\nCase 1 Round DOWN: " << wrkAmount << " for Tax :" << taxAmount << "\n";

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.2, wrkAmount, 1e-4);

    roundRule = UP;

    wrkAmount = taxRound.applyTaxRound(taxAmount, paymentCurrency, roundingUnit, roundRule);

    // std::cout << "Case 2 Round UP: " << wrkAmount << " for Tax :" << taxAmount << "\n";

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.3, wrkAmount, 1e-4);

    roundRule = NEAREST;

    wrkAmount = taxRound.applyTaxRound(taxAmount, paymentCurrency, roundingUnit, roundRule);

    // std::cout << "Case 3 Round NEAREST: " << wrkAmount << " for Tax :" << taxAmount << "\n";

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.2, wrkAmount, 1e-4);

    roundRule = NONE;

    wrkAmount = taxRound.applyTaxRound(taxAmount, paymentCurrency, roundingUnit, roundRule);

    // std::cout << "Case 4 Round NONE: " << wrkAmount << " for Tax :" << taxAmount << "\n";

    CPPUNIT_ASSERT_EQUAL(10.24, wrkAmount);
  }

  void testSpecialRoundingSmallFareAmt1()
  {
    TaxRound taxRound;
    PricingTrx trx;

    MoneyAmount fareAmount = 0.01;
    MoneyAmount taxAmount = 0.001;
    MoneyAmount wrkAmount;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    CPPUNIT_ASSERT_EQUAL(0.001, wrkAmount);
  }

  void testSpecialRoundingSmallFareAmt2()
  {
    TaxRound taxRound;
    PricingTrx trx;

    MoneyAmount fareAmount = 0.07;
    MoneyAmount taxAmount = 0.01;
    MoneyAmount wrkAmount;

    wrkAmount = taxRound.doSpecialTaxRound(trx, fareAmount, taxAmount);

    CPPUNIT_ASSERT_EQUAL(0.01, wrkAmount);
  }

  void testTrxNoGetOptions()
  {
    TaxRound taxRound;
    PricingTrx trx;

    RoundingFactor roundingUnit = .1;
    CurrencyNoDec roundingNoDec = 2;
    RoundingRule roundingRule = UP;

    NationCode nation = "BR";

    PricingRequest request;

    request.ticketingDT() = time(0);

    Agent agent;
    Loc loc;

    loc.nation() = "PE";

    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingAgent()->agentLocation() = &loc;

    CPPUNIT_ASSERT_NO_THROW(taxRound.retrieveNationRoundingSpecifications(
        trx, nation, roundingUnit, roundingNoDec, roundingRule));
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRoundTest);
}
