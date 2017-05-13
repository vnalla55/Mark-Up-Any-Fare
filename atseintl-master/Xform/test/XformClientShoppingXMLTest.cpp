#include <string>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PaxType.h"
#include "DataModel/PosPaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Common/ErrorResponseException.h"
#include "Common/DateTime.h"
#include "Xform/XformClientShoppingXML.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <string>

namespace tse
{

class XformClientShoppingXMLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XformClientShoppingXMLTest);
  CPPUNIT_TEST(testConvertRequestTrx);
  CPPUNIT_TEST(testConvertRequestError);
  CPPUNIT_TEST(testValidateTrx);
  CPPUNIT_TEST(testCorpIdNotFound);
  CPPUNIT_TEST_SUITE_END();

public:
  XformClientShoppingXMLTest()
  {
    _corporateId = "TZZ01"; // TST11
  }

  void testConvertRequestTrx()
  {
    TestConfigInitializer cfg;
    XformClientShoppingXML xform("CXS_FORM", Global::config());

    DataHandle dataHandle;
    std::string request = "<ShoppingRequest><DIA Q0A=\"905\"/><BIL/></ShoppingRequest>";
    Trx* trx = NULL;

    CPPUNIT_ASSERT(xform.initialize(0, NULL));
    CPPUNIT_ASSERT_THROW(xform.convert(dataHandle, request, trx, false), ErrorResponseException);
  }

  void testConvertRequestError()
  {
    TestConfigInitializer cfg;
    XformClientShoppingXML xform("CXS_FORM", Global::config());

    DataHandle dataHandle;
    std::string request;

    CPPUNIT_ASSERT(xform.initialize(0, NULL));
  }

  // Check if validateTrx does not crash
  // when supplied a valid trx
  void testValidateTrx()
  {
    TestConfigInitializer cfg;
    XformClientShoppingXML xform("CXS_FORM", Global::config());
    CPPUNIT_ASSERT(xform.initialize(0, NULL));
    preparePricingTrx();

    _dataHandle->setCorpIdExists(true);
    xform.validateTrx(*_pricingTrx);
  }

  // Check the format of text in ErrorResponseException
  // if corporate id not found
  void testCorpIdNotFound()
  {
    using namespace std;
    TestConfigInitializer cfg;
    XformClientShoppingXML xform("CXS_FORM", Global::config());
    CPPUNIT_ASSERT(xform.initialize(0, NULL));
    preparePricingTrx();

    _dataHandle->setCorpIdExists(false);
    try { xform.validateTrx(*_pricingTrx); }
    catch (ErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(string("Error in Request: INVALID CORPORATE ID ") + _corporateId,
                           string(e.what()));
    }
  }

  void setUp()
  {
    _pricingTrx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _option = _memHandle.create<PricingOptions>();

    // register DataHAndle at DataHandleMockFactory
    _dataHandle = _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

private:
  class MyDataHandle : public DataHandleMock
  {

  public:
    MyDataHandle() { _corpIdExists = false; }
    bool corpIdExists(const std::string& corpId, const DateTime& tvlDate) { return _corpIdExists; }
    void setCorpIdExists(bool val) { _corpIdExists = val; }

  private:
    bool _corpIdExists;
  };

  // see ItinAnalyzerServiceTest for more examples
  PricingTrx* _pricingTrx;
  PricingRequest* _request;
  PricingOptions* _option;
  TestMemHandle _memHandle;
  MyDataHandle* _dataHandle;
  std::string _corporateId;

  void preparePricingTrx()
  {
    _pricingTrx->setOptions(_option);

    Agent* agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = agent;
    // see EligibilityTest.cpp
    _request->corporateID() = _corporateId; // TST11

    _pricingTrx->setRequest(_request);

    Billing* billing = _memHandle.create<Billing>();
    _pricingTrx->billing() = billing;

    // Build Pax
    PaxType* paxType = _memHandle.create<PaxType>();
    _pricingTrx->paxType().push_back(paxType);

    // add posPaxType. If not, validateTrx crashes
    _pricingTrx->posPaxType().push_back(PosPaxTypePerPax());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XformClientShoppingXMLTest);

} // tse
