#include "Common/ServiceFeeUtil.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "Common/TrxUtil.h"
#include "DBAccess/Customer.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/MerchCarrierPreferenceInfo.h"
#include "DBAccess/OptionalServicesActivationInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServiceGroupInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/Diag875Collector.h"
#include "Diagnostic/Diag877Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalFeeCollector.h"
#include "ServiceFees/OptionalFeeConcurValidator.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TimeBomb.h"
#include "test/testdata/TestLocFactory.h"

#include <utility>

#include <boost/assign/std/vector.hpp>

namespace tse
{
using boost::assign::operator+=;

class MockOptionalFeeConcurValidator : public OptionalFeeConcurValidator
{
public:
  MockOptionalFeeConcurValidator(PricingTrx& trx, FarePath* farePath)
    : OptionalFeeConcurValidator(trx, farePath)
  {
  }
  bool validateS6(const CarrierCode& cxr,
                  const std::set<CarrierCode>& cxrs,
                  const ServiceFeesGroup* srvFeesGrp,
                  bool marketing,
                  ServiceFeesGroup* retSrvFeesGrp)
  {
    if (cxr == _cxrPass)
    {
      _result += "CXR-" + cxr + " GRP-" + srvFeesGrp->groupCode();
      return true;
    }
    return false;
  }
  void getOptCarrierVec(const std::set<CarrierCode>& marketingCxrs,
                        const std::vector<tse::TravelSeg*>::const_iterator& first,
                        const std::vector<tse::TravelSeg*>::const_iterator& last,
                        std::vector<CarrierCode>& outCarriers) const
  {
    outCarriers += "BB", "CC", "DD";
  }
  CarrierCode _cxrPass;
  std::string _result;
};

class MockOptionalFeeCollector : public OptionalFeeCollector
{
public:
  MockOptionalFeeCollector(PricingTrx& trx)
    : OptionalFeeCollector(trx),
      _isRTJourneyType(false),
      _isIntJourneyType(false),
      _merchActValidate(true),
      _merchRetriveData(true),
      _sliceAndDiceWasCalled(false),
      _processSingleServiceFeesGroup(false),
      _timeout(false)
  {
    _vMAI.push_back(&_mai1);
    _vMAI.push_back(&_mai2);
    _mai1.carrier() = ANY_CARRIER;
    _mai2.carrier() = ANY_CARRIER;
  }

  // overriden methods
  bool roundTripJouneyType() const { return _isRTJourneyType; }

  bool internationalJouneyType() const { return _isIntJourneyType; }

  void getJourneyDestination()
  {
    ServiceFeeUtil::TravelSegVecIC ret = _farePath->itin()->travelSeg().begin();

    if (!(_isRoundTrip.getValForKey(_farePath->itin())))
    {
      _beginsOfUOT[1] = _farePath->itin()->travelSeg().end();
      ret++;
    }
    else
      _beginsOfUOT[1] = ret;

    _journeyDestination = (*ret)->destination();
  }

  bool validateS7(const OptionalServicesValidator& validator, OCFees& ocFee, bool stopMatch) const
  {
    return true;
  }

  void initializeSubCodes(ServiceFeesGroup* srvFeesGroup,
                          const CarrierCode& carrier,
                          const ServiceTypeCode& srvTypeCode,
                          const ServiceGroup& srvGroup) const
  {
    _result += carrier;
  }

  const std::vector<MerchActivationInfo*>& getMerchActivation(MerchActivationValidator& validator,
                                                              const CarrierCode& carrierCode,
                                                              const PseudoCityCode& pcc) const
  {
    return _vMAI;
  }

  bool retrieveMerchActivation(MerchActivationValidator& validator,
                               const CarrierCode& carrierCode,
                               std::vector<ServiceGroup*>& groupCode,
                               std::vector<ServiceGroup*>& dispOnlyGroups) const
  {
    return _merchRetriveData;
  }

  void getAllATPGroupCodes()
  {
    _allGroupCodesOCF.push_back(&_sgI1);
    _allGroupCodesOCF.push_back(&_sgI2);
    _sgI1.svcGroup() = "BG";
    _sgI1.definition() = "BAGGAGE";
    _sgI1.effDate() = DateTime(2009, 12, 10);
    _sgI1.discDate() = DateTime(2010, 3, 31);
    _sgI1.expireDate() = DateTime(2010, 4, 1);

    _sgI2.svcGroup() = "ML";
    _sgI2.definition() = "MEAL";
    _sgI2.effDate() = DateTime(2010, 1, 10);
    _sgI2.discDate() = DateTime(2010, 3, 31);
    _sgI2.expireDate() = DateTime(2010, 4, 1);
  }

  const std::vector<OptionalServicesActivationInfo*>&
  getOptServiceActivation(Indicator crs,
                          const UserApplCode& userCode,
                          const std::string& application)
  {
    _optServices.clear();
    _optServices.push_back(&_opt1);
    _optServices.push_back(&_opt2);

    _opt1.userApplType() = crs;
    _opt1.userAppl() = userCode;
    _opt1.groupCode() = "BG";
    _opt1.effDate() = DateTime(2010, 1, 10);
    _opt1.discDate() = DateTime(2010, 3, 31);
    _opt1.application() = application;

    _opt2.userApplType() = crs;
    _opt2.userAppl() = userCode;
    _opt2.groupCode() = "ML";
    _opt2.effDate() = DateTime(2010, 1, 10);
    _opt2.discDate() = DateTime(2010, 3, 31);
    _opt2.application() = application;

    return _optServices;
  }

  bool prePaidBaggageActive() const { return true; }

  bool processSingleServiceFeesGroup(int unitNo,
                                     ServiceFeesGroup* srvFeesGrp,
                                     bool needS6Validation,
                                     OptionalFeeConcurValidator& s6Validator)
  {
    if (!_processSingleServiceFeesGroup)
      return OptionalFeeCollector::processSingleServiceFeesGroup(
          unitNo, srvFeesGrp, needS6Validation, s6Validator);
    return !_timeout;
  }

  void sliceAndDice(int unitNo)
  {
    if (!_timeout)
      _sliceAndDiceWasCalled = true;
  }

  bool _isRTJourneyType;
  bool _isIntJourneyType;
  bool _merchActValidate;
  bool _merchRetriveData;
  std::vector<MerchActivationInfo*> _vMAI;
  MerchActivationInfo _mai1;
  MerchActivationInfo _mai2;
  ServiceGroupInfo _sgI1;
  ServiceGroupInfo _sgI2;
  std::vector<OptionalServicesActivationInfo*> _optServices;
  OptionalServicesActivationInfo _opt1;
  OptionalServicesActivationInfo _opt2;
  mutable std::string _result;
  bool _sliceAndDiceWasCalled;
  bool _processSingleServiceFeesGroup;
  bool _timeout;
};

class MockServiceFeesGroup : public ServiceFeesGroup
{
public:
  MockServiceFeesGroup() : ServiceFeesGroup(), _marketDriven(false) {}

  void collectUnitsOfTravel(
      std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> >&
          unitsOfTravel,
      ServiceFeesGroup::TSIt begIt,
      const ServiceFeesGroup::TSIt endIt)
  {
    if (begIt != endIt)
      unitsOfTravel.push_back(std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool>(
          begIt, begIt, _marketDriven));
  }

  bool _marketDriven;
};


class MyDataHandle : public DataHandleMock
{
  public:
  const std::vector<EmdInterlineAgreementInfo*>&
  getEmdInterlineAgreements(const NationCode& country,
                                const CrsCode& gds,
                                const CarrierCode& validatingCarrier)
  {

    std::vector<EmdInterlineAgreementInfo*>* eiaList =
          _memHandle.create<std::vector<EmdInterlineAgreementInfo*> >();
     if ( (country == "US") && (gds == "1S") && (validatingCarrier == "EY"))
     {
       EmdInterlineAgreementInfo* eia =  createEmdInterlineAgreement(country, gds, validatingCarrier, "KL");
       eiaList->push_back(eia);
       eia =  createEmdInterlineAgreement(country, gds, validatingCarrier, "NW");
       eiaList->push_back(eia);
       eia =  createEmdInterlineAgreement(country, gds, validatingCarrier, "AA");
       eiaList->push_back(eia);
     }
     else if ( (country == "ZZ") && (gds == "1S") && (validatingCarrier == "EY"))
     {
       EmdInterlineAgreementInfo* eia =  createEmdInterlineAgreement("ZZ", gds, validatingCarrier, "KL");
       eiaList->push_back(eia);
     }

    return *eiaList;
  }

  EmdInterlineAgreementInfo* createEmdInterlineAgreement(const NationCode& country,
                                                         const CrsCode& gds,
                                                         const CarrierCode& validatingCarrier,
                                                         const CarrierCode& participatingCarrier)
 {
   EmdInterlineAgreementInfo* eia = _memHandle.create<EmdInterlineAgreementInfo>();
   eia->setCountryCode(country);
   eia->setGds(gds);
   eia->setValidatingCarrier(validatingCarrier);
   eia->setParticipatingCarrier(participatingCarrier);
   return eia;
 }

  private:
    TestMemHandle _memHandle;

};


namespace
{
ServiceGroup serviceGroups[] = { "BG", "ML" };
}

class OptionalFeeCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalFeeCollectorTest);
  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testCollect_EmptyFarePath);
  CPPUNIT_TEST(testCollect);
  CPPUNIT_TEST(testDiag_875_printActiveMrktDrivenCxrHeader);
  CPPUNIT_TEST(testDiag_printActiveMrktDrivenNCxrNoData);
  CPPUNIT_TEST(testDiag_printActiveMrktDrivenCxrData);

  CPPUNIT_TEST(testCreateDiagnostic_NoDiagnosticRequested);
  CPPUNIT_TEST(testCreateDiagnostic_875);
  CPPUNIT_TEST(testCreateDiagnostic_DDINFO);
  CPPUNIT_TEST(testCreateDiagnostic_877);
  CPPUNIT_TEST(testCreateDiagnostic_880);

  CPPUNIT_TEST(testPrintDiagHeader);

  CPPUNIT_TEST(testCheckFlightRelated_Pass);
  CPPUNIT_TEST(testCheckFlightRelated_Fail_Tkt);
  CPPUNIT_TEST(testCheckFlightRelated_Fail_Merch);
  CPPUNIT_TEST(testCheckFlightRelated_Fail_Empty);

  CPPUNIT_TEST(testSetJourneyDestination_OW_Int);
  CPPUNIT_TEST(testSetJourneyDestination_OW_Dom);
  CPPUNIT_TEST(testSetJourneyDestination_RT_Int);
  CPPUNIT_TEST(testSetJourneyDestination_RT_Dom);

  CPPUNIT_TEST(testDefineMarketingOperatingCarriers_OneUnit);
  CPPUNIT_TEST(testDefineMarketingOperatingCarriers_TwoUnits);

  CPPUNIT_TEST(testAddNewServiceGroup);
  CPPUNIT_TEST(testIdentifyGroupCodesToBeProcessedWhenMultipleItins);

  CPPUNIT_TEST(testProcessSubCodes_Fail_S5NotFound);
  CPPUNIT_TEST(testProcessSubCodes_Fail_S5Found);
  CPPUNIT_TEST(testProcessSubCodes_Pass);

  CPPUNIT_TEST(testCheckIndCrxIndWhenOperatingStrategyAndCarrierIndicator);
  CPPUNIT_TEST(testCheckIndCrxIndWhenMarketingStrategyAndCarrierIndicator);
  CPPUNIT_TEST(testCheckIndCrxIndWhenOperatingStrategyAndIndustryIndicator);
  CPPUNIT_TEST(testCheckIndCrxIndWhenMarketingStrategyAndIndustryIndicator);

  CPPUNIT_TEST(testProcessServiceFeesGroups_SingleMarketingCxr);
  CPPUNIT_TEST(testProcessServiceFeesGroups_SingleOpratingCxr);
  CPPUNIT_TEST(testProcessServiceFeesGroups_MultipleOperatingCxr);

  CPPUNIT_TEST(testProcessServiceFeesGroup_DiffOperAndMarketing_PassMarkting);
  CPPUNIT_TEST(testProcessServiceFeesGroup_DiffOperAndMarketing_NoOperating);
  CPPUNIT_TEST(testProcessServiceFeesGroup_MultiOperating);

  CPPUNIT_TEST(testProcessServiceFeesGroups_PassMerchActivation);
  CPPUNIT_TEST(testProcessServiceFeesGroups_FailMerchActivation);

  CPPUNIT_TEST(testCheckDisplayCategoryWhenSaleInStore);
  CPPUNIT_TEST(testCheckDisplayCategoryWhenNotSaleInStore);
  CPPUNIT_TEST(testCheckEMDTypeWhenElectronicTkt);
  CPPUNIT_TEST(testCheckEMDTypeWhenNotElectronicTkt);

  CPPUNIT_TEST(testIsAgencyActiveWhenNoDeactivatedAgency);
  CPPUNIT_TEST(testIsAgencyActiveWhenAgencyDeactivated);

  CPPUNIT_TEST(testFilterOutInputGroupCodesAllGroupCodesAreValid);
  CPPUNIT_TEST(testFilterOutInputGroupCodesRequestedGroupCodesIsInvalid);
  CPPUNIT_TEST(testFilterOutInputGroupCodesRequestedGroupCodesIsValidAndInvalid);
  CPPUNIT_TEST(testFilterOutInputGroupCodesRequestedGroupCodesIsInvalidForTktDate);
  CPPUNIT_TEST(testFilterOutInputGroupCodesRequestedOneInvalidAndOneIsInvalidForTktDate);
  CPPUNIT_TEST(testIsATPActiveGroupFoundReturnTrue);
  CPPUNIT_TEST(testIsATPActiveGroupFoundReturnFalse_NotFound);
  CPPUNIT_TEST(testIsATPActiveGroupFoundReturnTrue_InvalidDate);

  CPPUNIT_TEST(testSetUpIndForShoppingWhenItIsPricingReturnTrue);
  CPPUNIT_TEST(testSetUpIndForShoppingWhenItIsMIPNoGroupInTheEntryReturnFalse);
  CPPUNIT_TEST(testSetUpIndForShoppingWhenItIsMIPMaxNumberEqualZeroReturnFalse);
  CPPUNIT_TEST(testSetUpIndForShoppingWhenItIsMIPMaxNumberEqualNoLimitReturnFalse);
  CPPUNIT_TEST(testSetUpIndForShoppingWhenItIsMIPAndMaxNumberReturnTrue);

  CPPUNIT_TEST(testCheckNumbersToStopProcessingNoStopNeededReturnTrue);
  CPPUNIT_TEST(testCheckNumbersToStopProcessingForPricingWhenNoGroupRequestedReturnFalse);
  CPPUNIT_TEST(
      testCheckNumbersToStopProcessingForShoppingWhenNumberPassedOptionsLessMaxNumberReturnTrue);
  CPPUNIT_TEST(
      testCheckNumbersToStopProcessingForShoppingWhenNumberPassedOptionsIsMaxNumberPlusOneReturnFalse);

  CPPUNIT_TEST(testFinalCheckShoppingPricingForStopProcessingNoStopRequiredReturnTrue);
  CPPUNIT_TEST(testFinalCheckShoppingPricingForStopProcessingForPricingReturnFalse);
  CPPUNIT_TEST(testFinalCheckShoppingPricingForStopProcessingForShoppingReturnFalse);

  CPPUNIT_TEST(testGetAllActiveGroupCodesforSABRE_USER);
  CPPUNIT_TEST(testGetAllActiveGroupCodesforAXESS_USER);
  CPPUNIT_TEST(testGetAllActiveGroupCodesforABACUS_USER);
  CPPUNIT_TEST(test_sortOCFeeGroup_Empty);
  CPPUNIT_TEST(test_sortOCFeeGroup_Sorted);
  CPPUNIT_TEST(test_sortOCFeeGroup_Sorted_Shopping);
  CPPUNIT_TEST(test_getGroupCodesNotProcessedForTicketingDate);
  CPPUNIT_TEST(test_getGroupCodesNotProcessedForTicketingDate_Test);
  CPPUNIT_TEST(testSelectActiveNotActiveGroupCodesReturnActiveAndNotActive);
  CPPUNIT_TEST(testSelectActiveNotActiveGroupCodesReturnsAllRequestedAsActive);

  CPPUNIT_TEST(testCleanUpOCFeesNoValidOCFeesReturn);
  CPPUNIT_TEST(testCleanUpOCFeesOneValidOCFeesReturn);
  CPPUNIT_TEST(testCleanUpOCFeesTWOValidOCFeesReturn);
  CPPUNIT_TEST(testCleanUpOCFeesAllValidOCFeesReturn);

  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeProcessedAfterMarketingCxrStep);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeNotProcessedAfterMarketingCxrStep);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeProcessedAfterSliceAndDiceMarketingCxrStep);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeNotProcessedAfterSliceAndDiceMarketingCxrStep);
  CPPUNIT_TEST(testIsSubCodeProcessedWhenSubCodeProcessedAfterLargestPortionOfTravelStep);

  CPPUNIT_TEST(testProcessSliceAndDicePortion);

  CPPUNIT_TEST(testProcessLargestPortionOfTvl_Selected);
  CPPUNIT_TEST(testProcessLargestPortionOfTvl_NotSelected);

  CPPUNIT_TEST(testProcessCarrierMMRecords_Retrieved);
  CPPUNIT_TEST(testProcessCarrierMMRecords_NotRetrieved);

  CPPUNIT_TEST(testValidateS5Data_Fail_TktDate);
  CPPUNIT_TEST(testValidateS5Data_Fail_FlightRelated);
  CPPUNIT_TEST(testValidateS5Data_Fail_Industry);
  CPPUNIT_TEST(testValidateS5Data_Fail_Category);
  CPPUNIT_TEST(testValidateS5Data_Fail_Emd);


  CPPUNIT_TEST(testValidateS5Data_Pass);

  CPPUNIT_TEST(testCheckDiagForS5Detail);
  CPPUNIT_TEST(testCheckDiagForS5Detail_DDINFO);
  CPPUNIT_TEST(testPrintDiagS5Info);
  CPPUNIT_TEST(testPrintDiagS5Info_DDINFO);
  CPPUNIT_TEST(testPrintNoGroupCodeProvided);
  CPPUNIT_TEST(testPrintNoOptionsRequested);
  CPPUNIT_TEST(testPrintPccIsDeactivated);
  CPPUNIT_TEST(testPrintCanNotCollect);
  CPPUNIT_TEST(testPrintDiagS5NotFound);
  CPPUNIT_TEST(testPrintDiagS5NotFound_DDINFO);
  CPPUNIT_TEST(testPrintS7Header);
  CPPUNIT_TEST(testPrintS7Header_DDINFO);
  CPPUNIT_TEST(testPrintStopAtFirstMatchMsg);

  CPPUNIT_TEST(testGetValidRoutesWhenOneSegment);
  CPPUNIT_TEST(testGetValidRoutesWhen2Segments);
  CPPUNIT_TEST(testGetValidRoutesWhen3Segments);
  CPPUNIT_TEST(testGetValidRoutesWhen3SegmentsAndSecondIsArunk);
  CPPUNIT_TEST(testGetValidRoutesWhen3SegmentsWithRetransit);
  CPPUNIT_TEST(testAddInvalidGroupsPricing);
  CPPUNIT_TEST(testAddInvalidGroupsShopping);

  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenAllConfirmed);
  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenPartiallyConfirmed);
  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenAllUnconfirmed);
  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenPartiallyConfirmedDiag);
  CPPUNIT_TEST(testCheckAllSegsConfirmedWhenAllUnconfirmedDiag);

  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegTypeArunk);
  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegTypeSurface);
  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegStatusConfirm);
  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegStatusNoSeat);
  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenAllUnconfirmed);
  CPPUNIT_TEST(testCheckAllSegsUnconfirmedWhenAllConfirmed);

  CPPUNIT_TEST(testUpdateServiceFeesGroupStateWithFees);
  CPPUNIT_TEST(testUpdateServiceFeesGroupStateWhenNoFeesAndNotInCxrMap);
  CPPUNIT_TEST(testUpdateServiceFeesGroupStateWhenNoFeesAndInCxrMap);
  CPPUNIT_TEST(testUpdateServiceFeesDisplayOnlyState_DispOnly);
  CPPUNIT_TEST(testUpdateServiceFeesDisplayOnlyState_NotDispOnly_Carrier);
  CPPUNIT_TEST(testUpdateServiceFeesDisplayOnlyState_NotDispOnly_Group);

  CPPUNIT_TEST(testIsTimeOut_Pricing_TimeOut_Not_Occur);
  CPPUNIT_TEST(testIsTimeOut_Shopping_TimeOut_Not_Occur);
  CPPUNIT_TEST(testIsTimeOut_Pricing_TimeOut_Occur);
  CPPUNIT_TEST(testCleanUpAllOCFees_TimeOut);
  CPPUNIT_TEST(testCleanUpGroupCode_TimeOut);
  CPPUNIT_TEST(testCollectExcludedSFG_NothingToCollect);
  CPPUNIT_TEST(testCollectExcludedSFG_NVCollect);
  CPPUNIT_TEST(testCollectExcludedSFG_NACollect);
  CPPUNIT_TEST(testCollectExcludedSFG_NANVCollect);
  CPPUNIT_TEST(testIsTravelSegFound_NotFound);
  CPPUNIT_TEST(testIsTravelSegFound_Found);
  CPPUNIT_TEST(testCheckOCFeeMap_NotFound);
  CPPUNIT_TEST(testCheckOCFeeMap_Found);
  CPPUNIT_TEST(testCheckLargestPortion_SegmentNotFound);
  CPPUNIT_TEST(testCheckLargestPortion_SegmentFound);
  CPPUNIT_TEST(testIsOCFeesProcessedForLargestPortions_PASS);
  CPPUNIT_TEST(testIsOCFeesProcessedForLargestPortions_FAIL);
  CPPUNIT_TEST(testTimeOutPostProcessing_WP);
  CPPUNIT_TEST(testTimeOutPostProcessing_WPAE_with_no_groupCodes);
  CPPUNIT_TEST(testTimeOutPostProcessing_WPAE_with_Group_Not_Requested);
  CPPUNIT_TEST(testTimeOutPostProcessing_WPAE_with_Group_Requested_Not_Processed);
  CPPUNIT_TEST(testTimeOutPostProcessing_WPAE_with_Group_Requested_And_Completely_Processed);

  CPPUNIT_TEST(testMarketingSameAsOperating_OneOperatingCxr_MatchMarketingCxr);
  CPPUNIT_TEST(testMarketingSameAsOperating_OneOperatingCxr_NotMatchMarketingCxr);
  CPPUNIT_TEST(testMarketingSameAsOperating_MultipleOperatingCxr);

  CPPUNIT_TEST(testGetFilterMask);

  CPPUNIT_TEST(testCheckFlightRelated_Pass_Baggage);

  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenSliceAndDiceAndIsProcessed);
  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenSliceAndDiceAndIsNotProcessed);
  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenNoMarketDrivenFlights);
  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenHasInterlineJourneyFlight);
  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenHasInterlineJourneyFlightButGotTimeOut);
  CPPUNIT_TEST(testProcessMarketDrivenFeesGroupWhenHasMarketDrivenFlight);
  CPPUNIT_TEST(testUpdateTravelParamsWhenMarketDrivenFlight);
  CPPUNIT_TEST(testUpdateTravelParamsWhenInterlineJourneyFlight);

  CPPUNIT_TEST(testFilterOutBaggageForAcs_NotACS);
  CPPUNIT_TEST(testFilterOutBaggageForAcs);

  CPPUNIT_TEST(testEMDAgreement_Pass);
  CPPUNIT_TEST(testEMDAgreement_Fail);
  CPPUNIT_TEST(testEMDAgreement_Pass_OperatingSameAsPartition);
  CPPUNIT_TEST(testEMDAgreement_Fail_OperatingSameAsPartition);
  CPPUNIT_TEST(testEMDAgreement_TN_Pass);
  CPPUNIT_TEST(testEMDAgreement_TN_Fail);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _request->ticketingDT() = DateTime(2010, 1, 1);
    _request->validatingCarrier() = "KL";

    _request->ticketingAgent() = _memHandle.create<Agent>();

    _trx->billing() = _memHandle.create<Billing>();
    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;
    _trx->itin().push_back(_itin);
    _sfg = _memHandle.create<MockServiceFeesGroup>();
    _itin->farePath().push_back(_farePath);

    _paxType = _memHandle.create<PaxType>();
    _options = _memHandle.create<PricingOptions>();

    _farePath->paxType() = _paxType;

    _memHandle.insert(_ofc = new MockOptionalFeeCollector(*_trx));
    _ofc->_farePath = _farePath;
    _ofc->setStrategy(_ofc->_marketingCarrierStrategy);

    _LOC_DFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _LOC_DEN = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDEN.xml");
    _LOC_CHI = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCHI.xml");
    _LOC_NYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _LOC_LON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    _trx->setOptions(_options);

    _subCode = _memHandle.create<SubCodeInfo>();
    _subCode->effDate() = DateTime(2009, 1, 1);
    _subCode->discDate() = DateTime(2011, 1, 1);
    _subCode->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    _subCode->industryCarrierInd() = OptionalFeeCollector::S5_INDCRXIND_INDUSTRY;
    _subCode->serviceSubTypeCode() = "0A5";

    _gValid = _memHandle.create<std::vector<ServiceGroup*> >();
    _gValid->push_back(serviceGroups);
    _gValid->push_back(serviceGroups + 1);
    _mcpInfo = _memHandle.create<MerchCarrierPreferenceInfo>();
    _s6Validator = _memHandle.insert(new OptionalFeeConcurValidator(*_trx, _farePath));
    _memHandle.create<MyDataHandle>();
    const std::string activationDate = "2000-01-01";
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE",
                                    activationDate,
                                    "EMD_ACTIVATION");
    TestConfigInitializer::setValue("EMD_VALIDATION_ENABLE_RELAXED",
                                    activationDate,
                                    "EMD_ACTIVATION");
    TimerTaskExecutor::setInstance(new PriorityQueueTimerTaskExecutor);
  }

  void tearDown()
  {
    _memHandle.clear();
    TimerTaskExecutor::destroyInstance();
  }

  void createOneSegmentItin(const CarrierCode& marketingCarrier, const CarrierCode& operatingCarrier)
  {
    AirSeg* air = _memHandle.create<AirSeg>();
    air->pnrSegment() = 1;
    air->segmentOrder() = 1;
    air->origin() = _LOC_DFW;
    air->origAirport() = _LOC_DFW->loc();
    air->destination() = _LOC_CHI;
    air->destAirport() = _LOC_CHI->loc();
    air->setMarketingCarrierCode(marketingCarrier);
    air->setOperatingCarrierCode(operatingCarrier);
    _trx->travelSeg().push_back(air);
    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());
    _itin->travelSeg().push_back(air);
    _ofc->_first = *_itin->travelSeg().begin();
    _ofc->_beginsOfUOT[0] = _itin->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[0] = _itin->travelSeg().begin();
    _ofc->_last = *(_itin->travelSeg().end() - 1);
    _ofc->_beginsOfUOT[1] = _itin->travelSeg().end();
    _ofc->_beginsOfLargestUOT[1] = _itin->travelSeg().end();
  }


  void createTwoStopItin(const Loc* stop1, const Loc* stop2, const Loc* stop3)
  {
    AirSeg* air1 = _memHandle.create<AirSeg>();
    air1->pnrSegment() = 1;
    air1->segmentOrder() = 1;
    air1->origin() = stop1;
    air1->origAirport() = stop1->loc();
    air1->destination() = stop2;
    air1->destAirport() = stop2->loc();
    air1->setMarketingCarrierCode("AA");
    air1->setOperatingCarrierCode("NW");

    AirSeg* air2 = _memHandle.create<AirSeg>();
    air2->pnrSegment() = 2;
    air2->segmentOrder() = 2;
    air2->origin() = stop2;
    air2->origAirport() = stop2->loc();
    air2->destination() = stop3;
    air2->destAirport() = stop3->loc();
    air2->setMarketingCarrierCode("AA");
    air2->setOperatingCarrierCode("KL");

    _trx->travelSeg().push_back(air1);
    _trx->travelSeg().push_back(air2);

    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());

    _itin->travelSeg().push_back(air1);
    _itin->travelSeg().push_back(air2);

    _ofc->_first = *_itin->travelSeg().begin();
    _ofc->_beginsOfUOT[0] = _itin->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[0] = _itin->travelSeg().begin();
    _ofc->_last = *(_itin->travelSeg().end() - 1);
    _ofc->_beginsOfUOT[1] = _itin->travelSeg().end();
    _ofc->_beginsOfLargestUOT[1] = _itin->travelSeg().end();
  }

  void createServiceFeesGroup(const ServiceGroup& serviceGroup, bool empty = true)
  {
    std::vector<OCFees*>* ocF = _memHandle.create<std::vector<OCFees*> >();
    if (!empty)
    {
      ocF->push_back(_memHandle.create<OCFees>());
      (*ocF)[0]->carrierCode() = "AA";
      (*ocF)[0]->travelStart() = _ofc->_first;
      (*ocF)[0]->travelEnd() = _ofc->_last;
      (*ocF)[0]->subCodeInfo() = _subCode;

      _sfg->ocFeesMap().insert(std::make_pair(_farePath, *ocF));
    }

    _sfg->groupCode() = serviceGroup;
    _sfg->groupDescription() = "serviceGroup";

    _itin->ocFeesGroup().push_back(_sfg);
  }

  void createDiag(DiagnosticTypes diagType = Diagnostic875, bool isDDINFO = false, bool isAVEMDIA = false)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      if (isDDINFO)
        _trx->diagnostic().diagParamMap().insert(
            std::make_pair(Diagnostic::DISPLAY_DETAIL, "INFO"));
      if (isAVEMDIA)
        _trx->diagnostic().diagParamMap().insert(
            std::make_pair(Diagnostic::ALL_VALID, "EMDIA"));
      _trx->diagnostic().activate();
    }
    _ofc->createDiag();
  }

  void insertCarrier(const CarrierCode& carrier, bool operating)
  {
    (operating ? _ofc->_operatingCxr : _ofc->_marketingCxr)[0].insert(carrier);
    _ofc->_cXrGrpOCF.insert(std::make_pair(carrier, *_gValid));
  }

  void createmerchCxrPrefInfo()
  {
    _mcpInfo->carrier() = "US";
    _mcpInfo->prefVendor() = "USOC";
    _mcpInfo->altProcessInd() = 'Y';
    _mcpInfo->groupCode() = "SA";
    _mcpInfo->sectorPortionInd() = 'S';
    _mcpInfo->concurrenceInd() = 'N';
  }

  // tests

  void testConstructor() { CPPUNIT_ASSERT_EQUAL(_trx, &_ofc->_trx); }

  void testCollect_EmptyFarePath()
  {
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createDiag();

    _ofc->collect();
    CPPUNIT_ASSERT_EQUAL(
        std::string::npos,
        _ofc->diag875()->rootDiag()->toString().find("PORTION OF TRAVEL : DFW 1  - LON 2"));
  }

  void testCollect()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createDiag();
    _itin->farePath().push_back(_farePath);

    _ofc->collect();
    CPPUNIT_ASSERT(_ofc->diag875()->rootDiag()->toString().find("PORTION OF TRAVEL : DFW 1  - LON 2") !=
                   std::string::npos);
  }

  void testDiag_875_printActiveMrktDrivenCxrHeader()
  {
    createDiag();
    CPPUNIT_ASSERT(_ofc->diag875());
    CPPUNIT_ASSERT(!_ofc->_diagInfo);
    _ofc->printActiveMrktDrivenCxrHeader();
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("MERCH CARRIER PREFERENCE DATA") !=
                   std::string::npos);
  }

  void testDiag_printActiveMrktDrivenNCxrNoData()
  {
    createDiag();
    CPPUNIT_ASSERT(_ofc->diag875());
    CPPUNIT_ASSERT(!_ofc->_diagInfo);
    _ofc->printActiveMrktDrivenNCxrNoData();
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("NO DATA FOUND") != std::string::npos);
  }

  void testDiag_printActiveMrktDrivenCxrData()
  {
    createDiag();

    std::vector<MerchCarrierPreferenceInfo*> mCxrPrefVec;
    createmerchCxrPrefInfo();
    mCxrPrefVec.push_back(_mcpInfo);

    CPPUNIT_ASSERT(_ofc->diag875());
    CPPUNIT_ASSERT(!_ofc->_diagInfo);
    _ofc->printActiveMrktDrivenCxrData(mCxrPrefVec);
    CPPUNIT_ASSERT(
        _ofc->diag875()->str().find("US     USOC      Y         SA           S             N") !=
        std::string::npos);
  }

  void testCreateDiagnostic_NoDiagnosticRequested()
  {
    createDiag(DiagnosticNone);
    CPPUNIT_ASSERT(!_ofc->diag875());
    CPPUNIT_ASSERT(!_ofc->diag877());
    CPPUNIT_ASSERT(!_ofc->_diag880);
  }

  void testCreateDiagnostic_875()
  {
    createDiag();
    CPPUNIT_ASSERT(_ofc->diag875());
    CPPUNIT_ASSERT(!_ofc->_diagInfo);
  }

  void testCreateDiagnostic_DDINFO()
  {

    createDiag(Diagnostic875, true);
    CPPUNIT_ASSERT(_ofc->_diagInfo);
  }

  void testCreateDiagnostic_877()
  {
    createDiag(Diagnostic877);
    CPPUNIT_ASSERT(_ofc->diag877());
  }

  void testCreateDiagnostic_880()
  {
    createDiag(Diagnostic880);
    CPPUNIT_ASSERT(_ofc->_diag880);
  }

  void testPrintDiagHeader()
  {
    createDiag();

    _farePath->paxType()->paxType() = "CNN";

    _ofc->_first = _memHandle.create<AirSeg>();
    _ofc->_last = _memHandle.create<AirSeg>();

    _ofc->_first->boardMultiCity() = "DFW";
    _ofc->_last->offMultiCity() = "NYC";
    insertCarrier("AA", true);
    insertCarrier("BB", false);
    _ofc->printDiagHeader(0);

    CPPUNIT_ASSERT(_ofc->diag875()->str().find("PORTION OF TRAVEL") != std::string::npos);
  }

  void testCheckFlightRelated_Pass()
  {
    _subCode->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    CPPUNIT_ASSERT(_ofc->checkFlightRelated(*_subCode));
  }

  void testCheckFlightRelated_Fail_Tkt()
  {
    _subCode->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT(!_ofc->checkFlightRelated(*_subCode));
  }

  void testCheckFlightRelated_Fail_Merch()
  {
    _subCode->fltTktMerchInd() = MERCHANDISE_SERVICE;
    CPPUNIT_ASSERT(!_ofc->checkFlightRelated(*_subCode));
  }

  void testCheckFlightRelated_Fail_Empty()
  {
    _subCode->fltTktMerchInd() = 0;
    CPPUNIT_ASSERT(!_ofc->checkFlightRelated(*_subCode));
  }

  void testSetJourneyDestination_OW_Int()
  {
    _ofc->_isRoundTripOCF.insert(std::make_pair(_ofc->_farePath->itin(), false));
    _ofc->_isInternationalOCF.insert(std::make_pair(_ofc->_farePath->itin(), true));
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->setJourneyDestination();

    CPPUNIT_ASSERT_EQUAL(_LOC_LON->loc(), _ofc->_journeyDestination->loc());
    CPPUNIT_ASSERT(_itin->travelSeg().end() == _ofc->_beginsOfUOT[1]);
  }

  void testSetJourneyDestination_OW_Dom()
  {
    _ofc->_isRoundTripOCF.insert(std::make_pair(_ofc->_farePath->itin(), false));
    _ofc->_isInternationalOCF.insert(std::make_pair(_ofc->_farePath->itin(), false));
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_DEN);
    _ofc->setJourneyDestination();

    CPPUNIT_ASSERT_EQUAL(_LOC_DEN->loc(), _ofc->_journeyDestination->loc());
    CPPUNIT_ASSERT(_itin->travelSeg().end() == _ofc->_beginsOfUOT[1]);
  }

  void testSetJourneyDestination_RT_Int()
  {
    _ofc->_isRoundTripOCF.insert(std::make_pair(_ofc->_farePath->itin(), true));
    _ofc->_isInternationalOCF.insert(std::make_pair(_ofc->_farePath->itin(), true));
    createTwoStopItin(_LOC_DFW, _LOC_LON, _LOC_CHI);
    _ofc->setJourneyDestination();

    CPPUNIT_ASSERT_EQUAL(_LOC_LON->loc(), _ofc->_journeyDestination->loc());
    CPPUNIT_ASSERT_EQUAL(_LOC_LON->loc(), (*_ofc->_beginsOfUOT[1])->origin()->loc());
  }

  void testSetJourneyDestination_RT_Dom()
  {
    _ofc->_isRoundTripOCF.insert(std::make_pair(_ofc->_farePath->itin(), true));
    _ofc->_isInternationalOCF.insert(std::make_pair(_ofc->_farePath->itin(), false));
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_DFW);
    _ofc->setJourneyDestination();

    CPPUNIT_ASSERT_EQUAL(_LOC_CHI->loc(), _ofc->_journeyDestination->loc());
    CPPUNIT_ASSERT_EQUAL(_LOC_CHI->loc(), (*_ofc->_beginsOfUOT[1])->origin()->loc());
  }

  void testDefineMarketingOperatingCarriers_OneUnit()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfUOT[0] = _itin->travelSeg().begin();
    _ofc->_beginsOfUOT[1] = _itin->travelSeg().end();
    _ofc->_beginsOfUOT[2] = _itin->travelSeg().end();

    _ofc->defineMarketingOperatingCarriers();

    CPPUNIT_ASSERT_EQUAL(2, (int)_ofc->_operatingCxr[0].size());
    CPPUNIT_ASSERT_EQUAL(0, (int)_ofc->_operatingCxr[1].size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ofc->_marketingCxr[0].size());
    CPPUNIT_ASSERT_EQUAL(0, (int)_ofc->_marketingCxr[1].size());
  }

  void testDefineMarketingOperatingCarriers_TwoUnits()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_DFW);
    _ofc->_beginsOfUOT[0] = _itin->travelSeg().begin();
    _ofc->_beginsOfUOT[1] = _itin->travelSeg().begin() + 1;
    _ofc->_beginsOfUOT[2] = _itin->travelSeg().end();

    _ofc->defineMarketingOperatingCarriers();

    CPPUNIT_ASSERT_EQUAL(1, (int)_ofc->_operatingCxr[0].size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ofc->_operatingCxr[1].size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ofc->_marketingCxr[0].size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ofc->_marketingCxr[1].size());
  }

  void testAddNewServiceGroup()
  {
    ServiceGroup srvGroup = "ML";

    CPPUNIT_ASSERT_EQUAL(srvGroup, _ofc->addNewServiceGroup(srvGroup)->groupCode());
  }

  void testIdentifyGroupCodesToBeProcessedWhenMultipleItins()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML, requestedOcFeeGroupRO;
    requestedOcFeeGroupML.groupCode() = "ML";
    requestedOcFeeGroupRO.groupCode() = "RO";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);
    _options->serviceGroupsVec().push_back(requestedOcFeeGroupRO);

    Itin itin2;
    _trx->itin().push_back(&itin2);
    _trx->ticketingDate() = DateTime(2010, 2, 17);

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    std::vector<ServiceGroup> groupNP; // Group codes not active

    _ofc->identifyGroupCodesToBeProcessed(groupNV, groupNA, groupNP);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->ocFeesGroup().size());
    // CPPUNIT_ASSERT_EQUAL(requestedOcFeeGroupML.groupCode(),
    // _itin->ocFeesGroup().front()->groupCode());

    CPPUNIT_ASSERT_EQUAL(size_t(2), itin2.ocFeesGroup().size());
    // CPPUNIT_ASSERT_EQUAL(requestedOcFeeGroupML.groupCode(),
    // itin2.ocFeesGroup().front()->groupCode());
  }

  void testProcessSubCodes_Fail_S5NotFound()
  {
    createServiceFeesGroup(ServiceGroup("ML"));
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                       .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) ==
                   _sfg->subCodeMap(0).end());
  }

  void testProcessSubCodes_Fail_S5Found()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("XX"), 0, true);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                       .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) ==
                   _sfg->subCodeMap(0).end());
  }

  void testProcessSubCodes_Pass()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                       .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) !=
                   _sfg->subCodeMap(0).end());
  }

  void testCheckIndCrxIndWhenOperatingStrategyAndCarrierIndicator()
  {
    Indicator indCrxInd = OptionalFeeCollector::S5_INDCRXIND_CARRIER;
    _ofc->setStrategy(_ofc->_operatingCarrierStrategy);

    CPPUNIT_ASSERT(_ofc->checkIndCrxInd(indCrxInd));
  }

  void testCheckIndCrxIndWhenMarketingStrategyAndCarrierIndicator()
  {
    Indicator indCrxInd = OptionalFeeCollector::S5_INDCRXIND_CARRIER;
    _ofc->setStrategy(_ofc->_marketingCarrierStrategy);

    CPPUNIT_ASSERT(!_ofc->checkIndCrxInd(indCrxInd));
  }

  void testCheckIndCrxIndWhenOperatingStrategyAndIndustryIndicator()
  {
    Indicator indCrxInd = OptionalFeeCollector::S5_INDCRXIND_INDUSTRY;
    _ofc->setStrategy(_ofc->_operatingCarrierStrategy);

    CPPUNIT_ASSERT(_ofc->checkIndCrxInd(indCrxInd));
  }

  void testCheckIndCrxIndWhenMarketingStrategyAndIndustryIndicator()
  {
    Indicator indCrxInd = OptionalFeeCollector::S5_INDCRXIND_INDUSTRY;
    _ofc->setStrategy(_ofc->_marketingCarrierStrategy);

    CPPUNIT_ASSERT(_ofc->checkIndCrxInd(indCrxInd));
  }

  void testProcessServiceFeesGroups_SingleMarketingCxr()
  {
    insertCarrier("AA", false);
    createServiceFeesGroup(ServiceGroup("ML"));
    _ofc->processServiceFeesGroups(0);
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), _ofc->_result);
  }

  void testProcessServiceFeesGroups_SingleOpratingCxr()
  {
    insertCarrier("BB", true);
    createServiceFeesGroup(ServiceGroup("ML"));
    _ofc->processServiceFeesGroups(0);
    CPPUNIT_ASSERT_EQUAL(std::string("BB"), _ofc->_result);
  }
  void testProcessServiceFeesGroup_DiffOperAndMarketing_PassMarkting()
  {
    insertCarrier("AA", false);
    insertCarrier("BB", true);
    createServiceFeesGroup(ServiceGroup("ML"));
    MockOptionalFeeConcurValidator* s6validator =
        _memHandle.insert(new MockOptionalFeeConcurValidator(*_trx, _farePath));
    s6validator->_cxrPass = "AA";
    _ofc->processServiceFeesGroup(_sfg, 0, s6validator);
    CPPUNIT_ASSERT_EQUAL(std::string("CXR-AA GRP-ML"), s6validator->_result);
  }
  void testProcessServiceFeesGroup_DiffOperAndMarketing_NoOperating()
  {
    insertCarrier("AA", false);
    insertCarrier("BB", true);
    createServiceFeesGroup(ServiceGroup("ML"));
    MockOptionalFeeConcurValidator* s6validator =
        _memHandle.insert(new MockOptionalFeeConcurValidator(*_trx, _farePath));
    s6validator->_cxrPass = "BB";
    _ofc->processServiceFeesGroup(_sfg, 0, s6validator);
    CPPUNIT_ASSERT_EQUAL(std::string(""),
                         s6validator->_result); // no s6 validation for single operating
  }
  void testProcessServiceFeesGroup_MultiOperating()
  {
    insertCarrier("AA", false);
    insertCarrier("BB", true);
    insertCarrier("CC", true);
    insertCarrier("DD", true);
    createServiceFeesGroup(ServiceGroup("ML"));
    MockOptionalFeeConcurValidator* s6validator =
        _memHandle.insert(new MockOptionalFeeConcurValidator(*_trx, _farePath));
    s6validator->_cxrPass = "CC";
    _ofc->processServiceFeesGroup(_sfg, 0, s6validator);
    CPPUNIT_ASSERT_EQUAL(std::string("CXR-CC GRP-ML"), s6validator->_result);
  }

  void testProcessServiceFeesGroups_MultipleOperatingCxr()
  {
    insertCarrier("AA", false);
    insertCarrier("BB", false);
    insertCarrier("CC", true);
    insertCarrier("DD", true);
    createServiceFeesGroup(ServiceGroup("ML"));
    std::vector<ServiceFeesGroup*>::const_iterator sg = _itin->ocFeesGroup().begin();
    _ofc->processServiceFeesGroup(*sg, _ofc->_operatingCxr[0], 0);
    _ofc->processServiceFeesGroup(*sg, _ofc->_operatingCxr[1], 0);
    CPPUNIT_ASSERT_EQUAL(std::string("CCDD"), _ofc->_result);
  }

  void testProcessServiceFeesGroups_PassMerchActivation()
  {
    insertCarrier("AA", false);
    createServiceFeesGroup(ServiceGroup("ML"));
    ((MockOptionalFeeCollector*)_ofc)->_merchActValidate = true;
    _ofc->processServiceFeesGroups(0);
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), _ofc->_result);
  }

  void testProcessServiceFeesGroups_FailMerchActivation()
  {
    insertCarrier("AA", false);
    createServiceFeesGroup(ServiceGroup("ML"));
    ((MockOptionalFeeCollector*)_ofc)->_merchActValidate = false;
    _ofc->_cXrGrpOCF.clear();
    _ofc->processServiceFeesGroups(0);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _ofc->_result);
  }

  void testCheckDisplayCategoryWhenSaleInStore()
  {
    ServiceDisplayInd displayCat = "02";
    CPPUNIT_ASSERT(!_ofc->checkDisplayCategory(displayCat));
  }

  void testCheckDisplayCategoryWhenNotSaleInStore()
  {
    ServiceDisplayInd displayCat = "01";
    CPPUNIT_ASSERT(_ofc->checkDisplayCategory(displayCat));
  }

  void testCheckEMDTypeWhenElectronicTkt()
  {
    Indicator emdType = '5';
    CPPUNIT_ASSERT(!_ofc->checkEMDType(emdType));
  }

  void testCheckEMDTypeWhenNotElectronicTkt()
  {
    Indicator emdType = '1';
    CPPUNIT_ASSERT(_ofc->checkEMDType(emdType));
  }

  void testIsAgencyActiveWhenNoDeactivatedAgency()
  {
    ((MockOptionalFeeCollector*)_ofc)->_mai2.includeInd() = 'Y';
    CPPUNIT_ASSERT(_ofc->isAgencyActive());
  }

  void testIsAgencyActiveWhenAgencyDeactivated()
  {
    ((MockOptionalFeeCollector*)_ofc)->_mai2.includeInd() = 'N';
    CPPUNIT_ASSERT(!_ofc->isAgencyActive());
  }

  void testFilterOutInputGroupCodesAllGroupCodesAreValid()
  {
    std::vector<ServiceGroup> gC;
    std::vector<ServiceGroup> gActive;
    std::vector<ServiceGroup> gCInv;
    std::vector<ServiceGroup> gCInvTkt;
    gC.push_back("BG");
    gC.push_back("ML");
    _trx->ticketingDate() = DateTime(2010, 2, 17);
    _ofc->getAllATPGroupCodes();
    _ofc->filterOutInputGroupCodes(gC, gActive, gCInv, gCInvTkt);
    CPPUNIT_ASSERT(!gActive.empty());
    CPPUNIT_ASSERT(gCInv.empty());
    CPPUNIT_ASSERT(gCInvTkt.empty());
  }

  void testFilterOutInputGroupCodesRequestedGroupCodesIsInvalid()
  {
    std::vector<ServiceGroup> gC;
    std::vector<ServiceGroup> gActive;
    std::vector<ServiceGroup> gCInv;
    std::vector<ServiceGroup> gCInvTkt;
    gC.push_back("MM");
    _trx->ticketingDate() = DateTime(2010, 2, 17);
    _ofc->getAllATPGroupCodes();
    _ofc->filterOutInputGroupCodes(gC, gActive, gCInv, gCInvTkt);
    CPPUNIT_ASSERT(gActive.empty());
    CPPUNIT_ASSERT(!gCInv.empty());
    CPPUNIT_ASSERT(gCInvTkt.empty());
  }

  void testFilterOutInputGroupCodesRequestedGroupCodesIsValidAndInvalid()
  {
    std::vector<ServiceGroup> gC;
    std::vector<ServiceGroup> gActive;
    std::vector<ServiceGroup> gCInv;
    std::vector<ServiceGroup> gCInvTkt;
    gC.push_back("ML");
    gC.push_back("MM");
    _trx->ticketingDate() = DateTime(2010, 2, 17);
    _ofc->getAllATPGroupCodes();
    _ofc->filterOutInputGroupCodes(gC, gActive, gCInv, gCInvTkt);
    CPPUNIT_ASSERT(!gActive.empty());
    CPPUNIT_ASSERT(!gCInv.empty());
    CPPUNIT_ASSERT(gCInvTkt.empty());
  }

  void testFilterOutInputGroupCodesRequestedGroupCodesIsInvalidForTktDate()
  {
    std::vector<ServiceGroup> gC;
    std::vector<ServiceGroup> gActive;
    std::vector<ServiceGroup> gCInv;
    std::vector<ServiceGroup> gCInvTkt;
    gC.push_back("ML");
    _trx->ticketingDate() = DateTime(2010, 5, 10);
    _ofc->getAllATPGroupCodes();
    _ofc->filterOutInputGroupCodes(gC, gActive, gCInv, gCInvTkt);
    CPPUNIT_ASSERT(gActive.empty());
    CPPUNIT_ASSERT(gCInv.empty());
    CPPUNIT_ASSERT(!gCInvTkt.empty());
  }

  void testFilterOutInputGroupCodesRequestedOneInvalidAndOneIsInvalidForTktDate()
  {
    std::vector<ServiceGroup> gC;
    std::vector<ServiceGroup> gActive;
    std::vector<ServiceGroup> gCInv;
    std::vector<ServiceGroup> gCInvTkt;
    gC.push_back("MM");
    gC.push_back("ML");
    _trx->ticketingDate() = DateTime(2010, 5, 10);
    _ofc->getAllATPGroupCodes();
    _ofc->filterOutInputGroupCodes(gC, gActive, gCInv, gCInvTkt);
    CPPUNIT_ASSERT(gActive.empty());
    CPPUNIT_ASSERT(!gCInv.empty());
    CPPUNIT_ASSERT(!gCInvTkt.empty());
  }

  void testIsATPActiveGroupFoundReturnTrue()
  {
    std::vector<ServiceGroup> gCInvTkt;
    bool ind = false;
    _trx->ticketingDate() = DateTime(2010, 2, 17);
    _ofc->getAllATPGroupCodes();

    CPPUNIT_ASSERT(_ofc->isATPActiveGroupFound("ML", ind, gCInvTkt));
    CPPUNIT_ASSERT(!ind);
    CPPUNIT_ASSERT(gCInvTkt.empty());
  }

  void testIsATPActiveGroupFoundReturnFalse_NotFound()
  {
    std::vector<ServiceGroup> gCInvTkt;
    bool ind = false;
    _trx->ticketingDate() = DateTime(2010, 2, 17);
    _ofc->getAllATPGroupCodes();

    CPPUNIT_ASSERT(!_ofc->isATPActiveGroupFound("UN", ind, gCInvTkt));
    CPPUNIT_ASSERT(!ind);
    CPPUNIT_ASSERT(gCInvTkt.empty());
  }

  void testIsATPActiveGroupFoundReturnTrue_InvalidDate()
  {
    std::vector<ServiceGroup> gCInvTkt;
    bool ind = false;
    _trx->ticketingDate() = DateTime(2010, 5, 10);
    _ofc->getAllATPGroupCodes();

    CPPUNIT_ASSERT(_ofc->isATPActiveGroupFound("ML", ind, gCInvTkt));
    CPPUNIT_ASSERT(ind);
    CPPUNIT_ASSERT(!gCInvTkt.empty());
  }

  void testSetUpIndForShoppingWhenItIsPricingReturnTrue()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(_ofc->setsUpIndForShopping());
    CPPUNIT_ASSERT(!_ofc->_shopping);
  }

  void testSetUpIndForShoppingWhenItIsMIPNoGroupInTheEntryReturnFalse()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(!_ofc->setsUpIndForShopping());
    CPPUNIT_ASSERT(!_ofc->_shopping);
  }

  void testSetUpIndForShoppingWhenItIsMIPMaxNumberEqualZeroReturnFalse()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML;
    RequestedOcFeeGroup requestedOcFeeGroupBG;

    requestedOcFeeGroupML.groupCode() = "ML";
    requestedOcFeeGroupBG.groupCode() = "BG";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);
    _options->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->getOptions()->maxNumberOfOcFeesForItin() = 0;
    CPPUNIT_ASSERT(!_ofc->setsUpIndForShopping());
    CPPUNIT_ASSERT(!_ofc->_shopping);
  }
  void testSetUpIndForShoppingWhenItIsMIPMaxNumberEqualNoLimitReturnFalse()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML;
    RequestedOcFeeGroup requestedOcFeeGroupBG;

    requestedOcFeeGroupML.groupCode() = "ML";
    requestedOcFeeGroupBG.groupCode() = "BG";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);
    _options->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->getOptions()->maxNumberOfOcFeesForItin() = -1;
    CPPUNIT_ASSERT(_ofc->setsUpIndForShopping());
    CPPUNIT_ASSERT(!_ofc->_shopping);
  }

  void testSetUpIndForShoppingWhenItIsMIPAndMaxNumberReturnTrue()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML;
    RequestedOcFeeGroup requestedOcFeeGroupBG;

    requestedOcFeeGroupML.groupCode() = "ML";
    requestedOcFeeGroupBG.groupCode() = "BG";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);
    _options->serviceGroupsVec().push_back(requestedOcFeeGroupBG);

    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->getOptions()->maxNumberOfOcFeesForItin() = 5;
    CPPUNIT_ASSERT(_ofc->setsUpIndForShopping());
    CPPUNIT_ASSERT(_ofc->_shopping);
  }

  void testCheckNumbersToStopProcessingNoStopNeededReturnTrue()
  {
    CPPUNIT_ASSERT(_ofc->checkNumbersToStopProcessing(true));
    CPPUNIT_ASSERT(!_ofc->_stopMatchProcess);
  }

  void testCheckNumbersToStopProcessingForPricingWhenNoGroupRequestedReturnFalse()
  {
    _ofc->_needFirstMatchOnlyOCF = true;
    CPPUNIT_ASSERT(!_ofc->checkNumbersToStopProcessing(true));
    CPPUNIT_ASSERT(_ofc->_stopMatchProcess);
  }

  void testCheckNumbersToStopProcessingForShoppingWhenNumberPassedOptionsLessMaxNumberReturnTrue()
  {
    _ofc->_shoppingOCF = true;
    _trx->getOptions()->maxNumberOfOcFeesForItin() = 5;
    CPPUNIT_ASSERT(_ofc->checkNumbersToStopProcessing(true));
    CPPUNIT_ASSERT(!_ofc->_stopMatchProcess);
  }

  void
  testCheckNumbersToStopProcessingForShoppingWhenNumberPassedOptionsIsMaxNumberPlusOneReturnFalse()
  {
    _ofc->_shoppingOCF = true;
    _trx->getOptions()->maxNumberOfOcFeesForItin() = 5;
    _ofc->_numberOfOcFeesForItin = 5;
    CPPUNIT_ASSERT(!_ofc->checkNumbersToStopProcessing(true));
    CPPUNIT_ASSERT(_ofc->_stopMatchProcess);
  }

  void testFinalCheckShoppingPricingForStopProcessingNoStopRequiredReturnTrue()
  {
    CPPUNIT_ASSERT(_ofc->finalCheckShoppingPricing(*_itin));
  }

  void testFinalCheckShoppingPricingForStopProcessingForPricingReturnFalse()
  {
    _ofc->_stopMatchProcess = true;
    CPPUNIT_ASSERT(!_ofc->finalCheckShoppingPricing(*_itin));
  }

  void testFinalCheckShoppingPricingForStopProcessingForShoppingReturnFalse()
  {
    _ofc->_shoppingOCF = true;
    _ofc->_stopMatchProcess = true;
    CPPUNIT_ASSERT(!_ofc->finalCheckShoppingPricing(*_itin));
    CPPUNIT_ASSERT(_itin->moreFeesAvailable());
  }

  void testGetAllActiveGroupCodesforSABRE_USER()
  {
    _trx->ticketingDate() = DateTime(2010, 3, 3);
    _ofc->getAllATPGroupCodes();

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> gValid;
    _ofc->getAllActiveGroupCodes(gValid);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(4), gValid.size());
  }

  void testGetAllActiveGroupCodesforAXESS_USER()
  {
    _trx->ticketingDate() = DateTime(2010, 3, 3);
    _ofc->getAllATPGroupCodes();

    Customer customer;
    customer.crsCarrier() = "1J";
    customer.hostName() = "AXES";
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> gValid;
    _ofc->getAllActiveGroupCodes(gValid);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(4), gValid.size());
  }

  void testGetAllActiveGroupCodesforABACUS_USER()
  {
    _trx->ticketingDate() = DateTime(2010, 3, 3);
    _ofc->getAllATPGroupCodes();
    TrxUtil::enableAbacus();
    Customer customer;
    customer.crsCarrier() = "1B";
    customer.hostName() = "ABAC";
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> gValid;
    _ofc->getAllActiveGroupCodes(gValid);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(4), gValid.size());
  }

  void test_sortOCFeeGroup_Empty()
  {
    std::vector<ServiceGroup> gValid;
    _ofc->sortOCFeeGroup(gValid);

    CPPUNIT_ASSERT(gValid.empty());
  }

  void test_sortOCFeeGroup_Sorted()
  {
    std::vector<ServiceGroup> gValid;
    gValid.push_back("UN");
    ServiceGroup sg = "ML";
    gValid.push_back(sg);
    _ofc->sortOCFeeGroup(gValid);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), gValid.size());
    CPPUNIT_ASSERT_EQUAL(sg, gValid[0]);
  }

  void test_sortOCFeeGroup_Sorted_Shopping()
  {
    _ofc->_shoppingOCF = true;
    std::vector<ServiceGroup> gValid;
    gValid.push_back("UN");
    ServiceGroup sg = "ML";
    gValid.push_back(sg);
    _ofc->sortOCFeeGroup(gValid);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), gValid.size());
    CPPUNIT_ASSERT_EQUAL(sg, gValid[0]);
  }

  void test_getGroupCodesNotProcessedForTicketingDate()
  {
    std::vector<ServiceGroup> gValid;
    ServiceGroup sg = "UN";
    gValid.push_back(sg);
    std::vector<ServiceGroup> grNotValidForTicketDate;
    std::vector<ServiceGroup> grNotProcessedForTicketDate;
    _trx->ticketingDate() = DateTime(2010, 8, 13);
    _ofc->getAllATPGroupCodes();
    _ofc->getGroupCodesNotProcessedForTicketingDate(
        gValid, grNotValidForTicketDate, grNotProcessedForTicketDate);

    CPPUNIT_ASSERT(!grNotProcessedForTicketDate.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), grNotProcessedForTicketDate.size());
    ServiceGroup sg1 = "ML";
    CPPUNIT_ASSERT_EQUAL(sg1, grNotProcessedForTicketDate[1]);
    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(1), gValid.size());
    CPPUNIT_ASSERT_EQUAL(sg, gValid[0]);
  }

  void test_getGroupCodesNotProcessedForTicketingDate_Test()
  {
    std::vector<ServiceGroup> gValid;
    ServiceGroup sg = "UN";
    gValid.push_back(sg);
    std::vector<ServiceGroup> grNotValidForTicketDate;
    std::vector<ServiceGroup> grNotProcessedForTicketDate;
    grNotProcessedForTicketDate.push_back("ML");
    _trx->ticketingDate() = DateTime(2010, 8, 13);
    _ofc->getAllATPGroupCodes();
    _ofc->getGroupCodesNotProcessedForTicketingDate(
        gValid, grNotValidForTicketDate, grNotProcessedForTicketDate);

    CPPUNIT_ASSERT(!grNotProcessedForTicketDate.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), grNotProcessedForTicketDate.size());
    ServiceGroup sg1 = "ML";
    CPPUNIT_ASSERT_EQUAL(sg1, grNotProcessedForTicketDate[1]);
    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(1), gValid.size());
    CPPUNIT_ASSERT_EQUAL(sg, gValid[0]);
  }

  void testSelectActiveNotActiveGroupCodesReturnActiveAndNotActive()
  {
    _trx->ticketingDate() = DateTime(2010, 3, 3);
    _ofc->getAllATPGroupCodes();

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> gValid;
    std::vector<ServiceGroup> gCInvTkt;
    gValid.push_back("ML");
    gValid.push_back("UN");
    gValid.push_back("BG");
    std::vector<ServiceGroup> gCNotPrc;
    _ofc->selectActiveNotActiveGroupCodes(gValid, gCInvTkt, gCNotPrc);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), gValid.size());
    CPPUNIT_ASSERT(!gCInvTkt.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(1), gCInvTkt.size());
  }

  void testSelectActiveNotActiveGroupCodesReturnsAllRequestedAsActive()
  {
    _trx->ticketingDate() = DateTime(2010, 3, 3);
    _ofc->getAllATPGroupCodes();

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    std::vector<ServiceGroup> gValid;
    std::vector<ServiceGroup> gCInvTkt;
    std::vector<ServiceGroup> gCNA;
    gValid.push_back("ML");
    gValid.push_back("BG");
    _ofc->selectActiveNotActiveGroupCodes(gValid, gCInvTkt, gCNA);

    CPPUNIT_ASSERT(!gValid.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), gValid.size());
    CPPUNIT_ASSERT(gCInvTkt.empty());
    CPPUNIT_ASSERT_EQUAL(size_t(0), gCInvTkt.size());
  }

  void testCleanUpOCFeesNoValidOCFeesReturn()
  {
    createOCFeesBeforeCleanUp(3);
    _ofc->cleanUpOCFees(*_itin);

    int total = 0;

    CPPUNIT_ASSERT_EQUAL(0, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(false, _itin->isOcFeesFound());
  }

  void testCleanUpOCFeesOneValidOCFeesReturn()
  {
    createOCFeesBeforeCleanUp(3, true);
    _ofc->cleanUpOCFees(*_itin);

    int total = 0;

    CPPUNIT_ASSERT_EQUAL(1, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(true, _itin->isOcFeesFound());
  }

  void testCleanUpOCFeesTWOValidOCFeesReturn()
  {
    createOCFeesBeforeCleanUp(4, true);
    _ofc->cleanUpOCFees(*_itin);

    int total = 0;

    CPPUNIT_ASSERT_EQUAL(2, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(true, _itin->isOcFeesFound());
  }

  void testCleanUpOCFeesAllValidOCFeesReturn()
  {
    createOCFeesBeforeCleanUp(3, true, true);
    _ofc->cleanUpOCFees(*_itin);

    int total = 0;

    CPPUNIT_ASSERT_EQUAL(3, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(true, _itin->isOcFeesFound());
  }

  int collectOCFeesAfterCleanUp(int& total)
  {
    typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
    for (SrvFeesGrpI srvFeesGrpI = _itin->ocFeesGroup().begin();
         srvFeesGrpI != _itin->ocFeesGroup().end();
         ++srvFeesGrpI)
    {
      ServiceFeesGroup* sfG = *srvFeesGrpI;
      std::map<const FarePath*, std::vector<OCFees*> >::iterator mI;
      for (mI = sfG->ocFeesMap().begin(); mI != sfG->ocFeesMap().end(); ++mI)
      {
        std::vector<OCFees*>& vecOCFees = mI->second;
        total += vecOCFees.size();
      }
    }
    return total;
  }

  void createOCFeesBeforeCleanUp(int numberOfOCFees, bool oc = false, bool even = false)
  {
    std::vector<OCFees*>* ocF = _memHandle.create<std::vector<OCFees*> >();
    for (int i = 0; i < numberOfOCFees; ++i)
    {
      ocF->push_back(_memHandle.create<OCFees>());
      (*ocF)[i]->carrierCode() = "AA";
      (*ocF)[i]->travelStart() = _ofc->_first;
      (*ocF)[i]->travelEnd() = _ofc->_last;
      (*ocF)[i]->subCodeInfo() = _subCode;
      if (oc)
      {
        if (even || i / 2 != 0)
        {
          OptionalServicesInfo* s7pointer = _memHandle.create<OptionalServicesInfo>();
          (*ocF)[i]->optFee() = s7pointer;
        }
      }
    }

    _sfg->groupCode() = "BG";
    _sfg->groupDescription() = "serviceGroup";
    _sfg->ocFeesMap().insert(std::make_pair(_farePath, *ocF));

    _itin->ocFeesGroup().push_back(_sfg);
  }

  void testIsSubCodeProcessedWhenSubCodeProcessedAfterMarketingCxrStep()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);

    ServiceFeesGroup srvFeesGrp;
    srvFeesGrp.subCodeMap(0)[std::make_tuple(
        _farePath, _subCode->serviceSubTypeCode(), 'F')][std::make_pair(_ofc->_first, _ofc->_last)];
    CPPUNIT_ASSERT(_ofc->isSubCodePassed(srvFeesGrp, 0, _subCode->serviceSubTypeCode(), 'F'));
  }

  void testIsSubCodeProcessedWhenSubCodeNotProcessedAfterMarketingCxrStep()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);

    ServiceFeesGroup srvFeesGrp;
    ServiceSubTypeCode subCode = "0XF";
    srvFeesGrp.subCodeMap(
        0)[std::make_tuple(_farePath, subCode, 'F')][std::make_pair(_ofc->_first, _ofc->_last)];
    CPPUNIT_ASSERT(!_ofc->isSubCodePassed(srvFeesGrp, 0, _subCode->serviceSubTypeCode(), 'F'));
  }

  void testIsSubCodeProcessedWhenSubCodeProcessedAfterSliceAndDiceMarketingCxrStep()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_last = *(_itin->travelSeg().end() - 2);

    ServiceFeesGroup srvFeesGrp;
    srvFeesGrp.subCodeMap(0)[std::make_tuple(
        _farePath, _subCode->serviceSubTypeCode(), 'F')][std::make_pair(_ofc->_first, _ofc->_last)];
    CPPUNIT_ASSERT(_ofc->isSubCodePassed(srvFeesGrp, 0, _subCode->serviceSubTypeCode(), 'F'));
  }

  void testIsSubCodeProcessedWhenSubCodeNotProcessedAfterSliceAndDiceMarketingCxrStep()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_last = *(_itin->travelSeg().end() - 2);

    ServiceFeesGroup srvFeesGrp;
    ServiceSubTypeCode subCode = "0XF";
    srvFeesGrp.subCodeMap(
        0)[std::make_tuple(_farePath, subCode, 'F')][std::make_pair(_ofc->_first, _ofc->_last)];
    CPPUNIT_ASSERT(!_ofc->isSubCodePassed(srvFeesGrp, 0, _subCode->serviceSubTypeCode(), 'F'));
  }

  void testIsSubCodeProcessedWhenSubCodeProcessedAfterLargestPortionOfTravelStep()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_last = *(_itin->travelSeg().end() - 2);

    ServiceFeesGroup srvFeesGrp;
    srvFeesGrp.subCodeMap(0)[std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')]
                            [std::make_pair(_ofc->_first, *(_itin->travelSeg().end() - 1))];
    CPPUNIT_ASSERT(_ofc->isSubCodePassed(srvFeesGrp, 0, _subCode->serviceSubTypeCode(), 'F'));
  }

  void testProcessSliceAndDicePortion()
  {
    std::vector<TravelSeg*> travel;
    AirSeg seg1, seg2;
    travel.push_back(&seg1);
    travel.push_back(&seg2);
    std::pair<std::vector<TravelSeg*>::const_iterator, std::vector<TravelSeg*>::const_iterator>
    route(travel.begin(), travel.end());

    _ofc->processSliceAndDicePortion(0, route);

    CPPUNIT_ASSERT(travel.begin() == _ofc->_beginsOfUOT[0]);
    CPPUNIT_ASSERT(travel.end() == _ofc->_beginsOfUOT[1]);
  }

  void testProcessLargestPortionOfTvl_Selected()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createDiag();
    _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::FARE_MARKET, "DFWCHI"));

    _ofc->processLargestPortionOfTvl(0);
    CPPUNIT_ASSERT_EQUAL(std::string::npos,
                         _ofc->diag875()->str().find("PORTION OF TRAVEL : DFW 1  - LON 2"));
  }

  void testProcessLargestPortionOfTvl_NotSelected()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createDiag();

    _ofc->processLargestPortionOfTvl(0);
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("PORTION OF TRAVEL : DFW 1  - LON 2") !=
                   std::string::npos);
  }

  void testProcessCarrierMMRecords_Retrieved()
  {
    createDiag();
    insertCarrier("AA", true);

    _ofc->processCarrierMMRecords();
    CPPUNIT_ASSERT_EQUAL(std::string::npos,
                         _ofc->diag875()->str().find("NO CARRIERS ACTIVE FOR OC PROCESSING"));
  }

  void testProcessCarrierMMRecords_NotRetrieved()
  {
    createDiag();
    insertCarrier("AA", true);
    ((MockOptionalFeeCollector*)_ofc)->_merchRetriveData = false;

    _ofc->processCarrierMMRecords();
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("NO CARRIERS ACTIVE FOR OC PROCESSING") ==
                   std::string::npos);
  }

  void testValidateS5Data_Fail_TktDate()
  {
    _request->ticketingDT() = DateTime(2011, 12, 31);
    CPPUNIT_ASSERT_EQUAL(FAIL_S5_TKT_DATE, _ofc->validateS5Data(_subCode));
  }

  void testValidateS5Data_Fail_FlightRelated()
  {
    _subCode->fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S5_FLIGHT_RELATED, _ofc->validateS5Data(_subCode));
  }

  void testValidateS5Data_Fail_Industry()
  {
    _subCode->industryCarrierInd() = OptionalFeeCollector::S5_INDCRXIND_CARRIER;
    CPPUNIT_ASSERT_EQUAL(FAIL_S5_INDUSTRY_INDICATOR, _ofc->validateS5Data(_subCode));
  }

  void testValidateS5Data_Fail_Category()
  {
    _subCode->displayCat() = OptionalFeeCollector::DISPLAY_CAT_STORE;
    CPPUNIT_ASSERT_EQUAL(FAIL_S5_DISPLAY_CATEGORY, _ofc->validateS5Data(_subCode));
  }

  void testValidateS5Data_Fail_Emd()
  {
    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ELECTRONIC_TKT;
    CPPUNIT_ASSERT_EQUAL(FAIL_S5_EMD_TYPE, _ofc->validateS5Data(_subCode));
  }

  void testValidateS5Data_Pass() { CPPUNIT_ASSERT_EQUAL(PASS_S5, _ofc->validateS5Data(_subCode)); }

  void testCheckDiagForS5Detail()
  {
    createDiag();
    _ofc->checkDiagForS5Detail(_subCode);
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("SERVICE FEE DIAGNOSTIC") != std::string::npos);
  }

  void testCheckDiagForS5Detail_DDINFO()
  {
    createDiag(Diagnostic875, true);
    _ofc->checkDiagForS5Detail(_subCode);
    // CPPUNIT_ASSERT_EQUAL(std::string::npos, _ofc->diag875()->str().find("SERVICE FEE
    // DIAGNOSTIC"));
  }

  void testPrintDiagS5Info()
  {
    createDiag();
    _ofc->printDiagS5Info(_subCode, PASS_S5);
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _ofc->diag875()->str().find("----------"));
  }

  void testPrintDiagS5Info_DDINFO()
  {
    createDiag(Diagnostic875, true);
    _ofc->printDiagS5Info(_subCode, PASS_S5);
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("----------") != std::string::npos);
  }

  void testPrintNoGroupCodeProvided()
  {
    createDiag();
    _ofc->printNoGroupCodeProvided();
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("NOT PROCESSED - NO GROUP CODE PROVIDED") !=
                   std::string::npos);
  }

  void testPrintNoOptionsRequested()
  {
    createDiag();
    _ofc->printNoOptionsRequested();
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("NOT PROCESSED - REQUESTED OPTIONS = 0") !=
                   std::string::npos);
  }

  void testPrintPccIsDeactivated()
  {
    createDiag();
    _ofc->printPccIsDeactivated("ABCD");
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("NOT PROCESSED - AGENCY PCC ABCD IS NOT ACTIVE") !=
                   std::string::npos);
  }

  void testPrintCanNotCollect()
  {
    createDiag();
    _ofc->printCanNotCollect(PASS_S5);
    CPPUNIT_ASSERT(_ofc->diag875()->rootDiag()->toString().find("NOT PROCESSED - PASS") !=
                   std::string::npos);
  }

  void testPrintDiagS5NotFound()
  {
    createDiag();
    _ofc->printDiagS5NotFound("ML", "AA");
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("DATA NOT FOUND") != std::string::npos);
  }

  void testPrintDiagS5NotFound_DDINFO()
  {
    createDiag(Diagnostic875, true);
    _ofc->printDiagS5NotFound("ML", "AA");
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _ofc->diag875()->str().find("DATA NOT FOUND"));
  }

  void testPrintDiagS5ValidatingCarrier()
  {
    createDiag();
    _trx->billing()->validatingCarrier() = "XX";
    _ofc->printDiagS5NotFound("99", "XX");
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("XX V") != std::string::npos);
  }

  void testPrintDiagS5NoValidatingCarrier()
  {
    createDiag();
    _trx->billing()->validatingCarrier() = "";
    _ofc->printDiagS5NotFound("99", "XX");
    CPPUNIT_ASSERT(_ofc->diag875()->str().find("XX P") != std::string::npos);
  }

  void testPrintS7Header()
  {
    createDiag(Diagnostic877);
    _ofc->printS7Header("ML", "AA");
    CPPUNIT_ASSERT(_ofc->diag877()->str().find(
                       "V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS") != std::string::npos);
  }

  void testPrintS7Header_DDINFO()
  {
    createDiag(Diagnostic877, true);
    _ofc->printS7Header("ML", "AA");
    CPPUNIT_ASSERT_EQUAL(
        std::string::npos,
        _ofc->diag877()->str().find("V SCODE SERVICE   SEQ NUM  PAX    AMOUNT  STATUS"));
  }

  void testPrintS7HeaderNoValidatingCarrier()
  {
    createDiag(Diagnostic877);
    _trx->billing()->validatingCarrier() = "";
    _ofc->printS7Header("99", "XX");
    CPPUNIT_ASSERT(_ofc->diag877()->str().find("CXR      : XX  - PARTITION") != std::string::npos);
  }

  void testPrintS7HeaderValidatingCarrier()
  {
    createDiag(Diagnostic877);
    _trx->billing()->validatingCarrier() = "XX";
    _ofc->printS7Header("99", "XX");
    CPPUNIT_ASSERT(_ofc->diag877()->str().find("CXR      : XX  - VALIDATING") != std::string::npos);
  }

  void testPrintStopAtFirstMatchMsg()
  {
    createDiag();
    _ofc->printStopAtFirstMatchMsg();
    CPPUNIT_ASSERT(_ofc->diag875()->rootDiag()->toString().find(
                       "STOP AFTER FIRST MATCH - NO GROUP CODE IN THE ENTRY") != std::string::npos);
  }

  void testGetValidRoutesWhenOneSegment()
  {
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> > routes;
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1;
    tvlSeg.push_back(&seg1);

    _ofc->getValidRoutes(routes, tvlSeg.begin(), tvlSeg.end());
    CPPUNIT_ASSERT(routes.empty());
  }

  void testGetValidRoutesWhen2Segments()
  {
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> > routes;
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1, seg2;
    seg1.origAirport() = "LON";
    seg2.origAirport() = "PAR";
    seg1.destAirport() = "PAR";
    seg2.destAirport() = "CHI";
    tvlSeg.push_back(&seg1);
    tvlSeg.push_back(&seg2);

    _ofc->getValidRoutes(routes, tvlSeg.begin(), tvlSeg.end());
    CPPUNIT_ASSERT_EQUAL(size_t(2), routes.size());
    CPPUNIT_ASSERT(tvlSeg.begin() == routes[0].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[0].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[1].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[1].second);
  }

  void testGetValidRoutesWhen3Segments()
  {
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> > routes;
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1, seg2, seg3;
    seg1.origAirport() = "LON";
    seg2.origAirport() = "PAR";
    seg3.origAirport() = "CHI";
    seg1.destAirport() = "PAR";
    seg2.destAirport() = "CHI";
    seg3.origAirport() = "DFW";
    tvlSeg.push_back(&seg1);
    tvlSeg.push_back(&seg2);
    tvlSeg.push_back(&seg3);

    _ofc->getValidRoutes(routes, tvlSeg.begin(), tvlSeg.end());
    CPPUNIT_ASSERT_EQUAL(size_t(5), routes.size());
    CPPUNIT_ASSERT(tvlSeg.begin() == routes[0].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[0].second);
    CPPUNIT_ASSERT(tvlSeg.begin() == routes[1].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[1].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[2].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[2].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[3].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 3 == routes[3].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[4].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 3 == routes[4].second);
  }

  void testGetValidRoutesWhen3SegmentsAndSecondIsArunk()
  {
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> > routes;
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1, seg3;
    ArunkSeg arunkSeg2;
    seg1.origAirport() = "LON";
    arunkSeg2.origAirport() = "MAN";
    seg3.origAirport() = "CHI";
    seg1.destAirport() = "PAR";
    arunkSeg2.destAirport() = "CHI";
    seg3.origAirport() = "DFW";
    tvlSeg.push_back(&seg1);
    tvlSeg.push_back(&arunkSeg2);
    tvlSeg.push_back(&seg3);

    _ofc->getValidRoutes(routes, tvlSeg.begin(), tvlSeg.end());
    CPPUNIT_ASSERT_EQUAL(size_t(2), routes.size());
    CPPUNIT_ASSERT(tvlSeg.begin() == routes[0].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[0].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[1].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 3 == routes[1].second);
  }

  void testGetValidRoutesWhen3SegmentsWithRetransit()
  {
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> > routes;
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1, seg2, seg3;
    seg1.origAirport() = "LON";
    seg2.origAirport() = "PAR";
    seg3.origAirport() = "LON";
    seg1.destAirport() = "PAR";
    seg2.destAirport() = "LON";
    seg3.origAirport() = "CHI";
    tvlSeg.push_back(&seg1);
    tvlSeg.push_back(&seg2);
    tvlSeg.push_back(&seg3);

    _ofc->getValidRoutes(routes, tvlSeg.begin(), tvlSeg.end());
    CPPUNIT_ASSERT_EQUAL(size_t(4), routes.size());
    CPPUNIT_ASSERT(tvlSeg.begin() == routes[0].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[0].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[1].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[1].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 1 == routes[2].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 3 == routes[2].second);
    CPPUNIT_ASSERT(tvlSeg.begin() + 2 == routes[3].first);
    CPPUNIT_ASSERT(tvlSeg.begin() + 3 == routes[3].second);
  }

  void testAddInvalidGroupsPricing()
  {
    std::vector<ServiceGroup> groupNA; // Group codes not active
    std::vector<ServiceGroup> groupNP; // Group codes not active
    std::vector<ServiceGroup> groupNV; // Group codes invalid
    ServiceGroup group2 = "UP";
    ServiceGroup group3 = "FF";

    groupNA.push_back(group2);
    groupNP.push_back(group3);

    _ofc->addInvalidGroups(*_itin, groupNA, groupNP, groupNV);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _itin->ocFeesGroup().size());
    CPPUNIT_ASSERT_EQUAL(group2, _itin->ocFeesGroup()[0]->groupCode());
    CPPUNIT_ASSERT_EQUAL(group3, _itin->ocFeesGroup()[1]->groupCode());
  }

  void testAddInvalidGroupsShopping()
  {
    _ofc->_shoppingOCF = true;
    std::vector<ServiceGroup> groupNA; // Group codes not active
    std::vector<ServiceGroup> groupNP; // Group codes not active
    std::vector<ServiceGroup> groupNV; // Group codes invalid
    ServiceGroup group2 = "UP";
    ServiceGroup group3 = "FF";

    groupNA.push_back(group2);
    groupNP.push_back(group3);

    _ofc->addInvalidGroups(*_itin, groupNA, groupNP, groupNV);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _itin->ocFeesGroup().size());
  }

  void testCheckAllSegsConfirmedWhenAllConfirmed()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = CONFIRM_RES_STATUS;
    _trx->travelSeg()[1]->resStatus() = NOSEAT_RES_STATUS;

    CPPUNIT_ASSERT(
        _ofc->checkAllSegsConfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end(), false));
    CPPUNIT_ASSERT_EQUAL(true, _itin->allSegsConfirmed());
  }

  void testCheckAllSegsConfirmedWhenPartiallyConfirmed()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = CONFIRM_RES_STATUS;
    _trx->travelSeg()[1]->resStatus() = "";

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsConfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end(), false));
    CPPUNIT_ASSERT_EQUAL(false, _itin->allSegsConfirmed());
  }

  void testCheckAllSegsConfirmedWhenAllUnconfirmed()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[1]->resStatus() = "";

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsConfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end(), false));
    CPPUNIT_ASSERT_EQUAL(false, _itin->allSegsConfirmed());
  }

  void testCheckAllSegsConfirmedWhenPartiallyConfirmedDiag()
  {
    createDiag();

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[1]->resStatus() = CONFIRM_RES_STATUS;

    _ofc->checkAllSegsConfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end());
    CPPUNIT_ASSERT_EQUAL(
        std::string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                    "***  SOME SECTORS UNCONFIRMED  ***       NOT PROCESSED\n"),
        _ofc->diag875()->str());
  }

  void testCheckAllSegsConfirmedWhenAllUnconfirmedDiag()
  {
    createDiag();

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[1]->resStatus() = "";

    _ofc->checkAllSegsConfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end());
    CPPUNIT_ASSERT_EQUAL(
        std::string("******************* SERVICE FEE DIAGNOSTIC ********************\n"
                    "***  ALL SECTORS UNCONFIRMED  ***        NOT PROCESSED\n"),
        _ofc->diag875()->str());
  }

  void testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegTypeArunk()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = "";
    _trx->travelSeg()[1]->segmentType() = tse::Arunk;

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(!_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegTypeSurface()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = "";
    _trx->travelSeg()[1]->segmentType() = tse::Surface;

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(!_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegStatusConfirm()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = tse::CONFIRM_RES_STATUS;
    _trx->travelSeg()[1]->segmentType() = tse::Air;

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(!_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testCheckAllSegsUnconfirmedWhenPartiallyUnconfirmedSegStatusNoSeat()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = tse::NOSEAT_RES_STATUS;
    _trx->travelSeg()[1]->segmentType() = tse::Air;

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(!_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testCheckAllSegsUnconfirmedWhenAllUnconfirmed()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = "";
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = "";
    _trx->travelSeg()[1]->segmentType() = tse::Air;

    CPPUNIT_ASSERT(
        _ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testCheckAllSegsUnconfirmedWhenAllConfirmed()
  {
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _trx->travelSeg()[0]->resStatus() = tse::CONFIRM_RES_STATUS;
    _trx->travelSeg()[0]->segmentType() = tse::Air;
    _trx->travelSeg()[1]->resStatus() = tse::CONFIRM_RES_STATUS;
    _trx->travelSeg()[1]->segmentType() = tse::Air;

    CPPUNIT_ASSERT(
        !_ofc->checkAllSegsUnconfirmed(_trx->travelSeg().begin(), _trx->travelSeg().end()));
    CPPUNIT_ASSERT(!_ofc->_farePath->itin()->allSegsUnconfirmed());
  }

  void testUpdateServiceFeesGroupStateWithFees()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);

    _sfg->state() = ServiceFeesGroup::VALID;
    _ofc->updateServiceFeesGroupState(_sfg);

    CPPUNIT_ASSERT_EQUAL(ServiceFeesGroup::VALID, _sfg->state());
  }

  void testUpdateServiceFeesGroupStateWhenNoFeesAndNotInCxrMap()
  {
    createServiceFeesGroup(ServiceGroup("ML"), true);

    _sfg->state() = ServiceFeesGroup::VALID;
    _ofc->updateServiceFeesGroupState(_sfg);

    CPPUNIT_ASSERT_EQUAL(ServiceFeesGroup::NOT_AVAILABLE, _sfg->state());
  }

  void testUpdateServiceFeesGroupStateWhenNoFeesAndInCxrMap()
  {
    createServiceFeesGroup(ServiceGroup("ML"), true);

    _sfg->state() = ServiceFeesGroup::VALID;

    ServiceGroup sg("ML");
    std::vector<ServiceGroup*> sgs;
    sgs.push_back(&sg);

    _ofc->_cXrGrpOCF.insert(std::make_pair("LO", sgs));
    _ofc->updateServiceFeesGroupState(_sfg);

    CPPUNIT_ASSERT_EQUAL(ServiceFeesGroup::EMPTY, _sfg->state());
  }

  OCFees* createOCFees(const CarrierCode& carrierCode, const ServiceGroup& srvGroup)
  {
    SubCodeInfo* scInfo = _memHandle.create<SubCodeInfo>();
    scInfo->serviceGroup() = srvGroup;

    OCFees* fees = _memHandle.create<OCFees>();
    fees->carrierCode() = carrierCode;
    fees->subCodeInfo() = scInfo;

    return fees;
  }

  void testUpdateServiceFeesDisplayOnlyState_DispOnly()
  {
    std::vector<ServiceGroup*> sgVec;
    ServiceGroup sg = "BG";
    sgVec.push_back(&sg);
    _ofc->_cXrDispOnlyGrp.insert(std::make_pair("AA", sgVec));
    OCFees* fees = createOCFees("AA", "BG");

    _ofc->updateServiceFeesDisplayOnlyState(fees);
    CPPUNIT_ASSERT(fees->isDisplayOnly());
  }

  void testUpdateServiceFeesDisplayOnlyState_NotDispOnly_Carrier()
  {
    std::vector<ServiceGroup*> sgVec;
    ServiceGroup sg = "BG";
    sgVec.push_back(&sg);
    _ofc->_cXrDispOnlyGrp.insert(std::make_pair("AA", sgVec));
    OCFees* fees = createOCFees("LH", "BG");

    _ofc->updateServiceFeesDisplayOnlyState(fees);
    CPPUNIT_ASSERT(!fees->isDisplayOnly());
  }

  void testUpdateServiceFeesDisplayOnlyState_NotDispOnly_Group()
  {
    std::vector<ServiceGroup*> sgVec;
    ServiceGroup sg = "BG";
    sgVec.push_back(&sg);
    _ofc->_cXrDispOnlyGrp.insert(std::make_pair("AA", sgVec));
    OCFees* fees = createOCFees("AA", "SA");

    _ofc->updateServiceFeesDisplayOnlyState(fees);
    CPPUNIT_ASSERT(!fees->isDisplayOnly());
  }

  void testIsTimeOut_Pricing_TimeOut_Not_Occur()
  {
    std::string cfgTime = "100";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    TrxAborter* aborter = _trx->aborter();
    int hurryAt = 2;
    aborter->setHurry(hurryAt);

    CPPUNIT_ASSERT(!_ofc->isTimeOut(1));
  }

  void testIsTimeOut_Shopping_TimeOut_Not_Occur()
  {
    _ofc->_shoppingOCF = true;

    CPPUNIT_ASSERT(!_ofc->isTimeOut(1));
  }

  void testIsTimeOut_Pricing_TimeOut_Occur()
  {
    _ofc->_shoppingOCF = false;
    std::string cfgTime = "100";
    TestConfigInitializer::setValue("TRX_TIMEOUT", cfgTime, "TSE_SERVER");
    TrxUtil::createTrxAborter(*_trx);
    TrxAborter* aborter = _trx->aborter();
    int hurryAt = -1;
    aborter->setHurry(hurryAt);
    _trx->setHurry();

    CPPUNIT_ASSERT(_ofc->isTimeOut(1));
    CPPUNIT_ASSERT(_ofc->_stopMatchProcess);
    CPPUNIT_ASSERT(_ofc->_timeOut);
  }

  void testCleanUpAllOCFees_TimeOut()
  {
    createOCFeesBeforeCleanUp(3, true, true);
    _ofc->cleanUpAllOCFees(*_itin);

    int total = 0;
    CPPUNIT_ASSERT_EQUAL(0, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(true,
                         _itin->ocFeesGroup().front()->state() == ServiceFeesGroup::NOT_AVAILABLE);
  }

  void testCleanUpGroupCode_TimeOut()
  {
    createOCFeesBeforeCleanUp(3, true, true);
    _ofc->cleanUpGroupCode(*_sfg);

    int total = 0;
    CPPUNIT_ASSERT_EQUAL(0, collectOCFeesAfterCleanUp(total));
    CPPUNIT_ASSERT_EQUAL(true, _itin->ocFeesGroup().front()->state() == ServiceFeesGroup::VALID);
  }

  void testCollectExcludedSFG_NothingToCollect()
  {
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    std::vector<ServiceGroup> collect; // Group codes collected

    _ofc->collectExcludedSFG(collect, groupNV, groupNA);

    CPPUNIT_ASSERT_EQUAL(size_t(0), collect.size());
  }

  void testCollectExcludedSFG_NVCollect()
  {
    std::vector<ServiceGroup> collect; // Group codes collected
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    ServiceGroup group1 = "BB";
    ServiceGroup group2 = "UP";

    groupNV.push_back(group1);
    groupNV.push_back(group2);

    _ofc->collectExcludedSFG(collect, groupNV, groupNA);

    CPPUNIT_ASSERT_EQUAL(size_t(2), collect.size());
    CPPUNIT_ASSERT_EQUAL(group1, collect[0]);
    CPPUNIT_ASSERT_EQUAL(group2, collect[1]);
  }

  void testCollectExcludedSFG_NACollect()
  {
    std::vector<ServiceGroup> collect; // Group codes collected
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    ServiceGroup group1 = "BB";
    ServiceGroup group2 = "UP";

    groupNA.push_back(group1);
    groupNA.push_back(group2);

    _ofc->collectExcludedSFG(collect, groupNV, groupNA);

    CPPUNIT_ASSERT_EQUAL(size_t(2), collect.size());
    CPPUNIT_ASSERT_EQUAL(group1, collect[0]);
    CPPUNIT_ASSERT_EQUAL(group2, collect[1]);
  }

  void testCollectExcludedSFG_NANVCollect()
  {
    std::vector<ServiceGroup> collect; // Group codes collected
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    ServiceGroup group1 = "BB";
    ServiceGroup group2 = "UP";
    ServiceGroup group3 = "BC";
    ServiceGroup group4 = "UC";

    groupNA.push_back(group1);
    groupNA.push_back(group2);
    groupNV.push_back(group3);
    groupNV.push_back(group4);

    _ofc->collectExcludedSFG(collect, groupNV, groupNA);

    CPPUNIT_ASSERT_EQUAL(size_t(4), collect.size());
    CPPUNIT_ASSERT_EQUAL(group3, collect[0]);
    CPPUNIT_ASSERT_EQUAL(group4, collect[1]);
    CPPUNIT_ASSERT_EQUAL(group1, collect[2]);
    CPPUNIT_ASSERT_EQUAL(group2, collect[3]);
  }

  void testIsTravelSegFound_NotFound()
  {
    AirSeg air;
    createServiceFeesGroup(ServiceGroup("ML"), false);
    std::map<const FarePath*, std::vector<OCFees*> >::iterator mI = _sfg->ocFeesMap().begin();
    std::vector<OCFees*>& vecOCFees = mI->second;

    CPPUNIT_ASSERT(!_ofc->isTravelSegFound(&air, vecOCFees));
  }

  void testIsTravelSegFound_Found()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);
    std::map<const FarePath*, std::vector<OCFees*> >::iterator mI = _sfg->ocFeesMap().begin();
    std::vector<OCFees*>& vecOCFees = mI->second;

    CPPUNIT_ASSERT(_ofc->isTravelSegFound(vecOCFees.front()->travelStart(), vecOCFees));
    CPPUNIT_ASSERT(_ofc->isTravelSegFound(vecOCFees.back()->travelStart(), vecOCFees));
  }

  void testCheckOCFeeMap_NotFound()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);
    AirSeg air;

    CPPUNIT_ASSERT(!_ofc->checkOCFeeMap(&air, *_sfg));
  }

  void testCheckOCFeeMap_Found()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);

    CPPUNIT_ASSERT(_ofc->checkOCFeeMap(_ofc->_first, *_sfg));
    CPPUNIT_ASSERT(_ofc->checkOCFeeMap(_ofc->_last, *_sfg));
  }

  void testCheckLargestPortion_SegmentNotFound()
  {
    createServiceFeesGroup(ServiceGroup("ML"), false);
    AirSeg air1, air2;
    std::vector<TravelSeg*> tvls;
    tvls.push_back(&air1);
    tvls.push_back(&air2);

    CPPUNIT_ASSERT(!_ofc->checkLargestPortion(tvls.begin(), tvls.end(), *_sfg));
  }

  void testCheckLargestPortion_SegmentFound()
  {
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);

    CPPUNIT_ASSERT(_ofc->checkLargestPortion(_ofc->_beginsOfUOT[0], _ofc->_beginsOfUOT[1], *_sfg));
  }

  void testIsOCFeesProcessedForLargestPortions_PASS()
  {
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);

    CPPUNIT_ASSERT(_ofc->isOCFeesProcessedForLargestPortions(*_itin, *_sfg));
  }

  void testIsOCFeesProcessedForLargestPortions_FAIL()
  {
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);
    std::map<const FarePath*, std::vector<OCFees*> >::iterator mI = _sfg->ocFeesMap().begin();
    std::vector<OCFees*>& vecOCFees = mI->second;

    AirSeg air1, air2;
    for (std::vector<OCFees*>::iterator ocfee = vecOCFees.begin(); ocfee != vecOCFees.end();
         ++ocfee)
    {
      (*ocfee)->travelStart() = &air1;
      (*ocfee)->travelEnd() = &air2;
    }
    int total = 0;
    CPPUNIT_ASSERT_EQUAL(1, collectOCFeesAfterCleanUp(total));

    CPPUNIT_ASSERT(!_ofc->isOCFeesProcessedForLargestPortions(*_itin, *_sfg));
    CPPUNIT_ASSERT(_itin->timeOutForExceeded());
    total = 0;
    CPPUNIT_ASSERT_EQUAL(0, collectOCFeesAfterCleanUp(total));
  }

  void testTimeOutPostProcessing_WP()
  {
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->timeOutPostProcessing(*_itin, groupNV, groupNA);

    CPPUNIT_ASSERT(_itin->timeOutOCFForWP());
    CPPUNIT_ASSERT(!_itin->timeOutForExceeded());
  }

  void testTimeOutPostProcessing_WPAE_with_no_groupCodes()
  {
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    _trx->getOptions()->isProcessAllGroups() = true;
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->timeOutPostProcessing(*_itin, groupNV, groupNA);

    CPPUNIT_ASSERT(_itin->timeOutForExceeded());
    CPPUNIT_ASSERT(!_itin->timeOutOCFForWP());
  }

  void testTimeOutPostProcessing_WPAE_with_Group_Not_Requested()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML, requestedOcFeeGroupRO;
    requestedOcFeeGroupML.groupCode() = "ML";
    requestedOcFeeGroupRO.groupCode() = "RO";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);
    _options->serviceGroupsVec().push_back(requestedOcFeeGroupRO);

    _trx->ticketingDate() = DateTime(2010, 2, 17);

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("BG"), false);
    _ofc->timeOutPostProcessing(*_itin, groupNV, groupNA);

    CPPUNIT_ASSERT(_itin->timeOutForExceeded());
    CPPUNIT_ASSERT(!_itin->timeOutOCFForWP());
  }

  void testTimeOutPostProcessing_WPAE_with_Group_Requested_Not_Processed()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML;
    requestedOcFeeGroupML.groupCode() = "ML";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);

    _trx->ticketingDate() = DateTime(2010, 2, 17);

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"));
    _ofc->timeOutPostProcessing(*_itin, groupNV, groupNA);

    CPPUNIT_ASSERT(_itin->timeOutForExceeded());
    CPPUNIT_ASSERT(!_itin->timeOutOCFForWP());
  }

  void testTimeOutPostProcessing_WPAE_with_Group_Requested_And_Completely_Processed()
  {
    RequestedOcFeeGroup requestedOcFeeGroupML;
    requestedOcFeeGroupML.groupCode() = "ML";

    _options->serviceGroupsVec().push_back(requestedOcFeeGroupML);

    _trx->ticketingDate() = DateTime(2010, 2, 17);

    Customer customer;
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    _itin->timeOutOCFForWP() = false;
    _itin->timeOutForExceeded() = false;
    std::vector<ServiceGroup> groupNV; // Group codes not valid
    std::vector<ServiceGroup> groupNA; // Group codes not active
    _itin->farePath().clear();
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->timeOutPostProcessing(*_itin, groupNV, groupNA);

    CPPUNIT_ASSERT(_itin->timeOutForExceededSFGpresent());
    CPPUNIT_ASSERT(!_itin->timeOutForExceeded());
    CPPUNIT_ASSERT(!_itin->timeOutOCFForWP());
  }

  void testMarketingSameAsOperating_OneOperatingCxr_MatchMarketingCxr()
  {
    insertCarrier("AA", false);
    insertCarrier("AA", true);
    CPPUNIT_ASSERT(_ofc->marketingSameAsOperating(0));
  }

  void testMarketingSameAsOperating_OneOperatingCxr_NotMatchMarketingCxr()
  {
    insertCarrier("AA", false);
    insertCarrier("CC", true);
    CPPUNIT_ASSERT(!_ofc->marketingSameAsOperating(0));
  }

  void testMarketingSameAsOperating_MultipleOperatingCxr()
  {
    insertCarrier("AA", false);
    insertCarrier("CC", true);
    insertCarrier("DD", true);
    CPPUNIT_ASSERT(!_ofc->marketingSameAsOperating(0));
  }

  void testGetFilterMask()
  {
    std::vector<TravelSeg*> tvlSeg;
    AirSeg seg1, seg2, seg4, seg5, seg6, seg8;
    ArunkSeg arunkSeg3, arunkSeg7;
    tvlSeg.push_back(&seg1);
    tvlSeg.push_back(&seg2);
    tvlSeg.push_back(&arunkSeg3);
    tvlSeg.push_back(&seg4);
    tvlSeg.push_back(&seg5);
    tvlSeg.push_back(&seg6);
    tvlSeg.push_back(&arunkSeg7);
    tvlSeg.push_back(&seg8);
    arunkSeg3.segmentOrder() = 3;
    arunkSeg7.segmentOrder() = 7;
    _ofc->_beginsOfUOT[0] = tvlSeg.begin();
    _ofc->_beginsOfUOT[1] = tvlSeg.end();

    CPPUNIT_ASSERT(0x1010ULL == _ofc->getFilterMask(0));
  }

  void testCheckFlightRelated_Pass_Baggage()
  {
    _subCode->fltTktMerchInd() = PREPAID_BAGGAGE;
    CPPUNIT_ASSERT(_ofc->checkFlightRelated(*_subCode));
  }

  void testProcessMarketDrivenFeesGroupWhenSliceAndDiceAndIsProcessed()
  {
    _ofc->_slice_And_Dice = true;
    _sfg->setSliceAndDiceProcessed();
    CPPUNIT_ASSERT(_ofc->processMarketDrivenFeesGroup(0, _sfg, *_s6Validator));
  }

  void testProcessMarketDrivenFeesGroupWhenSliceAndDiceAndIsNotProcessed()
  {
    _ofc->_slice_And_Dice = true;
    CPPUNIT_ASSERT(!_ofc->processMarketDrivenFeesGroup(0, _sfg, *_s6Validator));
  }

  void testProcessMarketDrivenFeesGroupWhenNoMarketDrivenFlights()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    _ofc->_beginsOfUOT[unitNo] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfUOT[unitNo + 1] = _farePath->itin()->travelSeg().end();

    CPPUNIT_ASSERT(!_ofc->processMarketDrivenFeesGroup(unitNo, _sfg, *_s6Validator));
  }

  void testProcessMarketDrivenFeesGroupWhenHasInterlineJourneyFlight()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_processSingleServiceFeesGroup = true;

    bool res = _ofc->processMarketDrivenFeesGroup(unitNo, _sfg, *_s6Validator);

    CPPUNIT_ASSERT_EQUAL(true, _ofc->_sliceAndDiceWasCalled);
    CPPUNIT_ASSERT(_sfg->isSliceAndDiceProcessed());
    CPPUNIT_ASSERT(res);
  }

  void testProcessMarketDrivenFeesGroupWhenHasInterlineJourneyFlightButGotTimeOut()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_processSingleServiceFeesGroup = true;
    _ofc->_timeout = true;

    bool res = _ofc->processMarketDrivenFeesGroup(unitNo, _sfg, *_s6Validator);

    CPPUNIT_ASSERT_EQUAL(false, _ofc->_sliceAndDiceWasCalled);
    CPPUNIT_ASSERT(_sfg->isSliceAndDiceProcessed());
    CPPUNIT_ASSERT(res);
  }

  void testProcessMarketDrivenFeesGroupWhenHasMarketDrivenFlight()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_processSingleServiceFeesGroup = true;
    _sfg->_marketDriven = true;
    MerchCarrierPreferenceInfo rec;
    rec.prefVendor() = "SA";
    _sfg->merchCxrPref()["AA"] = &rec;

    bool res = _ofc->processMarketDrivenFeesGroup(unitNo, _sfg, *_s6Validator);

    CPPUNIT_ASSERT_EQUAL(static_cast<MerchCarrierStrategy*>(&_ofc->_multipleStrategy),
                         _ofc->_merchCrxStrategy);
    CPPUNIT_ASSERT_EQUAL(false, _ofc->_sliceAndDiceWasCalled);
    CPPUNIT_ASSERT(_sfg->isSliceAndDiceProcessed());
    CPPUNIT_ASSERT(res);
  }

  void testUpdateTravelParamsWhenMarketDrivenFlight()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> > unitsOfTravel;
    unitsOfTravel.push_back(std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool>(
        _farePath->itin()->travelSeg().begin(), _farePath->itin()->travelSeg().end(), true));
    MerchCarrierPreferenceInfo rec;
    rec.prefVendor() = "SA";
    _sfg->merchCxrPref()["AA"] = &rec;

    _ofc->updateTravelParams(unitNo, _sfg, unitsOfTravel.begin());

    CPPUNIT_ASSERT_EQUAL(static_cast<MerchCarrierStrategy*>(&_ofc->_singleStrategy),
                         _ofc->_merchCrxStrategy);
    CPPUNIT_ASSERT_EQUAL(VendorCode("SA"), _ofc->_merchCrxStrategy->getPreferedVendor());
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().begin() == _ofc->_beginsOfUOT[unitNo]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().end() == _ofc->_beginsOfUOT[unitNo + 1]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().begin() == _ofc->_beginsOfLargestUOT[unitNo]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().end() == _ofc->_beginsOfLargestUOT[unitNo + 1]);
    CPPUNIT_ASSERT_EQUAL(*_farePath->itin()->travelSeg().begin(), _ofc->_first);
    CPPUNIT_ASSERT_EQUAL(*(_farePath->itin()->travelSeg().end() - 1), _ofc->_last);
  }

  void testUpdateTravelParamsWhenInterlineJourneyFlight()
  {
    _ofc->_slice_And_Dice = false;
    int unitNo = 0;
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> > unitsOfTravel;
    unitsOfTravel.push_back(std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool>(
        _farePath->itin()->travelSeg().begin(), _farePath->itin()->travelSeg().end(), false));
    MerchCarrierPreferenceInfo rec;
    rec.prefVendor() = "SA";
    _sfg->merchCxrPref()["AA"] = &rec;

    _ofc->updateTravelParams(unitNo, _sfg, unitsOfTravel.begin());

    CPPUNIT_ASSERT_EQUAL(static_cast<MerchCarrierStrategy*>(&_ofc->_multipleStrategy),
                         _ofc->_merchCrxStrategy);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().begin() == _ofc->_beginsOfUOT[unitNo]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().end() == _ofc->_beginsOfUOT[unitNo + 1]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().begin() == _ofc->_beginsOfLargestUOT[unitNo]);
    CPPUNIT_ASSERT(_farePath->itin()->travelSeg().end() == _ofc->_beginsOfLargestUOT[unitNo + 1]);
    CPPUNIT_ASSERT_EQUAL(*_farePath->itin()->travelSeg().begin(), _ofc->_first);
    CPPUNIT_ASSERT_EQUAL(*(_farePath->itin()->travelSeg().end() - 1), _ofc->_last);
  }

  void testFilterOutBaggageForAcs_NotACS()
  {
    std::vector<ServiceGroup> grValid;
    grValid += "AB";
    grValid += "CD";
    grValid += "BG";
    grValid += "PT";
    _trx->billing()->requestPath() = PSS_PO_ATSE_PATH;
    std::vector<ServiceGroup> grNotValid;

    _ofc->filterOutForACS(grValid, grNotValid);
    CPPUNIT_ASSERT_EQUAL((size_t)4, grValid.size());
  }

  void testFilterOutBaggageForAcs()
  {
    std::vector<ServiceGroup> grValid;
    grValid += "AB";
    grValid += "CD";
    grValid += "BG";
    grValid += "PT";
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    std::vector<ServiceGroup> grNotValid;

    _ofc->filterOutForACS(grValid, grNotValid);
    CPPUNIT_ASSERT_EQUAL((size_t)0, grValid.size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, grNotValid.size());
    CPPUNIT_ASSERT(std::find(grValid.begin(), grValid.end(), "BG") == grValid.end());
    CPPUNIT_ASSERT(std::find(grValid.begin(), grValid.end(), "PT") == grValid.end());
  }

  void testEMDAgreement_Pass() {
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->partitionID() = "EY";
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) !=
                   _sfg->subCodeMap(0).end());
  }

  void testEMDAgreement_Fail() {
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->partitionID() = "AA";
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) ==
                   _sfg->subCodeMap(0).end());
  }

  void testEMDAgreement_Pass_OperatingSameAsPartition()
  {
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    _trx->billing()->partitionID() = "AA";
    CarrierCode marketingCarrier = "BA";
    CarrierCode operatingCarrier = "AA";

    createOneSegmentItin(marketingCarrier, operatingCarrier);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();
    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) !=
                   _sfg->subCodeMap(0).end());
  }

  void testEMDAgreement_Fail_OperatingSameAsPartition()
  {
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->requestPath() = AEBSO_PO_ATSE_PATH;
    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    _trx->billing()->partitionID() = "AA";
    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    AirSeg* seg = dynamic_cast<AirSeg*>(_trx->travelSeg().front());
    CPPUNIT_ASSERT(seg != nullptr);
    seg->setOperatingCarrierCode("AA");
    // SEG1: M: AA, O: AA
    // SEG2: M: AA, O: KL
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) ==
                   _sfg->subCodeMap(0).end());
  }

  void testEMDAgreement_TN_Pass() {
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->partitionID() = "EY";
    Customer customer;
    customer.crsCarrier() = "1J";
    customer.hostName() = "AXES";
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    createDiag(Diagnostic875, true, true);

    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) !=
                   _sfg->subCodeMap(0).end());
  }

  void testEMDAgreement_TN_Fail() {
    _trx->modifiableActivationFlags().setEmdForFlightRelatedServiceAndPrepaidBaggage(true);
    _trx->getRequest()->ticketingAgent()->cxrCode() = "1S";
    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;
    _trx->getRequest()->ticketingAgent()->agentLocation() = &loc;
    _trx->billing()->partitionID() = "AA";
    Customer customer;
    customer.crsCarrier() = "1J";
    customer.hostName() = "AXES";
    _trx->getRequest()->ticketingAgent()->agentTJR() = &customer;
    createDiag(Diagnostic875, true, true);

    _subCode->emdType() = OptionalFeeCollector::EMD_TYPE_ASSOCIATED_TKT;

    createTwoStopItin(_LOC_DFW, _LOC_CHI, _LOC_LON);
    _ofc->_beginsOfLargestUOT[0] = _farePath->itin()->travelSeg().begin();
    _ofc->_beginsOfLargestUOT[1] = _farePath->itin()->travelSeg().end();
    _ofc->_beginsOfLargestUOT[2] = _farePath->itin()->travelSeg().end();
    _ofc->defineMarketingOperatingCarriers();

    createServiceFeesGroup(ServiceGroup("ML"), false);
    _ofc->processSubCodes(*_sfg, CarrierCode("AA"), 0, true);
    CPPUNIT_ASSERT_EQUAL(_ofc->_nation, NationCode("US"));
    CPPUNIT_ASSERT_EQUAL((int)_ofc->_carrierEmdInfoMap.size(), 1);
    CPPUNIT_ASSERT(_sfg->subCodeMap(0)
                   .find(std::make_tuple(_farePath, _subCode->serviceSubTypeCode(), 'F')) ==
                   _sfg->subCodeMap(0).end());
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  FarePath* _farePath;
  Itin* _itin;
  PaxType* _paxType;
  MockOptionalFeeCollector* _ofc;
  TestMemHandle _memHandle;
  const Loc* _LOC_DFW;
  const Loc* _LOC_DEN;
  const Loc* _LOC_CHI;
  const Loc* _LOC_NYC;
  const Loc* _LOC_LON;
  PricingOptions* _options;
  SubCodeInfo* _subCode;
  MockServiceFeesGroup* _sfg;
  TseServer* _server;
  std::vector<ServiceGroup*>* _gValid;
  MerchCarrierPreferenceInfo* _mcpInfo;
  OptionalFeeConcurValidator* _s6Validator;
};
CPPUNIT_TEST_SUITE_REGISTRATION(OptionalFeeCollectorTest);
}
