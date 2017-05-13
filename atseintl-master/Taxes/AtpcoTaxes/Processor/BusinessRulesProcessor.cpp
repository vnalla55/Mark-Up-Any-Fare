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

#include "Common/DiagnosticUtil.h"
#include "DataModel/Common/GeoPathProperties.h"
#include "DataModel/RequestResponse/InputRequest.h"

#include "Diagnostic/BuildInfoDiagnostic.h"
#include "Diagnostic/DBDiagnostic.h"
#include "Diagnostic/InputRequestDiagnostic.h"
#include "Diagnostic/RequestDiagnostic.h"
#include "Diagnostic/PositiveDiagnostic.h"
#include "Diagnostic/NegativeDiagnostic.h"
#include "Diagnostic/Diagnostic817.h"

#include "DomainDataObjects/ErrorMessage.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/Response.h"

#include "Processor/ApplicatorFactory.h"
#include "Processor/ApplyRuleFunctor.h"
#include "Processor/CheckRemove.h"
#include "Processor/RawSubjectsCollector.h"
#include "Processor/RequestAnalyzer.h"
#include "Processor/TaxPointValidationProgress.h"
#include "Processor/TaxValidator.h"

#include "Rules/BusinessRule.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/GeoPathPropertiesCalculator.h"
#include "Rules/PaymentDetail.h"
#include "Rules/RequestLogicError.h"
#include "Rules/ServiceBaggageRule.h"
#include "Rules/TaxApplicationLimitProcessor.h"
#include "Rules/TaxData.h"
#include "Rules/TaxLimiter.h"

#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

#include <set>

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(fallbackAtpcoTaxTotalRounding)
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{
namespace
{
const type::Nation
ZZ("ZZ");
const type::TaxCode
AY("AY");

struct TaxKeyMatcher
{
  bool isSimple(const TaxKey& a) const { return !a.second.empty(); }
  bool operator()(const TaxKey& a,
                  const TaxKey& b) const
  {
    return (a.first == b.first) && (a.second == b.second || a.second.empty() || b.second.empty());
  }
};

size_t
estimateDeparture(const TaxData& tax,
                  const GeoPath& geoPath,
                  const type::ProcessingGroup& processingGroup,
                  type::Timestamp ticketingDate)
{
  size_t ans = 0U;

  for (type::Index id = 0; id < geoPath.geos().size(); id += 2)
  {
    const Geo& taxPoint = geoPath.geos()[id];
    if (tax.getTaxName().nation() == ZZ || tax.getTaxName().nation() == taxPoint.getNation())
    {
      ans += tax.getDateFilteredSize(processingGroup, ticketingDate);
    }
  }

  return ans;
}

size_t
estimateArrival(const TaxData& tax,
                const GeoPath& geoPath,
                const type::ProcessingGroup& processingGroup,
                type::Timestamp ticketingDate)
{
  size_t ans = 0U;

  for (type::Index id = geoPath.geos().size(); id > 1; id -= 2)
  {
    const Geo& taxPoint = geoPath.geos()[id - 1];
    if (tax.getTaxName().nation() == ZZ || tax.getTaxName().nation() == taxPoint.getNation())
    {
      ans += tax.getDateFilteredSize(processingGroup, ticketingDate);
    }
  }

  return ans;
}

bool
taxMatchesItinOrSalePoint(const TaxData& tax, const GeoPath& geoPath, const Geo& pointOfSale)
{
  if (UNLIKELY(tax.getTaxName().nation() == ZZ))
    return true;

  if (tax.getTaxName().nation() == pointOfSale.getNation())
    return true;

  for (const Geo& taxPoint : geoPath.geos())
  {
    if (tax.getTaxName().nation() == taxPoint.getNation())
      return true;
  }

  return false;
}

size_t
estimatSale(const TaxData& tax,
            const GeoPath& geoPath,
            const type::ProcessingGroup& processingGroup,
            const Geo& pointOfSale,
            type::Timestamp ticketingDate)
{
  if (taxMatchesItinOrSalePoint(tax, geoPath, pointOfSale))
  {
    return tax.getDateFilteredSize(processingGroup, ticketingDate);
  }
  else
  {
    return 0U;
  }
}

size_t
estimatePaymentDetailCount(const OrderedTaxes& orderedTaxes,
                           const GeoPath& geoPath,
                           const type::ProcessingGroup& processingGroup,
                           const Geo& pointOfSale,
                           type::Timestamp ticketingDate)
{
  size_t ans = 0L;

  for (const TaxValues& taxValues : orderedTaxes)
  {
    for (const TaxValue& tax : taxValues)
    {
      const TaxName& taxName = tax->getTaxName();
      if (taxName.taxPointTag() == type::TaxPointTag::Departure)
        ans += estimateDeparture(*tax, geoPath, processingGroup, ticketingDate);
      else if (taxName.taxPointTag() == type::TaxPointTag::Arrival)
        ans += estimateArrival(*tax, geoPath, processingGroup, ticketingDate);
      else if (LIKELY(taxName.taxPointTag() == type::TaxPointTag::Sale))
        ans += estimatSale(*tax, geoPath, processingGroup, pointOfSale, ticketingDate);
    }
  }
  return ans;
}

void
addEdges(const std::shared_ptr<BusinessRulesContainer>& rulesContainer,
         const TaxKey& taxKey,
         ProcessingOrderer& orderer,
         Services& services)
{
  std::shared_ptr<const ServiceBaggage> serviceBaggage =
      services.serviceBaggageService().getServiceBaggage(rulesContainer->vendor(),
                                                         rulesContainer->getServiceBaggageItemNo());

  if (!serviceBaggage)
    return;

  if (rulesContainer->getServiceBaggageApplTag() == type::ServiceBaggageApplTag::E &&
      !rulesContainer->taxableUnits().hasTag(type::TaxableUnit::TaxOnTax))
    return;

  for (const ServiceBaggage::entry_type& entry : serviceBaggage->entries)
  {
    if (entry.taxCode != "OC" && entry.taxCode != "YQ" && entry.taxCode != "YR")
    {
      const bool correctEntry = entry.taxTypeSubcode.empty() || entry.taxTypeSubcode.size() == 3U;
      if (BOOST_LIKELY(correctEntry)) // otherwise data error
      {
        type::TaxType entryTaxType;
        codeFromString(entry.taxTypeSubcode, entryTaxType);
        orderer.addEdge(taxKey, std::make_pair(entry.taxCode, entryTaxType));
      }
    }
  }
}

void
addEdges(const TaxKey& taxKey,
         const std::vector<std::shared_ptr<BusinessRulesContainer>>& rulesContainers,
         ProcessingOrderer& orderer,
         Services& services)
{
  for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer : rulesContainers)
  {
    if (rulesContainer->getServiceBaggageItemNo() != 0)
    {
      addEdges(rulesContainer, taxKey, orderer, services);
    }
  }
}

void
collectProcessingData(const type::Nation& nation,
                      const Geo& geo,
                      const type::Timestamp& ticketingDate,
                      const ProcessingOptions& processingOptions,
                      ProcessingOrderer& orderer,
                      TaxValues& lastGroup,
                      Request::ContainersMap& containersMap,
                      Services& services)
{
  Request::ContainersMap::key_type key(nation, geo.loc().tag());
  auto value = services.rulesRecordsService().getTaxRulesContainers(nation, geo.loc().tag(), ticketingDate);
  auto insertResult = containersMap.insert(Request::ContainersMap::value_type(key, value));
  auto rulesContainersGroupedByTaxName = std::make_pair(insertResult.first->second, insertResult.second);

  // data was already collected for this nation and tax point tag for this request
  if (!rulesContainersGroupedByTaxName.second)
    return;

  const std::vector<tax::type::ProcessingGroup>& allGroups = processingOptions.getProcessingGroups();

  const bool shouldProcessTaxOnTax =
      std::find(allGroups.begin(), allGroups.end(), type::ProcessingGroup::Itinerary) !=
      allGroups.end();

  for (const TaxData& taxData : *rulesContainersGroupedByTaxName.first)
  {
    const TaxName& taxName = taxData.getTaxName();
    TaxKey taxKey = std::make_pair(taxName.taxCode(), taxName.taxType());

    bool isInLastGroup = false;
    // only rulesForItinerary may contain TaxOnTax
    if (shouldProcessTaxOnTax)
    {
      for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer :
           taxData.getDateFilteredCopy(type::ProcessingGroup::Itinerary, ticketingDate))
      {
        if (rulesContainer->getServiceBaggageItemNo() == 0 &&
            rulesContainer->taxableUnits().hasTag(type::TaxableUnit::TaxOnTax))
        {
          lastGroup.push_back(&taxData);
          isInLastGroup = true;
          break;
        }
      }

      if (isInLastGroup)
        continue;
    }

    orderer.addValue(taxKey, &taxData);
    for (const tax::type::ProcessingGroup processingGroup : allGroups)
      addEdges(taxKey, taxData.getDateFilteredCopy(processingGroup, ticketingDate), orderer, services);
  }
}

void
collectProcessingData(const Itin& itin,
                      const Request& request,
                      ProcessingOrderer& orderer,
                      TaxValues& lastGroup,
                      Request::ContainersMap& containersMap,
                      Services& services)
{
  std::set<type::Nation> nationsSet;
  nationsSet.insert(ZZ);
  type::Timestamp ticketingDate(request.ticketingOptions().ticketingDate(),
                                request.ticketingOptions().ticketingTime());
  const GeoPath& geoPath = request.geoPaths()[itin.geoPathRefId()];

  for (const Geo& geo : geoPath.geos())
  {
    if (UNLIKELY(geo.loc().tag() != type::TaxPointTag::Departure &&
        geo.loc().tag() != type::TaxPointTag::Arrival))
      continue;

    nationsSet.insert(geo.loc().nation());
    collectProcessingData(geo.loc().nation(), geo, ticketingDate, request.processing(), orderer, lastGroup, containersMap, services);
    collectProcessingData(ZZ, geo, ticketingDate, request.processing(), orderer, lastGroup, containersMap, services);
  }
  nationsSet.insert(request.posTaxPoints()[itin.pointOfSaleRefId()].loc().nation());

  if (LIKELY(request.posTaxPoints()[itin.pointOfSaleRefId()].loc().tag() == type::TaxPointTag::Sale))
  {
    const Geo& posGeo = request.posTaxPoints()[itin.pointOfSaleRefId()];
    for (const type::Nation& nation : nationsSet)
    {
      collectProcessingData(nation, posGeo, ticketingDate, request.processing(), orderer, lastGroup, containersMap, services);
    }
  }
}


} // anonymous namespace

namespace BusinessRulesProcessorUtils
{

void
getOrderedTaxes(Request& request,
                Services& services,
                ProcessingOrderer& orderer,
                TaxValues& lastGroup,
                OrderedTaxes& orderedTaxes)
{
  for (Itin const* itin : request.itins())
  {
    collectProcessingData(*itin, request, orderer, lastGroup, request.getContainersMap(), services);
  }
  orderer.commit(TaxKeyMatcher());
  boost::optional<const std::vector<TaxValue>&> taxes;
  while ((taxes = orderer.getNext()))
  {
    orderedTaxes.push_back(boost::cref(*taxes));
  }
  orderedTaxes.push_back(boost::cref(lastGroup));
}

} // namespace BusinessRulesProcessorUtils

class RequestLogicError;

PaymentWithRules::PaymentWithRules(PaymentDetail& detail, const CalculatorsGroups* groups)
  : paymentDetail(&detail), calculatorsGroups(groups)
{
}

BusinessRulesProcessor::BusinessRulesProcessor(Services& services) : _services(services)
{
}

BusinessRulesProcessor::~BusinessRulesProcessor()
{
}

void
BusinessRulesProcessor::run(Request& request, const AtpcoTaxesActivationStatus& activationStatus)
{
  _ticketingDate = type::Timestamp(request.ticketingOptions().ticketingDate(),
                                   request.ticketingOptions().ticketingTime());
  _response._echoToken = request.echoToken();
  if (createDBDiagnosticResponse(request, _services) || !analyzeRequest(request, _services) ||
      createRequestDiagnosticResponse(request.diagnostic(), request))
  {
    return;
  }

  parseFilterParameters(request.diagnostic());

  ProcessingOrderer orderer;
  TaxValues lastGroup;
  OrderedTaxes orderedTaxes;
  BusinessRulesProcessorUtils::getOrderedTaxes(request, _services, orderer, lastGroup, orderedTaxes);

  ItinsRawPayments itinsRawPayments = calculateRawPayments(request, orderedTaxes);
  ItinsPayments itinsPayments =
      filterPaymentsByDiagnostic(request.processing().getProcessingGroups(),
                                 itinsRawPayments,
                                 request.itins(),
                                 request.diagnostic());

  boost::swap(itinsPayments._itinsRawPayments, itinsRawPayments);
  // itinsRawPayments is corrupt from now on
  itinsPayments.paymentCurrency = request.ticketingOptions().paymentCurrency();
  itinsPayments.paymentCurrencyNoDec =
      _services.currencyService().getCurrencyDecimals(itinsPayments.paymentCurrency);

  _detailsLevel = request.processing().taxDetailsLevel();

  addBuildInfoDiagnosticToResponse(request.diagnostic(), request.buildInfo());

  if (!createPostprocessResponse(request, std::move(itinsPayments), activationStatus))
  {
    throw RequestLogicError() << "UNKNOWN DIAGNOSTIC";
  }
}

void
BusinessRulesProcessor::runSingleItin(const Request& request,
                                      type::Index itinIndex,
                                      const OrderedTaxes& orderedTaxes,
                                      ItinsRawPayments& itinsRawPayments,
                                      ItinsPayments& itinsPayments)
{
  _ticketingDate = type::Timestamp(request.ticketingOptions().ticketingDate(),
                                   request.ticketingOptions().ticketingTime());
  _response._echoToken = request.echoToken();

  parseFilterParameters(request.diagnostic());
  calculateRawPaymentsSingleItin(request, orderedTaxes, itinIndex, itinsRawPayments);
  filterPaymentsSingleItin(request.processing().getProcessingGroups(),
                           itinsRawPayments,
                           request.itins(),
                           itinIndex,
                           itinsPayments);

  _detailsLevel = request.processing().taxDetailsLevel();
}



type::CarrierCode
BusinessRulesProcessor::getMarketingCarrier(const Request& request,
                                            const Geo& taxPoint,
                                            const type::Index& itinId)
{
  const Flight* f = taxPoint.getFlight(request.getItinByIndex(itinId).flightUsages());
  return f->marketingCarrier();
}

void
BusinessRulesProcessor::limitTax(const type::Index& itinIndex,
                                 const Request& request,
                                 RawPayments& rawPayments,
                                 std::vector<PaymentWithRules>& paymentsToCalculate,
                                 bool isAYTax /* = false */)
{
  // Itinerary
  std::vector<PaymentWithRules> paymentsToLimit;

  TaxLimiter::overlapItinerary(paymentsToCalculate);

  if (isAYTax)
  {
    for (RawPayments::value_type& each : rawPayments)
    {
      if (each.detail.isValidated() &&
          !each.detail.getItineraryDetail().isFailedRule() &&
          each.detail.taxName().taxCode() == AY)
        paymentsToLimit.push_back(PaymentWithRules(each.detail, nullptr));
    }
  }
  else
  {
    for (PaymentWithRules& each : paymentsToCalculate)
    {
      if (!each.paymentDetail->getItineraryDetail().isFailedRule() ||
          !each.paymentDetail->areAllOptionalServicesFailed())
        paymentsToLimit.push_back(each);
    }
  }

  const Itin& itin = request.getItinByIndex(itinIndex);
  TaxApplicationLimitProcessor(request.getGeoPath(itinIndex),
                               request.getFlightUsages(itinIndex)).run(paymentsToLimit);

  // YqYr
  if (itin.yqYrPathRefId().has_value())
  {
    assert(itin.yqYrPathRefId().value() < request.yqYrPaths().size());
    const YqYrPath& yqYrPath = request.yqYrPaths()[itin.yqYrPathRefId().value()];
    TaxLimiter::limitYqYrs(yqYrPath, paymentsToCalculate);
    TaxLimiter::overlapYqYrs(yqYrPath.yqYrUsages().size(), paymentsToCalculate);
  }
}

void
BusinessRulesProcessor::calculateTax(const type::Index& itinId,
                                     const Request& request,
                                     RawPayments& itinRawPayments,
                                     std::vector<PaymentWithRules>& paymentsToCalculate)
{
  std::vector<PaymentWithRules>::iterator paymentsToCalculateIter = paymentsToCalculate.begin();
  for (; paymentsToCalculateIter != paymentsToCalculate.end(); ++paymentsToCalculateIter)
  {
    if (!paymentsToCalculateIter->paymentDetail->isFailed())
    {
      paymentsToCalculateIter->calculatorsGroups->foreach<ApplyRuleFunctor>(
          itinId, request, _services, itinRawPayments, *paymentsToCalculateIter->paymentDetail);
      paymentsToCalculateIter->paymentDetail->setCalculated();

      paymentsToCalculateIter->paymentDetail->setCommandExempt(
          request.processing().isExempted(paymentsToCalculateIter->paymentDetail->sabreTaxCode()));
    }
  }
}

void
BusinessRulesProcessor::createDiagnosticResponse(AtpcoDiagnostic& diagnostic)
{
  _response._diagnosticResponse = DiagnosticResponse();
  diagnostic.createMessages(_response._diagnosticResponse->_messages);
}

bool
BusinessRulesProcessor::createDBDiagnosticResponse(const Request& request, Services& services)
{
  const DiagnosticCommand& diagnostic = request.diagnostic();
  if (diagnostic.number() != DBDiagnostic::NUMBER)
    return false;

  type::Timestamp ticketingDate(request.ticketingOptions().ticketingDate(),
                                request.ticketingOptions().ticketingTime());
  DBDiagnostic dbDiagnostic(services, diagnostic.parameters(), ticketingDate);
  createDiagnosticResponse(dbDiagnostic);
  return true;
}

bool
BusinessRulesProcessor::analyzeRequest(Request& request, Services& services)
{
  if (request.itins().size() != 0) // already analyzed
    return true;

  try
  {
    RequestAnalyzer(request, services).analyze();
    return true;
  }
  catch (RequestLogicError const& e)
  {
    std::cout << "RequestLogicError: " << e.what() << std::endl;
    response()._error = ErrorMessage();
    response()._error->_content = e.what();
    return false;
  }
}

bool
BusinessRulesProcessor::createRequestDiagnosticResponse(const DiagnosticCommand& diagnostic,
                                                        const Request& request)
{
  if (diagnostic.number() == RequestDiagnostic::NUMBER)
  {
    if (InputRequestDiagnostic::match(diagnostic) != InputRequestDiagnostic::Match::No)
      return false;

    RequestDiagnostic requestDiagnostic(request, diagnostic.parameters());
    createDiagnosticResponse(requestDiagnostic);
    return true;
  }
  else
  {
    return false;
  }
}

void
BusinessRulesProcessor::validateTax(const type::ProcessingGroup& processingGroup,
                                    const TaxValue& tax,
                                    const Geo& taxPoint,
                                    const Geo& nextPrevTaxPoint,
                                    const type::Index itinId,
                                    const GeoPathProperties& geoPathProperties,
                                    const Request& request,
                                    RawPayments& itinRawPayments,
                                    std::vector<PaymentWithRules>& paymentsToCalculate)
{
  if (UNLIKELY(!request.processing().isAllowed(tax->getTaxName())))
    return;

  type::CarrierCode marketingCarrier = getMarketingCarrier(request, taxPoint, itinId);

  TaxValidator taxValidator(*this,
                            _services,
                            tax,
                            taxPoint,
                            nextPrevTaxPoint,
                            itinId,
                            geoPathProperties,
                            request,
                            itinRawPayments,
                            paymentsToCalculate,
                            marketingCarrier,
                            processingGroup);

  for (const std::shared_ptr<BusinessRulesContainer>& rulesContainer :
       tax->getDateFilteredCopy(processingGroup, _ticketingDate))
  {
    if (taxValidator.validate(*rulesContainer))
      return;
  }
}

void
BusinessRulesProcessor::applyDepartureTax(const type::ProcessingGroup& processingGroup,
                                          const TaxValue& tax,
                                          const Itin& itin,
                                          const GeoPathProperties& geoPathProperties,
                                          const Request& request,
                                          RawPayments& itinRawPayments)
{
  std::vector<PaymentWithRules> paymentsToCalculate;

  const GeoPath& geoPath = request.geoPaths()[itin.geoPathRefId()];
  for (type::Index id = 0; id < geoPath.geos().size() - 1; id += 2)
  {
    const Geo& taxPoint = geoPath.geos()[id];

    if (tax->getTaxName().nation() != ZZ && tax->getTaxName().nation() != taxPoint.getNation())
      continue;

    const Geo& nextTaxPoint = geoPath.geos()[id + 1];

    validateTax(processingGroup,
                tax,
                taxPoint,
                nextTaxPoint,
                itin.id(),
                geoPathProperties,
                request,
                itinRawPayments,
                paymentsToCalculate);
  }

  if (paymentsToCalculate.empty())
    return;

  limitTax(
      itin.id(), request, itinRawPayments, paymentsToCalculate, tax->getTaxName().taxCode() == AY);
  calculateTax(itin.id(), request, itinRawPayments, paymentsToCalculate);
}

void
BusinessRulesProcessor::applyArrivalTax(const type::ProcessingGroup& processingGroup,
                                        const TaxValue& tax,
                                        const Itin& itin,
                                        const GeoPathProperties& geoPathProperties,
                                        const Request& request,
                                        RawPayments& itinRawPayments)
{
  std::vector<PaymentWithRules> paymentsToCalculate;

  const GeoPath& geoPath = request.geoPaths()[itin.geoPathRefId()];
  for (type::Index id = geoPath.geos().size(); id > 1; id -= 2)
  {
    const Geo& taxPoint = geoPath.geos()[id - 1];
    if (tax->getTaxName().nation() != ZZ && tax->getTaxName().nation() != taxPoint.getNation())
      continue;

    const Geo& prevTaxPoint = geoPath.geos()[id - 2];

    validateTax(processingGroup,
                tax,
                taxPoint,
                prevTaxPoint,
                itin.id(),
                geoPathProperties,
                request,
                itinRawPayments,
                paymentsToCalculate);
  }

  if (paymentsToCalculate.empty())
    return;

  limitTax(itin.id(), request, itinRawPayments, paymentsToCalculate);
  calculateTax(itin.id(), request, itinRawPayments, paymentsToCalculate);
}

void
BusinessRulesProcessor::applySaleTax(const type::ProcessingGroup& processingGroup,
                                     const TaxValue& tax,
                                     const Itin& itin,
                                     const GeoPathProperties& geoPathProperties,
                                     const Request& request,
                                     RawPayments& itinRawPayments)
{
  bool taxMatchesItinOrSalePoint = false;

  if (UNLIKELY(tax->getTaxName().nation() == ZZ))
    taxMatchesItinOrSalePoint = true;

  const Geo& pointOfSale = request.posTaxPoints()[itin.pointOfSaleRefId()];
  if (LIKELY(!taxMatchesItinOrSalePoint))
  {
    if (tax->getTaxName().nation() == pointOfSale.getNation())
      taxMatchesItinOrSalePoint = true;
  }

  const GeoPath& geoPath = request.geoPaths()[itin.geoPathRefId()];
  if (!taxMatchesItinOrSalePoint)
  {
    for (const Geo& taxPoint : geoPath.geos())
    {
      if (tax->getTaxName().nation() == taxPoint.getNation())
      {
        taxMatchesItinOrSalePoint = true;
        break;
      }
    }
  }

  if (!taxMatchesItinOrSalePoint)
    return;

  std::vector<PaymentWithRules> paymentsToCalculate;

  validateTax(processingGroup,
              tax,
              pointOfSale,
              geoPath.geos().back(),
              itin.id(),
              geoPathProperties,
              request,
              itinRawPayments,
              paymentsToCalculate);

  if (paymentsToCalculate.empty())
    return;
  // there is no point for limiting sale taxes
  calculateTax(itin.id(), request, itinRawPayments, paymentsToCalculate);
}

void
BusinessRulesProcessor::calculateRawPayments(const type::ProcessingGroup& processingGroup,
                                             const OrderedTaxes& orderedTaxes,
                                             Request& request,
                                             std::vector<RawPayments>& itinsRawPayments)
{
  GeoPathPropertiesCalculator calculator(request, _services.mileageService());
  for (type::Index i = 0; i < request.itins().size(); ++i)
  {
    Itin const* itin = request.itins()[i];
    if (processingGroup == type::ProcessingGroup::Itinerary &&
        itin->farePath()->fareUsages().empty())
      continue;
    RawPayments& itinRawPayments = itinsRawPayments[i];
    assert(itinRawPayments.empty());
    size_t estimatedCount =
        estimatePaymentDetailCount(orderedTaxes,
                                   request.geoPaths()[itin->geoPathRefId()],
                                   processingGroup,
                                   request.posTaxPoints()[itin->pointOfSaleRefId()],
                                   _ticketingDate);
    itinRawPayments.reserve(estimatedCount);
    GeoPathProperties properties;
    calculator.calculate(*itin, properties);

    for (const TaxValues& taxValues : orderedTaxes)
    {
      for (const TaxValue& tax : taxValues)
      {
        const TaxName& taxName = tax->getTaxName();
        if (taxName.taxPointTag() == type::TaxPointTag::Departure)
          applyDepartureTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
        else if (taxName.taxPointTag() == type::TaxPointTag::Arrival)
          applyArrivalTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
        else if (LIKELY(taxName.taxPointTag() == type::TaxPointTag::Sale))
          applySaleTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
      }
    }

    if (!_services.fallbackService().isSet(tse::fallback::fallbackAtpcoTaxTotalRounding))
    {
      finalRoundingForPercentageTaxes(itinRawPayments);
    }
  }
}

void
BusinessRulesProcessor::finalRoundingForPercentageTaxes(RawPayments& itinRawPayments)
{
  std::map<type::TaxCode, std::vector<PaymentDetail*>> taxToDetailsMap;

  for (auto& rawPayment : itinRawPayments)
  {
    if (rawPayment.taxName->percentFlatTag() == type::PercentFlatTag::Flat ||
        rawPayment.detail.isExempt() ||
        rawPayment.detail.taxEquivalentAmount() == 0)
      continue;

    taxToDetailsMap[rawPayment.taxName->taxCode()].push_back(&rawPayment.detail);
  }

  for (auto& elem : taxToDetailsMap)
  {
    if (elem.second.size() <= 1)
      continue;

    type::MoneyAmount totalUnrounded = 0;
    type::MoneyAmount totalRounded = 0;

    type::MoneyAmount totalWithMarkupUnrounded = 0;
    type::MoneyAmount totalWithMarkupRounded = 0;

    for (auto& detail : elem.second)
    {
      totalUnrounded += detail->calcDetails().taxBeforeRounding;
      totalRounded += detail->taxEquivalentAmount();

      if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
      {
        totalWithMarkupUnrounded += detail->calcDetails().taxWithMarkupBeforeRounding;
        totalWithMarkupRounded += detail->taxEquivalentWithMarkupAmount();
      }
    }

    _services.taxRoundingInfoService().doStandardRound(
        totalUnrounded,
        elem.second.front()->calcDetails().roundingUnit,
        elem.second.front()->calcDetails().roundingDir,
        elem.second.front()->calcDetails().currencyUnit);

    type::MoneyAmount diff = totalUnrounded - totalRounded;
    if (diff > 0)
      elem.second.front()->taxEquivalentAmount() += diff;
    else
      elem.second.back()->taxEquivalentAmount() += diff;

    if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
    {
      _services.taxRoundingInfoService().doStandardRound(
          totalWithMarkupUnrounded,
          elem.second.front()->calcDetails().roundingUnit,
          elem.second.front()->calcDetails().roundingDir,
          elem.second.front()->calcDetails().currencyUnit);


      diff = totalWithMarkupUnrounded - totalWithMarkupRounded;
      if (diff > 0)
        elem.second.front()->taxEquivalentWithMarkupAmount() += diff;
      else
        elem.second.back()->taxEquivalentWithMarkupAmount() += diff;
    }
  }
}

ItinsRawPayments
BusinessRulesProcessor::calculateRawPayments(Request& request, const OrderedTaxes& orderedTaxes)
{
  ItinsRawPayments itinsRawPayments(request.itins().size());
  for (type::ProcessingGroup processingGroup : request.processing().getProcessingGroups())
    calculateRawPayments(
        processingGroup, orderedTaxes, request, itinsRawPayments.get(processingGroup));

  return itinsRawPayments;
}

void
BusinessRulesProcessor::calculateRawPaymentsSingleItin(const type::ProcessingGroup& processingGroup,
                                                       const OrderedTaxes& orderedTaxes,
                                                       const Request& request,
                                                       std::vector<RawPayments>& itinsRawPayments,
                                                       type::Index itinIndex)
{
  GeoPathPropertiesCalculator calculator(request, _services.mileageService());

  Itin const* itin = request.itins()[itinIndex];
  if (processingGroup == type::ProcessingGroup::Itinerary &&
      itin->farePath()->fareUsages().empty())
    return;

  RawPayments& itinRawPayments = itinsRawPayments[itinIndex];
  assert(itinRawPayments.empty());
  size_t estimatedCount =
      estimatePaymentDetailCount(orderedTaxes,
                                 request.geoPaths()[itin->geoPathRefId()],
                                 processingGroup,
                                 request.posTaxPoints()[itin->pointOfSaleRefId()],
                                 _ticketingDate);
  itinRawPayments.reserve(estimatedCount);
  GeoPathProperties properties;
  calculator.calculate(*itin, properties);

  for (const TaxValues& taxValues : orderedTaxes)
  {
    for (const TaxValue& tax : taxValues)
    {
      const TaxName& taxName = tax->getTaxName();
      if (taxName.taxPointTag() == type::TaxPointTag::Departure)
        applyDepartureTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
      else if (taxName.taxPointTag() == type::TaxPointTag::Arrival)
        applyArrivalTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
      else if (LIKELY(taxName.taxPointTag() == type::TaxPointTag::Sale))
        applySaleTax(processingGroup, tax, *itin, properties, request, itinRawPayments);
    }
  }

  if (!_services.fallbackService().isSet(tse::fallback::fallbackAtpcoTaxTotalRounding))
  {
    finalRoundingForPercentageTaxes(itinRawPayments);
  }
}

void
BusinessRulesProcessor::calculateRawPaymentsSingleItin(const Request& request,
                                                       const OrderedTaxes& orderedTaxes,
                                                       type::Index itinIndex,
                                                       ItinsRawPayments& itinsRawPayments)
{
  for (type::ProcessingGroup processingGroup : request.processing().getProcessingGroups())
    calculateRawPaymentsSingleItin(
        processingGroup, orderedTaxes, request, itinsRawPayments.get(processingGroup), itinIndex);
}


ItinsPayments
BusinessRulesProcessor::filterPaymentsByDiagnostic(
    const std::vector<type::ProcessingGroup>& processingGroups,
    const ItinsRawPayments& itinsRawPayments,
    const std::vector<Itin*>& itins,
    const DiagnosticCommand& diagnostic) const
{
  ItinsPayments itinsPayments;

  for (const Itin* itin : itins)
  {
    itinsPayments._itinPayments.push_back(new ItinPayments(itin->id()));
    itinsPayments._itinPayments.back().requestedPassengerCode() = itin->passenger()->_code;
    itinsPayments._itinPayments.back().validatingCarrier() = itin->farePath()->validatingCarrier();
  }

  if (diagnostic.number() == PositiveDiagnostic::NUMBER ||
      diagnostic.number() == NegativeDiagnostic::NUMBER ||
      diagnostic.number() == BuildInfoDiagnostic::NUMBER ||
      diagnostic.number() == Diagnostic817::NUMBER)
  {
    for (const type::ProcessingGroup& processingGroup : processingGroups)
    {
      const std::vector<RawPayments>& rawPayments = itinsRawPayments.get(processingGroup);
      for (type::Index i = 0; i < itins.size(); ++i)
      {
        ItinPayments& itinPayments = itinsPayments._itinPayments[i];
        itinPayments.addAllTaxes(processingGroup, rawPayments[i], _services);
      }
    }
  }
  else // no diagnostic or diagnostics handled further in the process
  {
    for (const type::ProcessingGroup& processingGroup : processingGroups)
    {
      const std::vector<RawPayments>& rawPayments = itinsRawPayments.get(processingGroup);
      for (type::Index i = 0; i < itins.size(); ++i)
      {
        ItinPayments& itinPayments = itinsPayments._itinPayments[i];
        itinPayments.addValidTaxes(processingGroup, rawPayments[i], _services);
      }
    }
  }

  return itinsPayments;
}

void
BusinessRulesProcessor::filterPaymentsSingleItin(
    const std::vector<type::ProcessingGroup>& processingGroups,
    const ItinsRawPayments& itinsRawPayments,
    const std::vector<Itin*>& itins,
    type::Index itinIndex,
    ItinsPayments& itinsPayments) const
{
  const Itin* itin = itins[itinIndex];
  itinsPayments._itinPayments[itinIndex].setItinId(itin->id());
  itinsPayments._itinPayments[itinIndex].requestedPassengerCode() = itin->passenger()->_code;
  itinsPayments._itinPayments[itinIndex].validatingCarrier() = itin->farePath()->validatingCarrier();

  for (const type::ProcessingGroup& processingGroup : processingGroups)
  {
    const std::vector<RawPayments>& rawPayments = itinsRawPayments.get(processingGroup);

    ItinPayments& itinPayments = itinsPayments._itinPayments[itinIndex];
    itinPayments.addValidTaxes(processingGroup, rawPayments[itinIndex], _services);
  }
}


void
BusinessRulesProcessor::addBuildInfoDiagnosticToResponse(const DiagnosticCommand& diagnostic,
                                                         const std::string& buildInfo)
{
  if (diagnostic.number() == BuildInfoDiagnostic::NUMBER)
  {
    BuildInfoDiagnostic buildInfoDiagnostic(buildInfo, diagnostic.parameters());
    createDiagnosticResponse(buildInfoDiagnostic);
  }
}

bool
BusinessRulesProcessor::createRegularResponse(const DiagnosticCommand& diagnostic,
                                              ItinsPayments& itinsPayments)
{
  if (diagnostic.number() == 0 || (diagnostic.number() != PositiveDiagnostic::NUMBER &&
                                   diagnostic.number() != NegativeDiagnostic::NUMBER &&
                                   diagnostic.number() != Diagnostic817::NUMBER))
  {
    _response._itinsPayments.emplace(std::move(itinsPayments));
    return true;
  }
  else
  {
    return false;
  }
}

bool
BusinessRulesProcessor::createPositiveDiagnosticResponse(const DiagnosticCommand& diagnostic,
                                                         const ItinsPayments& itinsPayments)
{
  if (diagnostic.number() == PositiveDiagnostic::NUMBER)
  {
    PositiveDiagnostic positiveDiagnostic(itinsPayments, diagnostic.parameters());
    createDiagnosticResponse(positiveDiagnostic);
    return true;
  }
  else
  {
    return false;
  }
}

bool
BusinessRulesProcessor::createNegativeDiagnosticResponse(const DiagnosticCommand& diagnostic,
                                                         const ItinsPayments& itinsPayments)
{
  if (diagnostic.number() == NegativeDiagnostic::NUMBER)
  {
    NegativeDiagnostic negativeDiagnostic(itinsPayments, diagnostic.parameters(), _services);
    createDiagnosticResponse(negativeDiagnostic);
    return true;
  }
  else
  {
    return false;
  }
}

bool
BusinessRulesProcessor::create817DiagnosticResponse(
    const Request& request,
    const ItinsPayments& itinsPayments,
    const AtpcoTaxesActivationStatus& activationStatus)
{
  const DiagnosticCommand& diagnostic = request.diagnostic();
  if (diagnostic.number() == Diagnostic817::NUMBER)
  {
    Diagnostic817 diagnostic817(request, itinsPayments, activationStatus);
    createDiagnosticResponse(diagnostic817);
    return true;
  }
  else
  {
    return false;
  }
}

bool
BusinessRulesProcessor::createPostprocessResponse(
    const Request& request,
    ItinsPayments&& itinsPayments,
    const AtpcoTaxesActivationStatus& activationStatus)
{
  _taxesSelector.reset(new ExcItinTaxesSelector(itinsPayments, request));

  return createRegularResponse(request.diagnostic(), itinsPayments) ||
         createPositiveDiagnosticResponse(request.diagnostic(), itinsPayments) ||
         createNegativeDiagnosticResponse(request.diagnostic(), itinsPayments) ||
         create817DiagnosticResponse(request, itinsPayments, activationStatus);
}

void
BusinessRulesProcessor::parseFilterParameters(const DiagnosticCommand& diagnostic)
{
  for (const Parameter& parameter : diagnostic.parameters())
  {
    if (parameter.name() == "IV")
    {
      type::Vendor vendor(UninitializedCode);
      codeFromString(parameter.value(), vendor);
      _filter.vendor = vendor;
    }
    else if (parameter.name() == "IN")
    {
      type::Nation nation(UninitializedCode);
      codeFromString(parameter.value(), nation);
      _filter.nation = nation;
    }
    else if (parameter.name() == "IC")
    {
      type::TaxCode taxCode(UninitializedCode);
      codeFromString(parameter.value(), taxCode);
      _filter.taxCode = taxCode;
    }
    else if (parameter.name() == "IT")
    {
      type::TaxType taxType(UninitializedCode);
      codeFromString(parameter.value(), taxType);
      _filter.taxType = taxType;
    }
    else if (parameter.name() == "IS")
    {
      DiagnosticUtil::splitSeqValues(
          parameter.value(), _filter.seq, _filter.seqLimit, _filter.isSeqRange);
    }
  }
}

bool
BusinessRulesProcessor::matchesFilter(const TaxName& taxName,
                                      const BusinessRulesContainer& rulesContainer)
{
  bool vendorMatched = _filter.vendor.empty() || _filter.vendor == rulesContainer.vendor();
  bool nationMatched = _filter.nation.empty() || _filter.nation == taxName.nation();
  bool taxCodeMatched = _filter.taxCode.empty() || _filter.taxCode == taxName.taxCode();
  bool taxTypeMatched = _filter.taxType.empty() || _filter.taxType == taxName.taxType();

  bool isSeqMatched = DiagnosticUtil::isSeqNoMatching(
      rulesContainer.seqNo(), _filter.seq, _filter.seqLimit, _filter.isSeqRange);

  return vendorMatched && nationMatched && taxCodeMatched && taxTypeMatched && isSeqMatched;
}

std::set<PreviousTicketTaxInfo>
BusinessRulesProcessor::getComputedTaxesOnItin() const
{
  if (_taxesSelector)
    return _taxesSelector->get();
  else
    return std::set<PreviousTicketTaxInfo>{};
}
} // namespace tax
