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

#include "Common/CalcDetails.h"
#include "Common/TaxDetailsLevel.h"
#include "DataModel/RequestResponse/BCHOutputResponse.h"
#include "DataModel/RequestResponse/OutputResponse.h"
#include "DomainDataObjects/Response.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/TicketingFee.h"
#include "Factories/MakeSabreCode.h"
#include "Factories/OutputConverter.h"
#include "ServiceInterfaces/Services.h"
#include "Util/BranchPrediction.h"

#include <boost/optional.hpp>

#include <cassert>

namespace tax
{
namespace
{
using TaxDetailSeq = std::vector<std::shared_ptr<OutputTaxDetails>>;
using TaxDetailHash = std::pair<type::MoneyAmount, type::SabreTaxCode>;

template <class T>
struct OutputSharedPtrFinder
{
public:
  explicit OutputSharedPtrFinder(const std::shared_ptr<T>& src) : _source(src) {};

  bool operator()(const std::shared_ptr<T>& first) const { return *first == *_source; }

private:
  const std::shared_ptr<T>& _source;
};

IoTaxName
convertToIoTaxName(const TaxName& taxName)
{
  IoTaxName ioTaxName;
  ioTaxName.nation() = taxName.nation();
  ioTaxName.percentFlatTag() = taxName.percentFlatTag();
  ioTaxName.taxCode() = taxName.taxCode();
  ioTaxName.taxPointTag() = taxName.taxPointTag();
  ioTaxName.taxRemittanceId() = taxName.taxRemittanceId();
  ioTaxName.taxType() = taxName.taxType();

  return ioTaxName;
}

boost::optional<OutputOptionalServiceDetails>
convertToOutputOptionalServiceDetails(const OptionalService& optionalService)
{
  OutputOptionalServiceDetails optionalServiceDetails;

  optionalServiceDetails.amount = optionalService.amount();
  optionalServiceDetails.subCode = optionalService.subCode();
  optionalServiceDetails.serviceGroup = optionalService.serviceGroup();
  optionalServiceDetails.serviceSubGroup = optionalService.serviceSubGroup();
  optionalServiceDetails.optionalServiceType = optionalService.type();
  optionalServiceDetails.ownerCarrier = optionalService.ownerCarrier();
  optionalServiceDetails.pointOfDeliveryLoc = optionalService.pointOfDeliveryLoc();

  boost::optional<OutputOptionalServiceDetails> outputOptionalServiceDetails(
      optionalServiceDetails);
  // outputOptionalServiceDetails.reset(optionalServiceDetails);
  return outputOptionalServiceDetails;
}

// This method need to be rewrite when data is ready (check valgrind)
boost::optional<OutputGeoDetails>
convertToOutputGeoDetails(const PaymentDetail& paymentDetail)
{
  boost::optional<OutputGeoDetails> outputGeoDetails;

  OutputGeoDetails geoDetails;

  // geoDetails.unticketedPoint = paymentDetail.unticketedTransfer();
  geoDetails.taxPointIndexBegin() = paymentDetail.getTaxPointBegin().id();
  geoDetails.taxPointIndexEnd() = paymentDetail.getTaxPointEnd().id();

  // const Geo& g = paymentDetail.getTaxPointLoc1();
  /*
  geoDetails.taxPointLoc1.reset(std::string(paymentDetail.getTaxPointLoc1().locCode()));

  // adding this param causing problems with data corruption
  geoDetails.taxPointLoc2.reset(std::string(paymentDetail.getTaxPointLoc2().locCode()));
  */

  // boost::optional<std::string> taxPointLoc3 = ?
  // if(paymentDetail.hasJourneyLoc1())
  // geoDetails.journeyLoc1.reset(std::string(paymentDetail.getJourneyLoc1().locCode()));
  // if(paymentDetail.hasJourneyLoc2())
  // geoDetails.journeyLoc2.reset(std::string(paymentDetail.getJourneyLoc2().locCode()));
  // geoDetails.pointOfSaleLoc = ?
  // boost::optional<std::string> pointOfTicketingLoc = ?

  outputGeoDetails.reset(geoDetails);
  return outputGeoDetails;
}

// This method need to be rewrite when data is ready
boost::optional<OutputCalcDetails>
convertToOutputCalcDetails(const PaymentDetail& paymentDetail)
{
  boost::optional<OutputCalcDetails> outputCalcDetails;
  outputCalcDetails.reset(OutputCalcDetails());

  OutputCalcDetails& calcDetails = *outputCalcDetails;
  calcDetails.roundingDir = paymentDetail.calcDetails().roundingDir;
  calcDetails.roundingUnit = paymentDetail.calcDetails().roundingUnit;
  if (paymentDetail.calcDetails().intermediateCurrency.empty() ||
      paymentDetail.calcDetails().exchangeRate2 == 0)
  {
    calcDetails.taxCurToPaymentCurBSR = paymentDetail.calcDetails().exchangeRate1;
  }
  else
  {
    calcDetails.taxCurToPaymentCurBSR =
        paymentDetail.calcDetails().exchangeRate1 / paymentDetail.calcDetails().exchangeRate2;
  }

  calcDetails.exchangeRate1 = paymentDetail.calcDetails().exchangeRate1;
  calcDetails.exchangeRate1NoDec = paymentDetail.calcDetails().exchangeRate1NoDec;

  calcDetails.intermediateUnroundedAmount = paymentDetail.calcDetails().intermediateUnroundedAmount;
  calcDetails.intermediateAmount = paymentDetail.calcDetails().intermediateAmount;
  calcDetails.intermediateCurrency = paymentDetail.calcDetails().intermediateCurrency;
  calcDetails.intermediateNoDec = paymentDetail.calcDetails().intermediateNoDec;

  calcDetails.exchangeRate2 = paymentDetail.calcDetails().exchangeRate2;
  calcDetails.exchangeRate2NoDec = paymentDetail.calcDetails().exchangeRate2NoDec;

  return outputCalcDetails;
}

boost::optional<OutputExchangeReissueDetails>
convertToOutputExchangeReissueDetails(const PaymentDetail& paymentDetail)
{
  boost::optional<OutputExchangeReissueDetails> outputExchangeDetails;
  outputExchangeDetails.reset(OutputExchangeReissueDetails());

  OutputExchangeReissueDetails& exchangeDetails = *outputExchangeDetails;
  exchangeDetails.isMixedTax = paymentDetail.exchangeDetails().isMixedTax;
  exchangeDetails.isPartialTax = paymentDetail.exchangeDetails().isPartialTax;
  exchangeDetails.minTaxAmount = paymentDetail.exchangeDetails().minTaxAmount;
  exchangeDetails.maxTaxAmount = paymentDetail.exchangeDetails().maxTaxAmount;
  exchangeDetails.minMaxTaxCurrency = paymentDetail.exchangeDetails().minMaxTaxCurrency;
  exchangeDetails.minMaxTaxCurrencyDecimals =
      paymentDetail.exchangeDetails().minMaxTaxCurrencyDecimals;

  return outputExchangeDetails;
}

std::vector<type::TaxableUnit>
convertAncillaryTaxableUnitTags(const OptionalService& optionalService)
{
  std::vector<type::TaxableUnit> outputTaxableUnits;

  switch (optionalService.type())
  {
  case type::OptionalServiceTag::PrePaid:
    outputTaxableUnits.push_back(type::TaxableUnit::OCFlightRelated);
    break;

  case type::OptionalServiceTag::FlightRelated:
    outputTaxableUnits.push_back(type::TaxableUnit::OCFlightRelated);
    break;

  case type::OptionalServiceTag::TicketRelated:
    outputTaxableUnits.push_back(type::TaxableUnit::OCTicketRelated);
    break;

  case type::OptionalServiceTag::Merchandise:
    outputTaxableUnits.push_back(type::TaxableUnit::OCMerchandise);
    break;

  case type::OptionalServiceTag::FareRelated:
    outputTaxableUnits.push_back(type::TaxableUnit::OCFareRelated);
    break;

  case type::OptionalServiceTag::BaggageCharge:
    outputTaxableUnits.push_back(type::TaxableUnit::BaggageCharge);
    break;

  default:
    break;
  }

  return outputTaxableUnits;
}

OutputFaresDetails
buldTaxOnFares(const PaymentDetail& paymentDetail)
{
  OutputFaresDetails ans;
  ans.totalAmt = paymentDetail.getTotalFareAmount() ? *paymentDetail.getTotalFareAmount() : 0;
  return ans;
}

OutputTaxOnTaxDetails
buildTaxOnTax(const PaymentDetail& paymentDetail)
{
  OutputTaxOnTaxDetails ans;
  ans.totalAmt = paymentDetail.totalTaxOnTaxAmount();
  return ans;
}

OutputYQYRDetails
buildTaxOnYqYr(const PaymentDetail& paymentDetail)
{
  OutputYQYRDetails ans;
  ans.totalAmt = paymentDetail.getYqYrDetails()._taxableAmount;
  return ans;
}

OutputGeoDetails
buildGeoDetails(const OptionalService& optionalService)
{
  OutputGeoDetails ans;
  ans.taxPointIndexBegin() = optionalService.getTaxPointBegin().id();
  ans.taxPointIndexEnd() = optionalService.getTaxPointEnd().id();
  return ans;
}

std::shared_ptr<OutputTaxDetails>
convertToCommonOutputTaxDetails(const PaymentDetail& paymentDetail,
                                const std::vector<type::TaxableUnit>& tagsToUse,
                                const TaxDetailsLevel& detailsLevel,
                                const type::MoneyAmount& paymentAmount,
                                const type::MoneyAmount& paymentWithMarkupAmount = 0)
{
  std::shared_ptr<OutputTaxDetails> outputTaxDetails(new OutputTaxDetails);

  outputTaxDetails->paymentAmt() = paymentDetail.isCommandExempt() ? 0 : paymentAmount;
  outputTaxDetails->exemptAmt() = paymentDetail.isCommandExempt() ? paymentAmount : 0;

  if (paymentWithMarkupAmount)
  {
    outputTaxDetails->paymentWithMarkupAmt() =
        paymentDetail.isCommandExempt() ? 0 : paymentWithMarkupAmount;
  }

  outputTaxDetails->exempt() = paymentDetail.isExempt();
  outputTaxDetails->commandExempt() = paymentDetail.getCommandExempt();
  outputTaxDetails->seqNo() = paymentDetail.seqNo();
  outputTaxDetails->nation() = paymentDetail.taxName().nation();
  outputTaxDetails->code() = paymentDetail.taxName().taxCode();
  outputTaxDetails->type() = paymentDetail.taxName().taxType();
  outputTaxDetails->sabreCode() = paymentDetail.sabreTaxCode();
  outputTaxDetails->publishedAmt() = paymentDetail.taxAmt();
  outputTaxDetails->publishedCur() = paymentDetail.taxCurrency();
  outputTaxDetails->taxPointLocBegin() = paymentDetail.getTaxPointBegin().locCode();
  outputTaxDetails->taxPointLocEnd() = paymentDetail.getTaxPointEnd().locCode();
  outputTaxDetails->name() = paymentDetail.taxLabel();
  outputTaxDetails->totalFareAmount() =
      paymentDetail.getTotalFareAmount() ? *paymentDetail.getTotalFareAmount() : 0;
  outputTaxDetails->carrier() = paymentDetail.taxName().taxCarrier();
  outputTaxDetails->percentFlatTag() = paymentDetail.taxName().percentFlatTag();
  outputTaxDetails->exchangeDetails() = convertToOutputExchangeReissueDetails(paymentDetail);
  outputTaxDetails->gst() = paymentDetail.isVatTax();

  if (detailsLevel.taxOnFare)
    outputTaxDetails->taxOnFaresDetails = buldTaxOnFares(paymentDetail);

  if (detailsLevel.taxOnTax)
    outputTaxDetails->taxOnTaxDetails = buildTaxOnTax(paymentDetail);

  if (detailsLevel.taxOnYqYr)
    outputTaxDetails->taxOnYqYrDetails = buildTaxOnYqYr(paymentDetail);

  if (detailsLevel.calc)
    outputTaxDetails->calcDetails() = convertToOutputCalcDetails(paymentDetail);

  if (detailsLevel.geo)
    outputTaxDetails->geoDetails() = convertToOutputGeoDetails(paymentDetail);

  for (const type::TaxableUnit& tag : tagsToUse)
  {
    if (paymentDetail.taxableUnits().hasTag(tag))
    {
      outputTaxDetails->taxableUnitTags().push_back(tag);
    }
  }

  return outputTaxDetails;
}

std::vector<std::shared_ptr<OutputTaxDetails>>
convertToOutputTaxOptionalDetails(const PaymentDetail& paymentDetail,
                                  const TaxDetailsLevel& detailsLevel)
{
  std::vector<std::shared_ptr<OutputTaxDetails>> outputTaxDetailsVector;
  std::shared_ptr<OutputTaxDetails> outputTaxDetails;

  for (const OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (optionalService.isFailed())
    {
      continue;
    }

    outputTaxDetails.reset(new OutputTaxDetails);
    outputTaxDetails->seqNo() = paymentDetail.seqNo();
    outputTaxDetails->nation() = paymentDetail.taxName().nation();
    outputTaxDetails->code() = paymentDetail.taxName().taxCode();
    outputTaxDetails->type() = paymentDetail.taxName().taxType();
    outputTaxDetails->commandExempt() = paymentDetail.getCommandExempt();
    outputTaxDetails->sabreCode() = optionalService.sabreTaxCode();
    outputTaxDetails->paymentAmt() =
        paymentDetail.isCommandExempt() ? 0 : optionalService.getTaxEquivalentAmount();
    outputTaxDetails->exemptAmt() =
        paymentDetail.isCommandExempt() ? optionalService.getTaxEquivalentAmount() : 0;

    outputTaxDetails->publishedAmt() = paymentDetail.taxAmt();
    outputTaxDetails->publishedCur() = paymentDetail.taxCurrency();
    outputTaxDetails->taxPointLocBegin() = optionalService.getTaxPointBegin().locCode();
    outputTaxDetails->taxPointLocEnd() = optionalService.getTaxPointEnd().locCode();
    outputTaxDetails->name() = paymentDetail.taxLabel();
    outputTaxDetails->taxableUnitTags() = convertAncillaryTaxableUnitTags(optionalService);

    if (detailsLevel.geo)
      outputTaxDetails->geoDetails() = buildGeoDetails(optionalService);

    outputTaxDetails->optionalServiceId() = optionalService.index();

    if (detailsLevel.taxOnOc)
      outputTaxDetails->optionalServiceDetails() =
          convertToOutputOptionalServiceDetails(optionalService);

    outputTaxDetailsVector.push_back(outputTaxDetails);
  }

  return outputTaxDetailsVector;
}

template <typename Rec, typename Pred>
const Rec&
putUnique(std::vector<Rec>& vec, const Rec& rec, const Pred& pred)
{
  typename std::vector<Rec>::const_iterator it = std::find_if(vec.begin(), vec.end(), pred);
  if (it == vec.end())
  {
    vec.push_back(rec);
    rec->id() = vec.size() - 1;
    it = vec.end() - 1;
  }
  return *it;
}

template <typename Rec>
Rec&
putUnique(std::vector<std::shared_ptr<Rec>>& vec, const std::shared_ptr<Rec>& rec)
{
  return *putUnique(vec, rec, OutputSharedPtrFinder<Rec>(rec));
}

OutputTaxDetails&
putUniqueTaxDetailsForItin(const PaymentDetail& paymentDetail,
                           const TaxDetailsLevel& detailsLevel,
                           std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails)
{
  std::vector<type::TaxableUnit> tagsToUse{type::TaxableUnit::YqYr,
                                           type::TaxableUnit::TaxOnTax,
                                           type::TaxableUnit::Itinerary};

  std::shared_ptr<OutputTaxDetails> taxDetail = convertToCommonOutputTaxDetails(
      paymentDetail,
      tagsToUse,
      detailsLevel,
      paymentDetail.taxEquivalentAmount(),
      paymentDetail.taxEquivalentWithMarkupAmount());

  return putUnique(taxDetails, taxDetail);
}

OutputTaxDetails&
putUniqueTaxDetailsForChangeFee(const PaymentDetail& paymentDetail,
                                const TaxDetailsLevel& detailsLevel,
                                std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails)
{
  std::vector<type::TaxableUnit> tagsToUse{type::TaxableUnit::ChangeFee};

  std::shared_ptr<OutputTaxDetails> taxDetail = convertToCommonOutputTaxDetails(
      paymentDetail, tagsToUse, detailsLevel, paymentDetail.taxOnChangeFeeAmount());

  return putUnique(taxDetails, taxDetail);
}

OutputTaxDetails&
putUniqueTaxDetailsForTicketingFee(const PaymentDetail& paymentDetail,
                                   const TicketingFee& ticketingFee,
                                   const TaxDetailsLevel& detailsLevel,
                                   std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetails)
{
  std::vector<type::TaxableUnit> tagsToUse{type::TaxableUnit::TicketingFee};

  std::shared_ptr<OutputTaxDetails> taxDetail = convertToCommonOutputTaxDetails(
      paymentDetail, tagsToUse, detailsLevel, ticketingFee.taxAmount());

  return putUnique(taxDetails, taxDetail);
}

void
putTaxOnTaxReferences(std::vector<std::shared_ptr<OutputTaxDetails>>& outTaxDetails,
                      std::vector<OutputTaxDetailsRef>& outTaxDetailsRefs,
                      const std::vector<PaymentDetail*>& inTaxOnTaxItems,
                      const TaxDetailsLevel& detailsLevel)
{
  for (PaymentDetail* subDetail : inTaxOnTaxItems)
  {
    // this conversion is almost always redundant, but I cannot be sure if the record I am pointing
    // to has been skipped in deduplication.
    assert(subDetail);
    const OutputTaxDetails& taxDetail =
        putUniqueTaxDetailsForItin(*subDetail, detailsLevel, outTaxDetails);
    outTaxDetailsRefs.push_back(OutputTaxDetailsRef(*taxDetail.id()));
  }
}

void
processTaxesOnItin(const std::string& taxGroupName,
                   const ItinPayments& itinPayments,
                   const TaxDetailsLevel& detailsLevel,
                   boost::optional<OutputItins>& itins)
{
  type::MoneyAmount outputItinTotalAmt = 0;
  std::shared_ptr<OutputTaxGroup> outputTaxGroup(new OutputTaxGroup(taxGroupName));

  for (const Payment& payment : itinPayments.payments(type::ProcessingGroup::Itinerary))
  {
    std::unique_ptr<OutputTax> outputTax(new OutputTax());
    outputTax->taxName() = convertToIoTaxName(payment.taxName());
    outputTax->totalAmt() = payment.totalityAmt();

    for (const PaymentDetail* paymentDetail : payment.paymentDetail())
    {
      if (!paymentDetail->isValidForGroup(type::ProcessingGroup::Itinerary))
      {
        continue;
      }

      OutputTaxDetails& outTaxDetail =
          putUniqueTaxDetailsForItin(*paymentDetail, detailsLevel, itins->taxDetailsSeq());

      outputTax->taxDetailsRef().push_back(OutputTaxDetailsRef(*outTaxDetail.id()));

      if (detailsLevel.taxOnTax)
        putTaxOnTaxReferences(itins->taxDetailsSeq(),
                              outTaxDetail.taxOnTaxDetails->taxDetailsRef,
                              paymentDetail->taxOnTaxItems(),
                              detailsLevel);
    }
    outputItinTotalAmt += *(outputTax->totalAmt());

    if (outputTax->taxDetailsRef().size() > 0)
    {
      outputTaxGroup->taxSeq().push_back(outputTax.release());
    }
  }

  if (!outputTaxGroup->taxSeq().empty())
  {
    const OutputTaxGroup& taxGroup = putUnique(itins->taxGroupSeq(), outputTaxGroup);
    itins->itinSeq().back().taxGroupId() = taxGroup.id();

    outputTaxGroup->totalAmt() = outputItinTotalAmt;
  }
}

const OutputTaxGroup*
processTaxesOcLike(const std::string& taxGroupName,
                   const boost::ptr_vector<Payment>& payments,
                   const TaxDetailsLevel& detailsLevel,
                   boost::optional<OutputItins>& itins)
{
  std::shared_ptr<OutputTaxGroup> outputTaxGroup(new OutputTaxGroup(taxGroupName));

  for (const Payment& payment : payments)
  {
    std::unique_ptr<OutputTax> outputTax(new OutputTax());
    outputTax->taxName() = convertToIoTaxName(payment.taxName());
    outputTax->totalAmt() = payment.totalityAmt();

    for (const PaymentDetail* paymentDetail : payment.paymentDetail())
    {
      for (std::shared_ptr<OutputTaxDetails> outputTaxDetails :
           convertToOutputTaxOptionalDetails(*paymentDetail, detailsLevel))
      {
        const OutputTaxDetails& taxDetail = putUnique(itins->taxDetailsSeq(), outputTaxDetails);
        outputTax->taxDetailsRef().push_back(OutputTaxDetailsRef(*taxDetail.id()));
      }
    }

    if (outputTax->taxDetailsRef().size() > 0)
    {
      outputTaxGroup->taxSeq().push_back(outputTax.release());
    }
  }

  if (UNLIKELY(!outputTaxGroup->taxSeq().empty()))
    return &putUnique(itins->taxGroupSeq(), outputTaxGroup);
  else
    return nullptr;
}

void
processTaxesOnOptionalServices(const std::string& taxGroupName,
                               const boost::ptr_vector<Payment>& payments,
                               const TaxDetailsLevel& detailsLevel,
                               boost::optional<OutputItins>& itins)
{
  if (const OutputTaxGroup* taxGroup = processTaxesOcLike(taxGroupName, payments, detailsLevel, itins))
    itins->itinSeq().back().taxOnOptionalServiceGroupRefId() = taxGroup->id();
}

void
processTaxesOnBaggage(const std::string& taxGroupName,
                      const boost::ptr_vector<Payment>& payments,
                      const TaxDetailsLevel& detailsLevel,
                      boost::optional<OutputItins>& itins)
{
  if (const OutputTaxGroup* taxGroup = processTaxesOcLike(taxGroupName, payments, detailsLevel, itins))
    itins->itinSeq().back().setTaxOnBaggageGroupRefId(taxGroup->id());
}

void
processTaxesOnChangeFee(const std::string& taxGroupName,
                        const ItinPayments& itinPayments,
                        const TaxDetailsLevel& detailsLevel,
                        boost::optional<OutputItins>& itins)
{
  std::shared_ptr<OutputTaxGroup> outputTaxGroup(new OutputTaxGroup(taxGroupName));

  for (const Payment& payment : itinPayments.payments(type::ProcessingGroup::ChangeFee))
  {
    std::unique_ptr<OutputTax> outputTax(new OutputTax());
    outputTax->taxName() = convertToIoTaxName(payment.taxName());

    for (const PaymentDetail* paymentDetail : payment.paymentDetail())
    {
      if (paymentDetail->taxOnChangeFeeAmount() != 0)
      {
        OutputTaxDetails& outTaxDetail =
            putUniqueTaxDetailsForChangeFee(*paymentDetail, detailsLevel, itins->taxDetailsSeq());

        outputTax->taxDetailsRef().push_back(OutputTaxDetailsRef(*outTaxDetail.id()));

        outputTax->totalAmt() = paymentDetail->taxOnChangeFeeAmount();
        break;
      }
    }

    if (outputTax->taxDetailsRef().size() > 0)
    {
      outputTaxGroup->taxSeq().push_back(outputTax.release());
    }
  }

  if (!outputTaxGroup->taxSeq().empty())
  {
    const OutputTaxGroup& taxGroup = putUnique(itins->taxGroupSeq(), outputTaxGroup);
    itins->itinSeq().back().taxOnChangeFeeGroupRefId() = taxGroup.id();
  }
}

void
processTaxesOnTicketingFee(const std::string& taxGroupName,
                           const ItinPayments& itinPayments,
                           const TaxDetailsLevel& detailsLevel,
                           boost::optional<OutputItins>& itins)
{
  std::shared_ptr<OutputTaxGroup> outputTaxGroup(new OutputTaxGroup(taxGroupName));

  for (const Payment& payment : itinPayments.payments(type::ProcessingGroup::OB))
  {
    std::unique_ptr<OutputTax> outputTax(new OutputTax());
    outputTax->taxName() = convertToIoTaxName(payment.taxName());

    for (const PaymentDetail* paymentDetail : payment.paymentDetail())
    {
      for (const TicketingFee& each : paymentDetail->ticketingFees())
      {
        OutputTaxDetails& outTaxDetail = putUniqueTaxDetailsForTicketingFee(
            *paymentDetail, each, detailsLevel, itins->taxDetailsSeq());
        outputTax->taxDetailsRef().push_back(OutputTaxDetailsRef(*outTaxDetail.id()));
        outputTax->totalAmt() = each.taxAmount();
      }
    }

    if (outputTax->taxDetailsRef().size() > 0)
      outputTaxGroup->taxSeq().push_back(outputTax.release());
  }

  if (!outputTaxGroup->taxSeq().empty())
    putUnique(itins->taxGroupSeq(), outputTaxGroup);
}

void
convertItinsPayments(const boost::optional<ItinsPayments>& itinsPayment,
                     const TaxDetailsLevel& detailsLevel,
                     boost::optional<OutputItins>& itins,
                     const Request* request)
{
  if (!itinsPayment)
    return;

  itins.reset(OutputItins());

  itins->paymentCur() = itinsPayment->paymentCurrency;
  itins->paymentCurNoDec() = itinsPayment->paymentCurrencyNoDec;

  if (request && (request->itins().size() != request->allItins().size()))
  {
    for (type::Index i = 0; i < request->allItinsMap().size(); ++i)
    {
      std::map<type::Index, type::Index>::const_iterator it = request->allItinsMap().find(i);
      itins->itinSeq().push_back(new OutputItin(i));
      const ItinPayments& itinPayments = itinsPayment->_itinPayments[it->second];

      processTaxesOnItin("I", itinPayments, detailsLevel, itins);
      processTaxesOnOptionalServices(
          "O", itinPayments.payments(type::ProcessingGroup::OC), detailsLevel, itins);
      processTaxesOnBaggage(
          "B", itinPayments.payments(type::ProcessingGroup::Baggage), detailsLevel, itins);
      processTaxesOnChangeFee("X", itinPayments, detailsLevel, itins);
      processTaxesOnTicketingFee("T", itinPayments, detailsLevel, itins);
    }
  }
  else
  {
    for (const ItinPayments& itinPayments : itinsPayment->_itinPayments)
    {
      // Fill OutputItin
      itins->itinSeq().push_back(new OutputItin(itinPayments.itinId()));

      processTaxesOnItin("I", itinPayments, detailsLevel, itins);
      processTaxesOnOptionalServices(
          "O", itinPayments.payments(type::ProcessingGroup::OC), detailsLevel, itins);
      processTaxesOnBaggage(
          "B", itinPayments.payments(type::ProcessingGroup::Baggage), detailsLevel, itins);
      processTaxesOnChangeFee("X", itinPayments, detailsLevel, itins);
      processTaxesOnTicketingFee("T", itinPayments, detailsLevel, itins);
    }
  }
}

void convertSingleItinPayments(const ItinPayments& itinPayments,
                               boost::ptr_vector<BCHOutputItin>& itins,
                               boost::ptr_vector<BCHOutputTaxDetail>& taxDetails,
                               std::map<TaxDetailHash, type::Index>& taxDetailHashes)
{
  itins.back().mutablePaxDetail().setPtc(itinPayments.requestedPassengerCode());
  for (const Payment& payment : itinPayments.payments(type::ProcessingGroup::Itinerary))
  {
    if (payment.totalityAmt() == 0)
      continue;

    std::unique_ptr<BCHOutputTaxDetail> taxDetail = std::make_unique<BCHOutputTaxDetail>();

    const TaxName& taxName = payment.taxName();
    taxDetail->setSabreCode(
        makeItinSabreCode(taxName.taxCode(), taxName.taxType(), taxName.percentFlatTag()));
    taxDetail->setPaymentAmt(payment.totalityAmt());
    taxDetail->setName(payment.paymentDetail().front()->taxLabel());

    TaxDetailHash hash{std::make_pair(taxDetail->getPaymentAmt(), taxDetail->getSabreCode())};
    auto it = taxDetailHashes.find(hash);
    if (it == taxDetailHashes.end())
    {
      taxDetail->setId(taxDetails.size());
      itins.back().mutablePaxDetail().mutableTaxIds().insert(taxDetail->getId());
      taxDetailHashes[hash] = taxDetail->getId();
      taxDetails.push_back(taxDetail.release());
    }
    else
    {
      itins.back().mutablePaxDetail().mutableTaxIds().insert(it->second);
    }

    itins.back().mutablePaxDetail().increaseTotalAmount(payment.totalityAmt());
  }
}

void convertItinsPayments(const boost::optional<ItinsPayments>& itinsPayment,
                          const Request* request,
                          boost::ptr_vector<BCHOutputItin>& itins,
                          boost::ptr_vector<BCHOutputTaxDetail>& taxDetails)
{
  if (!itinsPayment)
    return;

  std::map<TaxDetailHash, type::Index> taxDetailHashes;
  if (request && (request->itins().size() != request->allItins().size()))
  {
    for (type::Index i = 0; i < request->allItinsMap().size(); ++i)
    {
      std::map<type::Index, type::Index>::const_iterator it = request->allItinsMap().find(i);
      const ItinPayments& itinPayments = itinsPayment->_itinPayments[it->second];

      itins.push_back(new BCHOutputItin(i));
      convertSingleItinPayments(itinPayments, itins, taxDetails, taxDetailHashes);
    }
  }
  else
  {
    for (const ItinPayments& itinPayments : itinsPayment->_itinPayments)
    {
      itins.push_back(new BCHOutputItin(itinPayments.itinId()));
      convertSingleItinPayments(itinPayments, itins, taxDetails, taxDetailHashes);
    }
  }
}

void
convertDiagnosticResponse(const boost::optional<DiagnosticResponse>& diagnostic,
                          boost::optional<OutputDiagnostic>& outputDiagnostic)
{
  if (diagnostic)
  {
    outputDiagnostic.reset(OutputDiagnostic());
    for (const Message& m : diagnostic->_messages)
    {
      outputDiagnostic->messages().push_back(new OutputMessage(m._content));
    }
  }
}

void
convertErrorMessage(const boost::optional<ErrorMessage>& error,
                    boost::optional<OutputError>& outputError)
{
  if (error)
  {
    outputError.reset(OutputError());
    outputError->messages().push_back(new OutputMessage(error->_content));
    if (error->_errorDetail.size() > 0)
      outputError->messages().push_back(new OutputMessage(error->_errorDetail));
  }
}

} // anonymous namespace

OutputResponse
OutputConverter::convertToTaxRs(const Response& response,
                                const TaxDetailsLevel& detailsLevel,
                                const Request* request /*= 0*/)
{
  OutputResponse taxRs;
  taxRs.echoToken() = response._echoToken;
  convertItinsPayments(response._itinsPayments, detailsLevel, taxRs.itins(), request);
  convertDiagnosticResponse(response._diagnosticResponse, taxRs.diagnostic());
  convertErrorMessage(response._error, taxRs.error());

  return taxRs;
}

BCHOutputResponse
OutputConverter::convertToBCHTaxRs(const Response& response,
                                   const Request* request /* = 0*/)
{
  BCHOutputResponse taxRs;
  convertItinsPayments(response._itinsPayments,
                       request,
                       taxRs.mutableItins(),
                       taxRs.mutableTaxDetails());
  convertDiagnosticResponse(response._diagnosticResponse, taxRs.mutableDiagnostic());
  convertErrorMessage(response._error, taxRs.mutableError());

  return taxRs;
}

} // anmespace tax
