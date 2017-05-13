// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "test/include/CppUnitHelperMacros.h"
#include "AtpcoTaxes/DataModel/Services/ReportingRecord.h"
#include "AtpcoTaxes/Factories/MakeSabreCode.h"
#include "AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"

namespace tax
{

class MakeSabreCodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MakeSabreCodeTest);
  CPPUNIT_TEST(itinCode);
  CPPUNIT_TEST(serviceCode);
  CPPUNIT_TEST_SUITE_END();

  void addRec(std::vector<std::shared_ptr<ReportingRecord>>& records,
              type::TaxCode code,
              type::TaxType type,
              type::Nation nation,
              type::CarrierCode carrier,
              type::Vendor vendor)
  {
    ReportingRecord rec;
    rec.vendor = vendor;
    rec.nation = nation;
    rec.taxCarrier = carrier;
    rec.taxCode = code;
    rec.taxType = type;
    records.push_back(std::shared_ptr<ReportingRecord>(new ReportingRecord(rec)));
  }

public:
  void itinCode()
  {
    const type::PercentFlatTag percent(type::PercentFlatTag::Percent);
    const type::PercentFlatTag flat(type::PercentFlatTag::Flat);

    CPPUNIT_ASSERT_EQUAL(std::string("UK" ), makeItinSabreCode("UK", "001", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UK2"), makeItinSabreCode("UK", "002", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UK3"), makeItinSabreCode("UK", "003", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UK4"), makeItinSabreCode("UK", "004", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UK5"), makeItinSabreCode("UK", "005", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UK6"), makeItinSabreCode("UK", "006", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UK7"), makeItinSabreCode("UK", "007", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UK8"), makeItinSabreCode("UK", "008", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UK9"), makeItinSabreCode("UK", "009", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKA"), makeItinSabreCode("UK", "010", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKB"), makeItinSabreCode("UK", "011", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKC"), makeItinSabreCode("UK", "012", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKD"), makeItinSabreCode("UK", "013", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKE"), makeItinSabreCode("UK", "014", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKF"), makeItinSabreCode("UK", "015", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKG"), makeItinSabreCode("UK", "016", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKH"), makeItinSabreCode("UK", "017", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKI"), makeItinSabreCode("UK", "018", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKJ"), makeItinSabreCode("UK", "019", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKK"), makeItinSabreCode("UK", "020", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "021", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "022", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "023", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "098", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "099", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "100", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "000", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "01A", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "0AA", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "AAA", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "A01", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "0ZA", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "0ZA", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "AAZ", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "Z01", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("UKL"), makeItinSabreCode("UK", "01G", percent));

    // special case for US
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "001", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "002", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "003", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "004", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "005", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "006", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "007", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "008", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "009", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "010", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "011", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "012", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "013", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "014", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "015", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "016", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "017", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "018", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "019", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "020", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "021", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "022", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "023", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "098", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "099", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "100", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "000", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "01A", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "0AA", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US1"), makeItinSabreCode("US", "AAA", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "A01", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "0ZA", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "0ZA", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "AAZ", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "Z01", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("US2"), makeItinSabreCode("US", "01G", flat));

    CPPUNIT_ASSERT_EQUAL(std::string("PL" ), makeItinSabreCode("PL", "001", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("PL2"), makeItinSabreCode("PL", "002", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("PLK"), makeItinSabreCode("PL", "020", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("PLL"), makeItinSabreCode("PL", "021", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("PLL"), makeItinSabreCode("PL", "022", percent));
    CPPUNIT_ASSERT_EQUAL(std::string("PLL"), makeItinSabreCode("PL", "000", flat));
    CPPUNIT_ASSERT_EQUAL(std::string("PLL"), makeItinSabreCode("PL", "00Z", flat));
  }

  void serviceCode()
  {
    DefaultServices services;
    services.setFallbackService(new FallbackServiceServer());
    ReportingRecordServiceServer* recordsServer = new ReportingRecordServiceServer();
    services.setReportingRecordService(recordsServer);
    std::vector<std::shared_ptr<ReportingRecord>>& records = recordsServer->reportingRecords();
    addRec(records, "US", "001", "US", "AA", "SBR");
    addRec(records, "PL", "AAA", "PL", "LO", "SBR");
    addRec(records, "UK", "109", "UK", "BA", "ATP");
    addRec(records, "DE", "103", "DE", "LH", "ATP");
    addRec(records, "CH", "100", "CH", "CH", "ATP");

    CPPUNIT_ASSERT_EQUAL(std::string("USA"), makeServiceSabreCode(type::TaxCode("US")));
    CPPUNIT_ASSERT_EQUAL(std::string("PLA"), makeServiceSabreCode(type::TaxCode("PL")));
    CPPUNIT_ASSERT_EQUAL(std::string("UKA"), makeServiceSabreCode(type::TaxCode("UK")));
    CPPUNIT_ASSERT_EQUAL(std::string("DEA"), makeServiceSabreCode(type::TaxCode("DE")));
    CPPUNIT_ASSERT_EQUAL(std::string("CHA"), makeServiceSabreCode(type::TaxCode("CH")));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MakeSabreCodeTest);

} // namespace tax

