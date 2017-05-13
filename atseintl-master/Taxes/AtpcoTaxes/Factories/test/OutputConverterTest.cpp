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

#include "AtpcoTaxes/Common/TaxDetailsLevel.h"
#include "AtpcoTaxes/DataModel/RequestResponse/BCHOutputResponse.h"
#include "AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "AtpcoTaxes/DomainDataObjects/Response.h"
#include "AtpcoTaxes/Factories/OutputConverter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/PaymentDetailMock.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"

#include <memory>

namespace tax
{

class OutputConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OutputConverterTest);
  CPPUNIT_TEST(ocGeneration);
  CPPUNIT_TEST(disablingDetails);
  CPPUNIT_TEST(testSetPaymentCurrency);
  CPPUNIT_TEST(testChangeFee);
  CPPUNIT_TEST(testExchangeDetails);
  CPPUNIT_TEST(testBCHResponse);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { buildResponseWithAllDetails(); }

  void tearDown()
  {
    _taxNames.clear();
    _response.reset();
    _paymentDetails.clear();
  }

  void ocGeneration()
  {
    withOptServices(2);
    TaxDetailsLevel level;
    OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);

    const boost::optional<OutputItins>& itins = output.itins();
    CPPUNIT_ASSERT(itins);
    const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails = itins->taxDetailsSeq();
    CPPUNIT_ASSERT_EQUAL(size_t(2), taxDetails.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), *(taxDetails[0]->optionalServiceId()));
    CPPUNIT_ASSERT_EQUAL(size_t(1), *(taxDetails[1]->optionalServiceId()));
  }

  void disablingDetails()
  {
    PaymentDetailMock* paymentDetailMock = addPaymentDetail(type::ProcessingGroup::Itinerary);
    paymentDetailMock->setTaxableUnit(type::TaxableUnit::Itinerary);
    paymentDetailMock->setTotalFareAmount(10);

    withOptServices(1);
    TaxDetailsLevel level;
    OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);

    const boost::optional<OutputItins>& itins = output.itins();
    CPPUNIT_ASSERT(itins);
    const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails = itins->taxDetailsSeq();
    CPPUNIT_ASSERT_EQUAL(size_t(2), taxDetails.size());
    const std::shared_ptr<OutputTaxDetails>& taxDetail = taxDetails[0];
    CPPUNIT_ASSERT(taxDetail);

    CPPUNIT_ASSERT(!taxDetail->calcDetails());
    CPPUNIT_ASSERT(!taxDetail->geoDetails());
    CPPUNIT_ASSERT(!taxDetail->optionalServiceDetails());
    CPPUNIT_ASSERT(!taxDetail->taxOnFaresDetails);
    CPPUNIT_ASSERT(!taxDetail->taxOnTaxDetails);
    CPPUNIT_ASSERT(!taxDetail->taxOnYqYrDetails);

    const std::shared_ptr<OutputTaxDetails>& optDetail = taxDetails[1];
    CPPUNIT_ASSERT(optDetail);
    CPPUNIT_ASSERT(!optDetail->calcDetails());
    CPPUNIT_ASSERT(!optDetail->geoDetails());
    CPPUNIT_ASSERT(!optDetail->optionalServiceDetails());
    CPPUNIT_ASSERT(!optDetail->taxOnFaresDetails);
    CPPUNIT_ASSERT(!optDetail->taxOnTaxDetails);
    CPPUNIT_ASSERT(!optDetail->taxOnYqYrDetails);

    level = TaxDetailsLevel::all();
    {
      OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);

      const boost::optional<OutputItins>& itins = output.itins();
      CPPUNIT_ASSERT(itins);
      const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails = itins->taxDetailsSeq();
      CPPUNIT_ASSERT_EQUAL(size_t(2), taxDetails.size());
      const std::shared_ptr<OutputTaxDetails>& taxDetail = taxDetails[0];
      CPPUNIT_ASSERT(taxDetail);

      CPPUNIT_ASSERT(taxDetail->calcDetails());
      CPPUNIT_ASSERT(taxDetail->geoDetails());
      CPPUNIT_ASSERT(!taxDetail->optionalServiceDetails());
      CPPUNIT_ASSERT(taxDetail->taxOnFaresDetails);
      CPPUNIT_ASSERT(taxDetail->taxOnTaxDetails);
      CPPUNIT_ASSERT(taxDetail->taxOnYqYrDetails);

      const std::shared_ptr<OutputTaxDetails>& optDetail = taxDetails[1];
      CPPUNIT_ASSERT(optDetail);
      CPPUNIT_ASSERT(!optDetail->calcDetails());
      CPPUNIT_ASSERT(optDetail->geoDetails());
      CPPUNIT_ASSERT(optDetail->optionalServiceDetails());
      CPPUNIT_ASSERT(!optDetail->taxOnFaresDetails);
      CPPUNIT_ASSERT(!optDetail->taxOnTaxDetails);
      CPPUNIT_ASSERT(!optDetail->taxOnYqYrDetails);
    }

    level.geo = false;
    level.taxOnTax = false;
    {
      OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);

      const boost::optional<OutputItins>& itins = output.itins();
      CPPUNIT_ASSERT(itins);
      const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails = itins->taxDetailsSeq();
      CPPUNIT_ASSERT_EQUAL(size_t(2), taxDetails.size());
      const std::shared_ptr<OutputTaxDetails>& taxDetail = taxDetails[0];
      CPPUNIT_ASSERT(taxDetail);

      CPPUNIT_ASSERT(taxDetail->calcDetails());
      CPPUNIT_ASSERT(!taxDetail->geoDetails());
      CPPUNIT_ASSERT(!taxDetail->optionalServiceDetails());
      CPPUNIT_ASSERT(taxDetail->taxOnFaresDetails);
      CPPUNIT_ASSERT(!taxDetail->taxOnTaxDetails);
      CPPUNIT_ASSERT(taxDetail->taxOnYqYrDetails);

      const std::shared_ptr<OutputTaxDetails>& optDetail = taxDetails[1];
      CPPUNIT_ASSERT(optDetail);
      CPPUNIT_ASSERT(!optDetail->calcDetails());
      CPPUNIT_ASSERT(!optDetail->geoDetails());
      CPPUNIT_ASSERT(optDetail->optionalServiceDetails());
      CPPUNIT_ASSERT(!optDetail->taxOnFaresDetails);
      CPPUNIT_ASSERT(!optDetail->taxOnTaxDetails);
      CPPUNIT_ASSERT(!optDetail->taxOnYqYrDetails);
    }
  }

  void testSetPaymentCurrency()
  {
    type::CurrencyCode currency("CR1");
    type::CurDecimals currencyDecimals(3);

    _response->_itinsPayments->paymentCurrency = currency;
    _response->_itinsPayments->paymentCurrencyNoDec = currencyDecimals;

    TaxDetailsLevel level;
    OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);
    const boost::optional<OutputItins>& itins = output.itins();

    CPPUNIT_ASSERT_EQUAL(currency, itins->paymentCur());
    CPPUNIT_ASSERT_EQUAL(currencyDecimals, itins->paymentCurNoDec());
  }

  void testChangeFee()
  {
    type::MoneyAmount tax = 34;

    PaymentDetailMock* changeFeeDetail = addPaymentDetail(type::ProcessingGroup::ChangeFee);
    changeFeeDetail->setTotalFareAmount(10);
    changeFeeDetail->taxOnChangeFeeAmount() = tax;
    changeFeeDetail->setTaxableUnit(type::TaxableUnit::ChangeFee);

    TaxDetailsLevel level;
    OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);
    const boost::optional<OutputItins>& itins = output.itins();

    const OutputTaxGroup* outputTaxGroup = findGroup(*itins, "X");
    CPPUNIT_ASSERT(outputTaxGroup != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), outputTaxGroup->taxSeq().size());
    CPPUNIT_ASSERT_EQUAL(tax, *(outputTaxGroup->taxSeq().back().totalAmt()));
  }

  void testExchangeDetails()
  {
    PaymentDetailMock* exchangeDetail = addPaymentDetail(type::ProcessingGroup::Itinerary);

    exchangeDetail->exchangeDetails().isMixedTax = true;
    exchangeDetail->exchangeDetails().isPartialTax = false;
    exchangeDetail->exchangeDetails().minTaxAmount = type::MoneyAmount(11);
    exchangeDetail->exchangeDetails().minMaxTaxCurrency = type::CurrencyCode("ABS");
    exchangeDetail->exchangeDetails().minMaxTaxCurrencyDecimals = 3;

    TaxDetailsLevel level;
    OutputResponse output = OutputConverter::convertToTaxRs(*_response, level);
    const boost::optional<OutputItins>& itins = output.itins();
    CPPUNIT_ASSERT(itins);

    const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails = itins->taxDetailsSeq();
    CPPUNIT_ASSERT_EQUAL(size_t(1), taxDetails.size());

    CPPUNIT_ASSERT(taxDetails.back()->exchangeDetails()->isMixedTax);
    CPPUNIT_ASSERT(!taxDetails.back()->exchangeDetails()->isPartialTax);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(11), *taxDetails.back()->exchangeDetails()->minTaxAmount);
    CPPUNIT_ASSERT_EQUAL(type::CurrencyCode("ABS"), *taxDetails.back()->exchangeDetails()->minMaxTaxCurrency);
    CPPUNIT_ASSERT_EQUAL(3u, *taxDetails.back()->exchangeDetails()->minMaxTaxCurrencyDecimals);
  }

  void testBCHResponse()
  {
    PaymentDetailMock* paymentDetailMock = addPaymentDetail(type::ProcessingGroup::Itinerary,
                                                            "AA",
                                                            "001",
                                                            type::PercentFlatTag::Flat,
                                                            type::MoneyAmount(9999, 100));
    paymentDetailMock->setTaxableUnit(type::TaxableUnit::Itinerary);
    paymentDetailMock->setTotalFareAmount(10);

    BCHOutputResponse output = OutputConverter::convertToBCHTaxRs(*_response);

    CPPUNIT_ASSERT(output.constTaxDetails().size() == 1);
    const auto& taxDetail = output.constTaxDetails()[0];
    CPPUNIT_ASSERT_EQUAL(size_t(0), taxDetail.getId());
    CPPUNIT_ASSERT_EQUAL(type::SabreTaxCode("AA"), taxDetail.getSabreCode());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(9999, 100), taxDetail.getPaymentAmt());
    CPPUNIT_ASSERT_EQUAL(type::TaxLabel(""), taxDetail.getName());

    CPPUNIT_ASSERT(output.constItins().size() == 1);
    const auto& itin = output.constItins()[0];
    CPPUNIT_ASSERT_EQUAL(size_t(1), itin.getId());
    CPPUNIT_ASSERT_EQUAL(type::PassengerCode("ADT"), itin.constPaxDetail().getPtc());
    CPPUNIT_ASSERT_EQUAL(size_t(1), itin.constPaxDetail().getPtcNumber());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(9999, 100), itin.constPaxDetail().getTotalAmount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), itin.constPaxDetail().constTaxIds().size());
    CPPUNIT_ASSERT(itin.constPaxDetail().constTaxIds().find(0) !=
                   itin.constPaxDetail().constTaxIds().end());
  }

private:
  const OutputTaxGroup* findGroup(const OutputItins& itins, const std::string& taxGroupName)
  {
    for (const std::shared_ptr<OutputTaxGroup>& taxGroup : itins.taxGroupSeq())
    {
      if (taxGroup->type() == taxGroupName)
      {
        return taxGroup.get();
      }
    }

    return 0;
  }

  PaymentDetailMock* addPaymentDetail(type::ProcessingGroup processingGroup)
  {
    ItinPayments& payments = _response->_itinsPayments->_itinPayments.back();

    _taxNames.push_back(new TaxName);
    payments.payments(processingGroup).push_back(new Payment(_taxNames.back()));
    Payment& payment = payments.payments(processingGroup).back();

    _paymentDetails.push_back(new PaymentDetailMock());
    PaymentDetailMock* newPaymentDetail = &_paymentDetails.back();
    payment.paymentDetail().push_back(newPaymentDetail);

    return newPaymentDetail;
  }

  PaymentDetailMock* addPaymentDetail(type::ProcessingGroup processingGroup,
                                      type::TaxCode taxCode,
                                      type::TaxType taxType,
                                      type::PercentFlatTag percentFlatTag,
                                      type::MoneyAmount totalityAmt)
  {
    ItinPayments& payments = _response->_itinsPayments->_itinPayments.back();

    _taxNames.push_back(new TaxName);
    _taxNames.back().taxCode() = taxCode;
    _taxNames.back().taxType() = taxType;
    _taxNames.back().percentFlatTag() = percentFlatTag;
    payments.payments(processingGroup).push_back(new Payment(_taxNames.back()));
    Payment& payment = payments.payments(processingGroup).back();
    payment.totalityAmt() = totalityAmt;

    _paymentDetails.push_back(new PaymentDetailMock());
    PaymentDetailMock* newPaymentDetail = &_paymentDetails.back();
    payment.paymentDetail().push_back(newPaymentDetail);

    newPaymentDetail->taxEquivalentAmount() = totalityAmt;

    return newPaymentDetail;
  }

  void buildResponseWithAllDetails()
  {
    _response.reset(new Response);
    _response->_itinsPayments.emplace(ItinsPayments());
    _response->_itinsPayments->_itinPayments.push_back(new ItinPayments(1));
    _response->_itinsPayments->_itinPayments.back().requestedPassengerCode() = "ADT";
  }

  void withOptServices(int n)
  {
    for (int i = 0; i < n; ++i)
    {
      PaymentDetailMock* ocPaymentDetail = addPaymentDetail(type::ProcessingGroup::OC);
      ocPaymentDetail->optionalServiceItems().push_back(new OptionalService);
      ocPaymentDetail->optionalServiceItems().back().setTaxPointBegin(
          ocPaymentDetail->getTaxPointBegin());
      ocPaymentDetail->optionalServiceItems().back().setTaxPointEnd(
          ocPaymentDetail->getTaxPointEnd());
      ocPaymentDetail->optionalServiceItems().back().index() = i;

      ocPaymentDetail->setTaxableUnit(type::TaxableUnit::OCFlightRelated);
    }
  }

  std::unique_ptr<Response> _response;
  boost::ptr_vector<TaxName> _taxNames;

  // managed by _response->_itinsPayments->_itinPayments.back().payments().back().paymentDetail()
  boost::ptr_vector<PaymentDetailMock> _paymentDetails;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OutputConverterTest);

} // namespace tax
