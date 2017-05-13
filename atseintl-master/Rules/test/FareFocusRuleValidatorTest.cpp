#include "Common/Logger.h"
#include "DataModel/Billing.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag240Collector.h"
#include "Rules/FareFocusRuleValidator.h"
#include "Rules/RuleUtil.cpp"
#include "test/include/TestFallbackUtil.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <vector>

//#define PERFORMANCE_TEST
#ifdef PERFORMANCE_TEST
#define PERFORMANCE_ITERATIONS 10000
#endif

namespace tse
{
class SpecificTestConfigInitializer : public TestConfigInitializer
{
public:
  SpecificTestConfigInitializer()
  {
    DiskCache::initialize(_config);
    _memHandle.create<MockDataManager>();
  }

  ~SpecificTestConfigInitializer() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

namespace
{
class FareFocusRuleValidatorMock : public FareFocusRuleValidator
{
public:
  FareFocusRuleValidatorMock(PricingTrx& trx, FareMarket& fareMarket)
    : FareFocusRuleValidator(trx, fareMarket) {}
  ~FareFocusRuleValidatorMock() {}

  bool matchGeo(const FareFocusRuleInfo& ffri, const PaxTypeFare& paxTypeFare) const
  {
    if (!validGeoType(ffri))
      return false;

    const LocCode& origMarket =
          (paxTypeFare.isReversed()) ? paxTypeFare.fare()->market2() : paxTypeFare.fare()->market1();
    const LocCode& destMarket =
          (paxTypeFare.isReversed()) ? paxTypeFare.fare()->market1() : paxTypeFare.fare()->market2();

    return ((ffri.loc1().loc() == origMarket) && (ffri.loc2().loc() == destMarket));
  }

  bool isInLoc(const LocCode& validatingLoc, LocTypeCode ruleLocType,
           const LocCode& ruleLoc, const PaxTypeFare& paxTypeFare) const
  {
    return validatingLoc == ruleLoc;
  }

  bool getFareClassMatchExpressions(const uint64_t& fareClassItemNo,
                                    std::vector<std::string>& result) const
  {
    FareClassCodeC fareClassCode = "FF";
    if (RuleUtil::validateMatchExpression(fareClassCode))
      result.push_back(fareClassCode);

    return true;
  }

  std::string getFareBasis(const PaxTypeFare* ptf) const
  {
    std::string fareBasis = "FF";

    return fareBasis;
  }
};
}

class FareFocusRuleValidatorTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareFocusRuleValidatorTest);
  CPPUNIT_TEST(testCreateDiagnostic);
  CPPUNIT_TEST(testMatchFMForDiagnostic_True_No_Diagnostic);
  CPPUNIT_TEST(testMatchFMForDiagnostic_True_Diagnostic);
  CPPUNIT_TEST(testMatchFMForDiagnostic_False_Diagnostic);
  CPPUNIT_TEST(testPrintDiagAndReturn);
  CPPUNIT_TEST(testIsFareFocusApplicable);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_True_No_Diagnostic);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_True_Diagnostic);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_True_Diagnostic1);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_True_Diagnostic2);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_False_Diagnostic);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_False_Diagnostic1);
  CPPUNIT_TEST(testMatchPTFareForDiagnostic_False_Diagnostic2);
  CPPUNIT_TEST(testPrintDiagSecurityHShakeNotFound);
  CPPUNIT_TEST(testPrintDiagFareFocusRulesNotFound);
  CPPUNIT_TEST(testValidateSecurityHandshake_True);
  CPPUNIT_TEST(testValidateSecurityHandshake_False);
  CPPUNIT_TEST(testPrintFareFocusLookup_Diag);
  CPPUNIT_TEST(testprintFareFocusNoData_Diag);
  CPPUNIT_TEST(testMatchOWRT_True_Rule_Blank);
  CPPUNIT_TEST(testMatchOWRT_True);
  CPPUNIT_TEST(testMatchOWRT_False);
  CPPUNIT_TEST(testMatchVendor_True);
  CPPUNIT_TEST(testMatchVendor_False);
  CPPUNIT_TEST(testMatchRule_True);
  CPPUNIT_TEST(testMatchRule_False);
  CPPUNIT_TEST(testMatchRuleTariff_True);
  CPPUNIT_TEST(testMatchRuleTariff_False);
  CPPUNIT_TEST(testMmtchFareType_True);
  CPPUNIT_TEST(testMmtchFareType_False);
  CPPUNIT_TEST(testMatchCarrier_True);
  CPPUNIT_TEST(testMatchCarrier_False);

  CPPUNIT_TEST(testMatchTravelRangeX5);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Carrier_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Carrier_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Vendor_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Vendor_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_OWRT_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_OWRT_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Cat35_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Cat35_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_FareType_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_FareType_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Public_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Public_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_RuleTariff_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_RuleTariff_False);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Rule_True);
  CPPUNIT_TEST(testIsFareFocusRuleMatched_Rule_False);

  CPPUNIT_TEST(testIsDiagRuleNumberMatch_True);
  CPPUNIT_TEST(testIsDiagRuleNumberMatch_False);

  CPPUNIT_TEST(testValidGeoType_False);
  CPPUNIT_TEST(testValidGeoType_True);

  CPPUNIT_TEST(testPrintFareFocusRuleProcess_No_Print);
  CPPUNIT_TEST(testPrintFareFocusRuleProcess_Print);
  CPPUNIT_TEST(testMatchDirectionality_Int_Both_True);
  CPPUNIT_TEST(testMatchDirectionality_Dom_Both_True);

  CPPUNIT_TEST(testMatchPublicInd);

#ifdef PERFORMANCE_TEST
  CPPUNIT_TEST(testMatchFareClassStringMultiple);
#endif

  CPPUNIT_TEST_SUITE_END();

  PaxTypeFare* createPaxTypeFare(const std::string& carrier,
                                 const std::string& fareClass,
                                 const std::string& rule,
                                 const TariffNumber ruleTariff,
                                 const TariffCategory tCat,
                                 const std::string& fareType,
                                 const std::string& vendor,
                                 const Indicator  cat35,
                                 const std::string& bookingCode)
  {

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->_carrier = carrier;
    fi->_fareClass = fareClass;
    fi->_ruleNumber = "0100";
    fi->fareAmount() = 12.34;
    fi->_currency = "USD";
    fi->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    fi->ruleNumber() = rule;
    fi->vendor() = vendor;
    fi->market1() = "DFW";
    fi->market2() = "NYC";

    Fare* fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = 12.34;
    fare->setFareInfo(fi);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();

    FareClassAppSegInfo* fcasi = _memHandle.create<FareClassAppSegInfo>();
    fcasi->_bookingCode[0] = bookingCode;
    ptf->fareClassAppSegInfo() = fcasi;

    FareClassAppInfo* fca = _memHandle.create<FareClassAppInfo>();
    fca->_fareType = fareType;
    fca->_displayCatType = cat35;
    ptf->fareClassAppInfo() = fca;

    TariffCrossRefInfo* tcr = _memHandle.create<TariffCrossRefInfo>();
    tcr->ruleTariff() = ruleTariff;
    tcr->tariffCat() = tCat;
    fare->setTariffCrossRefInfo(tcr);

    ptf->setFare(fare);
    return ptf;
  }

  void setFBR(PaxTypeFare& fare, bool specified)
  {
    if (specified)
      _fbrItemInfo->fareInd() = 'S'; //SPECIFIED
    else
      _fbrItemInfo->fareInd() = 'C'; //calc
    _fbrRuleData->ruleItemInfo() = _fbrItemInfo;

    PaxTypeFare::PaxTypeFareAllRuleData* data =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    data->fareRuleData = _fbrRuleData;
    (*fare.paxTypeFareRuleDataMap())[25] = data;
  }

  void createPaxTypeFareFareInfo()
  {
    _ptf = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();
    _fare->setFareInfo(_fareInfo);
    _ptf->setFare(_fare);

    _fareMarket = _memHandle.create<FareMarket>();
    _fareMarket->geoTravelType() = GeoTravelType::International;
    _ptf->fareMarket() = _fareMarket;
  }

  void testCreateDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWLON"));
    _ffrv->createDiagnostic();
    CPPUNIT_ASSERT(_ffrv->_diag != 0);
  }

  void testMatchFMForDiagnostic_True_No_Diagnostic()
  {
    CPPUNIT_ASSERT(_ffrv->matchFMForDiagnostic());
  }

  void testMatchFMForDiagnostic_True_Diagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWLON"));
    _ffrv->createDiagnostic();
    CPPUNIT_ASSERT(_ffrv->matchFMForDiagnostic());
  }

  void testMatchFMForDiagnostic_False_Diagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWLIM"));
    _ffrv->createDiagnostic();
    CPPUNIT_ASSERT(!_ffrv->matchFMForDiagnostic());
  }

  void testPrintDiagAndReturn()
  {
    CPPUNIT_ASSERT(!_ffrv->printDiagAndReturn(""));
    CPPUNIT_ASSERT(!_ffrv->printDiagAndReturn("TEST MESSAGE"));

    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _ffrv->createDiagnostic();

    CPPUNIT_ASSERT(!_ffrv->printDiagAndReturn("- TEST MESSAGE"));
    std::string expected = " \nNO FARE FOCUS PROCESSING - TEST MESSAGE\n \n";

    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
  }

  void testIsFareFocusApplicable()
  {
    PricingOptions options;
    options.setExcludeFareFocusRule(false);
    _trx->setOptions(&options);

    PricingRequest request;
    Agent agent;
    request.ticketingAgent() = &agent;
    request.ticketingDT() = DateTime::localTime();
    _trx->setRequest(&request);

    _trx->setTrxType(PricingTrx::MIP_TRX);

    Customer cust;
    cust.crsCarrier() = "1J";
    cust.hostName() = "NONAXES";
    agent.agentTJR() = &cust;

    Billing billing;
    billing.partitionID() = "AA";
    billing.aaaCity() = "CCCC";
    _trx->billing() = &billing;

    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    // Test XFF
    options.setExcludeFareFocusRule(true);
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    options.setExcludeFareFocusRule(false);
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    // test JAL/Axcess
    cust.hostName() = "AXES";
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    cust.crsCarrier() = "3J";
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    // test airline partition
    billing.aaaCity() = "CCC";
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    billing.partitionID() = "";
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    billing.partitionID() = "ZZ";
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    agent.tvlAgencyPCC() = "XYZ";
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    agent.tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    agent.tvlAgencyPCC() = "Z";
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());

    _trx->getOptions()->setRtw(true);
    CPPUNIT_ASSERT(!_ffrv->isFareFocusApplicable());

    _trx->getOptions()->setRtw(false);
    CPPUNIT_ASSERT(_ffrv->isFareFocusApplicable());
  }


  void testMatchPTFareForDiagnostic_True_No_Diagnostic()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "TESTFARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_True_Diagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "TESTFARE"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "TESTFARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");

    CPPUNIT_ASSERT(_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_True_Diagnostic1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("RU", "0100"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "TESTFARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");

    CPPUNIT_ASSERT(_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_True_Diagnostic2()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("FC", "TESTFARE"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("RU", "0100"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "TESTFARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");

    CPPUNIT_ASSERT(_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_False_Diagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FC", "TESTFARE"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(!_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_False_Diagnostic1()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("RU", "2000"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(!_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testMatchPTFareForDiagnostic_False_Diagnostic2()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("FC", "TESTFARE"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair<std::string, std::string>("RU", "0100"));
    _ffrv->createDiagnostic();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(!_ffrv->matchPTFareForDiagnostic(*ptf));
  }

  void testPrintDiagSecurityHShakeNotFound()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _ffrv->createDiagnostic();
    _ffrv->printDiagSecurityHShakeNotFound();
     std::string expected = " \nNO FARE FOCUS PROCESSING - SECURITY HANDSHAKE NOT FOUND\n";

    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
  }

  void testPrintDiagFareFocusRulesNotFound()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _ffrv->createDiagnostic();
    _ffrv->printDiagFareFocusRulesNotFound();
     std::string expected = " \nNO FARE FOCUS PROCESSING - FARE FOCUS RULES ARE NOT FOUND\n";

    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
  }

  void testValidateSecurityHandshake_True()
  {
    _ffri->sourcePCC() = "A1B1";
    _cshi->securityTargetPCC() = "A1B1";
    std::vector<CustomerSecurityHandshakeInfo*> cshiV;
    cshiV.push_back(_cshi);
    CPPUNIT_ASSERT(_ffrv->validateSecurityHandshake(*_ffri, cshiV));
  }

  void testValidateSecurityHandshake_False()
  {
    _ffri->sourcePCC() = "A1B2";
    _cshi->securityTargetPCC() = "A1B1";
    std::vector<CustomerSecurityHandshakeInfo*> cshiV;
    CPPUNIT_ASSERT(!_ffrv->validateSecurityHandshake(*_ffri, cshiV));

    cshiV.push_back(_cshi);
    CPPUNIT_ASSERT(!_ffrv->validateSecurityHandshake(*_ffri, cshiV));
  }

  void testPrintFareFocusLookup_Diag()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _ffrv->createDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "LOOKUP"));

    uint64_t fareFocusRuleId = 778899;
    StatusFFRuleValidation rc = FAIL_FF_CARRIER;
    _ffrv->printFareFocusLookup(fareFocusRuleId, rc);
     std::string expected = "  FARE FOCUS RULE ID: 778899  STATUS- NOT MATCH CARRIER\n";

    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
}

  void testprintFareFocusNoData_Diag()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _ffrv->createDiagnostic();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "LOOKUP"));

    StatusFFRuleValidation rc = FAIL_FF_LOOKUP_EMPTY;
    _ffrv->printFareFocusNoData(rc);
     std::string expected = "*** FARE FOCUS LOOKUP IS EMPTY\n";

    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
}

  void testMatchOWRT_True_Rule_Blank()
  {
    Indicator owrtFromFareFocusRule = ' ';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(_ffrv->matchOWRT(owrtFromFareFocusRule,owrtFromFare));
  }
  void testMatchOWRT_True()
  {
    Indicator owrtFromFareFocusRule = '1';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(_ffrv->matchOWRT(owrtFromFareFocusRule,owrtFromFare));
  }
  void testMatchOWRT_False()
  {
    Indicator owrtFromFareFocusRule = '2';
    Indicator owrtFromFare = '1';
    CPPUNIT_ASSERT(!_ffrv->matchOWRT(owrtFromFareFocusRule,owrtFromFare));
  }

  void testMatchVendor_False()
  {
    VendorCode vendorRule = "ATP";
    VendorCode vendorFare = "SITA";
    CPPUNIT_ASSERT(!_ffrv->matchVendor(vendorRule,vendorFare));
  }

  void testMatchVendor_True()
  {
    VendorCode vendorRule = "ATP";
    VendorCode vendorFare = "ATP";
    CPPUNIT_ASSERT(_ffrv->matchVendor(vendorRule,vendorFare));
  }

/*  void testMatchCat35Type_True()
  {
    Indicator displayTypeRule = ' ';
    Indicator displayTypeFare = 'S';
    CPPUNIT_ASSERT(_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'L';
    displayTypeFare = 'L';
    CPPUNIT_ASSERT(_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'L';
    CPPUNIT_ASSERT(_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'T';
    CPPUNIT_ASSERT(_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));
  }

  void testMatchCat35Type_False()
  {
    Indicator displayTypeRule = 'C';
    Indicator displayTypeFare = 'C';
    CPPUNIT_ASSERT(!_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));

    displayTypeRule = 'Q';
    displayTypeFare = 'C';
    CPPUNIT_ASSERT(!_ffrv->matchCat35Type(displayTypeRule,displayTypeFare));
  }
*/
  void testMatchRule_True()
  {
    RuleNumber ruleNumberRule;
    RuleNumber ruleNumberFare = "1234";
    CPPUNIT_ASSERT(_ffrv->matchRule(ruleNumberRule, ruleNumberFare));

    ruleNumberRule = "1234";
    ruleNumberFare = "1234";
    CPPUNIT_ASSERT(_ffrv->matchRule(ruleNumberRule, ruleNumberFare));
  }

  void testMatchRule_False()
  {
    RuleNumber ruleNumberRule = "5678";
    RuleNumber ruleNumberFare = "1234";
    CPPUNIT_ASSERT(!_ffrv->matchRule(ruleNumberRule, ruleNumberFare));
  }

  void testMatchRuleTariff_True()
  {
    TariffNumber tariffNumberRule = 0;
    TariffNumber tariffNumberFare =1;
    CPPUNIT_ASSERT(_ffrv->matchRuleTariff(tariffNumberRule, tariffNumberFare));

    tariffNumberRule = 1;
    tariffNumberFare =1;
    CPPUNIT_ASSERT(_ffrv->matchRuleTariff(tariffNumberRule, tariffNumberFare));
  }

  void testMatchRuleTariff_False()
  {
    TariffNumber tariffNumberRule = 1;
    TariffNumber tariffNumberFare = 2;
    CPPUNIT_ASSERT(!_ffrv->matchRuleTariff(tariffNumberRule, tariffNumberFare));
  }

  void testMmtchFareType_True()
  {
    FareTypeAbbrevC fareTypeRule;
    FareType fareTypeFare = "XYZ";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*X";
    fareTypeFare = "XEX";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*B";  // one of the family class B
    fareTypeFare = "BX";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*Y";  // one of the family class Y
    fareTypeFare = "EU";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*EW";  // one of the family class EW
    fareTypeFare = "ZEX";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*FR";  // one of the family class FR
    fareTypeFare = "RU";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));

  }

  void testMmtchFareType_False()
  {
    FareTypeAbbrevC fareTypeRule = "ABC";
    FareType fareTypeFare = "SIP";
    CPPUNIT_ASSERT(!_ffrv->matchFareType(fareTypeRule, fareTypeFare));

    fareTypeRule = "*EW";
    fareTypeFare = "ZIP";
    CPPUNIT_ASSERT(_ffrv->matchFareType(fareTypeRule, fareTypeFare));
  }

  void testMatchCarrier_False()
  {
    CarrierCode carrierRule = "AA";
    PaxTypeFare* ptf = createPaxTypeFare("DL", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(!_ffrv->matchCarrier(carrierRule, *ptf));
  }

  void testMatchCarrier_True()
  {
    CarrierCode carrierRule = "AA";
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    CPPUNIT_ASSERT(_ffrv->matchCarrier(carrierRule, *ptf));
  }

  void testIsFareFocusRuleMatched_Carrier_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;
    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Carrier_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "DL";
    _ffri->vendor() = "ATP";
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Vendor_True()
  {
    createPaxTypeFareFareInfo();
    _fareInfo->vendor() = "ATP";
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->vendor() = "ATP";
    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = LOCTYPE_AREA;
    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*_ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Vendor_False()
  {
    createPaxTypeFareFareInfo();
    _fareInfo->vendor() = "ATP";
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->vendor() = "SITA";
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*_ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_OWRT_True()
  {
    createPaxTypeFareFareInfo();
    _fareInfo->owrt() = '1';
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->owrt() = '1';
    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = LOCTYPE_AREA;
    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*_ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_OWRT_False()
  {
    createPaxTypeFareFareInfo();
    _fareInfo->owrt() = '1';
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->owrt() = '2';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*_ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Cat35_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->displayType() = 'L';
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;

    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Cat35_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->displayType() = 'C';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_FareType_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->fareType() = "XPN";
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;

    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_FareType_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->fareType() = "XEX";
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Public_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 1, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->publicPrivateIndicator() = 'Y';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Public_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");

    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->publicPrivateIndicator() = 'Y';
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;

    _ffri->directionality() = 'B';
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_RuleTariff_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->ruleTariff() = 3;
    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = LOCTYPE_AREA;
    _ffri->directionality() = 'B';
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;

    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_RuleTariff_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->ruleTariff() = 11;
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Rule_True()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->ruleCode() = "0100";
    _ffri->directionality() = 'B';
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().loc() = "NYC";
    _ffri->loc1().locType() = LOCTYPE_CITY;
    _ffri->loc2().locType() = LOCTYPE_CITY;

    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsFareFocusRuleMatched_Rule_False()
  {
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");
    std::vector<const FareFocusRuleInfo*> ffRulesV;
    _ffri->carrier() = "AA";
    _ffri->vendor() = "ATP";
    _ffri->ruleCode() = "0400";
    ffRulesV.push_back(_ffri);

    CPPUNIT_ASSERT(!_ffrv->isFareFocusRuleMatched(*ptf, ffRulesV));
  }

  void testIsDiagRuleNumberMatch_True()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("ID", "12399"));
    _ffrv->createDiagnostic();
    _ffri->fareFocusRuleId() = 12399;
    CPPUNIT_ASSERT(_ffrv->isDiagRuleNumberMatch(_ffri));
  }

  void testIsDiagRuleNumberMatch_False()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("ID", "12399"));
    _ffrv->createDiagnostic();
    _ffri->fareFocusRuleId() = 123;
    CPPUNIT_ASSERT(!_ffrv->isDiagRuleNumberMatch(_ffri));
  }

  void testValidGeoType_False()
  {
    CPPUNIT_ASSERT(!_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_SUBAREA;
    _ffri->loc2().locType() = LOCTYPE_NATION;
    CPPUNIT_ASSERT(!_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_STATE;
    CPPUNIT_ASSERT(!_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_AIRPORT;
    CPPUNIT_ASSERT(!_ffrv->validGeoType(*_ffri));

  }

  void testValidGeoType_True()
  {
    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_NATION;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_CITY;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_ZONE;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_NATION;
    _ffri->loc2().locType() = LOCTYPE_AREA;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = LOCTYPE_ZONE;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = LOCTYPE_AREA;
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = LOCTYPE_AREA;
    _ffri->loc2().locType() = 'G';
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

    _ffri->loc1().locType() = 'G';
    _ffri->loc2().locType() = 'G';
    CPPUNIT_ASSERT(_ffrv->validGeoType(*_ffri));

  }

  void testPrintFareFocusRuleProcess_No_Print()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("ID", "12399"));
    _ffrv->createDiagnostic();
    _ffri->fareFocusRuleId() = 123;
    StatusFFRuleValidation rc = PASS_FF;
    _ffrv->printFareFocusRuleProcess(_ffri, rc);

    std::string expected = "";
    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);

  }

  void testMatchTravelRangeX5()
  {
    DateTime adjustedTicketDate = DateTime::localTime();
    PaxTypeFare* ptf = createPaxTypeFare("AA", "FARE", "0100", 3, 0, "XPN", "ATP", 'L', "NA");
    _ffri->travelDayTimeApplItemNo() = 0;

    CPPUNIT_ASSERT(_ffrv->matchTravelRangeX5(*_trx, _ffri->travelDayTimeApplItemNo(), *ptf, adjustedTicketDate));
  }

  void testPrintFareFocusRuleProcess_Print()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic240;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("ID", "12399"));
    _ffrv->createDiagnostic();
    _ffri->fareFocusRuleId() = 12399;
    StatusFFRuleValidation rc = PASS_FF;
    _ffrv->printFareFocusRuleProcess(_ffri, rc);

    std::string expected = "  FARE FOCUS RULE ID: 12399  STATUS- MATCH\n";
    CPPUNIT_ASSERT(_ffrv->_diag->str() == expected);
  }

  void testMatchDirectionality_Int_Both_True()
  {
    _ffri->directionality() = 'B';
    createPaxTypeFareFareInfo();
    GeoTravelType geoTravelType = GeoTravelType::International;
    _ptf->fareMarket()->geoTravelType() = geoTravelType;

    CPPUNIT_ASSERT(_ffrv->matchDirectionality(*_ffri, *_ptf));

    geoTravelType = GeoTravelType::ForeignDomestic;
    _ptf->fareMarket()->geoTravelType() = geoTravelType;
    CPPUNIT_ASSERT(_ffrv->matchDirectionality(*_ffri, *_ptf));
  }

  void testMatchDirectionality_Dom_Both_True()
  {
    _ffri->directionality() = 'B';
    createPaxTypeFareFareInfo();
    GeoTravelType geoTravelType = GeoTravelType::Domestic;
    _ptf->fareMarket()->geoTravelType() = geoTravelType;
    CPPUNIT_ASSERT(_ffrv->matchDirectionality(*_ffri, *_ptf));

    geoTravelType = GeoTravelType::Transborder;
    _ptf->fareMarket()->geoTravelType() = geoTravelType;
    CPPUNIT_ASSERT(_ffrv->matchDirectionality(*_ffri, *_ptf));
  }

  void testMatchPublicInd()
  {
    Indicator publicIndRule = ' ';
    TariffCategory publicIndFare = RuleConst::PRIVATE_TARIFF;
    CPPUNIT_ASSERT(_ffrv->matchPublicInd(publicIndRule, publicIndFare));
    publicIndRule = '*';
    CPPUNIT_ASSERT(!_ffrv->matchPublicInd(publicIndRule, publicIndFare));
  }

public:
  void setUp()
  {
    _memHandle.create<tse::SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _fm = _memHandle.create<FareMarket>();
    _loc1 = _memHandle.create<Loc>();
    _loc1->loc() = "DFW";
    _loc2 = _memHandle.create<Loc>();
    _loc2->loc() = "LON";
    _fm->boardMultiCity() = "DFW";
    _fm->offMultiCity() = "LON";
    _fm->origin() = _loc1;
    _fm->destination() = _loc2;
    _ffrv = _memHandle.insert(new FareFocusRuleValidatorMock(*_trx, *_fm));
    _ffri = _memHandle.create<FareFocusRuleInfo>();
    _cshi = _memHandle.create<CustomerSecurityHandshakeInfo>();
    _fbrItemInfo = _memHandle.create<FareByRuleItemInfo>();
    _fbrRuleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    _ffbci = _memHandle.create<FareFocusBookingCodeInfo>();
    _ffbciV.push_back(_ffbci);
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingDT() = DateTime::localTime();
    _trx->setRequest(_request);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

private:
  TestMemHandle       _memHandle;
  FareFocusRuleValidator* _ffrv;
  FareMarket*          _fm;
  PricingTrx*         _trx;
  PricingRequest* _request;
  Loc* _loc1;
  Loc* _loc2;
  FareFocusRuleInfo* _ffri;
  CustomerSecurityHandshakeInfo* _cshi;
  FareInfo* _fareInfo;
  Fare* _fare;
  PaxTypeFare* _ptf;
  FareByRuleItemInfo* _fbrItemInfo;
  FBRPaxTypeFareRuleData* _fbrRuleData;
  FareFocusBookingCodeInfo* _ffbci;
  std::vector<FareFocusBookingCodeInfo*> _ffbciV;
  FareMarket* _fareMarket;

};
CPPUNIT_TEST_SUITE_REGISTRATION(FareFocusRuleValidatorTest);
}
