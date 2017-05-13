#include "Common/Config/ConfigMan.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangeOverrides.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/MileageTypeData.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DBAccess/DataHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "Xform/CommonParserUtils.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingModelMap.h"
#include <boost/assign/std/vector.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XercesDefs.hpp>

#include <memory>
#include <unordered_set>

using namespace boost::assign;

namespace tse
{
FALLBACKVALUE_DECL(azPlusUp)

class PricingModelMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingModelMapTest);

  CPPUNIT_TEST(testCurrentTicketAgentCreatedWhenAnalyzingNewItin);
  CPPUNIT_TEST(testPreviousTicketAgentCreatedWhenAnalyzingExchangeItin);

  CPPUNIT_TEST(testSaveFareComponentException);
  CPPUNIT_TEST(testSaveFareComponentPass);

  CPPUNIT_TEST(testGetPassengerFareComponent_doNothingForInvalidPaxType);
  CPPUNIT_TEST(testGetPassengerFareComponent_invalidFareComponentNumber);
  CPPUNIT_TEST(testGetPassengerFareComponent_VariousFCVariousNumber);
  CPPUNIT_TEST(testGetPassengerFareComponent_VariousFCVariousNumberMultiPassenger);

  CPPUNIT_TEST(testHasFullVCTR);
  CPPUNIT_TEST(testSetVCTRpass);
  CPPUNIT_TEST(testSetVCTRfail1);
  CPPUNIT_TEST(testSetVCTRfail2);
  CPPUNIT_TEST(testSetVCTRfail3);
  CPPUNIT_TEST(testStoreVCTRtoFareCompInfoPass);
  CPPUNIT_TEST(testStoreVCTRtoFareCompInfoFail);

  CPPUNIT_TEST(testNoMatchItinRBDs);
  CPPUNIT_TEST(testNoMatchItinNoRBDs);
  CPPUNIT_TEST(testSetHIPPlusUp);
  CPPUNIT_TEST(testSetBHCPlusUp);

  CPPUNIT_TEST(testSaveFareComponent);
  CPPUNIT_TEST(testSaveFareComponentArunk);
  CPPUNIT_TEST(testSaveFareComponentEmbeddedArunk);
  CPPUNIT_TEST(testSaveFareComponentOneSideTrip);
  CPPUNIT_TEST(testSaveFareComponentTwoSideTrips);

  CPPUNIT_TEST(testSaveFareComponentSideTripAndEmbeddedArunks);
  CPPUNIT_TEST(testSaveFareComponentSideTripAndArunks);
  CPPUNIT_TEST(testSaveFareComponentSideTripAndArunksNoSideTripNums);

  CPPUNIT_TEST(testSaveFareComponentSideTripInSideTrip);
  CPPUNIT_TEST(testSaveFareComponentSideTripInSideTrip2);
  CPPUNIT_TEST(testSaveFareComponentSideTripInSideTripWithArunks);
  CPPUNIT_TEST(testSaveFareComponentSideTripInSideTripInSideTrip);

  CPPUNIT_TEST(testSaveFareComponentSideTripBySideTrip);
  CPPUNIT_TEST(testSaveFareComponentSideTripBySideTripInSideTrip);

  CPPUNIT_TEST(testPrepareRexTrx_secPE);
  CPPUNIT_TEST(testPrepareRexTrx_secFE);
  CPPUNIT_TEST(testPrepareRexTrx_secCE);
  CPPUNIT_TEST(testPrepareRexTrx_secEmpty);

  CPPUNIT_TEST(testStoreProcOptsInformation_MAC1);
  CPPUNIT_TEST(testSaveProcOptsInformation_MAC1);
  CPPUNIT_TEST(testStoreProcOptsInformation_MAC2);
  CPPUNIT_TEST(testSaveProcOptsInformation_MAC2);
  CPPUNIT_TEST(testStoreProcOptsInformation_MAC3);
  CPPUNIT_TEST(testSaveProcOptsInformation_MAC3);
  CPPUNIT_TEST(testStoreProcOptsInformation_MAC4);
  CPPUNIT_TEST(testSaveProcOptsInformation_MAC4);
  CPPUNIT_TEST(testStorePricingInformation_S96_FE);
  CPPUNIT_TEST(testStorePricingInformation_S96_PE);
  CPPUNIT_TEST(testStorePricingInformation_S96_Invalid);
  CPPUNIT_TEST(testStoreProcOptsInformation_SHC);
  CPPUNIT_TEST(testStoreProcOptsInformation_SAT_Y);
  CPPUNIT_TEST(testStoreProcOptsInformation_SAT_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_SAT_other);
  CPPUNIT_TEST(testStoreProcOptsInformation_SFT_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_SFT_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_Q0S_0);
  CPPUNIT_TEST(testStoreProcOptsInformation_Q0S_1);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1Z_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1Z_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1Y_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1Y_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_P20_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P20_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1W_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1W_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_P21_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P21_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1V_F);
  CPPUNIT_TEST(testStoreProcOptsInformation_P1V_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_AAL_T);
  CPPUNIT_TEST(testStoreProcOptsInformation_AAL_F);

  CPPUNIT_TEST(testShouldStoreOverridesReturnFalseForARTrxWithNoSecondaryExchange);
  CPPUNIT_TEST(testShouldStoreOverridesReturnTrueForARTrxWithSecondaryExchange);
  CPPUNIT_TEST(testShouldStoreOverridesReturnTrueForPORTTrx);
  CPPUNIT_TEST(testExchangeTypeNotCEReturnTrueForARTrxWithNoSecodaryExchange);
  CPPUNIT_TEST(testExchangeTypeNotCEReturnFalseForARTrxWithNoSecodaryExchange);
  CPPUNIT_TEST(testExchangeTypeNotCEReturnFalseForPORTTrxCEType);
  CPPUNIT_TEST(testExchangeTypeNotCEReturnFalseForPORTTrxNotCEType);
  CPPUNIT_TEST(testSaveStopOverOverridesCopiedToExchangeOverrides);
  CPPUNIT_TEST(testSavePlusUpOverridesCopiedToExchangeOverrides);
  CPPUNIT_TEST(testSaveMileageDataOverridesCopiedToExchangeOverrides);
  CPPUNIT_TEST(testSaveExchangeOverridesDontSaveWhenFareCompNumZero);
  CPPUNIT_TEST(testSaveExchangeOverridesDontSaveWhenFareBasisEmpty);
  CPPUNIT_TEST(testSaveExchangeOverridesDontSaveWhenSecondaryExchangeEmpty);
  CPPUNIT_TEST(testSaveExchangeOverridesDontSaveWhenCurrentTvlSegZero);
  CPPUNIT_TEST(testSaveExchangeOverridesDontSaveForExchangeItin);
  CPPUNIT_TEST(testSaveExchangeOverridesSavedForNewItin);
  CPPUNIT_TEST(testSaveExchangeOverridesSavedSurchargeOverridesForNewItin);

  CPPUNIT_TEST(testCheckSideTripThrowWhenStartWithoutPrevous);
  CPPUNIT_TEST(testCheckSideTripThrowWhenStartAndEndInOneSegment);
  CPPUNIT_TEST(testCheckSideTripThrowWhenNumberExistWithoutStart);
  CPPUNIT_TEST(testCheckSideTripThrowWhenStartWithoutNumber);
  CPPUNIT_TEST(testCheckSideTripPassWhenValidBegin);
  CPPUNIT_TEST(testCheckSideTripPassWhenInside);
  CPPUNIT_TEST(testCheckSideTripPassWhenValidEnd);
  CPPUNIT_TEST(testCheckSideTripUnsetInsideFlagWhenEnd);
  CPPUNIT_TEST(testStoreSegmentInformationThrowWhenInvalidSideTripDataAndCat31);
  CPPUNIT_TEST(testStoreSegmentInformationThrowWhenInvalidSideTripDataAndCat33);
  CPPUNIT_TEST(testStoreSegmentInformationDontThrowWhenInvalidSideTripDataAndPortExc);
  CPPUNIT_TEST(testStoreSegmentInformationDontThrowWhenInvalidSideTripDataAndNotExc);
  CPPUNIT_TEST(testStoreBillingInformationSavesS0R);
  CPPUNIT_TEST(testSetRestrictCmdPricing_setTrue);
  CPPUNIT_TEST(testSetRestrictCmdPricing_setFalse);
  CPPUNIT_TEST(testSetRestrictCmdPricing_exchangeItin_setFalse);
  CPPUNIT_TEST(testSetEPRKeywordFlag_MUL375);
  CPPUNIT_TEST(testSetEPRKeywordFlag_TMPCRS);
  CPPUNIT_TEST(testSetEPRKeywordFlag_CRSAGT);
  CPPUNIT_TEST(testSetEPRKeywordFlag_Other);
  CPPUNIT_TEST(testSetEPRKeywordFlag_exchangeItin);
  CPPUNIT_TEST(testRestrictCmdPricing);
  CPPUNIT_TEST(testgetRefundTypes);

  CPPUNIT_TEST(setFreqFlyerStatus_empty);
  CPPUNIT_TEST(setFreqFlyerStatus_ADT);
  CPPUNIT_TEST(setFreqFlyerStatus_ADT_CNN);
  CPPUNIT_TEST(setFreqFlyerStatus_ADT_UNKNOW);
  CPPUNIT_TEST(setFreqFlyerStatus_UNKNOW_INF);
  CPPUNIT_TEST(testCheckCat35NetSellOptionWhenTicketing);
  CPPUNIT_TEST(testCheckCat35NetSellOptionWhenNotTicketing);

  CPPUNIT_TEST(testTryExtractVersion);
  CPPUNIT_TEST(testSwsNoAltPricingResp);

  CPPUNIT_TEST(testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenNoExc);
  CPPUNIT_TEST(testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenTime_0);
  CPPUNIT_TEST(testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenTag10);
  CPPUNIT_TEST(testCheckAndAdjustPurchaseDTwithTimeForExchangeAdjustWhenAM);
  CPPUNIT_TEST(testBrandCodePass);
  CPPUNIT_TEST(testLegId);
  CPPUNIT_TEST(testLegIdAndUnknownAttr);
  CPPUNIT_TEST(testStoreProcOptsInformationPBB);
  CPPUNIT_TEST(testStoreProcOptsInformationPBBWpaRequest);
  CPPUNIT_TEST(testStoreProcOptsInformationPBBWqRequest);
  CPPUNIT_TEST(testRestrictExcludeFareFocusRule_XFF_EPR_No_Error);
  CPPUNIT_TEST(testRestrictExcludeFareFocusRule_NO_XFF_EPR_No_Error);
  CPPUNIT_TEST(testRestrictExcludeFareFocusRule_XFF_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_PDO_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_NO_PDO_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_PDO_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_PDR_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_NO_PDR_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_PDR_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_XRS_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_NO_XRS_EPR_No_Error);
  CPPUNIT_TEST(testStoreProcessingOptions_XRS_NOEPR_Error);

  CPPUNIT_SKIP_TEST(testCheckRtwSegment_FailFareBrk);
  CPPUNIT_SKIP_TEST(testCheckRtwSegment_FailSideTrip);
  CPPUNIT_SKIP_TEST(testCheckRtwSegment_Pass);

  CPPUNIT_TEST(testStorePassengerInformation_BasicFail);
  CPPUNIT_TEST(testStorePassengerInformation_BasicPass);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass1);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass2);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass3);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass4);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass5);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyPass6);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyFail1);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyFail2);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyFail3);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyFail4);
  CPPUNIT_TEST(testStorePassengerInformation_MaxPenaltyFail5);

  CPPUNIT_TEST(teststorePassengerTypeFlightInformation_paxTypeExists);
  CPPUNIT_TEST(teststorePassengerTypeFlightInformation_paxTypeNotExists);

  CPPUNIT_TEST(testStoreProcOptsInformationBRA);
  CPPUNIT_TEST(testStoreProcOptsInformationBRAWpaRequest);
  CPPUNIT_TEST(testStoreProcOptsInformationBRAWqRequest);
  CPPUNIT_TEST(testStoreRequestedFareBasisInformation);
  CPPUNIT_TEST(testStoreRequestedFareBasisInformationNotAllOptional);
  CPPUNIT_TEST(testStoreMulitpleRequestedFareBasisInformation);
  CPPUNIT_TEST(testCommandPricingAndSpecificFareBasisSameSegment);
  CPPUNIT_TEST(testSpecificFareBasisAndCommandPricingSameSegment);
  CPPUNIT_TEST(testCommandPricingAndSpecificFareBasisSeparateSegment);
  CPPUNIT_TEST(testSpecificFareBasisAndCommandPricingSeparateSegment);
  CPPUNIT_TEST(testSpecificFareBasisPositiveOneTravelSegWithOneFbc);
  CPPUNIT_TEST(testSpecificFareBasisPositiveOneTravelSegWithOneRfb);
  CPPUNIT_TEST(testSpecificFareBasisPositiveOneTravelSegWithTwoRfbSameFbc);
  CPPUNIT_TEST(testSpecificFareBasisPositiveOneTravelSegWithTwoRfbDiffFbc);
  CPPUNIT_TEST(testSpecificFareBasisPositiveTwoTravelSegWithSameFbcEach);
  CPPUNIT_TEST(testSpecificFareBasisPositiveTwoTravelSegWithDiffFbcEach);
  CPPUNIT_TEST(testSpecificFareBasisPositiveTwoTravelSegWithSameRfbEach);
  CPPUNIT_TEST(testSpecificFareBasisPositiveTwoTravelSegWithDiffRfbButSameFbcEach);
  CPPUNIT_TEST(testSpecificFareBasisPositiveTwoTravelSegWithDiffRfbEach);
  CPPUNIT_TEST(testSpecificFareBasisNegativeOneTravelSegWithSameTwoFbc);
  CPPUNIT_TEST(testSpecificFareBasisNegativeOneTravelSegWithDiffTwoFbc);
  CPPUNIT_TEST(testSpecificFareBasisNegativeOneTravelSegWithOneFbcAndOneRfb);
  CPPUNIT_TEST(testSpecificFareBasisNegativeOneTravelSegWithOneRfbAndOneFbc);
  CPPUNIT_TEST(testSpecificFareBasisNegativeOneTravelSegWithSameTwoRfb);
  CPPUNIT_TEST(testSpecificFareBasisNegativeTwoTravelSegWithFbcInFirstAndRfbInSecond);
  CPPUNIT_TEST(testSpecificFareBasisNegativeTwoTravelSegWithRfbInFirstAndFbcInSecond);
  CPPUNIT_TEST(testSpecificFareBasisThreeTravelSegOneWithoutRfbPart);
  CPPUNIT_TEST(testSpecificFareBasisThreeTravelSegTwoWithoutRfbPart);
  CPPUNIT_TEST(testSpecificFareBasisDisabledCombinationWithWpaWpasWq);
  CPPUNIT_TEST(testSpecificFareBasisDisabledCombinationWithWpocb);
  CPPUNIT_TEST(testSpecificFareBasisDisabledCombinationWithWpncWpncb);
  CPPUNIT_TEST(testSpecificFareBasisDisabledCombinationWithWpncs);

  CPPUNIT_TEST(testSaveFlightInformationDiscountAndPlusUpAmount);
  CPPUNIT_TEST(testStoringSegmentInformationForDiscountAndPlusUpAmount);
  CPPUNIT_TEST(testSaveFlightInformationDiscountAndPlusUpPercentage);
  CPPUNIT_TEST(testStoringSegmentInformationForDiscountAndPlusUpPercentage);
  CPPUNIT_TEST(testAddZeroDiscountForSegsWithNoDiscountIfReqHasDiscount);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestPB);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestYB);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestThrowException);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestUndefinedClass);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestInvalidLengthOneChar);
  CPPUNIT_TEST(testStoreProcOptsInformationBI0CabinRequestInvalidLengthThreeChar);

  CPPUNIT_TEST(testIsValidAmount);

  CPPUNIT_TEST(testCheckMAXOBFeeDefaultValue);
  CPPUNIT_TEST(testCheckMAXOBFeeValueYes);
  CPPUNIT_TEST(testCheckMAXOBFeeValueNo);
  CPPUNIT_TEST(testCheckMAXOBEmptyXMLTag);
  CPPUNIT_TEST(testCheckMAXOBFeeGarbageValue);

  CPPUNIT_TEST(testStoreProcOptsInformation_PRM_RCQ_NoError);
  CPPUNIT_TEST(testCheckArunkSegmentForSfrReqShouldThrowWhenAllTravelSegmentsAreArunk);
  CPPUNIT_TEST(testCheckArunkSegmentForSfrReqShouldNotThrowWhenAirTravelSegmentPresentInFareMarket);
  CPPUNIT_TEST(
      testCheckArunkSegmentForSfrReqShouldThrowWhenTravelSegmentIsAirButNonAirEquipmentType);
  CPPUNIT_TEST(testCheckArunkSegmentForSfrReqShouldNotThrowWhenOpenTravelSegmentPresentInFareMarket);
  CPPUNIT_TEST(testSaveXrayAttributes);

  CPPUNIT_TEST_SUITE_END();

protected:
  Trx* _baseTrx;
  RexPricingTrx* _trx;
  RexPricingRequest* _request;
  DataHandle _dataHandle;
  PricingModelMap* _pricingModelMap;
  MinimumFareOverride* _minFareOverride;
  TestMemHandle _memHandle;

  // mocks
  class MockAttributes : public xercesc::Attributes
  {
  public:
    typedef std::basic_string<XMLCh> XMLString;

    const XMLCh* getValue(const XMLCh*) const { return 0; }
    const XMLCh* getValue(const XMLCh*, const XMLCh*) const { return 0; }
    const XMLCh* getType(const XMLCh*) const { return 0; }
    const XMLCh* getType(const XMLCh*, const XMLCh*) const { return 0; }
    int getIndex(const XMLCh*) const { return 0; }
    int getIndex(const XMLCh*, const XMLCh*) const { return 0; }
    const XMLCh* getType(unsigned int) const { return 0; }
    const XMLCh* getQName(unsigned int) const { return 0; }
    const XMLCh* getURI(unsigned int) const { return 0; }

    unsigned int getLength() const { return (unsigned int)cont_.size(); }
    const XMLCh* getValue(unsigned int i) const { return cont_[i].second.c_str(); }
    const XMLCh* getLocalName(unsigned int i) const { return cont_[i].first.c_str(); }

    void add(const std::string& name, const std::string& value)
    {
      cont_.push_back(make_pair(convert(name), convert(value)));
    }

    static XMLString convert(const std::string& str)
    {
      XMLString target;
      copy(str.begin(), str.end(), back_inserter(target));
      return target;
    }

  protected:
    std::vector<std::pair<XMLString, XMLString> > cont_;
  }; // MockAttributes

  class MockFareCompInfo : public FareCompInfo
  {
  public:
    MockFareCompInfo() : _fm(new FareMarket)
    {
      fareMarket() = _fm.get();
      _fm->fareCompInfo() = this;
    }

    std::shared_ptr<FareMarket> _fm;
  }; // MockFareCompInfo

  class MockPricingModelMap : public PricingModelMap
  {
  public:
    MockPricingModelMap(tse::ConfigMan& _config, DataHandle& _dataHandle, Trx*& _trx)
      : PricingModelMap(_config, _dataHandle, _trx)
    {
    }

    FareCompInfo* getFareComponent(Itin* itin, uint16_t fareCompNum) const
    {
      FareCompInfo* curFc = findFareCompInfo(itin->fareComponent(), fareCompNum);

      if (curFc == 0)
      {
        localFC.push_back(std::shared_ptr<MockFareCompInfo>(new MockFareCompInfo));
        curFc = localFC.back().get();
        curFc->fareCompNumber() = fareCompNum;
        _excItin->fareComponent().push_back(curFc);
      }
      return curFc;
    }

    void sideTripBegin(uint16_t fareCompNum, TravelSeg* currentTvlSeg, uint16_t sideTripNumber = 0)
    {
      _sideTripStart = true;
      _sideTripEnd = false;
      segment(fareCompNum, currentTvlSeg, sideTripNumber);
    }

    void sideTripEnd(uint16_t fareCompNum, TravelSeg* currentTvlSeg, uint16_t sideTripNumber = 0)
    {
      _sideTripStart = false;
      _sideTripEnd = true;
      segment(fareCompNum, currentTvlSeg, sideTripNumber);
    }

    void segment(uint16_t fareCompNum, TravelSeg* currentTvlSeg, uint16_t sideTripNumber = 0)
    {
      _fareCompNum = fareCompNum;
      _currentTvlSeg = currentTvlSeg;
      _sideTripNumber = sideTripNumber;

      saveFareComponent();
      saveSegmentInformation();
    }

    bool corpIdExist() const { return true; }

  protected:
    mutable std::vector<std::shared_ptr<MockFareCompInfo>> localFC;

  }; // MockPricingModelMap

  // helper methods
  void addMembersToMap(PricingModelMap::Mapping& mapp)
  {
    mapp.members[_pricingModelMap->SDBMHash("S37")] = 37;
    mapp.members[_pricingModelMap->SDBMHash("B09")] = 38;
    mapp.members[_pricingModelMap->SDBMHash("S89")] = 39;
    mapp.members[_pricingModelMap->SDBMHash("S90")] = 40;
    mapp.members[_pricingModelMap->SDBMHash("SB2")] = 42;

    mapp.members[_pricingModelMap->SDBMHash("C6I")] = 16;
    mapp.members[_pricingModelMap->SDBMHash("Q17")] = 17;
    mapp.members[_pricingModelMap->SDBMHash("DMA")] = 43;
    mapp.members[_pricingModelMap->SDBMHash("DMP")] = 44;
    mapp.members[_pricingModelMap->SDBMHash("C48")] = 27;
    mapp.members[_pricingModelMap->SDBMHash("Q12")] = 28;
  }

  void buildPricingRequestWithTravelSegments(const std::vector<TravelSeg*>& travelsSegments)
  {
    Itin* itin = _memHandle.create<Itin>();
    _pricingModelMap->_pricingTrx->itin().push_back(itin);

    FareCompInfo* fareCompInfo = _memHandle.create<FareCompInfo>();
    _pricingModelMap->_pricingTrx->itin().front()->fareComponent().push_back(fareCompInfo);

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->travelSeg() = travelsSegments;

    fareCompInfo->fareMarket() = fareMarket;
  }

  void addSideTripMembersToMap(PricingModelMap::Mapping& mapp)
  {
    mapp.members[_pricingModelMap->SDBMHash("Q6E")] = 34;
    mapp.members[_pricingModelMap->SDBMHash("S07")] = 35;
    mapp.members[_pricingModelMap->SDBMHash("S08")] = 36;
  }

  StructuredRuleTrx* createSFRTrxType()
  {
    StructuredRuleTrx* sfrTrx = _memHandle.create<StructuredRuleTrx>();
    sfrTrx->setRequest(_memHandle.create<PricingRequest>());
    sfrTrx->setTrxType(PricingTrx::PRICING_TRX);
    sfrTrx->altTrxType() = PricingTrx::WP;
    sfrTrx->getRequest()->markAsSFR();

    sfrTrx->createMultiPassangerFCMapping();
    _pricingModelMap->_pricingTrx = sfrTrx;
    return sfrTrx;
  }

  void addSideTripAttributes(MockAttributes& attr,
                             const std::string& start,
                             const std::string& end,
                             const std::string& number)
  {
    attr.add("S07", start);
    attr.add("S08", end);
    attr.add("Q6E", number);
  }

  void populateExchangeOverridesInPricingModelMapObject()
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    _pricingModelMap->_fareCompNum = 1;
    _pricingModelMap->_fareBasisCode = "Y26";
    _trx->secondaryExcReqType() = TAG_10_EXCHANGE;
    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->_currentTvlSeg = airSeg;
    _pricingModelMap->_isExchangeItin = false;
    _pricingModelMap->_forcedSideTrip = 'T';
    _pricingModelMap->_mileageSurchargeCity = "DFW";
    _pricingModelMap->_mileageTktCity = "LON";
  }

  void setValidCorpId()
  {
    _trx->ticketingDate() = DateTime(2009, 1, 12, 12, 0, 0);
    _request->corporateID() = "AAA12";
  }

  void setOptionsForTkt(PricingOptions& opt)
  {
    _request->ticketEntry() = 'T';
    opt.cat35Net() = 'T';
    opt.cat35Sell() = 0;
    opt.cat35NetSell() = true;
    _trx->setOptions(&opt);
  }

  void setOptionsForNonTkt(PricingOptions* opt)
  {
    _request->ticketEntry() = ' ';
    opt->cat35Net() = 'T';
    opt->cat35Sell() = 0;
    opt->cat35NetSell() = true;
    _trx->setOptions(opt);
  }

  void
  setMapAndAttributesWithInvalidSideTripData(PricingModelMap::Mapping& mapp, MockAttributes& attr)
  {
    addSideTripMembersToMap(mapp);
    _pricingModelMap->_currentMapEntry = (void*)&mapp;

    addSideTripAttributes(attr, "T", "T", "1");
  }
  template <class T>
  void assertSize(const int& size, const T& v)
  {
    CPPUNIT_ASSERT(size_t(size) == v.size());
  }

  MockAttributes& createAttribute(const char * attr, const char * value, int id)
  {
    PricingModelMap::Mapping *map = this->_memHandle.create<PricingModelMap::Mapping>();
    map->func = 0;
    map->trxFunc = 0;

    map->members[_pricingModelMap->SDBMHash(attr)] = id;
    _pricingModelMap->_currentMapEntry = (void*)map;

    MockAttributes *attrs = this->_memHandle.create<MockAttributes>();
    attrs->add(attr, value);

    return *attrs;
  }

  MockAttributes* setUpPassengerInformationInPricingModelMap()
  {
    PricingModelMap::Mapping* map = _memHandle.create<PricingModelMap::Mapping>();
    map->func = 0;
    map->trxFunc = 0;
    map->members[_pricingModelMap->SDBMHash("B70")] = 1;
    map->members[_pricingModelMap->SDBMHash("Q0U")] = 2;
    map->members[_pricingModelMap->SDBMHash("Q0T")] = 3;
    map->members[_pricingModelMap->SDBMHash("A30")] = 4;
    map->members[_pricingModelMap->SDBMHash("Q79")] = 5;
    map->members[_pricingModelMap->SDBMHash("MPO")] = 6;
    _pricingModelMap->_currentMapEntry = map;

    MockAttributes* attr = _memHandle.create<MockAttributes>();
    _pricingModelMap->storeBillingInformation(*attr);
    return attr;
  }

  MockAttributes* setUpPenaltyInformationInPricingModelMap()
  {
    PricingModelMap::Mapping* map = _memHandle.create<PricingModelMap::Mapping>();
    map->func = 0;
    map->trxFunc = 0;
    map->members[_pricingModelMap->SDBMHash("ABD")] = 1;
    map->members[_pricingModelMap->SDBMHash("MPT")] = 2;
    map->members[_pricingModelMap->SDBMHash("MPI")] = 3;
    map->members[_pricingModelMap->SDBMHash("MPA")] = 4;
    map->members[_pricingModelMap->SDBMHash("MPC")] = 5;
    _pricingModelMap->_currentMapEntry = map;

    MockAttributes* attr = _memHandle.create<MockAttributes>();
    _pricingModelMap->storeBillingInformation(*attr);
    return attr;
  }

  void setUpPassengerTypeFlightInformationInPricingModelMap()
  {
    PricingModelMap::Mapping* map = _memHandle.create<PricingModelMap::Mapping>();
    map->func = 0;
    map->trxFunc = 0;
    map->members[_pricingModelMap->SDBMHash("B50")] = 1;
    map->members[_pricingModelMap->SDBMHash("Q6D")] = 2;
    map->members[_pricingModelMap->SDBMHash("B70")] = 3;
    map->members[_pricingModelMap->SDBMHash("C51")] = 4;
    _pricingModelMap->_currentMapEntry = map;
  }

  void setUpXrayInPricingModelMap()
  {
    PricingModelMap::Mapping* map = _memHandle.create<PricingModelMap::Mapping>();
    map->func = 0;
    map->trxFunc = 0;
    map->members[_pricingModelMap->SDBMHash("MID")] = 1;
    map->members[_pricingModelMap->SDBMHash("CID")] = 2;
    _pricingModelMap->_currentMapEntry = map;
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    xercesc::XMLPlatformUtils::Initialize();

    _trx = _memHandle.create<RexPricingTrx>();
    _request = _memHandle.create<RexPricingRequest>();
    _request->setTrx(_trx);
    _trx->setRequest(_request);
    _request->corporateID() = "SM4";
    _baseTrx = _trx;
    _memHandle.insert(_pricingModelMap =
                          new PricingModelMap(Global::config(), _dataHandle, _baseTrx));
    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->_trx = _trx;
    _pricingModelMap->_excItin = _memHandle.create<ExcItin>();
    _minFareOverride = _memHandle.create<MinimumFareOverride>();

    TestConfigInitializer::setValue("MAX_PENALTY_FAILED_FARES_THRESHOLD", 100, "PRICING_SVC");
    TestConfigInitializer::setValue("FEATURE_ENABLE", "Y", "XRAY");
  }

  void tearDown()
  {
    _memHandle.clear();
    xercesc::XMLPlatformUtils::Terminate();
  }

  void testSaveFareComponentException()
  {
    _pricingModelMap->_fareCompNum = 2;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->saveFareComponent(), tse::ErrorResponseException);
  }

  void testSaveFareComponentPass()
  {
    _pricingModelMap->_fareCompNum = 1;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->saveFareComponent());
  }

  void testGetPassengerFareComponent_doNothingForInvalidPaxType()
  {
    CPPUNIT_ASSERT(!_pricingModelMap->getPassengerFareComponent(nullptr,1));
  }

  void testGetPassengerFareComponent_invalidFareComponentNumber()
  {
    createSFRTrxType();

    PaxType* paxType = _memHandle.create<PaxType>();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->getPassengerFareComponent(paxType, 10),
                         tse::ErrorResponseException);
  }

  void testGetPassengerFareComponent_VariousFCVariousNumber()
  {
    StructuredRuleTrx* sfrTrx = createSFRTrxType();

    PaxType* paxType = _memHandle.create<PaxType>();

    //    GetPassengerFareComponent should return various fc if we passed various
    //    fare component number
    FareCompInfo* fc1 = _pricingModelMap->getPassengerFareComponent(paxType, 1);
    FareCompInfo* fc2 = _pricingModelMap->getPassengerFareComponent(paxType, 2);
    CPPUNIT_ASSERT(!(fc1 == fc2));

    //    If FC was created before getPassengerFareComponent should return the same pointer
    FareCompInfo* fc3 = _pricingModelMap->getPassengerFareComponent(paxType, 2);
    CPPUNIT_ASSERT(fc2 == fc3);

    auto& fcMap = *sfrTrx->getMultiPassengerFCMapping();

    //    We have one passenger so we expect size() == 1
    CPPUNIT_ASSERT(fcMap.size() == 1);
  }

  void testGetPassengerFareComponent_VariousFCVariousNumberMultiPassenger()
  {
    StructuredRuleTrx* sfrTrx = createSFRTrxType();

    PaxType* paxType1 = _memHandle.create<PaxType>();
    PaxType* paxType2 = _memHandle.create<PaxType>();
    PaxType* paxType3 = _memHandle.create<PaxType>();

    std::unordered_set<FareCompInfo*> uniqueFcSet;
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType1, 1));
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType1, 2));

    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType2, 1));
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType2, 2));
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType2, 3));

    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType3, 1));
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType3, 2));
    uniqueFcSet.insert(_pricingModelMap->getPassengerFareComponent(paxType3, 3));

    // There should be 8 unique FC
    CPPUNIT_ASSERT(uniqueFcSet.size() == 8);

    // We have 3 passengers so we expect fcMap.size() == 3
    auto& fcMap = *sfrTrx->getMultiPassengerFCMapping();
    CPPUNIT_ASSERT(fcMap.size() == 3);

    // We expect 2 passengers with 3 FC and one with 2 FC

    std::unordered_multiset<int> expected_set = {2, 3, 3};
    std::unordered_multiset<int> result_set;
    for (auto& mapElem : fcMap)
      result_set.insert(mapElem.second.size());

    CPPUNIT_ASSERT(expected_set == result_set);
  }

  void testCurrentTicketAgentCreatedWhenAnalyzingNewItin()
  {
    _pricingModelMap->_isExchangeItin = false;
    Agent* agent = _pricingModelMap->createAgent();
    CPPUNIT_ASSERT(agent != 0);
    CPPUNIT_ASSERT(_request->ticketingAgent() == agent);
    CPPUNIT_ASSERT(_pricingModelMap->getAgent(_pricingModelMap->_isExchangeItin) == agent);
  }

  void testPreviousTicketAgentCreatedWhenAnalyzingExchangeItin()
  {
    _pricingModelMap->_isExchangeItin = true;
    Agent* agent = _pricingModelMap->createAgent();
    CPPUNIT_ASSERT(agent != 0);
    CPPUNIT_ASSERT(_request->prevTicketIssueAgent() == agent);
    CPPUNIT_ASSERT(_pricingModelMap->getAgent(_pricingModelMap->_isExchangeItin) == agent);
  }

  void testSetHIPPlusUp()
  {
    MinimumFareModule moduleName = HIP;
    _minFareOverride->plusUpAmount = 0;
    CPPUNIT_ASSERT(_pricingModelMap->setPlusUpType(moduleName, _minFareOverride));
  }

  void testSetBHCPlusUp()
  {
    MinimumFareModule moduleName = CTM;
    _minFareOverride->boardPoint = "DFW";
    _minFareOverride->offPoint = "NYC";
    _minFareOverride->fareBoardPoint = "DFW";
    _minFareOverride->fareOffPoint = "LAX";
    _pricingModelMap->setPlusUpType(moduleName, _minFareOverride);
    CPPUNIT_ASSERT(moduleName == BHC);
  }

  void testHasFullVCTR()
  {
    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());

    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Vendor, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Carrier, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Tariff, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Rule, true);
    CPPUNIT_ASSERT(_pricingModelMap->hasFullVCTR());

    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Vendor, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Carrier, false);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Tariff, false);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Rule, false);
    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());

    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Vendor, false);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Carrier, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Tariff, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Rule, true);
    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());
  }

  void testSetVCTRpass()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    addMembersToMap(mapp);

    _pricingModelMap->_currentMapEntry = (void*)&mapp;

    MockAttributes attr;
    attr.add("S37", "ATP");
    attr.add("B09", "AA");
    attr.add("S89", "100");
    attr.add("S90", "5000");

    _pricingModelMap->storeSegmentInformation(attr);

    CPPUNIT_ASSERT(_pricingModelMap->hasFullVCTR());

    CPPUNIT_ASSERT_EQUAL(VendorCode("ATP"), _pricingModelMap->_VCTR.vendor());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _pricingModelMap->_VCTR.carrier());
    CPPUNIT_ASSERT_EQUAL(TariffNumber(100), _pricingModelMap->_VCTR.tariff());
    CPPUNIT_ASSERT_EQUAL(RuleNumber("5000"), _pricingModelMap->_VCTR.rule());
  }

  void testSetVCTRfail1()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    addMembersToMap(mapp);

    _pricingModelMap->_currentMapEntry = (void*)&mapp;

    MockAttributes attr;

    _pricingModelMap->storeSegmentInformation(attr);

    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Vendor));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Carrier));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Tariff));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Rule));

    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());
  }

  void testSetVCTRfail2()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    addMembersToMap(mapp);

    _pricingModelMap->_currentMapEntry = (void*)&mapp;

    MockAttributes attr;
    attr.add("S37", "");
    attr.add("B09", "");
    attr.add("S89", "");
    attr.add("S90", "");

    _pricingModelMap->storeSegmentInformation(attr);

    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Vendor));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Carrier));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Tariff));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Rule));

    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());
  }

  void testSetVCTRfail3()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    addMembersToMap(mapp);

    _pricingModelMap->_currentMapEntry = (void*)&mapp;

    MockAttributes attr;
    attr.add("S37", "ATP");
    attr.add("S89", "100");

    _pricingModelMap->storeSegmentInformation(attr);

    CPPUNIT_ASSERT(_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Vendor));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Carrier));
    CPPUNIT_ASSERT(_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Tariff));
    CPPUNIT_ASSERT(!_pricingModelMap->_hasVCTR.isSet(PricingModelMap::VCTR_Rule));

    CPPUNIT_ASSERT(!_pricingModelMap->hasFullVCTR());
  }

  void testStoreVCTRtoFareCompInfoPass()
  {
    VCTR vctr("ATP", "AA", 100, "5000", 0);
    _pricingModelMap->_VCTR = vctr;
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_AllSet, true);

    uint16_t fareCompNum = 1;
    _pricingModelMap->_fareCompNum = fareCompNum;
    _pricingModelMap->saveFareComponent();

    const std::vector<FareCompInfo*>& itinFc = _pricingModelMap->_excItin->fareComponent();

    FareCompInfo* curFc = _pricingModelMap->findFareCompInfo(itinFc, fareCompNum);

    CPPUNIT_ASSERT(curFc);
    CPPUNIT_ASSERT(curFc->hasVCTR());
    CPPUNIT_ASSERT_EQUAL(curFc->VCTR(), vctr);
  }

  void testStoreVCTRtoFareCompInfoFail()
  {
    VCTR vctr("ATP", "AA", 100, "5000", 0);
    _pricingModelMap->_VCTR = vctr;
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_AllSet, true);
    _pricingModelMap->_hasVCTR.set(PricingModelMap::VCTR_Tariff, false);

    uint16_t fareCompNum = 1;
    _pricingModelMap->_fareCompNum = fareCompNum;
    _pricingModelMap->saveFareComponent();

    const std::vector<FareCompInfo*>& itinFc = _pricingModelMap->_excItin->fareComponent();

    FareCompInfo* curFc = _pricingModelMap->findFareCompInfo(itinFc, fareCompNum);

    CPPUNIT_ASSERT(curFc);
    CPPUNIT_ASSERT(!curFc->hasVCTR());
    CPPUNIT_ASSERT_EQUAL(curFc->VCTR(), VCTR());
  }

  void testNoMatchItinNoRBDs()
  {
    NoPNRPricingTrx _trx;
    AirSeg seg;

    seg.setBookingCode("");
    _trx.travelSeg().push_back(&seg);

    _pricingModelMap->checkNoMatchItin(_trx);

    CPPUNIT_ASSERT(_trx.noRBDItin());
  }

  void testNoMatchItinRBDs()
  {
    NoPNRPricingTrx _trx;
    AirSeg seg;

    seg.setBookingCode("Y");
    _trx.travelSeg().push_back(&seg);

    _pricingModelMap->checkNoMatchItin(_trx);

    CPPUNIT_ASSERT(!_trx.noRBDItin());
  }

  void testSaveFareComponent()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[3];

    pmm.segment(1, air);
    pmm.segment(1, air + 1);
    pmm.segment(2, air + 2);

    assertSize(2, fc);
    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(0, fc[0]->fareMarket()->sideTripTravelSeg());

    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(0, fc[1]->fareMarket()->sideTripTravelSeg());

    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 0), fc[0]->fareMarket()->travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 1), fc[0]->fareMarket()->travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2), fc[1]->fareMarket()->travelSeg()[0]);
  }

  void testSaveFareComponentArunk()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[2];
    ArunkSeg arunk;

    pmm.segment(1, air);
    pmm.segment(0, &arunk);
    pmm.segment(1, air + 1);

    assertSize(1, fc);
    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(0, fc[0]->fareMarket()->sideTripTravelSeg());

    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 0), fc[0]->fareMarket()->travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 1), fc[0]->fareMarket()->travelSeg()[1]);
  }

  void testSaveFareComponentEmbeddedArunk()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[2];
    ArunkSeg arunk;

    pmm.segment(1, air);
    pmm.segment(1, &arunk);
    pmm.segment(1, air + 1);

    assertSize(1, fc);
    assertSize(3, fc[0]->fareMarket()->travelSeg());
    assertSize(0, fc[0]->fareMarket()->sideTripTravelSeg());

    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 0), fc[0]->fareMarket()->travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(&arunk), fc[0]->fareMarket()->travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 1), fc[0]->fareMarket()->travelSeg()[2]);
  }

  void testSaveFareComponentOneSideTrip()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[4];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, air + 2, 1);
    pmm.sideTripEnd(3, air + 3, 1);
    pmm.segment(1, air + 1);

    assertSize(3, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
  }

  void testSaveFareComponentTwoSideTrips()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[8];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, air + 2, 1);
    pmm.sideTripEnd(3, air + 3, 1);
    pmm.segment(1, air + 1);
    pmm.sideTripBegin(4, air + 4, 2);
    pmm.segment(5, air + 5, 2);
    pmm.sideTripEnd(5, air + 6, 2);
    pmm.segment(1, air + 7);

    assertSize(5, fc);

    assertSize(3, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(2, fc[4]->fareMarket()->travelSeg());

    assertSize(2, fc[0]->fareMarket()->sideTripTravelSeg());
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[1][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[1][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[0]->fareMarket()->sideTripTravelSeg()[1][2]);

    CPPUNIT_ASSERT_EQUAL('\0', air[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[1].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[4].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[5].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[6].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[7].forcedSideTrip());
  }

  void testSaveFareComponentSideTripAndEmbeddedArunks()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[4];
    ArunkSeg arunk[2];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, arunk, 1);
    pmm.segment(2, air + 2, 1);
    pmm.segment(3, air + 3, 1);
    pmm.sideTripEnd(3, arunk + 1, 1);
    pmm.segment(1, air + 1);

    assertSize(3, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(2, fc[1]->fareMarket()->travelSeg());
    assertSize(2, fc[2]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(arunk + 0),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(arunk + 1),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][3]);

    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', arunk[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', arunk[1].forcedSideTrip());
  }

  void testSaveFareComponentSideTripAndArunks()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[4];
    ArunkSeg arunk[2];

    pmm.segment(1, air);
    pmm.sideTripBegin(0, arunk, 1);
    pmm.segment(2, air + 2, 1);
    pmm.segment(3, air + 3, 1);
    pmm.sideTripEnd(0, arunk + 1, 1);
    pmm.segment(1, air + 1);

    assertSize(3, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);

    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', arunk[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', arunk[1].forcedSideTrip());
  }

  void testSaveFareComponentSideTripAndArunksNoSideTripNums()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[4];
    ArunkSeg arunk[2];

    pmm.segment(1, air);
    pmm.sideTripBegin(0, arunk);
    pmm.segment(2, air + 2, 1);
    pmm.segment(3, air + 3, 1);
    pmm.sideTripEnd(0, arunk + 1);
    pmm.segment(1, air + 1);

    assertSize(3, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);

    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', arunk[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', arunk[1].forcedSideTrip());
  }

  void testSaveFareComponentSideTripInSideTrip()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[8];

    pmm.segment(1, air);
    pmm.segment(1, air + 1);
    pmm.sideTripBegin(2, air + 4, 1);
    pmm.segment(3, air + 5, 1);
    pmm.sideTripBegin(4, air + 2, 2);
    pmm.sideTripEnd(5, air + 3, 2);
    pmm.sideTripEnd(3, air + 6, 1);
    pmm.segment(1, air + 7);

    assertSize(5, fc);

    assertSize(3, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(2, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(1, fc[4]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(1, fc[2]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[2]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][1]);

    CPPUNIT_ASSERT_EQUAL('\0', air[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[1].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[4].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[5].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[6].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[7].forcedSideTrip());
  }

  void testSaveFareComponentSideTripInSideTrip2()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[8];

    pmm.segment(1, air);
    pmm.segment(1, air + 1);
    pmm.sideTripBegin(2, air + 4, 1);
    pmm.sideTripBegin(4, air + 2, 2);
    pmm.sideTripEnd(5, air + 3, 2);
    pmm.segment(2, air + 5, 1);
    pmm.sideTripEnd(3, air + 6, 1);
    pmm.segment(1, air + 7);

    assertSize(5, fc);

    assertSize(3, fc[0]->fareMarket()->travelSeg());
    assertSize(2, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(1, fc[4]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(1, fc[1]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[1]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[1]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[1]->fareMarket()->sideTripTravelSeg()[0][1]);

    CPPUNIT_ASSERT_EQUAL('\0', air[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[1].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[4].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[5].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[6].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[7].forcedSideTrip());
  }

  void testSaveFareComponentSideTripInSideTripWithArunks()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[8];
    ArunkSeg arunk[4];

    pmm.segment(1, air);
    pmm.segment(1, air + 1);
    pmm.sideTripBegin(0, arunk);
    pmm.segment(2, air + 4, 1);
    pmm.segment(3, air + 5, 1);
    pmm.sideTripBegin(0, arunk + 2);
    pmm.segment(4, air + 2, 2);
    pmm.segment(5, air + 3, 2);
    pmm.sideTripEnd(0, arunk + 3);
    pmm.segment(3, air + 6, 1);
    pmm.sideTripEnd(0, arunk + 1);
    pmm.segment(1, air + 7);

    assertSize(5, fc);

    assertSize(3, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(2, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(1, fc[4]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(1, fc[2]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[2]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][1]);

    CPPUNIT_ASSERT_EQUAL('\0', air[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[1].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[4].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[5].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[6].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[7].forcedSideTrip());
  }

  void testSaveFareComponentSideTripInSideTripInSideTrip()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[10];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, air + 4, 1);
    pmm.segment(3, air + 5, 1);
    pmm.sideTripBegin(4, air + 1, 2);
    pmm.segment(5, air + 2, 2);
    pmm.sideTripBegin(6, air + 8, 3);
    pmm.sideTripEnd(7, air + 9, 3);
    pmm.sideTripEnd(5, air + 3, 2);
    pmm.sideTripEnd(3, air + 6, 1);
    pmm.segment(1, air + 7);

    assertSize(7, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(2, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(2, fc[4]->fareMarket()->travelSeg());
    assertSize(1, fc[5]->fareMarket()->travelSeg());
    assertSize(1, fc[6]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(1, fc[2]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[2]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 1),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(1, fc[4]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[4]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 8),
                         fc[4]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 9),
                         fc[4]->fareMarket()->sideTripTravelSeg()[0][1]);
  }

  void testSaveFareComponentSideTripBySideTrip()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[6];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, air + 4, 1);
    pmm.sideTripEnd(3, air + 5, 1);
    pmm.sideTripBegin(4, air + 2, 2);
    pmm.sideTripEnd(5, air + 3, 2);
    pmm.segment(1, air + 1);

    assertSize(5, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(1, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(1, fc[4]->fareMarket()->travelSeg());

    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);

    assertSize(2, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);

    assertSize(2, fc[0]->fareMarket()->sideTripTravelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[1][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[0]->fareMarket()->sideTripTravelSeg()[1][1]);

    CPPUNIT_ASSERT_EQUAL('\0', air[0].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('\0', air[1].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[2].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[3].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[4].forcedSideTrip());
    CPPUNIT_ASSERT_EQUAL('T', air[5].forcedSideTrip());
  }

  void testSaveFareComponentSideTripBySideTripInSideTrip()
  {
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm._trx = _trx;
    pmm._pricingTrx = _trx;
    ExcItin excItin;
    pmm._excItin = &excItin;
    std::vector<FareCompInfo*>& fc = excItin.fareComponent();
    AirSeg air[9];

    pmm.segment(1, air);
    pmm.sideTripBegin(2, air + 1, 1);
    pmm.segment(3, air + 2, 1);
    pmm.sideTripBegin(4, air + 3, 2);
    pmm.sideTripEnd(5, air + 4, 2);
    pmm.sideTripBegin(6, air + 5, 3);
    pmm.sideTripEnd(7, air + 6, 3);
    pmm.sideTripEnd(3, air + 7, 1);
    pmm.segment(1, air + 8);

    assertSize(7, fc);

    assertSize(2, fc[0]->fareMarket()->travelSeg());
    assertSize(1, fc[1]->fareMarket()->travelSeg());
    assertSize(2, fc[2]->fareMarket()->travelSeg());
    assertSize(1, fc[3]->fareMarket()->travelSeg());
    assertSize(1, fc[4]->fareMarket()->travelSeg());
    assertSize(1, fc[5]->fareMarket()->travelSeg());
    assertSize(1, fc[6]->fareMarket()->travelSeg());

    assertSize(1, fc[0]->fareMarket()->sideTripTravelSeg());
    assertSize(3, fc[0]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 1),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 2),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 7),
                         fc[0]->fareMarket()->sideTripTravelSeg()[0][2]);

    assertSize(2, fc[2]->fareMarket()->sideTripTravelSeg());
    assertSize(2, fc[2]->fareMarket()->sideTripTravelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 3),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 4),
                         fc[2]->fareMarket()->sideTripTravelSeg()[0][1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 5),
                         fc[2]->fareMarket()->sideTripTravelSeg()[1][0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<TravelSeg*>(air + 6),
                         fc[2]->fareMarket()->sideTripTravelSeg()[1][1]);
  }

  void testPrepareRexTrx_secPE()
  {
    std::string secReqType = "PE";
    const std::pair<std::string, std::string> refundTypes = make_pair(secReqType, secReqType);
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm.prepareRexPricingTrx(refundTypes);
    CPPUNIT_ASSERT_EQUAL(secReqType, (static_cast<RexPricingTrx*>(pmm._trx))->reqType());
    CPPUNIT_ASSERT_EQUAL(secReqType,
                         (static_cast<RexPricingTrx*>(pmm._trx))->secondaryExcReqType());
  }

  void testPrepareRexTrx_secFE()
  {
    std::string secReqType = "FE";
    const std::pair<std::string, std::string> refundTypes = make_pair(secReqType, secReqType);
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm.prepareRexPricingTrx(refundTypes);
    CPPUNIT_ASSERT_EQUAL(secReqType,
                         (static_cast<RexPricingTrx*>(pmm._trx))->secondaryExcReqType());
  }

  void testPrepareRexTrx_secCE()
  {
    std::string secReqType = "CE";
    const std::pair<std::string, std::string> refundTypes = make_pair(secReqType, secReqType);
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm.prepareRexPricingTrx(refundTypes);
    CPPUNIT_ASSERT_EQUAL(secReqType,
                         (static_cast<RexPricingTrx*>(pmm._trx))->secondaryExcReqType());
  }

  void testPrepareRexTrx_secEmpty()
  {
    std::string secReqType = "HEN";
    const std::pair<std::string, std::string> refundTypes = make_pair(secReqType, secReqType);
    MockPricingModelMap pmm(Global::config(), _dataHandle, _baseTrx);
    pmm.prepareRexPricingTrx(refundTypes);
    CPPUNIT_ASSERT(static_cast<RexPricingTrx*>(pmm._trx)->secondaryExcReqType().empty());
  }

  void mockPricingModelMapPointer()
  {
    ExcItin* transferExcItin = _pricingModelMap->_excItin;
    _pricingModelMap =
        _memHandle.insert(new MockPricingModelMap(Global::config(), _dataHandle, _baseTrx));
    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->_trx = _trx;
    _pricingModelMap->_excItin = transferExcItin;
  }

  void testStorePricingInformation_S96_FE()
  {
    _pricingModelMap->storePricingInformation(createAttribute("S96", FULL_EXCHANGE.c_str(), 1));

    CPPUNIT_ASSERT( _pricingModelMap->_pricingTrx->isSkipGsa() );
    CPPUNIT_ASSERT( _pricingModelMap->_pricingTrx->isSkipNeutral() );
    CPPUNIT_ASSERT( !_pricingModelMap->_pricingTrx->onlyCheckAgreementExistence() );
  }

  void testStorePricingInformation_S96_PE()
  {
    _pricingModelMap->storePricingInformation(createAttribute("S96", PARTIAL_EXCHANGE.c_str(), 1));
    CPPUNIT_ASSERT( _pricingModelMap->_pricingTrx->isSkipGsa() );
    CPPUNIT_ASSERT( _pricingModelMap->_pricingTrx->isSkipNeutral() );
    CPPUNIT_ASSERT( _pricingModelMap->_pricingTrx->onlyCheckAgreementExistence() );
  }

  void testStorePricingInformation_S96_Invalid()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("S96", "XX", 1));
    CPPUNIT_ASSERT( !_pricingModelMap->_pricingTrx->isSkipGsa() );
    CPPUNIT_ASSERT( !_pricingModelMap->_pricingTrx->isSkipNeutral() );
    CPPUNIT_ASSERT( !_pricingModelMap->_pricingTrx->onlyCheckAgreementExistence() );
  }

  void testStoreProcOptsInformation_SHC()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SHC", "TESTTEST", 176));
    CPPUNIT_ASSERT_EQUAL(std::string("TESTTEST"), _trx->getRequest()->getTourCode());
  }

  void testStoreProcOptsInformation_MAC1()
  {
    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(createAttribute("SM1", "ACCNTCODE01", 148));

    assertSize(0, _trx->getRequest()->incorrectCorpIdVec());
    assertSize(0, _trx->getRequest()->corpIdVec());
    assertSize(1, _trx->getRequest()->accCodeVec());

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
  }

  void testSaveProcOptsInformation_MAC1()
  {
    mockPricingModelMapPointer();
    setValidCorpId();

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(createAttribute("SM1", "ACCNTCODE01", 148));

    _pricingModelMap->saveProcOptsInformation();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }

  void testStoreProcOptsInformation_MAC2()
  {
    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(createAttribute("S11", "ACCNTCODE01", 91));

    assertSize(0, _trx->getRequest()->incorrectCorpIdVec());
    assertSize(0, _trx->getRequest()->corpIdVec());
    assertSize(0, _trx->getRequest()->accCodeVec());

    CPPUNIT_ASSERT(_trx->getRequest()->accountCode() == "ACCNTCODE01");
  }

  void testSaveProcOptsInformation_MAC2()
  {
    mockPricingModelMapPointer();
    setValidCorpId();

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(createAttribute("S11", "ACCNTCODE01", 91));
    _pricingModelMap->saveProcOptsInformation();

    CPPUNIT_ASSERT(!_trx->getRequest()->isMultiAccCorpId());
  }

  void testStoreProcOptsInformation_MAC3()
  {
    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("S11")] = 91;
    map.members[_pricingModelMap->SDBMHash("SM1")] = 148;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("S11", "ACCNTCODE01");
    attrs.add("SM1", "ACCNTCODE01");

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(attrs);

    assertSize(0, _trx->getRequest()->incorrectCorpIdVec());
    assertSize(0, _trx->getRequest()->corpIdVec());
    assertSize(1, _trx->getRequest()->accCodeVec());

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
  }

  void testSaveProcOptsInformation_MAC3()
  {
    mockPricingModelMapPointer();
    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("S11")] = 91;
    map.members[_pricingModelMap->SDBMHash("SM1")] = 148;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("S11", "ACCNTCODE01");
    attrs.add("SM1", "ACCNTCODE01");
    setValidCorpId();

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(attrs);
    _pricingModelMap->saveProcOptsInformation();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }

  void testStoreProcOptsInformation_MAC4()
  {
    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("SM1")] = 148;
    map.members[_pricingModelMap->SDBMHash("SM2")] = 149;
    map.members[_pricingModelMap->SDBMHash("SM3")] = 150;
    map.members[_pricingModelMap->SDBMHash("SM4")] = 151;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");
    attrs.add("SM2", "ACCNTCODE02");
    attrs.add("SM3", "ACCNTCODE03");
    attrs.add("SM4", "ACCNTCODE04");

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(attrs);

    assertSize(0, _trx->getRequest()->incorrectCorpIdVec());
    assertSize(0, _trx->getRequest()->corpIdVec());
    assertSize(4, _trx->getRequest()->accCodeVec());

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[1] == "ACCNTCODE02");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[2] == "ACCNTCODE03");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[3] == "ACCNTCODE04");
  }

  void testSaveProcOptsInformation_MAC4()
  {
    mockPricingModelMapPointer();
    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("SM1")] = 148;
    map.members[_pricingModelMap->SDBMHash("SM2")] = 149;
    map.members[_pricingModelMap->SDBMHash("SM3")] = 150;
    map.members[_pricingModelMap->SDBMHash("SM4")] = 151;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");
    attrs.add("SM2", "ACCNTCODE02");
    attrs.add("SM3", "ACCNTCODE03");
    attrs.add("SM4", "ACCNTCODE04");
    setValidCorpId();

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(attrs);
    _pricingModelMap->saveProcOptsInformation();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId());
  }

  void testStoreProcOptsInformation_SAT_Y()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SAT", "Y", 179));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isSpecificAgencyText());
  }

  void testStoreProcOptsInformation_SAT_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SAT", "T", 179));
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isSpecificAgencyText());
  }

  void testStoreProcOptsInformation_SAT_other()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SAT", "N", 179));
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isSpecificAgencyText());
  }

  void testStoreProcOptsInformation_SFT_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SFT", "F", 183));
    CPPUNIT_ASSERT(!_trx->getOptions()->isServiceFeesTemplateRequested());
  }

  void testStoreProcOptsInformation_SFT_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("SFT", "T", 183));
    CPPUNIT_ASSERT(_trx->getOptions()->isServiceFeesTemplateRequested());
  }

  void testStoreProcOptsInformation_Q0S_0()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("Q0S", "0", 4));
    CPPUNIT_ASSERT(!_trx->getOptions()->getRequestedNumberOfSolutions());
  }

  void testStoreProcOptsInformation_Q0S_1()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("Q0S", "1", 4));
    CPPUNIT_ASSERT(_trx->getOptions()->getRequestedNumberOfSolutions());
  }

  void testStoreProcOptsInformation_P1Z_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1Z", "F", 9));
    CPPUNIT_ASSERT(!_trx->getOptions()->isPrivateFares());
  }

  void testStoreProcOptsInformation_P1Z_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1Z", "T", 9));
    CPPUNIT_ASSERT(_trx->getOptions()->isPrivateFares());
  }

  void testStoreProcOptsInformation_P1Y_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1Y", "F", 10));
    CPPUNIT_ASSERT(!_trx->getOptions()->isPublishedFares());
  }

  void testStoreProcOptsInformation_P1Y_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1Y", "T", 10));
    CPPUNIT_ASSERT(_trx->getOptions()->isPublishedFares());
  }

  void testStoreProcOptsInformation_P20_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P20", "F", 11));
    CPPUNIT_ASSERT(!_trx->getOptions()->isXoFares());
  }

  void testStoreProcOptsInformation_P20_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P20", "T", 11));
    CPPUNIT_ASSERT(_trx->getOptions()->isXoFares());
  }

  void testStoreProcOptsInformation_P1W_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1W", "F", 12));
    CPPUNIT_ASSERT(!_trx->getOptions()->isOnlineFares());
  }

  void testStoreProcOptsInformation_P1W_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1W", "T", 12));
    CPPUNIT_ASSERT(_trx->getOptions()->isOnlineFares());
  }

  void testStoreProcOptsInformation_P21_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P21", "F", 13));
    CPPUNIT_ASSERT(!_trx->getOptions()->isIataFares());
  }

  void testStoreProcOptsInformation_P21_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P21", "T", 13));
    CPPUNIT_ASSERT(_trx->getOptions()->isIataFares());
  }

  void testStoreProcOptsInformation_P1V_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1V", "F", 14));
    CPPUNIT_ASSERT_EQUAL('F', _trx->getRequest()->priceNoAvailability());
  }

  void testStoreProcOptsInformation_P1V_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("P1V", "T", 14));
    CPPUNIT_ASSERT_EQUAL('T', _trx->getRequest()->priceNoAvailability());
  }

  void testStoreProcOptsInformation_AAL_T()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("AAL", "T", 187));
    CPPUNIT_ASSERT(_trx->getOptions()->isMatchAndNoMatchRequested());
  }

  void testStoreProcOptsInformation_AAL_F()
  {
    _pricingModelMap->storeProcOptsInformation(createAttribute("AAL", "F", 187));
    CPPUNIT_ASSERT(!_trx->getOptions()->isMatchAndNoMatchRequested());
  }

  void testShouldStoreOverridesReturnFalseForARTrxWithNoSecondaryExchange()
  {
    _trx->secondaryExcReqType().clear();
    CPPUNIT_ASSERT(!_pricingModelMap->shouldStoreOverrides());
  }

  void testShouldStoreOverridesReturnTrueForARTrxWithSecondaryExchange()
  {
    _trx->secondaryExcReqType() = TAG_10_EXCHANGE;
    CPPUNIT_ASSERT(_pricingModelMap->shouldStoreOverrides());
  }

  void testShouldStoreOverridesReturnTrueForPORTTrx()
  {
    ExchangePricingTrx portTrx;
    _pricingModelMap->_pricingTrx = &portTrx;
    CPPUNIT_ASSERT(_pricingModelMap->shouldStoreOverrides());
  }

  void testExchangeTypeNotCEReturnTrueForARTrxWithNoSecodaryExchange()
  {
    _trx->secondaryExcReqType().clear();
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(_trx);
    CPPUNIT_ASSERT(_pricingModelMap->exchangeTypeNotCE(baseExcTrx));
  }

  void testExchangeTypeNotCEReturnFalseForARTrxWithNoSecodaryExchange()
  {
    _trx->secondaryExcReqType() = TAG_10_EXCHANGE;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(_trx);
    CPPUNIT_ASSERT(!_pricingModelMap->exchangeTypeNotCE(baseExcTrx));
  }

  void testExchangeTypeNotCEReturnFalseForPORTTrxCEType()
  {
    ExchangePricingTrx portTrx;
    portTrx.reqType() = TAG_10_EXCHANGE;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(&portTrx);
    CPPUNIT_ASSERT(!_pricingModelMap->exchangeTypeNotCE(baseExcTrx));
  }

  void testExchangeTypeNotCEReturnFalseForPORTTrxNotCEType()
  {
    ExchangePricingTrx portTrx;
    portTrx.reqType() = PARTIAL_EXCHANGE;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(&portTrx);
    CPPUNIT_ASSERT(_pricingModelMap->exchangeTypeNotCE(baseExcTrx));
  }

  void testSaveStopOverOverridesCopiedToExchangeOverrides()
  {
    StopoverOverride sto1, sto2;
    _pricingModelMap->_stopoverOverride += &sto1, &sto2;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(_trx);
    _pricingModelMap->saveStopoverOverride(*baseExcTrx);
    CPPUNIT_ASSERT_EQUAL(&sto1, baseExcTrx->exchangeOverrides().stopoverOverride()[0]);
    CPPUNIT_ASSERT_EQUAL(&sto2, baseExcTrx->exchangeOverrides().stopoverOverride()[1]);
  }

  void testSavePlusUpOverridesCopiedToExchangeOverrides()
  {
    PlusUpOverride pup1, pup2;
    _pricingModelMap->_plusUpOverride += &pup1, &pup2;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(_trx);
    _pricingModelMap->savePlusUpOverride(*baseExcTrx);
    CPPUNIT_ASSERT_EQUAL(&pup1, baseExcTrx->exchangeOverrides().plusUpOverride()[0]);
    CPPUNIT_ASSERT_EQUAL(&pup2, baseExcTrx->exchangeOverrides().plusUpOverride()[1]);
  }

  void testSaveMileageDataOverridesCopiedToExchangeOverrides()
  {
    MileageTypeData mil1, mil2;
    _pricingModelMap->_mileDataOverride += &mil1, &mil2;
    BaseExchangeTrx* baseExcTrx = dynamic_cast<BaseExchangeTrx*>(_trx);
    _pricingModelMap->saveMileDataOverride(*baseExcTrx);
    CPPUNIT_ASSERT_EQUAL(&mil1, baseExcTrx->exchangeOverrides().mileageTypeData()[0]);
    CPPUNIT_ASSERT_EQUAL(&mil2, baseExcTrx->exchangeOverrides().mileageTypeData()[1]);
  }

  void testSaveExchangeOverridesDontSaveWhenFareCompNumZero()
  {
    populateExchangeOverridesInPricingModelMapObject();
    _pricingModelMap->_fareCompNum = 0;
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT(_trx->exchangeOverrides().dummyFCSegs().empty());
  }

  void testSaveExchangeOverridesDontSaveWhenFareBasisEmpty()
  {
    populateExchangeOverridesInPricingModelMapObject();
    _pricingModelMap->_fareBasisCode.clear();
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT(_trx->exchangeOverrides().dummyFCSegs().empty());
  }

  void testSaveExchangeOverridesDontSaveWhenSecondaryExchangeEmpty()
  {
    populateExchangeOverridesInPricingModelMapObject();
    _trx->secondaryExcReqType().clear();
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT(_trx->exchangeOverrides().dummyFCSegs().empty());
  }

  void testSaveExchangeOverridesDontSaveWhenCurrentTvlSegZero()
  {
    populateExchangeOverridesInPricingModelMapObject();
    _pricingModelMap->_currentTvlSeg = 0;
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT(_trx->exchangeOverrides().dummyFCSegs().empty());
  }

  void testSaveExchangeOverridesDontSaveForExchangeItin()
  {
    populateExchangeOverridesInPricingModelMapObject();
    _pricingModelMap->_isExchangeItin = true;
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT(_trx->exchangeOverrides().dummyFCSegs().empty());
  }

  void testSaveExchangeOverridesSavedForNewItin()
  {
    populateExchangeOverridesInPricingModelMapObject();
    LocCode expectedMileageSurCity = _pricingModelMap->_mileageSurchargeCity;
    LocCode expectedMileageTktCity = _pricingModelMap->_mileageTktCity;
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_fareCompNum,
                         _trx->exchangeOverrides().dummyFCSegs()[_pricingModelMap->_currentTvlSeg]);
    CPPUNIT_ASSERT_EQUAL('T', _pricingModelMap->_currentTvlSeg->forcedNoFareBrk());
    CPPUNIT_ASSERT_EQUAL(
        _pricingModelMap->_forcedSideTrip,
        _trx->exchangeOverrides().forcedSideTrip()[_pricingModelMap->_currentTvlSeg]);
    CPPUNIT_ASSERT_EQUAL(
        (int16_t)(_pricingModelMap->_mileageSurchargePctg),
        _trx->exchangeOverrides().dummyFareMiles()[_pricingModelMap->_currentTvlSeg]);
    CPPUNIT_ASSERT_EQUAL(
        expectedMileageSurCity,
        _trx->exchangeOverrides().dummyFareMileCity()[_pricingModelMap->_currentTvlSeg]);
    CPPUNIT_ASSERT_EQUAL(
        expectedMileageTktCity,
        _trx->exchangeOverrides().dummyFareMileTktCity()[_pricingModelMap->_currentTvlSeg]);
  }

  void testSaveExchangeOverridesSavedSurchargeOverridesForNewItin()
  {
    populateExchangeOverridesInPricingModelMapObject();
    SurchargeOverride sur1, sur2;
    _pricingModelMap->_surchargeOverride += &sur1, &sur2;
    _pricingModelMap->saveExchangeOverrides();
    CPPUNIT_ASSERT_EQUAL(&sur1, _trx->exchangeOverrides().surchargeOverride()[0]);
    CPPUNIT_ASSERT_EQUAL(&sur2, _trx->exchangeOverrides().surchargeOverride()[1]);
  }

  void testCheckSideTripThrowWhenStartWithoutPrevous()
  {
    _pricingModelMap->_sideTripStart = true;
    _pricingModelMap->_insideSideTrip = true;
    _pricingModelMap->_sideTripNumber = 1;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkSideTrip(), ErrorResponseException);
  }

  void testCheckSideTripThrowWhenStartAndEndInOneSegment()
  {
    _pricingModelMap->_sideTripStart = true;
    _pricingModelMap->_sideTripEnd = true;
    _pricingModelMap->_sideTripNumber = 1;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkSideTrip(), ErrorResponseException);
  }

  void testCheckSideTripThrowWhenNumberExistWithoutStart()
  {
    _pricingModelMap->_sideTripStart = true;
    _pricingModelMap->_sideTripNumber = 0;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkSideTrip(), ErrorResponseException);
  }

  void testCheckSideTripThrowWhenStartWithoutNumber()
  {
    _pricingModelMap->_sideTripStart = false;
    _pricingModelMap->_sideTripNumber = 1;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkSideTrip(), ErrorResponseException);
  }

  void testCheckSideTripPassWhenValidBegin()
  {
    _pricingModelMap->_sideTripStart = true;
    _pricingModelMap->_sideTripEnd = false;
    _pricingModelMap->_sideTripNumber = 1;
    _pricingModelMap->_insideSideTrip = false;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkSideTrip());
  }

  void testCheckSideTripPassWhenInside()
  {
    _pricingModelMap->_sideTripStart = false;
    _pricingModelMap->_sideTripEnd = false;
    _pricingModelMap->_sideTripNumber = 1;
    _pricingModelMap->_insideSideTrip = true;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkSideTrip());
  }

  void testCheckSideTripPassWhenValidEnd()
  {
    _pricingModelMap->_sideTripStart = false;
    _pricingModelMap->_sideTripEnd = true;
    _pricingModelMap->_sideTripNumber = 1;
    _pricingModelMap->_insideSideTrip = true;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkSideTrip());
  }

  void testCheckSideTripUnsetInsideFlagWhenEnd()
  {
    _pricingModelMap->_sideTripStart = false;
    _pricingModelMap->_sideTripEnd = true;
    _pricingModelMap->_sideTripNumber = 1;
    _pricingModelMap->_insideSideTrip = true;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkSideTrip());
    CPPUNIT_ASSERT(!_pricingModelMap->_insideSideTrip);
  }

  void testStoreSegmentInformationThrowWhenInvalidSideTripDataAndCat31()
  {
    MockAttributes attr;
    PricingModelMap::Mapping mapp = {0, 0};
    setMapAndAttributesWithInvalidSideTripData(mapp, attr);
    _pricingModelMap->_isExchangeItin = true;
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeSegmentInformation(attr),
                         ErrorResponseException);
  }

  void testStoreSegmentInformationThrowWhenInvalidSideTripDataAndCat33()
  {
    MockAttributes attr;
    PricingModelMap::Mapping mapp = {0, 0};
    setMapAndAttributesWithInvalidSideTripData(mapp, attr);
    _pricingModelMap->_isExchangeItin = true;
    _trx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeSegmentInformation(attr),
                         ErrorResponseException);
  }

  void testStoreSegmentInformationDontThrowWhenInvalidSideTripDataAndPortExc()
  {
    MockAttributes attr;
    PricingModelMap::Mapping mapp = {0, 0};
    setMapAndAttributesWithInvalidSideTripData(mapp, attr);
    _pricingModelMap->_isExchangeItin = true;
    _trx->setExcTrxType(PricingTrx::PORT_EXC_TRX);
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storeSegmentInformation(attr));
  }

  void testStoreSegmentInformationDontThrowWhenInvalidSideTripDataAndNotExc()
  {
    MockAttributes attr;
    PricingModelMap::Mapping mapp = {0, 0};
    setMapAndAttributesWithInvalidSideTripData(mapp, attr);
    _pricingModelMap->_isExchangeItin = false;
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storeSegmentInformation(attr));
  }

  void testStoreBillingInformationSavesS0R()
  {
    MockAttributes attr;
    PricingModelMap::Mapping mapp = {0, 0};
    mapp.members[_pricingModelMap->SDBMHash("S0R")] = 13;
    _pricingModelMap->_currentMapEntry = (void*)&mapp;
    attr.add("S0R", PSS_PO_ATSE_PATH);

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeBillingInformation(attr);
    CPPUNIT_ASSERT(_trx->billing()->requestPath() == PSS_PO_ATSE_PATH);
  }

  void testSetRestrictCmdPricing_setTrue()
  {
    _pricingModelMap->setRestrictCmdPricing("ABC");
    CPPUNIT_ASSERT(_pricingModelMap->_restrictCmdPricing);
  }

  void testSetRestrictCmdPricing_setFalse()
  {
    _pricingModelMap->setRestrictCmdPricing("");
    CPPUNIT_ASSERT(!_pricingModelMap->_restrictCmdPricing);
  }

  void testSetRestrictCmdPricing_exchangeItin_setFalse()
  {
    _pricingModelMap->_isExchangeItin = true;
    _pricingModelMap->setRestrictCmdPricing("ABC");
    CPPUNIT_ASSERT(!_pricingModelMap->_restrictCmdPricing);
  }

  void testSetEPRKeywordFlag_MUL375()
  {
    _pricingModelMap->setEPRKeywordFlag("MUL375");
    CPPUNIT_ASSERT(_pricingModelMap->_isEPRKeyword);
  }

  void testSetEPRKeywordFlag_TMPCRS()
  {
    _pricingModelMap->setEPRKeywordFlag("TMPCRS");
    CPPUNIT_ASSERT(_pricingModelMap->_isEPRKeyword);
  }

  void testSetEPRKeywordFlag_CRSAGT()
  {
    _pricingModelMap->setEPRKeywordFlag("CRSAGT");
    CPPUNIT_ASSERT(_pricingModelMap->_isEPRKeyword);
  }

  void testSetEPRKeywordFlag_Other()
  {
    _pricingModelMap->setEPRKeywordFlag("ABCDEF");
    CPPUNIT_ASSERT(!_pricingModelMap->_isEPRKeyword);
  }

  void testSetEPRKeywordFlag_exchangeItin()
  {
    _pricingModelMap->_isExchangeItin = true;
    _pricingModelMap->setEPRKeywordFlag("CRSAGT");
    CPPUNIT_ASSERT(!_pricingModelMap->_isEPRKeyword);
  }

  void testRestrictCmdPricing()
  {
    _pricingModelMap->setRestrictCmdPricing("ABC");
    _pricingModelMap->_isWsUser = true;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->restrictCmdPricing(), ErrorResponseException);
  }
  void testgetRefundTypes()
  {
    std::string prmReqType = "AR";
    std::string secReqType = "FE";
    char primaryProcessType = 'A';
    char secondaryProcessType = 'A';
    std::pair<std::string, std::string> refundTypes = make_pair(prmReqType, secReqType);

    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("S96")] = 1;
    map.members[_pricingModelMap->SDBMHash("SA8")] = 2;
    map.members[_pricingModelMap->SDBMHash("N25")] = 3;
    map.members[_pricingModelMap->SDBMHash("N26")] = 4;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("S96", "AR");
    attrs.add("SA8", "FE");
    attrs.add("N25", "A");
    attrs.add("N26", "A");

    _pricingModelMap->_pricingTrx = _trx;
    std::pair<std::string, std::string> rTypes = _pricingModelMap->getRefundTypes(attrs);

    CPPUNIT_ASSERT_EQUAL(primaryProcessType, _pricingModelMap->_primaryProcessType);
    CPPUNIT_ASSERT_EQUAL(secondaryProcessType, _pricingModelMap->_secondaryProcessType);
  }

  PricingTrx* createPricingTrxWithPaxTypes()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PaxType* ptc = _memHandle.create<PaxType>();
    ptc->paxType() = "ADT";
    trx->paxType().push_back(ptc);
    ptc = _memHandle.create<PaxType>();
    ptc->paxType() = "CNN";
    trx->paxType().push_back(ptc);
    ptc = _memHandle.create<PaxType>();
    ptc->paxType() = "INF";
    trx->paxType().push_back(ptc);
    _pricingModelMap->_pricingTrx = trx;
    return trx;
  }
  void createFFData(PaxTypeCode ptc)
  {
    PaxType::FreqFlyerTierWithCarrier* ffd = _memHandle.create<PaxType::FreqFlyerTierWithCarrier>();
    _pricingModelMap->_ffData.insert(
        std::pair<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>(ptc, ffd));
  }
  struct matchptc
  {
    PaxTypeCode _ptc;
    matchptc(PaxTypeCode ptc) : _ptc(ptc) {};
    bool operator()(PaxType* p) { return p->paxType() == _ptc; }
  };
  int paxTypeFFStatusSize(PricingTrx* trx, PaxTypeCode ptc)
  {
    std::vector<PaxType*>::iterator p =
        std::find_if(trx->paxType().begin(), trx->paxType().end(), matchptc(ptc));
    return (p == trx->paxType().end()) ? 0 : (int)(*p)->freqFlyerTierWithCarrier().size();
  }
  void setFreqFlyerStatus_empty()
  {
    PricingTrx* trx = createPricingTrxWithPaxTypes();
    _pricingModelMap->setFreqFlyerStatus();
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "ADT"));
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "CNN"));
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "INF"));
  }
  void setFreqFlyerStatus_ADT()
  {
    PricingTrx* trx = createPricingTrxWithPaxTypes();
    createFFData("ADT");
    _pricingModelMap->setFreqFlyerStatus();
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "ADT"));
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "CNN"));
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "INF"));
  }
  void setFreqFlyerStatus_ADT_CNN()
  {
    PricingTrx* trx = createPricingTrxWithPaxTypes();
    createFFData("ADT");
    createFFData("CNN");
    _pricingModelMap->setFreqFlyerStatus();
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "ADT"));
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "CNN"));
    CPPUNIT_ASSERT_EQUAL(0, paxTypeFFStatusSize(trx, "INF"));
  }
  void setFreqFlyerStatus_ADT_UNKNOW()
  {
    PricingTrx* trx = createPricingTrxWithPaxTypes();
    createFFData("ADT");
    createFFData("");
    _pricingModelMap->setFreqFlyerStatus();
    CPPUNIT_ASSERT_EQUAL(2, paxTypeFFStatusSize(trx, "ADT"));
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "CNN"));
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "INF"));
  }
  void setFreqFlyerStatus_UNKNOW_INF()
  {
    PricingTrx* trx = createPricingTrxWithPaxTypes();
    createFFData("");
    createFFData("INF");
    _pricingModelMap->setFreqFlyerStatus();
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "ADT"));
    CPPUNIT_ASSERT_EQUAL(1, paxTypeFFStatusSize(trx, "CNN"));
    CPPUNIT_ASSERT_EQUAL(2, paxTypeFFStatusSize(trx, "INF"));
  }

  void testCheckCat35NetSellOptionWhenTicketing()
  {
    PricingOptions opt;
    setOptionsForTkt(opt);
    _pricingModelMap->checkCat35NetSellOption(_trx);
    CPPUNIT_ASSERT(!opt.isCat35Net());
    CPPUNIT_ASSERT(opt.cat35NetSell());
  }
  void testCheckCat35NetSellOptionWhenNotTicketing()
  {
    PricingOptions opt;
    setOptionsForNonTkt(&opt);
    _pricingModelMap->checkCat35NetSellOption(_trx);
    CPPUNIT_ASSERT(opt.isCat35Net());
    CPPUNIT_ASSERT(!opt.cat35NetSell());
  }
  void testTryExtractVersion()
  {
    XMLCh attribName[] = { 'V', 'e', 'R', 's', 'I', 'o', 'N', 0 };
    XMLCh attribValue[] = { '3', '.', '5', '.', '6', 0 };

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->tryExtractVersion(XMLChString(attribName), XMLChString(attribValue));

    CPPUNIT_ASSERT_EQUAL(3, (int)_pricingModelMap->_pricingTrx->getRequest()->majorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL(5, (int)_pricingModelMap->_pricingTrx->getRequest()->minorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL(6,
                         (int)_pricingModelMap->_pricingTrx->getRequest()->revisionSchemaVersion());
  }

  void testSwsNoAltPricingResp()
  {
    AltPricingTrx altPricingTrx;
    Billing billing;
    MockAttributes attr;
    altPricingTrx.setRequest(_request);
    altPricingTrx.altTrxType() = AltPricingTrx::WP;
    altPricingTrx.billing() = &billing;
    altPricingTrx.billing()->requestPath() = SWS_PO_ATSE_PATH;
    _pricingModelMap->_pricingTrx = &altPricingTrx;
    _pricingModelMap->storeProcOptsInformation(attr);
    CPPUNIT_ASSERT(altPricingTrx.getRequest()->turnOffNoMatch());
  }

  void testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenNoExc()
  {
    DateTime dt(2014, 1, 12, 12, 0, 0);
    _pricingModelMap->_isRexTrx = true;
    _pricingModelMap->_inPROCount = 0;
    _pricingModelMap->_processExchangePricingTrx = false;

    PricingOptions opt;
    _pricingModelMap->_options = &opt;
    _pricingModelMap->_options->ticketTimeOverride() = 100;

    ExchangePricingTrx excTrx;
    _pricingModelMap->_pricingTrx = &excTrx;
    excTrx.reqType() = AGENT_PRICING_MASK;
    PricingRequest request;
    excTrx.setRequest(&request);
    excTrx.currentTicketingDT() = dt;
    request.ticketingDT() = dt;
    _pricingModelMap->_originalTicketingDT = dt;
    excTrx.purchaseDT() = dt;
    _pricingModelMap->checkAndAdjustPurchaseDTwithTimeForExchange();
    CPPUNIT_ASSERT_EQUAL(dt, excTrx.purchaseDT());
  }

  void testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenTime_0()
  {
    DateTime dt(2014, 1, 12, 12, 0, 0);
    _pricingModelMap->_isRexTrx = true;
    _pricingModelMap->_inPROCount = 0;
    _pricingModelMap->_processExchangePricingTrx = true;

    PricingOptions opt;
    _pricingModelMap->_options = &opt;
    _pricingModelMap->_options->ticketTimeOverride() = 0;

    ExchangePricingTrx excTrx;
    _pricingModelMap->_pricingTrx = &excTrx;
    excTrx.reqType() = AGENT_PRICING_MASK;
    PricingRequest request;
    excTrx.setRequest(&request);
    excTrx.currentTicketingDT() = dt;
    request.ticketingDT() = dt;
    _pricingModelMap->_originalTicketingDT = dt;
    excTrx.purchaseDT() = dt;
    _pricingModelMap->checkAndAdjustPurchaseDTwithTimeForExchange();
    CPPUNIT_ASSERT_EQUAL(dt, excTrx.purchaseDT());
  }

  void testCheckAndAdjustPurchaseDTwithTimeForExchangeNoAdjustWhenTag10()
  {
    DateTime dt(2014, 1, 12, 12, 0, 0);
    _pricingModelMap->_isRexTrx = true;
    _pricingModelMap->_inPROCount = 0;
    _pricingModelMap->_processExchangePricingTrx = true;

    PricingOptions opt;
    _pricingModelMap->_options = &opt;
    _pricingModelMap->_options->ticketTimeOverride() = 100;

    ExchangePricingTrx excTrx;
    _pricingModelMap->_pricingTrx = &excTrx;
    excTrx.reqType() = TAG_10_EXCHANGE;
    PricingRequest request;
    excTrx.setRequest(&request);
    excTrx.currentTicketingDT() = dt;
    request.ticketingDT() = dt;
    _pricingModelMap->_originalTicketingDT = dt;
    excTrx.purchaseDT() = dt;
    _pricingModelMap->checkAndAdjustPurchaseDTwithTimeForExchange();
    CPPUNIT_ASSERT_EQUAL(dt, excTrx.purchaseDT());
  }

  void testCheckAndAdjustPurchaseDTwithTimeForExchangeAdjustWhenAM()
  {
    DateTime dt(2014, 1, 12, 12, 0, 0);
    _pricingModelMap->_isRexTrx = true;
    _pricingModelMap->_inPROCount = 0;
    _pricingModelMap->_processExchangePricingTrx = true;

    PricingOptions opt;
    _pricingModelMap->_options = &opt;
    _pricingModelMap->_options->ticketTimeOverride() = 100;

    ExchangePricingTrx excTrx;
    _pricingModelMap->_pricingTrx = &excTrx;
    excTrx.reqType() = TAG_10_EXCHANGE;
    PricingRequest request;
    excTrx.setRequest(&request);
    excTrx.currentTicketingDT() = dt;
    request.ticketingDT() = dt;
    _pricingModelMap->_originalTicketingDT = dt;
    excTrx.purchaseDT() = dt;
    _pricingModelMap->checkAndAdjustPurchaseDTwithTimeForExchange();
    CPPUNIT_ASSERT_EQUAL(dt, excTrx.purchaseDT());

    excTrx.reqType() = AGENT_PRICING_MASK;
    _pricingModelMap->checkAndAdjustPurchaseDTwithTimeForExchange();
    DateTime pDT = dt;
    pDT = pDT + tse::Hours(opt.ticketTimeOverride() / 60) +
          tse::Minutes(opt.ticketTimeOverride() % 60) + tse::Seconds(0);
    pDT.setHistoricalIncludesTime();

    CPPUNIT_ASSERT_EQUAL(pDT, excTrx.purchaseDT());
  }

  void testBrandCodePass()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    addMembersToMap(mapp);
    _pricingModelMap->_currentMapEntry = (void*)&mapp;
    MockAttributes attr;
    attr.add("S37", "ATP");
    attr.add("B09", "AA");
    attr.add("S89", "100");
    attr.add("S90", "5000");
    attr.add("SB2", "Apple");
    _pricingModelMap->storeSegmentInformation(attr);
    CPPUNIT_ASSERT_EQUAL(BrandCode("Apple"), _pricingModelMap->_brandCode);
  }

  void testLegId()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    mapp.members[_pricingModelMap->SDBMHash("Q0C")] = 1;
    mapp.members[_pricingModelMap->SDBMHash("Q0L")] = 45;
    _pricingModelMap->_currentMapEntry = (void*)&mapp;
    MockAttributes attr;
    attr.add("Q0C", "1");
    attr.add("Q0L", "2");
    _pricingModelMap->storeSegmentInformation(attr);
    CPPUNIT_ASSERT_EQUAL(1, int(_pricingModelMap->_legId));
  }

  void testInvalidLegId()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    mapp.members[_pricingModelMap->SDBMHash("Q0C")] = 1;
    mapp.members[_pricingModelMap->SDBMHash("Q0L")] = 45;
    _pricingModelMap->_currentMapEntry = (void*)&mapp;
    MockAttributes attr;
    attr.add("Q0C", "1");
    attr.add("Q0L", "XYZ");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeSegmentInformation(attr), NonFatalErrorResponseException);
  }

  void testLegIdAndUnknownAttr()
  {
    PricingModelMap::Mapping mapp = {0, 0};
    mapp.members[_pricingModelMap->SDBMHash("Q0C")] = 1;
    mapp.members[_pricingModelMap->SDBMHash("Q0L")] = 45;
    _pricingModelMap->_currentMapEntry = (void*)&mapp;
    MockAttributes attr;
    attr.add("Q0C", "1");
    attr.add("Q0L", "1");
    attr.add("B03", "VA");
    _pricingModelMap->storeSegmentInformation(attr);
    CPPUNIT_ASSERT_EQUAL(0, int(_pricingModelMap->_legId));
  }

  void testStoreProcOptsInformationPBB()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("PBB")] = 177;
    _pricingModelMap->_currentMapEntry = (void*)&map;
    MockAttributes attrs;
    attrs.add("PBB", "T");
    _pricingModelMap->_pricingTrx = trx;
    _pricingModelMap->storeProcOptsInformation(attrs);
    CPPUNIT_ASSERT(trx->isPbbRequest() == PBB_RQ_PROCESS_BRANDS);
  }

  void testStoreProcOptsInformationPBBWpaRequest()
  {
    PricingTrx* trx = _memHandle.create<AltPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("PBB")] = 177;
    _pricingModelMap->_currentMapEntry = (void*)&map;
    MockAttributes attrsWithPbb;
    attrsWithPbb.add("PBB", "T");
    MockAttributes attrsWithoutPbb;
    attrsWithoutPbb.add("PBB", "N");
    _pricingModelMap->_pricingTrx = trx;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeProcOptsInformation(attrsWithPbb), tse::ErrorResponseException);
    _pricingModelMap->storeProcOptsInformation(attrsWithoutPbb);
    CPPUNIT_ASSERT(trx->isPbbRequest() == NOT_PBB_RQ);
  }

  void testStoreProcOptsInformationPBBWqRequest()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("PBB")] = 177;
    _pricingModelMap->_currentMapEntry = (void*)&map;
    MockAttributes attrsWithPbb;
    attrsWithPbb.add("PBB", "T");
    MockAttributes attrsWithoutPbb;
    attrsWithoutPbb.add("PBB", "N");
    _pricingModelMap->_pricingTrx = trx;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeProcOptsInformation(attrsWithPbb), tse::ErrorResponseException);
    _pricingModelMap->storeProcOptsInformation(attrsWithoutPbb);
    CPPUNIT_ASSERT(trx->isPbbRequest() == NOT_PBB_RQ);
  }

  void testRestrictExcludeFareFocusRule_XFF_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setExcludeFareFocusRule(true);
    opt.eprKeywords().insert("FFOCUS");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->restrictExcludeFareFocusRule());
  }

  void testRestrictExcludeFareFocusRule_NO_XFF_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setExcludeFareFocusRule(false);
    opt.eprKeywords().insert("FFOCUS");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->restrictExcludeFareFocusRule());
  }

  void testRestrictExcludeFareFocusRule_XFF_NOEPR_Error()
  {
    PricingOptions opt;
    opt.setExcludeFareFocusRule(true);
    opt.eprKeywords().insert("FAREFOWRONG");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->restrictExcludeFareFocusRule(), ErrorResponseException);
  }

  void testStoreProcessingOptions_PDO_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setPDOForFRRule(true);
    opt.eprKeywords().insert("ORGFQD");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDOorXRS());
  }

  void testStoreProcessingOptions_NO_PDO_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setPDOForFRRule(false);
    opt.eprKeywords().insert("ORGFQD");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDOorXRS());
  }
  void testStoreProcessingOptions_PDO_NOEPR_Error()
  {
    PricingOptions opt;
    opt.setPDOForFRRule(true);
    opt.eprKeywords().insert("WRONGEPR");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkEprForPDOorXRS(), ErrorResponseException);
  }
  void testStoreProcessingOptions_PDR_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setPDRForFRRule(true);
    opt.eprKeywords().insert("AGYRET");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDR());
  }

  void testStoreProcessingOptions_NO_PDR_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setPDRForFRRule(false);
    opt.eprKeywords().insert("AGYRET");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDR());
  }

  void testStoreProcessingOptions_PDR_NOEPR_Error()
  {
    PricingOptions opt;
    opt.setPDRForFRRule(true);
    opt.eprKeywords().insert("WRONGEPR");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkEprForPDR(), ErrorResponseException);
  }

  void testStoreProcessingOptions_XRS_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setXRSForFRRule(true);
    opt.eprKeywords().insert("ORGFQD");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDOorXRS());
  }

  void testStoreProcessingOptions_NO_XRS_EPR_No_Error()
  {
    PricingOptions opt;
    opt.setXRSForFRRule(false);
    opt.eprKeywords().insert("ORGFQD");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkEprForPDOorXRS());
  }

  void testStoreProcessingOptions_XRS_NOEPR_Error()
  {
    PricingOptions opt;
    opt.setXRSForFRRule(true);
    opt.eprKeywords().insert("WRONGEPR");
    _pricingModelMap->_options = &opt;
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkEprForPDOorXRS(), ErrorResponseException);
  }

  void testCheckRtwSegment_FailFareBrk()
  {
    _pricingModelMap->_options = _memHandle.create<PricingOptions>();
    _pricingModelMap->_options->setRtw(true);
    AirSeg as;
    as.forcedFareBrk() = 'T';
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkRtwSegment(as),
                         tse::ErrorResponseException);
  }

  void testCheckRtwSegment_FailSideTrip()
  {
    _pricingModelMap->_options = _memHandle.create<PricingOptions>();
    _pricingModelMap->_options->setRtw(true);
    AirSeg as;
    as.forcedSideTrip() = 'T';
    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkRtwSegment(as),
                         tse::ErrorResponseException);
  }

  void testCheckRtwSegment_Pass()
  {
    _pricingModelMap->_options = _memHandle.create<PricingOptions>();
    _pricingModelMap->_options->setRtw(true);
    AirSeg as;
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkRtwSegment(as));
  }

  void testStorePassengerInformation_BasicFail()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePassengerInformation(*attr),
                         tse::ErrorResponseException);

    CPPUNIT_ASSERT(!_pricingModelMap->_mpo);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter);

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_BasicPass()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(!_pricingModelMap->_mpo);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter);

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass1()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "I");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    CPPUNIT_ASSERT(_pricingModelMap->_mpo);
    CPPUNIT_ASSERT_EQUAL(smp::INFO, *_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter);

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass2()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("MPI", "A");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("MPI", "A");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(_pricingModelMap->_mpo);
    CPPUNIT_ASSERT_EQUAL(smp::OR, *_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpChangeFilter->_departure);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter->_query);
    CPPUNIT_ASSERT_EQUAL(smp::CHANGEABLE,
                         _pricingModelMap->_mpChangeFilter->_query.get());
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter->_maxFee);

    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpRefundFilter->_departure);
    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter->_query);
    CPPUNIT_ASSERT_EQUAL(smp::CHANGEABLE,
                         _pricingModelMap->_mpRefundFilter->_query.get());
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter->_maxFee);

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass3()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("MPA", "100.0");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("MPA", "100.0");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(_pricingModelMap->_mpo);
    CPPUNIT_ASSERT_EQUAL(smp::OR, *_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpChangeFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(100.0, "USD"), _pricingModelMap->_mpChangeFilter->_maxFee.get());

    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpRefundFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(100.0, "USD"), _pricingModelMap->_mpRefundFilter->_maxFee.get());

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass4()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("ABD", "A");
    attr->add("MPI", "N");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("ABD", "B");
    attr->add("MPA", "100.0");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(!_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT_EQUAL(smp::AFTER,
                         _pricingModelMap->_mpChangeFilter->_departure);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter->_query);
    CPPUNIT_ASSERT_EQUAL(smp::NONCHANGEABLE,
                         _pricingModelMap->_mpChangeFilter->_query.get());
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter->_maxFee);

    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BEFORE,
                         _pricingModelMap->_mpRefundFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(100.0, "USD"), _pricingModelMap->_mpRefundFilter->_maxFee.get());

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass5()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    _pricingModelMap->createAgent()->currencyCodeAgent() = CurrencyCode("EUR");

    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("MPA", "100.0");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("MPA", "150.0");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(_pricingModelMap->_mpo);
    CPPUNIT_ASSERT_EQUAL(smp::OR, *_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpChangeFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(100.0, CurrencyCode("EUR")), _pricingModelMap->_mpChangeFilter->_maxFee.get());

    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpRefundFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpRefundFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(150.0, CurrencyCode("EUR")), _pricingModelMap->_mpRefundFilter->_maxFee.get());

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyPass6()
  {
    PricingTrx::ExcTrxType originalType = _trx->excTrxType();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    _pricingModelMap->createAgent()->currencyCodeAgent() = "USD";

    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "1");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("MPA", "10");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePenaltyInformation(*attr));

    CPPUNIT_ASSERT(!_pricingModelMap->_mpo);

    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH,
                         _pricingModelMap->_mpChangeFilter->_departure);
    CPPUNIT_ASSERT(!_pricingModelMap->_mpChangeFilter->_query);
    CPPUNIT_ASSERT(_pricingModelMap->_mpChangeFilter->_maxFee);
    CPPUNIT_ASSERT_EQUAL(Money(10.0, "USD"), _pricingModelMap->_mpChangeFilter->_maxFee.get());

    CPPUNIT_ASSERT(!_pricingModelMap->_mpRefundFilter);

    _trx->setExcTrxType(originalType);
  }

  void testStorePassengerInformation_MaxPenaltyFail1()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("ABD", "");
    attr->add("MPI", "N");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("ABD", "");
    attr->add("MPA", "100.0");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);
  }

  void testStorePassengerInformation_MaxPenaltyFail2()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "C");
    attr->add("ABD", "A");
    attr->add("MPI", "");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);
  }

  void testStorePassengerInformation_MaxPenaltyFail3()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "");
    attr->add("ABD", "B");
    attr->add("MPI", "N");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "");
    attr->add("ABD", "A");
    attr->add("MPA", "100.0");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);
  }

  void testStorePassengerInformation_MaxPenaltyFail4()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "O");
    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->storePassengerInformation(*attr));

    attr = setUpPenaltyInformationInPricingModelMap();
    attr->add("MPT", "R");
    attr->add("ABD", "A");
    attr->add("MPA", "");
    attr->add("MPC", "USD");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePenaltyInformation(*attr),
                         NonFatalErrorResponseException);
  }

  void testStorePassengerInformation_MaxPenaltyFail5()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    MockAttributes* attr = setUpPassengerInformationInPricingModelMap();
    attr->add("B70", "ADT");
    attr->add("Q0U", "01");
    attr->add("MPO", "");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePassengerInformation(*attr),
                         NonFatalErrorResponseException);
  }

  void teststorePassengerTypeFlightInformation_paxTypeNotExists()
  {
    createSFRTrxType();
    setUpPassengerTypeFlightInformationInPricingModelMap();

    MockAttributes attrs;

    attrs.add("B50", "JOWTX");
    attrs.add("Q6D", "1");
    attrs.add("B70", "ADT");
    attrs.add("C51", "1000");

    CPPUNIT_ASSERT_THROW(_pricingModelMap->storePassengerTypeFlightInformation(attrs),
                         ErrorResponseException);
  }

  void teststorePassengerTypeFlightInformation_paxTypeExists()
  {
    StructuredRuleTrx* trx = createSFRTrxType();

    auto* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = "ADT";
    trx->paxType().push_back(paxType);

    setUpPassengerTypeFlightInformationInPricingModelMap();

    MockAttributes attrs;

    attrs.add("B50", "JOWTX");
    attrs.add("Q6D", "1");
    attrs.add("B70", "ADT");
    attrs.add("C51", "1000");

    _pricingModelMap->storePassengerTypeFlightInformation(attrs);

    CPPUNIT_ASSERT(1 == _pricingModelMap->_fareCompNum);
    CPPUNIT_ASSERT(std::string("JOWTX") == _pricingModelMap->_fareBasisCode);
    CPPUNIT_ASSERT(paxType == _pricingModelMap->_paxType);
    CPPUNIT_ASSERT(std::string("1000") == _pricingModelMap->_fareCompAmountStr);
  }

  void testStoreProcOptsInformationBRA()
  {
    PricingTrx* trx = _memHandle.create<AltPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->setTrxType(PricingTrx::PRICING_TRX);
    trx->altTrxType() = PricingTrx::WP;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BRA")] = 186;
    _pricingModelMap->_currentMapEntry = (void*)&map;
    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsNoBra;
    _pricingModelMap->storeProcOptsInformation(attrsNoBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    attrsNoBra.add("BRA", "N");
    _pricingModelMap->storeProcOptsInformation(attrsNoBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    MockAttributes attrs;
    attrs.add("BRA", "T");
    _pricingModelMap->storeProcOptsInformation(attrs);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == true);


  }

  void testStoreProcOptsInformationBRAWpaRequest()
  {
    PricingTrx* trx = _memHandle.create<AltPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->setTrxType(PricingTrx::PRICING_TRX);
    trx->altTrxType() = PricingTrx::WPA;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BRA")] = 186;
    _pricingModelMap->_currentMapEntry = (void*)&map;
    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithoutBra;
    _pricingModelMap->storeProcOptsInformation(attrsWithoutBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    attrsWithoutBra.add("BRA", "N");
    _pricingModelMap->storeProcOptsInformation(attrsWithoutBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    MockAttributes attrsWithBra;
    attrsWithBra.add("BRA", "T");
    _pricingModelMap->storeProcOptsInformation(attrsWithBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);
  }

  void testStoreProcOptsInformationBRAWqRequest()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BRA")] = 186;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBra;
    attrsWithBra.add("BRA", "T");
    _pricingModelMap->storeProcOptsInformation(attrsWithBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    MockAttributes attrsWithoutBra;
    _pricingModelMap->storeProcOptsInformation(attrsWithoutBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);

    attrsWithoutBra.add("BRA", "N");
    _pricingModelMap->storeProcOptsInformation(attrsWithoutBra);
    CPPUNIT_ASSERT(trx->activationFlags().isSearchForBrandsPricing() == false);
  }

  void setupPricingModelMapCommon()
  {
    _pricingModelMap->_pricingTrx = _memHandle.create<PricingTrx>();
    _pricingModelMap->_pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    _pricingModelMap->_pricingTrx->ticketingDate() = DateTime(2051, 1, 1);

    _pricingModelMap->_itin = _memHandle.create<Itin>();
    _pricingModelMap->_pricingTrx->itin().push_back(_pricingModelMap->_itin);

    _pricingModelMap->_pricingTrx->billing()=_memHandle.create<Billing>();
    _pricingModelMap->_options = _memHandle.create<PricingOptions>();
  }

  void setupMappingFLI(PricingModelMap::Mapping& map)
  {
    map.members[_pricingModelMap->SDBMHash("Q0C")] = 1;
    map.members[_pricingModelMap->SDBMHash("N03")] = 2;
    map.members[_pricingModelMap->SDBMHash("B00")] = 3;
    map.members[_pricingModelMap->SDBMHash("Q0B")] = 4;
    map.members[_pricingModelMap->SDBMHash("D01")] = 5;
    map.members[_pricingModelMap->SDBMHash("D02")] = 6;
    map.members[_pricingModelMap->SDBMHash("D00")] = 7;
    map.members[_pricingModelMap->SDBMHash("D31")] = 8;
    map.members[_pricingModelMap->SDBMHash("D32")] = 9;
    map.members[_pricingModelMap->SDBMHash("D30")] = 10;
    map.members[_pricingModelMap->SDBMHash("B30")] = 11;
    map.members[_pricingModelMap->SDBMHash("BB0")] = 12;
    map.members[_pricingModelMap->SDBMHash("A01")] = 13;
    map.members[_pricingModelMap->SDBMHash("A02")] = 14;
    map.members[_pricingModelMap->SDBMHash("B01")] = 15;
    map.members[_pricingModelMap->SDBMHash("P2X")] = 16;
    map.members[_pricingModelMap->SDBMHash("BB2")] = 17;
    map.members[_pricingModelMap->SDBMHash("BB3")] = 18;
    map.members[_pricingModelMap->SDBMHash("A60")] = 19;
    map.members[_pricingModelMap->SDBMHash("B40")] = 20;
  }

  void setupMappingRFB(PricingModelMap::Mapping& map)
  {
    map.members[_pricingModelMap->SDBMHash("B50")] = 1;
    map.members[_pricingModelMap->SDBMHash("B70")] = 2;
    map.members[_pricingModelMap->SDBMHash("S37")] = 3;
    map.members[_pricingModelMap->SDBMHash("B09")] = 4;
    map.members[_pricingModelMap->SDBMHash("S89")] = 5;
    map.members[_pricingModelMap->SDBMHash("S90")] = 6;
    map.members[_pricingModelMap->SDBMHash("C51")] = 7;
  }

  void setupAttributesXRA(MockAttributes& attr)
  {
    attr.add("MID", "mid");
    attr.add("CID", "cid");
  }

  void setupAttributesSGI(MockAttributes& attr)
  {
    attr.add("S37", "ATP");
    attr.add("B09", "AA");
    attr.add("S89", "100");
    attr.add("S90", "5000");
  }

  void setupAttributesFLI(MockAttributes& attr)
  {
    attr.add("N03", "A");
  }

  void setupAttributesRFB(MockAttributes& attr, const std::string& fareBasisCode,
                          const std::string& paxType)
  {
    attr.add("B50", fareBasisCode.c_str());
    if (!paxType.empty())
    {
      attr.add("B70", paxType.c_str());
      attr.add("S37", "ATP");
      attr.add("B09", "AA");
      attr.add("S89", "T006");
      attr.add("S90", "IL76");
      attr.add("C51", "5.55");
    }
  }

  void saveFlightInformation()
  {
    if (_pricingModelMap->_currentTvlSeg)
    {
      // Set FareBasisCode for Q(FareBasis)-(specified FBC) with segment select
      if (!_pricingModelMap->_fareBasisCode.empty())
      {
        _pricingModelMap->_currentTvlSeg->fareBasisCode() = _pricingModelMap->_fareBasisCode;
        _pricingModelMap->_currentTvlSeg->fbcUsage() = _pricingModelMap->_fbcUsage;
      }

      if (_pricingModelMap->_itin)
        _pricingModelMap->_itin->travelSeg().push_back(_pricingModelMap->_currentTvlSeg);
    }
  }

  void testSaveXrayAttributes()
  {
    setUpXrayInPricingModelMap();

    MockAttributes attrs;
    setupAttributesXRA(attrs);

    _pricingModelMap->storeXrayInformation(attrs);
    _pricingModelMap->saveXrayInformation();

    CPPUNIT_ASSERT(std::string("mid") ==
        _pricingModelMap->_pricingTrx->getXrayJsonMessage()->getMid());
    CPPUNIT_ASSERT(std::string("cid") ==
        _pricingModelMap->_pricingTrx->getXrayJsonMessage()->getCid());
  }

  void testStoreRequestedFareBasisInformation()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping map = {0, 0};
    setupMappingRFB(map);
    _pricingModelMap->_currentMapEntry = (void*)&map;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    _pricingModelMap->_currentTvlSeg = airSeg;

    MockAttributes attrs;
    setupAttributesRFB(attrs, "FAIRBASE1", "ADT");

    _pricingModelMap->storeRequestedFareBasisInformation(attrs);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->saveRequestedFareBasisInSegmentInformation();

    CPPUNIT_ASSERT_EQUAL((size_t)1, airSeg->requestedFareBasis().size());
    CPPUNIT_ASSERT_EQUAL((FareBasisCode)"FAIRBASE1", airSeg->requestedFareBasis().front().fareBasisCode);
    CPPUNIT_ASSERT_EQUAL((PaxTypeCode)"ADT", airSeg->requestedFareBasis().front().passengerCode);
    CPPUNIT_ASSERT_EQUAL((VendorCode)"ATP", airSeg->requestedFareBasis().front().vendor);
    CPPUNIT_ASSERT_EQUAL((CarrierCode)"AA", airSeg->requestedFareBasis().front().carrier);
    CPPUNIT_ASSERT_EQUAL((TariffCode)"T006", airSeg->requestedFareBasis().front().tariff);
    CPPUNIT_ASSERT_EQUAL((RuleNumber)"IL76", airSeg->requestedFareBasis().front().rule);
    CPPUNIT_ASSERT_EQUAL(5.55, airSeg->requestedFareBasis().front().amount);
  }

  void testStoreRequestedFareBasisInformationNotAllOptional()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping map = {0, 0};
    setupMappingRFB(map);
    _pricingModelMap->_currentMapEntry = (void*)&map;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    _pricingModelMap->_currentTvlSeg = airSeg;

    MockAttributes attrs;
    attrs.add("B50", "FAIRBASE1");
    attrs.add("B70", "ADT");

    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeRequestedFareBasisInformation(attrs),
                         tse::ErrorResponseException);
  }

  void testStoreMulitpleRequestedFareBasisInformation()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping map = {0, 0};
    setupMappingRFB(map);
    _pricingModelMap->_currentMapEntry = (void*)&map;

    AirSeg* airSeg = _memHandle.create<AirSeg>();
    _pricingModelMap->_currentTvlSeg = airSeg;

    MockAttributes attr1, attr2;
    setupAttributesRFB(attr1, "FAIRBASE1", "ADT");
    setupAttributesRFB(attr2, "FAIRBASE2", "ADT");

    _pricingModelMap->storeRequestedFareBasisInformation(attr1);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->storeRequestedFareBasisInformation(attr2);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->saveRequestedFareBasisInSegmentInformation();

    CPPUNIT_ASSERT_EQUAL((size_t)2, airSeg->requestedFareBasis().size());
  }

  void testCommandPricingAndSpecificFareBasisSameSegment()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping mapSGI = {0, 0};
    addMembersToMap(mapSGI);
    mapSGI.members[_pricingModelMap->SDBMHash("B50")] = 18;
    MockAttributes attrSGICP;
    setupAttributesSGI(attrSGICP);
    attrSGICP.add("B50", "FAIRBASE");

    PricingModelMap::Mapping mapFLI = {0, 0};
    setupMappingFLI(mapFLI);
    MockAttributes attrFLI;
    setupAttributesFLI(attrFLI);

    PricingModelMap::Mapping mapRFB = {0, 0};
    setupMappingRFB(mapRFB);
    MockAttributes attrRFB;
    setupAttributesRFB(attrRFB, "FAIRBASE1", "ADT");

    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGICP);
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    _pricingModelMap->_currentMapEntry = (void*)&mapRFB;
    _pricingModelMap->storeRequestedFareBasisInformation(attrRFB);
    _pricingModelMap->saveRequestedFareBasisInformation();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->saveSegmentInformation(), tse::ErrorResponseException);
  }

  void testSpecificFareBasisAndCommandPricingSameSegment()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping mapSGI = {0, 0};
    addMembersToMap(mapSGI);
    mapSGI.members[_pricingModelMap->SDBMHash("B50")] = 18;
    MockAttributes attrSGICP;
    setupAttributesSGI(attrSGICP);
    attrSGICP.add("B50", "FAIRBASE");

    PricingModelMap::Mapping mapFLI = {0, 0};
    setupMappingFLI(mapFLI);
    MockAttributes attrFLI;
    setupAttributesFLI(attrFLI);

    PricingModelMap::Mapping mapRFB = {0, 0};
    setupMappingRFB(mapRFB);
    MockAttributes attrRFB;
    setupAttributesRFB(attrRFB, "FAIRBASE1", "ADT");

    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGICP);
    _pricingModelMap->_currentMapEntry = (void*)&mapRFB;
    _pricingModelMap->storeRequestedFareBasisInformation(attrRFB);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->saveSegmentInformation(), tse::ErrorResponseException);
  }

  void testCommandPricingAndSpecificFareBasisSeparateSegment()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping mapSGI = {0, 0};
    addMembersToMap(mapSGI);
    mapSGI.members[_pricingModelMap->SDBMHash("B50")] = 18;
    MockAttributes attrSGI, attrSGICP;
    setupAttributesSGI(attrSGI);
    setupAttributesSGI(attrSGICP);
    attrSGICP.add("B50", "FAIRBASE");

    PricingModelMap::Mapping mapFLI = {0, 0};
    setupMappingFLI(mapFLI);

    MockAttributes attrFLI;
    setupAttributesFLI(attrFLI);

    PricingModelMap::Mapping mapRFB = {0, 0};
    setupMappingRFB(mapRFB);

    MockAttributes attrRFB;
    setupAttributesRFB(attrRFB, "FAIRBASE1", "ADT");

    // First segment
    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGICP);
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    _pricingModelMap->saveSegmentInformation();

    //Second segment
    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGI);
    _pricingModelMap->_currentMapEntry = (void*)&mapRFB;
    _pricingModelMap->storeRequestedFareBasisInformation(attrRFB);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->saveSegmentInformation(), tse::ErrorResponseException);
  }

  void testSpecificFareBasisAndCommandPricingSeparateSegment()
  {
    setupPricingModelMapCommon();

    PricingModelMap::Mapping mapSGI = {0, 0};
    addMembersToMap(mapSGI);
    mapSGI.members[_pricingModelMap->SDBMHash("B50")] = 18;

    MockAttributes attrSGI, attrSGICP;
    setupAttributesSGI(attrSGI);
    setupAttributesSGI(attrSGICP);
    attrSGICP.add("B50", "FAIRBASE");

    PricingModelMap::Mapping mapFLI = {0, 0};
    setupMappingFLI(mapFLI);
    MockAttributes attrFLI;
    setupAttributesFLI(attrFLI);

    PricingModelMap::Mapping mapRFB = {0, 0l};
    setupMappingRFB(mapRFB);
    MockAttributes attrRFB;
    setupAttributesRFB(attrRFB, "FAIRBASE1", "ADT");

    // First segment
    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGI);
    _pricingModelMap->_currentMapEntry = (void*)&mapRFB;
    _pricingModelMap->storeRequestedFareBasisInformation(attrRFB);
    _pricingModelMap->saveRequestedFareBasisInformation();
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    _pricingModelMap->saveSegmentInformation();

    //Second segment
    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGICP);
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    CPPUNIT_ASSERT_THROW(_pricingModelMap->saveSegmentInformation(), tse::ErrorResponseException);
  }

  struct rfbTestStruct
  {
    std::string fbc;
    std::string pax;
    bool        isThrowing;
  };

  typedef std::vector<rfbTestStruct> rfbList_t;

  void buildTravelSegmentWithRFBlist(const rfbList_t& rfbList, bool isThrowingSGI)
  {
    PricingModelMap::Mapping mapSGI = {0, 0};
    addMembersToMap(mapSGI);
    MockAttributes attrSGI;
    setupAttributesSGI(attrSGI);

    PricingModelMap::Mapping mapFLI = {0, 0};
    setupMappingFLI(mapFLI);
    MockAttributes attrFLI;
    setupAttributesFLI(attrFLI);

    PricingModelMap::Mapping mapRFB = {0, 0};
    setupMappingRFB(mapRFB);

    _pricingModelMap->_currentMapEntry = (void*)&mapSGI;
    _pricingModelMap->storeSegmentInformation(attrSGI);
    _pricingModelMap->_currentMapEntry = (void*)&mapFLI;
    _pricingModelMap->storeFlightInformation(attrFLI);
    saveFlightInformation();
    _pricingModelMap->_currentMapEntry = (void*)&mapRFB;
    for (const rfbTestStruct rfbTest : rfbList)
    {
      MockAttributes attrRFB;
      setupAttributesRFB(attrRFB, rfbTest.fbc, rfbTest.pax);
      if (rfbTest.isThrowing)
      {
        CPPUNIT_ASSERT_THROW(_pricingModelMap->storeRequestedFareBasisInformation(attrRFB),
                             tse::ErrorResponseException);
      }
      else
        _pricingModelMap->storeRequestedFareBasisInformation(attrRFB);
      _pricingModelMap->saveRequestedFareBasisInformation();
    }
    if (isThrowingSGI)
    {
      CPPUNIT_ASSERT_THROW(_pricingModelMap->saveSegmentInformation(), tse::ErrorResponseException);
    }
    else
      _pricingModelMap->saveSegmentInformation();
  }

  void testSpecificFareBasisPositiveOneTravelSegWithOneFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPA { { "FBC1", "", false } };
    buildTravelSegmentWithRFBlist(rfbListPA, false);
  }

  void testSpecificFareBasisPositiveOneTravelSegWithOneRfb()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    buildTravelSegmentWithRFBlist(rfbListPC, false);
  }

  void testSpecificFareBasisPositiveOneTravelSegWithTwoRfbSameFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPF { { "FBC1", "PX1", false },
                          { "FBC1", "PX2", false } };
    buildTravelSegmentWithRFBlist(rfbListPF, false);
  }

  void testSpecificFareBasisPositiveOneTravelSegWithTwoRfbDiffFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPG { { "FBC1", "PX1", false },
                          { "FBC2", "PX2", false } };
    buildTravelSegmentWithRFBlist(rfbListPG,  false);
  }

  void testSpecificFareBasisPositiveTwoTravelSegWithSameFbcEach()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPA { { "FBC1", "", false } };
    buildTravelSegmentWithRFBlist(rfbListPA, false);
    buildTravelSegmentWithRFBlist(rfbListPA, false);
  }

  void testSpecificFareBasisPositiveTwoTravelSegWithDiffFbcEach()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPA { { "FBC1", "", false } };
    rfbList_t rfbListPB { { "FBC2", "", false } };
    buildTravelSegmentWithRFBlist(rfbListPA, false);
    buildTravelSegmentWithRFBlist(rfbListPB, false);
  }

  void testSpecificFareBasisPositiveTwoTravelSegWithSameRfbEach()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    buildTravelSegmentWithRFBlist(rfbListPC, false);
    buildTravelSegmentWithRFBlist(rfbListPC, false);
  }

  void testSpecificFareBasisPositiveTwoTravelSegWithDiffRfbButSameFbcEach()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    rfbList_t rfbListPD { { "FBC1", "PX2", false } };
    buildTravelSegmentWithRFBlist(rfbListPC, false);
    buildTravelSegmentWithRFBlist(rfbListPD, false);
  }

  void testSpecificFareBasisPositiveTwoTravelSegWithDiffRfbEach()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    rfbList_t rfbListPE { { "FBC2", "PX2", false } };
    buildTravelSegmentWithRFBlist(rfbListPC, false);
    buildTravelSegmentWithRFBlist(rfbListPE, false);
  }

  void testSpecificFareBasisNegativeOneTravelSegWithSameTwoFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListNA { { "FBC1", "", false },
                          { "FBC1", "", true  } };
    buildTravelSegmentWithRFBlist(rfbListNA,  false);
  }

  void testSpecificFareBasisNegativeOneTravelSegWithDiffTwoFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListNB { { "FBC1", "", false },
                          { "FBC2", "", true  } };
    buildTravelSegmentWithRFBlist(rfbListNB,  false);
  }

  void testSpecificFareBasisNegativeOneTravelSegWithOneFbcAndOneRfb()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListNC { { "FBC1", "",    false },
                          { "FBC1", "PX1", true  } };
    buildTravelSegmentWithRFBlist(rfbListNC,  false);
  }

  void testSpecificFareBasisNegativeOneTravelSegWithOneRfbAndOneFbc()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListND { { "FBC1", "PX1", false },
                          { "FBC1", "",    true  } };
    buildTravelSegmentWithRFBlist(rfbListND,  false);
  }

  void testSpecificFareBasisNegativeOneTravelSegWithSameTwoRfb()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListNE { { "FBC1", "PX1", false },
                          { "FBC1", "PX1", true  } };
    buildTravelSegmentWithRFBlist(rfbListNE,  false);
  }

  void testSpecificFareBasisNegativeTwoTravelSegWithFbcInFirstAndRfbInSecond()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPA { { "FBC1", ""   , false } };
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    buildTravelSegmentWithRFBlist(rfbListPA,  false);
    buildTravelSegmentWithRFBlist(rfbListPC,  true);
  }

  void testSpecificFareBasisNegativeTwoTravelSegWithRfbInFirstAndFbcInSecond()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListPC { { "FBC1", "PX1", false } };
    rfbList_t rfbListPA { { "FBC1", ""   , false } };
    buildTravelSegmentWithRFBlist(rfbListPC,  false);
    buildTravelSegmentWithRFBlist(rfbListPA,  true);
  }

  void testSpecificFareBasisThreeTravelSegOneWithoutRfbPart()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListEmpty;
    rfbList_t rfbListRFB { { "FBC1", "", false } };
    buildTravelSegmentWithRFBlist(rfbListRFB, false);
    buildTravelSegmentWithRFBlist(rfbListEmpty, false);
    buildTravelSegmentWithRFBlist(rfbListRFB, false);
  }

  void testSpecificFareBasisThreeTravelSegTwoWithoutRfbPart()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbListEmpty;
    rfbList_t rfbListRFB { { "FBC1", "", false } };
    buildTravelSegmentWithRFBlist(rfbListEmpty, false);
    buildTravelSegmentWithRFBlist(rfbListRFB, false);
    buildTravelSegmentWithRFBlist(rfbListEmpty, false);
  }

  void testSpecificFareBasisDisabledCombinationWithWpaWpasWq()
  {
    _pricingModelMap->_itin = _memHandle.create<Itin>();
    _pricingModelMap->_pricingTrx = _memHandle.create<AltPricingTrx>();
    _pricingModelMap->_pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    _pricingModelMap->_pricingTrx->billing()=_memHandle.create<Billing>();
    _pricingModelMap->_pricingTrx->ticketingDate() = DateTime(2051, 1, 1);
    _pricingModelMap->_pricingTrx->itin().push_back(_pricingModelMap->_itin);
    rfbList_t rfbList { { "FBC", "", false } };
    buildTravelSegmentWithRFBlist(rfbList, true);
  }

  void testSpecificFareBasisDisabledCombinationWithWpocb()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbList { { "FBC", "", false } };
    _pricingModelMap->_options->bookingCodeOverride() = true;
    buildTravelSegmentWithRFBlist(rfbList, true);
  }

  void testSpecificFareBasisDisabledCombinationWithWpncWpncb()
  {
    setupPricingModelMapCommon();
    _pricingModelMap->_pricingTrx->getRequest()->lowFareNoRebook() = 'T';
    _pricingModelMap->_pricingTrx->getRequest()->lowFareRequested() = 'T';
    rfbList_t rfbList { { "FBC", "", false } };
    buildTravelSegmentWithRFBlist(rfbList, true);
  }

  void testSpecificFareBasisDisabledCombinationWithWpncs()
  {
    setupPricingModelMapCommon();
    rfbList_t rfbList { { "FBC", "", false } };
    _pricingModelMap->_pricingTrx->getRequest()->lowFareNoRebook() = 'T';
    _pricingModelMap->_pricingTrx->getRequest()->lowFareNoAvailability() = 'T';
    buildTravelSegmentWithRFBlist(rfbList, true);
  }

  void cleanAndPreparePricingModelMap()
  {
    _pricingModelMap->reset();
    _pricingModelMap->_pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    _pricingModelMap->_currentTvlSeg = _memHandle.create<AirSeg>();
    Loc* originLoc = _memHandle.create<Loc>();
    Loc* destLoc = _memHandle.create<Loc>();
    originLoc->loc() = "LON";
    destLoc->loc() = "KRK";
    _pricingModelMap->_currentTvlSeg->origin() = originLoc;
    _pricingModelMap->_currentTvlSeg->destination() = destLoc;
    _pricingModelMap->_currentTvlSeg->origAirport() = "LON";
    _pricingModelMap->_currentTvlSeg->destAirport() = "KRK";
  }

  void testSaveFlightInformationDiscountAndPlusUpAmount()
  {
    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmountNew = MoneyAmount(-10);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), true);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntryNew(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmountNew = MoneyAmount(0);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntryNew(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmountNew = MoneyAmount(10);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntryNew(), true);

    fallback::value::azPlusUp.set(true);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmountNew = MoneyAmount(-10);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntry(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmountNew = MoneyAmount(0);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntry(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountAmount = MoneyAmount(10);
    _pricingModelMap->_discountCurrencyCode = CurrencyCode("USD");
    _pricingModelMap->_discountGroupNum = 1;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPAEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDAEntry(), true);
  }

  void testStoringSegmentInformationForDiscountAndPlusUpAmount()
  {
    PricingModelMap::Mapping mapp;
    addMembersToMap(mapp);
    _pricingModelMap->_currentMapEntry = static_cast<void*>(&mapp);

    MockAttributes attrs;
    attrs.add("DMA", "10"); // 43 -> DMA
    attrs.add("C48", "USD"); //27 -> C48
    attrs.add("Q12", "1"); //28 -> Q12

    _pricingModelMap->storeSegmentInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_discountAmountNew, MoneyAmount(-10));
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_discountCurrencyCode, CurrencyCode("USD"));
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_discountGroupNum, static_cast<size_t>(1));

    attrs.add("C6I", "10"); //16 -> C6I

    _pricingModelMap->storeSegmentInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_discountAmountNew, MoneyAmount(10));
  }

  void testSaveFlightInformationDiscountAndPlusUpPercentage()
  {
    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentageNew = -10;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), true);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntryNew(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentageNew = 0;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntryNew(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentageNew = 10;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntryNew(), true);

    fallback::value::azPlusUp.set(true);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentageNew = -10;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntry(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentageNew = 0;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntry(), false);

    cleanAndPreparePricingModelMap();
    _pricingModelMap->_discountPercentage = 10;
    _pricingModelMap->_currentTvlSeg->segmentOrder() = 1;
    _pricingModelMap->saveFlightInformation();
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isPPEntry(), false);
    CPPUNIT_ASSERT_EQUAL(_pricingModelMap->_pricingTrx->getRequest()->isDPEntry(), true);
  }

  void testStoringSegmentInformationForDiscountAndPlusUpPercentage()
  {
    PricingModelMap::Mapping mapp;
    addMembersToMap(mapp);
    _pricingModelMap->_currentMapEntry = static_cast<void*>(&mapp);

    MockAttributes attrs;
    attrs.add("DMP", "10"); // 44 -> PUP
    attrs.add("Q12", "1"); //28 -> Q12

    _pricingModelMap->storeSegmentInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(-10.0, _pricingModelMap->_discountPercentageNew);

    attrs.add("Q17", "10"); //17 -> Q17

    _pricingModelMap->storeSegmentInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(10.0, _pricingModelMap->_discountPercentageNew);
  }

  void testAddZeroDiscountForSegsWithNoDiscountIfReqHasDiscount()
  {
    fallback::value::azPlusUp.set(true);

    _pricingModelMap->_pricingTrx = _memHandle.create<PricingTrx>();
    _pricingModelMap->_pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    _pricingModelMap->_itin = _memHandle.create<Itin>();

    TravelSeg* seg1 = _memHandle.create<AirSeg>();
    TravelSeg* seg2 = _memHandle.create<ArunkSeg>();
    TravelSeg* seg3 = _memHandle.create<AirSeg>();
    TravelSeg* seg4 = _memHandle.create<AirSeg>();

    seg1->segmentOrder() = 1;
    seg2->segmentOrder() = 2;
    seg3->segmentOrder() = 3;
    seg4->segmentOrder() = 4;

    _pricingModelMap->_itin->travelSeg().push_back(seg1);
    _pricingModelMap->_itin->travelSeg().push_back(seg2);
    _pricingModelMap->_itin->travelSeg().push_back(seg3);
    _pricingModelMap->_itin->travelSeg().push_back(seg4);
    _pricingModelMap->_pricingTrx->getRequest()->addDiscountAmountNew(1, 1, 10.0, CurrencyCode("EUR"));

    CommonParserUtils::addZeroDiscountForSegsWithNoDiscountIfReqHasDiscount(
        _pricingModelMap->_pricingTrx->getRequest()->discountsNew(),
        _pricingModelMap->_itin->travelSeg(),
        _pricingModelMap->_pricingTrx->isMip());

    const DiscountAmount* amount = _pricingModelMap->_pricingTrx->getRequest()->discountAmountNew(1);
    CPPUNIT_ASSERT_EQUAL(amount->amount, 10.0);
    CPPUNIT_ASSERT_EQUAL(amount->currencyCode, CurrencyCode("EUR"));
    CPPUNIT_ASSERT_EQUAL(amount->startSegmentOrder, static_cast<int16_t>(1));
    CPPUNIT_ASSERT_EQUAL(amount->endSegmentOrder, static_cast<int16_t>(1));

    amount = _pricingModelMap->_pricingTrx->getRequest()->discountAmountNew(3);
    CPPUNIT_ASSERT_EQUAL(amount->amount, 0.0);
    CPPUNIT_ASSERT_EQUAL(amount->currencyCode, CurrencyCode("EUR"));
    CPPUNIT_ASSERT_EQUAL(amount->startSegmentOrder, static_cast<int16_t>(3));
    CPPUNIT_ASSERT_EQUAL(amount->endSegmentOrder, static_cast<int16_t>(4));

    amount = _pricingModelMap->_pricingTrx->getRequest()->discountAmountNew(4);
    CPPUNIT_ASSERT_EQUAL(amount->amount, 0.0);
    CPPUNIT_ASSERT_EQUAL(amount->currencyCode, CurrencyCode("EUR"));
    CPPUNIT_ASSERT_EQUAL(amount->startSegmentOrder, static_cast<int16_t>(3));
    CPPUNIT_ASSERT_EQUAL(amount->endSegmentOrder, static_cast<int16_t>(4));

    _pricingModelMap->_pricingTrx->setRequest(_memHandle.create<PricingRequest>());
    _pricingModelMap->_pricingTrx->getRequest()->addDiscountPercentage(1, 10.0);

    CommonParserUtils::addZeroDiscountForSegsWithNoDiscountIfReqHasDiscount(
        _pricingModelMap->_pricingTrx->getRequest()->discountsNew(),
        _pricingModelMap->_itin->travelSeg(),
        _pricingModelMap->_pricingTrx->isMip());

    const Percent* percentage = _pricingModelMap->_pricingTrx->getRequest()->discountPercentageNew(
        1, *_pricingModelMap->_pricingTrx);
    CPPUNIT_ASSERT_EQUAL(*percentage, 10.0);

    percentage = _pricingModelMap->_pricingTrx->getRequest()->discountPercentageNew(
        3, *_pricingModelMap->_pricingTrx);
    CPPUNIT_ASSERT_EQUAL(*percentage, 0.0);

    percentage = _pricingModelMap->_pricingTrx->getRequest()->discountPercentageNew(
        4, *_pricingModelMap->_pricingTrx);
    CPPUNIT_ASSERT_EQUAL(*percentage, 0.0);
  }

  void testStoreProcOptsInformationBI0CabinRequestPB()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "PB");
    _pricingModelMap->storeProcOptsInformation(attrsWithBI0);
    CPPUNIT_ASSERT_EQUAL(Indicator('1'), trx->getOptions()->cabin().getCabinIndicator());
  }

  void testStoreProcOptsInformationBI0CabinRequestYB()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "YB");
    _pricingModelMap->storeProcOptsInformation(attrsWithBI0);
    CPPUNIT_ASSERT_EQUAL(Indicator('8'), trx->getOptions()->cabin().getCabinIndicator());
  }

  void testStoreProcOptsInformationBI0CabinRequestThrowException()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "XX");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeProcOptsInformation(attrsWithBI0), ErrorResponseException);
  }

  void testStoreProcOptsInformationBI0CabinRequestUndefinedClass()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "");
    _pricingModelMap->storeProcOptsInformation(attrsWithBI0);
    CPPUNIT_ASSERT(_pricingModelMap->_options->cabin().isUndefinedClass() == true);
  }

  void testStoreProcOptsInformationBI0CabinRequestInvalidLengthOneChar()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "X");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeProcOptsInformation(attrsWithBI0), ErrorResponseException);
  }

  void testStoreProcOptsInformationBI0CabinRequestInvalidLengthThreeChar()
  {
    PricingTrx* trx = _memHandle.create<NoPNRPricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->noPNRPricing() = true;

    PricingModelMap::Mapping map = {0, 0};
    map.members[_pricingModelMap->SDBMHash("BI0")] = 192;
    _pricingModelMap->_currentMapEntry = (void*)&map;

    _pricingModelMap->_pricingTrx = trx;

    MockAttributes attrsWithBI0;
    attrsWithBI0.add("BI0", "XXX");
    CPPUNIT_ASSERT_THROW(_pricingModelMap->storeProcOptsInformation(attrsWithBI0), ErrorResponseException);
  }

  void testIsValidAmount()
  {
    std::string s = "0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "0");

    s = "0.0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "0");

    s = "000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "0");

    s = "00.0000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "0");

    s = ".0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "0");

    s = "-0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "-0.0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "-0.00";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "-0.000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "-000.00000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "33";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33");

    s = "-33";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "-33");

    s = "33.33";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33.33");

    s = "-33.33";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "-33.33");

    s = "33.333";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "33.330";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33.33");

    s = "33.3300";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33.33");

    s = "33.330";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33.33");

    s = "0033.330";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "33.33");

    s = "-0033.330";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == "-33.33");

    s = "33.333";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "33.333");

    s = "12345678901";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "12345678901");

    s = "12345678901.";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "12345678901");

    s = "12345678901.0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "12345678901");

    s = "12345678901.00";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "12345678901");

    s = "-12345678901";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678901");

    s = "-12345678901.";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678901");

    s = "-12345678901.0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678901");

    s = "-12345678901.00";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678901");

    s = "-12345678901.";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678901");

    s = "-1234567890.";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-1234567890");

    s = "-123456789.1";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-123456789.1");

    s = "-12345678.01";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-12345678.01");

    s = "-1234567.901";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-1234567.901");

    s = "-1234567.901000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-1234567.901");

    s = "-0001234567.901";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-1234567.901");

    s = "-0001234567.901000";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-1234567.901");

    s = "-123456.8901";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == false);

    s = "123e.00";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == false);

    s = "1234.#0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == false);

    s = "-3003.303";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-3003.303");

    s = "3003.303";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "3003.303");

    s = "-   03003.303";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-3003.303");

    s = " -   3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-3003.303");

    s = " -3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-3003.303");

    s = "   -   3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "-3003.303");

    s = "   03003.303";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "3003.303");

    s = "   3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "3003.303");

    s = "3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 3) == true);
    CPPUNIT_ASSERT(s == "3003.303");

    s = "   -   3003.303     ";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == false);

    s = "5.2";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 1) == true);
    CPPUNIT_ASSERT(s == "5.2");

    s = "5.20";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 1) == true);
    CPPUNIT_ASSERT(s == "5.2");

    s = "5.21";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 1) == false);

    s = "5.2";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 0) == false);

    s = "5.0";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 0) == true);
    CPPUNIT_ASSERT(s == "5");

    s = "0.50";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == ".5");

    s = "0.55";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == ".55");

    s = ".55";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == ".55");

    s = "0.01";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 2) == true);
    CPPUNIT_ASSERT(s == ".01");

    s = "0.01";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 1) == false);

    s = ".01";
    CPPUNIT_ASSERT(_pricingModelMap->isValidAmount(s, 1) == false);
  }

  void initMaxOBFee()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    trx->setRequest(req);
    _pricingModelMap->_pricingTrx = trx;

    // Now let's set directly a value for "MOB"
    PricingModelMap::Mapping* map = new PricingModelMap::Mapping;
    map->trxFunc = nullptr;
    map->func = nullptr;
    map->members[_pricingModelMap->SDBMHash("MOB")] = 194;
    _pricingModelMap->_currentMapEntry = static_cast<void*>(map);
  }

  void testCheckMAXOBFeeValueYes()
  {
    initMaxOBFee();
    PricingRequest* req = _pricingModelMap->_pricingTrx->getRequest();

    MockAttributes mobNo;
    mobNo.add("MOB", "Y");
    _pricingModelMap->storeProcOptsInformation(mobNo);

    CPPUNIT_ASSERT(req->returnMaxOBFeeOnly() == true);
    delete static_cast<PricingModelMap::Mapping*>(_pricingModelMap->_currentMapEntry);
  }

  void testCheckMAXOBFeeValueNo()
  {
    initMaxOBFee();
    PricingRequest* req = _pricingModelMap->_pricingTrx->getRequest();

    MockAttributes mobNo;
    mobNo.add("MOB", "N");
    _pricingModelMap->storeProcOptsInformation(mobNo);

    CPPUNIT_ASSERT(req->returnMaxOBFeeOnly() == false);
    delete static_cast<PricingModelMap::Mapping*>(_pricingModelMap->_currentMapEntry);
  }

  void testCheckMAXOBEmptyXMLTag()
  {
    initMaxOBFee();
    PricingRequest* req = _pricingModelMap->_pricingTrx->getRequest();

    MockAttributes mobNo;
    mobNo.add("MOB", "");
    _pricingModelMap->storeProcOptsInformation(mobNo);

    CPPUNIT_ASSERT(req->returnMaxOBFeeOnly() == false);
    delete static_cast<PricingModelMap::Mapping*>(_pricingModelMap->_currentMapEntry);
  }

  void testCheckMAXOBFeeGarbageValue()
  {
    initMaxOBFee();
    PricingRequest* req = _pricingModelMap->_pricingTrx->getRequest();

    MockAttributes mobNo;
    mobNo.add("MOB", "^#!&*^#*!@JJJ@@@");
    _pricingModelMap->storeProcOptsInformation(mobNo);

    CPPUNIT_ASSERT(req->returnMaxOBFeeOnly() == false);
    delete static_cast<PricingModelMap::Mapping*>(_pricingModelMap->_currentMapEntry);
  }

  void testCheckMAXOBFeeDefaultValue()
  {
    initMaxOBFee();
    PricingRequest* req = _pricingModelMap->_pricingTrx->getRequest();
    CPPUNIT_ASSERT(req->returnMaxOBFeeOnly() == false);
    delete static_cast<PricingModelMap::Mapping*>(_pricingModelMap->_currentMapEntry);
  }

  void testStoreProcOptsInformation_PRM_RCQ_NoError()
  {
    PricingModelMap::Mapping map = {0, 0};

    map.members[_pricingModelMap->SDBMHash("PRM")] = 195;
    map.members[_pricingModelMap->SDBMHash("RCQ")] = 196;

    _pricingModelMap->_currentMapEntry = (void*)&map;

    MockAttributes attrs;

    attrs.add("PRM", "F");
    attrs.add("RCQ", "Abcdefghijk,Abcdefghijklmnopqrst,Abcdefghijklmnopqrst,Abcdefghijklmnopqrst");

    _pricingModelMap->_pricingTrx = _trx;
    _pricingModelMap->storeProcOptsInformation(attrs);

    PricingRequest* request = _pricingModelMap->_pricingTrx->getRequest();

    CPPUNIT_ASSERT(request != nullptr);

    const std::vector<FareRetailerCode> rcqValues = request->rcqValues();
    CPPUNIT_ASSERT(rcqValues.size() == 4);
    CPPUNIT_ASSERT(rcqValues[0] == "Abcdefghijk");
    CPPUNIT_ASSERT(rcqValues[1] == "Abcdefghijklmnopqrst");
    CPPUNIT_ASSERT(rcqValues[2] == "Abcdefghijklmnopqrst");
    CPPUNIT_ASSERT(rcqValues[3] == "Abcdefghijklmnopqrst");
  }

  void testCheckArunkSegmentForSfrReqShouldThrowWhenAllTravelSegmentsAreArunk()
  {
    ArunkSeg arunk1, arunk2;
    arunk1.segmentType() = Arunk;
    arunk2.segmentType() = Arunk;

    const std::vector<TravelSeg*> arunks = {&arunk1, &arunk2};

    buildPricingRequestWithTravelSegments(arunks);

    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkArunkSegmentForSfrReq(),
                         tse::ErrorResponseException);
  }

  void testCheckArunkSegmentForSfrReqShouldNotThrowWhenAirTravelSegmentPresentInFareMarket()
  {
    AirSeg airSeg;
    ArunkSeg arunkSeg;
    airSeg.segmentType() = Air;
    arunkSeg.segmentType() = Arunk;

    const std::vector<TravelSeg*> travelSegs{&airSeg, &arunkSeg};

    buildPricingRequestWithTravelSegments(travelSegs);

    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkArunkSegmentForSfrReq());
  }

  void testCheckArunkSegmentForSfrReqShouldNotThrowWhenOpenTravelSegmentPresentInFareMarket()
  {
    AirSeg airSeg;
    ArunkSeg arunkSeg;
    airSeg.segmentType() = Open;
    arunkSeg.segmentType() = Arunk;

    const std::vector<TravelSeg*> travelSegs{&airSeg, &arunkSeg};

    buildPricingRequestWithTravelSegments(travelSegs);

    CPPUNIT_ASSERT_NO_THROW(_pricingModelMap->checkArunkSegmentForSfrReq());
  }

  void testCheckArunkSegmentForSfrReqShouldThrowWhenTravelSegmentIsAirButNonAirEquipmentType()
  {
    ArunkSeg arunk;
    AirSeg airWithNoAirEquipmentType;
    airWithNoAirEquipmentType.equipmentType() = TRAIN;
    const std::vector<TravelSeg*> travelSegs = {&arunk, &airWithNoAirEquipmentType};

    buildPricingRequestWithTravelSegments(travelSegs);

    CPPUNIT_ASSERT_THROW(_pricingModelMap->checkArunkSegmentForSfrReq(),
                         tse::ErrorResponseException);
  }

}; // PricingModelMapTest

CPPUNIT_TEST_SUITE_REGISTRATION(PricingModelMapTest);

std::ostream& operator<<(std::ostream& os, const VCTR& vctr)
{
  return os << vctr.vendor() << ',' << vctr.carrier() << ',' << vctr.tariff() << ',' << vctr.rule()
            << ',' << vctr.sequenceNumber();
}

} // end of tse namespace
