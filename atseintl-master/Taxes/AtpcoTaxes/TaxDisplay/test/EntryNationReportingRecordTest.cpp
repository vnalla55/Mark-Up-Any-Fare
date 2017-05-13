// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/TaxName.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Services/ReportingRecord.h"
#include "Rules/TaxData.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/LocService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/EntryNationReportingRecord.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/LocServiceMock.h"
#include "test/ReportingRecordServiceMock.h"
#include "test/RulesRecordsServiceMock.h"
#include "test/TaxRoundingInfoServiceMock.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <iterator>
#include <string>

namespace tax
{
namespace display
{

class EntryNationReportingRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EntryNationReportingRecordTest);
  CPPUNIT_TEST(testHeaderNationCode);
  CPPUNIT_TEST(testHeaderNationName);
  CPPUNIT_TEST(testHeaderAirportCode);
  CPPUNIT_TEST(testFooter);
  CPPUNIT_TEST(testBodyTaxTable);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _services.setRulesRecordsService(&_rulesRecordService);
  }

  void tearDown()
  {
  }

  void testHeaderNationCode()
  {
    LocServiceMock *locService = new LocServiceMock();
    _services.setLocService(locService);

    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryNationReportingRecord entry(formatter, _services, request);
    entry._reportingRecords.insert(std::make_shared<ReportingRecord>());
    request.nationCode = "NZ";

    CPPUNIT_ASSERT(entry.buildHeader() == false);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), formatter.linesList().size());
    CPPUNIT_ASSERT(formatter.linesList().front().str() == "COUNTRY CODE NOT FOUND");
    formatter.linesList().clear();

    locService->setNationName("NEW ZEALAND");
    CPPUNIT_ASSERT(entry.buildHeader() == true);
    CPPUNIT_ASSERT(entry._nationName == "NEW ZEALAND");
  }

  void testHeaderNationName()
  {
    LocServiceMock *locService = new LocServiceMock();
    _services.setLocService(locService);

    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryNationReportingRecord entry(formatter, _services, request);
    entry._reportingRecords.insert(std::make_shared<ReportingRecord>());
    request.nationName = "NEW ZEALAND";

    CPPUNIT_ASSERT(entry.buildHeader() == false);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(formatter.linesList().front().str(),
                         std::string("COUNTRY NAME NOT FOUND"));
    formatter.linesList().clear();

    locService->setNation("NZ");
    CPPUNIT_ASSERT(entry.buildHeader() == true);
    CPPUNIT_ASSERT(entry._nationCode == "NZ");
  }

  void testHeaderAirportCode()
  {
    LocServiceMock *locService = new LocServiceMock();
    _services.setLocService(locService);

    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryNationReportingRecord entry(formatter, _services, request);
    entry._reportingRecords.insert(std::make_shared<ReportingRecord>());
    request.airportCode = "KBZ";

    CPPUNIT_ASSERT(entry.buildHeader() == false);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(formatter.linesList().front().str(),
                         std::string("AIRPORT/CITY CODE NOT FOUND"));
    formatter.linesList().clear();

    locService->setNation("NZ");
    locService->setNationName("NEW ZEALAND");
    CPPUNIT_ASSERT(entry.buildHeader() == true);
    CPPUNIT_ASSERT_EQUAL(entry._nationName,
                         std::string("NEW ZEALAND"));
    CPPUNIT_ASSERT(entry._nationCode == "NZ");
  }

  void testFooter()
  {
    TaxRoundingInfoServiceMock* roundingService = new TaxRoundingInfoServiceMock();
    roundingService->unit() = 1;
    _services.setTaxRoundingInfoService(roundingService);

    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryNationReportingRecord entry(formatter, _services, request);

    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("NO ROUNDING INFO"),
                         std::next(formatter.linesList().begin(), 2)->str());
    formatter.linesList().clear();

    roundingService->dir() = type::TaxRoundingDir::RoundUp;
    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY TAX ROUNDING - ROUND UP TO NEXT 1"),
                         std::next(formatter.linesList().begin(), 2)->str());
    formatter.linesList().clear();

    roundingService->dir() = type::TaxRoundingDir::RoundDown;
    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY TAX ROUNDING - ROUND DOWN TO NEXT 1"),
                         std::next(formatter.linesList().begin(), 2)->str());
    formatter.linesList().clear();

    roundingService->dir() = type::TaxRoundingDir::Nearest;
    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY TAX ROUNDING - ROUND TO NEAREST 1"),
                         std::next(formatter.linesList().begin(), 2)->str());
    formatter.linesList().clear();

    roundingService->dir() = type::TaxRoundingDir::NoRounding;
    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY TAX ROUNDING - NO ROUNDING APPLIES"),
                         std::next(formatter.linesList().begin(), 2)->str());
    formatter.linesList().clear();

    request.userType = TaxDisplayRequest::UserType::TN;
    entry.buildFooter();
    CPPUNIT_ASSERT_EQUAL(std::size_t(5), formatter.linesList().size());
    formatter.linesList().clear();
  }

  void testBodyTaxTable()
  {
    TaxDisplayRequest request;
    ResponseFormatter formatter;
    EntryNationReportingRecord entry(formatter, _services, request);
    entry._nationName = "TATOOINE";
    entry._nationCode = "TA";

    auto reportingRecord1 = std::make_shared<ReportingRecord>();
    reportingRecord1->taxCode = "T1";
    reportingRecord1->taxType = "001";
    reportingRecord1->taxCarrier = "HS";
    reportingRecord1->vendor = "ASDF";
    entry._reportingRecords.insert(reportingRecord1);

    auto reportingRecord2 = std::make_shared<ReportingRecord>();
    reportingRecord2->taxCode = "T2";
    reportingRecord2->taxType = "002";
    reportingRecord2->taxCarrier = "HS";
    reportingRecord2->vendor = "QWER";
    reportingRecord2->entries.push_back(new ReportingRecordEntry);
    reportingRecord2->entries.front().taxLabel = "CANTINA BAND TAX";
    entry._reportingRecords.insert(reportingRecord2);

    entry.bodyTaxTable();
    std::list<Line>::iterator begin = formatter.linesList().begin();
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), formatter.linesList().size());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY NAME - TATOOINE"),
                         std::next(begin, 0)->str());
    CPPUNIT_ASSERT_EQUAL(std::string("COUNTRY CODE - TA"),
                         std::next(begin, 1)->str());
    CPPUNIT_ASSERT(      std::next(begin, 2)->str().empty());
    CPPUNIT_ASSERT_EQUAL(std::string("TAX  TAX  TAX TAX"),
                         std::next(begin, 3)->str());
    CPPUNIT_ASSERT_EQUAL(std::string("CODE TYPE CXR NAME"),
                         std::next(begin, 4)->str());
    CPPUNIT_ASSERT(      std::next(begin, 5)->str().empty());
    CPPUNIT_ASSERT_EQUAL(std::string("1   T1   001  HS  "),
                         std::next(begin, 6)->str());
    CPPUNIT_ASSERT_EQUAL(std::string("2   T2   002  HS  CANTINA BAND TAX"),
                         std::next(begin, 7)->str());
  }

private:
  RulesRecordsServiceMock _rulesRecordService;
  DefaultServices _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EntryNationReportingRecordTest);
} /* namespace display */
} /* namespace tax */
