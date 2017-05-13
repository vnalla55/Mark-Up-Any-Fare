/**
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "RequestResponse/RequestResponseService.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TseServerStub.h"

namespace tse
{

class RequestResponseServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequestResponseServiceTest);
  CPPUNIT_TEST(createTest);
  CPPUNIT_TEST(acceptsPricingTransaction);
  CPPUNIT_TEST(buildReqRespOACTest);
  CPPUNIT_TEST_SUITE_END();

  RequestResponseService* _service;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<RootLoggerGetOff>();
    _service =
        new RequestResponseService("RequestResponseService", *_memHandle.create<TseServerStub>());
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _service;
  }

  void createTest() { CPPUNIT_ASSERT_MESSAGE("Service not created", _service); }

  void acceptsPricingTransaction()
  {
    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    bool result = _service->process(trx);
    CPPUNIT_ASSERT(result); // Only check if service can be called with trx;
  }

  void buildReqRespOACTest()
  {
    const size_t MAX_STAT_DATA_OFFSET = 76; // TseSrvStats enum STATS

    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    trx.statData().assign(MAX_STAT_DATA_OFFSET, 0.0);

    Loc loc;
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;

    Agent agent;

    agent.agentLocation() = &loc;
    agent.agentCity() = "DFW";
    agent.tvlAgencyPCC() = "HDQ";
    agent.mainTvlAgencyPCC() = "HDQ";
    agent.tvlAgencyIATA() = "XYZ";
    agent.homeAgencyIATA() = "XYZ";
    agent.agentFunctions() = "XYZ";
    agent.agentDuty() = "XYZ";
    agent.airlineDept() = "XYZ";
    agent.cxrCode() = "AA";
    agent.currencyCodeAgent() = "USD";
    agent.coHostID() = 9;
    agent.agentCommissionType() = "PERCENT";
    agent.agentCommissionAmount() = 10;

    Billing billing;
    billing.userPseudoCityCode() = "ABC";
    billing.userStation() = "ABC";
    billing.userBranch() = "ABC";
    billing.partitionID() = "ABC";
    billing.userSetAddress() = "ABC";
    billing.serviceName() = "ABC";
    billing.aaaCity() = "ABC";
    billing.aaaSine() = "ABC";
    billing.actionCode() = "ABC";

    const std::string FareSearchResponse("FSR");

    XMLConstruct xml;
    xml.openElement(FareSearchResponse);

    std::string startTime, endTime;
    _service->buildReqRespTrxPart(xml, trx, billing, &agent, startTime, endTime);

    xml.closeElement();
    size_t found = xml.getXMLData().find("A20=\"ABC\"");
    CPPUNIT_ASSERT((found != std::string::npos));

    XMLConstruct xmlWithOAC;
    xmlWithOAC.openElement(FareSearchResponse);
    agent.officeDesignator() = "LA";

    _service->buildReqRespTrxPart(xmlWithOAC, trx, billing, &agent, startTime, endTime);
    xmlWithOAC.closeElement();
    found = xmlWithOAC.getXMLData().find("A20=\"LA\"");
    CPPUNIT_ASSERT((found != std::string::npos));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequestResponseServiceTest);

} // tse
