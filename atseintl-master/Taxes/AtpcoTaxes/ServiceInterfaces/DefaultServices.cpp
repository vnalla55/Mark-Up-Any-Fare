#include "ServiceInterfaces/DefaultServices.h"

#include "ServiceInterfaces/ActivationInfoService.h"
#include "ServiceInterfaces/AKHIFactorService.h"
#include "ServiceInterfaces/CarrierApplicationService.h"
#include "ServiceInterfaces/CarrierFlightService.h"
#include "ServiceInterfaces/ConfigService.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/CustomerService.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/LoggerService.h"
#include "ServiceInterfaces/MileageService.h"
#include "ServiceInterfaces/NationService.h"
#include "ServiceInterfaces/PassengerMapper.h"
#include "ServiceInterfaces/PassengerTypesService.h"
#include "ServiceInterfaces/PreviousTicketService.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "ServiceInterfaces/RepricingService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "ServiceInterfaces/SectorDetailService.h"
#include "ServiceInterfaces/ServiceBaggageService.h"
#include "ServiceInterfaces/ServiceFeeSecurityService.h"
#include "ServiceInterfaces/TaxCodeTextService.h"
#include "ServiceInterfaces/TaxReissueService.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

namespace tax
{
DefaultServices::DefaultServices()
: _configService(std::make_unique<ConfigService>())
{
}

DefaultServices::~DefaultServices() = default;

void DefaultServices::setActivationInfoService(const ActivationInfoService *activationInfoCache)
{
  _activationInfoService.reset(activationInfoCache);
}

void DefaultServices::setAKHIFactorService(const AKHIFactorService* aKHIFactorService)
{
  _AKHIFactorService.reset(aKHIFactorService);
}

void DefaultServices::setCarrierApplicationService(const CarrierApplicationService* carrierApplicationService)
{
  _carrierApplicationService.reset(carrierApplicationService);
}

void DefaultServices::setCarrierFlightService(const CarrierFlightService* carrierFlightService)
{
  _carrierFlightService.reset(carrierFlightService);
}

void DefaultServices::setConfigService(const ConfigService* configService) { _configService.reset(configService); }

void DefaultServices::setCurrencyService(const CurrencyService* currencyService) { _currencyService.reset(currencyService); }

void DefaultServices::setCustomerService(const CustomerService* service) { _customerService.reset(service); }

void DefaultServices::setFallbackService(const FallbackService* fallbackService) { _fallbackService.reset(fallbackService); }

void DefaultServices::setLocService(const LocService* locService) { _locService.reset(locService); }

void DefaultServices::setLoggerService(const LoggerService* loggerService) {  _loggerService.reset(loggerService); }

void DefaultServices::setMileageService(const MileageService* mileageService) { _mileageService.reset(mileageService); }

void DefaultServices::setNationService(const NationService* nationService) { _nationService.reset(nationService); }

void DefaultServices::setPassengerMapper(const PassengerMapper* paxMapper) { _passengerMapper.reset(paxMapper); }

void DefaultServices::setPassengerTypesService(const PassengerTypesService* passengerTypesService)
{
  _passengerTypesService.reset(passengerTypesService);
}

void DefaultServices::setPreviousTicketService(const PreviousTicketService* service)
{
  _previousTicketService.reset(service);
}

void DefaultServices::setReportingRecordService(const ReportingRecordService* reportingRecordService)
{
  _reportingRecordService.reset(reportingRecordService);
}

void DefaultServices::setRepricingService(const RepricingService* repricingService)
{
  _repricingService.reset(repricingService);
}

void DefaultServices::setRulesRecordsService(std::unique_ptr<const RulesRecordsService> rulesRecordsService)
{
  _rulesRecordsService.emplace(std::move(rulesRecordsService));
}

void DefaultServices::setRulesRecordsService(const RulesRecordsService* rulesRecordsService)
{
  if (rulesRecordsService == nullptr) // defensive?
    _rulesRecordsService = boost::none;
  else
    _rulesRecordsService.emplace(*rulesRecordsService);
}

void DefaultServices::setSectorDetailService(const SectorDetailService* sectorDetailService)
{
  _sectorDetailService.reset(sectorDetailService);
}

void DefaultServices::setServiceBaggageService(const ServiceBaggageService* serviceBaggageService)
{
  _serviceBaggageService.reset(serviceBaggageService);
}

void DefaultServices::setServiceFeeSecurityService(const ServiceFeeSecurityService* sfsCache)
{
  _serviceFeeSecurityService.reset(sfsCache);
}

void DefaultServices::setTaxCodeTextService(const TaxCodeTextService* taxCodeTextService)
{
  _taxCodeTextService.reset(taxCodeTextService);
}

void DefaultServices::setTaxReissueService(const TaxReissueService* taxReissueService)
{
  _taxReissueService.reset(taxReissueService);
}

void DefaultServices::setTaxRoundingInfoService(const TaxRoundingInfoService* roundingCache)
{
  _taxRoundingInfoService.reset(roundingCache);
}

}
