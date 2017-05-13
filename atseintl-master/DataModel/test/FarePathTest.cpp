#include "DataModel/FarePath.h"

#include "Common/CustomerActivationUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareProperties.h"
#include "Fares/FareByRuleController.h"
#include "Rules/RuleConst.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"


namespace tse
{

class FarePathTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FarePathTest);
  CPPUNIT_TEST(testTktFareVendor_Empty_NoFares);
  CPPUNIT_TEST(testTktFareVendor_ATP);
  CPPUNIT_TEST(testTktFareVendor_SITA);
  CPPUNIT_TEST(testTktFareVendor_SMFCarrier);
  CPPUNIT_TEST(testTktFareVendor_SMFAbacus);
  CPPUNIT_TEST(testTktFareVendor_Cat35);
  CPPUNIT_TEST(testTktFareVendor_Cat35_new);
  CPPUNIT_TEST(testTktFareVendor_ATPWithSita);
  CPPUNIT_TEST(testTktFareVendor_ATPWithSmf);
  CPPUNIT_TEST(testTktFareVendor_SitaWithAtp);
  CPPUNIT_TEST(testTktFareVendor_SitaWithSmf);
  CPPUNIT_TEST(testTktFareVendor_SMFCarrierWithSmfAbacus);
  CPPUNIT_TEST(testTktFareVendor_Cat35WithATP);
  CPPUNIT_TEST(testTktFareVendor_Cat35WithSITA);
  CPPUNIT_TEST(testTktFareVendor_Cat35WithSMF);
  CPPUNIT_TEST(testTktFareVendor_OptimusWithATP);
  CPPUNIT_TEST(testTktFareVendor_OptimusWithSITA);
  CPPUNIT_TEST(testTktFareVendor_OptimusWithSMF);
  CPPUNIT_TEST(testTktFareVendor_SMFCOptimus);
  CPPUNIT_TEST(testTktFareVendor_SMFWithOptimus);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenAllFaresSetToYes);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenPresetToYes);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenPresetToNo);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenOneOfFaresSetToNo);

  CPPUNIT_TEST(testIsSpanishCombinedDiscountApplies_Pass_whenAtLeastOneFareValid);
  CPPUNIT_TEST(testIsSpanishCombinedDiscountApplies_Pass_whenAtLeastOneFareValid_fareTypeG);
  CPPUNIT_TEST(testIsSpanishCombinedDiscountApplies_Fail_whenNotAtleastOneFareValid);
  CPPUNIT_TEST(testIsSpanishCombinedDiscountApplies_Pass_WhenSpanishC25CombineNonSpanishC25);
  CPPUNIT_TEST(
      testIsSpanishCombinedDiscountApplies_Pass_WhenSpanishC25CombineNonSpanishC25_fareTypeG);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Fail_WhenFareCalcIndNotC);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Fail_WhenNotSpanishResidence);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Fail_WhenNotSpanishPTC);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Pass_WhenSpanishResidence);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Pass_WhenSpanishResidence_fareTypeG);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Pass_WhenSpanishC25CombineNonC25);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Pass_WhenSpanishC25CombineNonC25_fareTypeG);
  CPPUNIT_TEST(testIsResidentDiscountApplies_Fail_WhenSpanishC25CombineNonSpanishC25);
  CPPUNIT_TEST(testForbidCreditCardFOP);

  CPPUNIT_TEST(testDefaultValCxrPerSp);
  CPPUNIT_TEST(testSettlementPlanValCxrs);

  CPPUNIT_TEST(test_doesValCarriersHaveDiffComm_true);
  CPPUNIT_TEST(test_doesValCarriersHaveDiffComm_false);

  CPPUNIT_TEST(test_findFUWithPUNumberWithFirstTravelSeg_onePU);
  CPPUNIT_TEST(test_findFUWithPUNumberWithFirstTravelSeg_threePUs);
  CPPUNIT_TEST(test_findFUWithPUNumberWithFirstTravelSeg_sideTripPU);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
  }

  void tearDown() { _memHandle.clear(); }

  PaxTypeFare* addFare(const VendorCode& vendor)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu);

    FareUsage* fu = _memHandle.create<FareUsage>();
    pu->fareUsage().push_back(fu);

    PaxTypeFareMock* ptf = _memHandle.create<PaxTypeFareMock>();
    fu->paxTypeFare() = ptf;

    ptf->vendor() = vendor;

    return ptf;
  }

  NegPaxTypeFareRuleData*
  addNegotiatedFare(const VendorCode& vendor, const PseudoCityCode& creatorPCC)
  {
    PaxTypeFare* ptf = addFare(vendor);

    ptf->status().set(PaxTypeFare::PTF_Negotiated);
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;

    ruleData->creatorPCC() = creatorPCC;

    (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

    return ruleData;
  }

  void addOptimusFare(const LocCode& loc, const VendorCode& vendor = SMF_ABACUS_CARRIER_VENDOR_CODE)
  {
    NegPaxTypeFareRuleData* negRuleData =
        addNegotiatedFare(vendor, vendor == SITA_VENDOR_CODE ? PCC : std::string(""));
    FareProperties* fareProperties = _memHandle.create<FareProperties>();
    fareProperties->vendor() = vendor;
    fareProperties->fareSource() = loc;

    negRuleData->fareProperties() = fareProperties;

    TestConfigInitializer::setValue("ACTIVATE_OPTIMUS_NET_REMIT", "Y", "PRICING_SVC", true);
    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  }

  // TESTS
  void testTktFareVendor_Empty_NoFares()
  {
    CPPUNIT_ASSERT_EQUAL(std::string(""), _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_ATP()
  {
    addFare(ATPCO_VENDOR_CODE);
    addFare(ATPCO_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(ATPCO_VENDOR_CODE + 'C', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SITA()
  {
    addFare(SITA_VENDOR_CODE);
    addFare(SITA_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SITA_VENDOR_CODE, _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SMFCarrier()
  {
    addFare(SMF_CARRIER_VENDOR_CODE);
    addFare(SMF_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE, _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SMFAbacus()
  {
    addFare(SMF_ABACUS_CARRIER_VENDOR_CODE);
    addFare(SMF_ABACUS_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_ABACUS_CARRIER_VENDOR_CODE, _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_Cat35()
  {
    addNegotiatedFare(SITA_VENDOR_CODE, PCC);
    addNegotiatedFare(ATPCO_VENDOR_CODE, PCC);
    CPPUNIT_ASSERT_EQUAL(PCC, _farePath->tktFareVendor(*_trx));
  }

  void setGNRTestOnDateAndsamePCC()
  {
    DateTime tktDate = DateTime(2012, 03, 19);
    _trx->getRequest()->ticketingDT() = tktDate;
    std::string projCode = "GNR";
    ActivationResult* acResult = _memHandle.create<ActivationResult>();
    acResult->finalActvDate() = DateTime(2012, 03, 19);
    acResult->isActivationFlag() = true;
    _trx->projCACMapData().insert(std::make_pair(projCode, acResult));
  }

  void testTktFareVendor_Cat35_new()
  {
    setGNRTestOnDateAndsamePCC();
    addNegotiatedFare(SITA_VENDOR_CODE, PCC);
    addNegotiatedFare(ATPCO_VENDOR_CODE, PCC);
    CPPUNIT_ASSERT_EQUAL(PCC, _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_ATPWithSita()
  {
    addFare(ATPCO_VENDOR_CODE);
    addFare(SITA_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(ATPCO_VENDOR_CODE + "CX", _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_ATPWithSmf()
  {
    addFare(ATPCO_VENDOR_CODE);
    addFare(SMF_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SitaWithAtp()
  {
    addFare(SITA_VENDOR_CODE);
    addFare(ATPCO_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SITA_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SitaWithSmf()
  {
    addFare(SITA_VENDOR_CODE);
    addFare(SMF_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SMFCarrierWithSmfAbacus()
  {
    addFare(SMF_CARRIER_VENDOR_CODE);
    addFare(SMF_ABACUS_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_Cat35WithATP()
  {
    addNegotiatedFare(ATPCO_VENDOR_CODE, PCC);
    addFare(ATPCO_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(PCC + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_Cat35WithSITA()
  {
    addNegotiatedFare(SITA_VENDOR_CODE, PCC);
    addFare(SITA_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(PCC + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_Cat35WithSMF()
  {
    addNegotiatedFare(SMF_CARRIER_VENDOR_CODE, PCC);
    addFare(SMF_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(PCC + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_OptimusWithATP()
  {
    addOptimusFare("CXLOC");
    addFare(ATPCO_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(std::string("CXLOC"), _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_OptimusWithSITA()
  {
    addOptimusFare("CXLOC");
    addFare(SITA_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(std::string("CXLOC"), _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_OptimusWithSMF()
  {
    addOptimusFare("CXLOC");
    addFare(SMF_CARRIER_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(std::string("CXLOC"), _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SMFCOptimus()
  {
    addOptimusFare("CXLOC", SMF_CARRIER_VENDOR_CODE);
    addFare(ATPCO_VENDOR_CODE);

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testTktFareVendor_SMFWithOptimus()
  {
    addFare(SMF_CARRIER_VENDOR_CODE);
    addOptimusFare("CXLOC");

    CPPUNIT_ASSERT_EQUAL(SMF_CARRIER_VENDOR_CODE + 'X', _farePath->tktFareVendor(*_trx));
  }

  void testApplyNonIATARoundingWhenAllFaresSetToYes()
  {
    VendorCode vendor = "5KAD";
    addFare(vendor);
    addFare(vendor);

    CPPUNIT_ASSERT(!static_cast<const FarePath*>(_farePath)->applyNonIATARounding(*_trx));

    CPPUNIT_ASSERT(_farePath->applyNonIATARounding(*_trx));

    CPPUNIT_ASSERT(static_cast<const FarePath*>(_farePath)->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingWhenPresetToYes()
  {
    VendorCode vendor1 = "5KAD";
    VendorCode vendor2 = "5KAD";
    addFare(vendor1);
    PaxTypeFare* ptf = addFare(vendor1);
    _farePath->_applyNonIATARounding = YES;
    CPPUNIT_ASSERT(_farePath->applyNonIATARounding(*_trx));

    static_cast<PaxTypeFareMock*>(ptf)->vendor() = vendor2;
    CPPUNIT_ASSERT(_farePath->applyNonIATARounding(*_trx));

    CPPUNIT_ASSERT(static_cast<const FarePath*>(_farePath)->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingWhenPresetToNo()
  {
    VendorCode vendor = "5KAD";
    addFare(vendor);
    addFare(vendor);
    _farePath->_applyNonIATARounding = NO;
    CPPUNIT_ASSERT(!_farePath->applyNonIATARounding(*_trx));

    CPPUNIT_ASSERT(!static_cast<const FarePath*>(_farePath)->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingWhenOneOfFaresSetToNo()
  {
    VendorCode vendor1 = "5KAD";
    VendorCode vendor2 = "8CI1";
    addFare(vendor1);
    PaxTypeFare* ptf = addFare(vendor2);
    CPPUNIT_ASSERT(!_farePath->applyNonIATARounding(*_trx));

    static_cast<PaxTypeFareMock*>(ptf)->vendor() = vendor1;
    CPPUNIT_ASSERT(!_farePath->applyNonIATARounding(*_trx));
  }

  void testIsSpanishCombinedDiscountApplies_Pass_whenAtLeastOneFareValid()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'S';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'C';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptfRuleData2.isSpanishResidence() = true;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);
    FareClassAppSegInfo fcas2;
    ptf2->fareClassAppSegInfo() = &fcas2;
    fcas2._paxType = "ADR";

    CPPUNIT_ASSERT(SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
        _farePath->pricingUnit()));
  }

  void testIsSpanishCombinedDiscountApplies_Pass_whenAtLeastOneFareValid_fareTypeG()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'S';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'G';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptfRuleData2.isSpanishResidence() = true;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);
    FareClassAppSegInfo fcas2;
    ptf2->fareClassAppSegInfo() = &fcas2;
    fcas2._paxType = "ADR";

    CPPUNIT_ASSERT(SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
        _farePath->pricingUnit()));
  }

  void testIsSpanishCombinedDiscountApplies_Fail_whenNotAtleastOneFareValid()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'S';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'S';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);

    CPPUNIT_ASSERT(!SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
                       _farePath->pricingUnit()));
  }

  void testIsSpanishCombinedDiscountApplies_Pass_WhenSpanishC25CombineNonSpanishC25()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'C';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptfRuleData2.isSpanishResidence() = false;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);
    FareClassAppSegInfo fcas2;
    ptf2->fareClassAppSegInfo() = &fcas2;
    fcas2._paxType = "ADT";

    CPPUNIT_ASSERT(SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
        _farePath->pricingUnit()));
  }

  void testIsSpanishCombinedDiscountApplies_Pass_WhenSpanishC25CombineNonSpanishC25_fareTypeG()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'G';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'C';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptfRuleData2.isSpanishResidence() = false;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);
    FareClassAppSegInfo fcas2;
    ptf2->fareClassAppSegInfo() = &fcas2;
    fcas2._paxType = "ADT";

    CPPUNIT_ASSERT(SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
        _farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Fail_WhenFareCalcIndNotC()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'S';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);

    CPPUNIT_ASSERT(!SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Fail_WhenNotSpanishResidence()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = false;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);

    CPPUNIT_ASSERT(!SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Fail_WhenNotSpanishPTC()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADT";

    CPPUNIT_ASSERT(!SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Pass_WhenSpanishResidence()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    CPPUNIT_ASSERT(SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Pass_WhenSpanishResidence_fareTypeG()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'G';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    CPPUNIT_ASSERT(SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Pass_WhenSpanishC25CombineNonC25()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    FBRPaxTypeFareRuleData ptfRuleData2;
    addFare("ATP");

    CPPUNIT_ASSERT(SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Pass_WhenSpanishC25CombineNonC25_fareTypeG()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'G';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    FBRPaxTypeFareRuleData ptfRuleData2;
    addFare("ATP");

    CPPUNIT_ASSERT(SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testIsResidentDiscountApplies_Fail_WhenSpanishC25CombineNonSpanishC25()
  {
    FBRPaxTypeFareRuleData ptfRuleData1;
    PaxTypeFare* ptf1 = addFare("ATP");
    ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem1;
    fbrItem1.fareInd() = 'C';
    ptfRuleData1.ruleItemInfo() = &fbrItem1;
    ptfRuleData1.isSpanishResidence() = true;
    ptf1->setRuleData(25, _trx->dataHandle(), &ptfRuleData1);
    FareClassAppSegInfo fcas1;
    ptf1->fareClassAppSegInfo() = &fcas1;
    fcas1._paxType = "ADR";

    FBRPaxTypeFareRuleData ptfRuleData2;
    PaxTypeFare* ptf2 = addFare("ATP");
    ptf2->status().set(PaxTypeFare::PTF_FareByRule);
    FareByRuleItemInfo fbrItem2;
    fbrItem2.fareInd() = 'C';
    ptfRuleData2.ruleItemInfo() = &fbrItem2;
    ptfRuleData2.isSpanishResidence() = false;
    ptf2->setRuleData(25, _trx->dataHandle(), &ptfRuleData2);
    FareClassAppSegInfo fcas2;
    ptf2->fareClassAppSegInfo() = &fcas2;
    fcas2._paxType = "ADT";

    CPPUNIT_ASSERT(!SRFEUtil::isSpanishResidentDiscountAppliesOld(_farePath->pricingUnit()));
  }

  void testForbidCreditCardFOP()
  {
    addFare(ATPCO_VENDOR_CODE);
    PaxTypeFare* ptf2 = addFare(ATPCO_VENDOR_CODE);
    CPPUNIT_ASSERT(!_farePath->forbidCreditCardFOP());

    FareUsage& fu = *_farePath->pricingUnit().front()->fareUsage().front();
    fu.mutableForbiddenFop().set(Fare::FOP_CREDIT);
    CPPUNIT_ASSERT(_farePath->forbidCreditCardFOP());

    fu.mutableForbiddenFop().clear(Fare::FOP_CREDIT);
    fu.mutableForbiddenFop().set(Fare::FOP_CASH);
    CPPUNIT_ASSERT(!_farePath->forbidCreditCardFOP());

    fu.mutableForbiddenFop().clear(Fare::FOP_CASH);
    Fare fare2;
    ptf2->initialize(&fare2, 0, 0);
    fare2.mutableForbiddenFop().set(Fare::FOP_CREDIT);
    CPPUNIT_ASSERT(_farePath->forbidCreditCardFOP());
  }

  void testDefaultValCxrPerSp()
  {
    const SettlementPlanType spArc = "ARC";
    const SettlementPlanType spBsp = "BSP";
    const SettlementPlanType spGen = "GEN";
    const CarrierCode cxrAA = "AA";
    const CarrierCode cxrBB = "BB";
    _farePath->defaultValCxrPerSp()[spArc] = cxrAA;
    _farePath->defaultValCxrPerSp()[spBsp] = cxrBB;
    _farePath->defaultValCxrPerSp()[spGen];
    CPPUNIT_ASSERT( 3 == _farePath->defaultValCxrPerSp().size() );
    _farePath->defaultValCxrPerSp().clear();
    CPPUNIT_ASSERT( _farePath->defaultValCxrPerSp().empty() );
  }

  void testSettlementPlanValCxrs()
  {
    const SettlementPlanType ARC = "ARC";
    const SettlementPlanType BSP = "BSP";
    const SettlementPlanType GEN = "GEN";
    const CarrierCode AA = "AA";
    const CarrierCode BA = "BA";
    _farePath->settlementPlanValidatingCxrs()[ARC].push_back(AA);
    _farePath->settlementPlanValidatingCxrs()[ARC].push_back(BA);
    CPPUNIT_ASSERT( 1 == _farePath->settlementPlanValidatingCxrs().size() );
    CPPUNIT_ASSERT( 2 == _farePath->settlementPlanValidatingCxrs()[ARC].size() );

    _farePath->settlementPlanValidatingCxrs()[BSP].push_back(AA);
    _farePath->settlementPlanValidatingCxrs()[BSP].push_back(BA);
    CPPUNIT_ASSERT( 2 == _farePath->settlementPlanValidatingCxrs().size() );
    CPPUNIT_ASSERT( 2 == _farePath->settlementPlanValidatingCxrs()[BSP].size() );

    _farePath->settlementPlanValidatingCxrs()[GEN].push_back(AA);
    _farePath->settlementPlanValidatingCxrs()[GEN].push_back(BA);
    CPPUNIT_ASSERT( 3 == _farePath->settlementPlanValidatingCxrs().size() );
    CPPUNIT_ASSERT( 2 == _farePath->settlementPlanValidatingCxrs()[GEN].size() );

    _farePath->settlementPlanValidatingCxrs().clear();
    CPPUNIT_ASSERT( _farePath->settlementPlanValidatingCxrs().empty() );
  }

  // def exists but no comm and alt exists without comm
  // def has comm but alt exits without comm
  // def and alt exists with diff comm
  // def and multiple alt exists with diff comm
  void test_doesValCarriersHaveDiffComm_true()
  {
    _farePath->storeCommissionForValidatingCarrier("AA", 10);
    _farePath->storeCommissionForValidatingCarrier("BA", 10);
    bool ret = _farePath->doesValCarriersHaveDiffComm("AA");
    CPPUNIT_ASSERT(!ret);
  }

  void test_doesValCarriersHaveDiffComm_false()
  {
    _farePath->storeCommissionForValidatingCarrier("AA", 10);
    _farePath->storeCommissionForValidatingCarrier("BA", 20);
    bool ret = _farePath->doesValCarriersHaveDiffComm("AA");
    CPPUNIT_ASSERT(ret);
  }

  void test_findFUWithPUNumberWithFirstTravelSeg_onePU()
  {
    AirSeg segment;
    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&segment);
    PricingUnit pricingUnit;
    pricingUnit.fareUsage().push_back(&fareUsage);
    _farePath->pricingUnit().push_back(&pricingUnit);

    auto result = _farePath->findFUWithPUNumberWithFirstTravelSeg(&segment);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)&fareUsage, result.first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), result.second);
  }

  void test_findFUWithPUNumberWithFirstTravelSeg_threePUs()
  {
    const int SIZE = 3;
    AirSeg seg[SIZE];
    FareUsage fu[SIZE];
    PricingUnit pu[SIZE];
    for (int i = 0; i < SIZE; ++i)
    {
      fu[i].travelSeg().push_back(seg + i);
      pu[i].fareUsage().push_back(fu + i);
      _farePath->pricingUnit().push_back(pu + i);
    }
    std::pair<const FareUsage*, uint16_t> result[3] = {
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg),
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg + 1),
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg + 2)};

    CPPUNIT_ASSERT_EQUAL((const FareUsage*)fu, result[0].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), result[0].second);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)(fu + 1), result[1].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), result[1].second);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)(fu + 2), result[2].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), result[2].second);
  }

  void test_findFUWithPUNumberWithFirstTravelSeg_sideTripPU()
  {
    AirSeg seg[6];
    FareUsage fu[4];
    PricingUnit pu[2];
    fu[0].travelSeg().push_back(seg);
    fu[0].travelSeg().push_back(seg + 3);
    fu[1].travelSeg().push_back(seg + 1);
    fu[2].travelSeg().push_back(seg + 2);
    fu[3].travelSeg().push_back(seg + 4);
    fu[3].travelSeg().push_back(seg + 5);
    pu[0].fareUsage().push_back(fu);
    pu[0].fareUsage().push_back(fu + 3);
    pu[1].fareUsage().push_back(fu + 1);
    pu[1].fareUsage().push_back(fu + 2);
    fu[0].sideTripPUs().push_back(pu + 1);
    pu[0].sideTripPUs().push_back(pu + 1);
    _farePath->pricingUnit().push_back(pu);
    _farePath->pricingUnit().push_back(pu + 1);

    std::pair<const FareUsage*, uint16_t> result[4] = {
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg),
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg + 1),
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg + 2),
        _farePath->findFUWithPUNumberWithFirstTravelSeg(seg + 4)};

    CPPUNIT_ASSERT_EQUAL((const FareUsage*)fu, result[0].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), result[0].second);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)(fu + 1), result[1].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), result[1].second);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)(fu + 2), result[2].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), result[2].second);
    CPPUNIT_ASSERT_EQUAL((const FareUsage*)(fu + 3), result[3].first);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), result[3].second);
  }

private:
  TestMemHandle _memHandle;
  FarePath* _farePath;
  PricingTrx* _trx;

  static const std::string PCC;

  class PaxTypeFareMock : public PaxTypeFare
  {
  public:
    const VendorCode& vendor() const { return _vendor; }
    VendorCode& vendor() { return _vendor; }
    VendorCode _vendor;

    bool applyNonIATAR(const PricingTrx& trx,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const RuleNumber& ruleNumber)
    {
      return vendor == "5KAD";
    }
  };
};

const std::string FarePathTest::PCC = "PCC";

CPPUNIT_TEST_SUITE_REGISTRATION(FarePathTest);
}
