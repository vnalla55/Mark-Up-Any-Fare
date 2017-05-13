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
#include "test/include/CppUnitHelperMacros.h"

#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/Request.h"
#include "Diagnostic/PositiveDiagnostic.h"
#include "Rules/PaymentRuleData.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/ServicesMock.h"

#include <memory>

namespace tax
{
class PositiveDiagnosticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PositiveDiagnosticTest);

  CPPUNIT_TEST(testDefaultPrinterEmptyItinsPayments);
  CPPUNIT_TEST(testDefaultPrinterOneItinsPayment);

  CPPUNIT_TEST(testGeoPropertiesPrinterEmptyItinsPayments);
  CPPUNIT_TEST(testGeoPropertiesPrinterOneItinsPayment);

  CPPUNIT_TEST(testDetailedCalculationPrinterEmptyItinsPayments);
  CPPUNIT_TEST(testDetailedCalculationPrinterOneItinsPayment);

  CPPUNIT_TEST(testPrintOcTaxLine);
  CPPUNIT_TEST(testPrintOcTaxLine_TaxInclInd);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _itinsPayments.reset(new ItinsPayments);

    _parameters.clear();

    _taxName.reset(new TaxName);
    _taxName->nation() = type::Nation("PL");
    _taxName->taxCode() = type::TaxCode("98");
    _taxName->taxType() = type::TaxType("076");
    _taxName->percentFlatTag() = type::PercentFlatTag::Flat;

    _seqNo.reset(new type::SeqNo);
    *_seqNo = 101;
    _tag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));

    _geoAAA.reset(new Geo);
    _geoAAA->loc().code() = "AAA";
    _geoAAA->id() = 0;
    _geoBBB.reset(new Geo);
    _geoBBB->loc().code() = "BBB";
    _geoBBB->id() = 1;
    _geoCCC.reset(new Geo);
    _geoCCC->loc().code() = "CCC";
    _geoCCC->id() = 2;
    _geoDDD.reset(new Geo);
    _geoDDD->loc().code() = "DDD";
    _geoDDD->id() = 3;
    _geoEEE.reset(new Geo);
    _geoEEE->loc().code() = "EEE";
    _geoEEE->id() = 4;

    _positiveDiagnostic.reset(
        new PositiveDiagnostic(*_itinsPayments, _parameters));
  }

  void tearDown() { _paymentDetailStorage.clear(); }

  PaymentDetail&
  addPaymentDetail(const type::ProcessingGroup processingGroup, ItinPayments& itinPayments)
  {
    type::CarrierCode marketingCarrier = "LH";
    type::CurrencyCode noCurrency (UninitializedCode);

    _paymentDetailStorage.push_back(new PaymentDetail(
        PaymentRuleData(
            *_seqNo, *_tag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        *_geoAAA,
        *_geoBBB,
        *_taxName,
        marketingCarrier));
    PaymentDetail& detail = _paymentDetailStorage.back();

    detail.setLoc3(&*_geoCCC);
    detail.setJourneyLoc1(&*_geoDDD);
    detail.setJourneyLoc2(&*_geoEEE);

    detail.getMutableTaxPointsProperties().resize(5);
    detail.getMutableTaxPointsProperties()[0].isFirst = true;
    detail.getMutableTaxPointsProperties()[4].isLast = true;

    itinPayments.payments(processingGroup).push_back(new Payment(*_taxName));
    itinPayments.payments(processingGroup).back().paymentDetail().push_back(&detail);

    detail.taxAmt() = 10;
    detail.taxCurrency() = "USD";
    detail.calcDetails().exchangeRate1 = doubleToAmount(1.234);
    detail.calcDetails().taxBeforeRounding = doubleToAmount(8.10373);
    detail.calcDetails().roundingDir = type::TaxRoundingDir::RoundUp;
    detail.calcDetails().roundingUnit = doubleToAmount(0.01);
    detail.taxEquivalentCurrency() = "EUR";
    detail.taxEquivalentAmount() = doubleToAmount(8.11);

    return detail;
  }

  OptionalService& addOC(PaymentDetail& paymentDetail, bool isTaxInclInd)
  {
    paymentDetail.optionalServiceItems().push_back(new OptionalService());
    paymentDetail.optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;
    paymentDetail.optionalServiceItems().back().subCode() = "1GG";
    paymentDetail.optionalServiceItems().back().taxAmount() = 10;
    paymentDetail.optionalServiceItems().back().setTaxEquivalentAmount(doubleToAmount(8.11));
    paymentDetail.optionalServiceItems().back().ownerCarrier() = "LO";
    paymentDetail.optionalServiceItems().back().serviceGroup() = "LM";
    paymentDetail.optionalServiceItems().back().serviceSubGroup() = "ML";
    paymentDetail.optionalServiceItems().back().setTaxPointBegin(*_geoAAA);
    paymentDetail.optionalServiceItems().back().setTaxPointEnd(*_geoBBB);
    paymentDetail.optionalServiceItems().back().taxInclInd() = isTaxInclInd;

    return paymentDetail.optionalServiceItems().back();
  }

  void addOCPaymentDetail(ItinPayments& itinPayments)
  {
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::OC, itinPayments);
    addOC(detail, false);
  }

  void addBaggagePaymentDetail(ItinPayments& itinPayments)
  {
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::Baggage, itinPayments);

    detail.optionalServiceItems().push_back(new OptionalService());
    detail.optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
    detail.optionalServiceItems().back().subCode() = "2BG";
    detail.optionalServiceItems().back().taxAmount() = 10;
    detail.optionalServiceItems().back().setTaxEquivalentAmount(doubleToAmount(8.11));
    detail.optionalServiceItems().back().ownerCarrier() = "AA";
    detail.optionalServiceItems().back().serviceGroup() = "BG";
    detail.optionalServiceItems().back().serviceSubGroup() = "BB";
    detail.optionalServiceItems().back().setTaxPointBegin(*_geoAAA);
    detail.optionalServiceItems().back().setTaxPointEnd(*_geoBBB);
  }

  void fillItinsPayments()
  {
    ItinPayments* itinPayments = new ItinPayments(0);
    itinPayments->validatingCarrier() = "LO";
    _itinsPayments->_itinPayments.push_back(itinPayments);
    addPaymentDetail(type::ProcessingGroup::Itinerary, *itinPayments);
    addOCPaymentDetail(*itinPayments);
    addBaggagePaymentDetail(*itinPayments);
  }

  void testDefaultPrinterEmptyItinsPayments()
  {
    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "------------------------ SEQS ANALYSIS ------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testDefaultPrinterOneItinsPayment()
  {
    fillItinsPayments();

    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "------------------------ SEQS ANALYSIS ------------------------",
                               "",
                               "------------------------- ITINERARY 1 -------------------------",
                               "            ----      TAXES ON ITIN 1 PSGR      ----           ",
                               "            ----     VALIDATING CARRIER: LO     ----           ",
                               "  NATION CODE TYPE TXPTAG P/F  TXAMT    TOTAL BOARD OFF  SEQNO",
                               "",
                               " 1  PL    98   076    S    F   10.00     8.11  AAA  BBB    101",
                               "            ----   TAXES ON OC - ITIN 1 PSGR    ----           ",
                               "            ----     VALIDATING CARRIER: LO     ----           ",
                               "  NATION CODE TYPE TXPTAG P/F  TXAMT    TOTAL BOARD OFF  SEQNO",
                               "",
                               " 2  PL    98   076    S    F   10.00     8.11  AAA  BBB    101",
                               "",
                               "            ---- TAXES ON BAGGAGE - ITIN 1 PSGR ----           ",
                               "            ----     VALIDATING CARRIER: LO     ----           ",
                               "  NATION CODE TYPE TXPTAG P/F  TXAMT    TOTAL BOARD OFF  SEQNO",
                               "",
                               " 3  PL    98   076    S    F   10.00     8.11  AAA  BBB    101",
                               "",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testGeoPropertiesPrinterEmptyItinsPayments()
  {
    _parameters.push_back(new Parameter());
    _parameters.back().name() = "GP";
    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "------------------------ GEO PROPERTIES -----------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testGeoPropertiesPrinterOneItinsPayment()
  {
    _parameters.push_back(new Parameter());
    _parameters.back().name() = "GP";

    fillItinsPayments();
    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "------------------------ GEO PROPERTIES -----------------------",
                               "",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "JRNY LOC1: DDD",
                               "JRNY LOC2: EEE",
                               "TXPT LOC1: AAA - STOPOVER   / NO FARE BREAK",
                               "  STOPOVER TAG: NONE",
                               "TXPT LOC2: BBB - CONNECTION / NO FARE BREAK",
                               "  STOPOVER TAG: NONE",
                               "TXPT LOC3: CCC - CONNECTION / NO FARE BREAK",
                               "RT OR OJ: 0",
                               "HIDDEN: N",
                               "",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "       ---- FOR OC --- 1GG LM ML LO (FLIGHTRELATED) ----       ",
                               "SERVICESUBTYPECODE: 1GG",
                               "SVCGROUP: LM",
                               "SVCSUBGROUP: ML",
                               "CARRIER: LO",
                               "OC BEGIN: AAA",
                               "OC END: BBB",
                               "",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "       ---- FOR OC --- 2BG BG BB AA (BAGGAGECHARGE) ----       ",
                               "SERVICESUBTYPECODE: 2BG",
                               "SVCGROUP: BG",
                               "SVCSUBGROUP: BB",
                               "CARRIER: AA",
                               "OC BEGIN: AAA",
                               "OC END: BBB",
                               "" };

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
    }
  }

  void testDetailedCalculationPrinterEmptyItinsPayments()
  {
    _parameters.push_back(new Parameter());
    _parameters.back().name() = "DC";
    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "-------------------- SEQS DETAILED ANALYSIS -------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testDetailedCalculationPrinterOneItinsPayment()
  {
    _parameters.push_back(new Parameter());
    _parameters.back().name() = "DC";

    fillItinsPayments();

    _positiveDiagnostic->createMessages(_messages);

    std::string expected[] = { "***************************************************************",
                               "            ----DIAGNOSTIC 832 - TAX APPLICATION----           ",
                               "***************************************************************",
                               "-------------------- SEQS DETAILED ANALYSIS -------------------",
                               "",
                               "------------------------- ITINERARY 1 -------------------------",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "FIXED TAX: 10 USD",
                               "TOTAL TAXABLE AMOUNT: 0",
                               "EXCHANGERATE: 1.234",
                               "UNROUNDED TAX: 8.10373",
                               "ROUNDING: UP TO 0.01",
                               "FINAL TAXAMOUNT: 8.11 EUR",
                               "",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "       ---- FOR OC --- 1GG LM ML LO (FLIGHTRELATED) ----       ",
                               "FIXED TAX: 10 USD",
                               "EXCHANGERATE: 1.234",
                               "UNROUNDED TAX: 8.10373",
                               "ROUNDING: UP TO 0.01",
                               "FINAL TAXAMOUNT: 8.11 EUR",
                               "",
                               "            ----  TAX PL 98 076 SEQ 101 - PSGR  ----           ",
                               "       ---- FOR OC --- 2BG BG BB AA (BAGGAGECHARGE) ----       ",
                               "FIXED TAX: 10 USD",
                               "EXCHANGERATE: 1.234",
                               "UNROUNDED TAX: 8.10373",
                               "ROUNDING: UP TO 0.01",
                               "FINAL TAXAMOUNT: 8.11 EUR",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + std::to_string(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testPrintOcTaxLine()
  {
    ItinPayments* itinPayments = new ItinPayments(0);
    _itinsPayments->_itinPayments.push_back(itinPayments);
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::OC, *itinPayments);
    OptionalService& oc = addOC(detail, false);
    _positiveDiagnostic->printTaxLine(1,
                                      *_taxName,
                                      detail,
                                      oc.getTaxPointBegin().locCode(),
                                      oc.getTaxPointEnd().locCode(),
                                      oc.getTaxEquivalentAmount(),
                                      ' ');

    std::string expectedResult = " 1  PL    98   076    S    F   10.00     8.11  AAA  BBB    101\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _positiveDiagnostic->_result.str());
  }

  void testPrintOcTaxLine_TaxInclInd()
  {
    ItinPayments* itinPayments = new ItinPayments(0);
    _itinsPayments->_itinPayments.push_back(itinPayments);
    PaymentDetail& detail = addPaymentDetail(type::ProcessingGroup::OC, *itinPayments);
    OptionalService& oc = addOC(detail, true);
    _positiveDiagnostic->printTaxLine(1,
                                      *_taxName,
                                      detail,
                                      oc.getTaxPointBegin().locCode(),
                                      oc.getTaxPointEnd().locCode(),
                                      oc.getTaxEquivalentAmount(),
                                      '*');

    std::string expectedResult = " 1  PL    98   076    S    F   10.00     8.11* AAA  BBB    101\n";
    CPPUNIT_ASSERT_EQUAL(expectedResult, _positiveDiagnostic->_result.str());
  }

private:
  std::unique_ptr<ItinsPayments> _itinsPayments;
  boost::ptr_vector<Parameter> _parameters;
  std::unique_ptr<TaxName> _taxName;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<type::TicketedPointTag> _tag;
  std::unique_ptr<Geo> _geoAAA;
  std::unique_ptr<Geo> _geoBBB;
  std::unique_ptr<Geo> _geoCCC;
  std::unique_ptr<Geo> _geoDDD;
  std::unique_ptr<Geo> _geoEEE;
  std::unique_ptr<PositiveDiagnostic> _positiveDiagnostic;
  boost::ptr_vector<Message> _messages;
  boost::ptr_vector<PaymentDetail> _paymentDetailStorage;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PositiveDiagnosticTest);

} // namespace tax
