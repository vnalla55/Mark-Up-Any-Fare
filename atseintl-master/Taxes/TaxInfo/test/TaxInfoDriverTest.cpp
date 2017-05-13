#include "Taxes/TaxInfo/TaxInfoDriver.h"
#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderPFC.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "Taxes/TaxInfo/TaxInfoBuilderMisc.h"
#include "Taxes/TaxInfo/test/TaxInfoDbMock.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Agent.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/PfcPFC.h"
#include "test/testdata/TestTaxCodeRegFactory.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const bool isHistorical() { return false; }
  const std::vector<PfcPFC*>& getAllPfcPFC()
  {
    std::vector<PfcPFC*>& ret = *_memHandle.create<std::vector<PfcPFC*> >();
    ret.push_back(_memHandle.create<PfcPFC>());
    ret.front()->effDate() = DateTime(2000, 1, 1);
    ret.front()->expireDate() = DateTime(2100, 1, 1);
    return ret;
  }
  const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode, const DateTime& date)
  {
    std::vector<TaxCodeReg*>& ret = *_memHandle.create<std::vector<TaxCodeReg*> >();
    if (taxCode == "ZP")
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml"));
    else if (taxCode == "US1")
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml"));
    else if (taxCode == "CA1")
      ret.push_back(
          TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_CA1.xml"));
    else
      return DataHandleMock::getTaxCode(taxCode, date);

    return ret;
  }
};
}

class TaxInfoTester
{
public:
  TaxInfoTester(LocCode agentLoc = "DFW", DateTime dt = DateTime::localTime())
  {
    _trx.taxInfoRequest() = &_taxInfoRequest;
    _trx.taxInfoRequest()->overrideDate() = dt;

    _trx.setRequest(&_request);

    _agent.agentCity() = agentLoc;
    _trx.getRequest()->ticketingAgent() = &_agent;
  }

  void addRequestItem(TaxCode taxCode, std::vector<LocCode> airports = std::vector<LocCode>())
  {
    TaxInfoItem item;

    item.taxCode() = taxCode;

    std::copy(airports.begin(), airports.end(), back_inserter(item.airports()));

    _trx.taxInfoRequest()->taxItems().push_back(item);
  }

  void execute()
  {
    TaxInfoDriver driver(&_trx);
    driver.buildTaxInfoResponse();
  }
  TaxTrx* trx() { return &_trx; }

  TaxTrx _trx;
  TaxInfoRequest _taxInfoRequest;
  TaxRequest _request;
  Agent _agent;
};

class TaxInfoDriverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxInfoDriverTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testBuildMisingTaxCodes);
  CPPUNIT_TEST(testBuildPfcForDFW);
  CPPUNIT_TEST(testBuildPfcForKRK);
  CPPUNIT_TEST(testBuildPfcForNYC);
  CPPUNIT_TEST(testBuildPfcForDFWJFK);
  CPPUNIT_TEST(testBuildPfcForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildPfcNoAirports);
  CPPUNIT_TEST(testBuildZpForDFW);
  CPPUNIT_TEST(testBuildZpForKRK);
  CPPUNIT_TEST(testBuildZpForNYC);
  CPPUNIT_TEST(testBuildZpForNYCJFK);
  CPPUNIT_TEST(testBuildZpForDFWJFK);
  CPPUNIT_TEST(testBuildZpForABIDFW);
  CPPUNIT_TEST(testBuildZpForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildZpNoAirports);
  CPPUNIT_TEST(testBuildZpPfc);
  CPPUNIT_TEST(testBuildMiscTransportUs1);
  CPPUNIT_TEST(testBuildMiscTransportCa1);
  CPPUNIT_TEST(testBuildMiscTransportUs1Ca1);
  CPPUNIT_TEST(testBuildZpPfcUs1Ca1);
  CPPUNIT_TEST(testTaxInfoFactoryPFC);
  CPPUNIT_TEST(testTaxInfoFactoryZP);
  CPPUNIT_TEST(testTaxInfoFactoryMiscTransport);
  CPPUNIT_TEST(testTaxInfoFactoryMiscTransportNoTaxCode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      TaxTrx trx;
      TaxInfoDriver driver(&trx);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testBuildMisingTaxCodes()
  {
    TaxInfoTester test;
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::ERROR>(
                       test.trx()->taxInfoResponseItems().front().taxAttrValues()) ==
                   TaxInfoBuilder::MISSING_TAX_CODE);
  }

  void testBuildPfcForDFW()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildPfcForKRK()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("KRK");

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildPfcForNYC()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("NYC");

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildPfcForDFWJFK()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");
    airports.push_back("JFK");

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildPfcForDFWKRKJFKNYC()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");
    airports.push_back("KRK");
    airports.push_back("JFK");
    airports.push_back("NYC");

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildPfcNoAirports()
  {
    TaxInfoTester test;

    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForDFW()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForKRK()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("KRK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForNYC()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("NYC");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForNYCJFK()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("NYC");
    airports.push_back("JFK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForDFWJFK()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");
    airports.push_back("JFK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForABIDFW()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("ABI");
    airports.push_back("DFW");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpForDFWKRKJFKNYC()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("ABI");
    airports.push_back("DFW");
    airports.push_back("JFK");
    airports.push_back("KRK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpNoAirports()
  {
    TaxInfoTester test;

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildZpPfc()
  {
    TaxInfoTester test;

    std::vector<LocCode> airports;
    airports.push_back("DFW");
    airports.push_back("JFK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 2);
  }

  void testBuildMiscTransportUs1()
  {
    TaxInfoTester test;

    test.addRequestItem("US1");
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildMiscTransportCa1()
  {
    TaxInfoTester test;

    test.addRequestItem("CA1");
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 1);
  }

  void testBuildMiscTransportUs1Ca1()
  {
    TaxInfoTester test;

    test.addRequestItem("US1");
    test.addRequestItem("CA1");
    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 2);
  }

  void testBuildZpPfcUs1Ca1()
  {
    TaxInfoTester test;

    test.addRequestItem("US1");
    test.addRequestItem("CA1");

    std::vector<LocCode> airports;
    airports.push_back("DFW");
    airports.push_back("JFK");

    test.addRequestItem(TaxInfoBuilderZP::TAX_CODE, airports);
    test.addRequestItem(TaxInfoBuilderPFC::TAX_CODE, airports);

    test.execute();

    CPPUNIT_ASSERT(test.trx()->taxInfoResponseItems().size() == 4);
  }

  void testTaxInfoFactoryPFC()
  {
    TaxTrx trx;

    TaxInfoBuilder* builder = TaxInfoBuilderFactory::getInstance(trx, TaxInfoBuilderPFC::TAX_CODE);

    CPPUNIT_ASSERT(dynamic_cast<TaxInfoBuilderPFC*>(builder));
  }

  void testTaxInfoFactoryZP()
  {
    TaxTrx trx;

    TaxInfoBuilder* builder = TaxInfoBuilderFactory::getInstance(trx, TaxInfoBuilderZP::TAX_CODE);

    CPPUNIT_ASSERT(dynamic_cast<TaxInfoBuilderZP*>(builder));
  }

  void testTaxInfoFactoryMiscTransport()
  {
    TaxTrx trx;

    TaxInfoBuilder* builder = TaxInfoBuilderFactory::getInstance(trx, "US1");

    CPPUNIT_ASSERT(dynamic_cast<TaxInfoBuilderMisc*>(builder));
  }

  void testTaxInfoFactoryMiscTransportNoTaxCode()
  {
    TaxTrx trx;

    TaxInfoBuilder* builder = TaxInfoBuilderFactory::getInstance(trx, "");

    CPPUNIT_ASSERT(dynamic_cast<TaxInfoBuilderMisc*>(builder));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxInfoDriverTest);
}
