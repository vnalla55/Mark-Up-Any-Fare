// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Services/ReportingRecord.h"
#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/EntryTaxRulesRecord.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/RulesRecordsServiceMock.h"
#include "test/ReportingRecordServiceMock.h"

#include <list>

namespace tax
{
namespace display
{

class EntryTaxRulesRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EntryTaxRulesRecordTest);
  CPPUNIT_TEST(testGetTaxTableData_taxCode);
  CPPUNIT_TEST(testGetTaxTableData_taxCodeTaxType);
  CPPUNIT_TEST(testGetTaxTableData_taxCodeTaxTypeCarrierCode);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void setData()
  {
    _request.requestDate = type::Timestamp(type::Date(2016, 8, 4));
    ReportingRecordService::SharedSingleValue reportingRecord1 = std::make_shared<ReportingRecord>();
    ReportingRecordService::SharedSingleValue reportingRecord2 = std::make_shared<ReportingRecord>();
    ReportingRecordService::SharedSingleValue reportingRecord3 = std::make_shared<ReportingRecord>();
    reportingRecord1->taxCode = "CC"; reportingRecord1->taxType = "100";
    reportingRecord1->entries.push_back(new ReportingRecordEntry{});
    reportingRecord2->taxCode = "CC"; reportingRecord2->taxType = "003";
    reportingRecord2->entries.push_back(new ReportingRecordEntry{"TAX LABEL"});
    reportingRecord3->taxCode = "TT"; reportingRecord3->taxType = "003";
    reportingRecord3->entries.push_back(new ReportingRecordEntry{});
    ReportingRecordServiceMock* reportingRecordService = new ReportingRecordServiceMock();
    reportingRecordService->records()->push_back(reportingRecord1);
    reportingRecordService->records()->push_back(reportingRecord2);
    reportingRecordService->records()->push_back(reportingRecord3);
    reportingRecordService->records()->push_back(std::make_shared<ReportingRecord>());

    std::shared_ptr<RulesRecord> rulesRecord1 = std::make_shared<RulesRecord>();
    std::shared_ptr<RulesRecord> rulesRecord2 = std::make_shared<RulesRecord>();
    std::shared_ptr<RulesRecord> rulesRecord3 = std::make_shared<RulesRecord>();
    std::shared_ptr<RulesRecord> rulesRecord4 = std::make_shared<RulesRecord>();
    std::shared_ptr<RulesRecord> rulesRecord5 = std::make_shared<RulesRecord>();
    rulesRecord1->taxName.taxCode() = "CC";
    rulesRecord1->taxName.taxType() = "001";
    rulesRecord1->taxName.nation() = "LI";
    rulesRecord1->vendor = "ATP";
    rulesRecord1->vendor = "ATP";
    rulesRecord1->effDate = type::Date(2016, 8, 3);
    rulesRecord1->discDate = type::Date(2016, 8, 5);
    rulesRecord1->expiredDate = type::Timestamp(type::Date(2016, 8, 4));
    rulesRecord2->taxName.taxCode() = "TT";
    rulesRecord2->taxName.taxType() = "002";
    rulesRecord2->taxName.nation() = "LI";
    rulesRecord2->vendor = "ATP";
    rulesRecord2->effDate = type::Date(2016, 8, 3);
    rulesRecord2->discDate = type::Date(2016, 8, 5);
    rulesRecord2->expiredDate = type::Timestamp(type::Date(2016, 8, 4));
    rulesRecord3->taxName.taxCode() = "CC";
    rulesRecord3->taxName.taxType() = "003";
    rulesRecord3->taxName.nation() = "MC";
    rulesRecord3->taxName.taxCarrier() = "QQ";
    rulesRecord3->vendor = "SABR";
    rulesRecord3->effDate = type::Date(2016, 8, 3);
    rulesRecord3->discDate = type::Date(2016, 8, 5);
    rulesRecord3->expiredDate = type::Timestamp(type::Date(2016, 8, 4));
    rulesRecord4->taxName.taxCode() = "CC";
    rulesRecord4->taxName.taxType() = "003";
    rulesRecord4->taxName.nation() = "MC";
    rulesRecord4->taxName.taxCarrier() = "WW";
    rulesRecord4->vendor = "ATP";
    rulesRecord4->effDate = type::Date(2016, 8, 3);
    rulesRecord4->discDate = type::Date(2016, 8, 5);
    rulesRecord4->expiredDate = type::Timestamp(type::Date(2016, 8, 4));
    rulesRecord5->taxName.taxCode() = "TT";
    rulesRecord5->taxName.taxType() = "100";
    rulesRecord5->taxName.nation() = "LI";
    rulesRecord5->vendor = "ATP";
    rulesRecord5->effDate = type::Date(2016, 8, 3);
    rulesRecord5->discDate = type::Date(2016, 8, 5);
    rulesRecord5->expiredDate = type::Timestamp(type::Date(2016, 8, 4));
    RulesRecordsServiceMock* rulesRecordService = new RulesRecordsServiceMock();
    rulesRecordService->addByCodeRecord(rulesRecord1);
    rulesRecordService->addByCodeRecord(rulesRecord2);
    rulesRecordService->addByCodeRecord(rulesRecord3);
    rulesRecordService->addByCodeRecord(rulesRecord4);
    rulesRecordService->addByCodeRecord(rulesRecord5);

    _services.setRulesRecordsService(rulesRecordService);
    _services.setReportingRecordService(reportingRecordService);
  }

  void testGetTaxTableData_taxCode()
  {
    _request.taxCode = "CC";
    setData();

    CPPUNIT_ASSERT_EQUAL(true, _entry.getTaxTableData());
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), _entry._taxTableData.size());
    CPPUNIT_ASSERT_EQUAL(false, _entry._hasSingleNationRecords);

    const ViewX1TaxTable::DataRow& dataRow1 = *_entry._taxTableData.begin();
    const TaxName& taxName1 = std::get<const TaxName&>(dataRow1);
    const type::TaxLabel* taxLabel1 = std::get<const type::TaxLabel*>(dataRow1);
    const type::Vendor& vendor1 = std::get<const type::Vendor&>(dataRow1);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName1.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("001"), taxName1.taxType().asString());
    CPPUNIT_ASSERT(taxLabel1 == nullptr);
    CPPUNIT_ASSERT_EQUAL(std::string("ATP"), vendor1.asString());

    const ViewX1TaxTable::DataRow& dataRow2 = *std::next(_entry._taxTableData.begin(), 1);
    const TaxName& taxName2 = std::get<const TaxName&>(dataRow2);
    const type::Vendor& vendor2 = std::get<const type::Vendor&>(dataRow2);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName2.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("003"), taxName2.taxType().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("ATP"), vendor2.asString());

    const ViewX1TaxTable::DataRow& dataRow3 = *std::next(_entry._taxTableData.begin(), 2);
    const TaxName& taxName3 = std::get<const TaxName&>(dataRow3);
    const type::TaxLabel* taxLabel3 = std::get<const type::TaxLabel*>(dataRow3);
    const type::Vendor& vendor3 = std::get<const type::Vendor&>(dataRow3);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName3.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("003"), taxName3.taxType().asString());
    CPPUNIT_ASSERT_EQUAL(type::TaxLabel("TAX LABEL"), *taxLabel3);
    CPPUNIT_ASSERT_EQUAL(std::string("SABR"), vendor3.asString());
  }

  void testGetTaxTableData_taxCodeTaxType()
  {
    _request.taxCode = "CC";
    _request.taxType = "003";
    setData();

    CPPUNIT_ASSERT_EQUAL(true, _entry.getTaxTableData());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), _entry._taxTableData.size());
    CPPUNIT_ASSERT_EQUAL(true, _entry._hasSingleNationRecords);

    const ViewX1TaxTable::DataRow& dataRow1 = *_entry._taxTableData.begin();
    const TaxName& taxName1 = std::get<const TaxName&>(dataRow1);
    const type::Vendor& vendor1 = std::get<const type::Vendor&>(dataRow1);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName1.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("003"), taxName1.taxType().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("WW"), taxName1.taxCarrier().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("ATP"), vendor1.asString());

    const ViewX1TaxTable::DataRow& dataRow2 = *std::next(_entry._taxTableData.begin(), 1);
    const TaxName& taxName2 = std::get<const TaxName&>(dataRow2);
    const type::Vendor& vendor2 = std::get<const type::Vendor&>(dataRow2);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName2.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("003"), taxName2.taxType().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("QQ"), taxName2.taxCarrier().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("SABR"), vendor2.asString());
  }

  void testGetTaxTableData_taxCodeTaxTypeCarrierCode()
  {
    _request.taxCode = "CC";
    _request.taxType = "003";
    _request.carrierCode1 = "WW";
    setData();

    CPPUNIT_ASSERT_EQUAL(true, _entry.getTaxTableData());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), _entry._taxTableData.size());
    CPPUNIT_ASSERT_EQUAL(true, _entry._hasSingleNationRecords);

    const ViewX1TaxTable::DataRow& dataRow1 = *_entry._taxTableData.begin();
    const TaxName& taxName1 = std::get<const TaxName&>(dataRow1);
    const type::Vendor& vendor1 = std::get<const type::Vendor&>(dataRow1);
    CPPUNIT_ASSERT_EQUAL(std::string("CC"), taxName1.taxCode().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("003"), taxName1.taxType().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("WW"), taxName1.taxCarrier().asString());
    CPPUNIT_ASSERT_EQUAL(std::string("ATP"), vendor1.asString());
  }

private:
  ResponseFormatter _formatter;
  DefaultServices _services;
  TaxDisplayRequest _request;
  EntryTaxRulesRecord _entry{_formatter, _services, _request};
};

CPPUNIT_TEST_SUITE_REGISTRATION(EntryTaxRulesRecordTest);
} /* namespace display */
} /* namespace tax */
