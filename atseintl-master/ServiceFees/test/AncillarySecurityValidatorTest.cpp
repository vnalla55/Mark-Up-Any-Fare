#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include <boost/assign/std/vector.hpp>
#include "ServiceFees/AncillarySecurityValidator.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockGlobal.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TrxUtil.h"
#include "DBAccess/Customer.h"
#include "DataModel/Agent.h"
#include "Rules/RuleConst.h"
#include "Diagnostic/Diag877Collector.h"

#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
const string LOC_NYCU = "/vobs/atseintl/test/testdata/data/LocNYC.xml";
const string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const string LOC_FRA = "/vobs/atseintl/test/testdata/data/LocFRA.xml";
const string LOC_DEN = "/vobs/atseintl/test/testdata/data/LocDEN.xml";

//---------------------------------------------------------
class AncillarySecurityValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillarySecurityValidatorTest);

  CPPUNIT_TEST(testCheckDutyCodeReturnTrueWhenGDSEmpty);
  CPPUNIT_TEST(testCheckDutyCodeReturnTrueWhenGDSAndDutyCodeEmpty);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_Less_than_9);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_A_and_Agent_Not_$);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_A_and_Agent_Is_$_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_A_and_Agent_Is_$_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_B_and_Agent_Is_AMP_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_B_and_Agent_Is_AMP_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_B_and_Agent_Is_$_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_C_and_Agent_Is_STAR_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_C_and_Agent_Is_STAR_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_D_and_Agent_Is_DASH_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_D_and_Agent_Is_DASH_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnFALSE_When_S7_dutyCode_E_and_Agent_Is_SLASH_FunctionNotMatch);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_E_and_Agent_Is_SLASH_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnFAIL_When_S7_dutyCode_F_and_Agent_Is_SLASH_Function_Match);
  CPPUNIT_TEST(testCheckDutyCodeReturnPASS_When_S7_dutyCode_F_and_Agent_Is_F_Function_Match);
  CPPUNIT_TEST(
      testCheckDutyCodeReturnPASS_When_S7_dutyCode_F_and_No_Function_and_Agent_Is_F_NO_Function_Match);

  CPPUNIT_TEST(testCreateDiagGetsCreated);
  CPPUNIT_TEST(testCreateDiagDisplaysHeader);
  CPPUNIT_TEST(testDetailDiagForCoreDumps);
  CPPUNIT_TEST(testMatchFareMarketInRequest_Fail);
  CPPUNIT_TEST(testMatchFareMarketInRequest_PASS);

  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _configMan = _memHandle.create<ConfigMan>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<AncRequest>());
    _agent.agentLocation() = TestLocFactory::create(LOC_NYCU);
    _agent.tvlAgencyPCC() = "W0H3";
    _agent.agentTJR() = &_customer;
    _trx->getRequest()->ticketingAgent() = &_agent;

    _travelSegs = _memHandle.create<vector<TravelSeg*> >();
    _seg1 = _memHandle.create<AirSeg>();
    _seg2Arunk = _memHandle.create<ArunkSeg>();
    _seg3 = _memHandle.create<AirSeg>();
    _travelSegs->push_back(_seg1);
    _travelSegs->push_back(_seg2Arunk);
    _travelSegs->push_back(_seg3);
    _segI = _travelSegs->begin();
    _segIE = _travelSegs->end();

    _secInfo = _memHandle.create<SvcFeesSecurityInfo>();
    _secInfo->travelAgencyInd() = SecurityValidator::MUST_BE_TVL_AGENCY;
    _secInfo->carrierGdsCode() = SABRE_MULTIHOST_ID;
    _secInfo->loc().locType() = LOCTYPE_NATION;
    _secInfo->loc().loc() = "US";
    _secInfo->code().locType() = RuleConst::TRAVEL_AGENCY;
    _secInfo->code().loc() = "W0H3";
    createValidator();
  }

  //---------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------

  void createValidator()
  {
    _validator = _memHandle.insert(new AncillarySecurityValidator(*_trx, _segI, _segIE));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnTrueWhenGDSEmpty()
  {
    _secInfo->carrierGdsCode().clear();
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnTrueWhenGDSAndDutyCodeEmpty()
  {
    _secInfo->carrierGdsCode().clear();
    _secInfo->dutyFunctionCode().clear();
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_Less_than_9()
  {
    _secInfo->dutyFunctionCode() = "0A";
    _agent.agentDuty() = "@A";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_A_and_Agent_Not_$()
  {
    _secInfo->dutyFunctionCode() = "A5";
    _agent.agentDuty() = "AA";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_A_and_Agent_Is_$_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "AU";
    _agent.agentDuty() = "$";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }
  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_A_and_Agent_Is_$_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "AS";
    _agent.agentDuty() = "$";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_B_and_Agent_Is_AMP_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "BU";
    _agent.agentDuty() = "@";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }
  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_B_and_Agent_Is_AMP_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "BS";
    _agent.agentDuty() = "@";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_B_and_Agent_Is_$_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "BU";
    _agent.agentDuty() = "$";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_C_and_Agent_Is_STAR_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "CU";
    _agent.agentDuty() = "*";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }
  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_C_and_Agent_Is_STAR_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "CS";
    _agent.agentDuty() = "*";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_D_and_Agent_Is_DASH_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "DU";
    _agent.agentDuty() = "-";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }
  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_D_and_Agent_Is_DASH_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "DS";
    _agent.agentDuty() = "-";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  void testCheckDutyCodeReturnFALSE_When_S7_dutyCode_E_and_Agent_Is_SLASH_FunctionNotMatch()
  {
    _secInfo->dutyFunctionCode() = "EU";
    _agent.agentDuty() = "/";
    _agent.agentFunctions() = "5";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }
  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_E_and_Agent_Is_SLASH_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "ES";
    _agent.agentDuty() = "/";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnFAIL_When_S7_dutyCode_F_and_Agent_Is_SLASH_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "FS";
    _agent.agentDuty() = "/";
    _agent.agentFunctions() = "S";
    CPPUNIT_ASSERT(!_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCheckDutyCodeReturnPASS_When_S7_dutyCode_F_and_Agent_Is_F_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "FS5";
    _agent.agentDuty() = "F987";
    _agent.agentFunctions() = "S678";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void
  testCheckDutyCodeReturnPASS_When_S7_dutyCode_F_and_No_Function_and_Agent_Is_F_NO_Function_Match()
  {
    _secInfo->dutyFunctionCode() = "F";
    _agent.agentDuty() = "F987";
    _agent.agentFunctions() = "S678";
    CPPUNIT_ASSERT(_validator->checkDutyCode(_secInfo));
  }

  //---------------------------------------------------------
  void testCreateDiagGetsCreated()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "100"));
    _validator->createDiag(100, 101);
    CPPUNIT_ASSERT(_validator->_diag != (SvcFeesDiagCollector*)(0));
  }

  //---------------------------------------------------------
  void testCreateDiagDisplaysHeader()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("DD", "INFO"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "100"));
    _validator->createDiag(100, 101);

    string expected;
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " * SECURITY T183 ITEM NO : 101     DETAIL INFO       * \n";
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n";

    string actual = _validator->_diag->str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  //---------------------------------------------------------
  void testDetailDiagForCoreDumps()
  {
    Diag877Collector dc;
    _validator->_diag = &dc;
    CPPUNIT_ASSERT_NO_THROW(_validator->detailDiag(_secInfo, PASS_T183));
  }

  //---------------------------------------------------------
  void testMatchFareMarketInRequest_Fail()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWFRA"));
    _seg1->boardMultiCity() = "DEN";
    _seg1->origin() = TestLocFactory::create(LOC_DEN);
    _seg1->offMultiCity() = "FRA";
    _seg3->destination() = TestLocFactory::create(LOC_FRA);

    CPPUNIT_ASSERT(!_validator->matchFareMarketInRequest());
  }

  //---------------------------------------------------------
  void testMatchFareMarketInRequest_PASS()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagParamMap().insert(std::make_pair("FM", "DFWFRA"));
    _seg1->boardMultiCity() = "DFW";
    _seg1->origin() = TestLocFactory::create(LOC_DFW);
    _seg1->offMultiCity() = "FRA";
    _seg3->destination() = TestLocFactory::create(LOC_FRA);

    CPPUNIT_ASSERT(_validator->matchFareMarketInRequest());
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  Agent _agent;
  Customer _customer;
  AncillarySecurityValidator* _validator;
  Loc _agentLoc;
  SvcFeesSecurityInfo* _secInfo;
  ConfigMan* _configMan;

  vector<TravelSeg*>* _travelSegs;
  AirSeg* _seg1;
  ArunkSeg* _seg2Arunk;
  AirSeg* _seg3;

  vector<TravelSeg*>::const_iterator _segI;
  vector<TravelSeg*>::const_iterator _segIE;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AncillarySecurityValidatorTest);
}
