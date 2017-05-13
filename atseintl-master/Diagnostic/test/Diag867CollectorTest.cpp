#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Diagnostic/Diag867Collector.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diagnostic.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/CollectedNegFareData.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionLocSegInfo.h"
#include "DBAccess/CommissionMarketSegInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CommissionTravelDatesSegInfo.h"
#include "DBAccess/Currency.h"
#include "Common/CommissionKeys.h"
#include "Rules/RuleConst.h"

namespace tse
{
// MOCKS
namespace
{
class Diag867CollectorDataHandleMock : public DataHandleMock
{
public:
  Diag867CollectorDataHandleMock(TestMemHandle& memHandle) : _memHandle(memHandle) {}

  const Currency* getCurrency(const CurrencyCode& currency)
  {
    if (currency == "COS" || currency == "AMC" || currency == "LOL")
    {
      Currency* cur = _memHandle.create<Currency>();
      cur->noDec() = 2;
      return cur;
    }
    else if (currency == "XXX")
      return 0;
    else
      return DataHandleMock::getCurrency(currency);
  }

private:
  TestMemHandle& _memHandle;
};

class Diag867CollectorTestMock : public Diag867Collector
{
MoneyAmount convertCurrency(const FarePath& fpath,
                                      MoneyAmount sourceAmount,
                                      const CurrencyCode& sourceCurrency,
                                      const CurrencyCode& targetCurrency,
                                      bool doNonIataRounding)
{
  return sourceAmount;
}
};
}

class Diag867CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag867CollectorTest);
  CPPUNIT_TEST(testDisplayFarePathHeader);
  CPPUNIT_TEST(testPrintAMCforCat35NotApplicable);
  CPPUNIT_TEST(testPrintAMCforCat35NotApplicable_PaxTypeNotMatch);
  CPPUNIT_TEST(testPrintAMCforASNotApplicable);
  CPPUNIT_TEST(testPrintAMCforASNotApplicable_PaxTypeNotMatch);
  CPPUNIT_TEST(testPrintAMCCommandNotSupported);
  CPPUNIT_TEST(testPrintAMCCommandNotSupported_PaxTypeNotMatch);
  CPPUNIT_TEST(testPrintNoSecurityHandShakeForPCC);
  CPPUNIT_TEST(testPrintNoSecurityHandShakeForPCC_PaxTypeNotMatch);
  CPPUNIT_TEST(testPrintNoCommissionContractFound);
  CPPUNIT_TEST(testPrintAMCforKPandEPRNotApplicable);
  CPPUNIT_TEST(testPrintAMCforKPandEPRNotApplicable_PaxTypeNotMatch);
  CPPUNIT_TEST(testPaxTypeMatched_Pass);
  CPPUNIT_TEST(testPaxTypeMatched_Fail);
  CPPUNIT_TEST(testPrintCommissionContractInfoLong);
  CPPUNIT_TEST(testPrintFpCurrencies_FP_PaymentCurrency);
  CPPUNIT_TEST(testPrintFpCurrencies_ITIN_PaymentCurrency);
  CPPUNIT_TEST(testPrintCommissionContractInfoShort);
  CPPUNIT_TEST(testPrintCommissionsRuleInfoType12PASS);
  CPPUNIT_TEST(testPrintCommissionsRuleInfoType_9_FAIL_MATCH_EXCL);
  CPPUNIT_TEST(printCommissionsRuleInfoShortType12Pass);
  CPPUNIT_TEST(printCommissionsRuleInfoShortType9_FAIL_InterlineCONNECTION);
  CPPUNIT_TEST(testPrintCommissionsRuleInfo_with_overflow_vector_data);
  CPPUNIT_TEST(testPrintCommissionsRuleInfo_with_NonStop_overflow_vector_data);
  CPPUNIT_TEST(testPrintCommissionsRuleInfo_with_overflow_vector_data_GreenScreen);

  CPPUNIT_TEST(testPrintCommissionsProgramInfo_with_Limited_data);
  CPPUNIT_TEST(testPrintCommissionsProgramInfo_with_PointOfSaleAndOrigin_data);
  CPPUNIT_TEST(testPrintCommissionsProgramInfo_with_PointOfSaleAndOrigin_data_FAIL);
  CPPUNIT_TEST(testPrintShortCommissionsProgramInfo_PASS);
  CPPUNIT_TEST(testPrintShortCommissionsProgramInfo_FAIL);
  CPPUNIT_TEST(testPrintCommissionFCHeader);
  CPPUNIT_TEST(testPrintNoCommissionFound);
  CPPUNIT_TEST(testPrintCommissionFC);
  CPPUNIT_TEST(testPrintFinalAgencyCommission);
  CPPUNIT_TEST(testPrintFinalFcCommission);
  CPPUNIT_TEST(testPrintFinalFcCommission_RollBack_Surcharges);

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

  Diag867Collector* _collector;
  Diag867CollectorTestMock* _collectorM;

  Diagnostic* _diagRoot;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  FareUsage* _fareUsage;
  CommissionRuleInfo * _cri;
  CommissionProgramInfo* _cpi;
  CommissionContractInfo* _cci;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diagRoot = _memHandle.insert(new Diagnostic(Diagnostic867));
    _diagRoot->activate();
    _collector = _memHandle.insert(new Diag867Collector(*_diagRoot));
    _collector->enable(Diagnostic867);
    _collectorM = _memHandle.insert(new Diag867CollectorTestMock());
    _collectorM->activate();
    _memHandle.insert(new Diag867CollectorDataHandleMock(_memHandle));
  }

  void tearDown() { _memHandle.clear(); }

  std::string itinNumFromFarePath(FarePath* fp)
  {
    std::string strItinNum("");
    if((fp->itin() != nullptr) && (_trx->getTrxType() == PricingTrx::MIP_TRX))
    {
        strItinNum += "ITIN-NO: " + std::to_string(fp->itin()->itinNum()) +  "\n";
    }
    return strItinNum;
  }

  void testDisplayFarePathHeader()
  {
    initTrx();
    _collector->displayFarePathHeader(*_farePath);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
    expectedResult += itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforCat35NotApplicable()
  {
    initTrx();
    _collector->printAMCforCat35NotApplicable(*_farePath);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
    expectedResult +=  itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n"
                               "AMC IS NOT APPLICABLE\n"
                               "REASON: CAT 35 NET TICKETING OR CAT 35 NET REMIT\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforCat35NotApplicable_PaxTypeNotMatch()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));

    _collector->printAMCforCat35NotApplicable(*_farePath);
    std::string expectedResult("");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforASNotApplicable()
  {
    initTrx();
    _collector->printAMCforASNotApplicable(*_farePath);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
     expectedResult += itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n"
                               "AMC IS NOT APPLICABLE\n"
                               "REASON: AIRLINE SOLUTION CUSTOMER\n";
     CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforASNotApplicable_PaxTypeNotMatch()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));
    _collector->printAMCforASNotApplicable(*_farePath);
    std::string expectedResult("");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCCommandNotSupported()
  {
    initTrx();
    _collector->printAMCCommandNotSupported(*_farePath);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
    expectedResult += itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n"
                               "AMC IS NOT APPLICABLE\n"
                               "REASON: COMMAND NOT SUPPORTED\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCCommandNotSupported_PaxTypeNotMatch()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));
    _collector->printAMCCommandNotSupported(*_farePath);
    std::string expectedResult("");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintNoSecurityHandShakeForPCC()
  {
    initTrx();
    PseudoCityCode pcc = "80K2";
    _collector->printNoSecurityHandShakeForPCC(*_farePath, pcc);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
     expectedResult += itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n"
                               "AMC IS NOT APPLICABLE\n"
                               "REASON:  PCC 80K2 DOES NOT HAVE SECURITY HANDSHAKE\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintNoSecurityHandShakeForPCC_PaxTypeNotMatch()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));
    PseudoCityCode pcc = "80K2";
    _collector->printNoSecurityHandShakeForPCC(*_farePath, pcc);
    std::string expectedResult("");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintNoCommissionContractFound()
  {
    initTrx();
    CarrierCode cxr = "AA ";
    PseudoCityCode pcc = "80K2";
    _collector->printContractNotFound(cxr, pcc);
    std::string expectedResult("AMC IS NOT APPLICABLE FOR CARRIER CODE AA  / PCC 80K2\n"
                               "REASON: NO CONTRACT FOUND\n"
                               " \n"
                               "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforKPandEPRNotApplicable()
  {
    initTrx();
    _collector->printAMCforKPandEPRNotApplicable(*_farePath);
    std::string expectedResult("***************************************************************\n"
                               "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n"
                               "***************************************************************\n");
     expectedResult += itinNumFromFarePath(_farePath) +
                               "                   ** REQUESTED PTC: ADT **\n"
                               "AMC IS NOT APPLICABLE\n"
                               "REASON: AGENT OVERRIDE INPUT WITH KEYWORD COMOVR\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintAMCforKPandEPRNotApplicable_PaxTypeNotMatch()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));
    _collector->printAMCforKPandEPRNotApplicable(*_farePath);
    std::string expectedResult("");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPaxTypeMatched_Pass()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "ADT"));
    CPPUNIT_ASSERT(_collector->paxTypeMatched(*_farePath));
  }

  void testPaxTypeMatched_Fail()
  {
    initTrx();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("PT", "SRR"));
    CPPUNIT_ASSERT(!_collector->paxTypeMatched(*_farePath));
  }

  void testPrintCommissionContractInfoLong()
  {
    initTrx();
    createCommissionContractInfo();
    _collector->printCommissionContractInfo(*_cci);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "CONTRACT CARRIER CODE: AA \n"
                               "CONTRACT ID          : 88\n"
                               "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
                               "CONTRACT VENDOR      : COS\n"
                               "EFFECTIVE DATE TIME  : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME    : 11-NOV-2016  23.45.12\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintFpCurrencies_FP_PaymentCurrency()
  {
    initTrx();
    createCommissionContractInfo();
    _farePath->baseFareCurrency() = "LOL";
    _farePath->calculationCurrency() = "COS";
    _collector->printFpCurrencies(*_farePath);
    std::string expectedResult("BASE FARE CURRENCY   : LOL\n"
                               "PAYMENT CURRENCY     : AMC\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintFpCurrencies_ITIN_PaymentCurrency()
  {
    initTrx();
    createCommissionContractInfo();
    _farePath->baseFareCurrency() = "LOL";
    _farePath->itin()->calculationCurrency() = "AMC";
    _collector->printFpCurrencies(*_farePath);
    std::string expectedResult("BASE FARE CURRENCY   : LOL\n"
                               "PAYMENT CURRENCY     : AMC\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionContractInfoShort()
  {
    initTrx();
    createCommissionContractInfo();
    _collector->printCommissionContractInfoShort(*_cci);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "CONTRACT CARRIER CODE: AA \n"
                               "CONTRACT ID          : 88\n"
                               "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
                               " \n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsRuleInfoType12PASS()
  {
    initTrx();
    createCommissionRuleInfo();
    CommissionValidationStatus rc = SKIP_CR;
    _collector->printCommissionsRuleInfo(*_cri, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "VALIDATION RESULT: SKIP\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION RULE DATA---\n"
                               "VENDOR: COS    PROGRAM ID: 45        \n"
                               "COMMISSION TYPE          : 12 - SEGMENT BONUS\n"
                               "COMMISSION AMOUNT        : 18.00 USD\n"
                               "EFFECTIVE DATE TIME      : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME        : 11-NOV-2016  23.45.12\n"
                               "PASSENGER TYPE REQUIRED  : CNN SRR \n"
                               "FBC STARTS WITH REQUIRED : F A Q \n"
                               "                EXCLUDED : X \n"
                               "FBC FRAGMENTS REQUIRED   : %Y% %10 \n"
                               "              EXCLUDED   : %FR12 \n"
                               "TKT DESIGNATOR REQUIRED  : ID10 ID80 \n"
                               "               EXCLUDED  : ID55 \n"
                               "CABIN TYPE REQUIRED      : J R W \n"
                               "           EXCLUDED      : Y \n"
                               "CLASS OF SERVICE REQUIRED: A  Z  R  \n"
                               "                 EXCLUDED: Q  \n"
                               "MARKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "OPERATING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "TICKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "MKT GOV CXR REQUIRED     : AA  BA  \n"
                               "            EXCLUDED     : SQ  CX  \n"
                               "OPER GOV CXR REQUIRED    : AA  BA  \n"
                               "             EXCLUDED    : SQ  CX  \n"
                               "CONNECT POINT REQUIRED   : ANY \n"
                               "              EXCLUDED   : KKK MMM \n"
                               "NON-STOP REQUIRED        : KKKMMM \n"
                               "INTERLINE REQUIRED       : Y\n"
                               "ROUND TRIP               : R - REQUIRED\n"
                               "FARE AMOUNT MINIMUM      : 100.00 USD\n"
                               "            MAXIMUM      : 2000.00 USD\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsRuleInfoType_9_FAIL_MATCH_EXCL()
  {
    initTrx();
    createCommissionRuleInfo();
    _cri->commissionTypeId() = 9;

    CommissionValidationStatus rc = FAIL_CR_FBC_FRAGMENT_EXCL;
    _collector->printCommissionsRuleInfo(*_cri, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "VALIDATION RESULT: FAIL - MATCH EXCL FARE BASIS FRAGMENT\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION RULE DATA---\n"
                               "VENDOR: COS    PROGRAM ID: 45        \n"
                               "COMMISSION TYPE          : 9 - NO PRORATION ALLOWED\n"
                               "COMMISSION PERCENT       : 18.00%\n"
                               "EFFECTIVE DATE TIME      : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME        : 11-NOV-2016  23.45.12\n"
                               "PASSENGER TYPE REQUIRED  : CNN SRR \n"
                               "FBC STARTS WITH REQUIRED : F A Q \n"
                               "                EXCLUDED : X \n"
                               "FBC FRAGMENTS REQUIRED   : %Y% %10 \n"
                               "              EXCLUDED   : %FR12 \n"
                               "TKT DESIGNATOR REQUIRED  : ID10 ID80 \n"
                               "               EXCLUDED  : ID55 \n"
                               "CABIN TYPE REQUIRED      : J R W \n"
                               "           EXCLUDED      : Y \n"
                               "CLASS OF SERVICE REQUIRED: A  Z  R  \n"
                               "                 EXCLUDED: Q  \n"
                               "MARKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "OPERATING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "TICKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "MKT GOV CXR REQUIRED     : AA  BA  \n"
                               "            EXCLUDED     : SQ  CX  \n"
                               "OPER GOV CXR REQUIRED    : AA  BA  \n"
                               "             EXCLUDED    : SQ  CX  \n"
                               "CONNECT POINT REQUIRED   : ANY \n"
                               "              EXCLUDED   : KKK MMM \n"
                               "NON-STOP REQUIRED        : KKKMMM \n"
                               "INTERLINE REQUIRED       : Y\n"
                               "ROUND TRIP               : R - REQUIRED\n"
                               "FARE AMOUNT MINIMUM      : 100.00 USD\n"
                               "            MAXIMUM      : 2000.00 USD\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void printCommissionsRuleInfoShortType12Pass()
  {
    initTrx();
    createCommissionRuleInfo();
    CommissionValidationStatus rc = SKIP_CR;
    _collector->printCommissionRuleShortHeader();
    _collector->printCommissionsRuleInfoShort(*_cri, rc);
    std::string expectedResult("  COMMISSION ID  TYPE  VALUE        STATUS\n"
                               "  321            12    18.00 USD    SKIP\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void printCommissionsRuleInfoShortType9_FAIL_InterlineCONNECTION()
  {
    initTrx();
    createCommissionRuleInfo();
    _cri->commissionTypeId() = 9;
    CommissionValidationStatus rc = FAIL_CR_INTERLINE_CONNECTION;
    _collector->printCommissionRuleShortHeader();
    _collector->printCommissionsRuleInfoShort(*_cri, rc);
    std::string expectedResult("  COMMISSION ID  TYPE  VALUE        STATUS\n"
                               "  321            9     18.00%       FAIL - INTERLINE CONNECTION\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsRuleInfo_with_overflow_vector_data()
  {
    initTrx();
    createCommissionRuleInfo();
    _cri->commissionTypeId() = 9;

    char ch = 'Z';
    for(int i = 0; i < 70; ++i)
    _cri->fareBasisCodeIncl().emplace_back(ch);

    std::string fbFragment = "%YRT%FOROS%";
    for(int i = 0; i < 10; ++i)
    _cri->fbcFragmentIncl().emplace_back(fbFragment);

    PaxTypeCode pCode = "SRR";
    BookingCode bk = "A ";
    CarrierCode cxr = "CX ";
    LocCode loc = "MMM";
    TktDesignator tktD = "ID100";
    for(int i = 0; i < 25; ++i)
    {
      _cri->requiredPaxType().emplace_back(pCode);
      _cri->classOfServiceIncl().emplace_back(bk);
      _cri->marketingCarrierIncl().emplace_back(cxr);
      _cri->excludedCnxAirPCodes().emplace_back(loc);
      _cri->requiredNonStop().emplace_back(loc, loc);
      _cri->requiredTktDesig().emplace_back(tktD);
    }

    CommissionValidationStatus rc = FAIL_CR_FBC_FRAGMENT_EXCL;
    _collector->printCommissionsRuleInfo(*_cri, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "VALIDATION RESULT: FAIL - MATCH EXCL FARE BASIS FRAGMENT\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION RULE DATA---\n"
                               "VENDOR: COS    PROGRAM ID: 45        \n"
                               "COMMISSION TYPE          : 9 - NO PRORATION ALLOWED\n"
                               "COMMISSION PERCENT       : 18.00%\n"
                               "EFFECTIVE DATE TIME      : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME        : 11-NOV-2016  23.45.12\n"
                               "PASSENGER TYPE REQUIRED  : CNN SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR \n"
                               "FBC STARTS WITH REQUIRED : F A Q Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z \n"
                               "                EXCLUDED : X \n"
                               "FBC FRAGMENTS REQUIRED   : %Y% %10 %YRT%FOROS% %YRT%FOROS% \n"
                               "                           %YRT%FOROS% %YRT%FOROS% \n"
                               "                           %YRT%FOROS% %YRT%FOROS% \n"
                               "                           %YRT%FOROS% %YRT%FOROS% \n"
                               "                           %YRT%FOROS% %YRT%FOROS% \n"
                               "              EXCLUDED   : %FR12 \n"
                               "TKT DESIGNATOR REQUIRED  : ID10 ID80 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 \n"
                               "               EXCLUDED  : ID55 \n"
                               "CABIN TYPE REQUIRED      : J R W \n"
                               "           EXCLUDED      : Y \n"
                               "CLASS OF SERVICE REQUIRED: A  Z  R  A  A  A  A  A  A  A  A  \n"
                               "                           A  A  A  A  A  A  A  A  A  A  A  \n"
                               "                           A  A  A  A  A  A  \n"
                               "                 EXCLUDED: Q  \n"
                               "MARKETING CXR REQUIRED   : AA  BA  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "OPERATING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "TICKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "MKT GOV CXR REQUIRED     : AA  BA  \n"
                               "            EXCLUDED     : SQ  CX  \n"
                               "OPER GOV CXR REQUIRED    : AA  BA  \n"
                               "             EXCLUDED    : SQ  CX  \n"
                               "CONNECT POINT REQUIRED   : ANY \n"
                               "              EXCLUDED   : KKK MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM \n"
                               "NON-STOP REQUIRED        : KKKMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM \n"
                               "INTERLINE REQUIRED       : Y\n"
                               "ROUND TRIP               : R - REQUIRED\n"
                               "FARE AMOUNT MINIMUM      : 100.00 USD\n"
                               "            MAXIMUM      : 2000.00 USD\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsRuleInfo_with_NonStop_overflow_vector_data()
  {
    initTrx();
    createCommissionRuleInfo();
    _cri->commissionTypeId() = 9;

    _cri->requiredPaxType().clear();
    _cri->fareBasisCodeIncl().clear();
    _cri->fareBasisCodeExcl().clear();
    _cri->fbcFragmentIncl().clear();
    _cri->fbcFragmentExcl().clear();
    _cri->requiredTktDesig().clear();
    _cri->excludedTktDesig().clear();
    _cri->requiredCabinType().clear();
    _cri->excludedCabinType().clear();
    _cri->classOfServiceIncl().clear();
    _cri->classOfServiceExcl().clear();
    _cri->marketingCarrierIncl().clear();
    _cri->marketingCarrierExcl().clear();
    _cri->operatingCarrierIncl().clear();
    _cri->operatingCarrierExcl().clear();
    _cri->ticketingCarrierIncl().clear();
    _cri->ticketingCarrierExcl().clear();
    _cri->requiredMktGovCxr().clear();
    _cri->excludedMktGovCxr().clear();
    _cri->requiredOperGovCxr().clear();
    _cri->excludedOperGovCxr().clear();
    _cri->requiredCnxAirPCodes().clear();
    _cri->excludedCnxAirPCodes().clear();

    LocCode loc = "MMM";
    for(int i = 0; i < 25; ++i)
    {
      _cri->requiredNonStop().emplace_back(loc, loc);
    }

    CommissionValidationStatus rc = FAIL_CR_FBC_FRAGMENT_EXCL;
    _collector->printCommissionsRuleInfo(*_cri, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "VALIDATION RESULT: FAIL - MATCH EXCL FARE BASIS FRAGMENT\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION RULE DATA---\n"
                               "VENDOR: COS    PROGRAM ID: 45        \n"
                               "COMMISSION TYPE          : 9 - NO PRORATION ALLOWED\n"
                               "COMMISSION PERCENT       : 18.00%\n"
                               "EFFECTIVE DATE TIME      : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME        : 11-NOV-2016  23.45.12\n"
                               "PASSENGER TYPE REQUIRED  : \n"
                               "FBC STARTS WITH REQUIRED : \n"
                               "                EXCLUDED : \n"
                               "FBC FRAGMENTS REQUIRED   : \n"
                               "              EXCLUDED   : \n"
                               "TKT DESIGNATOR REQUIRED  : \n"
                               "               EXCLUDED  : \n"
                               "CABIN TYPE REQUIRED      : \n"
                               "           EXCLUDED      : \n"
                               "CLASS OF SERVICE REQUIRED: \n"
                               "                 EXCLUDED: \n"
                               "MARKETING CXR REQUIRED   : \n"
                               "              EXCLUDED   : \n"
                               "OPERATING CXR REQUIRED   : \n"
                               "              EXCLUDED   : \n"
                               "TICKETING CXR REQUIRED   : \n"
                               "              EXCLUDED   : \n"
                               "MKT GOV CXR REQUIRED     : \n"
                               "            EXCLUDED     : \n"
                               "OPER GOV CXR REQUIRED    : \n"
                               "             EXCLUDED    : \n"
                               "CONNECT POINT REQUIRED   : \n"
                               "              EXCLUDED   : \n"
                               "NON-STOP REQUIRED        : KKKMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM \n"
                               "INTERLINE REQUIRED       : Y\n"
                               "ROUND TRIP               : R - REQUIRED\n"
                               "FARE AMOUNT MINIMUM      : 100.00 USD\n"
                               "            MAXIMUM      : 2000.00 USD\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsRuleInfo_with_overflow_vector_data_GreenScreen()
  {
    initTrx();
    _trx->billing()->requestPath() = PSS_PO_ATSE_PATH;
    createCommissionRuleInfo();
    _cri->commissionTypeId() = 9;

    char ch = 'Z';
    for(int i = 0; i < 70; ++i)
    _cri->fareBasisCodeIncl().emplace_back(ch);

    std::string fbFragment = "%YRT%FOROS%";
    for(int i = 0; i < 10; ++i)
    _cri->fbcFragmentIncl().emplace_back(fbFragment);

    PaxTypeCode pCode = "SRR";
    BookingCode bk = "A ";
    CarrierCode cxr = "CX ";
    LocCode loc = "MMM";
    TktDesignator tktD = "ID100";
    for(int i = 0; i < 25; ++i)
    {
      _cri->requiredPaxType().emplace_back(pCode);
      _cri->classOfServiceIncl().emplace_back(bk);
      _cri->marketingCarrierIncl().emplace_back(cxr);
      _cri->excludedCnxAirPCodes().emplace_back(loc);
      _cri->requiredNonStop().emplace_back(loc, loc);
      _cri->requiredTktDesig().emplace_back(tktD);
    }
    _cri->fareAmountMin() = -1;
    CommissionValidationStatus rc = FAIL_CR_FBC_FRAGMENT_EXCL;
    _collector->printCommissionsRuleInfo(*_cri, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "VALIDATION RESULT: FAIL - MATCH EXCL FARE BASIS FRAGMENT\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION RULE DATA---\n"
                               "VENDOR: COS    PROGRAM ID: 45        \n"
                               "COMMISSION TYPE          : 9 - NO PRORATION ALLOWED\n"
                               "COMMISSION PERCENT       : 18.00\n"
                               "EFFECTIVE DATE TIME      : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME        : 11-NOV-2016  23.45.12\n"
                               "PASSENGER TYPE REQUIRED  : CNN SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR SRR SRR SRR SRR SRR \n"
                               "                           SRR SRR SRR \n"
                               "FBC STARTS WITH REQUIRED : F A Q Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z Z \n"
                               "                           Z Z Z Z Z \n"
                               "                EXCLUDED : X \n"
                               "FBC FRAGMENTS REQUIRED   : =Y= =10 =YRT=FOROS= =YRT=FOROS= \n"
                               "                           =YRT=FOROS= =YRT=FOROS= \n"
                               "                           =YRT=FOROS= =YRT=FOROS= \n"
                               "                           =YRT=FOROS= =YRT=FOROS= \n"
                               "                           =YRT=FOROS= =YRT=FOROS= \n"
                               "              EXCLUDED   : =FR12 \n"
                               "TKT DESIGNATOR REQUIRED  : ID10 ID80 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 ID100 ID100 ID100 ID100 \n"
                               "                           ID100 \n"
                               "               EXCLUDED  : ID55 \n"
                               "CABIN TYPE REQUIRED      : J R W \n"
                               "           EXCLUDED      : Y \n"
                               "CLASS OF SERVICE REQUIRED: A  Z  R  A  A  A  A  A  A  A  A  \n"
                               "                           A  A  A  A  A  A  A  A  A  A  A  \n"
                               "                           A  A  A  A  A  A  \n"
                               "                 EXCLUDED: Q  \n"
                               "MARKETING CXR REQUIRED   : AA  BA  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  CX  CX  CX  CX  CX  \n"
                               "                           CX  CX  CX  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "OPERATING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "TICKETING CXR REQUIRED   : AA  BA  \n"
                               "              EXCLUDED   : SQ  CX  \n"
                               "MKT GOV CXR REQUIRED     : AA  BA  \n"
                               "            EXCLUDED     : SQ  CX  \n"
                               "OPER GOV CXR REQUIRED    : AA  BA  \n"
                               "             EXCLUDED    : SQ  CX  \n"
                               "CONNECT POINT REQUIRED   : ANY \n"
                               "              EXCLUDED   : KKK MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM MMM MMM MMM MMM MMM \n"
                               "                           MMM MMM MMM \n"
                               "NON-STOP REQUIRED        : KKKMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM MMMMMM MMMMMM MMMMMM MMMMMM \n"
                               "                           MMMMMM \n"
                               "INTERLINE REQUIRED       : Y\n"
                               "ROUND TRIP               : R - REQUIRED\n"
                               "FARE AMOUNT MINIMUM      : \n"
                               "            MAXIMUM      : 2000.00 USD\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void createCommissionRuleInfo()
 {
    _memHandle.get(_cri);
    _cri->vendor() = "COS";
    _cri->commissionId() = 321;
    _cri->effDate() = DateTime(2015, 11, 11, 15, 30, 44);
    _cri->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    _cri->programId() = 45;
    _cri->description() = "COS TESTING";
    _cri->currency() = "USD";
    _cri->commissionValue() = 18.00;
    _cri->commissionTypeId() = 12;
    PaxTypeCode pCode1 = "CNN";
    PaxTypeCode pCode2 = "SRR";
    _cri->requiredPaxType().emplace_back(pCode1);
    _cri->requiredPaxType().emplace_back(pCode2);
    char ch1 = 'F';
    char ch2 = 'A';
    char ch3 = 'Q';
    _cri->fareBasisCodeIncl().emplace_back(ch1);
    _cri->fareBasisCodeIncl().emplace_back(ch2);
    _cri->fareBasisCodeIncl().emplace_back(ch3);
    char ch4 = 'X';
    _cri->fareBasisCodeExcl().emplace_back(ch4);
    std::string fbFragment1 = "%Y%";
    std::string fbFragment2 = "%10";
    _cri->fbcFragmentIncl().emplace_back(fbFragment1);
    _cri->fbcFragmentIncl().emplace_back(fbFragment2);
    std::string fbFragment3 = "%FR12";
    _cri->fbcFragmentExcl().emplace_back(fbFragment3);
    TktDesignator tktD1 = "ID10";
    TktDesignator tktD2 = "ID80";
    TktDesignator tktD3 = "ID55";
    _cri->requiredTktDesig().emplace_back(tktD1);
    _cri->requiredTktDesig().emplace_back(tktD2);
    _cri->excludedTktDesig().emplace_back(tktD3);
    char ch11 = 'J';
    char ch12 = 'R';
    char ch13 = 'W';
    _cri->requiredCabinType().emplace_back(ch11);
    _cri->requiredCabinType().emplace_back(ch12);
    _cri->requiredCabinType().emplace_back(ch13);
    char ch14 = 'Y';
    _cri->excludedCabinType().emplace_back(ch14);
    BookingCode bk1 = "A ";
    BookingCode bk2 = "Z ";
    BookingCode bk3 = "R ";
    BookingCode bk4 = "Q ";
    _cri->classOfServiceIncl().emplace_back(bk1);
    _cri->classOfServiceIncl().emplace_back(bk2);
    _cri->classOfServiceIncl().emplace_back(bk3);
    _cri->classOfServiceExcl().emplace_back(bk4);
    CarrierCode cxr1 = "AA ";
    CarrierCode cxr2 = "BA ";
    CarrierCode cxr3 = "SQ ";
    CarrierCode cxr4 = "CX ";
    _cri->marketingCarrierIncl().emplace_back(cxr1);
    _cri->marketingCarrierIncl().emplace_back(cxr2);
    _cri->marketingCarrierExcl().emplace_back(cxr3);
    _cri->marketingCarrierExcl().emplace_back(cxr4);
    _cri->operatingCarrierIncl().emplace_back(cxr1);
    _cri->operatingCarrierIncl().emplace_back(cxr2);
    _cri->operatingCarrierExcl().emplace_back(cxr3);
    _cri->operatingCarrierExcl().emplace_back(cxr4);
    _cri->ticketingCarrierIncl().emplace_back(cxr1);
    _cri->ticketingCarrierIncl().emplace_back(cxr2);
    _cri->ticketingCarrierExcl().emplace_back(cxr3);
    _cri->ticketingCarrierExcl().emplace_back(cxr4);
    _cri->requiredMktGovCxr().emplace_back(cxr1);
    _cri->requiredMktGovCxr().emplace_back(cxr2);
    _cri->excludedMktGovCxr().emplace_back(cxr3);
    _cri->excludedMktGovCxr().emplace_back(cxr4);
    _cri->requiredOperGovCxr().emplace_back(cxr1);
    _cri->requiredOperGovCxr().emplace_back(cxr2);
    _cri->excludedOperGovCxr().emplace_back(cxr3);
    _cri->excludedOperGovCxr().emplace_back(cxr4);

    LocCode any = "ANY";
    LocCode loc1 = "KKK";
    LocCode loc2 = "MMM";
    _cri->requiredCnxAirPCodes().emplace_back(any);
    _cri->excludedCnxAirPCodes().emplace_back(loc1);
    _cri->excludedCnxAirPCodes().emplace_back(loc2);

    _cri->requiredNonStop().emplace_back(loc1, loc2);
    _cri->interlineConnectionRequired() = 'Y';
    _cri->roundTrip() = 'R';

    _cri->fareAmountMin() = 100.00;
    _cri->fareAmountCurrency() = "USD";
    _cri->fareAmountMax() = 2000.00;
  }


  void testPrintCommissionsProgramInfo_with_Limited_data()
  {
    initTrx();
    createCommissionProgramInfo();

    CommissionValidationStatus rc = PASS_CR;
    _collector->printCommissionsProgramInfo(*_cpi, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "PROGRAM ID: 45909\n"
                               "PROGRAM NAME: COMMISSION PROGRAM TESTING                       \n"
                               "VALIDATION RESULT: PASS\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION PROGRAM DATA---\n"
                               "VENDOR: COS  CONTRACT ID: 88\n"
                               "EFFECTIVE DATE TIME     : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME       : 11-NOV-2016  23.45.12\n"
                               "VALID POINTS OF SALE ITEM NO  : 2121390\n"
                               "VALID POINTS OF ORIGIN ITEM NO: 18009900\n"
                               "VALID MARKET ITEM NO : 12\n"
                               "TRAVEL DATES ITEM NO    : 4\n"
                               "START TICKETING DATE    : 11-NOV-2012  15.30.44\n"
                               "END TICKETING DATE      : N/A\n"
                               "SURCHARGE INDICATOR     : Y\n"
                               "THROUGH FARE INDICATOR  : N\n"
                               "MAX CONNECTION TIME     : 2400\n"
                               "LAND AGREEMENT INDICATOR:  \n \n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsProgramInfo_with_PointOfSaleAndOrigin_data()
  {
    initTrx();
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    CommissionValidationStatus rc = PASS_CR;
    _collector->printCommissionsProgramInfo(*prog, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "PROGRAM ID: 45909\n"
                               "PROGRAM NAME: COMMISSION PROGRAM TESTING                       \n"
                               "VALIDATION RESULT: PASS\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION PROGRAM DATA---\n"
                               "VENDOR: COS  CONTRACT ID: 88\n"
                               "EFFECTIVE DATE TIME     : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME       : 11-NOV-2016  23.45.12\n"
                               "VALID POINTS OF SALE ITEM NO  : 2121390\n"
                               "  ORDER NO          LOC       INCL/EXCL\n"
                               "  33333             N-AZ        INCL\n"
                               "  44444             C-FRA       EXCL\n"
                               "VALID POINTS OF ORIGIN ITEM NO: 18009900\n"
                               "  ORDER NO          LOC       INCL/EXCL\n"
                               "  33333             N-AZ        INCL\n"
                               "  555               C-FRA       INCL\n"
                               "VALID MARKET ITEM NO : 12\n"
                               "  ORDER NO    ORIG      DEST      BI-DIRECTIONAL INCL/EXCL\n"
                               "  818181      N-AZ      C-FRA           YES        INCL\n"
                               "TRAVEL DATES ITEM NO    : 4\n"
                               "  START TRAVEL DATE     : 11-NOV-2012  15.30.44\n"
                               "  END TRAVEL DATE       : N/A\n"
                               "START TICKETING DATE    : 11-NOV-2015  15.30.44\n"
                               "END TICKETING DATE      : N/A\n"
                               "SURCHARGE INDICATOR     : Y\n"
                               "THROUGH FARE INDICATOR  : N\n"
                               "MAX CONNECTION TIME     : 2400\n"
                               "LAND AGREEMENT INDICATOR: Y\n \n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionsProgramInfo_with_PointOfSaleAndOrigin_data_FAIL()
  {
    initTrx();
    LocKey lk1, lk2;
    populateLocs(lk1, lk2);
    CommissionProgramInfo* prog(new CommissionProgramInfo);
    createCommissionProgramInfo1(prog, lk1, lk2);
    CommissionValidationStatus rc = FAIL_CP_POINT_OF_ORIGIN;
    _collector->printCommissionsProgramInfo(*prog, rc);
    std::string expectedResult("*-------------------------------------------------------------*\n"
                               "PROGRAM ID: 45909\n"
                               "PROGRAM NAME: COMMISSION PROGRAM TESTING                       \n"
                               "VALIDATION RESULT: FAIL - NOT MATCH POINT OF ORIGIN\n"
                               "*-------------------------------------------------------------*\n"
                               "---COMMISSION PROGRAM DATA---\n"
                               "VENDOR: COS  CONTRACT ID: 88\n"
                               "EFFECTIVE DATE TIME     : 11-NOV-2015  15.30.44\n"
                               "EXPIRED DATE TIME       : 11-NOV-2016  23.45.12\n"
                               "VALID POINTS OF SALE ITEM NO  : 2121390\n"
                               "  ORDER NO          LOC       INCL/EXCL\n"
                               "  33333             N-AZ        INCL\n"
                               "  44444             C-FRA       EXCL\n"
                               "VALID POINTS OF ORIGIN ITEM NO: 18009900\n"
                               "  ORDER NO          LOC       INCL/EXCL\n"
                               "  33333             N-AZ        INCL\n"
                               "  555               C-FRA       INCL\n"
                               "VALID MARKET ITEM NO : 12\n"
                               "  ORDER NO    ORIG      DEST      BI-DIRECTIONAL INCL/EXCL\n"
                               "  818181      N-AZ      C-FRA           YES        INCL\n"
                               "TRAVEL DATES ITEM NO    : 4\n"
                               "  START TRAVEL DATE     : 11-NOV-2012  15.30.44\n"
                               "  END TRAVEL DATE       : N/A\n"
                               "START TICKETING DATE    : 11-NOV-2015  15.30.44\n"
                               "END TICKETING DATE      : N/A\n"
                               "SURCHARGE INDICATOR     : Y\n"
                               "THROUGH FARE INDICATOR  : N\n"
                               "MAX CONNECTION TIME     : 2400\n"
                               "LAND AGREEMENT INDICATOR: Y\n \n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintShortCommissionsProgramInfo_PASS()
  {
    initTrx();
    createCommissionProgramInfo();
    CommissionValidationStatus rc = PASS_CR;
    _collector->printCommissionsProgramInfoShort(*_cpi, rc);
    std::string expectedResult("PROGRAM ID: 45909       PASS      SURCHARGE ALLOWED-Y\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintShortCommissionsProgramInfo_FAIL()
  {
    initTrx();
    createCommissionProgramInfo();
    CommissionValidationStatus rc = FAIL_CP_POINT_OF_ORIGIN;
    _collector->printCommissionsProgramInfoShort(*_cpi, rc);
    std::string expectedResult("PROGRAM ID: 45909       FAIL - POINT OF ORIGIN\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void createCommissionContractInfo()
  {
    _memHandle.get(_cci);
    _cci->vendor() = "COS";
    _cci->effDateTime() = DateTime(2015, 11, 11, 15, 30, 44);
    _cci->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    _cci->description() = "COMMISSION Contract TESTING";
    _cci->contractId() = 88;
    _cci->carrier() = "AA ";
  }

  void createCommissionProgramInfo()
  {
    _memHandle.get(_cpi);
    _cpi->vendor() = "COS";
    _cpi->effDate() = DateTime(2015, 11, 11, 15, 30, 44);
    _cpi->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    _cpi->programId() = 45909;
    _cpi->programName() = "COMMISSION PROGRAM TESTING";
    _cpi->pointOfSaleItemNo() = 2121390;
    _cpi->pointOfOriginItemNo() = 18009900;
    _cpi->travelDatesItemNo() = 4;
    _cpi->marketItemNo() = 12;
    _cpi->contractId() = 88;
    _cpi->maxConnectionTime() = 2400;

    _cpi->startTktDate() = DateTime(2012, 11, 11, 15, 30, 44);
    _cpi->qSurchargeInd() = 'Y';
    _cpi->throughFareInd() = 'N';
  }

  void createCommissionProgramInfo1(CommissionProgramInfo* prog, LocKey& lk1, LocKey& lk2)
  {
    CommissionProgramInfo::dummyData(*prog);
    prog->vendor() = "COS";
    prog->effDate() = DateTime(2015, 11, 11, 15, 30, 44);
    prog->expireDate() = DateTime(2016, 11, 11, 23, 45, 12);
    prog->programId() = 45909;
    prog->programName() = "COMMISSION PROGRAM TESTING";
    prog->pointOfSaleItemNo() = 2121390;
    prog->pointOfOriginItemNo() = 18009900;
    prog->travelDatesItemNo() = 4;
    prog->marketItemNo() = 12;
    prog->contractId() = 88;
    prog->maxConnectionTime() = 2400;

    prog->startTktDate() = DateTime(2015, 11, 11, 15, 30, 44);
    prog->endTktDate() = DateTime::emptyDate();
    prog->landAgreementInd() = 'Y';
    prog->qSurchargeInd() = 'Y';
    prog->throughFareInd() = 'N';

    CommissionLocSegInfo* loc1 = prog->pointOfSale()[0];
    loc1->vendor() = "COS";
    loc1->orderNo() = 33333;
    loc1->inclExclInd() = 'I';
    loc1->loc() = lk1;

    CommissionLocSegInfo* loc2 = prog->pointOfSale()[1];
    loc2->vendor() = "COS";
    loc2->orderNo() = 44444;
    loc2->inclExclInd() = 'E';
    loc2->loc() = lk2;

    CommissionLocSegInfo* origin1 = prog->pointOfOrigin()[0];
    origin1->vendor() = "COS";
    origin1->orderNo() = 33333;
    origin1->inclExclInd() = 'I';
    origin1->loc() = lk1;
    CommissionLocSegInfo* origin2 = prog->pointOfOrigin()[1];
    origin2->vendor() = "COS";
    origin2->orderNo() = 555;
    origin2->inclExclInd() = 'I';
    origin2->loc() = lk2;

    CommissionTravelDatesSegInfo* travelDate1 = prog->travelDates()[0];
    travelDate1->firstTravelDate() = DateTime(2012, 11, 11, 15, 30, 44);
    travelDate1->endTravelDate() = DateTime::emptyDate();

    CommissionMarketSegInfo* market = prog->markets()[0];
    market->orderNo() = 818181;
    market->origin() = lk1;
    market->destination() = lk2;
    market->inclExclInd() = 'I';
    market->bidirectional() = 'Y';

  }

  void populateLocs(LocKey& lk1, LocKey& lk2)
  {
    lk1.loc() = "AZ ";
    lk1.locType() = 'N';
    lk2.loc() = "FRA";
    lk2.locType() = 'C';
  }

  void testPrintCommissionFCHeader()
  {
    initTrx();
    _collector->printCommissionFCHeader();
    std::string expectedResult("***************************************************************\n"
                               "*      AGENCY COMMISSION ANALYSIS - FARE COMPONENT LEVEL      *\n"
                               "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintNoCommissionFound()
  {
    initTrx();
    _collector->printNoCommissionFound();
    std::string expectedResult("  NO AGENCY COMMISSION FOUND\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionNotProcessed()
  {
    initTrx();
    _collector->printCommissionNotProcessed();
    std::string expectedResult(" \n  AGENCY MANAGED COMMISSION NOT PROCESSED OR NOT FOUND\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintCommissionFC()
  {
    initTrx();

    MockPaxTypeFare paxTfare;
    _collector->printCommissionFC(paxTfare);
    std::string expectedResult(" DEN - BA - LON  Y\n"
                               "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintFinalAgencyCommission()
  {
    initTrx();
    MoneyAmount amt = 555.55;
    _collector->printFinalAgencyCommission(amt, true);
    std::string expectedResult("***************************************************************\n"
                               "*        FINAL AGENCY FARE COMMISSION - FARE PATH LEVEL       *\n"
                               "***************************************************************\n"
                               "   CALCULATED AGENCY COMMISSION AMOUNT: 555.55 AMC\n"
                               "***************************************************************\n");
    CPPUNIT_ASSERT_EQUAL(expectedResult, _collector->str());
  }

  void testPrintFinalFcCommission()
  {
    initTrx();
    MockPaxTypeFare paxTfare;
    paxTfare.nucFareAmount() = 2000.00;
    _fareUsage->paxTypeFare() = &paxTfare;
    prepareFareUsageAndFarePath();
    MoneyAmount amt = 555.55;
    createCommissionProgramInfo();
    createCommissionRuleInfo();
    createCommissionContractInfo();
    _cri->commissionTypeId() = 9;
    amc::CommissionRuleData crd(_cri, _cpi, _cci);

    _collectorM->printFinalFcCommission(*_farePath, *_fareUsage, amt, crd);
    std::string expectedResult("***************************************************************\n"
                               "*      AGENCY COMMISSION ANALYSIS - FARE COMPONENT LEVEL      *\n"
                               "***************************************************************\n"
                               " DEN - BA - LON  Y\n"
                               "***************************************************************\n"
                               "CONTRACT CARRIER CODE: AA \n"
                               "CONTRACT ID          : 88\n"
                               "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
                               " \n"
                               "PROGRAM ID: 45909\n"
                               "PROGRAM NAME: COMMISSION PROGRAM TESTING                       \n"
                               "SURCHARGE COMMISSION ALLOWED: YES\n"
                               " \n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "AGENCY COMMISSION TYPE             : 9 - NO PRORATION ALLOWED\n"
                               " \n"
                               "AGENCY COMMISSION PERCENT          : 18.00 \n"
                               " \n"
                               "TOTAL FARE AMOUNT                  : 2019.99   COS\n"
                               "CAT12 SURCHARGE                    : 19.99     COS\n"
                               " \n"
                               "FARE AMOUNT FOR COMMISSION CALCULATION\n"
                               "           IN CALCULATION CURRENCY : 2019.99   COS\n"
                               "           IN BASE FARE CURRENCY   : 2019.99   LOL\n"
                               "           IN PAYMENT CURRENCY     : 2019.99   AMC\n"
                               " \n"
                               "AGENCY COMMISSION APPLICATION\n"
                               "FARE AMOUNT * COMM PERCENT         : 555.55    AMC\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collectorM->str());
  }

  void testPrintFinalFcCommission_RollBack_Surcharges()
  {
    initTrx();
    MockPaxTypeFare paxTfare;
    paxTfare.nucFareAmount() = 2000.00;
    _fareUsage->paxTypeFare() = &paxTfare;
    prepareFareUsageAndFarePath();
    _farePath->rollBackSurcharges();
    MoneyAmount amt = 555.55;
    createCommissionProgramInfo();
    createCommissionRuleInfo();
    createCommissionContractInfo();
    _cri->commissionTypeId() = 9;
    amc::CommissionRuleData crd(_cri, _cpi, _cci);

    _collectorM->printFinalFcCommission(*_farePath, *_fareUsage, amt, crd);
    std::string expectedResult("***************************************************************\n"
                               "*      AGENCY COMMISSION ANALYSIS - FARE COMPONENT LEVEL      *\n"
                               "***************************************************************\n"
                               " DEN - BA - LON  Y\n"
                               "***************************************************************\n"
                               "CONTRACT CARRIER CODE: AA \n"
                               "CONTRACT ID          : 88\n"
                               "CONTRACT DESCRIPTION : COMMISSION CONTRACT TESTING              \n"
                               " \n"
                               "PROGRAM ID: 45909\n"
                               "PROGRAM NAME: COMMISSION PROGRAM TESTING                       \n"
                               "SURCHARGE COMMISSION ALLOWED: YES\n"
                               " \n"
                               "COMMISSION ID: 321\n"
                               "DESCRIPTION  : COS TESTING                                     \n"
                               "AGENCY COMMISSION TYPE             : 9 - NO PRORATION ALLOWED\n"
                               " \n"
                               "AGENCY COMMISSION PERCENT          : 18.00 \n"
                               " \n"
                               "TOTAL FARE AMOUNT                  : 2019.99   COS\n"
                               "CAT12 SURCHARGE                    : 20.00     COS\n"
                               " \n"
                               "FARE AMOUNT FOR COMMISSION CALCULATION\n"
                               "           IN CALCULATION CURRENCY : 2019.99   COS\n"
                               "           IN BASE FARE CURRENCY   : 2019.99   LOL\n"
                               "           IN PAYMENT CURRENCY     : 2019.99   AMC\n"
                               " \n"
                               "AGENCY COMMISSION APPLICATION\n"
                               "FARE AMOUNT * COMM PERCENT         : 555.55    AMC\n");

    CPPUNIT_ASSERT_EQUAL(expectedResult, _collectorM->str());
  }

  void prepareFareUsageAndFarePath()
  {
    _fareUsage->surchargeAmtUnconverted() = 20.00;
    _fareUsage->surchargeAmt() = 19.99;
    _farePath->baseFareCurrency() = "LOL";
    _farePath->calculationCurrency() = "COS";
  }

private:
  void initTrx()
  {
    Billing*   billing;
    PricingRequest* request;
    PricingOptions* options;
    Itin* itin;
    Agent* agent;
    PaxType* paxType;
    PricingUnit* pu;
    CollectedNegFareData* cNegFareData;

    _memHandle.get(_trx);
    _memHandle.get(request);
    _memHandle.get(billing);
    _memHandle.get(options);
    _memHandle.get(itin);
    _memHandle.get(_farePath);
    _memHandle.get(_fareUsage);
    _memHandle.get(agent);
    _memHandle.get(paxType);
    _memHandle.get(pu);
    _memHandle.get(cNegFareData);

    _collector->initTrx(*_trx);
    _collectorM->initTrx(*_trx);
    request->ticketingAgent() = agent;
    agent->currencyCodeAgent() = "AMC";
    _trx->setRequest(request);
    _trx->billing() = billing;
    billing->requestPath() = LIBERTY_PO_ATSE_PATH;
    _trx->setOptions(options);
    _trx->itin().push_back(itin);
    itin->farePath().push_back(_farePath);
    paxType->paxType() = "ADT";
    _farePath->itin() = itin;
    pu->fareUsage().push_back(_fareUsage);
    _farePath->pricingUnit().push_back(pu);
    _farePath->paxType() = paxType;
    _farePath->collectedNegFareData() = cNegFareData;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag867CollectorTest);
}
