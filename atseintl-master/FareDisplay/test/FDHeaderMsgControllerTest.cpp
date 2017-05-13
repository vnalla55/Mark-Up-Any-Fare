#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/Config/ConfigMan.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FDHeaderMsg.h"
#include "DBAccess/Loc.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/FDHeaderMsgController.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class FDHeaderMsgControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDHeaderMsgControllerTest);
  CPPUNIT_TEST(testisEliminateOnCarrier);
  CPPUNIT_TEST(testisEliminateOnCarrier_FailEmptyPreferredCarriers);
  CPPUNIT_TEST(testisEliminateOnCarrier_FailEmptyHeaderMsg);
  CPPUNIT_TEST(testisEliminateOnCarrier_FailOnFound);
  CPPUNIT_TEST(testisEliminateOnRouting_FailNoRouting);
  CPPUNIT_TEST(testisEliminateOnRouting_PassRouting1);
  CPPUNIT_TEST(testisEliminateOnRouting_FailRouting1);
  CPPUNIT_TEST(testisEliminateOnRouting_PassRouting2);
  CPPUNIT_TEST(testisEliminateOnRouting_FailRouting2);
  CPPUNIT_TEST(testisEliminateOnGlobalDirection_FailEmpty);
  CPPUNIT_TEST(testisEliminateOnGlobalDirection_FailEqual);
  CPPUNIT_TEST(testisEliminateOnGlobalDirection);
  CPPUNIT_TEST(testisEliminateOnPosLoc);
  CPPUNIT_TEST(testisEliminateOnPosLoc_FailNoMatch);
  CPPUNIT_TEST(testisEliminateOnPosLoc_FailNoAgentLoc);
  CPPUNIT_TEST(testisEliminateOnPosLoc_FailNoAgent);
  CPPUNIT_TEST(testisEliminateOnCurrency_PassNo);
  CPPUNIT_TEST(testisEliminateOnCurrency_PassYes);
  CPPUNIT_TEST(testisEliminateOnCurrency_FailNo);
  CPPUNIT_TEST(testisEliminateOnCurrency_FailYes);
  CPPUNIT_TEST(testisEliminateOnCurrency_FailNoAgent);
  CPPUNIT_TEST(testisEliminateOnInclusionCode);
  CPPUNIT_TEST(testisEliminateOnInclusionCode_FailEmpty);
  CPPUNIT_TEST(testisEliminateOnInclusionCode_FailEqual);
  CPPUNIT_TEST(testeliminateRows_Pass);
  CPPUNIT_TEST(testeliminateRows_Fail);
  CPPUNIT_TEST(testeliminateRows_FailEmpty);
  CPPUNIT_TEST_SUITE_END();

  FDHeaderMsgController* _fdHmc;
  FareDisplayTrx* _trx;
  std::set<CarrierCode>* _preferredCarriers;
  FDHeaderMsg* _headerMsg;
  Agent* _agent;
  TestMemHandle _memHandle;
  ConfigMan* _configMan;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _trx->billing() = _memHandle.create<Billing>();
    FareDisplayRequest* fdRequest = _memHandle.create<FareDisplayRequest>();
    _agent = _memHandle.create<Agent>();
    Loc* agentLocation = _memHandle.create<Loc>();
    agentLocation->loc() = "AGT";
    _agent->currencyCodeAgent() = "AGT";
    _agent->agentLocation() = agentLocation;
    _agent->agentTJR() = _memHandle.create<Customer>();
    fdRequest->ticketingAgent() = _agent;
    _trx->setRequest(fdRequest);
    _trx->setOptions(_memHandle.create<FareDisplayOptions>());
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = agentLocation;
    seg->destination() = agentLocation;
    _trx->travelSeg().push_back(seg);
    FareMarket* fm = _memHandle.create<FareMarket>();
    _trx->fareMarket().push_back(fm);

    _preferredCarriers = _memHandle.create<std::set<CarrierCode> >();
    _headerMsg = _memHandle.create<FDHeaderMsg>();
    _fdHmc = _memHandle.insert(new FDHeaderMsgController(*_trx, *_preferredCarriers));

    _preferredCarriers->insert("C1");
    _preferredCarriers->insert("C2");
    _headerMsg->carrier() = "C3";
    _headerMsg->routing1() = "2";
    _headerMsg->exceptPosLoc() = NO;
    _headerMsg->posLoc() = "MSG";
    _headerMsg->posLocType() = LOCTYPE_CITY;
    _headerMsg->exceptCur() = NO;
    _headerMsg->cur() = "MSG";
    _headerMsg->exceptLoc() = YES;
    fm->governingCarrier() = "C1";

    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->routingNumber() = "7";
    fi->globalDirection() = GlobalDirection::AF;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->setIsShoppingFare(); // Set it true so that isValid() returns true
    _trx->allPaxTypeFare().push_back(ptf);
  }

  void tearDown() { _memHandle.clear(); }

  void testisEliminateOnCarrier() { CPPUNIT_ASSERT(_fdHmc->isEliminateOnCarrier(_headerMsg)); }

  void testisEliminateOnCarrier_FailEmptyPreferredCarriers()
  {
    _preferredCarriers->clear();
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCarrier(_headerMsg));
  }

  void testisEliminateOnCarrier_FailEmptyHeaderMsg()
  {
    _headerMsg->carrier() = "";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCarrier(_headerMsg));
  }

  void testisEliminateOnCarrier_FailOnFound()
  {
    _headerMsg->carrier() = "C1";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCarrier(_headerMsg));
  }

  void testisEliminateOnRouting_FailNoRouting()
  {
    _headerMsg->routing1() = "";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnRouting(_headerMsg));
  }

  void testisEliminateOnRouting_PassRouting1()
  {
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnRouting(_headerMsg));
  }

  void testisEliminateOnRouting_FailRouting1()
  {
    _headerMsg->routing1() = "7";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnRouting(_headerMsg));
  }

  void testisEliminateOnRouting_PassRouting2()
  {
    _headerMsg->routing2() = "5";
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnRouting(_headerMsg));
  }

  void testisEliminateOnRouting_FailRouting2()
  {
    _headerMsg->routing2() = "100";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnRouting(_headerMsg));
  }

  void testisEliminateOnGlobalDirection_FailEmpty()
  {
    _headerMsg->globalDir() = GlobalDirection::ZZ;
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnGlobalDirection(_headerMsg));
  }

  void testisEliminateOnGlobalDirection_FailEqual()
  {
    _headerMsg->globalDir() = GlobalDirection::AF;
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnGlobalDirection(_headerMsg));
  }

  void testisEliminateOnGlobalDirection()
  {
    _headerMsg->globalDir() = GlobalDirection::AT;
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnGlobalDirection(_headerMsg));
  }

  void testisEliminateOnPosLoc() { CPPUNIT_ASSERT(_fdHmc->isEliminateOnPosLoc(_headerMsg)); }

  void testisEliminateOnPosLoc_FailNoMatch()
  {
    _headerMsg->exceptPosLoc() = YES;
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnPosLoc(_headerMsg));
  }

  void testisEliminateOnPosLoc_FailNoAgentLoc()
  {
    _agent->agentLocation() = NULL;
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnPosLoc(_headerMsg));
  }

  void testisEliminateOnPosLoc_FailNoAgent()
  {
    FareDisplayRequest fdRequest;
    _trx->setRequest(&fdRequest);
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnPosLoc(_headerMsg));
  }

  void testisEliminateOnCurrency_PassNo()
  {
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnCurrency(_headerMsg));
  }

  void testisEliminateOnCurrency_PassYes()
  {
    _agent->currencyCodeAgent() = "MSG";
    _headerMsg->exceptCur() = YES;
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnCurrency(_headerMsg));
  }

  void testisEliminateOnCurrency_FailNo()
  {
    _agent->currencyCodeAgent() = "MSG";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCurrency(_headerMsg));
  }

  void testisEliminateOnCurrency_FailYes()
  {
    _headerMsg->exceptCur() = YES;
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCurrency(_headerMsg));
  }

  void testisEliminateOnCurrency_FailNoAgent()
  {
    FareDisplayRequest fdRequest;
    _trx->setRequest(&fdRequest);

    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnCurrency(_headerMsg));
  }

  void testisEliminateOnInclusionCode()
  {
    _headerMsg->inclusionCode() = "MSG";
    CPPUNIT_ASSERT(_fdHmc->isEliminateOnInclusionCode(_headerMsg, "INC"));
  }

  void testisEliminateOnInclusionCode_FailEmpty()
  {
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnInclusionCode(_headerMsg, "INC"));
  }

  void testisEliminateOnInclusionCode_FailEqual()
  {
    _headerMsg->inclusionCode() = "INC";
    CPPUNIT_ASSERT(!_fdHmc->isEliminateOnInclusionCode(_headerMsg, "INC"));
  }

  void testeliminateRows_Pass()
  {
    std::vector<const tse::FDHeaderMsg*> fdHeaderMsgDataList;
    std::vector<const tse::FDHeaderMsg*> fdFilteredHdrMsg;
    fdHeaderMsgDataList.push_back(_headerMsg);

    _headerMsg->carrier() = "";
    _headerMsg->routing1() = "";
    _headerMsg->globalDir() = GlobalDirection::ZZ;
    _agent->agentLocation() = NULL;
    _headerMsg->exceptCur() = YES;

    CPPUNIT_ASSERT(_fdHmc->eliminateRows(fdHeaderMsgDataList, fdFilteredHdrMsg));
    CPPUNIT_ASSERT(!fdFilteredHdrMsg.empty());
  }

  void testeliminateRows_Fail()
  {
    std::vector<const tse::FDHeaderMsg*> fdHeaderMsgDataList;
    std::vector<const tse::FDHeaderMsg*> fdFilteredHdrMsg;
    fdHeaderMsgDataList.push_back(_headerMsg);

    CPPUNIT_ASSERT(_fdHmc->eliminateRows(fdHeaderMsgDataList, fdFilteredHdrMsg));
    CPPUNIT_ASSERT(fdFilteredHdrMsg.empty());
  }

  void testeliminateRows_FailEmpty()
  {
    std::vector<const tse::FDHeaderMsg*> fdHeaderMsgDataList;
    std::vector<const tse::FDHeaderMsg*> fdFilteredHdrMsg;

    CPPUNIT_ASSERT(!_fdHmc->eliminateRows(fdHeaderMsgDataList, fdFilteredHdrMsg));
    CPPUNIT_ASSERT(fdFilteredHdrMsg.empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDHeaderMsgControllerTest);
}
