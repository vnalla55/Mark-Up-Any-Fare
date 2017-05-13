#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "Taxes/TaxInfo/TaxInfoBuilderMisc.h"
#include "Taxes/TaxInfo/test/TaxInfoDbMock.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Agent.h"

#include <iostream>
#include <string>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  const bool isHistorical() { return false; }
};
}
class TaxInfoBuilderMiscTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxInfoBuilderMiscTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testBuildNoTaxCode);
  CPPUNIT_TEST(testBuildWrongTaxCode);
  CPPUNIT_TEST(testBuildUS1NationUS);
  CPPUNIT_TEST(testBuildUS1);
  CPPUNIT_TEST(testBuildCA1);
  CPPUNIT_TEST(testBuildCA1WrongAgentLoc);
  CPPUNIT_TEST(testBuildBF1PLNoPCC);
  CPPUNIT_TEST(testBuildBF1BFNoPCC);
  CPPUNIT_TEST(testBuildAENoPCC);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxInfoBuilderMisc builder; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testBuildNoTaxCode()
  {
    TaxInfoBuilderMisc builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::ERROR>(builder.response().taxAttrValues()) ==
                   TaxInfoBuilder::MISSING_TAX_CODE);
  }

  void testBuildWrongTaxCode()
  {
    TaxInfoBuilderMiscMock builder;
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    agent.agentCity() = "DFW";
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "AA1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::ERROR>(builder.response().taxAttrValues()) ==
                   TaxInfoBuilder::TAX_NOT_FOUND);
  }

  void testBuildUS1NationUS()
  {

    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "US1";

    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();
    trx.taxInfoRequest()->countryCodes().push_back("US");

    TaxInfoItem item;
    item.taxCode() = "US1";

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "US1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CODE>(builder.response().taxAttrValues()) == "US1");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::TYPE>(builder.response().taxAttrValues()) == "P");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::AMOUNT>(builder.response().taxAttrValues()) == "7.5");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CURRENCY>(builder.response().taxAttrValues()) ==
                   "USD");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::NATION>(builder.response().taxAttrValues()) == "US");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::DESCRIPTION>(builder.response().taxAttrValues()) ==
                   "US DOMESTIC TRANSPORTATION TAX");
  }

  void testBuildUS1()
  {
    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "US1";
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    agent.mainTvlAgencyPCC() = "01J7"; // DFW
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();
    // trx.taxInfoRequest()->countryCodes().push_back("US");

    TaxInfoItem item;
    item.taxCode() = "US1";

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "US1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CODE>(builder.response().taxAttrValues()) == "US1");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::TYPE>(builder.response().taxAttrValues()) == "P");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::AMOUNT>(builder.response().taxAttrValues()) == "7.5");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CURRENCY>(builder.response().taxAttrValues()) ==
                   "USD");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::NATION>(builder.response().taxAttrValues()) == "US");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::DESCRIPTION>(builder.response().taxAttrValues()) ==
                   "US DOMESTIC TRANSPORTATION TAX");
  }

  void testBuildCA1()
  {
    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "CA1";
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    agent.mainTvlAgencyPCC() = "03KA"; // YYC
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = "CA1";

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "CA1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CODE>(builder.response().taxAttrValues()) == "CA1");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::TYPE>(builder.response().taxAttrValues()) == "F");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::AMOUNT>(builder.response().taxAttrValues()) ==
                   "4.67");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CURRENCY>(builder.response().taxAttrValues()) ==
                   "CAD");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::NATION>(builder.response().taxAttrValues()) == "CA");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::DESCRIPTION>(builder.response().taxAttrValues()) ==
                   "CANADA AIR SECURITY CHARGE - SUBJECT TO GST");
  }

  void testBuildCA1WrongAgentLoc()
  {
    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "CA1";
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    agent.mainTvlAgencyPCC() = "HDQ";
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = "CA1";

    trx.taxInfoRequest()->taxItems().push_back(item);
    // trx.taxInfoRequest()->countryCodes().push_back("CA");

    builder.taxCode() = "CA1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CODE>(builder.response().taxAttrValues()) == "CA1");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::TYPE>(builder.response().taxAttrValues()) == "F");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::AMOUNT>(builder.response().taxAttrValues()) ==
                   "4.67");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CURRENCY>(builder.response().taxAttrValues()) ==
                   "CAD");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::NATION>(builder.response().taxAttrValues()) == "CA");
    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::DESCRIPTION>(builder.response().taxAttrValues()) ==
                   "CANADA AIR SECURITY CHARGE - SUBJECT TO GST");
  }

  void testBuildBF1PLNoPCC()
  {
    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "BF1";
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    trx.taxInfoRequest()->countryCodes().push_back("PL");

    TaxInfoItem item;
    item.taxCode() = "BF1";

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "BF1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::ERROR>(builder.response().taxAttrValues()) ==
                   TaxInfoBuilder::TAX_NOT_FOUND);
  }

  void testBuildBF1BFNoPCC()
  {
    TaxInfoBuilderMiscMock builder;
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    // agent.mainTvlAgencyPCC() = "03KA"; //YYC
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    trx.taxInfoRequest()->countryCodes().push_back("BF");

    TaxInfoItem item;
    item.taxCode() = "BF1";

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "BF1";

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::CODE>(builder.response().taxAttrValues()) == "BF1");
  }

  void testBuildAENoPCC()
  {
    TaxInfoBuilderMiscMock builder;
    builder.reqTaxCode() = "AE";
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    Agent agent;
    // agent.mainTvlAgencyPCC() = "03KA"; //YYC
    trx.getRequest()->ticketingAgent() = &agent;

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    trx.taxInfoRequest()->countryCodes().push_back("PL");
    trx.taxInfoRequest()->countryCodes().push_back("AE");

    TaxInfoItem item;
    item.taxCode() = "AE"; // Loc1 is zone

    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.taxCode() = "AE";

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().taxAttrValues().get<TaxInfoResponse::TAX::CODE>() == "AE");
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxInfoBuilderMiscTest);
}
