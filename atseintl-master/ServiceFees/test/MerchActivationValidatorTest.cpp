//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include <boost/assign/std/vector.hpp>

#include "Common/ServiceFeeUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "ServiceFees/MerchActivationValidator.h"
#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/Customer.h"
#include "Common/TrxUtil.h"
#include "Diagnostic/Diag875Collector.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/MockTseServer.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
namespace
{
class MerchActivationValidatorMock : public MerchActivationValidator
{
public:
  MerchActivationValidatorMock(PricingTrx& trx) : MerchActivationValidator(trx, NULL) {}

  const std::vector<MerchActivationInfo*>& getMerchActivationInfo(const CarrierCode& carrier) const
  {
    if (carrier != "")
      return _merchActInfos;
    else
      return _merchActInfosNull;
  }

  const std::vector<MerchActivationInfo*>&
  getMerchActivation(const CarrierCode& carrier, const PseudoCityCode& pcc)
  {
    _vMAI.clear();
    _vMAI.push_back(&_mai1);
    _vMAI.push_back(&_mai2);
    _mai1.carrier() = ANY_CARRIER;
    _mai2.carrier() = ANY_CARRIER;

    return _vMAI;
  }

  void getMerchValidGroupCodes(std::vector<ServiceGroup*>& validGroups)
  {
    ServiceGroup Meal("ML");
    ServiceGroup SeatAllocation("SA");
    validGroups.push_back(&Meal);
    validGroups.push_back(&SeatAllocation);
  }

  std::vector<MerchActivationInfo*> _merchActInfos;
  std::vector<MerchActivationInfo*> _merchActInfosNull;
  std::vector<MerchActivationInfo*> _vMAI;
  MerchActivationInfo _mai1;
  MerchActivationInfo _mai2;
};
}

class MerchActivationValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MerchActivationValidatorTest);

  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_Fail);

  CPPUNIT_TEST(testCheckTravelDate_Pass);
  CPPUNIT_TEST(testCheckTravelDate_Fail_Create);
  CPPUNIT_TEST(testCheckTravelDate_Fail_Expire);
  CPPUNIT_TEST(testCheckTravelDate_Fail_Disc);
  CPPUNIT_TEST(testCheckTravelDate_Fail_Eff);
  CPPUNIT_TEST(test_isRequestFromTN_false);
  CPPUNIT_TEST(test_isRequestFromTN_NoAgentTJRfalse);
  CPPUNIT_TEST(test_isRequestFromTN_true);
  CPPUNIT_TEST(test_processMerchActivaionDataforHC_False);
  CPPUNIT_TEST(test_processMerchActivaionDataforHC_NegativeGroup_False);
  CPPUNIT_TEST(test_processMerchActivaionDataforHC_Carrier_PositiveGroup_True);
  CPPUNIT_TEST(test_processMerchActivaionDataforHC_SABRE_PositiveGroup_True);
  CPPUNIT_TEST(test_processMerchActivaionDataforHC_Blank_PositiveGroup_True);
  CPPUNIT_TEST(test_processMerchActivaionDataforTN_False);
  CPPUNIT_TEST(test_processMerchActivaionDataforTN_NegativeGroup_False);
  CPPUNIT_TEST(test_processMerchActivaionDataforTN_Abacus_PositiveGroup_True);
  CPPUNIT_TEST(test_processMerchActivaionDataforTN_Blank_PositiveGroup_True);
  CPPUNIT_TEST(test_satisfyMMRecordHCCondition_false);
  CPPUNIT_TEST(test_satisfyMMRecordHCCondition_true);
  CPPUNIT_TEST(test_satisfyDefaultMMRecordHCCondition_false);
  CPPUNIT_TEST(test_satisfyDefaultMMRecordHCCondition_true);
  CPPUNIT_TEST(test_satisfyDefaultMMRecordConditionBlank_false);
  CPPUNIT_TEST(test_satisfyDefaultMMRecordConditionBlank_true);
  CPPUNIT_TEST(test_satisfyMMRecordTNCondition_false);
  CPPUNIT_TEST(test_satisfyMMRecordTNCondition_true);
  CPPUNIT_TEST(test_processMAdataForHCAndTN_false);
  CPPUNIT_TEST(test_processMAdataForHCAndTN_AllPositve_True);
  CPPUNIT_TEST(test_processMAdataForHCAndTN_AllNegative_False);
  CPPUNIT_TEST(test_processMAdataForHCAndTN_BothPositiveAndNegative_True);
  CPPUNIT_TEST(test_isAllMAdataPositive_false);
  CPPUNIT_TEST(test_isAllMAdataPositive_true);
  CPPUNIT_TEST(test_isAllMAdataNegative_false);
  CPPUNIT_TEST(test_isAllMAdataNegative_true);
  CPPUNIT_TEST(test_retrieveMerchActivaionData_false);
  CPPUNIT_TEST(test_retrieveMerchActivaionData_false_withMAData);
  CPPUNIT_TEST(test_retrieveMerchActivaionData_True_HC_withMAData);
  CPPUNIT_TEST(test_retrieveMerchActivaionData_True_TN_withMAData);
  CPPUNIT_TEST(test_addMerchGroupCodes_false);
  CPPUNIT_TEST(test_addMerchGroupCodes_true);
  CPPUNIT_TEST(test_addMerchGroupCodes_AllGroupCodes_true);
  CPPUNIT_TEST(test_addMerchGroupCodes_GroupCode_false);
  CPPUNIT_TEST(test_removeMerchGroupCodes_false);
  CPPUNIT_TEST(test_removeMerchGroupCodes_true);
  CPPUNIT_TEST(test_processMerchGroupCodes_false);
  CPPUNIT_TEST(test_processMerchGroupCodes_true);
  CPPUNIT_TEST(test_processMerchGroupCodes_NoAllGroupCodes);
  CPPUNIT_TEST(test_processMerchGroupCodes_AllGroupCode_Yes);
  CPPUNIT_TEST(test_processMerchGroupCodes_GroupCode_No);
  CPPUNIT_TEST(test_processDisplayOnlyGroupCodes_OneDispOnly);
  CPPUNIT_TEST(test_processDisplayOnlyGroupCodes_AllDispOnly);
  CPPUNIT_TEST(test_processDisplayOnlyGroupCodes_NotDispOnly);
  CPPUNIT_TEST(test_printCxrNoMerchAdata_false);
  CPPUNIT_TEST(test_printCxrNoMerchAdata_true);
  CPPUNIT_TEST(test_printActiveCxrGroup_false);
  CPPUNIT_TEST(test_printActiveCxrGroup_true);
  CPPUNIT_TEST(test_printMerchActiveCxrGroups_false);
  CPPUNIT_TEST(test_printMerchActiveCxrGroups_true);
  CPPUNIT_TEST(test_getMerchValidGroupCodes_Empty);
  CPPUNIT_TEST(test_getMerchValidGroupCodes_Not_Empty);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  MerchActivationValidator* _merchActValidator;
  MerchActivationInfo* _merchActInfo;
  MerchActivationInfo* _merchActInfo1;
  PricingRequest* _request;
  Customer _customer;
  Agent _agent;
  Billing* _billing;
  std::vector<MerchActivationInfo*> mmInfoVec;
  std::vector<ServiceGroup*> groups;
  std::vector<ServiceGroup*> displayOnlyGroups;
  ServiceFeesGroup* _sfg;
  MockTseServer* _server;

public:
  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _request->ticketingDT() = DateTime(2010, 1, 1);
    _request->ticketingAgent() = _memHandle.create<Agent>();

    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);

    //_trx->setRequest(_memHandle.create<PricingRequest>());
    MerchActivationValidatorMock* validator =
        _memHandle.insert(new MerchActivationValidatorMock(*_trx));
    _merchActValidator = validator;
    _merchActInfo = _memHandle.create<MerchActivationInfo>();
    _merchActInfo1 = _memHandle.create<MerchActivationInfo>();

    validator->_merchActInfos.push_back(_merchActInfo);
    _trx->ticketingDate() = DateTime(2010, 1, 18);
    _merchActInfo->createDate() = _merchActInfo->expireDate() = _trx->ticketingDate();
    _merchActInfo->discDate() = _merchActInfo->effDate() = _trx->ticketingDate();

    _merchActInfo1->createDate() = _merchActInfo1->expireDate() = _trx->ticketingDate();
    _merchActInfo1->discDate() = _merchActInfo1->effDate() = _trx->ticketingDate();
  }

  void tearDown() { _memHandle.clear(); }

  void addAgent()
  {
    _agent.agentTJR() = &_customer;
    _trx->getRequest()->ticketingAgent() = &_agent;
  }

  void addAgentWithNoTJR()
  {
    _agent.agentTJR() = 0;
    _trx->getRequest()->ticketingAgent() = &_agent;
  }

  void setMerchActivationInfo_SABRE()
  {
    _merchActInfo->userApplType() = CRS_USER_APPL;
    _merchActInfo->userAppl() = SABRE_USER;
    _merchActInfo->carrier() = "AA";
  }

  void createDiagnostic(bool isDDInfo)
  {
    _merchActValidator->_diag875 =
        _memHandle.insert(new Diag875Collector(*_memHandle.create<Diagnostic>()));
    _merchActValidator->_diag875->activate();

    if (isDDInfo)
      _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DISPLAY_DETAIL, "INFO"));
  }

  void setMerchActivationInfo_ABACUS()
  {
    _merchActInfo->userApplType() = CRS_USER_APPL;
    _merchActInfo->userAppl() = ABACUS_USER;
    _merchActInfo->carrier() = "AA";
  }

  void setMerchActivationInfo_HC()
  {
    _merchActInfo->userAppl() = "NW";
    _merchActInfo->userApplType() = MULTIHOST_USER_APPL;
    _merchActInfo->carrier() = "NW";
  }

  void setMerchActivationInfo_Blank()
  {
    _merchActInfo->userApplType() = NO_PARAM;
    _merchActInfo->userAppl() = EMPTY_STRING();
    _merchActInfo->carrier() = "AA";
  }

  void addBilling()
  {
    _billing = _memHandle.create<Billing>();
    _trx->billing() = _billing;
  }

  void setAgent_Abacus()
  {
    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = ABACUS_MULTIHOST_ID;
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = ABACUS_USER;
  }

  void addrecordTommInfoVec() { mmInfoVec.push_back(_merchActInfo); }

  void addAnotherrecordTommInfoVec() { mmInfoVec.push_back(_merchActInfo1); }

  void createMAVMockObject()
  {
    MerchActivationValidatorMock* validator =
        _memHandle.insert(new MerchActivationValidatorMock(*_trx));
    _merchActValidator = validator;
    validator->_merchActInfos.clear();
  }

  void setGroupCode()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
  }

  void prepareDataForprocessMerchActivaionDataforHC()
  {
    addBilling();
    _trx->getRequest()->ticketingAgent()->agentTJR() = 0;
    _trx->billing()->partitionID() = "NW";
    _trx->billing()->aaaCity() = "ORD";
    _merchActInfo->groupCode() = "ML";
  }

  // TESTS

  void testValidate_Pass() { CPPUNIT_ASSERT(_merchActValidator->validate("AA")); }

  void testValidate_Fail() { CPPUNIT_ASSERT(!_merchActValidator->validate("")); }

  void testCheckTravelDate_Pass()
  {
    CPPUNIT_ASSERT(_merchActValidator->checkTravelDate(*_merchActInfo));
  }

  void testCheckTravelDate_Fail_Create()
  {
    _merchActInfo->createDate() = DateTime(2010, 1, 19);
    CPPUNIT_ASSERT(!_merchActValidator->checkTravelDate(*_merchActInfo));
  }

  void testCheckTravelDate_Fail_Expire()
  {
    _merchActInfo->expireDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT(!_merchActValidator->checkTravelDate(*_merchActInfo));
  }

  void testCheckTravelDate_Fail_Disc()
  {
    _merchActInfo->discDate() = DateTime(2010, 1, 17);
    CPPUNIT_ASSERT(!_merchActValidator->checkTravelDate(*_merchActInfo));
  }

  void testCheckTravelDate_Fail_Eff()
  {
    _merchActInfo->effDate() = DateTime(2010, 1, 19);
    CPPUNIT_ASSERT(!_merchActValidator->checkTravelDate(*_merchActInfo));
  }

  void test_isRequestFromTN_false()
  {
    _trx->getRequest()->ticketingAgent() = NULL;
    CPPUNIT_ASSERT_EQUAL(false, ServiceFeeUtil::isRequestFromTN(*_trx));
  }

  void test_isRequestFromTN_NoAgentTJRfalse()
  {
    addAgentWithNoTJR();
    CPPUNIT_ASSERT_EQUAL(false, ServiceFeeUtil::isRequestFromTN(*_trx));
  }

  void test_isRequestFromTN_true()
  {
    addAgent();
    setAgent_Abacus();
    CPPUNIT_ASSERT_EQUAL(true, ServiceFeeUtil::isRequestFromTN(*_trx));
  }

  void test_processMerchActivaionDataforHC_False()
  {
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->processMerchActivaionDataforHC(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforHC_NegativeGroup_False()
  {
    CarrierCode cxr = "NW";
    prepareDataForprocessMerchActivaionDataforHC();
    setMerchActivationInfo_HC();
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->processMerchActivaionDataforHC(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforHC_Carrier_PositiveGroup_True()
  {
    CarrierCode cxr = "NW";
    prepareDataForprocessMerchActivaionDataforHC();
    setMerchActivationInfo_HC();
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true,
                         _merchActValidator->processMerchActivaionDataforHC(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforHC_SABRE_PositiveGroup_True()
  {
    CarrierCode cxr = "NW";
    prepareDataForprocessMerchActivaionDataforHC();
    setMerchActivationInfo_SABRE();
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true,
                         _merchActValidator->processMerchActivaionDataforHC(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforHC_Blank_PositiveGroup_True()
  {
    CarrierCode cxr = "AA";
    prepareDataForprocessMerchActivaionDataforHC();
    setMerchActivationInfo_Blank();
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true,
                         _merchActValidator->processMerchActivaionDataforHC(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforTN_False()
  {
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->processMerchActivaionDataforTN(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforTN_NegativeGroup_False()
  {
    CarrierCode cxr = "AA";
    addAgent();
    setMerchActivationInfo_ABACUS();
    setAgent_Abacus();
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->processMerchActivaionDataforTN(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforTN_Abacus_PositiveGroup_True()
  {
    CarrierCode cxr = "AA";
    addAgent();
    setMerchActivationInfo_ABACUS();
    setAgent_Abacus();
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true,
                         _merchActValidator->processMerchActivaionDataforTN(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMerchActivaionDataforTN_Blank_PositiveGroup_True()
  {
    CarrierCode cxr = "AA";
    addAgent();
    setMerchActivationInfo_Blank();
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true,
                         _merchActValidator->processMerchActivaionDataforTN(
                             mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_satisfyMMRecordHCCondition_false()
  {
    CarrierCode cxr = "AA";
    setMerchActivationInfo_SABRE();
    addBilling();
    _trx->billing()->partitionID() = "NW";
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->satisfyMMRecordHCCondition(*_merchActInfo, cxr));
  }

  void test_satisfyMMRecordHCCondition_true()
  {
    CarrierCode cxr = "NW";
    addBilling();
    _trx->billing()->partitionID() = "NW";
    setMerchActivationInfo_HC();
    CPPUNIT_ASSERT_EQUAL(true, _merchActValidator->satisfyMMRecordHCCondition(*_merchActInfo, cxr));
  }

  void test_satisfyDefaultMMRecordHCCondition_false()
  {
    CarrierCode cxr = "AA";
    setMerchActivationInfo_Blank();
    CPPUNIT_ASSERT_EQUAL(
        false, _merchActValidator->satisfyDefaultMMRecordHCCondition(*_merchActInfo, cxr));
  }

  void test_satisfyDefaultMMRecordHCCondition_true()
  {
    CarrierCode cxr = "AA";
    setMerchActivationInfo_SABRE();
    CPPUNIT_ASSERT_EQUAL(
        true, _merchActValidator->satisfyDefaultMMRecordHCCondition(*_merchActInfo, cxr));
  }

  void test_satisfyDefaultMMRecordConditionBlank_false()
  {
    CarrierCode cxr = "AA";
    setMerchActivationInfo_SABRE();
    CPPUNIT_ASSERT_EQUAL(
        false, _merchActValidator->satisfyDefaultMMRecordConditionBlank(*_merchActInfo, cxr));
  }

  void test_satisfyDefaultMMRecordConditionBlank_true()
  {
    CarrierCode cxr = "AA";
    setMerchActivationInfo_Blank();
    CPPUNIT_ASSERT_EQUAL(
        true, _merchActValidator->satisfyDefaultMMRecordConditionBlank(*_merchActInfo, cxr));
  }

  void test_satisfyMMRecordTNCondition_false()
  {
    CarrierCode cxr = "AA";
    addAgent();
    setMerchActivationInfo_SABRE();
    setAgent_Abacus();
    CPPUNIT_ASSERT_EQUAL(false,
                         _merchActValidator->satisfyMMRecordTNCondition(*_merchActInfo, cxr));
  }

  void test_satisfyMMRecordTNCondition_true()
  {
    CarrierCode cxr = "AA";
    addAgent();
    setMerchActivationInfo_ABACUS();
    setAgent_Abacus();
    CPPUNIT_ASSERT_EQUAL(true, _merchActValidator->satisfyMMRecordTNCondition(*_merchActInfo, cxr));
  }

  void test_processMAdataForHCAndTN_false()
  {
    CarrierCode cxr = "AA";
    CPPUNIT_ASSERT_EQUAL(
        false,
        _merchActValidator->processMAdataForHCAndTN(mmInfoVec, cxr, groups, displayOnlyGroups));
  }

  void test_processMAdataForHCAndTN_AllPositve_True()
  {
    CarrierCode cxr = "AA";
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(
        true,
        _merchActValidator->processMAdataForHCAndTN(mmInfoVec, cxr, groups, displayOnlyGroups));
    CPPUNIT_ASSERT(groups.size() == 1);
  }

  void test_processMAdataForHCAndTN_AllNegative_False()
  {
    CarrierCode cxr = "AA";
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(
        false,
        _merchActValidator->processMAdataForHCAndTN(mmInfoVec, cxr, groups, displayOnlyGroups));
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_processMAdataForHCAndTN_BothPositiveAndNegative_True()
  {
    CarrierCode cxr = "AA";
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = NO;

    _merchActInfo1->groupCode() = "SA";
    _merchActInfo1->includeInd() = YES;
    addrecordTommInfoVec();
    addAnotherrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(
        true,
        _merchActValidator->processMAdataForHCAndTN(mmInfoVec, cxr, groups, displayOnlyGroups));
    CPPUNIT_ASSERT(groups.size() == 1);
  }

  void test_isAllMAdataPositive_false()
  {
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(false, _merchActValidator->isAllMAdataPositive(mmInfoVec));
  }

  void test_isAllMAdataPositive_true()
  {

    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true, _merchActValidator->isAllMAdataPositive(mmInfoVec));
  }

  void test_isAllMAdataNegative_false()
  {
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(false, _merchActValidator->isAllMAdataNegative(mmInfoVec));
  }

  void test_isAllMAdataNegative_true()
  {
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();
    CPPUNIT_ASSERT_EQUAL(true, _merchActValidator->isAllMAdataNegative(mmInfoVec));
  }

  void test_retrieveMerchActivaionData_false()
  {
    CarrierCode cxr = "BA";
    createMAVMockObject();
    CPPUNIT_ASSERT_EQUAL(
        false, _merchActValidator->retrieveMerchActivaionData(cxr, groups, displayOnlyGroups));
  }

  void test_retrieveMerchActivaionData_false_withMAData()
  {
    CarrierCode cxr = "LH";
    addAgent();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1R";
    CPPUNIT_ASSERT_EQUAL(
        false, _merchActValidator->retrieveMerchActivaionData(cxr, groups, displayOnlyGroups));
  }

  void test_retrieveMerchActivaionData_True_HC_withMAData()
  {
    CarrierCode cxr = "NW";
    MerchActivationValidatorMock* validator =
        _memHandle.insert(new MerchActivationValidatorMock(*_trx));
    _merchActValidator = validator;
    prepareDataForprocessMerchActivaionDataforHC();
    setMerchActivationInfo_HC();
    _merchActInfo->includeInd() = YES;
    validator->_merchActInfos.push_back(_merchActInfo);
    CPPUNIT_ASSERT_EQUAL(
        true, _merchActValidator->retrieveMerchActivaionData(cxr, groups, displayOnlyGroups));
  }

  void test_retrieveMerchActivaionData_True_TN_withMAData()
  {
    CarrierCode cxr = "NW";
    MerchActivationValidatorMock* validator =
        _memHandle.insert(new MerchActivationValidatorMock(*_trx));
    _merchActValidator = validator;
    setMerchActivationInfo_ABACUS();
    _merchActInfo->includeInd() = YES;
    validator->_merchActInfos.push_back(_merchActInfo);
    addAgent();
    setAgent_Abacus();
    CPPUNIT_ASSERT_EQUAL(
        true, _merchActValidator->retrieveMerchActivaionData(cxr, groups, displayOnlyGroups));
  }

  void test_addMerchGroupCodes_false()
  {
    _merchActValidator->addMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_addMerchGroupCodes_true()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();

    _merchActValidator->addMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 1);
  }

  void test_addMerchGroupCodes_AllGroupCodes_true()
  {
    _merchActInfo->groupCode() = "**";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();

    _merchActValidator->addMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 2);
  }

  void test_addMerchGroupCodes_GroupCode_false()
  {
    _merchActInfo->groupCode() = "SA";
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();

    _merchActValidator->addMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_removeMerchGroupCodes_false()
  {
    ServiceGroup Meal("ML");
    groups.push_back(&Meal);
    ServiceGroup Baggage("BG");
    groups.push_back(&Baggage);
    ServiceGroup SeatAllowance("SA");
    _merchActValidator->removeMerchGroupCodes(groups, SeatAllowance);

    CPPUNIT_ASSERT(groups.size() == 2);
  }

  void test_removeMerchGroupCodes_true()
  {
    ServiceGroup Meal("ML");
    groups.push_back(&Meal);
    ServiceGroup Baggage("BG");
    groups.push_back(&Baggage);
    _merchActValidator->removeMerchGroupCodes(groups, Baggage);

    CPPUNIT_ASSERT(groups.size() == 1);
  }

  void test_processMerchGroupCodes_false()
  {
    _merchActValidator->processMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_processMerchGroupCodes_true()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();

    _merchActValidator->processMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 1);
  }

  void test_processMerchGroupCodes_NoAllGroupCodes()
  {
    _merchActInfo->groupCode() = "**";
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();

    _merchActValidator->processMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_processMerchGroupCodes_AllGroupCode_Yes()
  {
    _merchActInfo->groupCode() = "**";
    _merchActInfo->includeInd() = YES;
    addrecordTommInfoVec();

    _merchActValidator->processMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 2);
  }

  void test_processMerchGroupCodes_GroupCode_No()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->includeInd() = NO;
    addrecordTommInfoVec();

    _merchActValidator->processMerchGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT(groups.size() == 0);
  }

  void test_processDisplayOnlyGroupCodes_OneDispOnly()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->displayOnly() = YES;
    addrecordTommInfoVec();

    _merchActValidator->processDisplayOnlyGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT_EQUAL((size_t)1, displayOnlyGroups.size());
  }

  void test_processDisplayOnlyGroupCodes_AllDispOnly()
  {
    _merchActInfo->groupCode() = "**";
    _merchActInfo->displayOnly() = YES;
    addrecordTommInfoVec();

    ServiceGroup sgML = "ML";
    groups.push_back(&sgML);
    ServiceGroup sgSA = "SA";
    groups.push_back(&sgSA);
    ServiceGroup sgBG = "BG";
    groups.push_back(&sgBG);

    _merchActValidator->processDisplayOnlyGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT_EQUAL((size_t)3, displayOnlyGroups.size());
  }

  void test_processDisplayOnlyGroupCodes_NotDispOnly()
  {
    _merchActInfo->groupCode() = "ML";
    _merchActInfo->displayOnly() = NO;
    addrecordTommInfoVec();

    _merchActValidator->processDisplayOnlyGroupCodes(mmInfoVec, groups, displayOnlyGroups);
    CPPUNIT_ASSERT_EQUAL((size_t)0, displayOnlyGroups.size());
  }

  void test_printCxrNoMerchAdata_false()
  {
    CarrierCode cxr = "LH";
    createDiagnostic(false);
    _merchActValidator->printCxrNoMerchAdata(cxr);

    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find("AA") == std::string::npos);
  }

  void test_printCxrNoMerchAdata_true()
  {
    CarrierCode cxr = "LH";
    createDiagnostic(false);
    _merchActValidator->printCxrNoMerchAdata(cxr);

    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find("NO MERCH ACTIVATION DATA") !=
                   std::string::npos);
  }

  void test_printActiveCxrGroup_false()
  {
    CarrierCode cxr = "LH";
    bool active = true;
    createDiagnostic(active);
    _merchActValidator->printActiveCxrGroup(cxr, active, "");

    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find("ML") == std::string::npos);
  }

  void test_printActiveCxrGroup_true()
  {
    CarrierCode cxr = "LH";
    std::string group1("ML");
    bool active = true;
    createDiagnostic(active);
    _merchActValidator->printActiveCxrGroup(cxr, active, group1);

    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find("ML") != std::string::npos);
  }

  void test_printMerchActiveCxrGroups_false()
  {
    CarrierCode cxr = "LH";
    ServiceGroup Meal("ML");
    bool active = true;
    createDiagnostic(active);

    _merchActValidator->printMerchActiveCxrGroups(cxr, active, groups);
    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find("ML") == std::string::npos);
  }

  void test_printMerchActiveCxrGroups_true()
  {
    CarrierCode cxr = "LH";
    ServiceGroup Meal("ML");
    bool active = true;
    createDiagnostic(active);
    groups.push_back(&Meal);

    _merchActValidator->printMerchActiveCxrGroups(cxr, active, groups);
    CPPUNIT_ASSERT(_merchActValidator->_diag875->str().find(Meal) != std::string::npos);
  }

  void test_getMerchValidGroupCodes_Empty()
  {
    MerchActivationValidator marchAct(*_trx, NULL);
    std::vector<ServiceGroup*> validGroups;

    marchAct.getMerchValidGroupCodes(validGroups);
    CPPUNIT_ASSERT(validGroups.size() == 0);
  }

  void test_getMerchValidGroupCodes_Not_Empty()
  {
    MerchActivationValidator marchAct(*_trx, NULL);
    _sfg = _memHandle.create<ServiceFeesGroup>();
    _sfg->groupCode() = "BG";
    _sfg->groupDescription() = "serviceGroup";

    _itin->ocFeesGroup().push_back(_sfg);
    std::vector<ServiceGroup*> validGroups;

    marchAct.getMerchValidGroupCodes(validGroups);
    CPPUNIT_ASSERT(validGroups.size() != 0);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MerchActivationValidatorTest);
}
