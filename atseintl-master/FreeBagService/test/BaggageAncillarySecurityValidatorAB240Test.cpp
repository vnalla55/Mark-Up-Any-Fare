#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "FreeBagService/BaggageAncillarySecurityValidatorAB240.h"
#include "Rules/RuleConst.h"

namespace tse
{

class BaggageAncillarySecurityValidatorAB240Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillarySecurityValidatorAB240Test);

  CPPUNIT_TEST(test_givenNoItinPointerIsPassed_whenConstructingValidatorForAnyAllowancesType_shouldThrow);
  CPPUNIT_TEST(test_whenRequestIsOfTypeWpgb_shouldBeHandledInTheSameWayAsBaggageCharge);
  CPPUNIT_TEST(test_checkGds_whenCarrierGdsCodeIsEmpty_shouldReturnTrue);
  CPPUNIT_TEST(test_checkGds_givenCarrierGdsCodeEqualsToSabreGds_whenGdsCodeFromRequestEqualsToOtherGdsThanSabreGds_shouldReturnTrue);
  CPPUNIT_TEST(test_checkGds_givenDefaultServiceType_whenCarrierGdsCodeEqualsToAnyGds_shouldValidateGdsAgainstVendorCrsCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenDefaultServiceType_whenCarrierGdsCodeNotEqualsToAnyGds_shouldValidateGdsAgainstCxrCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenBaggageChargesServiceType_whenCarrierGdsCodeEqualsToAnyGds_shouldValidateGdsAgainstCxrCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenBaggageChargesServiceType_whenCarrierGdsCodeNotEqualsToAnyGds_shouldValidateGdsAgainstPartitionIdFromBil);
  CPPUNIT_TEST(test_checkGds_givenBaggageEmbargoServiceType_whenHandlingTnRequest_shouldValidateGdsAgainstCxrCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenBaggageEmbargoServiceType_whenHandlingAsRequest_shouldValidateGdsAgainstVendorCrsCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenBaggageAllowanceServiceType_whenHandlingAsReqWithTag_shouldValidateGdsAgainstDefaultTicketingCarrierFromTag);
  CPPUNIT_TEST(test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingAsReqWithTag_shouldValidateGdsAgainstDefaultTicketingCarrierFromTag);
  CPPUNIT_TEST(test_checkGds_givenBaggageAllowanceServiceType_whenHandlingAsReqWithoutTag_shouldValidateGdsAgainstVendorCrsCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingAsReqWithoutTag_shouldValidateGdsAgainstVendorCrsCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenBaggageAllowanceServiceType_whenHandlingTnReqWithTag_shouldValidateGdsAgainstCxrCodeFromTag);
  CPPUNIT_TEST(test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingTnReqWithTag_shouldValidateGdsAgainstCxrCodeFromTag);
  CPPUNIT_TEST(test_checkGds_givenBaggageAllowanceServiceType_whenHandlingTnReqWithoutTag_shouldValidateGdsAgainstCxrCodeFromAgi);
  CPPUNIT_TEST(test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingTnReqWithoutTag_shouldValidateGdsAgainstCxrCodeFromAgi);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _ancPricingTrx = _memHandle.create<AncillaryPricingTrx>();

    _ancRequest = _memHandle.create<AncRequest>();
    _ancPricingTrx->setRequest(_ancRequest);

    _billing = _memHandle.create<Billing>();
    _ancPricingTrx->billing() = _billing;

    _checkInAgent = _memHandle.create<Agent>();
    _ancRequest->ticketingAgent() = _checkInAgent;

    _ticketingAgent = _memHandle.create<Agent>();
    _ancRequest->setBaggageTicketingAgent(_ticketingAgent, 1);

    _itin = _memHandle.create<Itin>();
    _itin->setTicketNumber(1);
  }

  void tearDown() override
  {
    _travelSegs.clear();
    _memHandle.clear();
  }

  void convertRequestToAirlineSolutionRequest(AncillaryPricingTrx *trx)
  {
    trx->getRequest()->ticketingAgent()->agentTJR() = nullptr;
    trx->billing()->partitionID() = "cannotBeEmpty";
    trx->billing()->aaaCity() = "<4";
  }

private:
  const std::string _someCarrierCode = "AB";
  const std::string _differentCarrierCode = "IW";
  const Indicator _defaultServiceType = '?';

  TestMemHandle _memHandle;
  AncillaryPricingTrx* _ancPricingTrx;
  AncRequest* _ancRequest;
  Billing* _billing;
  Agent* _checkInAgent;
  Agent* _ticketingAgent;
  Itin* _itin;
  std::vector<TravelSeg*> _travelSegs;

public:
  void test_givenNoItinPointerIsPassed_whenConstructingValidatorForAnyAllowancesType_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(
      BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, BAGGAGE_ALLOWANCE),
      std::invalid_argument);
    CPPUNIT_ASSERT_THROW(
      BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, CARRY_ON_ALLOWANCE),
      std::invalid_argument);
  }

  void test_whenRequestIsOfTypeWpgb_shouldBeHandledInTheSameWayAsBaggageCharge()
  {
    AncRequest *ancRequest = static_cast<AncRequest*>(_ancPricingTrx->getRequest());
    ancRequest->ancRequestType() = AncRequest::WPBGRequest;
    ancRequest->wpbgPostTicket() = false;
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, _defaultServiceType);
    CPPUNIT_ASSERT(validator._typeOfServiceToValidate == BAGGAGE_CHARGE);
  }

  void test_checkGds_whenCarrierGdsCodeIsEmpty_shouldReturnTrue()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, _defaultServiceType);
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "";

    CPPUNIT_ASSERT(validator.checkGds(&info));
  }

  void test_checkGds_givenCarrierGdsCodeEqualsToSabreGds_whenGdsCodeFromRequestEqualsToOtherGdsThanSabreGds_shouldReturnTrue()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, _defaultServiceType);
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1S;

    _checkInAgent->vendorCrsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->vendorCrsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->vendorCrsCode() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT(validator.checkGds(&info));
  }

  void test_checkGds_givenDefaultServiceType_whenCarrierGdsCodeEqualsToAnyGds_shouldValidateGdsAgainstVendorCrsCodeFromAgi()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, _defaultServiceType);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1F;

    _checkInAgent->vendorCrsCode() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->vendorCrsCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenDefaultServiceType_whenCarrierGdsCodeNotEqualsToAnyGds_shouldValidateGdsAgainstCxrCodeFromAgi()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, _defaultServiceType);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _checkInAgent->cxrCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->cxrCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageChargesServiceType_whenCarrierGdsCodeEqualsToAnyGds_shouldValidateGdsAgainstCxrCodeFromAgi()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, BAGGAGE_CHARGE);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1J;

    _checkInAgent->cxrCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->cxrCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageChargesServiceType_whenCarrierGdsCodeNotEqualsToAnyGds_shouldValidateGdsAgainstPartitionIdFromBil()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, BAGGAGE_CHARGE);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _billing->partitionID() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _billing->partitionID() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageEmbargoServiceType_whenHandlingTnRequest_shouldValidateGdsAgainstCxrCodeFromAgi()
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, BAGGAGE_EMBARGO);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _checkInAgent->cxrCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->cxrCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageEmbargoServiceType_whenHandlingAsRequest_shouldValidateGdsAgainstVendorCrsCodeFromAgi()
  {
    convertRequestToAirlineSolutionRequest(_ancPricingTrx);
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, BAGGAGE_EMBARGO);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _checkInAgent->vendorCrsCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->vendorCrsCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void runAirlineSolutionWithTicketingAgentScenarioTestCaseFor(const Indicator serviceType)
  {
    convertRequestToAirlineSolutionRequest(_ancPricingTrx);
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, serviceType, _itin);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _ticketingAgent->defaultTicketingCarrier() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _ticketingAgent->defaultTicketingCarrier() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageAllowanceServiceType_whenHandlingAsReqWithTag_shouldValidateGdsAgainstDefaultTicketingCarrierFromTag()
  {
    runAirlineSolutionWithTicketingAgentScenarioTestCaseFor(BAGGAGE_ALLOWANCE);
  }

  void test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingAsReqWithTag_shouldValidateGdsAgainstDefaultTicketingCarrierFromTag()
  {
    runAirlineSolutionWithTicketingAgentScenarioTestCaseFor(CARRY_ON_ALLOWANCE);
  }

  void runAirlineSolutionWithoutTicketingAgentScenarioTestCaseFor(const Indicator serviceType)
  {
    convertRequestToAirlineSolutionRequest(_ancPricingTrx);
    _itin->setTicketNumber(0);
    _ticketingAgent = nullptr;
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, serviceType, _itin);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _checkInAgent->vendorCrsCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->vendorCrsCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageAllowanceServiceType_whenHandlingAsReqWithoutTag_shouldValidateGdsAgainstVendorCrsCodeFromAgi()
  {
    runAirlineSolutionWithoutTicketingAgentScenarioTestCaseFor(BAGGAGE_ALLOWANCE);
  }

  void test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingAsReqWithoutTag_shouldValidateGdsAgainstVendorCrsCodeFromAgi()
  {
    runAirlineSolutionWithoutTicketingAgentScenarioTestCaseFor(CARRY_ON_ALLOWANCE);
  }

  void runTravelNetworkWithTicketingAgentScenarioTestCaseFor(const Indicator serviceType)
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, serviceType, _itin);

    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _ticketingAgent->cxrCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _ticketingAgent->cxrCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageAllowanceServiceType_whenHandlingTnReqWithTag_shouldValidateGdsAgainstCxrCodeFromTag()
  {
    runTravelNetworkWithTicketingAgentScenarioTestCaseFor(BAGGAGE_ALLOWANCE);
  }

  void test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingTnReqWithTag_shouldValidateGdsAgainstCxrCodeFromTag()
  {
    runTravelNetworkWithTicketingAgentScenarioTestCaseFor(CARRY_ON_ALLOWANCE);
  }

  void runTravelNetworkWithoutTicketingAgentScenarioTestCaseFor(const Indicator serviceType)
  {
    BaggageAncillarySecurityValidatorAB240 validator(*_ancPricingTrx, _travelSegs.begin(), _travelSegs.end(), false, serviceType, _itin);
    _itin->setTicketNumber(0);
    _ticketingAgent = nullptr;
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = _someCarrierCode;

    _checkInAgent->cxrCode() = _someCarrierCode;
    CPPUNIT_ASSERT(validator.checkGds(&info));

    _checkInAgent->cxrCode() = _differentCarrierCode;
    CPPUNIT_ASSERT(!validator.checkGds(&info));
  }

  void test_checkGds_givenBaggageAllowanceServiceType_whenHandlingTnReqWithoutTag_shouldValidateGdsAgainstCxrCodeFromAgi()
  {
    runAirlineSolutionWithoutTicketingAgentScenarioTestCaseFor(BAGGAGE_ALLOWANCE);
  }

  void test_checkGds_givenCarryOnAllowanceServiceType_whenHandlingTnReqWithoutTag_shouldValidateGdsAgainstCxrCodeFromAgi()
  {
    runAirlineSolutionWithoutTicketingAgentScenarioTestCaseFor(CARRY_ON_ALLOWANCE);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillarySecurityValidatorAB240Test);

} // tse
