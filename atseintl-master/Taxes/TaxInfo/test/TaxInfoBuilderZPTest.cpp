#include <string>
#include "Taxes/TaxInfo/test/TaxInfoDbMock.h"
#include "Taxes/TaxInfo/TaxInfoBuilder.h"
#include "Taxes/TaxInfo/TaxInfoBuilderZP.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxTrx.h"
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

class TaxInfoBuilderZPTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxInfoBuilderZPTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testBuildNoZp);
  CPPUNIT_TEST(testBuildZpForDFW);
  CPPUNIT_TEST(testBuildZpForKRK);
  CPPUNIT_TEST(testBuildZpForNYC);
  CPPUNIT_TEST(testBuildZpForNYCJFK);
  CPPUNIT_TEST(testBuildZpForDFWJFK);
  CPPUNIT_TEST(testBuildZpForABIDFW);
  CPPUNIT_TEST(testBuildZpForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildZpNoAirports);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxInfoBuilderZP builder; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testBuildNoZp()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().empty());
  }

  void testBuildZpForDFW()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string dfw = "DFW";
    item.airports().push_back(dfw);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().empty());
  }

  void testBuildZpForKRK()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string krk = "KRK";
    item.airports().push_back(krk);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().empty());
  }

  void testBuildZpForNYC()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string nyc = "NYC";
    item.airports().push_back(nyc);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().empty());
  }

  void testBuildZpForNYCJFK()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string nyc = "NYC";
    std::string jfk = "JFK";
    item.airports().push_back(nyc);
    item.airports().push_back(jfk);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 2);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::ZP::AIRPORT>(builder.response().aptAttrValues().front()) ==
                   nyc);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::ZP::ERROR>(builder.response().aptAttrValues().front()) ==
                   TaxInfoBuilder::INVALID_AIRPORT);
  }

  void testBuildZpForDFWJFK()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item1;
    TaxInfoItem item2;
    item1.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string dfw = "DFW";
    std::string jfk = "JFK";
    item1.airports().push_back(dfw);
    item1.airports().push_back(jfk);
    trx.taxInfoRequest()->taxItems().push_back(item1);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 2);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::ZP::AIRPORT>(builder.response().aptAttrValues().front()) ==
                   dfw);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::ZP::CURRENCY>(builder.response().aptAttrValues().front()) == "USD");
  }

  void testBuildZpForABIDFW()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item1;
    TaxInfoItem item2;
    item1.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string abi = "ABI";
    std::string dfw = "DFW";
    item1.airports().push_back(abi);
    item1.airports().push_back(dfw);
    trx.taxInfoRequest()->taxItems().push_back(item1);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 2);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::ZP::AIRPORT>(builder.response().aptAttrValues().front()) ==
                   abi);
  }

  void testBuildZpForDFWKRKJFKNYC()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item1;
    TaxInfoItem item2;
    item1.taxCode() = TaxInfoBuilderZP::TAX_CODE;

    std::string dfw = "DFW";
    std::string krk = "KRK";
    std::string jfk = "JFK";
    std::string nyc = "NYC";
    item1.airports().push_back(dfw);
    item1.airports().push_back(krk);
    item1.airports().push_back(jfk);
    item1.airports().push_back(nyc);
    trx.taxInfoRequest()->taxItems().push_back(item1);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 4);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().front().get<TaxInfoResponse::ZP::AIRPORT>() ==
                   dfw);
    /*
     CPPUNIT_ASSERT(builder.response().aptAttrValues().front().
                    get<TaxInfoResponse::ZP::AMOUNT>() == "0.00");

     CPPUNIT_ASSERT(builder.response().aptAttrValues().front().
                    get<TaxInfoResponse::ZP::CURRENCY>() == "USD");


     CPPUNIT_ASSERT(builder.response().aptAttrValues().front().
                    get<TaxInfoResponse::ZP::IS_DOMESTIC>() == "T");

     CPPUNIT_ASSERT(builder.response().aptAttrValues().front().
                    get<TaxInfoResponse::ZP::IS_RURAL>() == "F");



     CPPUNIT_ASSERT(builder.response().aptAttrValues()[1].
                    get<TaxInfoResponse::ZP::AIRPORT>() == krk);

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[1].
                    get<TaxInfoResponse::ZP::AMOUNT>() == "0.00");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[1].
                    get<TaxInfoResponse::ZP::CURRENCY>() == "USD");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[1].
                    get<TaxInfoResponse::ZP::IS_DOMESTIC>() == "F");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[1].
                    get<TaxInfoResponse::ZP::IS_RURAL>() == "F");


     CPPUNIT_ASSERT(builder.response().aptAttrValues()[2].
                    get<TaxInfoResponse::ZP::AIRPORT>() == jfk);

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[2].
                    get<TaxInfoResponse::ZP::AMOUNT>() == "0.00");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[2].
                    get<TaxInfoResponse::ZP::CURRENCY>() == "USD");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[2].
                    get<TaxInfoResponse::ZP::IS_DOMESTIC>() == "T");

     CPPUNIT_ASSERT(builder.response().aptAttrValues()[2].
                    get<TaxInfoResponse::ZP::IS_RURAL>() == "F"); */
  }

  void testBuildZpNoAirports()
  {
    TaxInfoBuilderZPMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderZP::TAX_CODE;
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::TAX::ERROR>(builder.response().taxAttrValues()) ==
                   TaxInfoBuilder::MISSING_AIRPORTS);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxInfoBuilderZPTest);
}
