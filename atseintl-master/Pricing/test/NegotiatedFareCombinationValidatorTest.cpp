#include <boost/assign/std/vector.hpp>
#include <string>

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/Tours.h"
#include "Pricing/NegotiatedFareCombinationValidator.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

namespace
{
class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(bool cat15HasSecurity = false) : PaxTypeFare()
  {
    _fareInfo.fareClass() = "MOCKMOCK";
    _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket);
    _fare.setCat15HasSecurity(cat15HasSecurity);

    setFare(&_fare);

    PaxTypeFare::_fareMarket = &this->_fareMarket;
  }

private:
  Fare _fare;
  FareInfo _fareInfo;
  FareMarket _fareMarket;
};

class NegPaxTypeFare : public MockPaxTypeFare
{
public:
  NegPaxTypeFare(const Indicator& bspMethod = RuleConst::NRR_METHOD_BLANK,
                 const Indicator& displayCatType = RuleConst::NET_SUBMIT_FARE,
                 bool with979 = false,
                 bool withCommission = false,
                 Indicator indNetGross = RuleConst::BLANK,
                 const std::string& cat27TourCode = "",
                 int noSegs = 1,
                 const Indicator& cat35TourCodeType1 = RuleConst::BLANK,
                 const std::string& cat35TourCode1 = "",
                 const Indicator& cat35TourCodeType2 = RuleConst::BLANK,
                 const std::string& cat35TourCode2 = "",
                 const Indicator& tktFareDataInd1 = RuleConst::BLANK)
    : MockPaxTypeFare()
  {
    _fareClassAppInfo._displayCatType = displayCatType;
    fareClassAppInfo() = &_fareClassAppInfo;

    _negFareRest.netRemitMethod() = bspMethod;
    _negFareRest.negFareCalcTblItemNo() = with979 ? 1000 : 0;
    _negFareRest.commPercent() = withCommission ? 50 : 0;
    _negFareRest.netGrossInd() = indNetGross;
    _negFareRest.noSegs() = noSegs;
    _negFareRest.tourBoxCodeType1() = cat35TourCodeType1;
    _negFareRest.tourBoxCode1() = cat35TourCode1;
    _negFareRest.tourBoxCodeType2() = cat35TourCodeType2;
    _negFareRest.tourBoxCode2() = cat35TourCode2;
    _negFareRest.tktFareDataInd1() = tktFareDataInd1;
    _status.set(PaxTypeFare::PTF_Negotiated);
    _negRuleData.ruleItemInfo() = &_negFareRest;
    _negAllRuleData.fareRuleData = &_negRuleData;
    (*(paxTypeFareRuleDataMap()))[RuleConst::NEGOTIATED_RULE] = &_negAllRuleData;

    _tours.tourNo() = cat27TourCode;
    _toursRuleData.ruleItemInfo() = &_tours;
    _toursAllRuleData.fareRuleData = &_toursRuleData;

    (*(paxTypeFareRuleDataMap()))[RuleConst::TOURS_RULE] = &_toursAllRuleData;
  }

  NegFareRest& negFareRest() { return _negFareRest; }
  const NegFareRest& negFareRest() const { return _negFareRest; }

private:
  FareClassAppInfo _fareClassAppInfo;
  PaxTypeFare::PaxTypeFareAllRuleData _negAllRuleData;
  NegPaxTypeFareRuleData _negRuleData;
  NegFareRest _negFareRest;

  Tours _tours;
  PaxTypeFareRuleData _toursRuleData;
  PaxTypeFareAllRuleData _toursAllRuleData;
};

class NegotiatedFareCombinationValidatorMock : public NegotiatedFareCombinationValidator
{
public:
  NegotiatedFareCombinationValidatorMock(PricingTrx& trx) : NegotiatedFareCombinationValidator(trx)
  {
  }

protected:
  virtual bool processNetRemitFareSelection(PricingTrx& trx,
                                            const FarePath& farePath,
                                            PricingUnit& pricingUnit,
                                            FareUsage& fareUsage,
                                            const NegFareRest& negFareRest) const
  {
    return false;
  }
};
} // anonymous namespace

class NegotiatedFareCombinationValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegotiatedFareCombinationValidatorTest);
  CPPUNIT_SKIP_TEST(testValidatePricingUnit_ToFewFareUsages_True);
  CPPUNIT_TEST(testValidatePricingUnit_TwoNetRemitFares_True);
  CPPUNIT_TEST(testValidatePricingUnit_TwoNetRemitFaresDifferentMethods_False);
  CPPUNIT_TEST(testValidateFarePath_ToFewPricingUnits_True);
  CPPUNIT_TEST(testValidateNegFareCombination_ToFewFares_True);
  CPPUNIT_TEST(testValidateNegFareCombination_TwoNetRemitFares_True);
  CPPUNIT_TEST(testValidateNegFareCombination_C35Security_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingL_NoT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingT_NoT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingT_WithT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingC_NoT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingC_WithT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_NetTicketingL_WithT979_NoCommission_And_Public_True);
  CPPUNIT_TEST(testValidateNegFareCombination_MixedFareTypes_False);
  CPPUNIT_TEST(testValidateNegFareCombination_SameTypes_MultipleBSPMethods_False);
  CPPUNIT_TEST(testValidateNetRemitCombination_BothNetRemitFares_True);
  CPPUNIT_TEST(testValidateNetRemitCombination_OneNetRemitFare_False);
  CPPUNIT_TEST(testValidateNetRemitCombination_NoNetRemitFares_True);
  CPPUNIT_TEST(testValidateMethodTypeCombination_SameMethods_True);
  CPPUNIT_TEST(testValidateMethodTypeCombination_DifferentMethods_False);
  CPPUNIT_TEST(testGetCat35Record3_WithNegotiatedRule_Record3Data);
  CPPUNIT_TEST(testGetCat35Record3_WithoutNegotiatedRule_Null);
  CPPUNIT_TEST(testGetCat35TktData_NegotiatedFare_Record3Data);
  CPPUNIT_TEST(testGetCat35TktData_PublicFare_Null);
  CPPUNIT_TEST(testGetBspMethodType_NoData_Blank);
  CPPUNIT_TEST(testGetBspMethodType_WithData);

  CPPUNIT_TEST(testValidateCommissionCombination_Different_NetGross_False);
  CPPUNIT_TEST(testValidateCommissionCombination_Same_NetGross_True);
  CPPUNIT_TEST(testValidateCommissionCombination_Same_CommPercent_True);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_SameTypes_True);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_DifferentTypes_False);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_BothBlank_True);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_CurrentFareBlank_True);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_PreviousFareBlank_True);
  CPPUNIT_TEST(testValidateTourCodeCombination_NonConflicting_True);
  CPPUNIT_TEST(testValidateTourCodeTypeCombination_CanBeSkippedForSoloCarnival);
  CPPUNIT_TEST(testPopulateAllPaxTypeFaresVector);
  CPPUNIT_TEST(test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_A_False);
  CPPUNIT_TEST(test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_F_True);
  CPPUNIT_TEST(test_getWarningMessage_CONFLICTING_TFD_BYTE101);
  CPPUNIT_TEST(test_getWarningMessage_TFD_RETRIEVE_FAIL);
  CPPUNIT_TEST(test_getWarningMessage_MIXED_FARE_BOX_AMT);

  CPPUNIT_TEST(testValidateAllTFDData_Pass_NoNetRemit);
  CPPUNIT_TEST(testValidateAllTFDData_Fail_OnNetRemit);

  CPPUNIT_TEST(testValidateFareBoxCombination_Pass);
  CPPUNIT_TEST(testValidateFareBoxCombination_Fail);

  CPPUNIT_TEST_SUITE_END();

protected:
  PricingTrx* _trx;
  PricingOptions* _options;
  NegotiatedFareCombinationValidator* _combinationValidator;
  PricingRequest* _request;
  Agent* _agent;
  Customer* _customer;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _trx = _memH.create<PricingTrx>();
    _options = _memH.create<PricingOptions>();
    _agent = _memH.create<Agent>();
    _customer = _memH.create<Customer>();
    _agent->agentTJR() = _customer;
    _request = _memH.create<PricingRequest>();
    _request->ticketingAgent() = _agent;
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _combinationValidator = _memH.insert(new NegotiatedFareCombinationValidatorMock(*_trx));
    _memH.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memH.clear();
  }

  void testValidatePricingUnit_ToFewFareUsages_True()
  {
    PricingUnit pricingUnit;

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validate(pricingUnit));
  }

  FarePath* createFarePath(const Indicator tktFareDataInd1 = RuleConst::BLANK)
  {
    FarePath* farePath = _memH.create<FarePath>();
    PricingUnit* pricingUnit = _memH.create<PricingUnit>();
    FareUsage* fareUsage1 = _memH.create<FareUsage>();
    FareUsage* fareUsage2 = _memH.create<FareUsage>();

    fareUsage1->paxTypeFare() = _memH.insert(new NegPaxTypeFare(RuleConst::NRR_METHOD_1,
                                                                RuleConst::NET_SUBMIT_FARE,
                                                                false,
                                                                true,
                                                                RuleConst::BLANK,
                                                                "",
                                                                1,
                                                                RuleConst::BLANK,
                                                                "",
                                                                RuleConst::BLANK,
                                                                "",
                                                                tktFareDataInd1));
    fareUsage2->paxTypeFare() = _memH.insert(new NegPaxTypeFare(RuleConst::NRR_METHOD_1,
                                                                RuleConst::NET_SUBMIT_FARE,
                                                                false,
                                                                true,
                                                                RuleConst::BLANK,
                                                                "",
                                                                1,
                                                                RuleConst::BLANK,
                                                                "",
                                                                RuleConst::BLANK,
                                                                "",
                                                                tktFareDataInd1));

    pricingUnit->fareUsage().push_back(fareUsage1);
    pricingUnit->fareUsage().push_back(fareUsage2);

    farePath->pricingUnit().push_back(pricingUnit);

    return farePath;
  }

  // Tests

  void testValidatePricingUnit_TwoNetRemitFares_True()
  {
    PricingUnit pricingUnit;
    FareUsage fareUsage1;
    FareUsage fareUsage2;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);

    fareUsage1.paxTypeFare() = &paxTypeFare1;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validate(pricingUnit));
  }

  void testValidatePricingUnit_TwoNetRemitFaresDifferentMethods_False()
  {
    PricingUnit pricingUnit;
    FareUsage fareUsage1;
    FareUsage fareUsage2;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2, RuleConst::NET_SUBMIT_FARE);

    fareUsage1.paxTypeFare() = &paxTypeFare1;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validate(pricingUnit));
  }

  void testValidateFarePath_ToFewPricingUnits_True()
  {
    FarePath farePath;

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validate(farePath));
  }

  void testValidateNegFareCombination_ToFewFares_True()
  {
    NegPaxTypeFare paxTypeFare;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
  }

  void testValidateNegFareCombination_TwoNetRemitFares_True()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_TwoSameNegotiatedFares_True()
  {
    NegPaxTypeFare paxTypeFare1;
    NegPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_C35Security_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_BLANK, RuleConst::SELLING_FARE);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingL_NoT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_BLANK, RuleConst::SELLING_FARE, false, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingT_NoT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_BLANK, RuleConst::NET_SUBMIT_FARE, false, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingT_WithT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_BLANK, RuleConst::NET_SUBMIT_FARE, true, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingC_NoT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_BLANK, RuleConst::NET_SUBMIT_FARE_UPD, false, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingC_WithT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_BLANK, RuleConst::NET_SUBMIT_FARE_UPD, true, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_NetTicketingL_WithT979_NoCommission_And_Public_True()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_BLANK, RuleConst::SELLING_FARE, true, false);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_MixedFareTypes_False()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    MockPaxTypeFare paxTypeFare2;
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::MIXED_FARES,
                         _combinationValidator->_warningCode);
  }

  void testValidateNegFareCombination_SameTypes_MultipleBSPMethods_False()
  {
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2, RuleConst::NET_SUBMIT_FARE);
    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateNegFareCombination(paxTypeFares));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::MULTIPLE_BSP,
                         _combinationValidator->_warningCode);
  }

  void testValidateNetRemitCombination_BothNetRemitFares_True()
  {
    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNetRemitCombination(true, true));
  }

  void testValidateNetRemitCombination_OneNetRemitFare_False()
  {
    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateNetRemitCombination(true, false));
    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateNetRemitCombination(false, true));
  }

  void testValidateNetRemitCombination_NoNetRemitFares_True()
  {
    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateNetRemitCombination(false, false));
  }

  void testValidateMethodTypeCombination_SameMethods_True()
  {
    CPPUNIT_ASSERT_EQUAL(true,
                         _combinationValidator->validateMethodTypeCombination(
                             RuleConst::NRR_METHOD_1, RuleConst::NRR_METHOD_1));
    CPPUNIT_ASSERT_EQUAL(true,
                         _combinationValidator->validateMethodTypeCombination(
                             RuleConst::NRR_METHOD_2, RuleConst::NRR_METHOD_2));
  }

  void testValidateMethodTypeCombination_DifferentMethods_False()
  {
    CPPUNIT_ASSERT_EQUAL(false,
                         _combinationValidator->validateMethodTypeCombination(
                             RuleConst::NRR_METHOD_5, RuleConst::NRR_METHOD_1));
    CPPUNIT_ASSERT_EQUAL(false,
                         _combinationValidator->validateMethodTypeCombination(
                             RuleConst::NRR_METHOD_5, RuleConst::NRR_METHOD_3));
  }

  void testGetCat35Record3_WithNegotiatedRule_Record3Data()
  {
    NegPaxTypeFare paxTypeFare;

    CPPUNIT_ASSERT(_combinationValidator->getCat35Record3(paxTypeFare));
  }

  void testGetCat35Record3_WithoutNegotiatedRule_Null()
  {
    PaxTypeFare paxTypeFare;

    CPPUNIT_ASSERT(!_combinationValidator->getCat35Record3(paxTypeFare));
  }

  void testGetCat35TktData_NegotiatedFare_Record3Data()
  {
    NegPaxTypeFare paxTypeFare;

    CPPUNIT_ASSERT(_combinationValidator->getCat35Record3(paxTypeFare));
  }

  void testGetCat35TktData_PublicFare_Null()
  {
    PaxTypeFare paxTypeFare;

    CPPUNIT_ASSERT(!_combinationValidator->getCat35Record3(paxTypeFare));
  }

  void testGetBspMethodType_NoData_Blank()
  {
    CPPUNIT_ASSERT_EQUAL(RuleConst::NRR_METHOD_BLANK, _combinationValidator->getBspMethodType(0));
  }

  void testGetBspMethodType_WithData()
  {
    NegFareRest negFareRest;

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_BLANK;
    CPPUNIT_ASSERT_EQUAL(RuleConst::NRR_METHOD_BLANK,
                         _combinationValidator->getBspMethodType(&negFareRest));

    negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_1;
    CPPUNIT_ASSERT_EQUAL(RuleConst::NRR_METHOD_1,
                         _combinationValidator->getBspMethodType(&negFareRest));
  }

  void testValidateCommissionCombination_Different_NetGross_False()
  {
    PricingUnit pricingUnit;
    FareUsage fareUsage1;
    FareUsage fareUsage2;

    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false, 'N');
    NegPaxTypeFare paxTypeFare2(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false, 'G');

    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateCommissionCombination(paxTypeFares));
  }

  void testValidateCommissionCombination_Same_NetGross_True()
  {
    PricingUnit pricingUnit;
    FareUsage fareUsage1;
    FareUsage fareUsage2;

    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false, 'G');
    NegPaxTypeFare paxTypeFare2(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false, 'G');

    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateCommissionCombination(paxTypeFares));
  }

  void testValidateCommissionCombination_Same_CommPercent_True()
  {
    PricingUnit pricingUnit;
    FareUsage fareUsage1;
    FareUsage fareUsage2;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, true);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, true);

    std::vector<const PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateCommissionCombination(paxTypeFares));
  }
  void testValidateTourCodeTypeCombination_SameTypes_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testValidateTourCodeTypeCombination_DifferentTypes_False()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'V',
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testValidateTourCodeTypeCombination_BothBlank_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                RuleConst::BLANK,
                                "TEST");
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                RuleConst::BLANK,
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testValidateTourCodeTypeCombination_CurrentFareBlank_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    NegPaxTypeFare paxTypeFare2(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false,
        RuleConst::BLANK, "TEST", 1, RuleConst::BLANK);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }
  void testValidateTourCodeTypeCombination_PreviousFareBlank_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(
        RuleConst::NRR_METHOD_1, RuleConst::NET_SUBMIT_FARE, false, false,
        RuleConst::BLANK, "TEST", 1, RuleConst::BLANK);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'V',
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testValidateTourCodeCombination_NonConflicting_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testValidateTourCodeTypeCombination_CanBeSkippedForSoloCarnival()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST");
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'V',
                                "TEST");

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    TestConfigInitializer::setValue("SKIP_TOUR_CODES_VALIDATION", "Y", "SOLO_CARNIVAL_OPT");

    _trx->getOptions()->setCarnivalSumOfLocal(true);
    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateTourCodeCombination(paxTypeFares));

    _trx->setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTourCodeCombination(paxTypeFares));

    _trx->getOptions()->setCarnivalSumOfLocal(false);
    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateTourCodeCombination(paxTypeFares));
  }

  void testPopulateAllPaxTypeFaresVector()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;
    _combinationValidator->getAllPaxTypeFares(*createFarePath(), paxTypeFares);

    CPPUNIT_ASSERT_EQUAL(2, (int)paxTypeFares.size());
  }

  void test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_A_False()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_A);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateTFDCombination(paxTypeFares));
  }

  void test_validateTFDCombination_Fare1Byte101_F_Fare2Byte101_F_True()
  {
    std::vector<const PaxTypeFare*> paxTypeFares;

    NegPaxTypeFare paxTypeFare1(RuleConst::NRR_METHOD_1,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);
    NegPaxTypeFare paxTypeFare2(RuleConst::NRR_METHOD_2,
                                RuleConst::NET_SUBMIT_FARE,
                                false,
                                false,
                                RuleConst::BLANK,
                                "TEST",
                                1,
                                'T',
                                "TEST",
                                'T',
                                "TEST",
                                RuleConst::NR_VALUE_F);

    paxTypeFares.push_back(&paxTypeFare1);
    paxTypeFares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateTFDCombination(paxTypeFares));
  }

  void test_getWarningMessage_CONFLICTING_TFD_BYTE101()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NET REMIT/FARE IND BYTE 101 CONFLICT"),
                         std::string(_combinationValidator->getWarningMessage(
                             NegotiatedFareCombinationValidator::CONFLICTING_TFD_BYTE101)));
  }

  void test_getWarningMessage_TFD_RETRIEVE_FAIL()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NET REMIT/UNA TO VAL TKT FARE DATA"),
                         std::string(_combinationValidator->getWarningMessage(
                             NegotiatedFareCombinationValidator::TFD_RETRIEVE_FAIL)));
  }

  void test_getWarningMessage_MIXED_FARE_BOX_AMT()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MIXED FARE BOX CUR/AMT"),
                         std::string(_combinationValidator->getWarningMessage(
                             NegotiatedFareCombinationValidator::MIXED_FARE_BOX_AMT)));
  }

  void testValidateAllTFDData_Pass_NoNetRemit()
  {
    std::vector<const PaxTypeFare*> ptfs;
    CPPUNIT_ASSERT(
        _combinationValidator->validateAllTFDData(*createFarePath(RuleConst::BLANK), ptfs));
  }

  void testValidateAllTFDData_Fail_OnNetRemit()
  {
    std::vector<const PaxTypeFare*> ptfs;
    CPPUNIT_ASSERT(
        !_combinationValidator->validateAllTFDData(*createFarePath(RuleConst::NR_VALUE_F), ptfs));
  }

  void testValidateFareBoxCombination_Pass()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().cur11() = "EUR";
    nptf1.negFareRest().fareAmt1() = 67890;
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().cur11() = "USD";
    nptf2.negFareRest().fareAmt1() = 12345;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf1, &nptf2;
    CPPUNIT_ASSERT_EQUAL(true, _combinationValidator->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::NO_WARNING,
                         _combinationValidator->_warningCode);
  }

  void testValidateFareBoxCombination_Fail()
  {
    NegPaxTypeFare nptf1('2');
    nptf1.negFareRest().cur11() = "EUR";
    nptf1.negFareRest().fareAmt1() = 67890;
    NegPaxTypeFare nptf2('2');
    nptf2.negFareRest().cur11() = "";
    nptf2.negFareRest().fareAmt1() = 0;
    std::vector<const PaxTypeFare*> ptfs;
    ptfs += &nptf1, &nptf2;
    CPPUNIT_ASSERT_EQUAL(false, _combinationValidator->validateFareBoxCombination(ptfs));
    CPPUNIT_ASSERT_EQUAL(NegotiatedFareCombinationValidator::MIXED_FARE_BOX_AMT,
                         _combinationValidator->_warningCode);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NegotiatedFareCombinationValidatorTest);
}
