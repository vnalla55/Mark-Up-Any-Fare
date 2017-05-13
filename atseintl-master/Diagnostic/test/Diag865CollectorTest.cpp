#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag865Collector.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diagnostic.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "Rules/Commissions.h"
#include "DBAccess/NegFareRest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/CollectedNegFareData.h"
#include "Rules/RuleConst.h"

namespace tse
{

namespace
{
  class MockCommissions : public Commissions
  {
  public:
    MockCommissions(PricingTrx& trx) : Commissions(trx)
    {
      _baseFareCurrency = "NUC";
      _calculationCurrency = "NUC";
      _paymentCurrency = "NUC";
    }
  };
}

class Diag865CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag865CollectorTest);
  CPPUNIT_TEST(testDisplayRequestData);
  CPPUNIT_TEST(testDisplayFareComponentHeader);
  CPPUNIT_TEST(testDisplayFarePathHeader);
  CPPUNIT_TEST(testDisplayCommissionData_FareComponent);
  CPPUNIT_TEST(testDisplayCommissionData_FarePath);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodNone);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodAmount);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodNetTimesCommPct);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodSellTimesCommPct);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodMarkupPlusSellTimesCommPct);
  CPPUNIT_TEST(testDisplayCommissionApplication_MethodMarkupPlusCommAmt);
  CPPUNIT_TEST(testDisplayFailInfo);
  CPPUNIT_TEST(testFormatAmount);
  CPPUNIT_TEST(testFormatPercent);
  CPPUNIT_TEST(testFormatPercent_NotAppliacable);
  CPPUNIT_TEST(testFormatPercent_ZeroNA_True);
  CPPUNIT_TEST(testFormatFlag_True);
  CPPUNIT_TEST(testFormatFlag_False);
  CPPUNIT_TEST_SUITE_END();

protected:
  class MockFareMarket : public FareMarket
  {
  public:
    MockFareMarket() : FareMarket()
    {
      _loc1.loc() = "DEN";
      _loc2.loc() = "LON";

      origin() = &_loc1;
      destination() = &_loc2;
    }

  protected:
    Loc _loc1;
    Loc _loc2;
  };

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare() : PaxTypeFare()
    {
      _tariffCrossRefInfo.ruleTariff() = 389;

      _fareInfo.vendor() = "ATP";
      _fareInfo.carrier() = "BA";
      _fareInfo.ruleNumber() = "JP01";
      _fareInfo.fareClass() = "Y";

      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);

      setFare(&_fare);
      fareMarket() = &_fareMarket;
    }

  protected:
    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    MockFareMarket _fareMarket;
  };

  Diag865Collector* _collector;
  Diagnostic* _diagRoot;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;

public:
  void setUp()
  {
    _diagRoot = _memHandle.insert(new Diagnostic(Diagnostic865));
    _diagRoot->activate();
    _collector = _memHandle.insert(new Diag865Collector(*_diagRoot));
    _collector->enable(Diagnostic865);
  }

  void tearDown() { _memHandle.clear(); }

  void testDisplayRequestData()
  {
    initTrx();

    MockCommissions comm(*_trx);
    CollectedNegFareData collectedNegFareData;

    _collector->displayRequestData(*_trx, *_farePath, comm, collectedNegFareData);

    std::string expectedResult("***************************************************************\n"
                               "*  ATSE REQUEST DATA FOR NEGOTIATED FARE COMMISSION PROCESS   *\n"
                               "***************************************************************\n"
                               "  ABACUS USER                      : N\n"
                               "  AXESS USER                       : N\n"
                               "  CWT USER                         : N\n"
                               "  NET REMIT METHOD                 :  \n"
                               "  TICKETING ENTRY                  : N\n"
                               "  PRINT SELLING CATEGORY 35 FARE   : N\n"
                               "  PRINT NET CATEGORY 35 FARE       : N\n"
                               "  CREDIT CARD FOP                  : N\n"
                               "  CASH FOP                         : N\n"
                               "  OTHER FOP                        : N\n"
                               "  BASE FARE CURRENCY               : NUC\n"
                               "  PAYMENT CURRENCY                 : NUC\n"
                               "  AGENT COMMISSION TYPE            : NONE\n"
                               "  AGENT COMMISSION VALUE           : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayFareComponentHeader()
  {
    _collector->displayFareComponentHeader();

    std::string expectedResult("***************************************************************\n"
                               "*         COMMISSION ANALYSIS - FARE COMPONENT LEVEL          *\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayFarePathHeader()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _farePath->collectedNegFareData()->currency() = "NUC";

    _collector->displayFarePathHeader(*_farePath);

    std::string expectedResult("***************************************************************\n"
                               "*            COMMISSION ANALYSIS - FARE PATH LEVEL            *\n"
                               "***************************************************************\n"
                               "***************************************************************\n"
                               "  REQUESTED PAXTYPE: ADT\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionData_FareComponent()
  {
    initTrx();

    MockCommissions comm(*_trx);
    FareUsage fareUsage;
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;
    MockPaxTypeFare paxTypeFare;

    negFareRest.cur1() = "NUC";
    negFareRest.cur2() = "NUC";

    fareUsage.paxTypeFare() = &paxTypeFare;

    _collector->displayCommissionData(comm, &fareUsage, *_farePath, &negFareRest, &negPaxTypeFare);

    std::string expectedResult("***************************************************************\n"
                               " DEN - BA - LON  Y\n"
                               "***************************************************************\n"
                               "  REQUESTED PAXTYPE: ADT      TEXT:          DISPLAY TYPE:  \n"
                               " \n"
                               "  COMMISSION PERCENT               : 0.00\n"
                               "  COMMISSION AMOUNT 1              : 0.00      NUC\n"
                               "  COMMISSION AMOUNT 2              : 0.00      NUC\n"
                               "  NET GROSS INDICATOR              :  \n"
                               " \n"
                               "  SELLING CATEGORY 35 FARE AMT     : 0.00      NUC\n"
                               "  EXTRA MILAGE SURCHARGE           : 0.00      NUC\n"
                               "  CAT8  STOPOVER CHARGE            : 0.00      NUC\n"
                               "  CAT9  TRANSFER CHARGE            : 0.00      NUC\n"
                               "  CAT12 SURCHARGE                  : 0.00      NUC\n"
                               "  TOTAL SELL AMT                   : 0.00      NUC\n"
                               "  TOTAL SELL AMT IN PAYMENT CURR   : 0.00      NUC\n"
                               " \n"
                               "  NET CATEGORY 35 FARE AMT         : 0.00      NUC\n"
                               "  EXTRA MILAGE SURCHARGE           : 0.00      NUC\n"
                               "  CAT8  STOPOVER CHARGE            : 0.00      NUC\n"
                               "  CAT9  TRANSFER CHARGE            : 0.00      NUC\n"
                               "  CAT12 SURCHARGE                  : 0.00      NUC\n"
                               "  TOTAL NET AMT                    : 0.00      NUC\n"
                               "  TOTAL NET AMT IN PAYMENT CURR    : 0.00      NUC\n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionData_FarePath()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _farePath->collectedNegFareData()->currency() = "NUC";

    _collector->displayCommissionData(comm, *_farePath);

    std::string expectedResult(" \n"
                               "  COMMISSION PERCENT               : 0.00\n"
                               "  COMMISSION AMOUNT                : 0.00      NUC\n"
                               "  NET GROSS INDICATOR              :  \n"
                               " \n"
                               "  SELLING CATEGORY 35 FARE AMT     : 0.00      NUC\n"
                               "  EXTRA MILAGE SURCHARGE           : 0.00      NUC\n"
                               "  CAT8  STOPOVER CHARGE            : 0.00      NUC\n"
                               "  CAT9  TRANSFER CHARGE            : 0.00      NUC\n"
                               "  CAT12 SURCHARGE                  : 0.00      NUC\n"
                               "  TOTAL SELL AMT                   : 0.00      NUC\n"
                               "  TOTAL SELL AMT IN PAYMENT CURR   : 0.00      NUC\n"
                               " \n"
                               "  NET CATEGORY 35 FARE AMT         : 0.00      NUC\n"
                               "  EXTRA MILAGE SURCHARGE           : 0.00      NUC\n"
                               "  CAT8  STOPOVER CHARGE            : 0.00      NUC\n"
                               "  CAT9  TRANSFER CHARGE            : 0.00      NUC\n"
                               "  CAT12 SURCHARGE                  : 0.00      NUC\n"
                               "  TOTAL NET AMT                    : 0.00      NUC\n"
                               "  TOTAL NET AMT IN PAYMENT CURR    : 0.00      NUC\n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodNone()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(comm);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : \n"
                               "  NET  * COMM PCT                  : \n"
                               "  SELL * COMM PCT                  : \n"
                               "  SELL - NET PLUS SELL * COMM PCT  : \n"
                               "  SELL - NET PLUS COMM AMOUNT      : \n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodAmount()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(comm, 100.0, Diag865Collector::AMOUNT);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : 100.00    NUC\n"
                               "  NET  * COMM PCT                  : \n"
                               "  SELL * COMM PCT                  : \n"
                               "  SELL - NET PLUS SELL * COMM PCT  : \n"
                               "  SELL - NET PLUS COMM AMOUNT      : \n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodNetTimesCommPct()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(comm, 100.0, Diag865Collector::NET_TIMES_COMM_PCT);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : \n"
                               "  NET  * COMM PCT                  : 100.00    NUC\n"
                               "  SELL * COMM PCT                  : \n"
                               "  SELL - NET PLUS SELL * COMM PCT  : \n"
                               "  SELL - NET PLUS COMM AMOUNT      : \n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodSellTimesCommPct()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(comm, 100.0, Diag865Collector::SELL_TIMES_COMM_PCT);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : \n"
                               "  NET  * COMM PCT                  : \n"
                               "  SELL * COMM PCT                  : 100.00    NUC\n"
                               "  SELL - NET PLUS SELL * COMM PCT  : \n"
                               "  SELL - NET PLUS COMM AMOUNT      : \n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodMarkupPlusSellTimesCommPct()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(
        comm, 100.0, Diag865Collector::MARKUP_PLUS_SELL_TIMES_COMM_PCT);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : \n"
                               "  NET  * COMM PCT                  : \n"
                               "  SELL * COMM PCT                  : \n"
                               "  SELL - NET PLUS SELL * COMM PCT  : 100.00    NUC\n"
                               "  SELL - NET PLUS COMM AMOUNT      : \n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayCommissionApplication_MethodMarkupPlusCommAmt()
  {
    initTrx();

    MockCommissions comm(*_trx);

    _collector->displayCommissionApplication(comm, 100.0, Diag865Collector::MARKUP_PLUS_COMM_AMT);

    std::string expectedResult("  COMMISSION APPLICATION\n"
                               "  AMOUNT                           : \n"
                               "  NET  * COMM PCT                  : \n"
                               "  SELL * COMM PCT                  : \n"
                               "  SELL - NET PLUS SELL * COMM PCT  : \n"
                               "  SELL - NET PLUS COMM AMOUNT      : 100.00    NUC\n"
                               "  SELL - NET DIFFERENCE            : \n"
                               " \n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testDisplayFailInfo()
  {
    _collector->displayFailInfo();

    std::string expectedResult("    CAT35 RULE - FAIL:\n"
                               " SKIP CAT35 COMMISSIONS - TOTAL NET AMOUNT EXCEEDS SELLING\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testFormatAmount()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("123.45    NUC"), _collector->formatAmount(123.45, NUC));
  }

  void testFormatPercent()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("99.00"), _collector->formatPercent(99));
  }

  void testFormatPercent_NotAppliacable()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NOT APPLICABLE"),
                         _collector->formatPercent(RuleConst::PERCENT_NO_APPL));
  }

  void testFormatPercent_ZeroNA_True()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NOT APPLICABLE"), _collector->formatPercent(0, true));
    CPPUNIT_ASSERT_EQUAL(std::string("12.00"), _collector->formatPercent(12, true));
  }

  void testFormatFlag_True()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("Y"), _collector->formatFlag(true));
  }

  void testFormatFlag_False()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("N"), _collector->formatFlag(false));
  }

private:
  void initTrx()
  {
    PricingRequest* request;
    PricingOptions* options;
    Itin* itin;
    Agent* agent;
    PaxType* paxType;
    CollectedNegFareData* cNegFareData;

    _memHandle.get(_trx);
    _memHandle.get(request);
    _memHandle.get(options);
    _memHandle.get(itin);
    _memHandle.get(_farePath);
    _memHandle.get(agent);
    _memHandle.get(paxType);
    _memHandle.get(cNegFareData);

    request->ticketingAgent() = agent;
    _trx->setRequest(request);
    _trx->setOptions(options);
    _trx->itin().push_back(itin);
    itin->farePath().push_back(_farePath);
    paxType->paxType() = "ADT";
    _farePath->itin() = itin;
    _farePath->paxType() = paxType;
    _farePath->collectedNegFareData() = cNegFareData;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag865CollectorTest);
}
