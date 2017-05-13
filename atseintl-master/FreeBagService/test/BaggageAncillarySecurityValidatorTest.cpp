#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "FreeBagService/BaggageAncillarySecurityValidator.h"
#include "Rules/RuleConst.h"

namespace tse
{

class BaggageAncillarySecurityValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillarySecurityValidatorTest);

  CPPUNIT_TEST(testCheckGds_Pass_Empty);
  CPPUNIT_TEST(testCheckGds_Pass_SABRE1B);
  CPPUNIT_TEST(testCheckGds_Pass_SABRE1J);
  CPPUNIT_TEST(testCheckGds_Pass_SABRE1F);
  CPPUNIT_TEST(testCheckGds_Pass_OtherCode);
  CPPUNIT_TEST(testCheckGds_Fail_SABRE1B);
  CPPUNIT_TEST(testCheckGds_Fail_SABRE1J);
  CPPUNIT_TEST(testCheckGds_Fail_SABRE1F);
  CPPUNIT_TEST(testCheckGds_Fail_OtherCode);
  CPPUNIT_TEST(testCheckGds_Pass_OtherCode_Vendor);
  CPPUNIT_TEST(testCheckGds_Fail_OtherCode_Vendor);
  CPPUNIT_TEST(testCheckGds_Pass_OtherCode_Carrier);
  CPPUNIT_TEST(testCheckGds_Fail_OtherCode_Carrier);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<AncillaryPricingTrx>();
    _billing = _memHandle.create<Billing>();
    _trx->billing() = _billing;
    _trx->setRequest(_memHandle.create<AncRequest>());
    _agent = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent() = _agent;
    _validator = _memHandle.insert(new BaggageAncillarySecurityValidator(
        *_trx, _travelSegs.begin(), _travelSegs.end(), false, true));
  }

  void tearDown()
  {
    _travelSegs.clear();
    _memHandle.clear();
  }

  TestMemHandle _memHandle;
  BaggageAncillarySecurityValidator* _validator;
  AncillaryPricingTrx* _trx;
  std::vector<TravelSeg*> _travelSegs;
  Billing* _billing;
  Agent* _agent;

public:
  // TESTS
  void testCheckGds_Pass_Empty()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "";

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Pass_SABRE1B()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1S;
    _agent->cxrCode() = RuleConst::SABRE1B;

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Pass_SABRE1J()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1S;
    _agent->cxrCode() = RuleConst::SABRE1J;

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Pass_SABRE1F()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = RuleConst::SABRE1S;
    _agent->cxrCode() = RuleConst::SABRE1F;

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Pass_OtherCode()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _billing->partitionID() = "XX";

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Fail_SABRE1B()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _billing->partitionID() = RuleConst::SABRE1B;

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }

  void testCheckGds_Fail_SABRE1J()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _billing->partitionID() = RuleConst::SABRE1J;

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }

  void testCheckGds_Fail_SABRE1F()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _billing->partitionID() = RuleConst::SABRE1F;

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }

  void testCheckGds_Fail_OtherCode()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _billing->partitionID() = "ZZ";

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }

  void testCheckGds_Pass_OtherCode_Vendor()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _agent->cxrCode() = "XX";

    _validator->_validateCharges = false;

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Fail_OtherCode_Vendor()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _agent->vendorCrsCode() = "ZZ";

    _validator->_validateCharges = false;

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }

  void testCheckGds_Pass_OtherCode_Carrier()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _agent->cxrCode() = "XX";

    _validator->_validateCharges = false;

    CPPUNIT_ASSERT(_validator->checkGds(&info));
  }

  void testCheckGds_Fail_OtherCode_Carrier()
  {
    SvcFeesSecurityInfo info;
    info.carrierGdsCode() = "XX";
    _agent->cxrCode() = "ZZ";

    _validator->_validateCharges = false;

    CPPUNIT_ASSERT(!_validator->checkGds(&info));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillarySecurityValidatorTest);
} // tse
