
#include "Common/CurrencyUtil.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/Currency.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "ServiceInterfaces/FallbackService.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputCalcDetails.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "Taxes/AtpcoTaxes/Diagnostic/Diagnostic817.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/ItinsPayments.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Response.h"
#include "Taxes/AtpcoTaxes/Rules/PaymentDetail.h"
#include "Taxes/Common/ReissueTaxInfoBuilder.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/ForEachTaxResponse.h"
#include "Taxes/LegacyFacades/GsaLogic.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/ResponseConverter.h"
#include "Taxes/LegacyFacades/TaxOnOcConverter.h"
#include "Taxes/LegacyFacades/TaxPointUtil.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/PfcTaxesExemption/AutomaticPfcTaxExemption.h"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <map>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackFixForRTPricingInSplit);
FALLBACK_DECL(markupAnyFareOptimization);
}

namespace tax
{
tse::Logger
ResponseConverter_DEPRECATED::_logger("AtpcoTaxes.ResponseConverter_DEPRECATED");

namespace
{
tse::CurrencyCode
getCurrency(const tax::type::CurrencyCode& currency,
            const tse::NationCode& nation,
            const tse::DateTime& ticketingDate)
{
  if (!currency.empty())
    return tse::toTseCurrencyCode(currency);

  if (nation.empty())
    return tse::CurrencyCode();

  tse::CurrencyCode nationCurrency;
  tse::CurrencyUtil::getNationCurrency(nation, nationCurrency, ticketingDate);

  return nationCurrency;
}

tse::CarrierCode
getMarketingCarrier(const tse::FarePath* farePath, const size_t travelSegIndex)
{
  if (!farePath || !farePath->itin() || farePath->itin()->travelSeg().size() <= travelSegIndex)
    return tse::CarrierCode("");

  const tse::TravelSeg* travelSeg = farePath->itin()->travelSeg()[travelSegIndex];
  if (travelSeg->isAir())
  {
    const tse::AirSeg* airSeg = static_cast<const tse::AirSeg*>(travelSeg);
    return airSeg->marketingCarrierCode();
  }
  return tse::CarrierCode("");
}

tse::LocCode
getBoardLoc(const type::AirportCode& outputTaxBoardLoc,
            const tse::FarePath* farePath,
            const boost::optional<type::TaxPointTag>& taxPointTag)
{
  if (!taxPointTag || taxPointTag.get() != type::TaxPointTag::Sale || !farePath ||
      !farePath->itin() || farePath->itin()->travelSeg().empty() ||
      !farePath->itin()->travelSeg().front())
    return tse::toTseAirportCode(outputTaxBoardLoc);

  return farePath->itin()->travelSeg().front()->origin()->loc();
}

} // anonymous namespace

class ResponseConverter_DEPRECATED::Impl
{
public:
  static void removeResponsesForEliminatedValCxrs(const ItinFarePathMapping& solutionMap);

  template <typename MapFarePath>
  static void updateV2TrxFromMappings(tse::PricingTrx& trx,
                                      const boost::optional<OutputItins>& solutions,
                                      const ItinFarePathMapping& solutionMap,
                                      const V2TrxMappingDetails& v2TrxMappingDetails,
                                      MapFarePath mapFarePath,
                                      const TaxDependenciesGetter& getTaxDependencies);

  static void
  addOcLikeTaxItems(const boost::optional<type::Index>& groupRefId,
                    const OutputItins& itins,
                    const tse::Itin* const tseItin,
                    const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
                    tse::PricingTrx& trx,
                    tse::TaxResponse* atseV2Response,
                    tse::FarePath* farePath,
                    const TaxDependenciesGetter& getTaxDependencies);
};

void
ResponseConverter_DEPRECATED::Impl::removeResponsesForEliminatedValCxrs(const ItinFarePathMapping& solutionMap)
{
  // Remove responses from eliminated validating carriers
  for (const tax::ItinFarePathMapping::value_type& mapping : solutionMap)
  {
    const tse::Itin* tseItin;
    const tse::FarePath* tseFarePath;
    tax::type::CarrierCode validatingCarrier;
    tax::type::Index id;
    std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;
    std::vector<CarrierCode> validValCxrs;
    validValCxrs.push_back(toTseCarrierCode(validatingCarrier));
    for (auto cxr : tseFarePath->validatingCarriers())
    {
      validValCxrs.push_back(cxr);
    }
    tse::Itin* itin = const_cast<tse::Itin*>(tseItin);
    itin->mutableTaxResponses().erase(
        std::remove_if(itin->mutableTaxResponses().begin(),
                       itin->mutableTaxResponses().end(),
                       [&](TaxResponse* response)
                       {
                         return response->farePath() == tseFarePath &&
                                std::find(validValCxrs.begin(), validValCxrs.end(), response->validatingCarrier())
                                  == validValCxrs.end();
                       }),
        itin->mutableTaxResponses().end());
  }
}

template <typename MapFarePath>
void
ResponseConverter_DEPRECATED::Impl::updateV2TrxFromMappings(tse::PricingTrx& trx,
                                                 const boost::optional<OutputItins>& solutions,
                                                 const ItinFarePathMapping& solutionMap,
                                                 const V2TrxMappingDetails& v2TrxMappingDetails,
                                                 MapFarePath mapFarePath,
                                                 const TaxDependenciesGetter& getTaxDependencies)
{
  if (!trx.atpcoTaxesActivationStatus().isOldTaxesCalculated())
    removeResponsesForEliminatedValCxrs(solutionMap);

  if (ItinSelector(trx).isNewItin())
  {
    for (const tax::ItinFarePathMapping::value_type& mapping : solutionMap)
    {
      const tse::Itin* tseItin;
      const tse::FarePath* tseFarePath;
      tax::type::CarrierCode validatingCarrier;
      tax::type::Index id;
      std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;

      const tse::FarePath* mappedFarePath = mapFarePath(tseFarePath);
      tse::TaxResponse* taxResponse =
          tax::detail::findTaxResponse(tseItin, mappedFarePath, validatingCarrier.asString());

      if (solutions)
      {
        ResponseConverter_DEPRECATED::convertOcToV2Response(trx,
                                                 *solutions,
                                                 v2TrxMappingDetails._optionalServicesRefs,
                                                 mapping,
                                                 taxResponse,
                                                 getTaxDependencies);
      }

      if (LIKELY(taxResponse)) // defensive?
      {
        if (LIKELY(solutions))
        {
          ResponseConverter_DEPRECATED::convertToV2Response(trx,
                                                 *solutions,
                                                 mapping,
                                                 taxResponse,
                                                 getTaxDependencies);
        }

        ResponseConverter_DEPRECATED::buildTicketLineForTaxResponse(trx, *taxResponse);

        if (!fallback::markupAnyFareOptimization(&trx))
        {
          ResponseConverter_DEPRECATED::prepareResponseForMarkups(trx, mapping, taxResponse);
        }
      }
    }
  }
}

void
ResponseConverter_DEPRECATED::Impl::addOcLikeTaxItems(
    const boost::optional<type::Index>& groupRefId,
    const OutputItins& itins,
    const tse::Itin* const tseItin,
    const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
    tse::PricingTrx& trx,
    tse::TaxResponse* atseV2Response,
    tse::FarePath* farePath,
    const TaxDependenciesGetter& getTaxDependencies)
{
  if (LIKELY(!groupRefId))
    return;

  const std::shared_ptr<tax::OutputTaxGroup>& taxGroupSeq = itins.taxGroupSeq()[*groupRefId];

  for (const OutputTax& outputTax : taxGroupSeq->taxSeq())
  {
    for (const OutputTaxDetailsRef& taxDetailsRef : outputTax.taxDetailsRef())
    {
      std::shared_ptr<OutputTaxDetails> outputTaxDetails =
          itins.taxDetailsSeq()[taxDetailsRef.id()];

      if (!tse::taxUtil::isAnciliaryItin(trx, *tseItin))
      {
        ResponseConverter_DEPRECATED::initializeOCFeesTaxItem(trx,
                                                   outputTax.taxName(),
                                                   itins.paymentCur(),
                                                   itins.paymentCurNoDec(),
                                                   optionalServicesMapping,
                                                   outputTaxDetails,
                                                   farePath);
      }
      else
      {
        if (atseV2Response)
        {
          ResponseConverter_DEPRECATED::initializeTaxItem(trx,
                                               *atseV2Response,
                                               outputTax.taxName(),
                                               itins.paymentCur(),
                                               itins.paymentCurNoDec(),
                                               outputTaxDetails,
                                               getTaxDependencies);
        }
      }
    }
  }
}

ResponseConverter_DEPRECATED::TseData::TseData(const tse::DateTime& ticketingDate_,
                                    const tse::FarePath* farePath_)
  : ticketingDate(ticketingDate_), farePath(farePath_)
{
}

void
ResponseConverter_DEPRECATED::updateV2Trx(tse::PricingTrx& trx,
                               const tax::OutputResponse& outResponse,
                               const V2TrxMappingDetails& v2TrxMappingDetails,
                               std::ostream* gsaDiag,
                               const TaxDependenciesGetter& getTaxDependencies)
{
  const boost::optional<OutputItins>& solutions = outResponse.itins();
  const bool doGSA = !v2TrxMappingDetails._farePathMap.empty();

  if (solutions && doGSA)
  {
    ItinFarePathMapping newMapping =
        buildBestFarePathMap(v2TrxMappingDetails, *solutions, trx, gsaDiag);
    Impl::updateV2TrxFromMappings(trx,
                                  solutions,
                                  newMapping,
                                  v2TrxMappingDetails,
                                  GetMainFarePath(v2TrxMappingDetails._farePathMap),
                                  getTaxDependencies);
  }
  else
  {
    Impl::updateV2TrxFromMappings(trx,
                                  solutions,
                                  v2TrxMappingDetails._itinFarePathMapping,
                                  v2TrxMappingDetails,
                                  ForwardFarePath(),
                                  getTaxDependencies);
  }
}

void
ResponseConverter_DEPRECATED::convertDiagnosticResponseToTaxResponse(
    tse::PricingTrx& trx, const DiagnosticResponse& diagnosticResponse)
{
  tse::DCFactory* factory = tse::DCFactory::instance();
  tse::DiagCollector& diagCollector = *(factory->create(trx));
  diagCollector.trx() = &trx;
  diagCollector.enable(trx.diagnostic().diagnosticType());

  ItinSelector itinSelector(trx);
  if (itinSelector.isExchangeTrx() && itinSelector.isExcItin() && trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
    diagCollector << "- BEGIN EXCH ITIN DIAGNOSTIC -" << std::endl;

  if (itinSelector.isNewItin() || trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
  {
    for (const Message& message : diagnosticResponse._messages)
      diagCollector << message._content << std::endl;
  }

  diagCollector.flushMsg();
}

void
ResponseConverter_DEPRECATED::convertToV2Response(
    tse::PricingTrx& trx,
    const OutputItins& solutions,
    const tax::ItinFarePathKey& mapping,
    tse::TaxResponse* atseV2Response,
    const TaxDependenciesGetter& getTaxDependencies)
{
  const tse::Itin* tseItin;
  const tse::FarePath* tseFarePath;
  tax::type::CarrierCode validatingCarrier;
  tax::type::Index id;
  std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;
  const OutputItin& itinSeq = solutions.itinSeq()[id];

  convertTaxGroup(trx, itinSeq.taxGroupId(), solutions, getTaxDependencies, atseV2Response);

  if (trx.atpcoTaxesActivationStatus().isTaxOnChangeFee())
  {
    convertChangeFeeGroup(trx, itinSeq.taxOnChangeFeeGroupRefId(), solutions, getTaxDependencies, atseV2Response);
  }
}

void
ResponseConverter_DEPRECATED::buildTicketLineForTaxResponse(tse::PricingTrx& trx, tse::TaxResponse& taxResponse)
{
  if (trx.atpcoTaxesActivationStatus().isTaxOnChangeFee() ||
      trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
  {
    tse::TaxRecord().buildTicketLine(trx, taxResponse, false, false, true);
  }
}

void
ResponseConverter_DEPRECATED::prepareResponseForMarkups(
    tse::PricingTrx& trx,
    const tax::ItinFarePathKey& mapping,
    const tse::TaxResponse* taxResponse)
{
  const tse::Itin* tseItin;
  const tse::FarePath* tseFarePath;
  tax::type::CarrierCode validatingCarrier;
  tax::type::Index id;
  std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;

  if(!tseFarePath->adjustedSellingFarePath())
  {
    return;
  }

  tse::TaxResponse* taxResponseMarkup = tax::detail::findTaxResponse(
      tseItin, tseFarePath->adjustedSellingFarePath(), validatingCarrier.asString());

  if(!taxResponseMarkup)
  {
    return;
  }

  // TODO MarkupAnyFare Do we need clear? Items should not exist.
  taxResponseMarkup->taxItemVector().clear();

  for(TaxItem* taxItem : taxResponse->taxItemVector())
  {
    TaxItem* newTaxItem = nullptr;
    trx.dataHandle().get(newTaxItem);
    if (newTaxItem == nullptr)
    {
      return;
    }

    *newTaxItem = *taxItem;
    if(newTaxItem->gstTax() && newTaxItem->taxAmountAdjusted())
    {
      newTaxItem->taxAmount() = newTaxItem->taxAmountAdjusted();
    }

    taxResponseMarkup->taxItemVector().push_back(newTaxItem);
  }

  taxResponseMarkup->taxRecordVector().clear();
  ResponseConverter_DEPRECATED::buildTicketLineForTaxResponse(trx, *taxResponseMarkup);
}

void
ResponseConverter_DEPRECATED::convertTaxGroup(
    tse::PricingTrx& trx,
    const boost::optional<type::Index> taxGroupId,
    const OutputItins& solutions,
    const TaxDependenciesGetter& getTaxDependencies,
    tse::TaxResponse* atseV2Response)
{
  if (taxGroupId)
  {
    const std::shared_ptr<tax::OutputTaxGroup>& taxGroupSeq = solutions.taxGroupSeq()[*taxGroupId];

    for (const OutputTax& outputTax : taxGroupSeq->taxSeq())
    {
      for (const OutputTaxDetailsRef& taxDetailsRef : outputTax.taxDetailsRef())
      {
        std::shared_ptr<OutputTaxDetails> outputTaxDetails =
            solutions.taxDetailsSeq()[taxDetailsRef.id()];

        if (!outputTaxDetails->taxableUnitTags().empty() && atseV2Response)
        {
          initializeTaxItem(trx,
                            *atseV2Response,
                            outputTax.taxName(),
                            solutions.paymentCur(),
                            solutions.paymentCurNoDec(),
                            outputTaxDetails,
                            getTaxDependencies);
        }
      }
    }
  }
}

void
ResponseConverter_DEPRECATED::convertChangeFeeGroup(
    tse::PricingTrx& trx,
    const boost::optional<type::Index> taxGroupId,
    const OutputItins& solutions,
    const TaxDependenciesGetter& getTaxDependencies,
    tse::TaxResponse* atseV2Response)
{
  if (taxGroupId)
  {
    const std::shared_ptr<tax::OutputTaxGroup>& taxGroupSeq = solutions.taxGroupSeq()[*taxGroupId];

    for (const OutputTax& outputTax : taxGroupSeq->taxSeq())
    {
      for (const OutputTaxDetailsRef& taxDetailsRef : outputTax.taxDetailsRef())
      {
        std::shared_ptr<OutputTaxDetails> outputTaxDetails =
            solutions.taxDetailsSeq()[taxDetailsRef.id()];

        initializeChangeFeeTaxItem(trx,
                                   *atseV2Response,
                                   outputTax.taxName(),
                                   solutions.paymentCur(),
                                   solutions.paymentCurNoDec(),
                                   outputTaxDetails,
                                   getTaxDependencies);
      }
    }
  }
}

void
ResponseConverter_DEPRECATED::convertOcToV2Response(
    tse::PricingTrx& trx,
    const OutputItins& solutions,
    const V2TrxMappingDetails::OptionalServicesRefs& optionalServicesMapping,
    const tax::ItinFarePathKey& mapping,
    tse::TaxResponse* atseV2Response,
    const TaxDependenciesGetter& getTaxDependencies)
{
  const tse::Itin* tseItin;
  tse::FarePath* tseFarePath;
  tax::type::CarrierCode validatingCarrier;
  tax::type::Index id;
  std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;

  const OutputItin& itinSeq = solutions.itinSeq()[id];

  // taxes on OptionalServices
  boost::optional<type::Index> taxOnOptionalServiceGroupRefId =
      itinSeq.taxOnOptionalServiceGroupRefId();

  Impl::addOcLikeTaxItems(itinSeq.taxOnOptionalServiceGroupRefId(),
                          solutions,
                          tseItin,
                          optionalServicesMapping,
                          trx,
                          atseV2Response,
                          tseFarePath,
                          getTaxDependencies);

  // taxes on Baggage
  Impl::addOcLikeTaxItems(itinSeq.getTaxOnBaggageGroupRefId(),
                          solutions,
                          tseItin,
                          optionalServicesMapping,
                          trx,
                          atseV2Response,
                          tseFarePath,
                          getTaxDependencies);
}

void
ResponseConverter_DEPRECATED::initializeTaxItem(tse::PricingTrx& trx,
                                     tse::TaxResponse& taxResponse,
                                     const boost::optional<IoTaxName>& taxName,
                                     const type::CurrencyCode& paymentCurrency,
                                     const type::CurDecimals& paymentCurrencyNoDec,
                                     std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                     const TaxDependenciesGetter& getTaxDependencies)
{
  tse::TaxItem* pTaxItem = nullptr;
  trx.dataHandle().get(pTaxItem);

  buildTaxItem(trx,
               pTaxItem,
               taxName,
               paymentCurrency,
               paymentCurrencyNoDec,
               outputTaxDetails,
               TseData(trx.ticketingDate(), taxResponse.farePath()),
               getTaxDependencies);

  if (tse::TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      tse::AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse))
  {
    pTaxItem->applyAutomaticPfcTaxExemption(trx, taxResponse);
  }

  taxResponse.taxItemVector().push_back(pTaxItem);
}

void
ResponseConverter_DEPRECATED::buildTaxItem(tse::PricingTrx& trx,
                                tse::TaxItem* pTaxItem,
                                const boost::optional<IoTaxName>& taxName,
                                const type::CurrencyCode& paymentCurrency,
                                const type::CurDecimals& paymentCurrencyNoDec,
                                std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                const TseData& tseData,
                                const TaxDependenciesGetter& getTaxDependencies)
{
  assert(outputTaxDetails);
  assert(pTaxItem);
  assert(getTaxDependencies);

  pTaxItem->exemptFromAtpco() = outputTaxDetails->exempt();
  boost::optional<type::TaxPointTag> taxPointTag;
  if (taxName)
  {
    pTaxItem->nation() = tse::toTseNationCode(taxName->nation());
    pTaxItem->taxCode() = outputTaxDetails->sabreCode();
    pTaxItem->taxType() = static_cast<tse::TaxTypeCode>(taxName->percentFlatTag());
    pTaxItem->atpcoTaxName().taxCode = outputTaxDetails->code().asString();
    pTaxItem->atpcoTaxName().taxType = outputTaxDetails->type().asString();
    pTaxItem->taxOnTaxCode() = getTaxDependencies(*outputTaxDetails);
    taxPointTag = taxName->taxPointTag();
  }

  if (outputTaxDetails->commandExempt() == type::CalcRestriction::ExemptAllTaxesAndFees ||
      outputTaxDetails->commandExempt() == type::CalcRestriction::ExemptAllTaxes)
    pTaxItem->setFailCode(tse::TaxItem::EXEMPT_ALL_TAXES);
  else if (outputTaxDetails->commandExempt() == type::CalcRestriction::ExemptSpecifiedTaxes)
    pTaxItem->setFailCode(tse::TaxItem::EXEMPT_SPECIFIED_TAXES);

  pTaxItem->taxAmount() = tax::amountToDouble(outputTaxDetails->paymentAmt());
  if (!fallback::markupAnyFareOptimization(&trx))
  {
    pTaxItem->taxAmountAdjusted() = tax::amountToDouble(outputTaxDetails->paymentWithMarkupAmt());
  }

  pTaxItem->taxExemptAmount() = tax::amountToDouble(outputTaxDetails->exemptAmt());
  pTaxItem->taxAmt() = tax::amountToDouble(outputTaxDetails->publishedAmt());

  if (outputTaxDetails->percentFlatTag() == type::PercentFlatTag::Percent)
  {
    pTaxItem->taxNodec() = tse::CurrencyNoDec(2);
  }
  else
  {
    pTaxItem->taxNodec() = paymentCurrencyNoDec;
  }

  if (outputTaxDetails->taxOnFaresDetails)
  {
    pTaxItem->taxableFare() = tax::amountToDouble(outputTaxDetails->taxOnFaresDetails->totalAmt);
  }

  pTaxItem->seqNo() = outputTaxDetails->seqNo();
  boost::optional<OutputGeoDetails>& geoDetails = outputTaxDetails->geoDetails();
  if (geoDetails && tseData.farePath && tseData.farePath->itin())
  {
    uint16_t startIndex = 0, endIndex = 0;
    tse::TaxPointUtil::setTravelSegIndices(geoDetails->taxPointIndexBegin(),
                                           geoDetails->taxPointIndexEnd(),
                                           *tseData.farePath->itin(),
                                           startIndex,
                                           endIndex);

    pTaxItem->setTravelSegStartIndex(startIndex);
    pTaxItem->setTravelSegEndIndex(endIndex);

    if (!fallback::fallbackFixForRTPricingInSplit(&trx))
    {
      pTaxItem->setTravelSegStart(tseData.farePath->itin()->travelSeg()[startIndex]);
      pTaxItem->setTravelSegEnd(tseData.farePath->itin()->travelSeg()[endIndex]);
    }

    pTaxItem->setCarrierCode(
        getMarketingCarrier(tseData.farePath, pTaxItem->travelSegStartIndex()));
    pTaxItem->setLegId(tseData.farePath->itin()->travelSeg()[startIndex]->legId());
  }
  pTaxItem->setTaxLocalBoard(
      getBoardLoc(outputTaxDetails->taxPointLocBegin(), tseData.farePath, taxPointTag));
  pTaxItem->setTaxLocalOff(tse::toTseAirportCode(outputTaxDetails->taxPointLocEnd()));
  if (taxPointTag && taxPointTag.get() == type::TaxPointTag::Arrival)
  {
    // std::swap won't work, accessors return const&
    // could integrate board/off choice in functions setTaxLocalBoard/Off functions
    LocCode tmp = pTaxItem->taxLocalOff();
    pTaxItem->setTaxLocalOff(pTaxItem->taxLocalBoard());
    pTaxItem->setTaxLocalBoard(tmp);
  }
  pTaxItem->taxDescription() = outputTaxDetails->name();

  pTaxItem->setPaymentCurrency(tse::toTseCurrencyCode(paymentCurrency));
  pTaxItem->setPaymentCurrencyNoDec(static_cast<uint16_t>(paymentCurrencyNoDec));
  pTaxItem->taxCur() =
      getCurrency(outputTaxDetails->publishedCur(), pTaxItem->nation(), tseData.ticketingDate);

  if (outputTaxDetails->exchangeDetails())
  {
    pTaxItem->setPartialTax(outputTaxDetails->exchangeDetails()->isPartialTax);
    pTaxItem->mixedTax() = outputTaxDetails->exchangeDetails()->isMixedTax;
    pTaxItem->minTax() =
        tax::amountToDouble(outputTaxDetails->exchangeDetails()->minTaxAmount.get_value_or(0));
    pTaxItem->maxTax() =
        tax::amountToDouble(outputTaxDetails->exchangeDetails()->maxTaxAmount.get_value_or(0));

    type::CurrencyCode cur = outputTaxDetails->exchangeDetails()->minMaxTaxCurrency.value_or(
        type::CurrencyCode(UninitializedCode));
    pTaxItem->minMaxTaxCurrency() = tse::toTseCurrencyCode(cur);

    pTaxItem->minMaxTaxCurrencyNoDec() =
        outputTaxDetails->exchangeDetails()->minMaxTaxCurrencyDecimals.get_value_or(0);
  }

  pTaxItem->exchangeRate1() = tax::amountToDouble(outputTaxDetails->calcDetails()->exchangeRate1);
  pTaxItem->exchangeRate1NoDec() = outputTaxDetails->calcDetails()->exchangeRate1NoDec;

  pTaxItem->intermediateUnroundedAmount() =
      tax::amountToDouble(outputTaxDetails->calcDetails()->intermediateUnroundedAmount);
  pTaxItem->intermediateAmount() =
      tax::amountToDouble(outputTaxDetails->calcDetails()->intermediateAmount);
  pTaxItem->intermediateCurrency() = toTseCurrencyCode(outputTaxDetails->calcDetails()->intermediateCurrency);
  pTaxItem->intermediateNoDec() = outputTaxDetails->calcDetails()->intermediateNoDec;

  pTaxItem->exchangeRate2() = tax::amountToDouble(outputTaxDetails->calcDetails()->exchangeRate2);
  pTaxItem->exchangeRate2NoDec() = outputTaxDetails->calcDetails()->exchangeRate2NoDec;

  pTaxItem->reissueTaxInfo() = tse::ReissueTaxInfoBuilder().build(
      trx, outputTaxDetails->code().asString(), outputTaxDetails->type().asString());

  if (!fallback::markupAnyFareOptimization(&trx))
  {
    pTaxItem->gstTax() = taxUtil::isGstTax(pTaxItem->taxCode());
  }
}

void
ResponseConverter_DEPRECATED::initializeOCFeesTaxItem(
    tse::PricingTrx& trx,
    const boost::optional<IoTaxName>& taxName,
    const type::CurrencyCode& paymentCurrency,
    const type::CurDecimals& paymentCurrencyNoDec,
    const V2TrxMappingDetails::OptionalServicesRefs& optServicesMap,
    std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
    tse::FarePath* farePath)
{
  tse::OCFees::TaxItem* pTaxItem = ResponseConverter_DEPRECATED::buildOCFeesTaxItem(
      trx, taxName, paymentCurrency, paymentCurrencyNoDec, outputTaxDetails);
  TaxOnOcConverter converter(optServicesMap, outputTaxDetails, farePath);
  converter.convert(pTaxItem);
}

void
ResponseConverter_DEPRECATED::buildOCFeesTaxItem(tse::OCFees::TaxItem* pTaxItem,
                                      const boost::optional<IoTaxName>& taxName,
                                      const type::CurrencyCode& paymentCurrency,
                                      const type::CurDecimals& paymentCurrencyNoDec,
                                      std::shared_ptr<OutputTaxDetails>& outputTaxDetails)
{
  assert(outputTaxDetails);
  assert(pTaxItem);

  pTaxItem->setSeqNo(outputTaxDetails->seqNo());
  pTaxItem->setTaxCode(outputTaxDetails->sabreCode());

  if (taxName)
  {
    pTaxItem->setTaxType(static_cast<tse::TaxTypeCode>(taxName->percentFlatTag()));
  }

  pTaxItem->setTaxAmount(tax::amountToDouble(outputTaxDetails->paymentAmt()));
  pTaxItem->setCurrency(tse::toTseCurrencyCode(paymentCurrency));
  pTaxItem->setNumberOfDec(static_cast<uint16_t>(paymentCurrencyNoDec));

  pTaxItem->setTaxAmountPub(tax::amountToDouble(outputTaxDetails->publishedAmt()));
  pTaxItem->setCurrencyPub(tse::toTseCurrencyCode(outputTaxDetails->publishedCur()));
}

tse::OCFees::TaxItem*
ResponseConverter_DEPRECATED::buildOCFeesTaxItem(tse::PricingTrx& trx,
                                      const boost::optional<IoTaxName>& taxName,
                                      const type::CurrencyCode& paymentCurrency,
                                      const type::CurDecimals& paymentCurrencyNoDec,
                                      std::shared_ptr<OutputTaxDetails>& outputTaxDetails)
{
  tse::OCFees::TaxItem* pTaxItem;
  trx.dataHandle().get(pTaxItem);

  assert(outputTaxDetails);
  assert(pTaxItem);

  pTaxItem->setSeqNo(outputTaxDetails->seqNo());
  pTaxItem->setTaxCode(outputTaxDetails->sabreCode());

  if (taxName)
  {
    pTaxItem->setTaxType(static_cast<tse::TaxTypeCode>(taxName->percentFlatTag()));
  }

  pTaxItem->setTaxAmount(tax::amountToDouble(outputTaxDetails->paymentAmt()));
  pTaxItem->setCurrency(tse::toTseCurrencyCode(paymentCurrency));
  pTaxItem->setNumberOfDec(static_cast<uint16_t>(paymentCurrencyNoDec));

  pTaxItem->setTaxAmountPub(tax::amountToDouble(outputTaxDetails->publishedAmt()));
  pTaxItem->setCurrencyPub(tse::toTseCurrencyCode(outputTaxDetails->publishedCur()));

  return pTaxItem;
}

void
ResponseConverter_DEPRECATED::initializeChangeFeeTaxItem(tse::PricingTrx& trx,
                                              tse::TaxResponse& taxResponse,
                                              const boost::optional<IoTaxName>& taxName,
                                              const type::CurrencyCode& paymentCurrency,
                                              const type::CurDecimals& paymentCurrencyNoDec,
                                              std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                              const TaxDependenciesGetter& getTaxDependencies)
{
  tse::TaxItem* pTaxItem = nullptr;
  trx.dataHandle().get(pTaxItem);

  buildTaxItem(trx,
               pTaxItem,
               taxName,
               paymentCurrency,
               paymentCurrencyNoDec,
               outputTaxDetails,
               TseData(trx.ticketingDate(), taxResponse.farePath()),
               getTaxDependencies);
  taxResponse.changeFeeTaxItemVector().push_back(pTaxItem);
}
}
