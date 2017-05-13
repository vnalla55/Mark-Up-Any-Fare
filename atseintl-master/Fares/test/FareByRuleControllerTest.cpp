#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NUCInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareByRuleController.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "Fares/FareTypeMatcher.h"

#include "test/DBAccessMock/DataHandleMock.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"


using namespace std;

namespace tse
{

namespace
{
class FareByRuleControllerDataHandle : public DataHandleMock
{
public:
  FareByRuleControllerDataHandle() { _nucInfo._nucFactor = 1; }

  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    return &_nucInfo;
  }

protected:
  NUCInfo _nucInfo;
};

class FareByRuleControllerStub : public FareByRuleController
{
public:
  FareByRuleControllerStub(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
    : FareByRuleController(trx,
                           0, // will stub call to FCO instance
                           // with overriding FBRC::publishedFaresStep()
                           itin,
                           fareMarket)
  {
    _baseFareRules.push_back(&_baseFareRule);
    _baseFareRule.baseglobalDir() = GlobalDirection::ZZ;
    _baseFareRule.baseFareClass() = "FARECLAS";
    _baseFareRule.basepsgType() = ADULT;

    _industryFareAppl.push_back(&_indFareAppl);
    _indFareAppl.globalDir() = GlobalDirection::ZZ;
    _indFareAppl.selectionType() = MULTILATERAL;

    _fareByRuleItem.resultglobalDir() = GlobalDirection::XX;
    _fareByRuleItem.fareInd() = FareByRuleItemInfo::SPECIFIED_K;
    _ptfRuleData.ruleItemInfo() = &_fareByRuleItem;
  }

  void getPricingCurrency(const NationCode& nation, CurrencyCode& currency) const
  {
    if (nation == "PL")
      currency = "PLN";
    else if (nation == "DE")
      currency = "EUR";
    else if (nation == "US")
      currency = "USD";
  }

  bool matchNationCurrency(const NationCode& nation, const CurrencyCode& currency) const
  {
    return (currency == "PLN" && nation == "PL") || (currency == "EUR" && nation == "DE") ||
           (currency == "USD" && nation == "US");
  }

  ~FareByRuleControllerStub() {}

  const PaxTypeInfo* getPaxTypeInfo(const BaseFareRule& baseFareRule) const
  {
    return &_paxTypeInfo;
  }

  const FareByRuleItemInfo* getFareByRuleItem(const VendorCode& vendor, int itemNo) const
  {
    return &_fareByRuleItem;
  }

  const std::vector<const BaseFareRule*>&
  getBaseFareRule(const VendorCode& vendor, int itemNo) const
  {
    return _baseFareRules;
  }

  const Loc* getLoc(const LocCode& locCode) const { return &_loc; }

  const LocCode getMultiTransportCity(const LocCode& locCode) const { return locCode; }

  const std::vector<tse::FareByRuleApp*>& getFareByRuleApp(const std::string& corpId,
                                                           const std::string& accountCode,
                                                           const std::string& tktDesignator,
                                                           std::vector<PaxTypeCode>& paxTypes) const
  {
    return _fareByRuleApp;
  }

  const std::vector<const IndustryFareAppl*>* getIndustryFareAppl(Indicator selectionType) const
  {
    return &_industryFareAppl;
  }

  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier) const
  {
    return &_carrierPreference;
  }

  bool resolveFareTypeMatrix(PaxTypeFare& paxTypeFare, const FareClassAppInfo& fcaInfo) const
  {
    return true;
  }

  void putIntoPTF(PaxTypeFare& ptf, FareInfo& fareInfo) {}

  void setAllowedVendors(const VendorCode& baseVend)
  {
    if (_allowedVend.empty())
      _allowedVend.push_back(&baseVend);
  }

  void prevalidatePaxTypeFare(PricingTrx& trx, Itin& itin, PaxTypeFare& ptf) {}

  void doPercent(const MoneyAmount percent) { _calcMoney.nucValue() *= percent / 100.0; }

  bool fareByRuleAppValidate(FareByRuleAppValidator& validator,
                             FareByRuleProcessingInfo& fbrProcessingInfo,
                             std::map<std::string, bool>& ruleTariffMap) const
  {
    static TariffCrossRefInfo tcrInfo;
    fbrProcessingInfo.tcrInfo() = &tcrInfo;
    return true;
  }

  bool getFareByRuleCtrlInfo(FareByRuleApp& fbrApp,
                             std::vector<std::pair<FareByRuleCtrlInfo*, bool> >& fbrCtrlInfoVec,
                             DiagManager& diagManager)
  {
    fbrCtrlInfoVec.push_back(std::make_pair(&_fareByRuleCtrlInfo, false));
    return true;
  }

  FareInfo* createFareInfo(const MoneyAmount& fareAmt,
                           const CurrencyCode& currency,
                           const DateTime& effDate,
                           const DateTime& expDate)
  {
    return &_fareInfo;
  }

  void createFBRPaxTypeFareRuleData(PaxTypeFare& ptf, PaxTypeFare* baseFare)
  {
    ptf.setRuleData(25, _trx.dataHandle(), &_ptfRuleData);
  }

  uint16_t getTPM(const Loc& market1,
                  const Loc& market2,
                  const GlobalDirection& glbDir,
                  const DateTime& tvlDate) const
  {
    return abs(market1.loc()[0] - market2.loc()[0]);
  }

  void publishedFaresStep(FareMarket& targetMarket) const {}

  GeneralFareRuleInfo* getCat35Rec2(const PaxTypeFare& ptFare) const { return _cat35r2; }

  void setCat35Rec2(GeneralFareRuleInfo* cat35r2) { _cat35r2 = cat35r2; }

  // Data
  Loc _loc;
  PaxTypeInfo _paxTypeInfo;
  FareInfo _fareInfo;
  FareByRuleItemInfo _fareByRuleItem;
  FareByRuleCtrlInfo _fareByRuleCtrlInfo;
  FBRPaxTypeFareRuleData _ptfRuleData;
  IndustryFareAppl _indFareAppl;
  CarrierPreference _carrierPreference;
  BaseFareRule _baseFareRule;
  std::vector<const BaseFareRule*> _baseFareRules;
  std::vector<tse::FareByRuleApp*> _fareByRuleApp;
  std::vector<const IndustryFareAppl*> _industryFareAppl;
  GeneralFareRuleInfo* _cat35r2;
};
}

class FareByRuleControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareByRuleControllerTest);
  CPPUNIT_TEST(testProcess);
  /*
  CPPUNIT_TEST(testProcess_Diag225);
  CPPUNIT_TEST(testProcess_Diag325);

  CPPUNIT_TEST(testProcessRule_Fail_IndCxr);
  CPPUNIT_TEST(testProcessRule_Fail_ActPtc);
  CPPUNIT_TEST(testProcessRule_Pass_EmptyRuleInfoSet);
  CPPUNIT_TEST(testProcessRule_Pass_RuleInfoNULL);
  CPPUNIT_TEST(testProcessRule_Pass_RuleInfoSetWithIf);
  CPPUNIT_TEST(testProcessRule_Pass_RuleInfoSetWithAnd);
  CPPUNIT_TEST(testProcessRule_Pass_RuleInfoSet);
  CPPUNIT_TEST(testProcessRule_Pass_FD);

  CPPUNIT_TEST(testCreateSpecifiedFare);
  CPPUNIT_TEST(testCreateSpecifiedFare_Fail_YY);

  CPPUNIT_TEST(testCreateCalculatedFare);

  CPPUNIT_TEST(testCreateCalculatedFares_Pass);
  CPPUNIT_TEST(testCreateCalculatedFares_Pass_WithNotMatchedRec3);
  CPPUNIT_TEST(testCreateCalculatedFares_Fail_CalculateFareAmt);

  CPPUNIT_TEST(testProcessCalculatedFare_Fail_NoFaresInFareMarket);
  CPPUNIT_TEST(testProcessCalculatedFare_Fail_MatchFareToRule);
  CPPUNIT_TEST(testProcessCalculatedFare_Fail_NotApplied);
  CPPUNIT_TEST(testProcessCalculatedFare_Fail_CxrNotMatched);
  CPPUNIT_TEST(testProcessCalculatedFare_Pass);
  CPPUNIT_TEST(testProcessCalculatedFare_PassWithDiag);
  CPPUNIT_TEST(testProcessCalculatedFare_PassWithRequestedDiag);

  CPPUNIT_TEST(testInvalidDCT_Pass_WPNET_NotSelling);
  CPPUNIT_TEST(testInvalidDCT_Pass_WP_Selling);
  CPPUNIT_TEST(testInvalidDCT_Fail);

  CPPUNIT_TEST(testMatchFareToFbrItem_Pass_InvalidDCT);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_Industry);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_Normal);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_FareType);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_GlobalDir);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_Markets);
  CPPUNIT_TEST(testMatchFareToFbrItem_Pass_MarketsFD);
  CPPUNIT_TEST(testMatchFareToFbrItem_Fail_Pass);
  CPPUNIT_TEST(testMatchFareToRule_Pass);
  CPPUNIT_TEST(testMatchFareToRule_Fail_Tariff1);
  CPPUNIT_TEST(testMatchFareToRule_Fail_DifferentTariff);
  CPPUNIT_TEST(testMatchFareToRule_Fail_RuleNo);
  CPPUNIT_TEST(testMatchFareToRule_Fail_Carrier);
  CPPUNIT_TEST(testMatchFareToRule_Fail_OwRt);
  CPPUNIT_TEST(testMatchFareToRule_Fail_GlobalDir);
  CPPUNIT_TEST(testMatchFareToRule_Fail_PsgType);
  CPPUNIT_TEST(testMatchFareToRule_Fail_PsgTypeAdtWithAppSeg);
  CPPUNIT_TEST(testMatchFareToRule_Fail_PsgTypeWithAppSeg);
  CPPUNIT_TEST(testMatchFareToRule_Fail_FareClass);
  CPPUNIT_TEST(testMatchFareToRule_Fail_CatType);
  CPPUNIT_TEST(testMatchFareToRule_Fail_EmptyMlgRtg);
  CPPUNIT_TEST(testMatchFareToRule_Fail_MlgRtg);
  CPPUNIT_TEST(testMatchFareToRule_Fail_Rtg);
  CPPUNIT_TEST(testMatchFareToRule_Pass_Rtg);
  CPPUNIT_TEST(testMatchFareToRule_Fail_Footnote);
  CPPUNIT_TEST(testMatchFareToRule_Fail_BkgCode);
  CPPUNIT_TEST(testMatchFareToRule_Fail_BelowMin1);
  CPPUNIT_TEST(testMatchFareToRule_Fail_OverMax1);
  CPPUNIT_TEST(testMatchFareToRule_Fail_BelowMin2);
  CPPUNIT_TEST(testMatchFareToRule_Fail_OverMax2);
  CPPUNIT_TEST(testMatchFareToRule_Fail_Currency);

  CPPUNIT_TEST(testCalculateFareAmt_Unsupported_Indicator);
  CPPUNIT_TEST(testCalculateFareAmt_Percent);
  CPPUNIT_TEST(testCalculateFareAmt_Add);
  CPPUNIT_TEST(testCalculateFareAmt_Sub);
  CPPUNIT_TEST(testCalculateFareAmt_Sub_Resulting_Negative_Amt);
  CPPUNIT_TEST(testCalculateFareAmt_Add_Base);
  CPPUNIT_TEST(testCalculateFareAmt_Sub_Base);
  CPPUNIT_TEST(testCalculateFareAmt_Sub_Base_Resulting_Negative_Amt);

  CPPUNIT_TEST(testChangePaxType_PopulatePaxTypeInPaxTypeBucket);

  CPPUNIT_TEST(testFindPaxType_Pass);
  CPPUNIT_TEST(testFindPaxType_Fail);

  CPPUNIT_TEST(testProcessFareByRuleApp_NoNewAppls_NULL);
  CPPUNIT_TEST(testProcessFareByRuleApp_NoNewAppls_NotYY);
  CPPUNIT_TEST(testProcessFareByRuleApp_YY);

  CPPUNIT_TEST(testCalculateFareAmtPerMileage);

  CPPUNIT_TEST(testFindMinAndMaxFares_Sorted);
  CPPUNIT_TEST(testFindMinAndMaxFares_NotSorted);
  CPPUNIT_TEST(testFindMinAndMaxFares_Sorted_FailCur);
  CPPUNIT_TEST(testFindMinAndMaxFares_NotSorted_FailCur);
  CPPUNIT_TEST(testFindMinAndMaxFares_Sorted_FailMinRange);
  CPPUNIT_TEST(testFindMinAndMaxFares_NotSorted_FailMinRange);
  CPPUNIT_TEST(testFindMinAndMaxFares_Sorted_FailMaxRange);
  CPPUNIT_TEST(testFindMinAndMaxFares_NotSorted_FailMaxRange);
  CPPUNIT_TEST(testFindMinAndMaxFares_Sorted_FailBaseRuleTariff);
  CPPUNIT_TEST(testFindMinAndMaxFares_NotSorted_FailBaseRuleTariff);

  CPPUNIT_TEST(testDisplayBkcAvail_Fail_FD);
  CPPUNIT_TEST(testDisplayBkcAvail_Fail_DiagType);
  CPPUNIT_TEST(testDisplayBkcAvail_Pass_ExistingDiag);
  CPPUNIT_TEST(testDisplayBkcAvail_Pass_NewDiag);

  CPPUNIT_TEST(testProcessFare_Fail_NoCurrency);
  CPPUNIT_TEST(testProcessFare_Pass_Blank_FmBlank);
  CPPUNIT_TEST(testProcessFare_Pass_Blank_FmNotBlank);
  CPPUNIT_TEST(testProcessFare_Pass_Specified);
  CPPUNIT_TEST(testProcessFare_Pass_K);
  CPPUNIT_TEST(testProcessFare_Pass_E);
  CPPUNIT_TEST(testProcessFare_Pass_E_FmCur1);
  CPPUNIT_TEST(testProcessFare_Pass_E_FmCur2);
  CPPUNIT_TEST(testProcessFare_Pass_E_FmFail);
  CPPUNIT_TEST(testProcessFare_Pass_E_OrigCur1);
  CPPUNIT_TEST(testProcessFare_Pass_E_OrigCur2);
  CPPUNIT_TEST(testProcessFare_Pass_E_OrigFail);
  CPPUNIT_TEST(testProcessFare_Pass_E_DestCur1);
  CPPUNIT_TEST(testProcessFare_Pass_E_DestCur2);
  CPPUNIT_TEST(testProcessFare_Pass_E_DestFail);
  CPPUNIT_TEST(testProcessFare_Pass_F);
  CPPUNIT_TEST(testProcessFare_Fail_Calculated);

  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Highest_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Highest_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Highest_Cur2);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Highest_Fail);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Lowest_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Lowest_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Lowest_Cur2);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Lowest_Fail);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Other_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Other_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Other_Cur2);
  CPPUNIT_TEST(testEnsureMinMaxRange_Rec3Other_Fail);
  CPPUNIT_TEST(testEnsureMinMaxRange_Highest_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Highest_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Highest_Cur2);
  CPPUNIT_TEST(testEnsureMinMaxRange_Highest_Fail);
  CPPUNIT_TEST(testEnsureMinMaxRange_Lowest_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Lowest_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Lowest_Cur2);
  CPPUNIT_TEST(testEnsureMinMaxRange_Lowest_Fail);
  CPPUNIT_TEST(testEnsureMinMaxRange_Other_PassEmpty);
  CPPUNIT_TEST(testEnsureMinMaxRange_Other_Cur1);
  CPPUNIT_TEST(testEnsureMinMaxRange_Other_Cur2);

  CPPUNIT_TEST(testFindOtherFares_CxrMatch);
  CPPUNIT_TEST(testFindOtherFares_CxrNoMatch);

  CPPUNIT_TEST(testCreatePaxTypeFareMap);
  CPPUNIT_TEST(testCloneFareMarket);

  CPPUNIT_TEST(testMatchBookingCode_Fail_EmptyVec);
  CPPUNIT_TEST(testMatchBookingCode_Fail);
  CPPUNIT_TEST(testMatchBookingCode_Pass_Bkc1);
  CPPUNIT_TEST(testMatchBookingCode_Pass_Bkc1WithAsterisk);
  CPPUNIT_TEST(testMatchBookingCode_Pass_PassedBkc1Avail);
  CPPUNIT_TEST(testMatchBookingCode_Fail_MatchedBkc1Avail);
  CPPUNIT_TEST(testMatchBookingCode_Pass_Bkc2);
  CPPUNIT_TEST(testMatchBookingCode_Pass_Bkc2WithAsterisk);
  CPPUNIT_TEST(testMatchBookingCode_Pass_PassedBkc2Avail);
  CPPUNIT_TEST(testMatchBookingCode_Fail_MatchedBkc2Avail);
  CPPUNIT_TEST(testMatchBookingCode_BkcAvailMap_Empty);
  CPPUNIT_TEST(testMatchBookingCode_BkcAvailMap_NotEmpty);

  CPPUNIT_TEST(testUpdateFareMarket);
  CPPUNIT_TEST(testUpdatePaxTypeBucket);

  CPPUNIT_TEST(testMatchBaseFareVendorATP);
  CPPUNIT_TEST(testMatchFbrAppVendorATP);
  CPPUNIT_TEST(testMatchBaseFareVendorSITA);
  CPPUNIT_TEST(testMatchFbrAppVendorSITA);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFCWithFbrAppVendorSMFC);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFCWithFbrAppVendor5KAD);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFCWithFbrAppVendorSMFA);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFAWithFbrAppVendorSMFA);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFAWithFbrAppVendor5KAD);
  CPPUNIT_TEST(testMatchBaseFareVendorSMFAWithFbrAppVendorSMFC);
  CPPUNIT_TEST(testMatchBaseFareVendor5KADWithFbrAppVendor5KAD);
  CPPUNIT_TEST(testMatchBaseFareVendor5KADWithFbrAppVendorSMFC);

  CPPUNIT_TEST(testCheckSecurityPassWhenFareVendorIsAtpcoAndR8NotPCC);
  CPPUNIT_TEST(testCheckSecurityPassWhenFareVendorIsSitaAndR8NotPCC);
  CPPUNIT_SKIP_TEST(testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsSellingAndR8NotPCC);
  CPPUNIT_SKIP_TEST(
      testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsNetSubmitUpdAndR8NotPCC);
  CPPUNIT_TEST(testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsE);
  CPPUNIT_TEST(testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsSellingAndNoCat35Rec2);
  CPPUNIT_TEST(testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsNetAndNoCat35Rec2);
  CPPUNIT_TEST(testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsNetUpdAndNoCat35Rec2);
  CPPUNIT_TEST(testCheckSecurityFailWhenFareVendorIsSmfcAndDisplayTypeIsNetUpdAndNoCat35Rec2);
  CPPUNIT_TEST(testCheckSecurityFailWhenFareVendorIsSmfcAndDisplayTypeIsSellingAndNoCat35Rec2);
  CPPUNIT_TEST(testSetCombinedPercentReturnPositiveValue);
  CPPUNIT_TEST(testSetCombinedPercentReturnNegativeValue);
  */

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  FareByRuleController* _fbrc;
  PricingTrx* _trx;
  Itin* _itin;
  FareMarket* _fareMarket;
  PaxTypeFare* _baseFare;
  FareInfo* _fareInfo;
  PaxTypeFare* _calculatedFare;
  FareByRuleProcessingInfo* _fbrProcInfo;
  FareByRuleItemInfo* _fbrRuleItemInfo;
  FareByRuleCtrlInfo* _fbrCtrlInfo;
  CategoryRuleItemInfoSet* _catRuleItemInfoSet;
  RoutingUtil::PaxTypeFareMap _routingPtfMap;
  vector<CategoryRuleItemInfo*> _segQual;
  Loc* _locPL;
  Loc* _locUS;

public:
  void setUp()
  {
    _memHandle.create<FareByRuleControllerDataHandle>();
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->paxType().push_back(_memHandle.create<PaxType>());
    _trx->paxType().front()->actualPaxType()[CarrierCode("")] = &_trx->paxType();
    _trx->paxType().front()->paxTypeInfo() = _memHandle.create<PaxTypeInfo>();

    _fareMarket = _memHandle.create<FareMarket>();
    _fareMarket->paxTypeCortege().push_back(PaxTypeBucket());
    _fareMarket->paxTypeCortege().front().actualPaxType().push_back(_trx->paxType().front());
    _fareMarket->paxTypeCortege().front().requestedPaxType() = _trx->paxType().front();
    _itin = _memHandle.create<Itin>();
    _itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    _trx->itin().push_back(_itin);
    _fbrc = _memHandle.insert(new FareByRuleControllerStub(*_trx, *_itin, *_fareMarket));

    _locPL = _memHandle.create<Loc>();
    _locUS = _memHandle.create<Loc>();
    _locPL->nation() = "PL";
    _locUS->nation() = "US";
    _fareMarket->origin() = _locPL;
    _fareMarket->destination() = _locUS;
    _fareMarket->geoTravelType() = GeoTravelType::International;
    _trx->getRequest()->ticketingAgent()->agentLocation() = _locUS;

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = _locPL;
    seg->destination() = _locUS;

    _fareMarket->travelSeg().push_back(seg);

    setupBaseFare();
    _calculatedFare = _memHandle.create<PaxTypeFare>();

    _fbrRuleItemInfo = _memHandle.create<FareByRuleItemInfo>();
    _fbrRuleItemInfo->resultglobalDir() = GlobalDirection::XX;
    _fbrCtrlInfo = _memHandle.create<FareByRuleCtrlInfo>();

    CategoryRuleItemInfo catRuleItemInfo;
    catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::ELSE);

    _catRuleItemInfoSet = new CategoryRuleItemInfoSet();
    _catRuleItemInfoSet->push_back(catRuleItemInfo);
    _fbrCtrlInfo->addItemInfoSetNosync(_catRuleItemInfoSet);
    _fbrProcInfo = setupProcessingInfo();

    _fbrc->_creator.initCreationData(_fbrProcInfo,
                                     _memHandle.create<std::vector<CategoryRuleItemInfo> >(),
                                     _memHandle.create<CategoryRuleItemInfo>(),
                                     _memHandle.create<CategoryRuleItemInfoSet>(),
                                     false);

    _fbrc->_passedBkc1Avail = true;
    _fbrc->_matchedBkc1Avail = true;

  }

  void tearDown()
  {
    _memHandle.clear();
    _segQual.clear();
    _routingPtfMap.clear();
  }

  FareByRuleProcessingInfo* setupProcessingInfo()
  {
    FareByRuleProcessingInfo* ret = _memHandle.create<FareByRuleProcessingInfo>();
    FareByRuleApp* fbrApp = _memHandle.create<FareByRuleApp>();
    TariffCrossRefInfo* tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    DiagManager* diagManager = new DiagManager(*_trx);
    _memHandle.insert(diagManager);
    ret->initialize(_trx, _itin, _fareMarket, fbrApp, tcrInfo, _fbrRuleItemInfo, diagManager);
    ret->fbrCtrlInfo() = _fbrCtrlInfo;
    ret->fbrItemInfo() = _fbrRuleItemInfo;

    return ret;
  }

  void setupBaseFare()
  {
    _baseFare = _memHandle.create<PaxTypeFare>();

    // Fare
    Fare* fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* tcrRefInfo = _memHandle.create<TariffCrossRefInfo>();
    _baseFare->setFare(fare);
    fare->initialize(Fare::FS_ForeignDomestic, _fareInfo, *_fareMarket, tcrRefInfo);
    _fareInfo->_fareClass = "FARECLAS";
    _fareInfo->globalDirection() = GlobalDirection::ZZ;
    _fareInfo->originalFareAmount() = 100;
    _fareInfo->fareAmount() = 100;
    _fareInfo->currency() = "USD";
    _baseFare->nucFareAmount() = 100;
    tcrRefInfo->ruleTariff() = 5;

    // FareClassAppInfo
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    _baseFare->fareClassAppInfo() = appInfo;
    FareClassAppSegInfo* appSegInfo = _memHandle.create<FareClassAppSegInfo>();
    _baseFare->fareClassAppSegInfo() = appSegInfo;
    appInfo->_segs.push_back(appSegInfo);
    appSegInfo->_bookingCode[0] = "BK";

    _fareMarket->allPaxTypeFare().push_back(_baseFare);
    _baseFare->fareMarket() = _fareMarket;
  }

  BaseFareRule* setupBaseFareRule(const FareClassCode& fareClass)
  {
    BaseFareRule* ret = _memHandle.create<BaseFareRule>();
    ret->baseglobalDir() = GlobalDirection::ZZ;
    ret->baseFareClass() = fareClass.c_str();
    ret->basepsgType() = ADULT;

    return ret;
  }

  void createDiagnostic(DiagnosticTypes type = Diagnostic325)
  {
    _trx->diagnostic().diagnosticType() = type;
    _trx->diagnostic().activate();
    _fbrProcInfo->diagManager() = _memHandle.insert(new DiagManager(*_trx, type));

    _fbrc->_diag325Requested = (type == Diagnostic325);
    _fbrc->_diagWithRuleNumber = true;
    _fbrc->_matchedBkc1Avail = true;
    _fbrc->_matchedBkc2Avail = true;
  }

  void makeAxessAgent()
  {
    Customer* customer = _memHandle.create<Customer>();
    customer->crsCarrier() = "1J";
    customer->hostName() = "AXES";

    _trx->getRequest()->ticketingAgent()->agentTJR() = customer;
  }

  void setFbrRuleItemInfoCurValues(Indicator fareInd,
                                   const CurrencyCode& cur1,
                                   const CurrencyCode& cur2,
                                   const CurrencyCode& specCur1,
                                   const CurrencyCode& specCur2,
                                   MoneyAmount min1,
                                   MoneyAmount min2,
                                   MoneyAmount max1,
                                   MoneyAmount max2,
                                   MoneyAmount spec1,
                                   MoneyAmount spec2)
  {
    _fbrRuleItemInfo->fareInd() = fareInd;
    _fbrRuleItemInfo->minFareAmt1() = min1;
    _fbrRuleItemInfo->minFareAmt2() = min2;
    _fbrRuleItemInfo->maxFareAmt1() = max1;
    _fbrRuleItemInfo->maxFareAmt2() = max2;
    _fbrRuleItemInfo->specifiedFareAmt1() = spec1;
    _fbrRuleItemInfo->specifiedFareAmt2() = spec2;
    _fbrRuleItemInfo->cur1() = cur1;
    _fbrRuleItemInfo->cur2() = cur2;
    _fbrRuleItemInfo->specifiedCur1() = specCur1;
    _fbrRuleItemInfo->specifiedCur2() = specCur2;
  }

  void clearItemInfoSet()
  {
    tools::non_const(_fbrCtrlInfo->categoryRuleItemInfoSet()).clear();
  }

  //-----------------------------------------------------------------------------
  // TESTS
  //-----------------------------------------------------------------------------
  void testProcess()
  {
    CPPUNIT_ASSERT(_fbrc->process());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testProcess_Diag225()
  {
    createDiagnostic(Diagnostic225);
    CPPUNIT_ASSERT(_fbrc->process());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find("VENDOR") != std::string::npos);
  }

  void testProcess_Diag325()
  {
    createDiagnostic();
    CPPUNIT_ASSERT(_fbrc->process());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().find(" PSGR-") != std::string::npos);
  }

  void testProcessRule_Fail_IndCxr()
  {
    _fbrProcInfo->fbrApp()->carrier() = INDUSTRY_CARRIER;
    CPPUNIT_ASSERT(!_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
  }

  void testProcessRule_Fail_ActPtc()
  {
    _trx->paxType().clear();
    CPPUNIT_ASSERT(!_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
  }

  void testProcessRule_Pass_EmptyRuleInfoSet()
  {
    clearItemInfoSet();
    _fbrc->_fbrPaxTypeFares.clear();

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessRule_Pass_RuleInfoNULL()
  {
    clearItemInfoSet();
    _fbrCtrlInfo->addItemInfoSetNosync(nullptr);
    _fbrc->_fbrPaxTypeFares.clear();

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessRule_Pass_RuleInfoSetWithIf()
  {
    clearItemInfoSet();
    CategoryRuleItemInfo catRuleItemInfo;
    catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::IF);
    auto* catRuleItemInfoSet = new CategoryRuleItemInfoSet();
    catRuleItemInfoSet->push_back(catRuleItemInfo);
    _fbrCtrlInfo->addItemInfoSetNosync(catRuleItemInfoSet);

    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    _fbrRuleItemInfo->specifiedCur1() = FareByRuleController::BLANK_CURRENCY;

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessRule_Pass_RuleInfoSetWithAnd()
  {
    clearItemInfoSet();
    CategoryRuleItemInfo catRuleItemInfo;
    catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::AND);
    auto* catRuleItemInfoSet = new CategoryRuleItemInfoSet();
    catRuleItemInfoSet->push_back(catRuleItemInfo);
    _fbrCtrlInfo->addItemInfoSetNosync(catRuleItemInfoSet);

    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    _fbrRuleItemInfo->specifiedCur1() = FareByRuleController::BLANK_CURRENCY;

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessRule_Pass_RuleInfoSet()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    _fbrRuleItemInfo->specifiedCur1() = FareByRuleController::BLANK_CURRENCY;

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessRule_Pass_FD()
  {
    FareDisplayTrx trx;
    _fbrc->_fdTrx = &trx;

    _fbrc->_fbrPaxTypeFares.clear();

    CPPUNIT_ASSERT(_fbrc->processRule(*_fbrProcInfo, *_fbrCtrlInfo, false));
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testCreateSpecifiedFare()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    MoneyAmount fareAmt;
    CurrencyCode currency;

    _fbrc->createSpecifiedFare(*_fbrProcInfo, fareAmt, currency);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testCreateSpecifiedFare_Fail_YY()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();
    _fbrProcInfo->fbrApp()->carrier() = "YY";

    MoneyAmount fareAmt;
    CurrencyCode currency;

    _fbrc->createSpecifiedFare(*_fbrProcInfo, fareAmt, currency);

    CPPUNIT_ASSERT_EQUAL(0, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testCreateCalculatedFare()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    CPPUNIT_ASSERT(_fbrc->createCalculatedFare(_baseFare, *_calculatedFare, *_fbrProcInfo));
  }

  void testCreateCalculatedFares_Pass()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::CALCULATED;
    _fbrc->_fbrPaxTypeFares.clear();
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    _fbrc->createCalculatedFares(paxTypeFares, *_fbrProcInfo);
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testCreateCalculatedFares_Pass_WithNotMatchedRec3()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::CALCULATED;
    _fbrRuleItemInfo->baseFareType() = "WRONG";
    _fbrc->_fbrPaxTypeFares.clear();
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    _fbrc->createCalculatedFares(paxTypeFares, *_fbrProcInfo);
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testCreateCalculatedFares_Fail_CalculateFareAmt()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    _fbrc->createCalculatedFares(paxTypeFares, *_fbrProcInfo);
    CPPUNIT_ASSERT_EQUAL(0, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessCalculatedFare_Fail_NoFaresInFareMarket()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    _fareMarket->allPaxTypeFare().clear();

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().front().paxTypeFare().empty());
  }

  void testProcessCalculatedFare_Fail_MatchFareToRule()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    ((FareByRuleControllerStub*)_fbrc)->_baseFareRule.baseFareClass() = "WRONG";

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().front().paxTypeFare().empty());
  }

  void testProcessCalculatedFare_Fail_NotApplied()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    ((FareByRuleControllerStub*)_fbrc)->_baseFareRule.baseFareAppl() = 'N';

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().front().paxTypeFare().empty());
  }

  void testProcessCalculatedFare_Fail_CxrNotMatched()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    ((FareByRuleControllerStub*)_fbrc)->_baseFareRule.carrier() = "NO";

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().front().paxTypeFare().empty());
  }

  void testProcessCalculatedFare_Pass()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().front().paxTypeFare().size());
  }

  void testProcessCalculatedFare_PassWithDiag()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    createDiagnostic();
    _fareInfo->currency() = NUC;

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().front().paxTypeFare().size());
    CPPUNIT_ASSERT_EQUAL(
        0, (int)_fbrProcInfo->diagManager()->collector().str().find("RESULTING CAT25 INFORMATION"));
    CPPUNIT_ASSERT_EQUAL(string::npos,
                         _trx->diagnostic().toString().find("RESULTING CAT25 INFORMATION"));
  }

  void testProcessCalculatedFare_PassWithRequestedDiag()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;
    createDiagnostic();
    _fbrProcInfo->diagManager() = NULL;
    _fareInfo->currency() = NUC;

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().front().paxTypeFare().size());
    CPPUNIT_ASSERT_EQUAL(0, (int)_trx->diagnostic().toString().find("RESULTING CAT25 INFORMATION"));
  }

  void testInvalidDCT_Pass_WPNET_NotSelling()
  {
    makeAxessAgent();
    FareClassAppInfo appInfo;
    appInfo._displayCatType = RuleConst::BLANK;
    _baseFare->fareClassAppInfo() = &appInfo;
    _trx->getRequest()->wpNettRequested() = 'Y';
    CPPUNIT_ASSERT(_fbrc->invalidDCT(*_baseFare, *_fbrRuleItemInfo));
  }

  void testInvalidDCT_Pass_WP_Selling()
  {
    makeAxessAgent();
    FareClassAppInfo appInfo;
    appInfo._displayCatType = RuleConst::SELLING_FARE;
    _baseFare->fareClassAppInfo() = &appInfo;
    _trx->getRequest()->wpNettRequested() = 'N';
    CPPUNIT_ASSERT(_fbrc->invalidDCT(*_baseFare, *_fbrRuleItemInfo));
  }

  void testInvalidDCT_Fail() { CPPUNIT_ASSERT(!_fbrc->invalidDCT(*_baseFare, *_fbrRuleItemInfo)); }

  void testMatchFareToFbrItem_Pass_InvalidDCT()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    FareClassAppInfo appInfo;
    appInfo._displayCatType = RuleConst::NET_SUBMIT_FARE;
    _baseFare->fareClassAppInfo() = &appInfo;

    CPPUNIT_ASSERT(_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_Industry()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    IndustryFare fare;
    fare.validForPricing() = false;
    _baseFare->setFare(&fare);
    fare.setFareInfo(_fareInfo);

    CPPUNIT_ASSERT(!_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_Normal()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    _trx->getOptions()->normalFare() = 'Y';

    CPPUNIT_ASSERT(!_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_FareType()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    _trx->getOptions()->fareFamilyType() = 'N';

    CPPUNIT_ASSERT(!_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_GlobalDir()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    _baseFare->fare()->setGlobalDirectionValid(false);

    CPPUNIT_ASSERT(!_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_Markets()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    _fareInfo->market1() = "BAD";

    CPPUNIT_ASSERT(!_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Pass_MarketsFD()
  {
    _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    _fareInfo->market1() = "LOC";
    _locUS->loc() = "LOC";

    CPPUNIT_ASSERT(_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToFbrItem_Fail_Pass()
  {
    FareTypeMatcher ftm(*_trx);
    VendorCode vendor;
    _fbrc->setAllowedVendors(vendor);

    CPPUNIT_ASSERT(_fbrc->matchFareToFbrItem(*_baseFare, *_fbrProcInfo, ftm));
  }

  void testMatchFareToRule_Pass()
  {
    CPPUNIT_ASSERT(
        _fbrc->matchFareToRule(*_baseFare, *setupBaseFareRule("FARECLAS"), &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_Tariff1()
  {
    TariffCrossRefInfo tcrRefInfo;
    tcrRefInfo.tariffCat() = 1;
    _baseFare->fare()->setTariffCrossRefInfo(&tcrRefInfo);
    _baseFare->setFare(_baseFare->fare());
    CPPUNIT_ASSERT(
        !_fbrc->matchFareToRule(*_baseFare, *setupBaseFareRule("FARECLAS"), &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_DifferentTariff()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRuleTariff() = 99;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_RuleNo()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRuleNo() = "BAD";
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_Carrier()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->carrier() = "NN";
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_OwRt()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseowrt() = FareByRuleController::ASTERISK;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_GlobalDir()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseglobalDir() = GlobalDirection::XX;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_PsgType()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->basepsgType() = CHILD;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_PsgTypeAdtWithAppSeg()
  {
    FareClassAppSegInfo appSegInfo;
    appSegInfo._paxType = CHILD;
    _baseFare->fareClassAppSegInfo() = &appSegInfo;
    CPPUNIT_ASSERT(
        !_fbrc->matchFareToRule(*_baseFare, *setupBaseFareRule("FARECLAS"), &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_PsgTypeWithAppSeg()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->basepsgType() = CHILD;
    FareClassAppSegInfo appSegInfo;
    _baseFare->fareClassAppSegInfo() = &appSegInfo;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_FareClass()
  {
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *setupBaseFareRule("XXX"), &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_CatType()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->basepricingcatType() = FareByRuleController::ASTERISK;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_EmptyMlgRtg()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRouting() = FareByRuleController::MILEAGE_ROUTING;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, NULL));
  }

  void testMatchFareToRule_Fail_MlgRtg()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRouting() = FareByRuleController::MILEAGE_ROUTING;
    _routingPtfMap.insert(
        RoutingUtil::PaxTypeFareMap::value_type(_baseFare, UNKNOWN_ROUTING_FARE_TYPE));
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_Rtg()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRouting() = "BAD";
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Pass_Rtg()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseRouting() = "2015";
    FareInfo* fareInfo = const_cast<FareInfo*>(_baseFare->fare()->fareInfo());
    fareInfo->routingNumber() = "2015";
    CPPUNIT_ASSERT(_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }
  void testMatchFareToRule_Fail_Footnote()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->basefootNote1() = "X";
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_BkgCode()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->bookingCode1() = "X";
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_BelowMin1()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseminFare1() = 1000;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_OverMax1()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseMaxFare1() = 10;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_BelowMin2()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseCur1() = "BAD";
    rule->baseminFare1() = 10;
    rule->baseminFare2() = 1000;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_OverMax2()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseCur1() = "BAD";
    rule->baseminFare1() = 10;
    rule->baseMaxFare2() = 10;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testMatchFareToRule_Fail_Currency()
  {
    BaseFareRule* rule = setupBaseFareRule("FARECLAS");
    rule->baseCur1() = "BAD";
    rule->baseCur2() = "BAD";
    rule->baseminFare1() = 10;
    CPPUNIT_ASSERT(!_fbrc->matchFareToRule(*_baseFare, *rule, &_routingPtfMap));
  }

  void testCalculateFareAmt_Unsupported_Indicator()
  {
    _fbrRuleItemInfo->fareInd() = '*';

    CPPUNIT_ASSERT_EQUAL(false,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
  }

  void testCalculateFareAmt_Percent()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::CALCULATED;
    _fbrRuleItemInfo->percent() = 75;

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(75, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Add()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::ADD_SPECIFIED_TO_CALCULATED;
    _fbrRuleItemInfo->percent() = 75;
    _fbrRuleItemInfo->specifiedFareAmt1() = 50.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(125, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Sub()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SUBTRACT_SPECIFIED_FROM_CALCULATED;
    _fbrRuleItemInfo->percent() = 75;
    _fbrRuleItemInfo->specifiedFareAmt1() = 50.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Sub_Resulting_Negative_Amt()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SUBTRACT_SPECIFIED_FROM_CALCULATED;
    _fbrRuleItemInfo->percent() = 50;
    _fbrRuleItemInfo->specifiedFareAmt1() = 50.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Add_Base()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::ADD_SPECIFIED_TO_BASE_CALC_PERCENTAGE;
    _fbrRuleItemInfo->percent() = 10;
    _fbrRuleItemInfo->specifiedFareAmt1() = 50.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    _trx->ticketingDate() = DateTime(2026, 1, 1);

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Sub_Base()
  {
    _fbrRuleItemInfo->fareInd() =
        FareByRuleController::SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE;
    _fbrRuleItemInfo->percent() = 10;
    _fbrRuleItemInfo->specifiedFareAmt1() = 50.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    _trx->ticketingDate() = DateTime(2026, 1, 1);

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testCalculateFareAmt_Sub_Base_Resulting_Negative_Amt()
  {
    _fbrRuleItemInfo->fareInd() =
        FareByRuleController::SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE;
    _fbrRuleItemInfo->percent() = 10;
    _fbrRuleItemInfo->specifiedFareAmt1() = 500.00;
    _fbrRuleItemInfo->specifiedCur1() = "USD";

    _trx->ticketingDate() = DateTime(2026, 1, 1);

    CPPUNIT_ASSERT_EQUAL(true,
                         _fbrc->calculateFareAmt(*_baseFare, *_fbrRuleItemInfo, *_fbrProcInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _fbrc->_calcMoney.nucValue(), 0.01);
  }

  void testChangePaxType_PopulatePaxTypeInPaxTypeBucket()
  {
    _fbrc->changePaxType(*setupBaseFareRule(""), *_fareMarket);
    CPPUNIT_ASSERT_EQUAL(ADULT, _fareMarket->paxTypeCortege()[0].requestedPaxType()->paxType());
    CPPUNIT_ASSERT_EQUAL(ADULT, _fareMarket->paxTypeCortege()[0].actualPaxType()[0]->paxType());
  }

  void testFindPaxType_Pass()
  {
    _fbrc->changePaxType(*setupBaseFareRule(""), *_fareMarket);
    CPPUNIT_ASSERT(_fbrc->findPaxType(ADULT, _fareMarket->paxTypeCortege()));
  }

  void testFindPaxType_Fail()
  {
    CPPUNIT_ASSERT(!_fbrc->findPaxType(ADULT, _fareMarket->paxTypeCortege()));
  }

  void testProcessFareByRuleApp_NoNewAppls_NULL()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    std::map<string, bool> ruleTariffMap;
    _fbrc->processFareByRuleApp(NULL, diag, ruleTariffMap);

    CPPUNIT_ASSERT(!_fbrc->_multiAppls);
    CPPUNIT_ASSERT(!_fbrc->_indAppls);
  }

  void testProcessFareByRuleApp_NoNewAppls_NotYY()
  {
    _fbrProcInfo->fbrApp()->carrier() = "CX";
    DiagManager diag(*_trx, DiagnosticNone);
    std::map<string, bool> ruleTariffMap;
    _fbrc->processFareByRuleApp(_fbrProcInfo->fbrApp(), diag, ruleTariffMap);

    CPPUNIT_ASSERT(!_fbrc->_multiAppls);
    CPPUNIT_ASSERT(!_fbrc->_indAppls);
  }

  void testProcessFareByRuleApp_YY()
  {
    _fbrProcInfo->fbrApp()->carrier() = "YY";
    DiagManager diag(*_trx, DiagnosticNone);
    std::map<string, bool> ruleTariffMap;
    _fbrc->processFareByRuleApp(_fbrProcInfo->fbrApp(), diag, ruleTariffMap);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_multiAppls->size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_indAppls->size());
  }

  void testCalculateFareAmtPerMileage()
  {
    MoneyAmount fareAmount = 8.50;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        85, FareByRuleController::calculateFareAmtPerMileage(fareAmount, 1000), 0.01);
  }

  void testFindMinAndMaxFares_Sorted()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, _fbrc->findMinAndMaxFares(&paxTypeFares, true, *_fbrRuleItemInfo, "USD", 0), 0.01);
  }

  void testFindMinAndMaxFares_NotSorted()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100, _fbrc->findMinAndMaxFares(&paxTypeFares, false, *_fbrRuleItemInfo, "USD", 0), 0.01);
  }

  void testFindMinAndMaxFares_Sorted_FailCur()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, true, *_fbrRuleItemInfo, "XXX", 0), 0.01);
  }

  void testFindMinAndMaxFares_NotSorted_FailCur()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, false, *_fbrRuleItemInfo, "XXX", 0), 0.01);
  }

  void testFindMinAndMaxFares_Sorted_FailMinRange()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);
    _fbrRuleItemInfo->minFareAmt1() = 200;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, true, *_fbrRuleItemInfo, "", 0), 0.01);
  }

  void testFindMinAndMaxFares_NotSorted_FailMinRange()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);
    _fbrRuleItemInfo->minFareAmt1() = 200;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, false, *_fbrRuleItemInfo, "", 0), 0.01);
  }

  void testFindMinAndMaxFares_Sorted_FailMaxRange()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);
    _fbrRuleItemInfo->maxFareAmt1() = 50;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, true, *_fbrRuleItemInfo, "", 0), 0.01);
  }

  void testFindMinAndMaxFares_NotSorted_FailMaxRange()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);
    _fbrRuleItemInfo->maxFareAmt1() = 50;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, false, *_fbrRuleItemInfo, "", 0), 0.01);
  }

  void testFindMinAndMaxFares_Sorted_FailBaseRuleTariff()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, true, *_fbrRuleItemInfo, "", 7), 0.01);
  }

  void testFindMinAndMaxFares_NotSorted_FailBaseRuleTariff()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, _fbrc->findMinAndMaxFares(&paxTypeFares, false, *_fbrRuleItemInfo, "", 7), 0.01);
  }

  void testDisplayBkcAvail_Fail_FD()
  {
    createDiagnostic();
    _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    _fbrc->displayBkcAvail(*_fbrProcInfo, *setupBaseFareRule(""));

    CPPUNIT_ASSERT(_fbrProcInfo->diagManager()->collector().str().empty());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testDisplayBkcAvail_Fail_DiagType()
  {
    createDiagnostic();
    _fbrc->_diag325Requested = false;
    _fbrc->displayBkcAvail(*_fbrProcInfo, *setupBaseFareRule(""));

    CPPUNIT_ASSERT(_fbrProcInfo->diagManager()->collector().str().empty());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testDisplayBkcAvail_Pass_ExistingDiag()
  {
    createDiagnostic();
    _fbrc->displayBkcAvail(*_fbrProcInfo, *setupBaseFareRule(""));

    CPPUNIT_ASSERT(!_fbrProcInfo->diagManager()->collector().str().empty());
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testDisplayBkcAvail_Pass_NewDiag()
  {
    createDiagnostic();
    _fbrProcInfo->diagManager() = NULL;
    _fbrc->displayBkcAvail(*_fbrProcInfo, *setupBaseFareRule(""));

    CPPUNIT_ASSERT(!_trx->diagnostic().toString().empty());
  }

  void testProcessFare_Fail_NoCurrency()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessFare_Pass_Blank_FmBlank()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    _fbrRuleItemInfo->specifiedCur1() = FareByRuleController::BLANK_CURRENCY;
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(2, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_Blank_FmNotBlank()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();

    _fbrRuleItemInfo->specifiedCur1() = FareByRuleController::BLANK_CURRENCY;
    _fbrc->_fmCurrency = "CUR";
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_Specified()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fbrc->_fbrPaxTypeFares.clear();
    _fbrRuleItemInfo->specifiedCur1() = "PLN";
    _fbrRuleItemInfo->specifiedCur2() = "USD";
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(2, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_K()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_K;
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_FmCur1()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmCurrency = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "CUR";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();

    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_FmCur2()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmCurrency = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "CUR";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_FmFail()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmCurrency = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessFare_Pass_E_OrigCur1()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmOrigCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "CUR";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_OrigCur2()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmOrigCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "CUR";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_OrigFail()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmOrigCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessFare_Pass_E_DestCur1()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmDestCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "CUR";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_DestCur2()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmDestCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "CUR";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Pass_E_DestFail()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_E;
    _fbrc->_fmDestCurr = "CUR";
    _fbrRuleItemInfo->specifiedCur1() = "BAD";
    _fbrRuleItemInfo->specifiedCur2() = "BAD";
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testProcessFare_Pass_F()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED_F;
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->_fbrPaxTypeFares.size());
  }

  void testProcessFare_Fail_Calculated()
  {
    _fbrRuleItemInfo->fareInd() = 'X';
    _fbrc->_fbrPaxTypeFares.clear();
    PaxTypeFare dummyFare;

    _fbrc->processFare(*_fbrProcInfo, dummyFare);

    CPPUNIT_ASSERT(_fbrc->_fbrPaxTypeFares.empty());
  }

  void testEnsureMinMaxRange_Rec3Highest_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "", "", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Highest_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "CUR", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Highest_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "BAD", "CUR", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Highest_Fail()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "BAD", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Lowest_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "", "", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Lowest_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "CUR", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Lowest_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "BAD", "CUR", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Lowest_Fail()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "BAD", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Other_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "CUR", "CUR", "", "", 0, 2, 0, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Other_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "CUR", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Other_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "BAD", "CUR", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Rec3Other_Fail()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "BAD", "BAD", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        true, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Highest_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "", "", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Highest_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "", "", "CUR", "BAD", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Highest_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "", "", "BAD", "CUR", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Highest_Fail()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_HIGHEST, "", "", "BAD", "BAD", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Lowest_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "", "", "", "", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Lowest_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "", "", "CUR", "BAD", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Lowest_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "", "", "BAD", "CUR", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Lowest_Fail()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues(
        FareByRuleController::SELECT_LOWEST, "", "", "BAD", "BAD", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(!_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Other_PassEmpty()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "", "", "CUR", "CUR", 0, 2, 0, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Other_Cur1()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "", "", "CUR", "BAD", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testEnsureMinMaxRange_Other_Cur2()
  {
    MoneyAmount fareAmt = 0.5, minFareAmount = 0, maxFareAmount = 0;
    setFbrRuleItemInfoCurValues('X', "", "", "BAD", "CUR", 1, 2, 3, 4, 5, 6);

    CPPUNIT_ASSERT(_fbrc->ensureMinMaxRange(
        false, fareAmt, minFareAmount, maxFareAmount, "CUR", *_fbrRuleItemInfo));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, fareAmt, 0.01);
  }

  void testFindOtherFares_CxrMatch()
  {
    bool sorted = false;
    const std::vector<PaxTypeFare*>* otherFares;
    _fbrc->findOtherFares(*_fbrRuleItemInfo, otherFares, sorted);
    CPPUNIT_ASSERT(!sorted);
  }

  void testFindOtherFares_CxrNoMatch()
  {
    bool sorted = false;
    const std::vector<PaxTypeFare*>* otherFares;
    _fbrRuleItemInfo->carrier() = "NO";
    _fbrc->findOtherFares(*_fbrRuleItemInfo, otherFares, sorted);
    CPPUNIT_ASSERT(sorted);
  }

  void testCreatePaxTypeFareMap()
  {
    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(_baseFare);
    PTFRange range(paxTypeFares.begin(), paxTypeFares.end());
    CPPUNIT_ASSERT(_fbrc->createPaxTypeFareMap(range));
  }

  void testCloneFareMarket()
  {
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _fbrc->cloneFareMarket("AA")->governingCarrier());
  }

  void testMatchBookingCode_Fail_EmptyVec()
  {
    _baseFare->fareClassAppSegInfo() = NULL;
    CPPUNIT_ASSERT(!_fbrc->matchBookingCode("BK", "BK", *_baseFare));
  }

  void testMatchBookingCode_Fail()
  {
    CPPUNIT_ASSERT(!_fbrc->matchBookingCode("X", "X", *_baseFare));
  }

  void testMatchBookingCode_Pass_Bkc1()
  {
    CPPUNIT_ASSERT(_fbrc->matchBookingCode("BK", "X", *_baseFare));
  }

  void testMatchBookingCode_Pass_Bkc1WithAsterisk()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;
    CPPUNIT_ASSERT(_fbrc->matchBookingCode("B*", "X", *_baseFare));
  }

  void testMatchBookingCode_Pass_PassedBkc1Avail()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;
    _fbrc->_passedBkc1Avail = true;
    _fbrc->_matchedBkc1Avail = true;

    CPPUNIT_ASSERT(_fbrc->matchBookingCode("B*", "X", *_baseFare));
  }

  void testMatchBookingCode_Fail_MatchedBkc1Avail()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;
    _fbrc->_passedBkc1Avail = false;
    _fbrc->_matchedBkc1Avail = true;

    CPPUNIT_ASSERT(!_fbrc->matchBookingCode("B*", "X", *_baseFare));
  }

  void testMatchBookingCode_Pass_Bkc2()
  {
    CPPUNIT_ASSERT(_fbrc->matchBookingCode("X", "BK", *_baseFare));
  }

  void testMatchBookingCode_Pass_Bkc2WithAsterisk()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;

    _fbrc->_passedBkc2Avail = true;
    _fbrc->_matchedBkc2Avail = true;

    CPPUNIT_ASSERT(_fbrc->matchBookingCode("X", "B*", *_baseFare));
  }

  void testMatchBookingCode_Pass_PassedBkc2Avail()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;
    _fbrc->_passedBkc2Avail = true;
    _fbrc->_matchedBkc2Avail = true;

    CPPUNIT_ASSERT(_fbrc->matchBookingCode("X", "B*", *_baseFare));
  }

  void testMatchBookingCode_Fail_MatchedBkc2Avail()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    _baseFare->fareClassAppSegInfo() = &segInfo;
    _fbrc->_passedBkc2Avail = false;
    _fbrc->_matchedBkc2Avail = true;

    CPPUNIT_ASSERT(!_fbrc->matchBookingCode("X", "B*", *_baseFare));
  }

  void testMatchBookingCode_BkcAvailMap_Empty()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    segInfo._bookingCode[1] = "X";
    _baseFare->fareClassAppSegInfo() = &segInfo;

    CPPUNIT_ASSERT(_fbrc->matchBookingCode("B*", "X", *_baseFare));
    CPPUNIT_ASSERT(_fbrc->_baseFareInfoBkcAvailMap[_baseFare].empty());
  }

  void testMatchBookingCode_BkcAvailMap_NotEmpty()
  {
    FareClassAppSegInfo segInfo;
    segInfo._bookingCode[0] = "B";
    segInfo._bookingCode[1] = "X";
    _fbrc->_matchedBkc2Avail = true;
    _fbrc->_passedBkc2Avail = true;
    _baseFare->fareClassAppSegInfo() = &segInfo;

    CPPUNIT_ASSERT(_fbrc->matchBookingCode("B*", "X*", *_baseFare));
    const std::set<BookingCode>& bkcAvail = _fbrc->_baseFareInfoBkcAvailMap[_baseFare];
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), bkcAvail.size());
  }

  void testUpdateFareMarket()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SPECIFIED;
    _fareMarket->allPaxTypeFare().clear();

    MoneyAmount fareAmt;
    CurrencyCode currency;

    _fbrc->createSpecifiedFare(*_fbrProcInfo, fareAmt, currency);
    _fbrc->updateFareMarket();

    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->allPaxTypeFare().size());
  }

  void testUpdatePaxTypeBucket()
  {
    DiagManager diag(*_trx, DiagnosticNone);
    _fbrRuleItemInfo->fareInd() = FareByRuleController::SELECT_HIGHEST;

    _fbrc->processCalculatedFare(*_fbrProcInfo);
    _fbrc->updateFareMarket();
    CPPUNIT_ASSERT_EQUAL(1, (int)_fareMarket->paxTypeCortege().front().paxTypeFare().size());
  }

  void testMatchBaseFareVendorATP()
  {
    VendorCode baseFareVendor("ATP"), fbrAppVendor("5KAD");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchFbrAppVendorATP()
  {
    VendorCode baseFareVendor("SMFC"), fbrAppVendor("ATP");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSITA()
  {
    VendorCode baseFareVendor("SITA"), fbrAppVendor("5KAD");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchFbrAppVendorSITA()
  {
    VendorCode baseFareVendor("SMFC"), fbrAppVendor("SITA");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFCWithFbrAppVendorSMFC()
  {
    VendorCode baseFareVendor("SMFC"), fbrAppVendor("SMFC");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFCWithFbrAppVendor5KAD()
  {
    VendorCode baseFareVendor("SMFC"), fbrAppVendor("5KAD");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFCWithFbrAppVendorSMFA()
  {
    VendorCode baseFareVendor("SMFC"), fbrAppVendor("SMFA");
    CPPUNIT_ASSERT_EQUAL(0, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFAWithFbrAppVendorSMFA()
  {
    VendorCode baseFareVendor("SMFA"), fbrAppVendor("SMFA");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFAWithFbrAppVendor5KAD()
  {
    VendorCode baseFareVendor("SMFA"), fbrAppVendor("5KAD");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendorSMFAWithFbrAppVendorSMFC()
  {
    VendorCode baseFareVendor("SMFA"), fbrAppVendor("SMFC");
    CPPUNIT_ASSERT_EQUAL(0, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendor5KADWithFbrAppVendor5KAD()
  {
    VendorCode baseFareVendor("5KAD"), fbrAppVendor("5KAD");
    CPPUNIT_ASSERT_EQUAL(1, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testMatchBaseFareVendor5KADWithFbrAppVendorSMFC()
  {
    VendorCode baseFareVendor("5KAD"), fbrAppVendor("SMFC");
    CPPUNIT_ASSERT_EQUAL(0, (int)_fbrc->matchBaseFareVendor(baseFareVendor, fbrAppVendor));
  }

  void testCheckSecurityPassWhenFareVendorIsAtpcoAndR8NotPCC()
  {
    _fareInfo->vendor() = ATPCO_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    _fbrProcInfo->isFbrAppVendorPCC() = false;
    CPPUNIT_ASSERT(_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityPassWhenFareVendorIsSitaAndR8NotPCC()
  {
    _fareInfo->vendor() = SITA_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    _fbrProcInfo->isFbrAppVendorPCC() = false;
    CPPUNIT_ASSERT(_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsSellingAndR8NotPCC()
  {
    _fareInfo->vendor() = ATPCO_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::SELLING_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsNetSubmitUpdAndR8NotPCC()
  {
    _fareInfo->vendor() = ATPCO_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE_UPD;
    _baseFare->fareClassAppInfo() = appInfo;
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityPassWhenFareVendorIsAtpcoAndDisplayTypeIsE()
  {
    _fareInfo->vendor() = ATPCO_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = 'E';
    _baseFare->fareClassAppInfo() = appInfo;
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsSellingAndNoCat35Rec2()
  {
    _fareInfo->vendor() = SMF_ABACUS_CARRIER_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::SELLING_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    static_cast<FareByRuleControllerStub*>(_fbrc)->setCat35Rec2(NULL);
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(!_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsNetAndNoCat35Rec2()
  {
    _fareInfo->vendor() = SMF_ABACUS_CARRIER_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    static_cast<FareByRuleControllerStub*>(_fbrc)->setCat35Rec2(NULL);
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(!_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityFailWhenFareVendorIsSmfaAndDisplayTypeIsNetUpdAndNoCat35Rec2()
  {
    _fareInfo->vendor() = SMF_ABACUS_CARRIER_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE_UPD;
    _baseFare->fareClassAppInfo() = appInfo;
    static_cast<FareByRuleControllerStub*>(_fbrc)->setCat35Rec2(NULL);
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(!_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityFailWhenFareVendorIsSmfcAndDisplayTypeIsNetUpdAndNoCat35Rec2()
  {
    _fareInfo->vendor() = SMF_CARRIER_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::NET_SUBMIT_FARE_UPD;
    _baseFare->fareClassAppInfo() = appInfo;
    static_cast<FareByRuleControllerStub*>(_fbrc)->setCat35Rec2(NULL);
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(!_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testCheckSecurityFailWhenFareVendorIsSmfcAndDisplayTypeIsSellingAndNoCat35Rec2()
  {
    _fareInfo->vendor() = SMF_CARRIER_VENDOR_CODE;
    FareClassAppInfo* appInfo = _memHandle.create<FareClassAppInfo>();
    appInfo->_displayCatType = RuleConst::SELLING_FARE;
    _baseFare->fareClassAppInfo() = appInfo;
    static_cast<FareByRuleControllerStub*>(_fbrc)->setCat35Rec2(NULL);
    _fbrProcInfo->isFbrAppVendorPCC() = true;
    CPPUNIT_ASSERT(!_fbrc->matchBaseFareSecurity(*_baseFare, *_fbrProcInfo));
  }

  void testSetCombinedPercentReturnPositiveValue()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::CALCULATED;
    _fbrRuleItemInfo->percent() = 50.00;
    _fbrc->_largeFamilyDiscountPercent = 10.00;
    _fbrc->setCombinedPercent(*_fbrProcInfo);
    CPPUNIT_ASSERT(_fbrProcInfo->combinedPercent() == 40.00);
  }

  void testSetCombinedPercentReturnNegativeValue()
  {
    _fbrRuleItemInfo->fareInd() = FareByRuleController::CALCULATED;
    _fbrRuleItemInfo->percent() = 5.00;
    _fbrc->_largeFamilyDiscountPercent = 10.00;
    _fbrc->setCombinedPercent(*_fbrProcInfo);
    CPPUNIT_ASSERT(_fbrProcInfo->combinedPercent() == 0.00);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleControllerTest);
}
