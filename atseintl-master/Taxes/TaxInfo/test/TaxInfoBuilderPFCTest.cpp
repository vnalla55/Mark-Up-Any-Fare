#include <string>
#include "Taxes/TaxInfo/test/TaxInfoDbMock.h"
#include "DataModel/TaxInfoRequest.h"
#include "DataModel/TaxTrx.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/PfcPFC.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<PfcPFC*>& getAllPfcPFC()
  {
    std::vector<PfcPFC*>& ret = *_memHandle.create<std::vector<PfcPFC*> >();
    ret.push_back(_memHandle.create<PfcPFC>());
    ret.front()->effDate() = DateTime(2000, 1, 1);
    ret.front()->expireDate() = DateTime(2100, 1, 1);
    return ret;
  }
};
}
class TaxInfoBuilderPFCTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxInfoBuilderPFCTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testBuildNoPfc);
  CPPUNIT_TEST(testBuildPfcForDFW);
  CPPUNIT_TEST(testBuildPfcForKRK);
  CPPUNIT_TEST(testBuildPfcForNYC);
  CPPUNIT_TEST(testBuildPfcForDFWJFK);
  CPPUNIT_TEST(testBuildPfcForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildPfcNoAirports);
  CPPUNIT_TEST(testToString);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxInfoBuilderPFC builder; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testBuildNoPfc()
  {
    TaxInfoBuilderPFC builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().empty());
  }

  void testBuildPfcForDFW()
  {

    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;

    TaxRequest taxReq;
    trx.setRequest(&taxReq);

    TaxInfoRequest taxInfoRequest;

    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderPFC::TAX_CODE;

    std::string dfw = "DFW";
    item.airports().push_back(dfw);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 1);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().front()) == dfw);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AMOUNT>(builder.response().aptAttrValues().front()) ==
                   "4.50");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::CURRENCY>(builder.response().aptAttrValues().front()) == "USD");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::EFF_DATE>(builder.response().aptAttrValues().front()) ==
        "2002-07-01");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::DISC_DATE>(builder.response().aptAttrValues().front()) ==
        "2017-03-01");
  }

  void testBuildPfcForKRK()
  {
    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderPFC::TAX_CODE;

    std::string krk = "KRK";
    item.airports().push_back(krk);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 1);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().front()) == krk);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AMOUNT>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::CURRENCY>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::EFF_DATE>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::DISC_DATE>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::ERROR>(builder.response().aptAttrValues().front()) ==
                   TaxInfoBuilderPFC::TAX_NOT_FOUND);
  }

  void testBuildPfcForNYC()
  {
    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderPFC::TAX_CODE;

    std::string nyc = "NYC";
    item.airports().push_back(nyc);
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 1);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().front()) == nyc);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AMOUNT>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::CURRENCY>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::EFF_DATE>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::DISC_DATE>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::ERROR>(builder.response().aptAttrValues().front()) ==
                   TaxInfoBuilderPFC::INVALID_AIRPORT);
  }

  void testBuildPfcForDFWJFK()
  {
    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;
    TaxRequest taxReq;
    trx.setRequest(&taxReq);

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item1;
    TaxInfoItem item2;
    item1.taxCode() = TaxInfoBuilderPFC::TAX_CODE;

    std::string dfw = "DFW";
    std::string jfk = "JFK";
    item1.airports().push_back(dfw);
    item1.airports().push_back(jfk);
    trx.taxInfoRequest()->taxItems().push_back(item1);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().aptAttrValues().size() == 2);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().front()) == dfw);

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::ERROR>(builder.response().aptAttrValues().front()).empty());

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().back()) ==
                   jfk);

    CPPUNIT_ASSERT(
        builder.response().aptAttrValues().back().get<TaxInfoResponse::PFC::ERROR>().empty());
  }

  void testBuildPfcForDFWKRKJFKNYC()
  {
    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;
    TaxRequest taxReq;
    trx.setRequest(&taxReq);

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item1;
    TaxInfoItem item2;
    item1.taxCode() = TaxInfoBuilderPFC::TAX_CODE;

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

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues().front()) == dfw);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AMOUNT>(builder.response().aptAttrValues().front()) ==
                   "4.50");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::CURRENCY>(builder.response().aptAttrValues().front()) == "USD");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::EFF_DATE>(builder.response().aptAttrValues().front()) ==
        "2002-07-01");

    CPPUNIT_ASSERT(
        std::get<TaxInfoResponse::PFC::DISC_DATE>(builder.response().aptAttrValues().front()) ==
        "2017-03-01");

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues()[1]) ==
                   krk);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::ERROR>(builder.response().aptAttrValues()[1]) ==
                   TaxInfoBuilderPFC::TAX_NOT_FOUND);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues()[2]) ==
                   jfk);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AMOUNT>(builder.response().aptAttrValues()[2]) ==
                   "3.50");

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::CURRENCY>(builder.response().aptAttrValues()[2]) ==
                   "USD");

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::EFF_DATE>(builder.response().aptAttrValues()[2]) ==
                   "2003-07-01");

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::DISC_DATE>(builder.response().aptAttrValues()[2]) ==
                   "2010-03-01");

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::AIRPORT>(builder.response().aptAttrValues()[3]) ==
                   nyc);

    CPPUNIT_ASSERT(std::get<TaxInfoResponse::PFC::ERROR>(builder.response().aptAttrValues()[3]) ==
                   TaxInfoBuilderPFC::INVALID_AIRPORT);
  }

  void testBuildPfcNoAirports()
  {
    TaxInfoBuilderPFCMock builder;
    TaxTrx trx;

    TaxInfoRequest taxInfoRequest;
    trx.taxInfoRequest() = &taxInfoRequest;
    trx.taxInfoRequest()->overrideDate() = DateTime::localTime();

    TaxInfoItem item;
    item.taxCode() = TaxInfoBuilderPFC::TAX_CODE;
    trx.taxInfoRequest()->taxItems().push_back(item);

    builder.build(trx);

    CPPUNIT_ASSERT(builder.response().taxAttrValues().get<TaxInfoResponse::TAX::ERROR>() ==
                   TaxInfoBuilder::MISSING_AIRPORTS);
  }

  void testToString()
  {
    TaxInfoBuilderPFC builder;

    DateTime dt = boost::posix_time::time_from_string("2009-01-20 23:59:59.000");

    CPPUNIT_ASSERT(builder.amtToString(4.5, "USD", dt) == "4.50");

    CPPUNIT_ASSERT(builder.dateToString(dt) == "2009-01-20");
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxInfoBuilderPFCTest);
}
