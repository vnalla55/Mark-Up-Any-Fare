#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/ValueCodeAlgorithm.h"
#include "Rules/RuleConst.h"
#include "Rules/ValueCodeUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class ValueCodeUtilTest : public CppUnit::TestFixture
{
  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare(VendorCode vendor, CarrierCode carrier)
    {
      _fareInfo.vendor() = vendor;
      _fareInfo.carrier() = carrier;
      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);
      setFare(&_fare);
      status().set(PaxTypeFare::PTF_Negotiated);
      _fareClassAppInfo._displayCatType = RuleConst::NET_SUBMIT_FARE;
      fareClassAppInfo() = &_fareClassAppInfo;
      _negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_2;
      _negRuleData.ruleItemInfo() = &_negFareRest;
      _negRuleData.negFareRestExt() = &_negFareRestExt;
      _negAllRuleData.fareRuleData = &_negRuleData;
      (*(paxTypeFareRuleDataMap()))[RuleConst::NEGOTIATED_RULE] = &_negAllRuleData;
    }

    NegPaxTypeFareRuleData& negRuleData() { return _negRuleData; }
    const NegPaxTypeFareRuleData& negRuleData() const { return _negRuleData; }

    NegFareRestExt& negFareRestExt() { return _negFareRestExt; }
    const NegFareRestExt& negFareRestExt() const { return _negFareRestExt; }

  protected:
    FareClassAppInfo _fareClassAppInfo;
    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    FareMarket _fareMarket;
    NegFareRest _negFareRest;
    NegFareRestExt _negFareRestExt;
    PaxTypeFare::PaxTypeFareAllRuleData _negAllRuleData;
    NegPaxTypeFareRuleData _negRuleData;
  };

  class MyDataHandle : public DataHandleMock
  {
  public:
    char getVendorType(const VendorCode& vendor)
    {
      if (vendor == "SMFC")
        return 'T';
      return DataHandleMock::getVendorType(vendor);
    }
  };

  CPPUNIT_TEST_SUITE(ValueCodeUtilTest);
  CPPUNIT_TEST(testDecodeValueWhenDecimalChar);
  CPPUNIT_TEST(testDecodeValueWhenDigit);

  CPPUNIT_TEST(testGetDecodedValueCodeWhenOnlyPrefixAndSuffixExist);
  CPPUNIT_TEST(testGetDecodedValueCodeWhenBadDataInDigits);
  CPPUNIT_TEST(testGetDecodedValueCodeWhenAreDigitsButNoDecimalChar);
  CPPUNIT_TEST(testGetDecodedValueCodeWhenAreDigitsAndDecimalChar);
  CPPUNIT_TEST(testGetDecodedValueCodeWhenNoDigitsAndNoDecimalChar);
  CPPUNIT_TEST(testGetDecodedValueCodeWhenNoDigitsAndIsDecimalChar);

  CPPUNIT_TEST(testGetRuleDataForNetRemitCat35FareWhenNotCat35Fare);
  CPPUNIT_TEST(testGetRuleDataForNetRemitCat35FareWhenCat35Fare);
  CPPUNIT_TEST(testGetRuleDataForNetRemitCat35FareWhenCat35NetRemitFare);

  CPPUNIT_TEST(testGetHashedValueCodeData);
  CPPUNIT_TEST(testGetHashedValueCodeDataWhenPass);
  CPPUNIT_TEST(testGetHashedValueCodeDataWhenFail);

  CPPUNIT_TEST(testValidateDVACCombinationWhenNoFares);
  CPPUNIT_TEST(testValidateDVACCombinationWhenNonSMFFare);
  CPPUNIT_TEST(testValidateDVACCombinationWhenSameDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenDifferentDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenSameCarrierWithEmptyDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenDifferentCarrierWithEmptyDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenMixedCarrierWithEmptyDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenSameGoverningCxrWithEmptyDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenDifferentGoverningCxrWithEmptyDVAC);
  CPPUNIT_TEST(testValidateDVACCombinationWhenSameExcCat12Ind);
  CPPUNIT_TEST(testValidateDVACCombinationWhenDifferentExcCat12Ind);

  CPPUNIT_TEST(testHasDynamicValueCodeFareReturnFalseWhenNoDVACFare);
  CPPUNIT_TEST(testHasDynamicValueCodeFareReturnTrueWhenDVACFareExists);

  CPPUNIT_TEST(testHasCat35ValueCode_Pass);
  CPPUNIT_TEST(testHasCat35ValueCode_Fail);

  CPPUNIT_TEST(testValidateStaticValueCodeCombination_Pass_OneFuWith1st);
  CPPUNIT_TEST(testValidateStaticValueCodeCombination_Pass_OneFuWith2nd);
  CPPUNIT_TEST(testValidateStaticValueCodeCombination_Fail_TwoWith1st);
  CPPUNIT_TEST(testValidateStaticValueCodeCombination_Pass_Match);
  CPPUNIT_TEST(testValidateStaticValueCodeCombination_Pass_NoCat35);

  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_Equal);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_DifferentInd);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_DifferentVcWithBlank);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_BlankC35WithC18Equal);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_BlankC35WithC18NotEqual);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_1stC35WithC18);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_2stC35WithC18);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_AllC35WithC18);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_C18WithBlankC35Equal);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_C18WithBlankC35NotEqual);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_C18With1stC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_C18With2ndC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_C18WithAllC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_1stC35WithBlank);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_2ndC35WithBlank);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_AllC35WithBlank);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_BlankC35WithBlank_OnCxr);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_BlankC35WithBlank_OnCxr);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_BlankWith1stC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_BlankWith2ndC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_BlankWithAllC35);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_BlankWithBlankC35_OnCxr);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Fail_BlankWithBlankC35_OnCxr);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_Matched);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_C18WithC18);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_C18WithBlank);
  CPPUNIT_TEST(testMatchStaticValueCodeCombination_Pass_BlankWithC18);

  CPPUNIT_TEST(testGetCat18ValueCodeReturnEmptyWhenNoEndorsement);
  CPPUNIT_TEST(testGetCat18ValueCodeReturnEmptyWhenNoValueCodeInEndorsement);
  CPPUNIT_TEST(testGetCat18ValueCodeReturnNotEmptyWhenValueCodeInEndorsement);

  CPPUNIT_TEST(testGetStaticValueCodeReturnEmptyWhenNoVCodeEXTAndNoCat18);
  CPPUNIT_TEST(testGetStaticValueCodeReturnNotEmptyWhenVCodeEXTAndNoCat18);
  CPPUNIT_TEST(testGetStaticValueCodeReturnNotEmptyWhenNoVCodeEXTAndCat18);

  CPPUNIT_TEST(testGetFormattedTotalNetAmt);

  CPPUNIT_TEST(testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetTo1st);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetTo1st);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetTo2nd);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetTo2nd);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToAll);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToAllCheckUnique);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToBlank);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenFare2HasntSVCandFare1HasSVCIndSetToBlank);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetToBlank);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenFare1And2HasntSVCandFare3HasSVCIndSetToBlank);
  CPPUNIT_TEST(testCreateStaticValueCodeWhenOneFareAndSVCIndIsSetTo2nd);
  CPPUNIT_TEST(testSaveStaticValueCodeWhenHasValueCode);
  CPPUNIT_TEST(testSaveStaticValueCodeWhenNoStaticVC);
  CPPUNIT_TEST(testGetStaticValueCodeInfoWhenHasStaticValueCode);
  CPPUNIT_TEST(testGetStaticValueCodeInfoWhenStaticValueCodeIsEmpty);
  CPPUNIT_TEST(testDecodeValueCodeInfoWhenBaseAMTOptionTrue);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _valueCodeUtil = _memHandle.create<ValueCodeUtil>();
    _algorithm = _memHandle.create<ValueCodeAlgorithm>();
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<MyDataHandle>();
    _typeInd = _memHandle.create<Indicator>();
    _fcConfig = _memHandle.create<FareCalcConfig>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _agent->agentTJR() = _customer;
    _request->ticketingAgent() = _agent;
    _trx->setRequest(_request);
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
  }

  void tearDown() { _memHandle.clear(); }

  FareUsage* createFareUsage(PaxTypeFare& paxTypeFare)
  {
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    fareUsage->paxTypeFare() = &paxTypeFare;
    return fareUsage;
  }

  FareUsage* createFareUsage(const std::string& vc = "VC",
                             Indicator cat35VCInd = RuleConst::DISPLAY_OPTION_BLANK,
                             bool isCat35VC = true,
                             const CarrierCode& cxr = "AA")
  {
    FareUsage* fareUsage = _memHandle.create<FareUsage>();
    MockPaxTypeFare* ptf = _memHandle.insert(new MockPaxTypeFare("SMFC", cxr));
    fareUsage->paxTypeFare() = ptf;
    NegFareRestExt* restExt = _memHandle.create<NegFareRestExt>();
    ptf->negRuleData().negFareRestExt() = restExt;

    if (isCat35VC)
      restExt->staticValueCode() = vc;
    else
    {
      TicketEndorseItem* eItem = _memHandle.create<TicketEndorseItem>();
      eItem->tktLocInd = '1';
      eItem->endorsementTxt = vc;

      fareUsage->tktEndorsement().push_back(*eItem);
    }
    restExt->staticValueCodeCombInd() = cat35VCInd;

    return fareUsage;
  }

  void testDecodeValueWhenDecimalChar()
  {
    _algorithm->decimalChar() = 'Z';

    CPPUNIT_ASSERT_EQUAL(_algorithm->decimalChar(), ValueCodeUtil::decodeValue('.', *_algorithm));
  }

  void testDecodeValueWhenDigit()
  {
    _algorithm->digitToChar()[4] = 'F';

    CPPUNIT_ASSERT_EQUAL(_algorithm->digitToChar()[4],
                         ValueCodeUtil::decodeValue('4', *_algorithm));
  }

  void prepareGetDecodedValueCode(ValueCodeAlgorithm& valCodeAlg)
  {
    valCodeAlg.prefix() = "QWE";
    valCodeAlg.suffix() = "RTY";
    valCodeAlg.digitToChar()[0] = 'F';
    valCodeAlg.digitToChar()[1] = 'H';
    valCodeAlg.digitToChar()[2] = 'C';
    valCodeAlg.digitToChar()[3] = 'A';
    valCodeAlg.digitToChar()[4] = 'T';
    valCodeAlg.digitToChar()[5] = 'D';
    valCodeAlg.digitToChar()[6] = 'S';
    valCodeAlg.digitToChar()[7] = 'U';
    valCodeAlg.digitToChar()[8] = 'V';
    valCodeAlg.digitToChar()[9] = 'I';
    valCodeAlg.decimalChar() = 'X';
  }

  void testGetDecodedValueCodeWhenOnlyPrefixAndSuffixExist()
  {
    _algorithm->prefix() = "QWE";
    _algorithm->suffix() = "RTY";
    std::string inputValue = "1539.340";
    std::string expectedResult = "QWERTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetDecodedValueCodeWhenBadDataInDigits()
  {
    prepareGetDecodedValueCode(*_algorithm);
    _algorithm->digitToChar()[6] = ' ';
    std::string inputValue = "1539.340";
    std::string expectedResult = "QWERTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetDecodedValueCodeWhenAreDigitsButNoDecimalChar()
  {
    prepareGetDecodedValueCode(*_algorithm);
    _algorithm->decimalChar() = ' ';
    std::string inputValue = "1539.340";
    std::string expectedResult = "QWEHDAIRTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetDecodedValueCodeWhenAreDigitsAndDecimalChar()
  {
    prepareGetDecodedValueCode(*_algorithm);
    std::string inputValue = "1539.340";
    std::string expectedResult = "QWEHDAIXATFRTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetDecodedValueCodeWhenNoDigitsAndNoDecimalChar()
  {
    prepareGetDecodedValueCode(*_algorithm);
    _algorithm->decimalChar() = ' ';
    std::string inputValue = "1539";
    std::string expectedResult = "QWEHDAIRTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetDecodedValueCodeWhenNoDigitsAndIsDecimalChar()
  {
    prepareGetDecodedValueCode(*_algorithm);
    std::string inputValue = "1539";
    std::string expectedResult = "QWEHDAIRTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult,
                         ValueCodeUtil::getDecodedValueCode(*_algorithm, inputValue));
  }

  void testGetRuleDataForNetRemitCat35FareWhenNotCat35Fare()
  {
    PaxTypeFare ptf;
    NegPaxTypeFareRuleData* ruleData = 0;

    CPPUNIT_ASSERT_EQUAL(ruleData, ValueCodeUtil::getRuleDataForNetRemitCat35Fare(ptf));
  }

  void testGetRuleDataForNetRemitCat35FareWhenCat35Fare()
  {
    MockPaxTypeFare ptf("SMFC", "AA");
    FareClassAppInfo fci;
    fci._displayCatType = RuleConst::SELLING_FARE_NOT_FOR_SEC;
    ptf.fareClassAppInfo() = &fci;
    NegPaxTypeFareRuleData* ruleData = 0;

    CPPUNIT_ASSERT_EQUAL(ruleData, ValueCodeUtil::getRuleDataForNetRemitCat35Fare(ptf));
  }

  void testGetRuleDataForNetRemitCat35FareWhenCat35NetRemitFare()
  {
    MockPaxTypeFare ptf("SMFC", "AA");

    CPPUNIT_ASSERT_EQUAL(&ptf.negRuleData(), ValueCodeUtil::getRuleDataForNetRemitCat35Fare(ptf));
  }

  void testGetHashedValueCodeData()
  {
    prepareGetDecodedValueCode(*_algorithm);
    std::string expectedResult = "QWEXFHCATDSUVIRTY";

    CPPUNIT_ASSERT_EQUAL(expectedResult, ValueCodeUtil::getHashedValueCodeData(*_algorithm));
  }

  void testGetHashedValueCodeDataWhenPass()
  {
    prepareGetDecodedValueCode(*_algorithm);
    ValueCodeAlgorithm valCodeAlg2;
    prepareGetDecodedValueCode(valCodeAlg2);

    CPPUNIT_ASSERT(ValueCodeUtil::sameHashedValueCodeData(*_algorithm, valCodeAlg2));
  }

  void testGetHashedValueCodeDataWhenFail()
  {
    prepareGetDecodedValueCode(*_algorithm);
    ValueCodeAlgorithm valCodeAlg2;
    prepareGetDecodedValueCode(valCodeAlg2);

    valCodeAlg2.suffix() = "WER1";

    CPPUNIT_ASSERT(!ValueCodeUtil::sameHashedValueCodeData(*_algorithm, valCodeAlg2));
  }

  void testValidateDVACCombinationWhenNoFares()
  {
    std::vector<const FareUsage*> fareUsages;
    CPPUNIT_ASSERT(ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenNonSMFFare()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("ATP", "AA");
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenSameDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "AA");
    ValueCodeAlgorithm valCodeAlg2;
    valCodeAlg2.prefix() = "TEST";
    paxTypeFare2.negRuleData().valueCodeAlgorithm() = &valCodeAlg2;
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenDifferentDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "AA");
    ValueCodeAlgorithm valCodeAlg2;
    valCodeAlg2.prefix() = "TEST2";
    paxTypeFare2.negRuleData().valueCodeAlgorithm() = &valCodeAlg2;
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenSameCarrierWithEmptyDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "AA");
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenDifferentCarrierWithEmptyDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "BA");
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenMixedCarrierWithEmptyDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "BA");
    ValueCodeAlgorithm valCodeAlg2;
    valCodeAlg2.prefix() = "TEST";
    paxTypeFare2.negRuleData().valueCodeAlgorithm() = &valCodeAlg2;
    MockPaxTypeFare paxTypeFare3("SMFC", "AA");
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    fareUsages.push_back(createFareUsage(paxTypeFare3));
    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenSameGoverningCxrWithEmptyDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "YY");
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";
    paxTypeFare2.fareMarket() = &fareMarket;
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenDifferentGoverningCxrWithEmptyDVAC()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "YY");
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "BA";
    paxTypeFare2.fareMarket() = &fareMarket;
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenSameExcCat12Ind()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    FareProperties fareProperties1;
    fareProperties1.excludeQSurchargeInd() = 'N';
    paxTypeFare1.negRuleData().fareProperties() = &fareProperties1;
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;
    MockPaxTypeFare paxTypeFare2("SMFC", "AA");
    FareProperties fareProperties2;
    fareProperties2.excludeQSurchargeInd() = 'N';
    paxTypeFare2.negRuleData().fareProperties() = &fareProperties2;
    ValueCodeAlgorithm valCodeAlg2;
    valCodeAlg2.prefix() = "TEST";
    paxTypeFare2.negRuleData().valueCodeAlgorithm() = &valCodeAlg2;
    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));
    CPPUNIT_ASSERT(ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  void testValidateDVACCombinationWhenDifferentExcCat12Ind()
  {
    std::vector<const FareUsage*> fareUsages;
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    FareProperties fareProperties1;
    fareProperties1.excludeQSurchargeInd() = 'Y';
    paxTypeFare1.negRuleData().fareProperties() = &fareProperties1;
    _algorithm->prefix() = "TEST";
    paxTypeFare1.negRuleData().valueCodeAlgorithm() = _algorithm;

    MockPaxTypeFare paxTypeFare2("SMFC", "AA");
    FareProperties fareProperties2;
    fareProperties2.excludeQSurchargeInd() = 'N';
    paxTypeFare2.negRuleData().fareProperties() = &fareProperties2;
    ValueCodeAlgorithm valCodeAlg2;
    valCodeAlg2.prefix() = "TEST";
    paxTypeFare2.negRuleData().valueCodeAlgorithm() = &valCodeAlg2;

    fareUsages.push_back(createFareUsage(paxTypeFare1));
    fareUsages.push_back(createFareUsage(paxTypeFare2));

    CPPUNIT_ASSERT(!ValueCodeUtil::validateDynamicValueCodeCombination(*_trx, fareUsages));
  }

  FarePath* createFarePath(const std::string& staticVC,
                           bool createVCAlgorithm,
                           int noFUs = 1,
                           Indicator staticVCodeInd = RuleConst::DISPLAY_OPTION_BLANK)
  {
    MockPaxTypeFare* ptf = _memHandle.insert(new MockPaxTypeFare("SMFC", "AA"));
    PricingUnit* pu = _memHandle.create<PricingUnit>();

    for (int j = noFUs - 1; j > 0; j--)
      pu->fareUsage().push_back(
          createFareUsage(*_memHandle.insert(new MockPaxTypeFare("SMFC", "AA"))));
    pu->fareUsage().push_back(createFareUsage(*ptf));

    FarePath* fp = _memHandle.create<FarePath>();
    fp->pricingUnit().push_back(pu);
    NegFareRestExt* restExt = _memHandle.create<NegFareRestExt>();
    restExt->staticValueCode() = staticVC;
    restExt->staticValueCodeCombInd() = staticVCodeInd;
    ptf->negRuleData().negFareRestExt() = restExt;

    if (createVCAlgorithm)
    {
      ValueCodeAlgorithm* valCodeAlg = _memHandle.create<ValueCodeAlgorithm>();
      ptf->negRuleData().valueCodeAlgorithm() = valCodeAlg;
    }

    return fp;
  }

  void testHasDynamicValueCodeFareReturnFalseWhenNoDVACFare()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::hasDynamicValueCodeFare(*createFarePath("", false)));
  }

  void testHasDynamicValueCodeFareReturnTrueWhenDVACFareExists()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::hasDynamicValueCodeFare(*createFarePath("", true)));
  }

  void testHasCat35ValueCode_Pass()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::hasCat35ValueCode(*createFarePath("SVC", false)));
  }

  void testHasCat35ValueCode_Fail()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::hasCat35ValueCode(*createFarePath("", false)));
  }

  void testValidateStaticValueCodeCombination_Pass_OneFuWith1st()
  {
    CPPUNIT_ASSERT(
        ValueCodeUtil::validateStaticValueCodeCombination(*_trx, *createFarePath("VC", false, 1)));
  }

  void testValidateStaticValueCodeCombination_Pass_OneFuWith2nd()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::validateStaticValueCodeCombination(
        *_trx, *createFarePath("VC", false, 1, RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testValidateStaticValueCodeCombination_Fail_TwoWith1st()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::validateStaticValueCodeCombination(
                       *_trx, *createFarePath("VC", false, 2, RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testValidateStaticValueCodeCombination_Pass_Match()
  {
    CPPUNIT_ASSERT(
        ValueCodeUtil::validateStaticValueCodeCombination(*_trx, *createFarePath("VC", false, 2)));
  }

  void testValidateStaticValueCodeCombination_Pass_NoCat35()
  {
    CPPUNIT_ASSERT(
        ValueCodeUtil::validateStaticValueCodeCombination(*_trx, *createFarePath("", false, 2)));
  }

  void testMatchStaticValueCodeCombination_Pass_Equal()
  {
    FareUsage* fu1 = createFareUsage();
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(*_trx, fu1, fu1));
  }

  void testMatchStaticValueCodeCombination_Fail_DifferentInd()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC", RuleConst::DISPLAY_OPTION_1ST),
                       createFareUsage("VC", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchStaticValueCodeCombination_Fail_DifferentVcWithBlank()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchStaticValueCodeCombination_Pass_BlankC35WithC18Equal()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK),
        createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Fail_BlankC35WithC18NotEqual()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Fail_1stC35WithC18()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_1ST),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Fail_2stC35WithC18()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_2ND),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Fail_AllC35WithC18()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_ALL),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Pass_C18WithBlankC35Equal()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
        createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchStaticValueCodeCombination_Fail_C18WithBlankC35NotEqual()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchStaticValueCodeCombination_Fail_C18With1stC35()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchStaticValueCodeCombination_Fail_C18With2ndC35()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchStaticValueCodeCombination_Fail_C18WithAllC35()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_ALL)));
  }

  void testMatchStaticValueCodeCombination_Pass_1stC35WithBlank()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx, createFareUsage("VC1", RuleConst::DISPLAY_OPTION_1ST), createFareUsage("")));
  }

  void testMatchStaticValueCodeCombination_Fail_2ndC35WithBlank()
  {
    CPPUNIT_ASSERT(
        !ValueCodeUtil::matchStaticValueCodeCombination(
            *_trx, createFareUsage("VC1", RuleConst::DISPLAY_OPTION_2ND), createFareUsage("")));
  }

  void testMatchStaticValueCodeCombination_Fail_AllC35WithBlank()
  {
    CPPUNIT_ASSERT(
        !ValueCodeUtil::matchStaticValueCodeCombination(
            *_trx, createFareUsage("VC1", RuleConst::DISPLAY_OPTION_ALL), createFareUsage("")));
  }

  void testMatchStaticValueCodeCombination_Pass_BlankC35WithBlank_OnCxr()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx, createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK), createFareUsage("")));
  }

  void testMatchStaticValueCodeCombination_Fail_BlankC35WithBlank_OnCxr()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK),
                       createFareUsage("", RuleConst::DISPLAY_OPTION_BLANK, true, "UA")));
  }

  void testMatchStaticValueCodeCombination_Fail_BlankWith1stC35()
  {
    CPPUNIT_ASSERT(
        !ValueCodeUtil::matchStaticValueCodeCombination(
            *_trx, createFareUsage(""), createFareUsage("VC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchStaticValueCodeCombination_Pass_BlankWith2ndC35()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx, createFareUsage(""), createFareUsage("VC2", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchStaticValueCodeCombination_Fail_BlankWithAllC35()
  {
    CPPUNIT_ASSERT(
        !ValueCodeUtil::matchStaticValueCodeCombination(
            *_trx, createFareUsage(""), createFareUsage("VC2", RuleConst::DISPLAY_OPTION_ALL)));
  }

  void testMatchStaticValueCodeCombination_Pass_BlankWithBlankC35_OnCxr()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx, createFareUsage(""), createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchStaticValueCodeCombination_Fail_BlankWithBlankC35_OnCxr()
  {
    CPPUNIT_ASSERT(!ValueCodeUtil::matchStaticValueCodeCombination(
                       *_trx,
                       createFareUsage(""),
                       createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, true, "UA")));
  }

  void testMatchStaticValueCodeCombination_Pass_Matched()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage("VC", RuleConst::DISPLAY_OPTION_1ST),
        createFareUsage("VC", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchStaticValueCodeCombination_Pass_C18WithC18()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
        createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testMatchStaticValueCodeCombination_Pass_C18WithBlank()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage("VC1", RuleConst::DISPLAY_OPTION_BLANK, false),
        createFareUsage("")));
  }

  void testMatchStaticValueCodeCombination_Pass_BlankWithC18()
  {
    CPPUNIT_ASSERT(ValueCodeUtil::matchStaticValueCodeCombination(
        *_trx,
        createFareUsage(""),
        createFareUsage("VC2", RuleConst::DISPLAY_OPTION_BLANK, false)));
  }

  void testGetCat18ValueCodeReturnEmptyWhenNoEndorsement()
  {
    FareUsage* fu1 = createFareUsage();
    CPPUNIT_ASSERT((ValueCodeUtil::getCat18ValueCode(*fu1)).empty());
  }

  void testGetCat18ValueCodeReturnEmptyWhenNoValueCodeInEndorsement()
  {
    FareUsage* fu1 = createFareUsage();
    TicketEndorseItem eItem;
    eItem.tktLocInd = '2'; // not valueCode in Cat18
    fu1->tktEndorsement().push_back(eItem);

    CPPUNIT_ASSERT((ValueCodeUtil::getCat18ValueCode(*fu1)).empty());
  }

  void testGetCat18ValueCodeReturnNotEmptyWhenValueCodeInEndorsement()
  {
    FareUsage* fu1 = createFareUsage();
    TicketEndorseItem eItem;
    eItem.tktLocInd = '3'; //  valueCode in cat18
    eItem.endorsementTxt = "value";
    fu1->tktEndorsement().push_back(eItem);

    std::string staticC18 = ValueCodeUtil::getCat18ValueCode(*fu1);
    CPPUNIT_ASSERT(!staticC18.empty());
  }

  void testGetStaticValueCodeReturnEmptyWhenNoVCodeEXTAndNoCat18()
  {
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    FareUsage* fu1 = createFareUsage(paxTypeFare1);
    TicketEndorseItem eItem;
    eItem.tktLocInd = '2'; // not valueCode in Cat18
    fu1->tktEndorsement().push_back(eItem);
    NegFareRestExt negFareRestExt1;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    NegPaxTypeFareRuleData& negRuleData = paxTypeFare1.negRuleData();

    CPPUNIT_ASSERT((ValueCodeUtil::getStaticValueCode(*fu1, &negRuleData)).empty());
  }

  void testGetStaticValueCodeReturnNotEmptyWhenVCodeEXTAndNoCat18()
  {
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    FareUsage* fu1 = createFareUsage(paxTypeFare1);
    TicketEndorseItem eItem;
    eItem.tktLocInd = '2'; // not valueCode in Cat18
    fu1->tktEndorsement().push_back(eItem);

    NegFareRestExt negFareRestExt1;
    negFareRestExt1.staticValueCode() = "static";
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    NegPaxTypeFareRuleData& negRuleData = paxTypeFare1.negRuleData();

    CPPUNIT_ASSERT(!(ValueCodeUtil::getStaticValueCode(*fu1, &negRuleData)).empty());
  }

  void testGetStaticValueCodeReturnNotEmptyWhenNoVCodeEXTAndCat18()
  {
    MockPaxTypeFare paxTypeFare1("SMFC", "AA");
    FareUsage* fu1 = createFareUsage(paxTypeFare1);
    TicketEndorseItem eItem;
    eItem.tktLocInd = '3'; //  valueCode in cat18
    eItem.endorsementTxt = "value";
    fu1->tktEndorsement().push_back(eItem);

    NegFareRestExt negFareRestExt1;
    paxTypeFare1.negRuleData().negFareRestExt() = &negFareRestExt1;
    NegPaxTypeFareRuleData& negRuleData = paxTypeFare1.negRuleData();

    CPPUNIT_ASSERT(!(ValueCodeUtil::getStaticValueCode(*fu1, &negRuleData)).empty());
  }

  void testGetFormattedTotalNetAmt()
  {
    MoneyAmount amt = 345.3456;
    int numDecimal = 3;

    CPPUNIT_ASSERT_EQUAL(std::string("345.346"),
                         ValueCodeUtil::getFormattedTotalNetAmt(amt, numDecimal));
  }

  void
  prepareCreateStaticValueCodeCase(std::vector<const FareUsage*>& fareUsages,
                                   std::string staticValueCode1,
                                   char staticValueCodeCombInd1,
                                   std::string staticValueCode2 = "",
                                   char staticValueCodeCombInd2 = RuleConst::DISPLAY_OPTION_BLANK,
                                   std::string staticValueCode3 = "",
                                   char staticValueCodeCombInd3 = RuleConst::DISPLAY_OPTION_BLANK)
  {
    MockPaxTypeFare* ptf1 = _memHandle.insert(new MockPaxTypeFare("SMFC", "AA"));
    MockPaxTypeFare* ptf2 = _memHandle.insert(new MockPaxTypeFare("SMFC", "AA"));
    MockPaxTypeFare* ptf3 = _memHandle.insert(new MockPaxTypeFare("SMFC", "AA"));

    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    FareUsage* fu3 = _memHandle.create<FareUsage>();

    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    fu3->paxTypeFare() = ptf3;

    ptf1->negFareRestExt().staticValueCode() = staticValueCode1;
    ptf1->negFareRestExt().staticValueCodeCombInd() = staticValueCodeCombInd1;

    ptf2->negFareRestExt().staticValueCode() = staticValueCode2;
    ptf2->negFareRestExt().staticValueCodeCombInd() = staticValueCodeCombInd2;

    ptf3->negFareRestExt().staticValueCode() = staticValueCode3;
    ptf3->negFareRestExt().staticValueCodeCombInd() = staticValueCodeCombInd3;

    fareUsages.push_back(fu1);
    fareUsages.push_back(fu2);
    fareUsages.push_back(fu3);
  }

  void testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetTo1st()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages,
                                     "SVC1",
                                     RuleConst::DISPLAY_OPTION_1ST,
                                     "SVC1",
                                     RuleConst::DISPLAY_OPTION_1ST,
                                     "SVC1",
                                     RuleConst::DISPLAY_OPTION_1ST);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[0]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetTo1st()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages, "SVC1", RuleConst::DISPLAY_OPTION_1ST);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[0]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetTo2nd()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages,
                                     "SVC2",
                                     RuleConst::DISPLAY_OPTION_2ND,
                                     "SVC2",
                                     RuleConst::DISPLAY_OPTION_2ND,
                                     "SVC2",
                                     RuleConst::DISPLAY_OPTION_2ND);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[1]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetTo2nd()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(
        fareUsages, "", RuleConst::DISPLAY_OPTION_BLANK, "SVC2", RuleConst::DISPLAY_OPTION_2ND);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[1]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToAll()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages, "SVC1", 'B', "SVC2", 'B', "SVC3", 'B');

    CPPUNIT_ASSERT_EQUAL(std::string("SVC1/SVC2/SVC3"),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToAllCheckUnique()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages, "SVC", 'B', "SVC2", 'B', "SVC", 'B');

    CPPUNIT_ASSERT_EQUAL(std::string("SVC/SVC2"),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenAllFaresHaveSVCIndSetToBlank()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages,
                                     "SVC",
                                     RuleConst::DISPLAY_OPTION_BLANK,
                                     "SVC",
                                     RuleConst::DISPLAY_OPTION_BLANK,
                                     "SVC",
                                     RuleConst::DISPLAY_OPTION_BLANK);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[0]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenFare2HasntSVCandFare1HasSVCIndSetToBlank()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(
        fareUsages, "SVC", RuleConst::DISPLAY_OPTION_BLANK, "", RuleConst::DISPLAY_OPTION_BLANK);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[0]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenFare1HasntSVCandFare2HasSVCIndSetToBlank()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(
        fareUsages, "", RuleConst::DISPLAY_OPTION_BLANK, "SVC", RuleConst::DISPLAY_OPTION_BLANK);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[1]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenFare1And2HasntSVCandFare3HasSVCIndSetToBlank()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages,
                                     "",
                                     RuleConst::DISPLAY_OPTION_BLANK,
                                     "",
                                     RuleConst::DISPLAY_OPTION_BLANK,
                                     "SVC",
                                     RuleConst::DISPLAY_OPTION_BLANK);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[2]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void testCreateStaticValueCodeWhenOneFareAndSVCIndIsSetTo2nd()
  {
    std::vector<const FareUsage*> fareUsages;
    prepareCreateStaticValueCodeCase(fareUsages, "SVC", RuleConst::DISPLAY_OPTION_2ND);
    fareUsages.resize(1);

    CPPUNIT_ASSERT_EQUAL(static_cast<const MockPaxTypeFare*>(fareUsages[0]->paxTypeFare())
                             ->negFareRestExt()
                             .staticValueCode(),
                         ValueCodeUtil::createStaticValueCode(*_trx, fareUsages, *_typeInd));
  }

  void prepareSaveStaticValueCodeCase(FarePath& farePath,
                                      std::string staticValueCode,
                                      char staticValueCodeCombInd)
  {
    std::vector<const FareUsage*>* fareUsages = _memHandle.create<std::vector<const FareUsage*> >();
    prepareCreateStaticValueCodeCase(*fareUsages, staticValueCode, staticValueCodeCombInd);
    PricingUnit* pricingUnit = _memHandle.create<PricingUnit>();
    farePath.pricingUnit().push_back(pricingUnit);
    pricingUnit->fareUsage().swap(reinterpret_cast<std::vector<FareUsage*>&>(*fareUsages));

    CollectedNegFareData* fareDate = _memHandle.create<CollectedNegFareData>();
    farePath.collectedNegFareData() = fareDate;
  }

  void testSaveStaticValueCodeWhenHasValueCode()
  {
    FarePath farePath;
    prepareSaveStaticValueCodeCase(farePath, "SVC", RuleConst::DISPLAY_OPTION_1ST);

    ValueCodeUtil::saveStaticValueCode(*_trx, farePath);

    CPPUNIT_ASSERT_EQUAL(std::string("SVC"), farePath.collectedNegFareData()->valueCode());
  }

  void testSaveStaticValueCodeWhenNoStaticVC()
  {
    FarePath farePath;
    prepareSaveStaticValueCodeCase(farePath, "", RuleConst::DISPLAY_OPTION_BLANK);

    ValueCodeUtil::saveStaticValueCode(*_trx, farePath);

    CPPUNIT_ASSERT_EQUAL(std::string(""), farePath.collectedNegFareData()->valueCode());
  }

  void testGetStaticValueCodeInfoWhenHasStaticValueCode()
  {

    PricingTrx trx;
    FareUsage fareUsage;
    std::string staticValueCode;
    Indicator staticVCodeInd;

    MockPaxTypeFare mPTF("SMFC", "AA");
    fareUsage.paxTypeFare() = &mPTF;

    mPTF.negFareRestExt().staticValueCode() = "SVC";
    mPTF.negFareRestExt().staticValueCodeCombInd() = RuleConst::DISPLAY_OPTION_1ST;

    std::tuple<std::string&, Indicator&, Indicator&> codesTuple(
        staticValueCode, staticVCodeInd, staticVCodeInd);

    CPPUNIT_ASSERT(ValueCodeUtil::getNonEmptyStaticValueCodeInfo(trx, &fareUsage, codesTuple));
    CPPUNIT_ASSERT_EQUAL(mPTF.negFareRestExt().staticValueCode(), staticValueCode);
    CPPUNIT_ASSERT_EQUAL(mPTF.negFareRestExt().staticValueCodeCombInd(), staticVCodeInd);
  }

  void testGetStaticValueCodeInfoWhenStaticValueCodeIsEmpty()
  {

    PricingTrx trx;
    FareUsage fareUsage;
    std::string staticValueCode;
    Indicator staticVCodeInd;

    MockPaxTypeFare mPTF("SMFC", "AA");
    fareUsage.paxTypeFare() = &mPTF;

    mPTF.negFareRestExt().staticValueCode() = "";
    mPTF.negFareRestExt().staticValueCodeCombInd() = RuleConst::DISPLAY_OPTION_1ST;

    std::tuple<std::string&, Indicator&, Indicator&> codesTuple(
        staticValueCode, staticVCodeInd, staticVCodeInd);

    CPPUNIT_ASSERT(!ValueCodeUtil::getNonEmptyStaticValueCodeInfo(trx, &fareUsage, codesTuple));
    bool forcePassForNA = true;
    CPPUNIT_ASSERT(
        ValueCodeUtil::getStaticValueCodeInfo(trx, &fareUsage, codesTuple, forcePassForNA));
  }
  void testDecodeValueCodeInfoWhenBaseAMTOptionTrue()
  {
    FarePath farePath;
    CollectedNegFareData* cNegFareData = 0;
    _trx->getRequest()->ticketingAgent()->currencyCodeAgent() = "MYR";
    _trx->fareCalcConfig() = _fcConfig;
    _fcConfig->valueCodeBase() = 'Y';
    farePath = *createFarePath("", true);
    prepareSaveStaticValueCodeCase(farePath, "SVC", RuleConst::DISPLAY_OPTION_1ST);
    ValueCodeUtil::saveStaticValueCode(*_trx, farePath);
    //   farePath.collectedNegFareData() =  _memHandle.create<CollectedNegFareData>();
    cNegFareData = farePath.collectedNegFareData();
    cNegFareData->netTotalAmt() = 615.08;
    cNegFareData->totalMileageCharges() = 0;
    cNegFareData->otherSurchargeTotalAmt() = 0;
    CPPUNIT_ASSERT(!ValueCodeUtil::decodeValueCode(*_trx, farePath));
  }

private:
  TestMemHandle _memHandle;
  ValueCodeUtil* _valueCodeUtil;
  ValueCodeAlgorithm* _algorithm;
  PricingTrx* _trx;
  Indicator* _typeInd;
  FareCalcConfig* _fcConfig;
  Agent* _agent;
  Customer* _customer;
  PricingRequest* _request;
  PricingOptions* _options;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValueCodeUtilTest);
};
