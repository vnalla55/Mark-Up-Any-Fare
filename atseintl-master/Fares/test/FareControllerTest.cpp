#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VCTR.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/QualifyFltAppRuleData.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareDisplayPref.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/TravelRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag201Collector.h"
#include "Fares/FareController.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFareClassAppInfoFactory.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

FALLBACKVALUE_DECL(fallbackAPO37838Record1EffDateCheck);
FALLBACKVALUE_DECL(fallbackSimpleTripCorrectionOWFare);
FALLBACKVALUE_DECL(dffOaFareCreation);

using boost::assign::operator+=;

class FareControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareControllerTest);

  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testGetVendorWhenFCIsEmpty);
  CPPUNIT_TEST(testGetVendorWhenItIsNotCat31Trx);
  CPPUNIT_TEST(testGetVendorWhenItIsNotCat31RepriceExcItinPhase);
  CPPUNIT_TEST(testGetVendorWhenVCTRIsMissing);
  CPPUNIT_TEST(testGetVendorWhenVCTRIsPresent);
  CPPUNIT_TEST(testGetVendorWhenVCTRIsDifferent);
  CPPUNIT_TEST(testFilterByVendorWhenThereIsNoIndustryAppls);
  CPPUNIT_TEST(testFilterByVendorWhenVendorExist);
  CPPUNIT_TEST(testFilterByVendorWhenVendorDoesntExist);
  CPPUNIT_TEST(testGetVendorReturnEmptyWhenVendorIsSmf);

  CPPUNIT_TEST(testGlobalDirectionStorage);

  CPPUNIT_TEST(testResolveTariffCrossRef_Pass);
  CPPUNIT_TEST(testResolveTariffCrossRef_Fail_NULL);
  CPPUNIT_TEST(testResolveTariffCrossRef_Fail_TariffInhibits);
  CPPUNIT_TEST(testResolveTariffCrossRef_Fail_TariffInhibitsWithDiag201);
  CPPUNIT_TEST(testResolveTariffCrossRef_Fail_InternationalGlobalDir);

  CPPUNIT_TEST(testResolveFareClassAppESV_Fail);
  CPPUNIT_TEST(testResolveFareClassAppESV_Pass);

  CPPUNIT_TEST(testValidateEligibilityQualifier_EligibilityInfoNull);
  CPPUNIT_TEST(testValidateEligibilityQualifier_EligibilityInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateEligibilityQualifier_CheckAccountCodeForDiscountRule);
  CPPUNIT_TEST(testValidateEligibilityQualifier_CheckAccountCodeForNegotiatedRule);
  CPPUNIT_TEST(testValidateEligibilityQualifier_CheckAccountCodeForSellingFare);
  CPPUNIT_TEST(testValidateEligibilityQualifier_Validation);
  CPPUNIT_TEST(testValidateEligibilityQualifier_Validation_FdTrx);

  CPPUNIT_TEST(testValidateDayTimeQualifier_DayTimeInfoNull);
  CPPUNIT_TEST(testValidateDayTimeQualifier_DayTimeInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateDayTimeQualifier_Validation);
  CPPUNIT_TEST(testValidateDayTimeQualifier_Validation_FdTrx);

  CPPUNIT_TEST(testValidateSeasonalQualifier_SeasonalInfoNull);
  CPPUNIT_TEST(testValidateSeasonalQualifier_SeasonalInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateSeasonalQualifier_Validation);
  CPPUNIT_TEST(testValidateSeasonalQualifier_Validation_FdTrx);

  CPPUNIT_TEST(testSetQualifyFltAppRuleData);

  CPPUNIT_TEST(testValidateFlightApplicationQualifier_FlighInfoNull);
  CPPUNIT_TEST(testValidateFlightApplicationQualifier_FlighInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateFlightApplicationQualifier_FdTrx);
  CPPUNIT_TEST(testValidateFlightApplicationQualifier_FCOPhase);
  CPPUNIT_TEST(testValidateFlightApplicationQualifier_Validation);

  CPPUNIT_TEST(testValidateTravelRestrictionsQualifier_TravelInfoNull);
  CPPUNIT_TEST(testValidateTravelRestrictionsQualifier_TravelInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateTravelRestrictionsQualifier_Validation);
  CPPUNIT_TEST(testValidateTravelRestrictionsQualifier_Validation_FdTrx);

  CPPUNIT_TEST(testValidateSaleRestrictionsQualifier_SalesInfoNull);
  CPPUNIT_TEST(testValidateSaleRestrictionsQualifier_SalesInfoNull_FdTrx);
  CPPUNIT_TEST(testValidateSaleRestrictionsQualifier_Validation);
  CPPUNIT_TEST(testValidateSaleRestrictionsQualifier_Validation_FareByRule);
  CPPUNIT_TEST(testValidateSaleRestrictionsQualifier_Validation_FdTrx);

  CPPUNIT_TEST(testCreatePaxTypeFares_Fail_Location);
  CPPUNIT_TEST(testCreatePaxTypeFares_Pass);

  CPPUNIT_TEST(testEliminateDuplicates_NoDuplicatedFares);
  CPPUNIT_TEST(testEliminateDuplicates_DuplicatedFares);
  CPPUNIT_TEST(testEliminateDuplicates_resizeResult);
  CPPUNIT_TEST(testEliminateDuplicates_NullFare);
  CPPUNIT_TEST(testEliminateDuplicates_Cat23Published);
  CPPUNIT_TEST(testEliminateDuplicates_SpecifiedOverConstructedYes);
  CPPUNIT_TEST(testEliminateDuplicates_SpecifiedOverConstructedNo);
  CPPUNIT_TEST(testEliminateDuplicates_AmountDiff);

  CPPUNIT_TEST(testCreatePaxTypeFareESV_Fail_Resolve);
  CPPUNIT_TEST(testCreatePaxTypeFareESV_Pass);

  CPPUNIT_TEST(testCheckAge_requestAgeZero);
  CPPUNIT_TEST(testCheckAge_requestAgeLessMinAge);
  CPPUNIT_TEST(testCheckAge_requestAgeMoreMaxAge);
  CPPUNIT_TEST(testCheckAge_requestAgeBetween);

  CPPUNIT_TEST(testValidateFareGroupPCC_SalesRestrictionPass);
  CPPUNIT_TEST(testValidateFareGroupPCC_SalesRestrictionFail);
  CPPUNIT_TEST(testValidateFareGroupPCC_NoSalesRestrictionNoPosPaxType);
  CPPUNIT_TEST(testValidateFareGroupPCC_NoSalesRestrictionPosPaxTypePositive);
  CPPUNIT_TEST(testValidateFareGroupPCC_NoSalesRestrictionNoPCC);
  CPPUNIT_TEST(testValidateFareGroupPCC_NoSalesRestrictionFail);

  CPPUNIT_TEST(testAddMatchedFareToPaxTypeBucket_Fail_EmptyCortage);
  CPPUNIT_TEST(testAddMatchedFareToPaxTypeBucket_Pass);

  CPPUNIT_TEST(testFindFaresESV_Fail_CollectBoundFares);
  CPPUNIT_TEST(testFindFaresESV_Fail_NoDataFromDatabase);
  CPPUNIT_TEST(testFindFaresESV_Pass);
  CPPUNIT_TEST(testCreateFareTwoOnewaySimpleTrip);

  CPPUNIT_TEST_SUITE_END();

private:
  RexPricingTrx* _trx;
  FareDisplayTrx* _fdTrx;
  Itin* _itin;
  FareMarket* _fareMarket;
  FareCompInfo* _fareCompInfo;
  FareController* _fareController;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = create<RexPricingTrx>();
    _trx->setRequest(create<PricingRequest>());
    _trx->setOptions(create<PricingOptions>());
    _trx->setCreateFareClassAppInfoBF(true);
    _trx->setCollectBoundFares(true);
    _itin = create<Itin>();
    _trx->itin().push_back(_itin);
    PaxType* pt = create<PaxType>();
    pt->paxType() = ADULT;
    _trx->paxType().push_back(pt);
    pt->actualPaxType()[""] = &_trx->paxType();

    _fdTrx = create<FareDisplayTrx>();
    _fdTrx->setRequest(create<FareDisplayRequest>());
    _fdTrx->setOptions(create<FareDisplayOptions>());
    _fdTrx->getRequest()->ticketingAgent() = create<Agent>();
    _fdTrx->getOptions()->fareDisplayPref() = create<FareDisplayPref>();

    _fdTrx->itin().push_back(_itin);
    _fdTrx->paxType().push_back(pt);

    _fareMarket = create<FareMarket>();
    _fareMarket->origin() = create<Loc>();
    _fareMarket->destination() = create<Loc>();
    _fareMarket->governingCarrierPref() = createCarrierPreference();
    _fareMarket->travelSeg().push_back(create<AirSeg>());
    DateTime testDate(2010, tse::Aug, 10, 22, 15, 34);
    _fareMarket->travelSeg()[0]->departureDT()= testDate;

    _fareCompInfo = create<FareCompInfo>();
    _fareController = _memHandle.insert(new FareController(*_trx, *_itin, *_fareMarket));
    _fareController->_prevalidationRuleController.categorySequence().clear();
    _fareMarket->fareCompInfo() = _fareCompInfo;
    _memHandle.create<MyDataHandle>();
    
    fallback::value::fallbackAPO37838Record1EffDateCheck.set(true);
    fallback::value::fallbackSimpleTripCorrectionOWFare.set(true);
    fallback::value::dffOaFareCreation.set(false);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  CarrierPreference* createCarrierPreference(Indicator applyspecoveraddon = ' ')
  {
    CarrierPreference* cp = create<CarrierPreference>();
    cp->applyspecoveraddon() = applyspecoveraddon;
    return cp;
  }

  void setVCTRForExchangePhase(const std::string& vctr)
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    _fareCompInfo->hasVCTR() = true;
    _fareCompInfo->VCTR() = VCTR(vctr, "JL", 513, "C012", 700000);
  }

  void createIndustryFareAppl(std::vector<const IndustryFareAppl*>& vec, const VendorCode& vendor)
  {
    IndustryFareAppl* appl = _memHandle.create<IndustryFareAppl>();
    vec.push_back(appl);

    appl->seqNo() = vec.size();
    appl->vendor() = vendor;
  }

  //-----------------------------------------------------------------------------
  // TESTS
  //-----------------------------------------------------------------------------
  void testConstructor()
  {
    CPPUNIT_ASSERT_NO_THROW(FareController fc(*_trx, *_itin, *_fareMarket));
  }

  void testGetVendorWhenFCIsEmpty()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    CPPUNIT_ASSERT_EQUAL(Vendor::EMPTY, _fareController->getVendorForCat31());
  }

  void testGetVendorWhenItIsNotCat31Trx()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    CPPUNIT_ASSERT_EQUAL(Vendor::EMPTY, _fareController->getVendorForCat31());
  }

  void testGetVendorWhenItIsNotCat31RepriceExcItinPhase()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    _trx->trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    CPPUNIT_ASSERT_EQUAL(Vendor::EMPTY, _fareController->getVendorForCat31());
  }

  void testGetVendorWhenVCTRIsMissing()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    _trx->trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    CPPUNIT_ASSERT_EQUAL(Vendor::EMPTY, _fareController->getVendorForCat31());
  }

  void testGetVendorWhenVCTRIsPresent()
  {
    setVCTRForExchangePhase(Vendor::SITA);

    CPPUNIT_ASSERT_EQUAL(Vendor::SITA, _fareController->getVendorForCat31());
  }

  void testGetVendorWhenVCTRIsDifferent()
  {
    setVCTRForExchangePhase(Vendor::ATPCO);

    CPPUNIT_ASSERT(!(_fareController->getVendorForCat31() == Vendor::SITA));
  }

  void testGetVendorReturnEmptyWhenVendorIsSmf()
  {
    setVCTRForExchangePhase("SMF");

    CPPUNIT_ASSERT_EQUAL(Vendor::EMPTY, _fareController->getVendorForCat31());
  }

  void testFilterByVendorWhenThereIsNoIndustryAppls()
  {
    setVCTRForExchangePhase(Vendor::SITA);

    std::vector<const IndustryFareAppl*> inputIndustryAppls;
    std::vector<const IndustryFareAppl*> tmpIndustryAppls;

    _fareController->filterByVendorForCat31(inputIndustryAppls, tmpIndustryAppls);

    CPPUNIT_ASSERT(tmpIndustryAppls.empty());
  }

  void testFilterByVendorWhenVendorExist()
  {
    setVCTRForExchangePhase(Vendor::SITA);

    std::vector<const IndustryFareAppl*> inputIndustryAppls;
    std::vector<const IndustryFareAppl*> tmpIndustryAppls;

    createIndustryFareAppl(inputIndustryAppls, Vendor::ATPCO);
    createIndustryFareAppl(inputIndustryAppls, Vendor::SITA);
    createIndustryFareAppl(inputIndustryAppls, Vendor::SITA);
    createIndustryFareAppl(inputIndustryAppls, Vendor::SABRE);

    _fareController->filterByVendorForCat31(inputIndustryAppls, tmpIndustryAppls);

    CPPUNIT_ASSERT_EQUAL(2, (int)tmpIndustryAppls.size());
  }

  void testFilterByVendorWhenVendorDoesntExist()
  {
    setVCTRForExchangePhase(Vendor::SITA);

    std::vector<const IndustryFareAppl*> inputIndustryAppls;
    std::vector<const IndustryFareAppl*> tmpIndustryAppls;
    createIndustryFareAppl(inputIndustryAppls, Vendor::ATPCO);
    createIndustryFareAppl(inputIndustryAppls, Vendor::SABRE);

    _fareController->filterByVendorForCat31(inputIndustryAppls, tmpIndustryAppls);

    CPPUNIT_ASSERT(tmpIndustryAppls.empty());
  }

  void testFilterOutSMFOfares()
  {
    std::vector<const FareInfo*> allFares;
    std::vector<const FareInfo*> filteredFares;
    allFares.push_back(createFareInfo(Vendor::SITA));
    allFares.push_back(createFareInfo(Vendor::SMFO));
    allFares.push_back(createFareInfo(Vendor::SABRE));
    allFares.push_back(createFareInfo(Vendor::SMFO));

    filteredFares = _fareController->filterByVendorForCat31AndDFF(allFares, filteredFares);

    CPPUNIT_ASSERT(filteredFares.size() == 2);
    CPPUNIT_ASSERT(filteredFares.front()->vendor() == Vendor::SITA);
    CPPUNIT_ASSERT(filteredFares.back()->vendor() == Vendor::SABRE);
  }

  void testfilterBothByVendorForCat31AndDFF()
  {
    setVCTRForExchangePhase(Vendor::SITA);

    std::vector<const FareInfo*> allFares;
    std::vector<const FareInfo*> filteredFares;
    allFares.push_back(createFareInfo(Vendor::SITA));
    allFares.push_back(createFareInfo(Vendor::SMFO));
    allFares.push_back(createFareInfo(Vendor::SABRE));
    allFares.push_back(createFareInfo(Vendor::SMFO));

    filteredFares = _fareController->filterByVendorForCat31AndDFF(allFares, filteredFares);

    CPPUNIT_ASSERT(filteredFares.size() == 1);
    CPPUNIT_ASSERT(filteredFares.front()->vendor() == Vendor::SITA);
  }

  void testGlobalDirectionStorage()
  {
    FareMarket fm;
    FareInfo fi;
    Fare fare;
    PaxTypeFare ptf;

    fi.globalDirection() = GlobalDirection::CA;
    fare.setFareInfo(&fi);
    ptf.setFare(&fare);
    ptf.fareMarket() = &fm;
    fm.setGlobalDirection(GlobalDirection::DO);

    CPPUNIT_ASSERT_EQUAL(GlobalDirection::DO, fm.getGlobalDirection());
    {
      FareController::GlobalDirectionStorage dir(ptf);
      CPPUNIT_ASSERT_EQUAL(GlobalDirection::CA, fm.getGlobalDirection());
    }
    CPPUNIT_ASSERT_EQUAL(GlobalDirection::DO, fm.getGlobalDirection());
  }

  void testResolveTariffCrossRef_Pass()
  {
    CPPUNIT_ASSERT(_fareController->resolveTariffCrossRef(*createFare()));
  }

  void testResolveTariffCrossRef_Fail_NULL()
  {
    Fare& fare = *createFare("NO");
    fare.setTariffCrossRefInfo(0);
    CPPUNIT_ASSERT(!_fareController->resolveTariffCrossRef(fare));
  }

  void testResolveTariffCrossRef_Fail_TariffInhibits()
  {
    VendorCode vendor(FareController::INHIBIT_ALL);
    CPPUNIT_ASSERT(!_fareController->resolveTariffCrossRef(*createFare(vendor)));
  }

  void testResolveTariffCrossRef_Fail_TariffInhibitsWithDiag201()
  {
    VendorCode vendor(FareController::INHIBIT_ALL);
    populateDiagnostic(Diagnostic201);
    _fareController->resolveTariffCrossRef(*createFare(vendor));
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(" D     0 ") != std::string::npos);
  }

  void testResolveTariffCrossRef_Fail_InternationalGlobalDir()
  {
    Fare* fare = createFare();
    fare->status().set(Fare::FS_International);
    populateDiagnostic(Diagnostic204);
    CPPUNIT_ASSERT(!_fareController->resolveTariffCrossRef(*fare));
  }

  void testResolveFareClassAppESV_Fail()
  {
    PaxTypeFare* ptf = createPaxTypeFare();
    _trx->setCreateFareClassAppInfoBF(false);
    CPPUNIT_ASSERT(!_fareController->resolveFareClassAppESV(*ptf->fare(), ptf));
  }

  void testResolveFareClassAppESV_Pass()
  {
    PaxTypeFare* ptf = createPaxTypeFare();
    CPPUNIT_ASSERT(_fareController->resolveFareClassAppESV(*ptf->fare(), ptf));
  }

  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = create<PaxType>();
    pt->paxType() = code;
    pt->paxTypeInfo() = create<PaxTypeInfo>();
    return pt;
  }

  PaxTypeFare*
  createPaxTypeFare(bool sellingFare = false, const VendorCode& vendor = VendorCode(""))
  {
    PaxTypeFare* ptf = create<PaxTypeFare>();
    ptf->fareDisplayInfo() = create<FareDisplayInfo>();

    FareClassAppInfo* fca = create<FareClassAppInfo>();
    if (sellingFare)
      fca->_displayCatType = RuleConst::SELLING_FARE;

    FareClassAppSegInfo* fcasi = create<FareClassAppSegInfo>();
    fcasi->_minAge = 10;
    fcasi->_maxAge = 15;

    ptf->setFare(createFare(vendor));
    ptf->fareClassAppInfo() = fca;
    ptf->fareClassAppSegInfo() = fcasi;
    ptf->paxType() = createPaxType(ADULT);
    ptf->fareMarket() = create<FareMarket>();
    return ptf;
  }

  class MockConstructedFareInfo : public ConstructedFareInfo
  {
  public:
    MockConstructedFareInfo() : ConstructedFareInfo(true) {}
  };

  FareInfo* createFareInfo(const VendorCode& vendor = VendorCode(""))
  {
    FareInfo* fInfo = create<FareInfo>();
    fInfo->vendor() = vendor;
    return fInfo;
  }

  Fare* createFare(const VendorCode& vendor = VendorCode(""))
  {
    Fare* f = create<Fare>();
    FareInfo* fInfo = create<FareInfo>();
    fInfo->vendor() = vendor;
    fInfo->market1() = "LON";
    fInfo->market2() = "FRA";

    if (!vendor.empty())
    {
      fInfo->_pAdditionalInfoContainer = new AdditionalInfoContainer(); // released in ~FareInfo
    }
    f->setFareInfo(fInfo);
    f->setTariffCrossRefInfo(create<TariffCrossRefInfo>());

    return f;
  }

  Fare* createConstructedFare(const MoneyAmount& amt = 0)
  {
    ConstructedFareInfo* cfi = create<MockConstructedFareInfo>();
    cfi->constructedNucAmount() = amt;
    cfi->fareInfo().market1() = "LON";
    cfi->fareInfo().market2() = "FRA";

    Fare* fare = createFare();
    fare->constructedFareInfo() = cfi;

    return fare;
  }

  CategoryRuleInfo* createCategoryRuleInfo(uint16_t cat = 0)
  {
    CategoryRuleInfo* ri = create<CategoryRuleInfo>();
    ri->categoryNumber() = cat;
    return ri;
  }

  FareDisplayTrx* populateFdTrx() { return _fareController->_fdTrx = _fdTrx; }

  void testValidateEligibilityQualifier_EligibilityInfoNull()
  {
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateEligibilityQualifier(
                                          *createPaxTypeFare(), 1, 0, 0, eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
  }

  void assertFareDisplayInfo(const FareDisplayInfo& fdi, int16_t catitem)
  {
    const std::set<uint16_t>& catNbrs = fdi._incompleteRuleCatNbrs;
    CPPUNIT_ASSERT(catNbrs.find(catitem) != catNbrs.end());
    CPPUNIT_ASSERT(fdi.incompleteR3Rule());
  }

  void testValidateEligibilityQualifier_EligibilityInfoNull_FdTrx()
  {
    populateFdTrx();
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    PaxTypeFare& ptf = *createPaxTypeFare();
    uint16_t catitem = 1;
    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateEligibilityQualifier(
                           ptf, catitem, 0, 0, eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), catitem);
    CPPUNIT_ASSERT(!eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
  }

  void populateDiagnostic(DiagnosticTypes diag)
  {
    _fareController->_trx.diagnostic().diagnosticType() = diag;
    _fareController->_trx.diagnostic().activate();
  }

  void testValidateEligibilityQualifier_CheckAccountCodeForDiscountRule()
  {
    populateDiagnostic(Diagnostic301);
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateEligibilityQualifier(
                                          *createPaxTypeFare(),
                                          1,
                                          createCategoryRuleInfo(RuleConst::CHILDREN_DISCOUNT_RULE),
                                          create<EligibilityInfo>(),
                                          eligibilityApp);
    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
    CPPUNIT_ASSERT_EQUAL(std::string("FARE PAX TYPE :  MIN AGE : 10 MAX AGE : 15\n"
                                     "R3 PAX TYPE   :  MIN AGE : 0 MAX AGE : 0\n"
                                     "R3 ACCOUNT CODE/CORP ID : \n"
                                     "QUALIFIED CAT1 TO CAT19\n"),
                         _trx->diagnostic().toString());
  }

  void testValidateEligibilityQualifier_CheckAccountCodeForNegotiatedRule()
  {
    populateDiagnostic(Diagnostic301);
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateEligibilityQualifier(
                                          *createPaxTypeFare(true),
                                          1,
                                          createCategoryRuleInfo(RuleConst::NEGOTIATED_RULE),
                                          create<EligibilityInfo>(),
                                          eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
    CPPUNIT_ASSERT_EQUAL(std::string("FARE PAX TYPE :  MIN AGE : 10 MAX AGE : 15\n"
                                     "R3 PAX TYPE   :  MIN AGE : 0 MAX AGE : 0\n"
                                     "R3 ACCOUNT CODE/CORP ID : \n"
                                     "QUALIFIED CAT1 TO CAT35\n"),
                         _trx->diagnostic().toString());
  }

  void testValidateEligibilityQualifier_CheckAccountCodeForSellingFare()
  {
    populateDiagnostic(Diagnostic301);
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateEligibilityQualifier(
                           *createPaxTypeFare(true),
                           1,
                           createCategoryRuleInfo(RuleConst::STOPOVER_RULE),
                           create<EligibilityInfo>(),
                           eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _trx->diagnostic().toString());
  }

  void testValidateEligibilityQualifier_Validation()
  {
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateEligibilityQualifier(
                                          *createPaxTypeFare(),
                                          1,
                                          createCategoryRuleInfo(RuleConst::STOPOVER_RULE),
                                          create<EligibilityInfo>(),
                                          eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(eligibilityApp._validate._run);
  }

  void testValidateEligibilityQualifier_Validation_FdTrx()
  {
    populateFdTrx();
    MockEligibility& eligibilityApp = *create<MockEligibility>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateEligibilityQualifier(
                                          *createPaxTypeFare(),
                                          1,
                                          createCategoryRuleInfo(RuleConst::STOPOVER_RULE),
                                          create<EligibilityInfo>(),
                                          eligibilityApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!eligibilityApp._checkAccountCode._run);
    CPPUNIT_ASSERT(!eligibilityApp._validate._run);
  }

  void testValidateDayTimeQualifier_DayTimeInfoNull()
  {
    MockDayTimeApplication& dayTimeApp = *create<MockDayTimeApplication>();
    Record3ReturnTypes expect = FAIL,
                       result = _fareController->validateDayTimeQualifier(
                           *createPaxTypeFare(), 2, createCategoryRuleInfo(), 0, dayTimeApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!dayTimeApp._run);
  }

  void testValidateDayTimeQualifier_DayTimeInfoNull_FdTrx()
  {
    populateFdTrx();
    MockDayTimeApplication& dayTimeApp = *create<MockDayTimeApplication>();

    PaxTypeFare& ptf = *createPaxTypeFare();
    uint16_t catitem = 2;

    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateDayTimeQualifier(
                           ptf, catitem, createCategoryRuleInfo(), 0, dayTimeApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), catitem);
    CPPUNIT_ASSERT(!dayTimeApp._run);
  }

  void testValidateDayTimeQualifier_Validation()
  {
    MockDayTimeApplication& dayTimeApp = *create<MockDayTimeApplication>();
    Record3ReturnTypes expect = PASS,
                       result = _fareController->validateDayTimeQualifier(*createPaxTypeFare(),
                                                                          2,
                                                                          createCategoryRuleInfo(),
                                                                          create<DayTimeAppInfo>(),
                                                                          dayTimeApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(dayTimeApp._run);
  }

  void testValidateDayTimeQualifier_Validation_FdTrx()
  {
    populateFdTrx();
    MockDayTimeApplication& dayTimeApp = *create<MockDayTimeApplication>();
    Record3ReturnTypes expect = PASS,
                       result = _fareController->validateDayTimeQualifier(*createPaxTypeFare(),
                                                                          2,
                                                                          createCategoryRuleInfo(),
                                                                          create<DayTimeAppInfo>(),
                                                                          dayTimeApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!dayTimeApp._run);
  }

  void testValidateSeasonalQualifier_SeasonalInfoNull()
  {
    MockSeasonalApplication& seasonalApp = *create<MockSeasonalApplication>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateSeasonalQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          0,
                                          seasonalApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!seasonalApp._run);
  }

  void testValidateSeasonalQualifier_SeasonalInfoNull_FdTrx()
  {
    populateFdTrx();
    MockSeasonalApplication& seasonalApp = *create<MockSeasonalApplication>();

    PaxTypeFare& ptf = *createPaxTypeFare();
    CategoryRuleItemInfo& ruleItem = *create<CategoryRuleItemInfo>();
    ruleItem.setItemcat(3);

    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateSeasonalQualifier(
                           ptf, ruleItem, createCategoryRuleInfo(), 0, seasonalApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), ruleItem.itemcat());
  }

  void testValidateSeasonalQualifier_Validation()
  {
    MockSeasonalApplication& seasonalApp = *create<MockSeasonalApplication>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateSeasonalQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          create<SeasonalAppl>(),
                                          seasonalApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(seasonalApp._run);
  }

  void testValidateSeasonalQualifier_Validation_FdTrx()
  {
    populateFdTrx();
    MockSeasonalApplication& seasonalApp = *create<MockSeasonalApplication>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateSeasonalQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          create<SeasonalAppl>(),
                                          seasonalApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!seasonalApp._run);
  }

  void testSetQualifyFltAppRuleData()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    uint16_t rule = RuleConst::STOPOVER_RULE;
    const CategoryRuleInfo* cri = createCategoryRuleInfo(rule);

    _fareController->setQualifyFltAppRuleData(ptf, cri);

    VecMap<uint32_t, QualifyFltAppRuleData*>& data = ptf.qualifyFltAppRuleDataMap();
    CPPUNIT_ASSERT(data.find(rule) != data.end());
    CPPUNIT_ASSERT_EQUAL(cri, data[rule]->categoryRuleInfo());
    CPPUNIT_ASSERT(ptf.isCategoryProcessed(rule));
  }

  void testValidateFlightApplicationQualifier_FlighInfoNull()
  {
    FlightApplication& flightApp = *create<MockFlightApplication>();
    Record3ReturnTypes expect = FAIL,
                       result = _fareController->validateFlightApplicationQualifier(
                           *createPaxTypeFare(), 4, "CX", createCategoryRuleInfo(), 0, flightApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testValidateFlightApplicationQualifier_FlighInfoNull_FdTrx()
  {
    populateFdTrx();
    FlightApplication& flightApp = *create<MockFlightApplication>();
    PaxTypeFare& ptf = *createPaxTypeFare();
    uint16_t catitem = 4;
    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateFlightApplicationQualifier(
                           ptf, catitem, "CX", createCategoryRuleInfo(), 0, flightApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), catitem);
  }

  void testValidateFlightApplicationQualifier_FdTrx()
  {
    populateFdTrx();
    FlightApplication& flightApp = *create<MockFlightApplication>();
    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateFlightApplicationQualifier(
                           *createPaxTypeFare(),
                           4,
                           "CX",
                           createCategoryRuleInfo(),
                           create<FlightAppRule>(),
                           flightApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testValidateFlightApplicationQualifier_FCOPhase()
  {
    FlightApplication& flightApp = *create<MockFlightApplication>();
    _fareController->setPhase(RuleItem::FCOPhase);
    Record3ReturnTypes expect = SOFTPASS,
                       result = _fareController->validateFlightApplicationQualifier(
                           *createPaxTypeFare(),
                           4,
                           "CX",
                           createCategoryRuleInfo(),
                           create<FlightAppRule>(),
                           flightApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testValidateFlightApplicationQualifier_Validation()
  {
    populateDiagnostic(Diagnostic304);
    FlightApplication& flightApp = *create<MockFlightApplication>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateFlightApplicationQualifier(
                                          *createPaxTypeFare(),
                                          4,
                                          "CX",
                                          createCategoryRuleInfo(),
                                          create<FlightAppRule>(),
                                          flightApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(std::string("----- FARE COLLECTION PHASE ----\n"
                                     "QUALIFIER FOR CAT 0\n"),
                         _trx->diagnostic().toString());
  }

  void testValidateTravelRestrictionsQualifier_TravelInfoNull()
  {
    MockTravelRestrictions& travelApp = *create<MockTravelRestrictions>();
    Record3ReturnTypes expect = FAIL,
                       result = _fareController->validateTravelRestrictionsQualifier(
                           *createPaxTypeFare(), 14, createCategoryRuleInfo(), 0, travelApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!travelApp._run);
  }

  void testValidateTravelRestrictionsQualifier_TravelInfoNull_FdTrx()
  {
    populateFdTrx();
    MockTravelRestrictions& travelApp = *create<MockTravelRestrictions>();
    PaxTypeFare& ptf = *createPaxTypeFare();
    uint16_t catitem = 14;
    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateTravelRestrictionsQualifier(
                           ptf, catitem, createCategoryRuleInfo(), 0, travelApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), catitem);
    CPPUNIT_ASSERT(!travelApp._run);
  }

  void testValidateTravelRestrictionsQualifier_Validation()
  {
    MockTravelRestrictions& travelApp = *create<MockTravelRestrictions>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateTravelRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          14,
                                          createCategoryRuleInfo(),
                                          create<TravelRestriction>(),
                                          travelApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(travelApp._run);
  }

  void testValidateTravelRestrictionsQualifier_Validation_FdTrx()
  {
    populateFdTrx();
    MockTravelRestrictions& travelApp = *create<MockTravelRestrictions>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateTravelRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          14,
                                          createCategoryRuleInfo(),
                                          create<TravelRestriction>(),
                                          travelApp);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!travelApp._run);
  }

  void testValidateSaleRestrictionsQualifier_SalesInfoNull()
  {
    MockSalesRestrictionRule& salesApp = *create<MockSalesRestrictionRule>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateSaleRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          0,
                                          salesApp,
                                          "");

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!salesApp._run);
  }

  void testValidateSaleRestrictionsQualifier_SalesInfoNull_FdTrx()
  {
    populateFdTrx();
    MockSalesRestrictionRule& salesApp = *create<MockSalesRestrictionRule>();

    PaxTypeFare& ptf = *createPaxTypeFare();
    CategoryRuleItemInfo& ruleItem = *create<CategoryRuleItemInfo>();
    ruleItem.setItemcat(15);

    Record3ReturnTypes expect = NOTPROCESSED,
                       result = _fareController->validateSaleRestrictionsQualifier(
                           ptf, ruleItem, createCategoryRuleInfo(), 0, salesApp, "");

    CPPUNIT_ASSERT_EQUAL(expect, result);
    assertFareDisplayInfo(*ptf.fareDisplayInfo(), ruleItem.itemcat());
    CPPUNIT_ASSERT(!salesApp._run);
  }

  void testValidateSaleRestrictionsQualifier_Validation()
  {
    MockSalesRestrictionRule& salesApp = *create<MockSalesRestrictionRule>();
    Record3ReturnTypes expect = PASS, result = _fareController->validateSaleRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          create<SalesRestriction>(),
                                          salesApp, "");

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(salesApp._run);
  }

  void testValidateSaleRestrictionsQualifier_Validation_FareByRule()
  {
    MockSalesRestrictionRule& salesApp = *create<MockSalesRestrictionRule>();
    salesApp._return = SKIP;

    Record3ReturnTypes expect = FAIL, result = _fareController->validateSaleRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(RuleConst::FARE_BY_RULE),
                                          create<SalesRestriction>(),
                                          salesApp, "");

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(salesApp._run);
  }

  void testValidateSaleRestrictionsQualifier_Validation_FdTrx()
  {
    populateFdTrx();
    MockSalesRestrictionRule& salesApp = *create<MockSalesRestrictionRule>();
    Record3ReturnTypes expect = FAIL, result = _fareController->validateSaleRestrictionsQualifier(
                                          *createPaxTypeFare(),
                                          *create<CategoryRuleItemInfo>(),
                                          createCategoryRuleInfo(),
                                          create<SalesRestriction>(),
                                          salesApp, "");

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT(!salesApp._run);
  }

  void testCreatePaxTypeFares_Fail_Location()
  {
    FareInfo fInfo;
    fInfo.market1() = "BAD";
    fInfo.market2() = "BAD";
    Fare* fare = createFare();
    fare->setFareInfo(&fInfo);

    _fareController->createPaxTypeFares(fare);
    CPPUNIT_ASSERT_EQUAL(0, int(_fareMarket->allPaxTypeFare().size()));
  }

  void testCreatePaxTypeFares_Pass()
  {
    _fareController->createPaxTypeFares(createFare("ATP"));
    CPPUNIT_ASSERT_EQUAL(1, int(_fareMarket->allPaxTypeFare().size()));
  }

  void testEliminateDuplicates_NoDuplicatedFares()
  {
    populateDiagnostic(Diagnostic258);
    Fare* fare[] = { createConstructedFare(), createFare("ATP") };
    std::vector<Fare*> constructed(1, fare[0]), published(1, fare[1]), result,
        expect;

    expect.push_back(fare[0]);
    expect.push_back(fare[1]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testEliminateDuplicates_DuplicatedFares()
  {
    populateDiagnostic(Diagnostic258);
    Fare* fare[] = { createConstructedFare(), createFare() };
    std::vector<Fare*> constructed(1, fare[0]), published(1, fare[1]), result,
        expect(1, fare[1]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testEliminateDuplicates_resizeResult()
  {
    populateDiagnostic(Diagnostic258);
    Fare* fare[] = { createConstructedFare(), createConstructedFare(), createConstructedFare(), createFare() };
    std::vector<Fare*> constructed(fare + 1, fare + 3), published(1, fare[3]), result(1, fare[0]),
        expect;

    expect.push_back(fare[0]);
    expect.push_back(fare[3]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testEliminateDuplicates_NullFare()
  {
    populateDiagnostic(Diagnostic258);
    Fare* fare[] = { 0, createFare(), 0 };
    std::vector<Fare*> constructed, published(fare, fare + 3), result, expect(1, fare[1]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _trx->diagnostic().toString());
  }

  void testEliminateDuplicates_Cat23Published()
  {
    populateDiagnostic(Diagnostic258);
    Fare* fare[] = { createFare(), createFare(Vendor::ATPCO) };
    fare[1]->status().set(Fare::FS_Cat23PublishedFail, true);

    std::vector<Fare*> constructed, published(fare, fare + 2), result, expect(1, fare[0]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _trx->diagnostic().toString());
  }

  void testEliminateDuplicates_SpecifiedOverConstructedYes()
  {
    populateDiagnostic(Diagnostic258);
    _fareMarket->governingCarrierPref() = createCarrierPreference('Y');

    Fare* fare[] = { createConstructedFare(), createFare() };

    std::vector<Fare*> constructed(1, fare[0]), published(1, fare[1]), result, expect(1, fare[1]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" \nPUBLISHED VS CONSTRUCTED DUPLICATES ELIMINATION\n"
                    "CARRIER PREFERS PUBLISHED FARES OVER CONSTRUCTED: YES\n"
                    "FARE MARKET: // - //\n"
                    "---------------------------------------------------------------\n"
                    "  MK1  MK2    EFF     EXP    O O    AMT/CUR    TAR CLASS\n"
                    "                             R I\n"
                    "---------------------------------------------------------------\n"
                    "P LON  FRA N/A N/A   T          0   \n"
                    "C LON  FRA N/A N/A   T          0   \n"
                    "CARRIER PREFERS PUBLISHED FARE OVER CONSTRUCTED\n"
                    "CONSTRUCTED FARE REMOVED\n \n"
                    "---------------------------------------------------------------\n"),
        _trx->diagnostic().toString());
  }

  void testEliminateDuplicates_SpecifiedOverConstructedNo()
  {
    populateDiagnostic(Diagnostic258);
    _fareMarket->governingCarrierPref() = createCarrierPreference('N');

    Fare* fare[] = { createConstructedFare(), createFare() };

    std::vector<Fare*> constructed(1, fare[0]), published(1, fare[1]), result, expect(1, fare[1]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" \nPUBLISHED VS CONSTRUCTED DUPLICATES ELIMINATION\n"
                    "CARRIER PREFERS PUBLISHED FARES OVER CONSTRUCTED: NO\n"
                    "FARE MARKET: // - //\n"
                    "---------------------------------------------------------------\n"
                    "  MK1  MK2    EFF     EXP    O O    AMT/CUR    TAR CLASS\n"
                    "                             R I\n"
                    "---------------------------------------------------------------\n"
                    "P LON  FRA N/A N/A   T          0   \n"
                    "C LON  FRA N/A N/A   T          0   \n"
                    "PUBLISHED FARE HAS LESS OR EQUAL FARE AMOUNT\n"
                    "CONSTRUCTED FARE REMOVED\n \n"
                    "---------------------------------------------------------------\n"),
        _trx->diagnostic().toString());
  }

  void testEliminateDuplicates_AmountDiff()
  {
    populateDiagnostic(Diagnostic258);
    _fareMarket->governingCarrierPref() = createCarrierPreference('N');

    Fare* fare[] = { createConstructedFare(-10), createFare() };

    std::vector<Fare*> constructed(1, fare[0]), published(1, fare[1]), result, expect(1, fare[0]);

    _fareController->eliminateDuplicates(constructed, published, result);

    CPPUNIT_ASSERT_EQUAL(expect, result);
    CPPUNIT_ASSERT_EQUAL(
        std::string(" \nPUBLISHED VS CONSTRUCTED DUPLICATES ELIMINATION\n"
                    "CARRIER PREFERS PUBLISHED FARES OVER CONSTRUCTED: NO\n"
                    "FARE MARKET: // - //\n"
                    "---------------------------------------------------------------\n"
                    "  MK1  MK2    EFF     EXP    O O    AMT/CUR    TAR CLASS\n"
                    "                             R I\n"
                    "---------------------------------------------------------------\n"
                    "P LON  FRA N/A N/A   T          0   \n"
                    "C LON  FRA N/A N/A   T          0   \n"
                    "CONSTRUCTED FARE HAS LESS FARE AMOUNT\n"
                    "PUBLISHED FARE REMOVED\n \n"
                    "---------------------------------------------------------------\n"),
        _trx->diagnostic().toString());
  }

  void testCreatePaxTypeFareESV_Fail_Resolve()
  {
    _trx->setCreateFareClassAppInfoBF(false);
    PaxTypeBucket paxTypeCortege;
    _fareController->createPaxTypeFareESV(createFare("VND"), &paxTypeCortege);
    CPPUNIT_ASSERT_EQUAL(0, int(paxTypeCortege.paxTypeFare().size()));
  }

  void testCreatePaxTypeFareESV_Pass()
  {
    PaxTypeBucket paxTypeCortege;
    _fareController->createPaxTypeFareESV(createFare("VND"), &paxTypeCortege);
    CPPUNIT_ASSERT_EQUAL(1, int(paxTypeCortege.paxTypeFare().size()));
  }

  void testCheckAge_requestAgeZero()
  {
    PaxType& pt = *createPaxType(CHILD);
    pt.age() = 0;
    CPPUNIT_ASSERT(_fareController->checkAge(*createPaxTypeFare(), pt));
  }

  void testCheckAge_requestAgeLessMinAge()
  {
    PaxType& pt = *createPaxType(CHILD);
    pt.age() = 5;
    CPPUNIT_ASSERT(!_fareController->checkAge(*createPaxTypeFare(), pt));
  }

  void testCheckAge_requestAgeMoreMaxAge()
  {
    PaxType& pt = *createPaxType(CHILD);
    pt.age() = 20;
    CPPUNIT_ASSERT(!_fareController->checkAge(*createPaxTypeFare(), pt));
  }

  void testCheckAge_requestAgeBetween()
  {
    PaxType& pt = *createPaxType(CHILD);
    pt.age() = 12;
    CPPUNIT_ASSERT(_fareController->checkAge(*createPaxTypeFare(), pt));
  }

  void populateSalesRestrictionRuleData(PaxTypeFare& ptf)
  {
    ptf.setRuleData(
        RuleConst::SALE_RESTRICTIONS_RULE, _trx->dataHandle(), create<PaxTypeFareRuleData>(), true);
  }

  void testValidateFareGroupPCC_SalesRestrictionPass()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    populateSalesRestrictionRuleData(ptf);
    ptf.actualPaxTypeItem() += PaxTypeFare::PAXTYPE_FAIL, 0, PaxTypeFare::PAXTYPE_FAIL,
        PaxTypeFare::PAXTYPE_FAIL;
    std::vector<uint16_t> expect = ptf.actualPaxTypeItem();

    CPPUNIT_ASSERT(_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void testValidateFareGroupPCC_SalesRestrictionFail()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    populateSalesRestrictionRuleData(ptf);
    ptf.actualPaxTypeItem() += PaxTypeFare::PAXTYPE_FAIL, PaxTypeFare::PAXTYPE_FAIL,
        PaxTypeFare::PAXTYPE_FAIL;
    std::vector<uint16_t> expect = ptf.actualPaxTypeItem();

    CPPUNIT_ASSERT(!_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void populateActualPaxTypeInCortage(std::vector<PaxTypeBucket>& cortage, PaxType* paxType)
  {
    cortage.resize(1);
    cortage[0].actualPaxType() += paxType;
  }

  void testValidateFareGroupPCC_NoSalesRestrictionNoPosPaxType()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    ptf.actualPaxTypeItem() += 0;
    std::vector<uint16_t> expect = ptf.actualPaxTypeItem();

    populateActualPaxTypeInCortage(ptf.fareMarket()->paxTypeCortege(), create<PaxType>());

    CPPUNIT_ASSERT(!_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void testValidateFareGroupPCC_NoSalesRestrictionPosPaxTypePositive()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    ptf.actualPaxTypeItem() += 0;
    std::vector<uint16_t> expect = ptf.actualPaxTypeItem();

    PosPaxType* paxType = create<PosPaxType>();
    paxType->pcc() = "ABC";
    paxType->positive() = false;

    populateActualPaxTypeInCortage(ptf.fareMarket()->paxTypeCortege(), paxType);

    CPPUNIT_ASSERT(_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void testValidateFareGroupPCC_NoSalesRestrictionNoPCC()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    ptf.actualPaxTypeItem() += 0;
    std::vector<uint16_t> expect(1, PaxTypeFare::PAXTYPE_FAIL);

    populateActualPaxTypeInCortage(ptf.fareMarket()->paxTypeCortege(), create<PosPaxType>());

    CPPUNIT_ASSERT(_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void testValidateFareGroupPCC_NoSalesRestrictionFail()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    ptf.actualPaxTypeItem() += PaxTypeFare::PAXTYPE_FAIL, PaxTypeFare::PAXTYPE_NO_MATCHED,
        PaxTypeFare::PAXTYPE_FAIL, PaxTypeFare::PAXTYPE_NO_MATCHED;
    std::vector<uint16_t> expect = ptf.actualPaxTypeItem();

    CPPUNIT_ASSERT(!_fareController->validateFareGroupPCC(ptf));
    CPPUNIT_ASSERT_EQUAL(expect, ptf.actualPaxTypeItem());
  }

  void testAddMatchedFareToPaxTypeBucket_Fail_EmptyCortage()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();

    _fareController->addMatchedFareToPaxTypeBucket(ptf);
    CPPUNIT_ASSERT_EQUAL(0, (int)_fareMarket->paxTypeCortege().size());
  }

  void testAddMatchedFareToPaxTypeBucket_Pass()
  {
    PaxTypeFare& ptf = *createPaxTypeFare();
    populateActualPaxTypeInCortage(_fareMarket->paxTypeCortege(), create<PaxType>());

    _fareController->addMatchedFareToPaxTypeBucket(ptf);
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().front().paxTypeFare().size());
  }

  void testFindFaresESV_Fail_CollectBoundFares()
  {
    _trx->setCollectBoundFares(false);
    std::vector<Fare*> fares;
    _fareController->findFaresESV(fares);
    CPPUNIT_ASSERT_EQUAL(0, (int)fares.size());
  }

  void testFindFaresESV_Fail_NoDataFromDatabase()
  {
    std::vector<Fare*> fares;
    _fareController->findFaresESV(fares);
    CPPUNIT_ASSERT_EQUAL(0, (int)fares.size());
  }

  void testFindFaresESV_Pass()
  {
    std::vector<Fare*> fares;
    Loc loc;
    loc.loc() = "AAA";
    _fareMarket->origin() = &loc;
    ;

    _fareController->findFaresESV(fares);
    CPPUNIT_ASSERT_EQUAL(1, (int)fares.size());
  }

  void testCreateFareTwoOnewaySimpleTrip()
  {
    FareInfo fInfo;
    fInfo.market1() = "ORG";
    fInfo.market2() = "DES";
    fInfo.directionality() = FROM;

    LocCode origin = "ORG";

    _itin->simpleTrip() = true;

    fallback::value::fallbackSimpleTripCorrectionOWFare.set(false);
    Fare* fare = _fareController->createFare(&fInfo, origin,
                           Fare::FS_International, 0);
    CPPUNIT_ASSERT(fare != 0);
  }

  // ----===##===----

  struct RunStatus
  {
    RunStatus() : _run(false), _return(PASS) {}

    mutable bool _run;
    Record3ReturnTypes _return;

    Record3ReturnTypes go() const
    {
      _run = true;
      return _return;
    }
  };

  class MockEligibility : public Eligibility
  {
  public:
    RunStatus _checkAccountCode, _validate;

    virtual Record3ReturnTypes checkAccountCode(const EligibilityInfo*,
                                                PaxTypeFare&,
                                                PricingTrx&,
                                                DCFactory* factory,
                                                const bool&,
                                                DiagCollector* diagPtr,
                                                bool) const
    {
      diagPtr->flushMsg();

      return _checkAccountCode.go();
    }

    Record3ReturnTypes validate(PricingTrx&,
                                Itin&,
                                PaxTypeFare&,
                                const RuleItemInfo*,
                                const FareMarket&,
                                const bool&,
                                const bool&)
    {
      return _validate.go();
    }
  };

  class MockTravelRestrictions : public TravelRestrictionsObserverWrapper, public RunStatus
  {
  public:
    Record3ReturnTypes
    validate(PricingTrx&, Itin&, PaxTypeFare&, const RuleItemInfo*, const FareMarket&) override
    {
      return go();
    }
  };

  class MockSalesRestrictionRule : public SalesRestrictionRuleWrapper, public RunStatus
  {
  public:
    Record3ReturnTypes validate(PricingTrx&,
                                Itin&,
                                FareUsage*,
                                PaxTypeFare&,
                                const CategoryRuleInfo&,
                                const CategoryRuleItemInfo*,
                                const SalesRestriction*,
                                bool,
                                bool&,
                                bool)
    {
      return go();
    }
  };

  class MockFlightApplication : public FlightApplication, public RunStatus
  {
  public:
    Record3ReturnTypes process(PaxTypeFare&, PricingTrx&) { return go(); }
  };

  class MockSeasonalApplication : public SeasonalApplication, public RunStatus
  {
  public:
    Record3ReturnTypes
    validate(PricingTrx&, Itin&, const PaxTypeFare&, const RuleItemInfo*, const FareMarket&)
    {
      return go();
    }
  };

  class MockDayTimeApplication : public DayTimeApplication, public RunStatus
  {
  public:
    Record3ReturnTypes
    validate(PricingTrx&, Itin&, const PaxTypeFare&, const RuleItemInfo*, const FareMarket&)
    {
      return go();
    }
  };

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    std::vector<TariffCrossRefInfo*> _tariffCrossRefInfoVec;
    TariffCrossRefInfo _tariffCrossRefInfo;
    std::vector<const FareClassAppInfo*> _fareClassAppInfoVec;
    FareClassAppInfo _fareClassAppInfo;
    FareClassAppSegInfo _fareClassAppSegInfo;
    std::vector<const FareInfo*> _fareInfoVec;
    FareInfo _fareInfo;

  public:
    MyDataHandle()
    {
      _tariffCrossRefInfoVec.push_back(&_tariffCrossRefInfo);
      _fareClassAppInfoVec.push_back(&_fareClassAppInfo);
      _fareClassAppInfo._segs.push_back(&_fareClassAppSegInfo);
      _fareInfoVec.push_back(&_fareInfo);
    }

    const std::vector<TariffCrossRefInfo*>&
    getTariffXRefByFareTariff(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const TariffNumber& fareTariff,
                              const DateTime& date)
    {
      if (vendor != "NO")
      {
        _tariffCrossRefInfoVec.front()->vendor() = vendor;
        return _tariffCrossRefInfoVec;
      }
      static std::vector<TariffCrossRefInfo*> ret;
      return ret;
    }
    const Indicator getTariffInhibit(const VendorCode& vendor,
                                     const Indicator tariffCrossRefType,
                                     const CarrierCode& carrier,
                                     const TariffNumber& fareTariff,
                                     const TariffCode& ruleTariffCode)
    {
      return vendor[0];
    }

    const std::vector<const FareClassAppInfo*>& getFareClassApp(const VendorCode& vendor,
                                                                const CarrierCode& carrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& ruleNumber,
                                                                const FareClassCode& fareClass)
    {
      return _fareClassAppInfoVec;
    }

    const std::vector<const FareClassAppInfo*> & getFareClassAppByTravelDT( const VendorCode &vendor,
                                                                            const CarrierCode &carrier,
                                                                            const TariffNumber &ruleTariff,
                                                                            const RuleNumber &ruleNumber,
                                                                            const FareClassCode &fareClass,
                                                                            DateTime travelDate)
      {
        if (carrier == "AA")
        {
          FareClassAppInfo* info = TestFareClassAppInfoFactory::create("/vobs/atseintl/test/testdata/data/FareClassAppInfo.xml");
          _fareClassAppInfoVec.push_back(info);
          return _fareClassAppInfoVec;
        }
        DateTime dt = boost::posix_time::time_from_string("2015-05-27 23:59:59.000");
        return DataHandleMock::getFareClassAppByTravelDT(vendor, carrier, ruleTariff, ruleNumber, fareClass, dt);
      }

    const FareTypeMatrix* getFareTypeMatrix(const FareType& key, const DateTime& date)
    {
      static FareTypeMatrix ftMatrix;
      return &ftMatrix;
    }

    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      if (locCode == "BAD")
        return NULL;
      return DataHandleMock::getLoc(locCode, date);
    }

    const std::vector<const FareInfo*>& getBoundFaresByMarketCxr(const LocCode& market1,
                                                                 const LocCode& market2,
                                                                 const CarrierCode& cxr,
                                                                 const DateTime& date)
    {
      static std::vector<const FareInfo*> empty;

      if (market1.empty())
        return empty;

      return _fareInfoVec;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareControllerTest);

} // tse

// unfortunately we can't use PrintCollection.h because of op<< for std::string :(
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& cont)
{
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<T>(os, " "));
  return os;
}
