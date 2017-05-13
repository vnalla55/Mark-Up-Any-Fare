#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/RexExchangeTrx.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/FareByRuleApp.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "test/include/TestConfigInitializer.h"
#include <boost/assign/list_of.hpp>

namespace tse
{

namespace
{
class PaxTypeFareTestDataHandleMock : public DataHandleMock
{
public:
  PaxTypeFareTestDataHandleMock()
  {
    CorpId* id = _memHandle.create<CorpId>();
    id->accountCode() = "ABC";
    _corpIds.push_back(id);
  }

  ~PaxTypeFareTestDataHandleMock() { _memHandle.clear(); }

  const std::vector<tse::CorpId*>&
  getCorpId(const std::string& corpId, const CarrierCode& carrier, const DateTime& tvlDate)
  {
    return _corpIds;
  }

private:
  TestMemHandle _memHandle;
  std::vector<tse::CorpId*> _corpIds;
};

class PaxTypeFareMock : public PaxTypeFare
{
protected:
  bool applyNonIATAR(const PricingTrx& trx,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const RuleNumber& ruleNumber)
  {
    return vendor == "5KAD";
  }

public:
  const MoneyAmount fareAmount() const
  {
    return 350.0;
  }
};
}

class PaxTypeFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeFareTest);

  CPPUNIT_TEST(testIsNonrefundablePass);
  CPPUNIT_TEST(testIsNonrefundableFail);

  CPPUNIT_TEST(testGettersSetters);
  CPPUNIT_TEST(testSecondNucAmountAreReturnedWhenSecondDateFlagOn);

  CPPUNIT_TEST(testIsValidAsKeepFareNoRetrievalInfo);
  CPPUNIT_TEST(testIsValidAsKeepFareNotKeepFare);
  CPPUNIT_TEST(testIsValidAsKeepFarePassRoutingNotProcessed);
  CPPUNIT_TEST(testIsValidAsKeepFarePassRoutingValid);
  CPPUNIT_TEST(testIsValidAsKeepFareFail);

  CPPUNIT_TEST(testIsValidAccountCodeNotFareByRule);
  CPPUNIT_TEST(testIsValidAccountCodeFail);
  CPPUNIT_TEST(testIsValidAccountCodeFailEmptyCorporateId);
  CPPUNIT_TEST(testIsValidAccountCodePassEmptyAccountCode);
  CPPUNIT_TEST(testIsValidAccountCodePassSameAccountCodeInRequest);
  CPPUNIT_TEST(testIsValidAccountCodePassSameAccountCodeInGroupId);

  CPPUNIT_TEST(testApplyNonIATARoundingDefaultValue);
  CPPUNIT_TEST(testApplyNonIATARoundingPresetValueNo);
  CPPUNIT_TEST(testApplyNonIATARoundingPresetValueYes);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenDBReturnsYes);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenDBReturnsNo);

  CPPUNIT_TEST(testBrandingFunctions);

  CPPUNIT_TEST(testSetValidatingCxrInvalid);
  CPPUNIT_TEST(testSetValidatingCxrInvalidVec);
  CPPUNIT_TEST(testConsolidValidatingCxrList);
  CPPUNIT_TEST(testGetFailedCatNum);
  CPPUNIT_TEST(testFareFromFlownFareMarket);
  CPPUNIT_TEST(testFareFromNotShoppedFareMarket);
  CPPUNIT_TEST(testFareWithLowerPrice);
  CPPUNIT_TEST(testFareWithHigherPrice);
  CPPUNIT_TEST(testFareWithHigherPriceShoppedFM);
  CPPUNIT_TEST(testKeepOriginalBrandDifferentBrandCodesShopped);
  CPPUNIT_TEST(testKeepOriginalBrandDifferentBrandCodesNotShopped);
  CPPUNIT_TEST(testKeepOriginalBrandSameBrandCodesFailStatus);
  CPPUNIT_TEST(testKeepOriginalBrandSameBrandCodesPassStatusKeepBrandMatch);

  CPPUNIT_TEST(testIsValidInternalRequestedFareBasisValid);
  CPPUNIT_TEST(testIsValidInternalRequestedFareBasisInvalid);

  CPPUNIT_TEST(testgetDirectionFmOriginalFromNotReversed);
  CPPUNIT_TEST(testgetDirectionFmOriginalFromReversed);
  CPPUNIT_TEST(testgetDirectionFmOriginalToNotReversed);
  CPPUNIT_TEST(testgetDirectionFmOriginalToReversed);
  CPPUNIT_TEST(testgetDirectionFmReversedFromNotReversed);
  CPPUNIT_TEST(testgetDirectionFmReversedFromReversed);
  CPPUNIT_TEST(testgetDirectionFmReversedToNotReversed);
  CPPUNIT_TEST(testgetDirectionFmReversedToReversed);

  CPPUNIT_TEST(testSetFareStatusForInclCode_PB_False);
  CPPUNIT_TEST(testSetFareStatusForInclCode_PB_True);
  CPPUNIT_TEST(testIsFarePassForInclCode_all);
  CPPUNIT_TEST(testIsFarePassForInclCode_one_YB);
  CPPUNIT_TEST(testValidForSFRMultiPaxRequest_emptyFareBasisContainer);
  CPPUNIT_TEST(testValidForSFRMultiPaxRequest_findingFareInContainer);

  CPPUNIT_TEST_SUITE_END();

private:
  DataHandleMock* _dataHandleMock;
  TestMemHandle _memHandle;
  PaxTypeFare* _paxTypeFare;
  PricingTrx* _trx;
  PricingOptions _options;
  PricingRequest* _request;
  FareByRuleApp* _fareByRuleApp;
  FareDisplayInfo* _fareDisplayInfo;
  FareMarket::RetrievalInfo* _retrievalInfo;
  FareDisplayRequest* _requestFD;
  FareDisplayTrx* _trxFD;

  Fare* _fare;
  FareInfo* _fareInfo;
  FareMarket* _fm;
  static const BrandCode SV;
  static const BrandCode BZ;
  const Loc*  _locDFW;
  const Loc*  _locLON;
  Loc* _locBer;
  Loc* _locBuh;

  void prepareDirectionalityTest(bool isFareMarketDirectionBerBuh,
    bool isFareInfoDirectionFrom, bool isFareReversed)
  {
    _locBer = _memHandle.create<Loc>();
    _locBer->loc() = "BER";
    _locBuh = _memHandle.create<Loc>();
    _locBuh->loc() = "BUH";

    _fm = _memHandle.create<FareMarket>();
    if (isFareMarketDirectionBerBuh)
    {
      _fm->origin() = _locBer;
      _fm->destination() = _locBuh;
    }
    else
    {
      _fm->origin() = _locBuh;
      _fm->destination() = _locBer;
    }

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->market1() = _locBer->loc();
    _fareInfo->market2() = _locBuh->loc();
    if (isFareInfoDirectionFrom)
      _fareInfo->directionality() = FROM;
    else
      _fareInfo->directionality() = TO;

    _fare = _memHandle.create<Fare>();
    _fare->setFareInfo(_fareInfo);
    if (isFareReversed)
      _fare->status().set(Fare::FareState::FS_ReversedDirection, true);
    else
      _fare->status().set(Fare::FareState::FS_ReversedDirection, false);

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->setFare(_fare);
    _paxTypeFare->fareMarket() = _fm;
  }

  FareMarket* createFareMarket()
  {
    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    seg1->origin() = _locDFW;
    seg1->boardMultiCity() = _locDFW->loc();
    TravelSeg* seg2 = _memHandle.create<AirSeg>();
    seg2->destination() = _locLON;
    seg2->offMultiCity() = _locLON->loc();

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg().push_back(seg1);
    fm->travelSeg().push_back(seg2);
    fm->brandProgramIndexVec().push_back(0);
    fm->setOrigDestByTvlSegs();

    return fm;
  }

  PaxTypeFare* createPaxTypeFare(FareMarket* fm,
                                 PaxTypeFare::BrandStatus status = PaxTypeFare::BS_FAIL)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFareMock>();
    ptf->fareMarket() = fm;
    ptf->getMutableBrandStatusVec().push_back(std::make_pair(status, Direction::BOTHWAYS));

    return ptf;
  }

  RexExchangeTrx* createRexExchangeTrx(BrandCode brandCode)
  {
    RexExchangeTrx* rTrx = _memHandle.create<RexExchangeTrx>();

    BrandInfo* bi = _memHandle.create<BrandInfo>();
    bi->brandCode() = brandCode;

    rTrx->setKeepOriginal(true);
    rTrx->brandProgramVec().push_back(std::make_pair(static_cast<BrandProgram*>(0), bi));

    rTrx->setKeepOriginalStrategy(RexExchangeTrx::KEEP_BRAND);

    return rTrx;
  }

public:
  void setUp()
  {
    _dataHandleMock = _memHandle.create<PaxTypeFareTestDataHandleMock>();
    _fareByRuleApp = _memHandle.create<FareByRuleApp>();
    _fareDisplayInfo = _memHandle.create<FareDisplayInfo>();
    _fareByRuleApp->accountCode() = "DEF";
    FBRPaxTypeFareRuleData* ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    ruleData->fbrApp() = _fareByRuleApp;
    PaxTypeFare::PaxTypeFareAllRuleData* allRuleData =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    allRuleData->fareRuleData = ruleData;

    _retrievalInfo = _memHandle.create<FareMarket::RetrievalInfo>();
    _fare = _memHandle.create<Fare>();

    _paxTypeFare = _memHandle.create<PaxTypeFareMock>();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule);
    (*_paxTypeFare->paxTypeFareRuleDataMap())[25] = allRuleData;
    _paxTypeFare->retrievalInfo() = _retrievalInfo;
    _paxTypeFare->setFare(_fare);
    _paxTypeFare->fareDisplayInfo() = _fareDisplayInfo;

    _fareInfo = _memHandle.create<FareInfo>();
    _fare->setFareInfo(_fareInfo);

    _request = _memHandle.create<PricingRequest>();
    _request->accountCode() = "GHI";
    _request->corporateID() = "JKL";
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_request);
    _trx->setOptions(&_options);
    tse::Agent* agent = _memHandle.create<tse::Agent>();
    _request->ticketingAgent() = agent;

    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    _memHandle.create<TestConfigInitializer>();
    _requestFD = _memHandle.create<FareDisplayRequest>();
    _trxFD = _memHandle.create<FareDisplayTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testIsNonrefundablePass()
  {
    FareClassAppInfo fca;
    _paxTypeFare->fareClassAppInfo() = &fca;

    fca._fareType = "XAN";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
    fca._fareType = "XBN";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
    fca._fareType = "XPN";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
    fca._fareType = "XPV";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
    fca._fareType = "EIP";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
    fca._fareType = "SIP";
    CPPUNIT_ASSERT(_paxTypeFare->isNonRefundableByFareType());
  }

  void testIsNonrefundableFail()
  {
    FareClassAppInfo fca;
    _paxTypeFare->fareClassAppInfo() = &fca;

    fca._fareType = "BR";
    CPPUNIT_ASSERT(!_paxTypeFare->isNonRefundableByFareType());
  }

  void testGettersSetters()
  {
    _paxTypeFare->rexSecondMileageSurchargeAmt() = 100.00;
    CPPUNIT_ASSERT_EQUAL(100.00, _paxTypeFare->rexSecondMileageSurchargeAmt());
    CPPUNIT_ASSERT_EQUAL(100.00, _fare->rexSecondMileageSurchargeAmt());

    _paxTypeFare->rexSecondNucFareAmount() = 100.10;
    CPPUNIT_ASSERT_EQUAL(100.10, _paxTypeFare->rexSecondNucFareAmount());
    CPPUNIT_ASSERT_EQUAL(100.10, _fare->rexSecondNucFareAmount());
  }

  void testSecondNucAmountAreReturnedWhenSecondDateFlagOn()
  {
    FareMarket fm;
    RexPricingTrx rexTrx;
    fm.rexBaseTrx() = &rexTrx;
    _paxTypeFare->fareMarket() = &fm;

    _paxTypeFare->mileageSurchargeAmt() = 50.00;
    _paxTypeFare->nucFareAmount() = 50.10;
    _paxTypeFare->rexSecondMileageSurchargeAmt() = 100.00;
    _paxTypeFare->rexSecondNucFareAmount() = 100.10;

    rexTrx.useSecondROEConversionDate() = true;

    CPPUNIT_ASSERT_EQUAL(100.00, _paxTypeFare->mileageSurchargeAmt());
    CPPUNIT_ASSERT_EQUAL(100.10, _paxTypeFare->nucFareAmount());

    rexTrx.useSecondROEConversionDate() = false;

    CPPUNIT_ASSERT_EQUAL(50.00, _paxTypeFare->mileageSurchargeAmt());
    CPPUNIT_ASSERT_EQUAL(50.10, _paxTypeFare->nucFareAmount());
  }

  void testIsValidAsKeepFareNoRetrievalInfo()

  {
    _paxTypeFare->retrievalInfo() = 0;
    CPPUNIT_ASSERT(!_paxTypeFare->isValidAsKeepFare(*_trx));
  }

  void testIsValidAsKeepFareNotKeepFare()

  {
    _retrievalInfo->_flag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_paxTypeFare->isValidAsKeepFare(*_trx));
  }

  void testIsValidAsKeepFarePassRoutingNotProcessed()

  {
    _retrievalInfo->_flag = FareMarket::RetrievKeep;
    _fare->status().clear(Fare::FS_RoutingProcessed);

    CPPUNIT_ASSERT(_paxTypeFare->isValidAsKeepFare(*_trx));
  }

  void testIsValidAsKeepFarePassRoutingValid()

  {
    _retrievalInfo->_flag = FareMarket::RetrievKeep;
    _fare->status().set(Fare::FS_RoutingProcessed);
    _fare->status().set(Fare::FS_RoutingValid);

    CPPUNIT_ASSERT(_paxTypeFare->isValidAsKeepFare(*_trx));
  }

  void testIsValidAsKeepFareFail()

  {
    _retrievalInfo->_flag = FareMarket::RetrievKeep;
    _fare->status().set(Fare::FS_RoutingProcessed);
    _fare->status().clear(Fare::FS_RoutingValid);

    CPPUNIT_ASSERT(!_paxTypeFare->isValidAsKeepFare(*_trx));
  }

  void testIsValidAccountCodeNotFareByRule()
  {
    _paxTypeFare->status().clear(PaxTypeFare::PTF_FareByRule);
    CPPUNIT_ASSERT(_paxTypeFare->isValidAccountCode(*_trx));
  }

  void testIsValidAccountCodeFail() { CPPUNIT_ASSERT(!_paxTypeFare->isValidAccountCode(*_trx)); }

  void testIsValidAccountCodeFailEmptyCorporateId()
  {
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_paxTypeFare->isValidAccountCode(*_trx));
  }

  void testIsValidAccountCodePassEmptyAccountCode()
  {
    _fareByRuleApp->accountCode() = "";
    CPPUNIT_ASSERT(_paxTypeFare->isValidAccountCode(*_trx));
  }

  void testIsValidAccountCodePassSameAccountCodeInRequest()
  {
    _fareByRuleApp->accountCode() = "GHI";
    CPPUNIT_ASSERT(_paxTypeFare->isValidAccountCode(*_trx));
  }

  void testIsValidAccountCodePassSameAccountCodeInGroupId()
  {
    _fareByRuleApp->accountCode() = "ABC";
    CPPUNIT_ASSERT(_paxTypeFare->isValidAccountCode(*_trx));
  }

  void testApplyNonIATARoundingDefaultValue()
  {
    CPPUNIT_ASSERT(!_paxTypeFare->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingPresetValueNo()
  {
    _paxTypeFare->_applyNonIATARounding = NO;
    CPPUNIT_ASSERT(!_paxTypeFare->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingPresetValueYes()
  {
    _paxTypeFare->_applyNonIATARounding = YES;
    CPPUNIT_ASSERT(_paxTypeFare->applyNonIATARounding(*_trx));
  }

  void testApplyNonIATARoundingWhenDBReturnsYes()
  {
    _fareInfo->vendor() = "5KAD";
    CPPUNIT_ASSERT(_paxTypeFare->applyNonIATARounding(*_trx));
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->_applyNonIATARounding, YES);
  }

  void testApplyNonIATARoundingWhenDBReturnsNo()
  {
    _fareInfo->vendor() = "8CI1";
    CPPUNIT_ASSERT(!_paxTypeFare->applyNonIATARounding(*_trx));
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->_applyNonIATARounding, NO);
  }

  void testBrandingFunctions()
  {
    Loc origin, destination;
    origin.loc() = "SYD";
    destination.loc() = "AKL";

    FareMarket fm;
    fm.brandProgramIndexVec().push_back(0);
    fm.brandProgramIndexVec().push_back(1);
    fm.brandProgramIndexVec().push_back(3);
    fm.origin() = &origin;
    fm.destination() = &destination;

    FareInfo fareInfo;
    fareInfo.directionality() = BOTH;
    fareInfo.market1() = "SYD";
    fareInfo.market2() = "AKL";

    Fare fare;
    fare.initialize(Fare::FareState::FS_Domestic, &fareInfo, fm, nullptr, nullptr);

    _paxTypeFare->fareMarket() = &fm;
    _paxTypeFare->setFare(&fare);

    PricingTrx trx;
    BrandProgram bProgram1;
    BrandInfo brand1, brand2, brand3, brand4, brand5;

    brand1.brandCode() = "AS";
    brand2.brandCode() = "BS";
    brand3.brandCode() = "CS";
    brand4.brandCode() = "DS";
    // for ANY_BRAND or ANY_BRAND_LEG_PARITY scenarios a given fare is valid if has any brand assigned
    // status for those fake brands is computed as best status is any brand a fare is assigned to
    brand5.brandCode() = ANY_BRAND_LEG_PARITY;

    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand1));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand2));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand3));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand4));

    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    CPPUNIT_ASSERT(_paxTypeFare->getBestStatusInAnyBrand(true) == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand5.brandCode()) == PaxTypeFare::BS_FAIL);

    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));

    CPPUNIT_ASSERT(_paxTypeFare->getBestStatusInAnyBrand(true) == PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand5.brandCode()) == PaxTypeFare::BS_SOFT_PASS);

    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));

    CPPUNIT_ASSERT(_paxTypeFare->getBestStatusInAnyBrand(true) == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand5.brandCode()) == PaxTypeFare::BS_HARD_PASS);

    // Brand Filter map in regular MIP filters only by brandCode.
    BrandFilterMap filter;
    // brand codes not valid for a fare:
    filter["X1"];
    filter["X2"];

    CPPUNIT_ASSERT(_paxTypeFare->isValidForRequestedBrands(trx, filter, false) == false);

    // adding a soft passed brand to filter.
    filter["BS"];
    CPPUNIT_ASSERT(_paxTypeFare->isValidForRequestedBrands(trx, filter, false) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForRequestedBrands(trx, filter, true) == false);

    // adding a hard passed brand.
    filter["DS"];

    CPPUNIT_ASSERT(_paxTypeFare->isValidForRequestedBrands(trx, filter, false) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForRequestedBrands(trx, filter, true) == true);
    // End of brand filtering tests

    // When both soft and hard passed brands present first hard passed one should be returned
    BrandCode firstValidBrand = _paxTypeFare->getFirstValidBrand(trx, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(firstValidBrand == "DS");
    CPPUNIT_ASSERT(_paxTypeFare->hasValidBrands());

    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand1.brandCode()) ==
        PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand2.brandCode()) ==
        PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand4.brandCode()) ==
        PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(_paxTypeFare->getBrandStatus(trx, &brand3.brandCode()) ==
        PaxTypeFare::BS_FAIL);

    CPPUNIT_ASSERT(_paxTypeFare->getValidBrandIndex(trx, &brand1.brandCode(), Direction::BOTHWAYS) == INVALID_BRAND_INDEX);
    CPPUNIT_ASSERT(_paxTypeFare->getValidBrandIndex(trx, &brand2.brandCode(), Direction::BOTHWAYS) == 1);
    CPPUNIT_ASSERT(_paxTypeFare->getValidBrandIndex(trx, &brand4.brandCode(), Direction::BOTHWAYS) == 3);
    CPPUNIT_ASSERT(_paxTypeFare->getValidBrandIndex(trx, &brand3.brandCode(), Direction::BOTHWAYS) == INVALID_BRAND_INDEX);

    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand1.brandCode()) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand1.brandCode(), false) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand1.brandCode(), true) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand2.brandCode()) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand2.brandCode(), false) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand2.brandCode(), true) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand4.brandCode()) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand4.brandCode(), false) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand4.brandCode(), true) == true);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand3.brandCode()) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand3.brandCode(), false) == false);
    CPPUNIT_ASSERT(_paxTypeFare->isValidForBrand(trx, &brand3.brandCode(), true) == false);

    std::vector<int> brandIndices;
    _paxTypeFare->getValidBrands(trx, brandIndices);

    CPPUNIT_ASSERT(brandIndices.size() == 2);
    CPPUNIT_ASSERT(brandIndices[0] == 1);
    CPPUNIT_ASSERT(brandIndices[1] == 3);

    brandIndices.clear();
    _paxTypeFare->getValidBrands(trx, brandIndices, false);

    CPPUNIT_ASSERT(brandIndices.size() == 2);
    CPPUNIT_ASSERT(brandIndices[0] == 1);
    CPPUNIT_ASSERT(brandIndices[1] == 3);

    brandIndices.clear();
    _paxTypeFare->getValidBrands(trx, brandIndices, true);

    CPPUNIT_ASSERT(brandIndices.size() == 1);
    CPPUNIT_ASSERT(brandIndices[0] == 3);

    std::vector<BrandCode> brandCodes;
    _paxTypeFare->getValidBrands(trx, brandCodes);

    CPPUNIT_ASSERT(brandCodes.size() == 2);
    CPPUNIT_ASSERT(brandCodes[0] == "BS");
    CPPUNIT_ASSERT(brandCodes[1] == "DS");

    brandCodes.clear();
    _paxTypeFare->getValidBrands(trx, brandCodes, false);

    CPPUNIT_ASSERT(brandCodes.size() == 2);
    CPPUNIT_ASSERT(brandCodes[0] == "BS");
    CPPUNIT_ASSERT(brandCodes[1] == "DS");

    brandCodes.clear();
    _paxTypeFare->getValidBrands(trx, brandCodes, true);

    CPPUNIT_ASSERT(brandCodes.size() == 1);
    CPPUNIT_ASSERT(brandCodes[0] == "DS");

    // Checking that when only soft passed brands present , the first one will be returned
    _paxTypeFare->getMutableBrandStatusVec().clear();
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));

    firstValidBrand = _paxTypeFare->getFirstValidBrand(trx, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(firstValidBrand == "BS");
    CPPUNIT_ASSERT(_paxTypeFare->hasValidBrands());

    // If no valid brands then first valid brand should return empty brand
    _paxTypeFare->getMutableBrandStatusVec().clear();
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    _paxTypeFare->getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

    firstValidBrand = _paxTypeFare->getFirstValidBrand(trx, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(firstValidBrand == "");
    CPPUNIT_ASSERT(_paxTypeFare->hasValidBrands() == false);

  }

  void testSetValidatingCxrInvalid()
  {
  }

  void testSetValidatingCxrInvalidVec()
  {
  }

  void testConsolidValidatingCxrList()
  {
    PaxTypeFare ptf2;
    ptf2.setFare(_paxTypeFare->fare());

    _paxTypeFare->validatingCarriers().push_back(CarrierCode("AA"));
    _paxTypeFare->validatingCarriers().push_back(CarrierCode("BA"));
    _paxTypeFare->validatingCarriers().push_back(CarrierCode("UA"));
    ptf2.validatingCarriers().push_back(CarrierCode("AA"));
    ptf2.validatingCarriers().push_back(CarrierCode("UA"));

    _paxTypeFare->setValidatingCxrInvalid( CarrierCode("UA"), 3);
    ptf2.consolidValidatingCxrList();
    CPPUNIT_ASSERT(ptf2.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(ptf2.validatingCarriers().back() == "AA");

    _paxTypeFare->setValidatingCxrInvalid( CarrierCode("BA"), 4);
    ptf2.consolidValidatingCxrList();
    CPPUNIT_ASSERT(ptf2.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(ptf2.validatingCarriers().back() == "AA");

    _paxTypeFare->setValidatingCxrInvalid( CarrierCode("AA"), 5);
    ptf2.consolidValidatingCxrList();
    CPPUNIT_ASSERT(ptf2.validatingCarriers().size() == 0);
    CPPUNIT_ASSERT(ptf2.isValid() == false);
  }

  void testGetFailedCatNum()
  {
    std::vector<uint16_t> cats = boost::assign::list_of(1)(2)(3)(11)(14)(15);

    CPPUNIT_ASSERT(0 == _paxTypeFare->getFailedCatNum(cats) );

    _paxTypeFare->setCategoryValid(1, false);
    CPPUNIT_ASSERT(1 == _paxTypeFare->getFailedCatNum(cats) );

    _paxTypeFare->setCategoryValid(1, true);
    _paxTypeFare->setCategoryValid(15, false);
    CPPUNIT_ASSERT(15 == _paxTypeFare->getFailedCatNum(cats) );

    _paxTypeFare->setCategoryValid(15, true);
    _paxTypeFare->setCategoryValid(11, false);
    CPPUNIT_ASSERT(11 == _paxTypeFare->getFailedCatNum(cats) );
  }

  void testFareFromFlownFareMarket()
  {
    FareMarket* fm = createFareMarket();
    fm->changeStatus() = FL;

    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm);

    CPPUNIT_ASSERT(paxTypeFare->isFromFlownOrNotShoppedFM());
  }

  void testFareFromNotShoppedFareMarket()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm);

    CPPUNIT_ASSERT(paxTypeFare->isFromFlownOrNotShoppedFM());
  }

  void testFareWithLowerPrice()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm);

    RexExchangeTrx rexTrx;
    rexTrx.addNewFMtoMaxPrice(fm, 300);

    //max available price is 300, fare market is not shopped, fare amount is 350, FAIL

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(rexTrx, "  ") == PaxTypeFare::BS_FAIL);
  }

  void testFareWithHigherPrice()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm);

    RexExchangeTrx rexTrx;
    rexTrx.addNewFMtoMaxPrice(fm, 400);

    //max available price is 400, fare market is not shopped, fare amount is 350, PASS

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(rexTrx, "  ") == PaxTypeFare::BS_HARD_PASS);
  }

  void testFareWithHigherPriceShoppedFM()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm);

    RexExchangeTrx rexTrx;
    rexTrx.addNewFMtoMaxPrice(fm, 400);

    //max available price is 400, fare market is not shopped, fare amount is 350, PASS

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(rexTrx, "  ") == PaxTypeFare::BS_HARD_PASS);
  }

  void testKeepOriginalBrandDifferentBrandCodesShopped()
  {
    FareMarket* fm = createFareMarket();
    fm->travelSeg().front()->isShopped() = true;
    fm->travelSeg().back()->isShopped() = true;

    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm, PaxTypeFare::BS_HARD_PASS);
    RexExchangeTrx* rexTrx = createRexExchangeTrx(SV);
    rexTrx->addNewFMtoBrandCodeSet(fm, SV);

    BrandCode checkBrandCode = BZ;

    //Fare is valid for SV brand, SV brand was used for original ticket, shopped FM, for BZ fare is FAIL

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(*rexTrx, checkBrandCode) == PaxTypeFare::BS_FAIL);

  }

  void testKeepOriginalBrandDifferentBrandCodesNotShopped()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm, PaxTypeFare::BS_HARD_PASS);
    BrandCode checkBrandCode = BZ;
    RexExchangeTrx* rexTrx = createRexExchangeTrx(SV);
    rexTrx->addNewFMtoBrandCodeSet(fm, SV);

    //Fare is valid for SV brand, SV brand was used for original ticket, not shopped FM, PASS

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(*rexTrx, checkBrandCode) == PaxTypeFare::BS_HARD_PASS);

  }

  void testKeepOriginalBrandSameBrandCodesFailStatus()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm, PaxTypeFare::BS_FAIL);
    BrandCode checkBrandCode = BZ;
    RexExchangeTrx* rexTrx = createRexExchangeTrx(SV);
    rexTrx->addNewFMtoBrandCodeSet(fm, SV);

    //Fare isn't valid for SV brand, SV brand was used for original ticket, not shopped FM, FAIL

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(*rexTrx, checkBrandCode) == PaxTypeFare::BS_FAIL);

  }

  void testKeepOriginalBrandSameBrandCodesPassStatusKeepBrandMatch()
  {
    FareMarket* fm = createFareMarket();
    PaxTypeFare* paxTypeFare = createPaxTypeFare(fm, PaxTypeFare::BS_HARD_PASS);
    BrandCode checkBrandCode = SV;

    RexExchangeTrx* rexTrx = createRexExchangeTrx(SV);
    rexTrx->addNewFMtoBrandCodeSet(fm, BZ);

    //Fare is valid for SV brand, BZ brand was used for original ticket, not shopped FM, FAIL

    CPPUNIT_ASSERT(paxTypeFare->isValidForExchangeWithBrands(*rexTrx, checkBrandCode) == PaxTypeFare::BS_FAIL);
  }

  void testIsValidInternalRequestedFareBasisValid()
  {
    FareMarket*  fm = createFareMarket();
    PaxTypeFare* ptf = createPaxTypeFare(fm);

    ptf->setAllCategoryValidForShopping();
    ptf->_isMipUniqueFare = true;
    ptf->resetRuleStatus();
    ptf->setRoutingProcessed(false);
    ptf->setRoutingValid();
    ptf->bookingCodeStatus()= PaxTypeFare::BKS_PASS;

    CPPUNIT_ASSERT(ptf->isValidInternal() == true);
  }

  void testIsValidInternalRequestedFareBasisInvalid()
  {
    FareMarket*  fm = createFareMarket();
    PaxTypeFare* ptf = createPaxTypeFare(fm);

    ptf->setAllCategoryValidForShopping();
    ptf->_isMipUniqueFare = true;
    ptf->resetRuleStatus();
    ptf->setRoutingProcessed(false);
    ptf->setRoutingValid();
    ptf->bookingCodeStatus()= PaxTypeFare::BKS_PASS;
    ptf->setRequestedFareBasisInvalid();

    CPPUNIT_ASSERT(ptf->isValidInternal() == false);
  }

  void testgetDirectionFmOriginalFromNotReversed()
  {
    // FM:       BER -> BUH
    // FAREINFO: BER/BUH FROM (->)
    // FARE:     NOT REVERSED
    // PAX:      BER -> BUH

    // PAX FM vs FAREINFO: ORIGINAL

    prepareDirectionalityTest(true, true, false);
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmOriginalFromReversed()
  {
    // FM:       BER -> BUH
    // FAREINFO: BER/BUH FROM (->)
    // FARE:     REVERSED
    // PAX:      BER <- BUH ===> BUH -> BER

    // PAX FM vs FAREINFO: ORIGINAL

    prepareDirectionalityTest(true, true, true);
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmOriginalToNotReversed()
  {
    // FM:       BER -> BUH
    // FAREINFO: BER/BUH TO (<-)
    // FARE:     NOT REVERSED
    // PAX:      BUH <- BER ===> BER -> BUH

    // PAX FM vs FAREINFO: REVERSED

    prepareDirectionalityTest(true, false, false);
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmOriginalToReversed()
  {
    // FM:       BER -> BUH
    // FAREINFO: BER/BUH TO (<-)
    // FARE:     REVERSED
    // PAX:      BUH -> BER

    // PAX FM vs FAREINFO: ORIGINAL

    prepareDirectionalityTest(true, false, true);
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmReversedFromNotReversed()
  {
    // FM:       BUH -> BER
    // FAREINFO: BER/BUH FROM (->)
    // FARE:     NOT REVERSED
    // PAX:      BER -> BUH

    // PAX FM vs FAREINFO: REVERSED

    prepareDirectionalityTest(false, true, false);
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmReversedFromReversed()
  {
    // FM:       BUH -> BER
    // FAREINFO: BER/BUH FROM (->)
    // FARE:     REVERSED
    // PAX:      BER <- BUH ===> BUH -> BER

    // PAX FM vs FAREINFO: REVERSED

    prepareDirectionalityTest(false, true, true);
    CPPUNIT_ASSERT_EQUAL(Direction::REVERSED, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmReversedToNotReversed()
  {
    // FM:       BUH -> BER
    // FAREINFO: BER/BUH TO (<-)
    // FARE:     NOT REVERSED
    // PAX:      BUH <- BER ===> BER -> BUH

    // PAX FM vs FAREINFO: ORIGINAL

    prepareDirectionalityTest(false, false, false);
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, _paxTypeFare->getDirection());
  }

  void testgetDirectionFmReversedToReversed()
  {
    // FM:       BUH -> BER
    // FAREINFO: BER/BUH TO (<-)
    // FARE:     REVERSED
    // PAX:      BUH -> BER

    // PAX FM vs FAREINFO: ORIGINAL

    prepareDirectionalityTest(false, false, true);
    CPPUNIT_ASSERT_EQUAL(Direction::ORIGINAL, _paxTypeFare->getDirection());
  }

  void testSetFareStatusForInclCode_PB_False()
  {
    _trx->setRequest(_requestFD);
    _fareDisplayInfo->setFareDisplayTrx(_trxFD);
    // PB equal 1 - premium first cabin
    // no status holds in the vector
    CPPUNIT_ASSERT(_paxTypeFare->fareStatusPerInclCode().empty());
    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() == 0);

    uint8_t inclusionNumber = 1; // PB
    _paxTypeFare->setFareStatusForInclCode( inclusionNumber , false);
    // no update for failed inclusion code
    CPPUNIT_ASSERT(!_paxTypeFare->fareStatusPerInclCode().empty());
    CPPUNIT_ASSERT(!_paxTypeFare->isFarePassForInclCode(inclusionNumber));

    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() == 0);
  }

  void testSetFareStatusForInclCode_PB_True()
  {
    _trx->setRequest(_requestFD);
    _fareDisplayInfo->setFareDisplayTrx(_trxFD);
    // PB equal 1 - premium first cabin
    // no status holds in the vector
    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() == 0);

    uint8_t inclusionNumber = 1; // PB
    _paxTypeFare->setFareStatusForInclCode( inclusionNumber , true);
    // update for passed inclusion code
    CPPUNIT_ASSERT(_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() != 0);
  }

  void testIsFarePassForInclCode_one_YB()
  {
    _trx->setRequest(_requestFD);
    _fareDisplayInfo->setFareDisplayTrx(_trxFD);
    // PB equal 1 - premium first cabin
    // no status holds in the vector
    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() == 0);

    uint8_t inclusionNumber = 6; // YB
    _paxTypeFare->setFareStatusForInclCode( inclusionNumber , true);

    CPPUNIT_ASSERT(_paxTypeFare->isFarePassForInclCode(inclusionNumber));
  }

  void testValidForSFRMultiPaxRequest_emptyFareBasisContainer()
  {
    FareMarket fm;
    fm.createMultiPaxUniqueFareBasisCodes();
    fm.fbcUsage() = COMMAND_PRICE_FBC;
    CPPUNIT_ASSERT(!_paxTypeFare->validForCmdPricing(fm, false, _trx));
  }

  void testValidForSFRMultiPaxRequest_findingFareInContainer()
  {
    FareMarket fm;
    fm.createMultiPaxUniqueFareBasisCodes();
    fm.fbcUsage() = COMMAND_PRICE_FBC;
    fm.getMultiPaxUniqueFareBasisCodes().insert("dummyFare1");
    fm.getMultiPaxUniqueFareBasisCodes().insert("dummyFare2");
    fm.getMultiPaxUniqueFareBasisCodes().insert("dummyFare3");
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, false);
    _fareInfo->fareClass() = "dummyFare2";
    _fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(_fare);

    CPPUNIT_ASSERT(_paxTypeFare->validForCmdPricing(fm, false, _trx));
  }

  void testIsFarePassForInclCode_all()
  {
    _trx->setRequest(_requestFD);
    _fareDisplayInfo->setFareDisplayTrx(_trxFD);
    // PB equal 1 - premium first cabin
    // no status holds in the vector
    CPPUNIT_ASSERT(_paxTypeFare->fareDisplayInfo()->inclusionCabinNum() == 0);

    uint8_t inclusionNumber = 1; // PB
    bool status = true;
    size_t remainder = 0;
    for(; inclusionNumber < 7; ++inclusionNumber)
    {
     remainder = inclusionNumber % 2;
     if(remainder)
       status = true;
     else
       status = false;
      _paxTypeFare->setFareStatusForInclCode( inclusionNumber , status);
    }
    //
    inclusionNumber = 1;
    CPPUNIT_ASSERT(_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    inclusionNumber = 2;
    CPPUNIT_ASSERT(!_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    inclusionNumber = 3;
    CPPUNIT_ASSERT(_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    inclusionNumber = 4;
    CPPUNIT_ASSERT(!_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    inclusionNumber = 5;
    CPPUNIT_ASSERT(_paxTypeFare->isFarePassForInclCode(inclusionNumber));
    inclusionNumber = 6;
    CPPUNIT_ASSERT(!_paxTypeFare->isFarePassForInclCode(inclusionNumber));
  }
};

const BrandCode PaxTypeFareTest::SV = "SV";
const BrandCode PaxTypeFareTest::BZ = "BZ";

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeFareTest);
}
