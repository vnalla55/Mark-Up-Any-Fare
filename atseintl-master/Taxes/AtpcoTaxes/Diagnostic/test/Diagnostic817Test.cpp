// test
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

#include "AtpcoTaxes/Common/TaxDetailsLevel.h"
#include "AtpcoTaxes/DomainDataObjects/GeoPath.h"
#include "AtpcoTaxes/DomainDataObjects/Itin.h"
#include "AtpcoTaxes/DomainDataObjects/Response.h"
#include "AtpcoTaxes/Factories/OutputConverter.h"
#include "AtpcoTaxes/ServiceInterfaces/Services.h"
#include "DataModel/RequestResponse/OutputResponse.h"
#include "Diagnostic/Diagnostic817.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/PaymentRuleData.h"
#include "ServiceInterfaces/Services.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/ServicesMock.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <memory>

namespace tax
{
class Diagnostic817Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diagnostic817Test);

  CPPUNIT_TEST(testEmptyResponse);
  CPPUNIT_TEST(testPrintDiagnostic817);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetailStorage.reserve(10);
    _itinsPayments.reset(new ItinsPayments);

    _request.reset(new Request);
    _request->diagnostic().parameters().clear();

    _taxName.reset(new TaxName);
    _taxName->nation() = type::Nation("PL");
    _taxName->taxCode() = type::TaxCode("98");
    _taxName->taxType() = type::TaxType("76_");

    _seqNo.reset(new type::SeqNo);
    *_seqNo = 101;
    _tag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));

    _geoAAA.reset(new Geo);
    _geoAAA->loc().code() = "AAA";
    _geoBBB.reset(new Geo);
    _geoBBB->loc().code() = "BBB";
    _geoCCC.reset(new Geo);
    _geoCCC->loc().code() = "CCC";
    _geoDDD.reset(new Geo);
    _geoDDD->loc().code() = "DDD";
    _geoEEE.reset(new Geo);
    _geoEEE->loc().code() = "EEE";

    _activationStatus.reset(new AtpcoTaxesActivationStatus);
    _activationStatus->setAllEnabled();

    _geoPath.reset(new GeoPath);
    _geoPath->geos().push_back(*_geoAAA);

    _diagnostic817.reset(new Diagnostic817(*_request, *_itinsPayments, *_activationStatus));
  }

  void tearDown()
  {
    _paymentDetailStorage.clear();
    _request->itins().clear();
  }

  PaymentDetail&
  addPaymentDetail(const type::ProcessingGroup& processingGroup, ItinPayments& itinPayments)
  {
    type::CarrierCode marketingCarrier = "LH";
    type::CurrencyCode noCurrency (UninitializedCode);
    itinPayments.payments(processingGroup).push_back(new Payment(*_taxName));
    itinPayments.requestedPassengerCode() = "ADT";

    _paymentDetailStorage.push_back(new PaymentDetail(
        PaymentRuleData(
            *_seqNo, *_tag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        *_geoAAA,
        *_geoBBB,
        *_taxName,
        marketingCarrier));
    PaymentDetail& detail = _paymentDetailStorage.back();

    itinPayments.payments(processingGroup).back().paymentDetail().push_back(&detail);

    detail.setLoc3(&*_geoCCC);
    detail.setJourneyLoc1(&*_geoDDD);
    detail.setJourneyLoc2(&*_geoEEE);

    return detail;
  }

  void addOCPaymentDetail(ItinPayments& itinPayments)
  {
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::OC, itinPayments);

    detail.optionalServiceItems().push_back(new OptionalService());
    detail.optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;
    detail.optionalServiceItems().back().subCode() = "1GG";
    detail.optionalServiceItems().back().amount() = 20;
    detail.optionalServiceItems().back().taxAmount() = 10;
    detail.optionalServiceItems().back().ownerCarrier() = "LO";
    detail.optionalServiceItems().back().serviceGroup() = "LM";
    detail.optionalServiceItems().back().serviceSubGroup() = "ML";
    detail.optionalServiceItems().back().setTaxPointBegin(*_geoAAA);
    detail.optionalServiceItems().back().setTaxPointEnd(*_geoBBB);
  }

  void addBaggagePaymentDetail(ItinPayments& itinPayments)
  {
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::Baggage, itinPayments);

    detail.optionalServiceItems().push_back(new OptionalService());
    detail.optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
    detail.optionalServiceItems().back().subCode() = "2BG";
    detail.optionalServiceItems().back().amount() = 30;
    detail.optionalServiceItems().back().taxAmount() = 15;
    detail.optionalServiceItems().back().ownerCarrier() = "AA";
    detail.optionalServiceItems().back().serviceGroup() = "BG";
    detail.optionalServiceItems().back().serviceSubGroup() = "BB";
    detail.optionalServiceItems().back().setTaxPointBegin(*_geoAAA);
    detail.optionalServiceItems().back().setTaxPointEnd(*_geoBBB);
  }

  void fillItinsPayments()
  {
    _request->allItins().push_back(Itin());
    _request->itins().push_back(&_request->allItins().back());
    _request->allItins().back().geoPath() = _geoPath.get();
    _request->allItins().back().yqYrPathRefId() = 10;

    ItinPayments* itinPayments = new ItinPayments(_request->itins().size() - 1);
    itinPayments->validatingCarrier() = "LO";
    _itinsPayments->_itinPayments.push_back(itinPayments);

    addPaymentDetail(type::ProcessingGroup::Itinerary, *itinPayments);
    addOCPaymentDetail(*itinPayments);
    addBaggagePaymentDetail(*itinPayments);
  }

  void testEmptyResponse()
  {
    _diagnostic817->createMessages(_messages);

    std::string expected[] = { "---------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void testPrintDiagnostic817()
  {
    fillItinsPayments();
    _diagnostic817->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    CPPUNIT_ASSERT(std::string::npos != messages.find("ATPCO TAX OUT VECTOR"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("PSGR ADT"));
    CPPUNIT_ASSERT(std::string::npos != messages.find("VALIDATING CARRIER: LO"));

    CPPUNIT_ASSERT(
        std::string::npos !=
        messages.find("  CODE TYP  TXAMT     TXTTL  TXFARE BOARD OFF CARRIER SEQNO SPN"));

    CPPUNIT_ASSERT(
        std::string::npos !=
        messages.find(" 1     F     0.00      0.00    0.00  AAA  AAA    LH      101  0"));
  }

private:
  std::unique_ptr<Request> _request;
  std::unique_ptr<ItinsPayments> _itinsPayments;
  std::unique_ptr<TaxName> _taxName;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<type::TicketedPointTag> _tag;
  std::unique_ptr<Geo> _geoAAA;
  std::unique_ptr<Geo> _geoBBB;
  std::unique_ptr<Geo> _geoCCC;
  std::unique_ptr<Geo> _geoDDD;
  std::unique_ptr<Geo> _geoEEE;
  std::unique_ptr<Diagnostic817> _diagnostic817;
  boost::ptr_vector<Message> _messages;
  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<PaymentDetail> _paymentDetailStorage;
  std::unique_ptr<AtpcoTaxesActivationStatus> _activationStatus;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diagnostic817Test);

} // namespace tax
//*/
