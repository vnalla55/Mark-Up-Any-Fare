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
#include "test/MakePaymentDetail.h"

#include "AtpcoTaxes/DataModel/Services/ReportingRecord.h"
#include "Rules/TaxCodeConversionApplicator.h"
#include "Rules/TaxCodeConversionRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"

#include <memory>

namespace tax
{
class TaxCodeConversionApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCodeConversionApplicatorTest);
  CPPUNIT_TEST(detailLevel);
  CPPUNIT_TEST(ancillaryLevel);
  CPPUNIT_TEST(taxCodeInDetailThatContainsOc);
  CPPUNIT_TEST_SUITE_END();

  static void addRec(std::vector<std::shared_ptr<ReportingRecord>>& records,
                     type::TaxCode code,
                     type::TaxType type,
                     std::string nation,
                     type::CarrierCode carrier,
                     type::Vendor vendor)
  {
    std::shared_ptr<ReportingRecord> rec(new ReportingRecord());
    rec->vendor = vendor;
    codeFromString(nation, rec->nation);
    rec->taxCarrier = carrier;
    rec->taxCode = code;
    rec->taxType = type;
    records.push_back(rec);
  }

public:
  void ancillaryLevel()
  {
    DefaultServices services;
    services.setFallbackService(new FallbackServiceServer());
    ReportingRecordServiceServer* recordsServer = new ReportingRecordServiceServer();
    services.setReportingRecordService(recordsServer);
    std::vector<std::shared_ptr<ReportingRecord>>& records = recordsServer->reportingRecords();
    addRec(records, "UK", "102", "UK", "BA", "SBR");
    addRec(records, "UK", "104", "UK", "BA", "SBR");

    TaxCodeConversionRule rule;
    TaxCodeConversionApplicator applicator(rule, services);

    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "104", "UK").withOptionalService();
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT(payDetail.optionalServiceItems().size());
      CPPUNIT_ASSERT_EQUAL(std::string("UKA"), payDetail.optionalServiceItems()[0].sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "102", "UK").withOptionalService();
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT(payDetail.optionalServiceItems().size());
      CPPUNIT_ASSERT_EQUAL(std::string("UKA"), payDetail.optionalServiceItems()[0].sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "101", "UK").withOptionalService();
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT(payDetail.optionalServiceItems().size());
      CPPUNIT_ASSERT_EQUAL(std::string("UKA"), payDetail.optionalServiceItems()[0].sabreTaxCode());
    }
  }

  void taxCodeInDetailThatContainsOc()
  {
    DefaultServices services;
    services.setFallbackService(new FallbackServiceServer());
    ReportingRecordServiceServer* recordsServer = new ReportingRecordServiceServer();
    services.setReportingRecordService(recordsServer);
    std::vector<std::shared_ptr<ReportingRecord>>& records = recordsServer->reportingRecords();
    addRec(records, "UK", "100", "UK", "BA", "SBR");
    addRec(records, "UK", "101", "UK", "BA", "SBR");

    TaxCodeConversionRule rule;
    TaxCodeConversionApplicator applicator(rule, services);

    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "101", "UK");
      AddOptionalService(payDetail).failedRule(rule);
      AddOptionalService(payDetail).failedRule(rule);

      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("UKL"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "101", "UK");
      AddOptionalService(payDetail).failedRule(rule);
      (AddOptionalService(payDetail));

      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("UKA"), payDetail.sabreTaxCode());
    }
  }

  void detailLevel()
  {
    DefaultServices services;
    services.setFallbackService(new FallbackServiceServer());
    ReportingRecordServiceServer* recordsServer = new ReportingRecordServiceServer();
    services.setReportingRecordService(recordsServer);

    TaxCodeConversionRule rule;
    TaxCodeConversionApplicator applicator(rule, services);

    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "002", "UK");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("UK2"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "001", "UK");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("UK"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("UK", "003", "UK");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("UK3"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("US", "002", "US");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("US2"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("US", "001", "US");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("US2"), payDetail.sabreTaxCode());
    }
    {
      PaymentDetail payDetail = MakePaymentDetail("US", "003", "US");
      CPPUNIT_ASSERT(applicator.apply(payDetail));
      CPPUNIT_ASSERT_EQUAL(std::string("US2"), payDetail.sabreTaxCode());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxCodeConversionApplicatorTest);

} // namespace tax

