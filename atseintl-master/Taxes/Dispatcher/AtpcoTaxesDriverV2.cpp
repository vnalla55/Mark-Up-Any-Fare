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

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "Taxes/AtpcoTaxes/Common/TaxDetailsLevel.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "Taxes/AtpcoTaxes/Diagnostic/InputRequestDiagnostic.h"
#include "Taxes/AtpcoTaxes/Diagnostic/LoggingLocService.h"
#include "Taxes/AtpcoTaxes/Diagnostic/LoggingRulesRecordService.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Response.h"
#include "Taxes/AtpcoTaxes/Factories/OutputConverter.h"
#include "Taxes/AtpcoTaxes/Factories/RequestFactory.h"
#include "Taxes/AtpcoTaxes/Processor/BusinessRulesProcessor.h"
#include "Taxes/Dispatcher/AtpcoTaxesDriverV2.h"
#include "Taxes/LegacyFacades/ActivationInfoServiceV2.h"
#include "Taxes/LegacyFacades/AKHIFactorServiceV2.h"
#include "Taxes/LegacyFacades/CarrierApplicationServiceV2.h"
#include "Taxes/LegacyFacades/CarrierFlightServiceV2.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/CopyTaxResponse.h"
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"
#include "Taxes/LegacyFacades/CustomerServiceV2.h"
#include "Taxes/LegacyFacades/FallbackServiceV2.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyFacades/LocServiceV2.h"
#include "Taxes/LegacyFacades/LoggerServiceV2.h"
#include "Taxes/LegacyFacades/MileageServiceV2.h"
#include "Taxes/LegacyFacades/NationServiceV2.h"
#include "Taxes/LegacyFacades/PassengerMapperV2.h"
#include "Taxes/LegacyFacades/PassengerTypesServiceV2.h"
#include "Taxes/LegacyFacades/PreviousTicketServiceV2.h"
#include "Taxes/LegacyFacades/ReportingRecordServiceV2.h"
#include "Taxes/LegacyFacades/RepricingServiceV2.h"
#include "Taxes/LegacyFacades/ResponseConverter.h"
#include "Taxes/LegacyFacades/ResponseConverter2.h"
#include "Taxes/LegacyFacades/RulesRecordsServiceV2.h"
#include "Taxes/LegacyFacades/SectorDetailServiceV2.h"
#include "Taxes/LegacyFacades/ServiceBaggageServiceV2.h"
#include "Taxes/LegacyFacades/ServiceFeeSecurityServiceV2.h"
#include "Taxes/LegacyFacades/TaxDependencies.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "Taxes/LegacyFacades/TaxRoundingInfoServiceV2.h"

#include <sstream> // for cache

namespace tse
{

FALLBACK_DECL(AF_CAT33_ResponseConverter2)
FALLBACK_DECL(AF_CAT33_TaxRequestBuilder)

namespace
{
bool
checkInputLevelDiagnostic(tax::InputRequest& inputRequest,
                          std::string& xmlRequest, bool& doServiceLogging)
{
  switch (tax::InputRequestDiagnostic::match(inputRequest))
  {
  case tax::InputRequestDiagnostic::Match::RequestOnly:
    xmlRequest = tax::InputRequestDiagnostic::makeContent(inputRequest);
    return true;
  case tax::InputRequestDiagnostic::Match::RequestWithCache:
    doServiceLogging = true;
    BOOST_FALLTHROUGH;
  case tax::InputRequestDiagnostic::Match::No:
    return false;
  default:
    assert (false);
    BOOST_UNREACHABLE_RETURN (false);
  }
}

Logger
requestlogger("atseintl.AtpcoTaxes.Request");

} // namespace

class AtpcoTaxesDriverV2::Impl
{
public:

static void makeTaxRqResponse(AtpcoTaxesDriverV2& _this)
{
  assert (_this._inputRequest);
  std::ostringstream cache;
  for (tax::LoggingService* logger : _this._serviceLoggers)
  {
    assert (logger);
    cache << logger->getLog() << "\n";
  }
  std::string stringCache = cache.str();
  std::string diagResponse = tax::InputRequestDiagnostic::makeContent(*_this._inputRequest, stringCache);
  _this._trx.taxRequestToBeReturnedAsResponse() += diagResponse;
}

};

AtpcoTaxesDriverV2::AtpcoTaxesDriverV2(PricingTrx& pricingTrx,
                                       const tax::ServicesFeesMap& servicesFees)
  : _trx(pricingTrx), _servicesFees(servicesFees), _gsaDiagnostic(nullptr), _request(nullptr), _xmlRequest("")
{
}

AtpcoTaxesDriverV2::~AtpcoTaxesDriverV2()
{
}

bool
AtpcoTaxesDriverV2::buildRequest()
{
  _v2TrxMappingDetails = tax::V2TrxMappingDetails();

  tax::InputRequest* inputRequest = nullptr;

  if (!fallback::AF_CAT33_TaxRequestBuilder(&_trx))
  {
    inputRequest = tax::TaxRequestBuilder(_trx,
        _trx.atpcoTaxesActivationStatus()).buildInputRequest(_servicesFees,
        _v2TrxMappingDetails, _gsaDiagnostic);
  }
  else
  {
    inputRequest = tax::TaxRequestBuilder_DEPRECATED().buildInputRequest(_trx, _servicesFees, _v2TrxMappingDetails, _gsaDiagnostic);
  }

  if (inputRequest == nullptr)
    return false;

  _inputRequest = inputRequest;
  if (checkInputLevelDiagnostic(*inputRequest, _xmlRequest, _doServiceLogging))
    return true;

  tax::RequestFactory factory;
  _request.reset(new tax::Request());
  factory.createFromInput(*inputRequest, *_request);
  return true;
}

void
AtpcoTaxesDriverV2::setServices()
{
  _services.setCarrierApplicationService(
      new tse::CarrierApplicationServiceV2(_trx.ticketingDate()));
  _services.setCarrierFlightService(new tse::CarrierFlightServiceV2(_trx.ticketingDate()));
  _services.setCurrencyService(new tse::CurrencyServiceV2(_trx, _trx.getRequest()->ticketingDT()));

  _services.setLocService(install<tax::LoggingLocService>(new tse::LocServiceV2{_trx.ticketingDate()}).release());

  _services.setMileageService(new tse::MileageServiceV2(_trx));
  _services.setNationService(new tse::NationServiceV2());
  _services.setAKHIFactorService(new tse::AKHIFactorServiceV2(_trx.ticketingDate()));
  _services.setFallbackService(new tse::FallbackServiceV2(_trx));
  _services.setLoggerService(new tse::LoggerServiceV2());
  _services.setSectorDetailService(new tse::SectorDetailServiceV2(_trx.ticketingDate()));
  _services.setServiceBaggageService(new tse::ServiceBaggageServiceV2(_trx.ticketingDate()));
  _services.setPassengerTypesService(new tse::PassengerTypesServiceV2(_trx.ticketingDate()));
  _services.setRepricingService(new tse::RepricingServiceV2(_trx, &_v2TrxMappingDetails));
  _services.setReportingRecordService(
      new tse::ReportingRecordServiceV2(_trx.getRequest()->ticketingDT()));
  _services.setPassengerMapper(new PassengerMapperV2(_trx));
  _services.setServiceFeeSecurityService(new ServiceFeeSecurityServiceV2(_trx.ticketingDate()));
  _services.setTaxRoundingInfoService(new TaxRoundingInfoServiceV2(_trx));
  _services.setActivationInfoService(new ActivationInfoServiceV2(_trx));

  if (_doServiceLogging)
  {
    std::unique_ptr<tax::LoggingRulesRecordService> loggingSvc {
      new tax::LoggingRulesRecordService{tse::RulesRecordsServiceV2::instance()}};
    _serviceLoggers.push_back(loggingSvc.get());
    std::unique_ptr<tax::RulesRecordsService> handle {loggingSvc.release()};
    _services.setRulesRecordsService(std::move(handle));
  }
  else
  {
    _services.setRulesRecordsService(&tse::RulesRecordsServiceV2::instance());
  }

  _services.setCustomerService(new CustomerServiceV2(_trx.dataHandle()));
  _services.setPreviousTicketService(new PreviousTicketServiceV2(_trx, new UtcConfig(_trx)));
}

void
AtpcoTaxesDriverV2::processShortPath(const std::string& diagResponse)
{
  _processor.reset(new tax::BusinessRulesProcessor(_services));
  // tax::Response& atpcoResponse = _processor->response();

  // atpcoResponse._diagnosticResponse = tax::DiagnosticResponse();
  _trx.taxRequestToBeReturnedAsResponse() += diagResponse;
  // Normally, when this is called once += is the same as =
  // However, in case the Tax trx is called multiple times,
  // we want to record all the transactions
}

void
AtpcoTaxesDriverV2::processFullRequest(tax::Request& request)
{
  _processor.reset(new tax::BusinessRulesProcessor(_services));
  _processor->run(request, _trx.atpcoTaxesActivationStatus());

  ItinSelector itinSelector(_trx);
  if (itinSelector.isExchangeTrx() && itinSelector.isExcItin())
  {
    std::set<tse::PreviousTicketTaxInfo> taxesForPreviousTicket;
    for(const tax::PreviousTicketTaxInfo& each : _processor->getComputedTaxesOnItin())
    {
      taxesForPreviousTicket.emplace(each.getSabreTaxCode(),
          each.getPercentage(), each.isCanadaExch());
    }

    _trx.addParentTaxes(taxesForPreviousTicket);
  }

  handleOcWithTaxInclInd(request);
}

bool
AtpcoTaxesDriverV2::processTaxes()
{
  if (_request)
  {
    processFullRequest(*_request);
    return true;
  }

  if (!_xmlRequest.empty())
  {
    processShortPath(_xmlRequest);
    return true;
  }

  return false;
}

bool
AtpcoTaxesDriverV2::convertResponse()
{
  if (!_processor)
    return false;

  if (_doServiceLogging)
  {
    Impl::makeTaxRqResponse(*this);
    return true;
  }

  tax::Response& atpcoResponse = _processor->response();

  if (atpcoResponse.isDiagnosticResponse())
  {
    if (!fallback::AF_CAT33_ResponseConverter2(&_trx))
    {
      tax::ResponseConverter(_trx, _trx.atpcoTaxesActivationStatus()).convertDiagnosticResponseToTaxResponse(
          *atpcoResponse._diagnosticResponse);
    }
    else
    {
      tax::ResponseConverter_DEPRECATED::convertDiagnosticResponseToTaxResponse(
          _trx, *atpcoResponse._diagnosticResponse);
    }

    return true;
  }

  if (!atpcoResponse._itinsPayments || atpcoResponse._itinsPayments->_itinPayments.empty())
    return true; // defensive or optimization?

  const tax::OutputResponse& outputResponse =
      tax::OutputConverter::convertToTaxRs(atpcoResponse, tax::TaxDetailsLevel::all());

  tse::TaxDependencies taxDeps(_services.rulesRecordsService(), _services.serviceBaggageService(),
                               toTimestamp(_trx.ticketingDate()));

  if (!fallback::AF_CAT33_ResponseConverter2(&_trx))
  {
    tax::ResponseConverter(_trx, _trx.atpcoTaxesActivationStatus()).updateV2Trx(
        outputResponse, _v2TrxMappingDetails, _gsaDiagnostic,
        [&](const tax::OutputTaxDetails& dt)
        { return taxDeps.taxDependencies(dt);});
  }
  else
  {
    tax::ResponseConverter_DEPRECATED::updateV2Trx(_trx, outputResponse, _v2TrxMappingDetails, _gsaDiagnostic,
                                        [&](const tax::OutputTaxDetails& dt) {return taxDeps.taxDependencies(dt);});
  }
  return true;
}

MoneyAmount
AtpcoTaxesDriverV2::getFeeAmount(tax::OptionalService& oc, const CurrencyCode& feeCurrency) const
{
  CurrencyCode sellingCurrency = ServiceFeeUtil::getSellingCurrency(_trx);

  if (feeCurrency != sellingCurrency)
  {
    tse::ServiceFeeUtil serviceFeeUtil(_trx);
    return serviceFeeUtil.convertMoney(amountToDouble(oc.amount()), sellingCurrency, feeCurrency)
        .value();
  }
  else
  {
    return amountToDouble(oc.amount());
  }
}

void
AtpcoTaxesDriverV2::handleOcWithTaxInclInd(tax::Request& request)
{
  for (tax::OptionalService& oc: request.optionalServices())
  {
    if (oc.checkBackingOutTaxes() && _v2TrxMappingDetails.isInTaxInclIndMap(oc.index()))
    {
      OCFees* ocFees = _v2TrxMappingDetails._ocTaxInclIndMap[oc.index()].first;
      size_t& segIndex = _v2TrxMappingDetails._ocTaxInclIndMap[oc.index()].second;

      OCFees::Memento memento = ocFees->saveToMemento();
      ocFees->setSeg(segIndex);

      const MoneyAmount& feeAmountInFeeCurrency = getFeeAmount(oc, ocFees->feeCurrency());
      const MoneyAmount& feeAmountInSellingCurrency = amountToDouble(oc.amount());
      const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes =
          amountToDouble(oc.feeAmountInSellingCurrencyPlusTaxes());

      ocFees->setBackingOutTaxes(
          feeAmountInFeeCurrency, feeAmountInSellingCurrency, feeAmountInSellingCurrencyPlusTaxes);

      ocFees->restoreFromMemento(memento);
    }
  }
}

template <typename LogSrvc, typename ISrvc>
std::unique_ptr<typename LogSrvc::BaseService>
AtpcoTaxesDriverV2::install(ISrvc* base)
{
  assert (base);
  std::unique_ptr<typename LogSrvc::BaseService> baseHandle {base};

  if (_doServiceLogging)
  {
    std::unique_ptr<LogSrvc> logingSvcHandle {new LogSrvc{std::move(baseHandle)}};
    _serviceLoggers.push_back(logingSvcHandle.get());
    assert (baseHandle == nullptr);
    baseHandle.reset(logingSvcHandle.release());
  }

  return baseHandle;
}

} // namespace tse
