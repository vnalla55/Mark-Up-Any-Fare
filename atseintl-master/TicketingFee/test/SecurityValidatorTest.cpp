#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Rules/RuleConst.h"
#include "Diagnostic/Diag870Collector.h"
#include "Diagnostic/Diag877Collector.h"
#include "TicketingFee/SecurityValidator.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/Customer.h"
#include "Common/TrxUtil.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
const string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const string LOC_FRA = "/vobs/atseintl/test/testdata/data/LocFRA.xml";
const string LOC_DEN = "/vobs/atseintl/test/testdata/data/LocDEN.xml";

class SecurityValidator;
//---------------------------------------------------------
class SecurityValidatorStub : public SecurityValidator
{
public:
  SecurityValidatorStub(PricingTrx& trx,
                        const std::vector<TravelSeg*>::const_iterator segI,
                        const std::vector<TravelSeg*>::const_iterator segIE)
    : SecurityValidator(trx, segI, segIE), passLoc(false), noT183Sequences(false)
  {
  }

  ~SecurityValidatorStub()
  {
    std::vector<SvcFeesSecurityInfo*>::const_iterator secI = _secInfoVec.begin();
    const std::vector<SvcFeesSecurityInfo*>::const_iterator secEnd = _secInfoVec.end();
    for (; secI != secEnd; ++secI)
      delete *secI;
  }

  //---------------------------------------------------------
  const Loc* locationOverride(const LocCode& loc) const
  {
    if (!_trx.getRequest()->ticketPointOverride().empty())
      return &ticketPointOverride;

    return _trx.getRequest()->ticketingAgent()->agentLocation();
  }

  //---------------------------------------------------------
  bool isInLoc(const SvcFeesSecurityInfo* secInfo, const Loc& loc) const { return passLoc; }

  //---------------------------------------------------------
  const std::vector<SvcFeesSecurityInfo*>& getSecurityInfo(VendorCode vc, int t183ItemNo)
  {
    if (!noT183Sequences)
    {
      SvcFeesSecurityInfo* _secInfo = new SvcFeesSecurityInfo();
      _secInfo->vendor() = ATPCO_VENDOR_CODE;
      if (_secInfo->vendor() != vc)
        _secInfo->vendor() = vc;
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

//---------------------------------------------------------
class SecurityValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SecurityValidatorTest);
  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testCheckTvlAgencyReturnTrueWhenTvlAgencyNotX);
  CPPUNIT_TEST(testCheckTvlAgencyReturnTrueWhenTvlAgencyXAndEntryFromPCC);
  CPPUNIT_TEST(testCheckTvlAgencyReturnFalseWhenTvlAgencyXAndEntryFromAirline);

  CPPUNIT_TEST(testCheckGdsReturnTrueWhenGDSEmpty);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenGDSNotSabreAndAgentIsSABRE);

  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAbacusAndGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAbacusAndGDSAbacus);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAbacusAndGDSAxess);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAbacusAndGDSInfini);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAxessAndGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenAxessAndGDSAxess);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAxessAndGDSAbacus);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenAxessAndGDSInfini);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenCarrierCodeMatchesPartitionID);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenCarrierCodeNotMatchesPartitionID);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenInfiniAndGDSSabre);
  CPPUNIT_TEST(testCheckGdsReturnTrueWhenInfiniAndGDSInfini);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenInfiniAndGDSAbacus);
  CPPUNIT_TEST(testCheckGdsReturnFalseWhenInfiniAndGDSAxess);

  CPPUNIT_TEST(testCheckDutyCodeReturnTrueWhenGDSEmpty);
  CPPUNIT_TEST(testCheckDutyCodeReturnTrueWhenGDSAndDutyCodeEmpty);

  CPPUNIT_TEST(testGetLocationReturnAgentLocationWhenNoOverride);
  CPPUNIT_TEST(testGetLocationReturnTicketOverrideWhenPresent);
  CPPUNIT_TEST(testCheckLocReturnTrueWhenLocEmpty);
  CPPUNIT_TEST(testCheckLocReturnFalseIfAgentLocationUndetermined);
  CPPUNIT_TEST(testCheckLocReturnFalseWhenAgentNotInLocation);
  CPPUNIT_TEST(testCheckLocReturnTrueWhenAgentInLocation);

  CPPUNIT_TEST(testCheckCodeReturnTrueWhenEmptyInDB);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenAirlineSpecificTypeCodesPresentOld);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatchOld);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatchOld);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatchOld);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeIATAandMatchOld);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeNotDefinedOld);

  CPPUNIT_TEST(testCheckCodeReturnFalseWhenNotAllowedTypeCodesPresent);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeAirlineDeptButNoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeAirlineDeptAndMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeOfficeDesinatorAndMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeOfficeDesinatorNotMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatch);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenTypeCodeIATAandMatch);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenTypeCodeNotDefined);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenCarrierGdsCodeIsBlank);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenChannelCodeEmpty);
  CPPUNIT_TEST(testCheckCodeReturnTrueWhenChannelCodeMatchForA);
  CPPUNIT_TEST(testCheckCodeReturnFalseWhenChannelCodeNoMatchForA);

  CPPUNIT_TEST(testValidateSequenceReturnFAIL_TVL_AGENCYwhenTvlAgencyFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_CXR_GDSwhenCheckGdsFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_DUTYwhenCheckDutyFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_GEOwhenCheckLocFail);
  CPPUNIT_TEST(testValidateSequenceReturnFAIL_TYPE_CODEwhenCheckCodeFail);
  CPPUNIT_TEST(testValidateSequenceReturnPASS_T183whenSequencePass);

  CPPUNIT_TEST(testValidateSequenceReturnPASS_T183hasViewEqual_2);
  CPPUNIT_TEST(testValidateSequenceReturnPASS_T183hasViewEqual_1);

  CPPUNIT_TEST(testValidateReturnTrueWhenSecurityPass);
  CPPUNIT_TEST(testValidateReturnTrueWhenNoSequencesFoundInTable183);
  CPPUNIT_TEST(testValidateReturnFalseWhenSecurityFail);

  CPPUNIT_TEST(testCreateDiagDoesNotCreateDiagWhenNotDiag870);
  CPPUNIT_TEST(testCreateDiagDoesNotCreateDiagWhenNotDDINFO);
  CPPUNIT_TEST(testCreateDiagDoesNotCreateDiagWhenSQNumberNotMatched);
  CPPUNIT_TEST(testCreateDiagGetsCreated);
  CPPUNIT_TEST(testCreateDiagDisplaysHeader);
  CPPUNIT_TEST(testDetailDiagForCoreDumps);
  CPPUNIT_TEST(testEndDiagForCoreDumps);

  CPPUNIT_TEST(testValidateReturnTrueWhenSecurityPass_Vendor_MMGR);
  CPPUNIT_TEST(testValidateReturnFailSecurity_Vendor_MMGR);

  CPPUNIT_TEST(testMatchFareMarketInRequest_Fail);
  CPPUNIT_TEST(testMatchFareMarketInRequest_PASS);
  CPPUNIT_TEST(testAirlineSpecificXNotMatch);

  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------
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
    _seg3 = _memHandle.create<AirSeg>();
    _travelSegs->push_back(_seg3);
    _segI = _travelSegs->begin();
    _segIE = _travelSegs->end();
  }

  //---------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------

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

  void setInfini()
  {
    _agent.agentTJR()->crsCarrier() = "1F";
    _agent.agentTJR()->hostName() = "INFI";
  }

  void testConstructor()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    CPPUNIT_ASSERT_EQUAL(&_trx, &(sv._trx));
    CPPUNIT_ASSERT_EQUAL((SvcFeesDiagCollector*)(0), sv._diag);
  }

  //---------------------------------------------------------
  void testCheckTvlAgencyReturnTrueWhenTvlAgencyNotX()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->travelAgencyInd() = ' ';
    CPPUNIT_ASSERT(sv.checkTvlAgency(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckTvlAgencyReturnTrueWhenTvlAgencyXAndEntryFromPCC()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    CPPUNIT_ASSERT(sv.checkTvlAgency(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckTvlAgencyReturnFalseWhenTvlAgencyXAndEntryFromAirline()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC().clear();
    CPPUNIT_ASSERT(!sv.checkTvlAgency(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckGdsReturnTrueWhenGDSEmpty()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode().clear();
    CPPUNIT_ASSERT(sv.checkGds(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckGdsReturnTrueWhenGDSSabre()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    CPPUNIT_ASSERT(sv.checkGds(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckGdsReturnFalseWhenGDSNotSabreAndAgentIsSABRE()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode() = ABACUS_MULTIHOST_ID;
    CPPUNIT_ASSERT(!sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAbacusAndGDSSabre()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAbacusAndGDSAbacus()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAbacusAndGDSAxess()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAbacusAndGDSInfini()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAbacus();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAxessAndGDSSabre()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenAxessAndGDSAxess()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAxessAndGDSAbacus()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenAxessAndGDSInfini()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setAxess();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenInfiniAndGDSSabre()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setInfini();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1S;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenInfiniAndGDSInfini()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setInfini();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1F;
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenInfiniAndGDSAbacus()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setInfini();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1B;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenInfiniAndGDSAxess()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    setInfini();
    _secInfo->carrierGdsCode() = RuleConst::SABRE1J;
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnTrueWhenCarrierCodeMatchesPartitionID()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC() = "";
    _secInfo->carrierGdsCode() = "AA";
    _billing.partitionID() = "AA";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkGds(_secInfo));
  }

  void testCheckGdsReturnFalseWhenCarrierCodeNotMatchesPartitionID()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC() = "";
    _secInfo->carrierGdsCode() = "AA";
    _billing.partitionID() = "BB";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkGds(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnTrueWhenGDSEmpty()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode().clear();
    CPPUNIT_ASSERT(sv.checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnTrueWhenGDSAndDutyCodeEmpty()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode().clear();
    _secInfo->dutyFunctionCode().clear();
    CPPUNIT_ASSERT(sv.checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testGetLocationReturnAgentLocationWhenNoOverride()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    const Loc* expectedLoc = &_agentLoc;
    CPPUNIT_ASSERT_EQUAL(expectedLoc, sv.getLocation());
  }

  //---------------------------------------------------------
  void testGetLocationReturnTicketOverrideWhenPresent()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    _request.ticketPointOverride() = "DEL";
    const Loc* expectedLoc = &svStub.ticketPointOverride;
    CPPUNIT_ASSERT_EQUAL(expectedLoc, svStub.getLocation());
  }

  //---------------------------------------------------------
  void testCheckLocReturnTrueWhenLocEmpty()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->loc().loc().clear();
    CPPUNIT_ASSERT(sv.checkLoc(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckLocReturnFalseIfAgentLocationUndetermined()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.agentLocation() = 0;
    CPPUNIT_ASSERT(!sv.checkLoc(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckLocReturnFalseWhenAgentNotInLocation()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = false;
    CPPUNIT_ASSERT(!svStub.checkLoc(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckLocReturnTrueWhenAgentInLocation()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    CPPUNIT_ASSERT(svStub.checkLoc(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnTrueWhenEmptyInDB()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().loc().clear();
    _secInfo->code().locType() = ' ';
    CPPUNIT_ASSERT(sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnFalseWhenAirlineSpecificTypeCodesPresentOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_X;
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_A;
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::ERSP_NUMBER;
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
    _secInfo->code().locType() = SecurityValidator::LNIATA_NUMBER;
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatchOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC() = "5MWA";
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatchOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    CPPUNIT_ASSERT(sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatchOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA().clear();
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnTrueWhenTypeCodeIATAandMatchOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA() = "1234567";
    CPPUNIT_ASSERT(sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckCodeReturnFalseWhenTypeCodeNotDefinedOld()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = 'H';
    CPPUNIT_ASSERT(!sv.checkCode(_secInfo));
  }

  //////////////////////-------------------

  void testCheckCodeReturnFalseWhenNotAllowedTypeCodesPresent()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
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
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->code().loc() = "XXXX";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeAirlineDeptAndMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode()[0] = '2';
    _secInfo->code().loc() = "TEST";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeOfficeDesinatorAndMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode()[0] = '2';
    _secInfo->code().loc() = "ARTEST";
    _agent.airlineDept() = "";
    _agent.officeDesignator() = "ARTEST";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeOfficeDesinatorNotMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode()[0] = '2';
    _secInfo->code().loc() = "ARTEST";
    _agent.airlineDept() = "";
    _agent.officeDesignator() = "AR1TEST";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeTravelAgencyButNoMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "5MWA";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeTravelAgencyAndMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "W0H3";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeIATAandButIATAnoMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA().clear();
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenTypeCodeIATAandMatch()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = RuleConst::IATA_TVL_AGENCY_NO;
    _secInfo->code().loc() = "1234567";
    _agent.tvlAgencyIATA() = "1234567";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenTypeCodeNotDefined()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = 'H';
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenCarrierGdsCodeIsBlank()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_V;
    _secInfo->carrierGdsCode() = "";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenChannelCodeEmpty()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_A;
    _secInfo->code().loc() = "RES";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnTrueWhenChannelCodeMatchForA()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_A;
    _secInfo->code().loc() = "WEB";
    _agent.airlineChannelCode() = "WEB";
    CPPUNIT_ASSERT_EQUAL(true, sv.checkCode(_secInfo));
  }

  void testCheckCodeReturnFalseWhenChannelCodeNoMatchForA()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_A;
    _secInfo->code().loc() = "WEB";
    _agent.airlineChannelCode() = "CTO";
    CPPUNIT_ASSERT_EQUAL(false, sv.checkCode(_secInfo));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnFAIL_TVL_AGENCYwhenTvlAgencyFail()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.tvlAgencyPCC().clear();
    CPPUNIT_ASSERT_EQUAL(FAIL_TVL_AGENCY, sv.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnFAIL_CXR_GDSwhenCheckGdsFail()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->carrierGdsCode() = ABACUS_MULTIHOST_ID;
    CPPUNIT_ASSERT_EQUAL(FAIL_CXR_GDS, sv.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnFAIL_DUTYwhenCheckDutyFail()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _agent.agentDuty() = "*";
    _secInfo->dutyFunctionCode() = "A";
    CPPUNIT_ASSERT_EQUAL(FAIL_DUTY, sv.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnFAIL_GEOwhenCheckLocFail()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = false;
    CPPUNIT_ASSERT_EQUAL(FAIL_GEO, svStub.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnFAIL_TYPE_CODEwhenCheckCodeFail()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    _agent.tvlAgencyPCC() = "5MWA";
    CPPUNIT_ASSERT_EQUAL(FAIL_TYPE_CODE, svStub.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnPASS_T183whenSequencePass()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    CPPUNIT_ASSERT_EQUAL(PASS_T183, svStub.validateSequence(_secInfo, _view));
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnPASS_T183hasViewEqual_2()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    _secInfo->viewBookTktInd() = '2';
    CPPUNIT_ASSERT_EQUAL(PASS_T183, svStub.validateSequence(_secInfo, _view));
    CPPUNIT_ASSERT_EQUAL(true, _view);
  }

  //---------------------------------------------------------
  void testValidateSequenceReturnPASS_T183hasViewEqual_1()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    _secInfo->viewBookTktInd() = '1';
    CPPUNIT_ASSERT_EQUAL(PASS_T183, svStub.validateSequence(_secInfo, _view));
    CPPUNIT_ASSERT_EQUAL(false, _view);
  }

  //---------------------------------------------------------
  void testValidateReturnTrueWhenSecurityPass()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    CPPUNIT_ASSERT(svStub.validate(100, 101, _view));
  }

  //---------------------------------------------------------
  void testValidateReturnTrueWhenNoSequencesFoundInTable183()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.noT183Sequences = true;
    CPPUNIT_ASSERT(!svStub.validate(100, 101, _view));
  }

  //---------------------------------------------------------
  void testValidateReturnFalseWhenSecurityFail()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = false;
    CPPUNIT_ASSERT(!svStub.validate(100, 101, _view));
  }

  //---------------------------------------------------------
  void testValidateReturnTrueWhenSecurityPass_Vendor_MMGR()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.passLoc = true;
    VendorCode vendor = "MMGR";

    CPPUNIT_ASSERT(svStub.validate(100, 101, _view, vendor));
    CPPUNIT_ASSERT_EQUAL(vendor, svStub._secInfoVec[0]->vendor());
  }

  void testValidateReturnFailSecurity_Vendor_MMGR()
  {
    SecurityValidatorStub svStub(_trx, _segI, _segIE);
    svStub.noT183Sequences = true;
    VendorCode vendor = "MMGR";

    CPPUNIT_ASSERT(!svStub.validate(100, 101, _view, vendor));
  }

  //---------------------------------------------------------
  void testCreateDiagDoesNotCreateDiagWhenNotDiag870()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = DiagnosticNone;
    sv.createDiag(100, 101);
    CPPUNIT_ASSERT_EQUAL((SvcFeesDiagCollector*)(0), sv._diag);
  }

  //---------------------------------------------------------
  void testCreateDiagDoesNotCreateDiagWhenNotDDINFO()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic870;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("DD", "NOINFO"));
    sv.createDiag(100, 101);
    CPPUNIT_ASSERT_EQUAL((SvcFeesDiagCollector*)(0), sv._diag);
  }

  //---------------------------------------------------------
  void testCreateDiagDoesNotCreateDiagWhenSQNumberNotMatched()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic870;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    _trx.diagnostic().diagParamMap().insert(std::make_pair("SQ", "99"));
    sv.createDiag(100, 101);
    CPPUNIT_ASSERT_EQUAL((SvcFeesDiagCollector*)(0), sv._diag);
  }

  //---------------------------------------------------------
  void testCreateDiagGetsCreated()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic870;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    _trx.diagnostic().diagParamMap().insert(std::make_pair("SQ", "100"));
    sv.createDiag(100, 101);
    CPPUNIT_ASSERT(sv._diag != (SvcFeesDiagCollector*)(0));
  }

  //---------------------------------------------------------
  void testCreateDiagDisplaysHeader()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic870;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    _trx.diagnostic().diagParamMap().insert(std::make_pair("SQ", "100"));
    sv.createDiag(100, 101);

    string expected;
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " * SECURITY T183 ITEM NO : 101     DETAIL INFO       * \n";
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n";

    string actual = sv._diag->str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  //---------------------------------------------------------
  void testDetailDiagForCoreDumps()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    Diag870Collector dc;
    sv._diag = &dc;
    CPPUNIT_ASSERT_NO_THROW(sv.detailDiag(_secInfo, PASS_T183));
  }

  //---------------------------------------------------------
  void testEndDiagForCoreDumps()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    Diag870Collector dc;
    sv._diag = &dc;
    CPPUNIT_ASSERT_NO_THROW(sv.endDiag());
  }

  //---------------------------------------------------------
  void testMatchFareMarketInRequest_Fail()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic877;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWFRA"));
    _seg1->boardMultiCity() = "DEN";
    _seg1->origin() = TestLocFactory::create(LOC_DEN);
    _seg1->offMultiCity() = "FRA";
    _seg3->destination() = TestLocFactory::create(LOC_FRA);

    CPPUNIT_ASSERT(!sv.matchFareMarketInRequest());
  }

  //---------------------------------------------------------
  void testMatchFareMarketInRequest_PASS()
  {
    SecurityValidator sv(_trx, _segI, _segIE);
    _trx.diagnostic().diagnosticType() = Diagnostic877;
    _trx.diagnostic().activate();
    _trx.diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWFRA"));
    _seg1->boardMultiCity() = "DFW";
    _seg1->origin() = TestLocFactory::create(LOC_DFW);
    _seg1->offMultiCity() = "FRA";
    _seg3->destination() = TestLocFactory::create(LOC_FRA);

    CPPUNIT_ASSERT(sv.matchFareMarketInRequest());
  }

  void testAirlineSpecificXNotMatch()
  {
    _agent.agentTJR() = 0;
    _request.ticketingAgent() = &_agent;
    _billing.requestPath() = AEBSO_PO_ATSE_PATH;
    _billing.partitionID() = "AA";
    SecurityValidator sv(_trx, _segI, _segIE);
    _secInfo->code().locType() = SecurityValidator::AIRLINE_SPECIFIC_X;
    CPPUNIT_ASSERT(!sv.processAirlineSpecificX(_secInfo));
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
  AirSeg* _seg3;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SecurityValidatorTest);
}
