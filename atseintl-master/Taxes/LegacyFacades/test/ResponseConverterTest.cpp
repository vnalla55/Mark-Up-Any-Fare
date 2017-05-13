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

#include <boost/optional.hpp>

#include "AtpcoTaxes/DataModel/RequestResponse/IoTaxName.h"
#include "AtpcoTaxes/DataModel/RequestResponse/OutputTaxDetails.h"
#include "Common/Config/ConfigMan.h"
#include "Common/CustomerActivationUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/DiskCache.h"
#include "DomainDataObjects/Response.h"
#include "Taxes/Common/AtpcoTypes.h"
#include "Taxes/LegacyFacades/ResponseConverter2.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/AtpcoTaxes/Common/TaxDetailsLevel.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "Taxes/AtpcoTaxes/Factories/OutputConverter.h"
#include "Taxes/AtpcoTaxes/Rules/PaymentRuleData.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <memory>

namespace tax
{

namespace
{
  const type::CurrencyCode noCurrency(UninitializedCode);

  struct ReturnNothing
  {
    template <typename T>
    std::vector<std::string> operator()(T&&) const { return {}; }
  };
}

class ResponseConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ResponseConverterTest);

  CPPUNIT_TEST(testEmptyTaxVector);
  CPPUNIT_TEST(testOneTax);
  CPPUNIT_SKIP_TEST(testOneTaxChangeFee); // failed on jenkins build, check ongoing
  CPPUNIT_TEST(testTwoTaxes);

  CPPUNIT_TEST(testBuildOCFeesTaxItem);
  CPPUNIT_TEST(testBuildTaxItem_exchangeDetails);

  CPPUNIT_TEST_SUITE_END();

  tse::PricingTrx* _trx;
  std::unique_ptr<tse::PricingRequest> _request;
  Response* _atpcoResponse;
  ItinsPayments* _itinsPayments;
  ItinPayments* _itinPayments;
  tse::FarePath* _farePath;
  tse::Itin* _itin;
  tax::TaxRequestBuilder::OptionalServicesRefs* _optionalServicesMapping;
  tse::TestMemHandle _memHandle;
  tse::ActivationResult* _acResult;

  // no longer provided by ResponseConverter, but still testing the sequence of two conversions
  void
  convertResponseToTaxResponse(tse::PricingTrx& trx,
                               Response& atpcoResponse,
                               tse::TaxResponse& atseV2Response,
                               const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
                               const tax::ItinFarePathKey& mapping)
  {
    const OutputResponse& outputResponse =
        OutputConverter::convertToTaxRs(atpcoResponse, TaxDetailsLevel::all());
    if (outputResponse.itins())
      tax::ResponseConverter(trx, trx.atpcoTaxesActivationStatus()).convertToV2Response(*outputResponse.itins(),
                                             mapping, &atseV2Response, ReturnNothing{});
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _farePath = new tse::FarePath();
    _itin = new tse::Itin();
    _farePath->itin() = _itin;
    _itin->travelSeg().push_back(new tse::AirSeg());
    _itin->travelSeg().push_back(new tse::AirSeg());
    _itin->travelSeg()[0]->legId() = 0;
    _itin->travelSeg()[1]->legId() = 1;
    _trx = new tse::PricingTrx();
    _request.reset(new tse::PricingRequest());
    _request->ticketingDT() = tse::DateTime(2014, 12, 8);
    _acResult = new tse::ActivationResult();
    _acResult->isActivationFlag() = true;
    _acResult->finalActvDate() = tse::DateTime(2014, 10, 8); // activation date before ticketing date
    _trx->setRequest(_request.get());
    _trx->projCACMapData().insert(std::make_pair("TOE", _acResult));
    _atpcoResponse = new Response();
    _itinsPayments = new ItinsPayments();
    _itinPayments = new ItinPayments(0);
    _itinsPayments->_itinPayments.push_back(_itinPayments);
    _optionalServicesMapping = new tax::TaxRequestBuilder::OptionalServicesRefs();

    TestConfigInitializer::setValue("ATPCO_TAXES_ONCHANGEFEE_ENABLED", "Y", "TAX_SVC");
  }

  void tearDown()
  {
    delete _itinsPayments;
    delete _atpcoResponse;
    _request.reset();
    delete _trx;
    delete _farePath;
    delete _itin;
    delete _optionalServicesMapping;
    delete _acResult;
    _memHandle.clear();
  }

  void testEmptyTaxVector()
  {
    tse::TaxResponse atseV2Response;
    tse::Itin tseItin;
    tax::type::CarrierCode cc{"CC"};
    const tax::ItinFarePathKey key = std::make_tuple(&tseItin, atseV2Response.farePath(), cc, 0);

    convertResponseToTaxResponse(
        *_trx, *_atpcoResponse, atseV2Response, *_optionalServicesMapping, key);
    CPPUNIT_ASSERT(atseV2Response.taxItemVector().empty());
  }

  void testOneTax()
  {
    TaxName taxName;
    taxName.nation() = "US";
    taxName.taxCode() = "ZP";
    taxName.taxType() = "001";
    taxName.percentFlatTag() = type::PercentFlatTag::Flat;
    taxName.taxPointTag() = type::TaxPointTag::Departure;

    Geo geo1;
    geo1.loc().code() = "DFW";
    geo1.id() = 0;
    Geo geo2;
    geo2.loc().code() = "NYC";
    geo2.id() = 1;
    type::CarrierCode marketingCarrier = "LH";

    TaxableUnitTagSet taxableUnitTagSet(TaxableUnitTagSet::none());
    taxableUnitTagSet.setTag(type::TaxableUnit::Itinerary);

    boost::ptr_vector<Payment>& payments =
        _itinPayments->payments(type::ProcessingGroup::Itinerary);
    payments.push_back(new Payment(taxName));

    PaymentDetail detail(
        PaymentRuleData(100,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        taxableUnitTagSet,
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        geo1,
        geo2,
        taxName,
        marketingCarrier);
    PaymentDetail* pd = &detail;

    payments[0].paymentDetail().push_back(pd);
    pd->setTotalFareAmount(660);
    pd->taxAmt() = 6;
    pd->taxEquivalentAmount() = 7;
    pd->taxCurrency() = "CR1";
    pd->sabreTaxCode() = "ZP";
    _itinsPayments->paymentCurrency = "CR3";
    _atpcoResponse->_itinsPayments.emplace(std::move(*_itinsPayments));

    tse::TaxResponse atseV2Response;
    atseV2Response.farePath() = _farePath;

    tse::Itin tseItin;
    tax::type::CarrierCode cc{"CC"};
    const tax::ItinFarePathKey key = std::make_tuple(&tseItin, atseV2Response.farePath(), cc, 0);

    convertResponseToTaxResponse(
        *_trx, *_atpcoResponse, atseV2Response, *_optionalServicesMapping, key);
    CPPUNIT_ASSERT(!atseV2Response.taxItemVector().empty());

    const tse::TaxItem* taxItem = atseV2Response.taxItemVector()[0];
    CPPUNIT_ASSERT_EQUAL(tse::NationCode("US"), taxItem->nation());
    CPPUNIT_ASSERT_EQUAL(tse::TaxCode("ZP"), taxItem->taxCode());
    CPPUNIT_ASSERT_EQUAL('F', taxItem->taxType());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(7), taxItem->taxAmount());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(6), taxItem->taxAmt());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR3"), taxItem->paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR1"), taxItem->taxCur());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(660), taxItem->taxableFare());
    CPPUNIT_ASSERT_EQUAL(100, taxItem->seqNo());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem->travelSegStartIndex());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem->travelSegEndIndex());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("DFW"), taxItem->taxLocalBoard());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("NYC"), taxItem->taxLocalOff());
  }

  void testOneTaxChangeFee()
  {
    // tse::TestConfigInitializer::setValue("ATPCO_TAXES_PHASE2_ENABLED", "Y", "TAX_SVC");

    TaxName taxName;
    taxName.nation() = "US";
    taxName.taxCode() = "ZP";
    taxName.taxType() = "001";
    taxName.percentFlatTag() = type::PercentFlatTag::Flat;
    taxName.taxPointTag() = type::TaxPointTag::Departure;

    Geo geo1;
    geo1.loc().code() = "DFW";
    geo1.id() = 0;
    Geo geo2;
    geo2.loc().code() = "NYC";
    geo2.id() = 1;
    type::CarrierCode marketingCarrier = "LH";

    TaxableUnitTagSet taxableUnitTagSet(TaxableUnitTagSet::none());
    taxableUnitTagSet.setTag(type::TaxableUnit::ChangeFee);

    boost::ptr_vector<Payment>& payments =
        _itinPayments->payments(type::ProcessingGroup::ChangeFee);
    payments.push_back(new Payment(taxName));

    PaymentDetail detail(
        PaymentRuleData(100,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        taxableUnitTagSet,
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        geo1,
        geo2,
        taxName,
        marketingCarrier);
    PaymentDetail* pd = &detail;
    payments[0].paymentDetail().push_back(pd);
    pd->setTotalFareAmount(660);
    pd->taxAmt() = 6;
    pd->taxEquivalentAmount() = 7;
    pd->taxCurrency() = "CR1";
    pd->sabreTaxCode() = "ZP";
    pd->taxOnChangeFeeAmount() = type::MoneyAmount(8);
    _itinsPayments->paymentCurrency = "CR3";
    _atpcoResponse->_itinsPayments.emplace(std::move(*_itinsPayments));

    tse::TaxResponse atseV2Response;
    atseV2Response.farePath() = _farePath;

    tse::Itin tseItin;
    tax::type::CarrierCode cc{"CC"};
    const tax::ItinFarePathKey key = std::make_tuple(&tseItin, atseV2Response.farePath(), cc, 0);

    convertResponseToTaxResponse(
        *_trx, *_atpcoResponse, atseV2Response, *_optionalServicesMapping, key);
    CPPUNIT_ASSERT(atseV2Response.taxItemVector().empty());
    CPPUNIT_ASSERT(!atseV2Response.changeFeeTaxItemVector().empty());

    const tse::TaxItem* taxItem = atseV2Response.changeFeeTaxItemVector()[0];
    CPPUNIT_ASSERT_EQUAL(tse::NationCode("US"), taxItem->nation());
    CPPUNIT_ASSERT_EQUAL(tse::TaxCode("ZP"), taxItem->taxCode());
    CPPUNIT_ASSERT_EQUAL('F', taxItem->taxType());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(8), taxItem->taxAmount());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(6), taxItem->taxAmt());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR3"), taxItem->paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR1"), taxItem->taxCur());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(660), taxItem->taxableFare());
    CPPUNIT_ASSERT_EQUAL(100, taxItem->seqNo());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem->travelSegStartIndex());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem->travelSegEndIndex());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("DFW"), taxItem->taxLocalBoard());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("NYC"), taxItem->taxLocalOff());
  }

  void testTwoTaxes()
  {
    TaxName taxName;
    taxName.nation() = "US";
    taxName.taxCode() = "ZP";
    taxName.taxType() = "002";
    taxName.percentFlatTag() = type::PercentFlatTag::Percent;
    taxName.taxPointTag() = type::TaxPointTag::Departure;

    TaxName taxName2;
    taxName2.nation() = "DE";
    taxName2.taxCode() = "OY";
    taxName2.taxType() = "001";
    taxName2.percentFlatTag() = type::PercentFlatTag::Flat;
    taxName2.taxPointTag() = type::TaxPointTag::Departure;

    Geo geo1;
    geo1.loc().code() = "DFW";
    geo1.id() = 0;
    Geo geo2;
    geo2.loc().code() = "NYC";
    geo2.id() = 1;

    type::CarrierCode marketingCarrier = "LH";
    TaxableUnitTagSet taxableUnitTagSet(TaxableUnitTagSet::none());
    taxableUnitTagSet.setTag(type::TaxableUnit::Itinerary);

    boost::ptr_vector<Payment>& payments =
        _itinPayments->payments(type::ProcessingGroup::Itinerary);
    payments.push_back(new Payment(taxName));

    PaymentDetail detail(
        PaymentRuleData(100,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        taxableUnitTagSet,
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        geo1,
        geo2,
        taxName,
        marketingCarrier);
    PaymentDetail* pd = &detail;
    payments[0].paymentDetail().push_back(pd);
    pd->setTotalFareAmount(660);
    pd->taxAmt() = 6;
    pd->taxEquivalentAmount() = 7;
    pd->taxCurrency() = "CR1";
    pd->sabreTaxCode() = "ZP2";

    payments.push_back(new Payment(taxName2));

    PaymentDetail detail2(
        PaymentRuleData(200,
                        type::TicketedPointTag::MatchTicketedAndUnticketedPoints,
                        taxableUnitTagSet,
                        0,
                        noCurrency,
                        type::TaxAppliesToTagInd::Blank),
        geo1,
        geo2,
        taxName2,
        marketingCarrier);
    PaymentDetail* pd2 = &detail2;
    payments[1].paymentDetail().push_back(pd2);
    pd2->setTotalFareAmount(300);
    pd2->taxAmt() = 8;
    pd2->taxEquivalentAmount() = 9;
    pd2->taxCurrency() = "CR2";
    pd2->sabreTaxCode() = "OY";

    _itinsPayments->paymentCurrency = "CR3";
    _itinsPayments->paymentCurrencyNoDec = 3;
    _atpcoResponse->_itinsPayments.emplace(std::move(*_itinsPayments));

    tse::TaxResponse atseV2Response;
    atseV2Response.farePath() = _farePath;
    tse::Itin tseItin;
    tax::type::CarrierCode cc{"CC"};
    const tax::ItinFarePathKey key = std::make_tuple(&tseItin, atseV2Response.farePath(), cc, 0);
    convertResponseToTaxResponse(
        *_trx, *_atpcoResponse, atseV2Response, *_optionalServicesMapping, key);

    CPPUNIT_ASSERT(!atseV2Response.taxItemVector().empty());

    const tse::TaxItem& taxItem0 = *atseV2Response.taxItemVector()[0];
    CPPUNIT_ASSERT_EQUAL(tse::NationCode("US"), taxItem0.nation());
    CPPUNIT_ASSERT_EQUAL(tse::TaxCode("ZP2"), taxItem0.taxCode());
    CPPUNIT_ASSERT_EQUAL('P', taxItem0.taxType());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(7), taxItem0.taxAmount());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(6), taxItem0.taxAmt());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyNoDec(2), taxItem0.taxNodec());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR3"), taxItem0.paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR1"), taxItem0.taxCur());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(660), taxItem0.taxableFare());
    CPPUNIT_ASSERT_EQUAL(100, taxItem0.seqNo());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem0.travelSegStartIndex());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem0.travelSegEndIndex());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("DFW"), taxItem0.taxLocalBoard());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("NYC"), taxItem0.taxLocalOff());

    const tse::TaxItem& taxItem1 = *atseV2Response.taxItemVector()[1];
    CPPUNIT_ASSERT_EQUAL(tse::NationCode("DE"), taxItem1.nation());
    CPPUNIT_ASSERT_EQUAL(tse::TaxCode("OY"), taxItem1.taxCode());
    CPPUNIT_ASSERT_EQUAL('F', taxItem1.taxType());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(9), taxItem1.taxAmount());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(8), taxItem1.taxAmt());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyNoDec(3), taxItem1.taxNodec());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR3"), taxItem1.paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("CR2"), taxItem1.taxCur());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(300), taxItem1.taxableFare());
    CPPUNIT_ASSERT_EQUAL(200, taxItem1.seqNo());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem1.travelSegStartIndex());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), taxItem1.travelSegEndIndex());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("DFW"), taxItem1.taxLocalBoard());
    CPPUNIT_ASSERT_EQUAL(tse::LocCode("NYC"), taxItem1.taxLocalOff());
  }

  void testBuildOCFeesTaxItem()
  {
    tse::OCFees::TaxItem taxItem;
    boost::optional<IoTaxName> taxName;
    const type::CurrencyCode paymentCurrency("DEF");
    const type::CurDecimals currencyDecimals(3);

    std::shared_ptr<OutputTaxDetails> outputTaxDetails;
    outputTaxDetails.reset(new OutputTaxDetails);
    outputTaxDetails->seqNo() = 1234;
    outputTaxDetails->sabreCode() = "ABC";
    outputTaxDetails->paymentAmt() = 1;
    outputTaxDetails->publishedAmt() = 2;
    outputTaxDetails->publishedCur() = "PFF";

    tse::PricingTrx trx;
    tax::ResponseConverter(trx, trx.atpcoTaxesActivationStatus()).buildOCFeesTaxItem(
        &taxItem, taxName, paymentCurrency, currencyDecimals, outputTaxDetails);

    CPPUNIT_ASSERT_EQUAL(1234, taxItem.getSeqNo());
    CPPUNIT_ASSERT_EQUAL(tse::TaxCode("ABC"), taxItem.getTaxCode());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(1), taxItem.getTaxAmount());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(2), taxItem.getTaxAmountPub());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("DEF"), taxItem.getCurrency());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), taxItem.getNumberOfDec());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("PFF"), taxItem.getCurrencyPub());
  }

  void testBuildTaxItem_exchangeDetails()
  {
    tse::TaxItem taxItem;
    boost::optional<IoTaxName> taxName;
    const type::CurrencyCode paymentCurrency("DEF");
    const type::CurDecimals currencyDecimals(3);

    std::shared_ptr<OutputTaxDetails> outputTaxDetails;
    outputTaxDetails.reset(new OutputTaxDetails);

    boost::optional<OutputExchangeReissueDetails>& exchangeDetails = outputTaxDetails->exchangeDetails();
    exchangeDetails.reset(OutputExchangeReissueDetails());

    boost::optional<OutputCalcDetails>& calcDetails = outputTaxDetails->calcDetails();
    calcDetails.reset(OutputCalcDetails());

    exchangeDetails->isPartialTax = false;
    exchangeDetails->isMixedTax = true;
    exchangeDetails->minTaxAmount = 11;
    exchangeDetails->maxTaxAmount = 21;
    exchangeDetails->minMaxTaxCurrency = ("ABC");
    exchangeDetails->minMaxTaxCurrencyDecimals = 2;

    ResponseConverter::TseData tseData(_trx->ticketingDate(), _farePath);

    tax::ResponseConverter(*_trx, _trx->atpcoTaxesActivationStatus()).buildTaxItem(
        &taxItem, taxName, paymentCurrency, currencyDecimals,
        outputTaxDetails, tseData, ReturnNothing { });

    CPPUNIT_ASSERT(!taxItem.partialTax());
    CPPUNIT_ASSERT(taxItem.mixedTax());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(11), taxItem.minTax());
    CPPUNIT_ASSERT_EQUAL(tse::MoneyAmount(21), taxItem.maxTax());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyCode("ABC"), taxItem.minMaxTaxCurrency());
    CPPUNIT_ASSERT_EQUAL(tse::CurrencyNoDec(2), taxItem.minMaxTaxCurrencyNoDec());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ResponseConverterTest);

} // namespace tax
