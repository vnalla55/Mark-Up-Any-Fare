// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Diagnostic/NegativeDiagnostic.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include "DomainDataObjects/ItinsPayments.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/PaymentRuleData.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/ServicesMock.h"

#include <gtest/gtest.h>

#include <memory>

namespace tax
{

namespace
{
  const type::CurrencyCode noCurrency (UninitializedCode);
}

class NegativeDiagnosticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegativeDiagnosticTest);

  CPPUNIT_TEST(testEmptyItinsPayments);

  CPPUNIT_TEST(testPrintPaymentDetailsFiltered_print);
  CPPUNIT_TEST(geoDetails);
  CPPUNIT_TEST(testPrintPaymentDetailsFiltered_notPrint);
  CPPUNIT_TEST(testPrintPaymentDetailsFiltered_optionalServices);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _parameters.clear();

    _seqNo.reset(new type::SeqNo);
    *_seqNo = 101;

    _taxName.reset(new TaxName);
    _taxName->nation() = type::Nation("PL");
    _taxName->taxCode() = type::TaxCode("TC");
    _taxName->taxType() = type::TaxType("0TT");

    _begin.reset(new Geo);
    _begin->id() = 0;
    _begin->loc().code() = "KRK";
    _end.reset(new Geo);
    _end->id() = 1;
    _end->loc().code() = "DFW";

    _failedRule.reset(new ExemptTagRule());
    _itinsPayments.reset(new ItinsPayments);

    _servicesMock.reset(new ServicesMock());
    _servicesMock->setFallbackService(new FallbackServiceServer());

    _negativeDiagnostic.reset(new NegativeDiagnostic(*_itinsPayments, _parameters, *_servicesMock.get()));
  }

  void tearDown() {}

  void fillItinsPayments()
  {
    _itinsPayments->_itinPayments.push_back(new ItinPayments(0));
    _itinsPayments->_itinPayments.back().payments(type::ProcessingGroup::Itinerary).push_back(
        new Payment(*_taxName));
    type::CarrierCode marketingCarrier = "LH";
    _paymentDetail = std::make_shared<PaymentDetail>(
        PaymentRuleData(*_seqNo,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        TaxableUnitTagSet::none(),
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        *_begin,
        *_end,
        *_taxName,
        marketingCarrier);
    PaymentDetail& theDetail = *_paymentDetail;
    theDetail.getMutableItineraryDetail().setFailedRule(_failedRule.get());
    theDetail.getMutableTaxPointsProperties().resize(2);
    theDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    theDetail.getMutableTaxPointsProperties()[0].isExtendedStopover.setTag(
        type::ConnectionsTag::TurnaroundPointForConnection);
    theDetail.getMutableTaxPointsProperties()[0].isExtendedStopover.setTag(
        type::ConnectionsTag::DomesticToInternational);
    theDetail.getMutableTaxPointsProperties()[0].isExtendedStopover.setTag(
        type::ConnectionsTag::InternationalToDomestic);
    _itinsPayments->_itinPayments.back()
        .payments(type::ProcessingGroup::Itinerary)
        .back()
        .paymentDetail()
        .push_back(_paymentDetail.get());
  }

  void fillOptionalServices()
  {
    _itinsPayments->_itinPayments.back().payments(type::ProcessingGroup::OC).push_back(
        new Payment(*_taxName));
    type::CarrierCode marketingCarrier = "LH";
    _ocDetail = std::make_shared<PaymentDetail>(
        PaymentRuleData(*_seqNo,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        TaxableUnitTagSet::none(),
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        *_begin,
        *_end,
        *_taxName,
        marketingCarrier);
    ;

    PaymentDetail& paymentDetail = *_ocDetail;
    // Not failed - should not print
    paymentDetail.optionalServiceItems().push_back(new OptionalService());
    paymentDetail.optionalServiceItems().back().setTaxPointBegin(*_begin);
    paymentDetail.optionalServiceItems().back().setTaxPointEnd(*_end);
    // Failed and matches filter - should print
    paymentDetail.optionalServiceItems().push_back(new OptionalService());
    paymentDetail.optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;
    paymentDetail.optionalServiceItems().back().amount() = 200;
    paymentDetail.optionalServiceItems().back().subCode() = "1GG";
    paymentDetail.optionalServiceItems().back().serviceGroup() = "ML";
    paymentDetail.optionalServiceItems().back().serviceSubGroup() = "LM";
    paymentDetail.optionalServiceItems().back().ownerCarrier() = "AA";
    paymentDetail.optionalServiceItems().back().setFailedRule(_failedRule.get());
    paymentDetail.optionalServiceItems().back().setTaxPointBegin(*_begin);
    paymentDetail.optionalServiceItems().back().setTaxPointEnd(*_end);
    // Failed and doesn't match filter - shouldn't print
    paymentDetail.optionalServiceItems().push_back(new OptionalService());
    paymentDetail.optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
    paymentDetail.optionalServiceItems().back().amount() = 100;
    paymentDetail.optionalServiceItems().back().subCode() = "1GG";
    paymentDetail.optionalServiceItems().back().serviceGroup() = "BG";
    paymentDetail.optionalServiceItems().back().serviceSubGroup() = "BB";
    paymentDetail.optionalServiceItems().back().ownerCarrier() = "XX";
    paymentDetail.optionalServiceItems().back().setFailedRule(_failedRule.get());
    paymentDetail.optionalServiceItems().back().setTaxPointBegin(*_begin);
    paymentDetail.optionalServiceItems().back().setTaxPointEnd(*_end);
    _itinsPayments->_itinPayments.back()
        .payments(type::ProcessingGroup::OC)
        .back()
        .paymentDetail()
        .push_back(_ocDetail.get());
  }

  void testEmptyItinsPayments()
  {
    _negativeDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----   DIAGNOSTIC 833 - FAILED TAX  ----           ",
                               "***************************************************************",
                               "----------------------- FAILED SEQUENCES ----------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void testPrintPaymentDetailsFiltered_print()
  {
    fillItinsPayments();
    _negativeDiagnostic->_filter.nation = "PL";
    _negativeDiagnostic->_filter.taxCode = "TC";

    _negativeDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----   DIAGNOSTIC 833 - FAILED TAX  ----           ",
                               "***************************************************************",
                               "----------------------- FAILED SEQUENCES ----------------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "NATION: PL   TAXCODE: TC   TAXTYPE: 0TT   SEQNO: 101",
                               "TXPTAG: S",
                               "BOARD: KRK                      OFF: DFW",
                               "BOARD INDEX: 0 - STOP/NFB       OFF INDEX: 1 - CONN/NFB",
                               "",
                               "FAILED ON: THIS SEQUENCE IS EXEMPTED",
                               "***************************************************************",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());
    for (size_t i = 0; i < _messages.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void geoDetails()
  {
    fillItinsPayments();
    _negativeDiagnostic->_filter.nation = "PL";
    _negativeDiagnostic->_filter.taxCode = "TC";
    _negativeDiagnostic->_filter.showGP = true;

    _negativeDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----   DIAGNOSTIC 833 - FAILED TAX  ----           ",
                               "***************************************************************",
                               "----------------------- FAILED SEQUENCES ----------------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "NATION: PL   TAXCODE: TC   TAXTYPE: 0TT   SEQNO: 101",
                               "TXPTAG: S",
                               "BOARD: KRK                      OFF: DFW",
                               "BOARD INDEX: 0 - STOP/NFB       OFF INDEX: 1 - CONN/NFB",
                               "  STOPOVER TAG: NONE              STOPOVER TAG: NONE",
                               "  CONN TAGS: A H I",
                               "",
                               "FAILED ON: THIS SEQUENCE IS EXEMPTED",
                               "***************************************************************",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());
    for (size_t i = 0; i < _messages.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void testPrintPaymentDetailsFiltered_notPrint()
  {
    fillItinsPayments();
    _negativeDiagnostic->_filter.nation = "PL";
    _negativeDiagnostic->_filter.taxCode = "TX";

    _negativeDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----   DIAGNOSTIC 833 - FAILED TAX  ----           ",
                               "***************************************************************",
                               "----------------------- FAILED SEQUENCES ----------------------",
                               "            ----            ITIN: 1             ----           ",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());
    for (size_t i = 0; i < _messages.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void testPrintPaymentDetailsFiltered_optionalServices()
  {
    fillItinsPayments();
    fillOptionalServices();
    _negativeDiagnostic->_filter.subCode = "1GG";
    _negativeDiagnostic->_filter.type = type::OptionalServiceTag::FlightRelated;

    _negativeDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----   DIAGNOSTIC 833 - FAILED TAX  ----           ",
                               "***************************************************************",
                               "----------------------- FAILED SEQUENCES ----------------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "NATION: PL   TAXCODE: TC   TAXTYPE: 0TT   SEQNO: 101",
                               "TXPTAG: S",
                               "BOARD: KRK                      OFF: DFW",
                               "BOARD INDEX: 0 - STOP/NFB       OFF INDEX: 1 - CONN/NFB",
                               "",
                               "FAILED ON: THIS SEQUENCE IS EXEMPTED",
                               "***************************************************************",
                               "NATION: PL   TAXCODE: TC   TAXTYPE: 0TT   SEQNO: 101",
                               "OCTYPE: F (FLIGHTRELATED)   SERVICESUBTYPECODE: 1GG",
                               "SVCGROUP: ML   SVCSUBGROUP: LM   CARRIER: AA",
                               "TXPTAG: S",
                               "BOARD: KRK     OFF: DFW",
                               "BOARD INDEX: 0\tOFF INDEX: 1",
                               "",
                               "FAILED ON: THIS SEQUENCE IS EXEMPTED",
                               "***************************************************************",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());
    for (size_t i = 0; i < _messages.size(); ++i)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

private:
  std::unique_ptr<ServicesMock> _servicesMock;
  std::unique_ptr<ItinsPayments> _itinsPayments;
  std::shared_ptr<PaymentDetail> _paymentDetail;
  std::shared_ptr<PaymentDetail> _ocDetail;
  std::unique_ptr<BusinessRule> _failedRule;
  std::unique_ptr<TaxName> _taxName;
  std::unique_ptr<Geo> _begin;
  std::unique_ptr<Geo> _end;
  std::unique_ptr<type::SeqNo> _seqNo;
  boost::ptr_vector<Parameter> _parameters;
  std::unique_ptr<NegativeDiagnostic> _negativeDiagnostic;
  boost::ptr_vector<Message> _messages;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NegativeDiagnosticTest);

} // namespace tax
