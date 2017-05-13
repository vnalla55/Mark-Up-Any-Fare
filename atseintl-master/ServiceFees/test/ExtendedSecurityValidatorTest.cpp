#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Customer.h"
#include "Rules/RuleConst.h"
#include "Diagnostic/Diag870Collector.h"
#include "ServiceFees/ExtendedSecurityValidator.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Common/TrxUtil.h"

namespace tse
{
using namespace std;
namespace
{
class ExtendedSecurityValidatorMock : public tse::ExtendedSecurityValidator
{
public:
  ExtendedSecurityValidatorMock(PricingTrx& trx,
                                const std::vector<TravelSeg*>::const_iterator segI,
                                const std::vector<TravelSeg*>::const_iterator segIE)
    : ExtendedSecurityValidator(trx, segI, segIE), passLoc(false), noT183Sequences(false)
  {
  }

  ~ExtendedSecurityValidatorMock()
  {
    std::vector<SvcFeesSecurityInfo*>::const_iterator secI = _secInfoVec.begin();
    const std::vector<SvcFeesSecurityInfo*>::const_iterator secEnd = _secInfoVec.end();

    for (; secI != secEnd; ++secI)
      delete *secI;
  }

  const Loc* locationOverride(const LocCode& loc) const
  {
    if (!_trx.getRequest()->ticketPointOverride().empty())
      return &ticketPointOverride;

    return _trx.getRequest()->ticketingAgent()->agentLocation();
  }

  bool isInLoc(const SvcFeesSecurityInfo* secInfo, const Loc& loc) const { return passLoc; }

  const std::vector<SvcFeesSecurityInfo*>& getSecurityInfo(int t183ItemNo)
  {
    if (!noT183Sequences)
    {
      SvcFeesSecurityInfo* _secInfo = new SvcFeesSecurityInfo();
      _secInfo->travelAgencyInd() = 'X';
      _secInfo->carrierGdsCode() = SABRE_MULTIHOST_ID;
      _secInfo->loc().locType() = LOCTYPE_NATION;
      _secInfo->loc().loc() = "US";
      _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
      _secInfo->code().loc() = "W0H3";
      _secInfoVec.push_back(_secInfo);
    }
    return _secInfoVec;
  }
  Loc ticketPointOverride;
  bool passLoc;
  std::vector<SvcFeesSecurityInfo*> _secInfoVec;
  bool noT183Sequences;
};
}

class ExtendedSecurityValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExtendedSecurityValidatorTest);
  // remove all codes below with the fallback
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenGDSEmpty);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAbacusAndGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAbacusAndGDSAbacus);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAbacusAndGDSAxess);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAxessAndGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAxessAndGDSAxess);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAxessAndGDSAbacus);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenCarrierCodeMatchesPartitionID);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenCarrierCodeNotMatchesPartitionID);
  // remove all codes above with the fallback

  CPPUNIT_TEST(testCheckDutyCodeReturnTrueWhenDutyCodeEmpty);

  // remove all codes below with the fallback
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenNotAllowedTypeCodesPresent);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeAirlineDeptButNoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeAirlineDeptAndMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeIATAandMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeNotDefined);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenCarrierGdsCodeIsBlank);
  // remove all codes above with the fallback

  CPPUNIT_TEST(testCheckViewIndicatorReturnFalseWhenViewBookIndNotValid);
  CPPUNIT_TEST(testCheckViewIndicatorReturnTrueWhenViewBookIndValid);

  CPPUNIT_TEST(testValidateSequenceReturnFAIL_TVL_AGENCYwhenTvlAgencyFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_CXR_GDSwhenCheckGdsFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_DUTYwhenCheckDutyFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_GEOwhenCheckLocFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_TYPE_CODEwhenCheckCodeFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_VIEW_INDwhenViewIndFail);
  CPPUNIT_TEST(testValidateSequenceReturnPASS_T183whenSequencePass);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    TrxUtil::enableAbacus();
    _agent.agentLocation() = &_agentLoc;
    _agent.tvlAgencyPCC() = "W0H3";
    _agent.airlineDept() = "TEST";
    _agent.agentTJR() = &_customer;
    _request.ticketingAgent() = &_agent;
    _trx.setRequest(&_request);
    _trx.billing() = &_billing;
    _secInfo = _memHandle.create<SvcFeesSecurityInfo>();
    _secInfo->travelAgencyInd() = SecurityValidator::MUST_BE_TVL_AGENCY;
    _secInfo->carrierGdsCode() = SABRE_MULTIHOST_ID;
    _secInfo->loc().locType() = LOCTYPE_NATION;
    _secInfo->loc().loc() = "US";
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "W0H3";
    _view = false;
    _travelSegs = _memHandle.create<vector<TravelSeg*> >();
    _seg1 = _memHandle.create<AirSeg>();
    _travelSegs->push_back(_seg1);
    _segI = _travelSegs->begin();
    _segIE = _travelSegs->end();
  }

  void tearDown() { _memHandle.clear(); }

  // remove all codes below with the fallback

  void setAbacus()
  {
    _agent.agentTJR()->crsCarrier() = "1B";
    _agent.agentTJR()->hostName() = "ABAC";
  }

  void setAxess()
  {
    _agent.agentTJR()->crsCarrier() = "1J";
    _agent.agentTJR()->hostName() = "AXES";
  }

  void testCheckGdsReturnTrueWhenGDSEmpty()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode().clear();
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenGDSSabre()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAbacusAndGDSSabre()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAbacusAndGDSAbacus()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAbacusAndGDSAxess()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAxessAndGDSSabre()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAxessAndGDSAxess()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAxessAndGDSAbacus()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenCarrierCodeMatchesPartitionID()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC() = "";
    _secInfo->carrierGdsCode() = "AA";
    _billing.partitionID() = "AA";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenCarrierCodeNotMatchesPartitionID()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC() = "";
    _secInfo->carrierGdsCode() = "AA";
    _billing.partitionID() = "BB";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }
  // remove all codes above with the fallback

  void testCheckDutyCodeReturnTrueWhenDutyCodeEmpty()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->dutyFunctionCode().clear();
    CPPUNIT_ASSERT_EQUAL(true, sv.checkDutyCode(_secInfo));
  }

  // remove all codes below with the fallback

  void testCheckCodeReturnFalseWhenNotAllowedTypeCodesPresent()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_X;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_A;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::ERSP_NUMBER;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::LNIATA_NUMBER;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeAirlineDeptButNoMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->code().loc() = "XXXX";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeAirlineDeptAndMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode()[0] = '2';
    _secInfo->code().loc() = "TEST";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "5MWA";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "W0H3";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA().clear();
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeIATAandMatch()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA() = "1234567";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeNotDefined()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = 'H';
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenCarrierGdsCodeIsBlank()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode() = "";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }
  // remove all codes above with the fallback

  void testCheckViewIndicatorReturnFalseWhenViewBookIndNotValid()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->viewBookTktInd() = '2';
    CPPUNIT_ASSERT_EQUAL(false, sv.checkViewIndicator(_secInfo));
  }

  void testCheckViewIndicatorReturnTrueWhenViewBookIndValid()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->viewBookTktInd() = '1';
    CPPUNIT_ASSERT_EQUAL(true, sv.checkViewIndicator(_secInfo));
  }

  void testValidateSequenceReturnFAIL_TVL_AGENCYwhenTvlAgencyFail()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC().clear();
    CPPUNIT_ASSERT_EQUAL(FAIL_TVL_AGENCY, sv.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnFAIL_CXR_GDSwhenCheckGdsFail()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode() = ABACUS_MULTIHOST_ID;
    CPPUNIT_ASSERT_EQUAL(FAIL_CXR_GDS, sv.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnFAIL_DUTYwhenCheckDutyFail()
  {
    tse::ExtendedSecurityValidator sv(_trx, _segI, _segIE);
    _agent.agentDuty() = "*";
    _secInfo->dutyFunctionCode() = "A";
    CPPUNIT_ASSERT_EQUAL(FAIL_DUTY, sv.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnFAIL_GEOwhenCheckLocFail()
  {
    ExtendedSecurityValidatorMock svMock(_trx, _segI, _segIE);
    svMock.passLoc = false;
    CPPUNIT_ASSERT_EQUAL(FAIL_GEO, svMock.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnFAIL_TYPE_CODEwhenCheckCodeFail()
  {
    ExtendedSecurityValidatorMock svMock(_trx, _segI, _segIE);
    svMock.passLoc = true;
    _agent.tvlAgencyPCC() = "5MWA";
    CPPUNIT_ASSERT_EQUAL(FAIL_TYPE_CODE, svMock.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnFAIL_VIEW_INDwhenViewIndFail()
  {
    ExtendedSecurityValidatorMock svMock(_trx, _segI, _segIE);
    svMock.passLoc = true;
    _secInfo->viewBookTktInd() = '2';
    CPPUNIT_ASSERT_EQUAL(FAIL_VIEW_IND, svMock.validateSequence(_secInfo, _view));
  }

  void testValidateSequenceReturnPASS_T183whenSequencePass()
  {
    ExtendedSecurityValidatorMock svMock(_trx, _segI, _segIE);
    svMock.passLoc = true;
    CPPUNIT_ASSERT_EQUAL(PASS_T183, svMock.validateSequence(_secInfo, _view));
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx _trx;
  PricingRequest _request;
  Billing _billing;
  Agent _agent;
  Loc _agentLoc;
  Customer _customer;
  SvcFeesSecurityInfo* _secInfo;
  bool _view;
  vector<TravelSeg*>::const_iterator _segI;
  vector<TravelSeg*>::const_iterator _segIE;
  vector<TravelSeg*>* _travelSegs;
  AirSeg* _seg1;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExtendedSecurityValidatorTest);
}
