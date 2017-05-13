#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/RexFareBytesValidator.h"
#include "RexPricing/test/CommonRexPricing.h"

#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace std;

enum
{
  CXRAPPLITEMNO_CARRIER_INCLUDED = 1,
  CXRAPPLITEMNO_CARRIER_EXCLUDED,
  CXRAPPLITEMNO_CARRIER_DOLLAR
};

static const CarrierCode TEST_CARRIER = "AA";
static const CarrierCode OTHER_CARRIER = "LH";
static const CarrierCode CARRIER_AA = "AA";
static const CarrierCode CARRIER_LH = "LH";
static const CarrierCode CARRIER_BA = "BA";
static const CarrierCode CARRIER_UA = "UA";

using boost::assign::list_of;

class RexFareBytesValidatorStub : public RexFareBytesValidator
{
public:
  RexFareBytesValidatorStub(RexPricingTrx& trx,
                            const std::vector<const PaxTypeFare*>& allRepricePTFs,
                            uint16_t itinIndex,
                            const GenericRexMapper* grm)
    : RexFareBytesValidator(trx, allRepricePTFs, itinIndex, grm, nullptr)
  {
  }

protected:
  virtual void updateCaches(const ProcessTagInfo& pti, RepriceFareValidationResult result) {}
  virtual bool checkCaches(const ProcessTagInfo& pti, RepriceFareValidationResult& cachedResult)
  {
    return false;
  }
  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo)
  {
    static vector<CarrierApplicationInfo*> _returnedCxrApplTbl;
    static CarrierApplicationInfo testCarrierInclApp;
    static CarrierApplicationInfo testCarrierExclApp;
    static CarrierApplicationInfo otherCarrierInclApp;
    static CarrierApplicationInfo otherCarrierExclApp;
    static CarrierApplicationInfo dollarCarrier;

    testCarrierInclApp.applInd() = ' ';
    testCarrierInclApp.carrier() = TEST_CARRIER;

    testCarrierExclApp.applInd() = 'X';
    testCarrierExclApp.carrier() = TEST_CARRIER;

    otherCarrierInclApp.applInd() = ' ';
    otherCarrierInclApp.carrier() = OTHER_CARRIER;

    otherCarrierExclApp.applInd() = 'X';
    otherCarrierExclApp.carrier() = OTHER_CARRIER;

    dollarCarrier.applInd() = ' ';
    dollarCarrier.carrier() = DOLLAR_CARRIER;

    _returnedCxrApplTbl.clear();
    switch (itemNo)
    {
    case CXRAPPLITEMNO_CARRIER_INCLUDED:
      _returnedCxrApplTbl.push_back(&otherCarrierInclApp);
      _returnedCxrApplTbl.push_back(&testCarrierInclApp);
      break;
    case CXRAPPLITEMNO_CARRIER_EXCLUDED:
      _returnedCxrApplTbl.push_back(&dollarCarrier);
      _returnedCxrApplTbl.push_back(&testCarrierExclApp);
      break;
    case CXRAPPLITEMNO_CARRIER_DOLLAR:
      _returnedCxrApplTbl.push_back(&dollarCarrier);
      break;
    }
    return _returnedCxrApplTbl;
  }
};

class PaxTypeFareMock : public PaxTypeFare
{
public:
  PaxTypeFareMock() { _vendor = ATPCO_VENDOR_CODE; }
  PaxTypeFareMock(VendorCode vendor) : _vendor(vendor) {}
  const VendorCode& vendor() const { return _vendor; }
  const CarrierCode& carrier() const { return _carrier; }
  VendorCode _vendor;
  CarrierCode _carrier;
};

struct TestPreValidateFareCxrBytesData
{
  PaxTypeFare ptf;
  Fare fare;
  FareInfo fareInfo;
  FareMarket fm;
  TestPreValidateFareCxrBytesData(const char* cxr)
  {
    fareInfo.carrier() = cxr;
    fare.setFareInfo(&fareInfo);
    ptf.fareMarket() = &fm;
    ptf.setFare(&fare);
  }
};

class RexFareBytesValidatorTest : public CppUnit::TestFixture, public CommonRexPricing
{
  CPPUNIT_TEST_SUITE(RexFareBytesValidatorTest);
  CPPUNIT_TEST(testPrevalidatePassed);
  CPPUNIT_TEST(testPrevalidateNo988Passed);
  CPPUNIT_TEST(testPrevalidateFailed);

  CPPUNIT_TEST(testPermInternationalAllMappedNoIntl);
  CPPUNIT_TEST(testPermInternationalAllMapped);
  CPPUNIT_TEST(testPermInternationalAllNotMapped);
  CPPUNIT_TEST(testPermInternationalOneNotMappedFail);
  CPPUNIT_TEST(testPermInternationalOneNotMappedIntMapped);

  CPPUNIT_TEST(testPermDomesticAllMapped);
  CPPUNIT_TEST(testPermDomesticNoneMapped);

  // RSV migrated

  CPPUNIT_TEST(testCheckRTPass);
  CPPUNIT_TEST(testCheckRTFailWhenOwMayNotBeDoubled);
  CPPUNIT_TEST(testCheckRTFailWhenOwMayBeDoubled);
  CPPUNIT_TEST(testCheckOWPassWhenOwMayNotBeDoubled);
  CPPUNIT_TEST(testCheckOWPassWhenOwMayBeDoubled);
  CPPUNIT_TEST(testCheckOWFailWhenRoundTripMayNotBeHalved);
  CPPUNIT_TEST(testCheckOWRTPass);

  CPPUNIT_TEST(testCheckFareRulesBlank);
  CPPUNIT_TEST(testCheckFareRulesRuleNumberPass);
  CPPUNIT_TEST(testCheckFareRulesRuleNumberFail);
  CPPUNIT_TEST(testCheckFareRulesSameRulePass);
  CPPUNIT_TEST(testCheckFareRulesSameRuleFail);
  CPPUNIT_TEST(testCheckFareRulesNotSameRulePass);
  CPPUNIT_TEST(testCheckFareRulesNotSameRuleFail);
  CPPUNIT_TEST(testCheckFareRulesWildcardPass);
  CPPUNIT_TEST(testCheckFareRulesWildcardFail);

  CPPUNIT_TEST(testCheckFareTrfNumberPassZero);
  CPPUNIT_TEST(testCheckFareTrfNumberPass);
  CPPUNIT_TEST(testCheckFareTrfNumberFail);

  CPPUNIT_TEST(testCheckFareAmountBlank);
  CPPUNIT_TEST(testCheckFareAmountHigherEquallPass);
  CPPUNIT_TEST(testCheckFareAmountHigherEquallFail);
  CPPUNIT_TEST(testCheckFareAmountHigherPass);
  CPPUNIT_TEST(testCheckFareAmountHigherFail);

  CPPUNIT_TEST(testCheckFareAmountHigherEquallApplyReissueEchangeSameCurrencyPass);
  CPPUNIT_TEST(testCheckFareAmountHigherEquallApplyReissueEchangeSameCurrencyFail);
  CPPUNIT_TEST(testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrencyPass);
  CPPUNIT_TEST(testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrencyFail);
  CPPUNIT_TEST(
      testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCPass);
  CPPUNIT_TEST(
      testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCPass);
  CPPUNIT_TEST(
      testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCFail);
  CPPUNIT_TEST(
      testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCNotPresent1stNUCFailFinalFail);

  CPPUNIT_TEST(testCheckFareAmountHigherApplyReissueEchangeSameCurrencyPass);
  CPPUNIT_TEST(testCheckFareAmountHigherApplyReissueEchangeSameCurrencyFail);
  CPPUNIT_TEST(testCheckFareAmountHigherApplyReissueEchangeDifferentCurrencyPass);
  CPPUNIT_TEST(testCheckFareAmountHigherApplyReissueEchangeDifferentCurrencyFail);
  CPPUNIT_TEST(testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCPass);
  CPPUNIT_TEST(
      testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCPass);
  CPPUNIT_TEST(
      testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCFail);
  CPPUNIT_TEST(
      testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCNotPresent1stNUCFailFinalFail);

  CPPUNIT_TEST(testCheckFareNormalSpecialBlank);
  CPPUNIT_TEST(testCheckFareNormalSpecialNormalPass);
  CPPUNIT_TEST(testCheckFareNormalSpecialNormalFail);
  CPPUNIT_TEST(testCheckFareNormalSpecialSpecialPass);
  CPPUNIT_TEST(testCheckFareNormalSpecialSpecialFail);

  CPPUNIT_TEST(testCheckExcludePrivateIndicatorByteNotApplyNotAtpcoVendor);
  CPPUNIT_TEST(
      testCheckExcludePrivateIndicatorByteNotApplyPreviousFareComponentTariffNumberNotZeroPass);
  CPPUNIT_TEST(testCheckExcludePrivateIndicatorXFail);
  CPPUNIT_TEST(testCheckExcludePrivateIndicatorBlankPass);
  CPPUNIT_TEST(testCheckExcludePrivateIndicatorXnewFarePublicPass);
  CPPUNIT_TEST(testCheckExcludePrivateIndicatorXnewFarePrivatePass);
  CPPUNIT_TEST(testCheckExcludePrivateIndicatorXnewFarePublicFail);

  CPPUNIT_TEST(testCheckFareCxrApplTblSameFail_);
  CPPUNIT_TEST(testCheckFareCxrApplTblSameFail);
  CPPUNIT_TEST(testCheckFareCxrApplTblPass);
  CPPUNIT_TEST(testCheckFareCxrApplTblOverriding);
  CPPUNIT_TEST(testCheckFareCxrApplTblDollarCarrier);

  CPPUNIT_TEST(testFCC_FareIndOrFCCBlankNoRestrictionPass);
  CPPUNIT_TEST(testFCC_FareClassPass);
  CPPUNIT_TEST(testFCC_FareClassFail);
  CPPUNIT_TEST(testFCC_FareClassFamilyFareBadData);
  CPPUNIT_TEST(testFCC_FareClassFamilyFarePass);
  CPPUNIT_TEST(testFCC_FareClassFamilyFareFail);
  CPPUNIT_TEST(testFCC_FareClassFromSecondPosPass);
  CPPUNIT_TEST(testFCC_FareClassFromSecondPosFail);
  CPPUNIT_TEST(testFCC_FareTypePass);
  CPPUNIT_TEST(testFCC_FareTypeFail);

  CPPUNIT_TEST(testPreValidateFareCxrBytesPassOnCxr);
  CPPUNIT_TEST(testPreValidateFareCxrBytesPassOnCarrApp);
  CPPUNIT_TEST(testPreValidateFareCxrBytesFailOnCarrApp);
  CPPUNIT_TEST(testPreValidateFareCxrBytesFailNoCarrApp);

  CPPUNIT_TEST(testCheckSameIndPassWhenNotApply);
  CPPUNIT_TEST(testCheckSameIndPassWhenFareType);
  CPPUNIT_TEST(testCheckSameIndFailWhenFareTypeDifferent);
  CPPUNIT_TEST(testCheckSameIndPassWhenFareClass);
  CPPUNIT_TEST(testCheckSameIndFailWhenFareClassDifferent);
  CPPUNIT_TEST(testCheckFareRestrictionFailOnSameIndicator);

  CPPUNIT_TEST_SUITE_END();

public:
  RexFareBytesValidatorTest() : _nullPtr(0) {}

  void setUp()
  {
    CommonRexPricing::setUp();
    _memHandle.create<TestConfigInitializer>();

    _mapper = _memHandle(new GenericRexMapper(*_trx, _allRepricePTFs));
    _rfbv = _memHandle(new RexFareBytesValidatorStub(*_trx, *_allRepricePTFs, 0, _mapper));

    //_rfbv->setDiag(_memHandle(new Diag689Collector));

    // RSV migrated

    _ffExchangeItin = _memHandle.create<ExcItin>();
    addAirSegment(_ffExchangeItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "LON", "PAR", "LON", "PAR", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "PAR", "LON", "PAR", "LON", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "LON", "NYC", "LON", "NYC", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);
    _fm = _memHandle.create<FareMarket>();
  }

  void tearDown()
  {
    // turn on if you want to see how it goes
    // std::cout << "\n" << _rfbv->_mappingDiag.str();
    CommonRexPricing::tearDown();
  }

  ProcessTagInfo* createProcTagInfo(ReissueSequence* seq, FareCompInfo* fc)
  {
    ProcessTagInfo* tag = _memHandle.create<ProcessTagInfo>();
    tag->reissueSequence()->orig() = seq;
    tag->fareCompInfo() = fc;

    return tag;
  }

  FareBytesData* makeFareBytesData()
  {
    FareBytesData* fbd = _memHandle(new FareBytesData(makePTI()));
    return fbd;
  }

  ProcessTagInfo* makePTI()
  {
    ProcessTagInfo* pti = _memHandle(new ProcessTagInfo);
    pti->paxTypeFare() = makePTF();
    pti->reissueSequence()->orig() = _memHandle(new ReissueSequence);
    // below: fail if same rule, to limit setups
    const_cast<ReissueSequence*>(pti->reissueSequence()->orig())->ruleInd() = 'N';
    return pti;
  }

  void testPrevalidatePassed()
  {
    createPtiWithT988AndExcPtf();
    FareBytesData& fbd = *_memHandle(new FareBytesData(_pti));
    const_cast<RuleNumber&>((**fbd.processTags().begin()).paxTypeFare()->ruleNumber()) = "864";
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_DOLLAR;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;
    // fi2.carrier() = OTHER_CARRIER;
    CPPUNIT_ASSERT(_rfbv->validate(fbd, *_ptf));
  }

  void testPrevalidateNo988Passed()
  {
    FareBytesData& fbd = *makeFareBytesData();
    (**fbd.processTags().begin()).reissueSequence()->orig() = 0;
    CPPUNIT_ASSERT(_rfbv->validate(fbd, *makePTF()));
  }

  void testPrevalidateFailed()
  {
    CPPUNIT_ASSERT(!_rfbv->validate(*makeFareBytesData(), *makePTF()));
  }

  ProcessTagPermutation* makePermutation()
  {
    ProcessTagPermutation* p = _memHandle(new ProcessTagPermutation);

    for (int i = 0; i != 3; ++i)
    {
      ProcessTagInfo* pti = makePTI();
      pti->fareCompInfo() = _rfbv->_trx.exchangeItin().front()->fareComponent().at(i);
      p->processTags().push_back(pti);
    }

    return p;
  }

  void testPermInternationalAllMappedNoIntl()
  {
    setUpDirectCityCityMapping();
    _mapper->map();
    ProcessTagPermutation* p = makePermutation();
    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  void testPermInternationalAllMapped()
  {
    setUpDirectCityCityMapping();
    _mapper->map();
    ProcessTagPermutation* p = makePermutation();
    getPTF(1)->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  void testPermInternationalAllNotMapped()
  {
    ProcessTagPermutation* p = makePermutation();
    CPPUNIT_ASSERT(_rfbv->validate(*p));
  }

  void testPermInternationalOneNotMappedFail()
  {
    setUpDirectCityCityMapping();
    _mapper->map();
    ProcessTagPermutation* p = makePermutation();
    //_rfbv->_mapping[1] = _nullPtr;
    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  void testPermInternationalOneNotMappedIntMapped()
  {
    setUpDirectCityCityMapping();
    _mapper->map();
    ProcessTagPermutation* p = makePermutation();
    getPTF(1)->geoTravelType() = GeoTravelType::International;
    //        _rfbv->_mapping[0] = _nullPtr;
    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  void testPermDomesticAllMapped()
  {
    _rfbv->_trx.exchangeItin().front()->geoTravelType() = GeoTravelType::Domestic;
    setUpDirectCityCityMapping();
    _mapper->map();
    ProcessTagPermutation* p = makePermutation();
    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  void testPermDomesticNoneMapped()
  {
    _rfbv->_trx.exchangeItin().front()->geoTravelType() = GeoTravelType::Domestic;
    ProcessTagPermutation* p = makePermutation();

    CPPUNIT_ASSERT(!_rfbv->validate(*p));
  }

  // migrated from RSV

  void setUpReissueSequence(Indicator reissueSequenceOwrt, Indicator fareInfoOwrt)
  {
    createPaxTypeFareWithFareAndFareInfo();
    createPtiWithT988();
    _t988->owrt() = reissueSequenceOwrt;
    _fareInfo->owrt() = fareInfoOwrt;
  }

  FareMarket* FM(TravelSeg* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    if (p1)
      fm->travelSeg().push_back(p1);
    if (p2)
      fm->travelSeg().push_back(p2);
    if (p3)
      fm->travelSeg().push_back(p3);
    fm->setFCChangeStatus(-1);

    return fm;
  }

  FareCompInfo* FC(FareMarket* fm, uint16_t fareCompNumber = 0)
  {
    FareCompInfo* fc = _memHandle.create<FareCompInfo>();
    fc->fareCompNumber() = fareCompNumber;
    fc->fareMarket() = fm;

    return fc;
  }

  void addAirSegment(vector<TravelSeg*>& tvlSeg,
                     const LocCode& board,
                     const LocCode& off,
                     const LocCode& boardMCity = "",
                     const LocCode& offMCity = "",
                     bool unflown = false)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->unflown() = unflown;
    seg->origAirport() = board;
    seg->destAirport() = off;
    seg->segmentType() = Air;
    if (boardMCity == "")
    {
      seg->boardMultiCity() = board;
      seg->offMultiCity() = off;
    }
    else
    {
      seg->boardMultiCity() = boardMCity;
      seg->offMultiCity() = offMCity;
    }
    seg->segmentOrder() = tvlSeg.size() + 1;
    seg->pnrSegment() = tvlSeg.size() + 1;
    tvlSeg.push_back(seg);
  }

  void initPaxTypeFare(PaxTypeFare& ptf, const TariffCategory& tariff)
  {
    TariffCrossRefInfo* crossRefInf = _memHandle.create<TariffCrossRefInfo>();
    crossRefInf->tariffCat() = tariff;
    Fare* fare = _memHandle.create<Fare>();
    fare->setTariffCrossRefInfo(crossRefInf);
    ptf.initialize(fare, NULL, NULL);
  }

  void createPtiWithT988()
  {
    _t988 = _memHandle.create<ReissueSequence>();
    _pti = _memHandle.create<ProcessTagInfo>();
    _pti->reissueSequence()->orig() = _t988;
  }

  void createPtiWithT988AndExcPtf()
  {
    createPtiWithT988();
    _excPtf = _memHandle.create<PaxTypeFare>();
    _excFare = _memHandle.create<Fare>();
    _excFareInfo = _memHandle.create<FareInfo>();
    _excFare->setFareInfo(_excFareInfo);
    _excPtf->setFare(_excFare);
    _pti->paxTypeFare() = _excPtf;
  }

  void createPaxTypeFareWithFareAndFareInfo()
  {
    _ptf = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();
    _fare->setFareInfo(_fareInfo);
    _ptf->setFare(_fare);
  }

  void testCheckFareRulesBlank()
  {
    createPtiWithT988();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->ruleInd() = FareRestrictions::NOT_APPLICABLE;
    _fareInfo->ruleNumber() = "1234";
    CPPUNIT_ASSERT(_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesRuleNumberPass()
  {
    createPtiWithT988AndExcPtf();
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NUMBER;
    _t988->ruleNo() = "1234";

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = _t988->ruleNo();
    Fare* fare = _ptf->fare();
    fare->setFareInfo(_fareInfo);
    _ptf->setFare(fare);
    CPPUNIT_ASSERT(_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesRuleNumberFail()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->ruleNumber() = "999";
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NUMBER;
    _t988->ruleNo() = "1234";

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "444";

    CPPUNIT_ASSERT(!_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesSameRulePass()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->ruleNumber() = "444";
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_SAME_RULE;

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "444";

    CPPUNIT_ASSERT(_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesSameRuleFail()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->ruleNumber() = "555";
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_SAME_RULE;

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "444";
    Fare* fare = _ptf->fare();
    fare->setFareInfo(_fareInfo);
    _ptf->setFare(fare);

    CPPUNIT_ASSERT(!_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesNotSameRulePass()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->ruleNumber() = "555";
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NOT_THE_SAME;

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "444";
    Fare* fare = _ptf->fare();
    fare->setFareInfo(_fareInfo);
    _ptf->setFare(fare);

    CPPUNIT_ASSERT(_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesNotSameRuleFail()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->ruleNumber() = "444";
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NOT_THE_SAME;

    // Reprice fare
    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "444";

    CPPUNIT_ASSERT(!_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesWildcardPass()
  {
    createPtiWithT988AndExcPtf();
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NUMBER;
    _t988->ruleNo() = "12**";

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "1266";
    Fare* fare = _ptf->fare();
    fare->setFareInfo(_fareInfo);
    _ptf->setFare(fare);

    CPPUNIT_ASSERT(_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareRulesWildcardFail()
  {
    createPtiWithT988AndExcPtf();
    _t988->ruleInd() = FareRestrictions::RULE_INDICATOR_RULE_NUMBER;
    _t988->ruleNo() = "12**";

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->ruleNumber() = "1366";

    CPPUNIT_ASSERT(!_rfbv->checkFareRules(*_pti, *_ptf));
  }

  void testCheckFareTrfNumberPassZero()
  {
    createPtiWithT988();
    _t988->ruleTariffNo() = 0;

    FareClassAppInfo fareClassAppInfo;
    TariffCrossRefInfo tcr;
    createPaxTypeFareWithFareAndFareInfo();
    _fare->setTariffCrossRefInfo(&tcr);
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._ruleTariff = 1234;
    tcr.ruleTariff() = 1234;

    CPPUNIT_ASSERT(_rfbv->checkFareTrfNumber(*_pti, *_ptf));
  }

  void testCheckFareTrfNumberPass()
  {
    createPtiWithT988();
    _t988->ruleTariffNo() = 1234;

    FareClassAppInfo fareClassAppInfo;
    TariffCrossRefInfo tcr;
    createPaxTypeFareWithFareAndFareInfo();
    _fare->setTariffCrossRefInfo(&tcr);
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._ruleTariff = 1234;
    tcr.ruleTariff() = 1234;

    CPPUNIT_ASSERT(_rfbv->checkFareTrfNumber(*_pti, *_ptf));
  }

  void testCheckFareTrfNumberFail()
  {
    createPtiWithT988();
    _t988->ruleTariffNo() = 777;

    FareClassAppInfo fareClassAppInfo;
    TariffCrossRefInfo tcr;
    createPaxTypeFareWithFareAndFareInfo();
    _fare->setTariffCrossRefInfo(&tcr);
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._ruleTariff = 1234;
    tcr.ruleTariff() = 1234;

    CPPUNIT_ASSERT(!_rfbv->checkFareTrfNumber(*_pti, *_ptf));
  }

  void testCheckFareAmountBlank()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::NOT_APPLICABLE;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = 50.00;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));

    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.0;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.00;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherFail()
  {
    createPtiWithT988AndExcPtf();
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _ptf->nucFareAmount() = _excPtf->nucFareAmount();

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));

    _ptf->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallApplyReissueEchangeSameCurrencyPass()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->fareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->fareAmount() = _excPtf->fareAmount();

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "USD";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));

    _fareInfo->fareAmount() = _excPtf->fareAmount() + 20.0;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallApplyReissueEchangeSameCurrencyFail()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->fareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->fareAmount() = _excPtf->fareAmount() - 20.00;

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "USD";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrencyPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));

    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.0;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrencyFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));

    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.0;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;
    _ptf->rexSecondNucFareAmount() = _excPtf->nucFareAmount();

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));

    _ptf->rexSecondNucFareAmount() = _excPtf->nucFareAmount() + 20.0;

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;
    _ptf->rexSecondNucFareAmount() = _excPtf->nucFareAmount() - 10.00;

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherEquallApplyReissueEchangeDifferentCurrency2NUCNotPresent1stNUCFailFinalFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_OR_EQUAL_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;
    _ptf->rexSecondNucFareAmount() = 0.0;

    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherApplyReissueEchangeSameCurrencyPass()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->fareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->fareAmount() = _excPtf->fareAmount() + 20.00;
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "USD";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherApplyReissueEchangeSameCurrencyFail()
  {
    createPtiWithT988AndExcPtf();
    _excFareInfo->fareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->fareAmount() = _excPtf->fareAmount();
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "USD";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));

    _fareInfo->fareAmount() = _excPtf->fareAmount() - 20.00;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherApplyReissueEchangeDifferentCurrencyPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.00;
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherApplyReissueEchangeDifferentCurrencyFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));

    _fare->nucFareAmount() = _excPtf->nucFareAmount() - 20.00;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount() + 20.00;
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCPass()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();
    _ptf->rexSecondNucFareAmount() = _excPtf->nucFareAmount() + 20.00;
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCPresent1stNUCFail2ndNUCFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));

    _ptf->rexSecondNucFareAmount() = _excPtf->nucFareAmount() - 20.00;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void
  testCheckFareAmountHigherApplyReissueEchangeDifferentCurrency2NUCNotPresent1stNUCFailFinalFail()
  {
    createPtiWithT988AndExcPtf();
    _excFare->nucFareAmount() = 100.00;
    _t988->fareAmtInd() = FareRestrictions::HIGHER_AMOUNT;

    createPaxTypeFareWithFareAndFareInfo();
    _fare->nucFareAmount() = _excPtf->nucFareAmount();
    _trx->setRexPrimaryProcessType('A');
    _excFareInfo->currency() = "USD";
    _fareInfo->currency() = "GBP";

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));

    _ptf->rexSecondNucFareAmount() = 0.0;

    CPPUNIT_ASSERT(!_rfbv->checkFareAmount(*_pti, *_ptf));
  }

  void testCheckFareNormalSpecialBlank()
  {
    createPtiWithT988();
    _t988->normalspecialInd() = CommonSolutionValidator::BLANK;

    FareClassAppInfo fareClassAppInfo;
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::NORMAL_FARE;
    CPPUNIT_ASSERT(_rfbv->checkFareNormalSpecial(*_pti, *_ptf));

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::SPECIAL_FARE;
    CPPUNIT_ASSERT(_rfbv->checkFareNormalSpecial(*_pti, *_ptf));
  }

  void testCheckFareNormalSpecialNormalPass()
  {
    createPtiWithT988();
    _t988->normalspecialInd() = CommonSolutionValidator::NORMAL_FARE;

    FareClassAppInfo fareClassAppInfo;
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::NORMAL_FARE;
    CPPUNIT_ASSERT(_rfbv->checkFareNormalSpecial(*_pti, *_ptf));
  }

  void testCheckFareNormalSpecialNormalFail()
  {
    createPtiWithT988();
    _t988->normalspecialInd() = CommonSolutionValidator::NORMAL_FARE;

    FareClassAppInfo fareClassAppInfo;
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::SPECIAL_FARE;
    CPPUNIT_ASSERT(!_rfbv->checkFareNormalSpecial(*_pti, *_ptf));
  }

  void testCheckFareNormalSpecialSpecialPass()
  {
    createPtiWithT988();
    _t988->normalspecialInd() = CommonSolutionValidator::SPECIAL_FARE;

    FareClassAppInfo fareClassAppInfo;
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::SPECIAL_FARE;
    CPPUNIT_ASSERT(_rfbv->checkFareNormalSpecial(*_pti, *_ptf));
  }

  void testCheckFareNormalSpecialSpecialFail()
  {
    createPtiWithT988();
    _pti->reissueSequence()->orig() = _t988;
    _t988->normalspecialInd() = CommonSolutionValidator::SPECIAL_FARE;

    FareClassAppInfo fareClassAppInfo;
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = &fareClassAppInfo;

    fareClassAppInfo._pricingCatType = CommonSolutionValidator::NORMAL_FARE;
    CPPUNIT_ASSERT(!_rfbv->checkFareNormalSpecial(*_pti, *_ptf));
  }

  void testCheckExcludePrivateIndicatorByteNotApplyNotAtpcoVendor()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf(SITA_VENDOR_CODE);
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PRIVATE_TARIFF);
    _pti->paxTypeFare() = &oldPtf;
    const_cast<int&>(_pti->reissueSequence()->ruleTariffNo()) = 0;

    CPPUNIT_ASSERT(_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorByteNotApplyPreviousFareComponentTariffNumberNotZeroPass()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PUBLIC_VENDOR);
    _pti->paxTypeFare() = &oldPtf;
    const_cast<int&>(_pti->reissueSequence()->ruleTariffNo()) = 5;

    CPPUNIT_ASSERT(_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorXFail()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PUBLIC_VENDOR);
    _pti->paxTypeFare() = &oldPtf;
    _t988->ruleTariffNo() = 0;

    initPaxTypeFare(newPtf, RuleConst::PRIVATE_TARIFF);
    _t988->excludePrivate() = 'X';

    CPPUNIT_ASSERT(!_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorBlankPass()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PUBLIC_VENDOR);
    _pti->paxTypeFare() = &oldPtf;
    _t988->ruleTariffNo() = 0;

    initPaxTypeFare(newPtf, RuleConst::PRIVATE_TARIFF);
    _t988->excludePrivate() = ' ';

    CPPUNIT_ASSERT(_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorXnewFarePublicPass()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PUBLIC_VENDOR);
    _pti->paxTypeFare() = &oldPtf;
    _t988->ruleTariffNo() = 0;

    initPaxTypeFare(newPtf, RuleConst::PUBLIC_VENDOR);
    _t988->excludePrivate() = 'X';

    CPPUNIT_ASSERT(_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorXnewFarePrivatePass()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PRIVATE_TARIFF);
    _pti->paxTypeFare() = &oldPtf;
    _t988->ruleTariffNo() = 0;

    initPaxTypeFare(newPtf, RuleConst::PRIVATE_TARIFF);
    _t988->excludePrivate() = 'X';

    CPPUNIT_ASSERT(_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckExcludePrivateIndicatorXnewFarePublicFail()
  {
    createPtiWithT988();
    PaxTypeFareMock newPtf;
    PaxTypeFare oldPtf;

    initPaxTypeFare(oldPtf, RuleConst::PRIVATE_TARIFF);
    _pti->paxTypeFare() = &oldPtf;
    _t988->ruleTariffNo() = 0;

    initPaxTypeFare(newPtf, RuleConst::PUBLIC_VENDOR);
    _t988->excludePrivate() = 'X';

    CPPUNIT_ASSERT(!_rfbv->checkExcludePrivateIndicator(*_pti, newPtf));
  }

  void testCheckFareCxrApplTblSameFail_()
  {
    createPtiWithT988();
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_INCLUDED;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;

    _pti->paxTypeFare() = _ptf;

    CPPUNIT_ASSERT(!_rfbv->checkFareCxrApplTbl(*_pti, *_ptf));
  }

  void testCheckFareCxrApplTblSameFail()
  {
    createPtiWithT988();
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_EXCLUDED;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;
    Fare* fare = _ptf->fare();
    fare->setFareInfo(_fareInfo);
    _ptf->setFare(fare);

    PaxTypeFare ptf2;
    Fare fare2;
    FareInfo fi2;
    fi2.carrier() = OTHER_CARRIER;
    fare2.setFareInfo(&fi2);
    ptf2.setFare(&fare2);

    _pti->paxTypeFare() = &ptf2;

    CPPUNIT_ASSERT(!_rfbv->checkFareCxrApplTbl(*_pti, *_ptf));
  }

  void testCheckFareCxrApplTblPass()
  {
    createPtiWithT988();
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_INCLUDED;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;
    _ptf->setFare(_fare);

    PaxTypeFare ptf2;
    Fare fare2;
    FareInfo fi2;
    fi2.carrier() = OTHER_CARRIER;
    fare2.setFareInfo(&fi2);
    ptf2.setFare(&fare2);

    _pti->paxTypeFare() = &ptf2;

    CPPUNIT_ASSERT(_rfbv->checkFareCxrApplTbl(*_pti, *_ptf));
  }

  void testCheckFareCxrApplTblOverriding()
  {
    createPtiWithT988();
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_INCLUDED;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;
    _fareInfo->carrier() = TEST_CARRIER;
    _ptf->setFare(_fare);

    PaxTypeFare ptf2;
    Fare fare2;
    FareInfo fi2;
    fi2.carrier() = OTHER_CARRIER;
    fare2.setFareInfo(&fi2);
    ptf2.setFare(&fare2);

    PaxTypeFare ptf3;
    Fare fare3;
    FareInfo fi3;
    fi3.carrier() = TEST_CARRIER;
    fare3.setFareInfo(&fi3);
    ptf3.setFare(&fare3);

    _pti->paxTypeFare() = &ptf2;
    _pti->overridingPTF() = &ptf3;

    VoluntaryChangesInfo record3;

    _pti->record3()->overriding() = &record3;

    CPPUNIT_ASSERT(_rfbv->checkFareCxrApplTbl(*_pti, *_ptf));
  }

  void testCheckFareCxrApplTblDollarCarrier()
  {
    createPtiWithT988();
    _t988->fareCxrApplTblItemNo() = CXRAPPLITEMNO_CARRIER_DOLLAR;

    createPaxTypeFareWithFareAndFareInfo();
    _fareInfo->carrier() = TEST_CARRIER;

    PaxTypeFare ptf2;
    Fare fare2;
    FareInfo fi2;
    fi2.carrier() = OTHER_CARRIER;
    fare2.setFareInfo(&fi2);
    ptf2.setFare(&fare2);

    _pti->paxTypeFare() = &ptf2;

    CPPUNIT_ASSERT(_rfbv->checkFareCxrApplTbl(*_pti, *_ptf));
  }

  void testFCC_FareIndOrFCCBlankNoRestrictionPass()
  {
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(_memHandle.create<FareInfo>());

    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::NOT_APPLICABLE;
    seq->fareClass() = "";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));

    seq->fareTypeInd() = FareRestrictions::NOT_APPLICABLE;
    seq->fareClass() = "Y";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FARE_CLASS;
    seq->fareClass() = "";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassPass()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FARE_CLASS;
    seq->fareClass() = "LOWGB";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "LOWGB";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFail()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FARE_CLASS;
    seq->fareClass() = "LOWGB";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "LOWGB1";

    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "LOWG";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFamilyFareBadData()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FAMILY_FARE;
    seq->fareClass() = "LOWGB";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "YCLOWGB";

    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFamilyFarePass()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FAMILY_FARE;
    seq->fareClass() = "-LOWGB50";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "YCLOWGB50";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));

    seq->fareClass() = "FR-LOWGB50";
    fi->fareClass() = "FRYCLOWGB50X";
    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "FRLOWGB50";
    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFamilyFareFail()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_FAMILY_FARE;
    seq->fareClass() = "-LOWGB50";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "YCLOWGB501";

    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "YCLOWGB5";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "LOWGB50";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    seq->fareClass() = "FR-LOWGB50";
    fi->fareClass() = "FRYCLOWGB501";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "FRYCLOWGB5";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFromSecondPosPass()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_SECOND_POS;
    seq->fareClass() = "LOWGB";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "CLOWGB";

    CPPUNIT_ASSERT(_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareClassFromSecondPosFail()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareTypeInd() = FareRestrictions::FARE_TYPE_IND_SECOND_POS;
    seq->fareClass() = "LOWGB";
    FareInfo* fi = _memHandle.create<FareInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fare()->setFareInfo(fi);
    fi->fareClass() = "LOWGB";

    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "LOWG";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "CLOWGB1";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    fi->fareClass() = "CLOWG";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));

    seq->fareClass() = "-LOWGB";
    fi->fareClass() = "CLOWGB";
    CPPUNIT_ASSERT(!_rfbv->checkFareClassCode(*tag, *_ptf));
  }

  void testFCC_FareTypePass()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareType() = "XAP";
    FareClassAppInfo* fcai = _memHandle.create<FareClassAppInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = fcai;
    fcai->_fareType = "XAP";

    CPPUNIT_ASSERT(_rfbv->checkFareTypeCode(*tag, *_ptf));
  }

  void testFCC_FareTypeFail()
  {
    ReissueSequence* seq = _memHandle.create<ReissueSequence>();
    ProcessTagInfo* tag = createProcTagInfo(seq, FC(FM(_ffExchangeItin->travelSeg()[1])));

    seq->fareType() = "XAP";
    FareClassAppInfo* fcai = _memHandle.create<FareClassAppInfo>();
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareClassAppInfo() = fcai;
    fcai->_fareType = "XAN";

    CPPUNIT_ASSERT(!_rfbv->checkFareTypeCode(*tag, *_ptf));
  }

  void testCheckRTPass()
  {
    setUpReissueSequence(ROUND_TRIP_MAYNOT_BE_HALVED, ROUND_TRIP_MAYNOT_BE_HALVED);
    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckRTFailWhenOwMayNotBeDoubled()
  {
    setUpReissueSequence(ROUND_TRIP_MAYNOT_BE_HALVED, ONE_WAY_MAYNOT_BE_DOUBLED);
    CPPUNIT_ASSERT(!_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckRTFailWhenOwMayBeDoubled()
  {
    setUpReissueSequence(ROUND_TRIP_MAYNOT_BE_HALVED, ONE_WAY_MAY_BE_DOUBLED);
    CPPUNIT_ASSERT(!_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckOWPassWhenOwMayBeDoubled()
  {
    setUpReissueSequence(ONE_WAY_MAY_BE_DOUBLED, ONE_WAY_MAY_BE_DOUBLED);
    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckOWPassWhenOwMayNotBeDoubled()
  {
    setUpReissueSequence(ONE_WAY_MAY_BE_DOUBLED, ONE_WAY_MAYNOT_BE_DOUBLED);
    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckOWFailWhenRoundTripMayNotBeHalved()
  {
    setUpReissueSequence(ONE_WAY_MAY_BE_DOUBLED, ROUND_TRIP_MAYNOT_BE_HALVED);
    CPPUNIT_ASSERT(!_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testCheckOWRTPass()
  {
    setUpReissueSequence(ONE_WAY_MAY_BE_DOUBLED, ONE_WAY_MAY_BE_DOUBLED);

    _t988->owrt() = ALL_WAYS;

    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));

    _fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;

    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));

    _fareInfo->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;

    CPPUNIT_ASSERT(_rfbv->checkOWRT(*_pti, *_ptf));
  }

  void testPreValidateFareCxrBytesPassOnCxr()
  {
    createPtiWithT988();
    createPaxTypeFareWithFareAndFareInfo();
    TestPreValidateFareCxrBytesData fbd("AA");

    _t988->fareCxrApplTblItemNo() = 1;
    _ptf->fareMarket() = _fm;
    _pti->paxTypeFare() = _ptf;
    _fareInfo->carrier() = CARRIER_AA;

    CPPUNIT_ASSERT(_rfbv->checkFareCxrApplTbl(*_pti, fbd.ptf));
  }

  void testPreValidateFareCxrBytesPassOnCarrApp()
  {
    createPtiWithT988();
    createPaxTypeFareWithFareAndFareInfo();
    TestPreValidateFareCxrBytesData fbd("AA");

    _t988->fareCxrApplTblItemNo() = 1;
    _ptf->fareMarket() = _fm;
    _pti->paxTypeFare() = _ptf;
    _fareInfo->carrier() = CARRIER_BA;

    CPPUNIT_ASSERT(_rfbv->checkFareCxrApplTbl(*_pti, fbd.ptf));
  }

  void testPreValidateFareCxrBytesFailOnCarrApp()
  {
    createPtiWithT988();
    createPaxTypeFareWithFareAndFareInfo();
    TestPreValidateFareCxrBytesData fbd("LO");

    _t988->fareCxrApplTblItemNo() = 1;
    _ptf->fareMarket() = _fm;
    _pti->paxTypeFare() = _ptf;
    _fareInfo->carrier() = CARRIER_BA;

    CPPUNIT_ASSERT(!_rfbv->checkFareCxrApplTbl(*_pti, fbd.ptf));
  }

  void testPreValidateFareCxrBytesFailNoCarrApp()
  {
    createPtiWithT988();
    createPaxTypeFareWithFareAndFareInfo();
    TestPreValidateFareCxrBytesData fbd("LO");

    _t988->fareCxrApplTblItemNo() = 0;
    _ptf->fareMarket() = _fm;
    _pti->paxTypeFare() = _ptf;
    _fareInfo->carrier() = CARRIER_BA;

    CPPUNIT_ASSERT(!_rfbv->checkFareCxrApplTbl(*_pti, fbd.ptf));
  }

  void testCheckSameIndPassWhenNotApply()
  {
    createPtiWithT988();
    _t988->sameInd() = FareRestrictions::NOT_APPLICABLE;

    CPPUNIT_ASSERT(_rfbv->checkSameInd(*_pti, *_ptf));
  }

  void testCheckSameIndPassWhenFareType()
  {
    createPtiWithT988AndExcPtf();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->sameInd() = FareRestrictions::SAME_IND_FARE_TYPE;
    FareClassAppInfo fca1, fca2;
    fca1._fareType = "XPN";
    fca2._fareType = "XPN";
    _ptf->fareClassAppInfo() = &fca1;
    _excPtf->fareClassAppInfo() = &fca2;

    CPPUNIT_ASSERT(_rfbv->checkSameInd(*_pti, *_ptf));
  }

  void testCheckSameIndFailWhenFareTypeDifferent()
  {
    createPtiWithT988AndExcPtf();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->sameInd() = FareRestrictions::SAME_IND_FARE_TYPE;
    FareClassAppInfo fca1, fca2;
    fca1._fareType = "XPN";
    fca2._fareType = "XEX";
    _ptf->fareClassAppInfo() = &fca1;
    _excPtf->fareClassAppInfo() = &fca2;

    CPPUNIT_ASSERT(!_rfbv->checkSameInd(*_pti, *_ptf));
  }

  void testCheckSameIndPassWhenFareClass()
  {
    createPtiWithT988AndExcPtf();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->sameInd() = FareRestrictions::SAME_IND_FARE_CLASS;
    _fareInfo->fareClass() = "ABC";
    _excFareInfo->fareClass() = "ABC";

    CPPUNIT_ASSERT(_rfbv->checkSameInd(*_pti, *_ptf));
  }

  void testCheckSameIndFailWhenFareClassDifferent()
  {
    createPtiWithT988AndExcPtf();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->sameInd() = FareRestrictions::SAME_IND_FARE_CLASS;
    _fareInfo->fareClass() = "ABC";
    _excFareInfo->fareClass() = "CBA";

    CPPUNIT_ASSERT(!_rfbv->checkSameInd(*_pti, *_ptf));
  }

  void testCheckFareRestrictionFailOnSameIndicator()
  {
    createPtiWithT988AndExcPtf();
    createPaxTypeFareWithFareAndFareInfo();
    _t988->sameInd() = FareRestrictions::SAME_IND_FARE_CLASS;
    _fareInfo->fareClass() = "ABC";
    _excFareInfo->fareClass() = "CBA";

    CPPUNIT_ASSERT(!_rfbv->checkSameInd(*_pti, *_ptf));
  }

private:
  RexFareBytesValidator* _rfbv;
  const PaxTypeFare* _nullPtr;
  GenericRexMapper* _mapper;

  // migrated from RSV
  ProcessTagInfo* _pti;
  ReissueSequence* _t988;
  FareInfo* _fareInfo;
  FareInfo* _excFareInfo;
  Fare* _fare;
  Fare* _excFare;
  PaxTypeFare* _ptf;
  PaxTypeFare* _excPtf;
  ExcItin* _ffExchangeItin;
  FareMarket* _fm;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareBytesValidatorTest);
}
