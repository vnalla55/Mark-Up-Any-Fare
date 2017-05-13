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

#pragma once

#include "Common/Handle.h"
#include "ServiceInterfaces/Services.h"

#include <boost/optional.hpp>

#include <memory>

namespace tax
{

class DefaultServices : public Services
{
public:
  DefaultServices();
  ~DefaultServices() override;

  const ActivationInfoService& activationInfoService() const final { return *_activationInfoService; }
  void setActivationInfoService(const ActivationInfoService *activationInfoCache);

  const AKHIFactorService& aKHIFactorService() const final { return *_AKHIFactorService; }
  void setAKHIFactorService(const AKHIFactorService* aKHIFactorService);

  const CarrierApplicationService& carrierApplicationService() const final { return *_carrierApplicationService; }
  void setCarrierApplicationService(const CarrierApplicationService* carrierApplicationService);

  const CarrierFlightService& carrierFlightService() const final { return *_carrierFlightService; }
  void setCarrierFlightService(const CarrierFlightService* carrierFlightService);

  const ConfigService& configService() const final { return *_configService; }
  void setConfigService(const ConfigService* configService);

  const CurrencyService& currencyService() const final { return *_currencyService; }
  void setCurrencyService(const CurrencyService* currencyService);

  const CustomerService& customerService() const final { return *_customerService; }
  void setCustomerService(const CustomerService* service);

  const FallbackService& fallbackService() const final { return *_fallbackService; }
  void setFallbackService(const FallbackService* fallbackService);

  const LocService& locService() const final { return *_locService; }
  void setLocService(const LocService* locService);

  const LoggerService& loggerService() const final { return *_loggerService; }
  void setLoggerService(const LoggerService* loggerService);

  const MileageService& mileageService() const final { return *_mileageService; }
  void setMileageService(const MileageService* mileageService);

  const NationService& nationService() const final { return *_nationService; }
  void setNationService(const NationService* nationService);

  const PassengerMapper& passengerMapper() const final { return *_passengerMapper; }
  void setPassengerMapper(const PassengerMapper* paxMapper);

  const PassengerTypesService& passengerTypesService() const final { return *_passengerTypesService; }
  void setPassengerTypesService(const PassengerTypesService* passengerTypesService);

  const PreviousTicketService& previousTicketService() const final { return *_previousTicketService; }
  void setPreviousTicketService(const PreviousTicketService* service);

  const ReportingRecordService& reportingRecordService() const final {  return *_reportingRecordService; }
  void setReportingRecordService(const ReportingRecordService* reportingRecordService);

  const RepricingService& repricingService() const final {  return *_repricingService; }
  void setRepricingService(const RepricingService* repricingService);

  const RulesRecordsService& rulesRecordsService() const final { return _rulesRecordsService->get(); }
  void setRulesRecordsService(const RulesRecordsService* rulesRecordsService);
  void setRulesRecordsService(std::unique_ptr<const RulesRecordsService> rulesRecordsService);

  const SectorDetailService& sectorDetailService() const final { return *_sectorDetailService; }
  void setSectorDetailService(const SectorDetailService* sectorDetailService);

  const ServiceBaggageService& serviceBaggageService() const final { return *_serviceBaggageService; }
  void setServiceBaggageService(const ServiceBaggageService* serviceBaggageService);

  const ServiceFeeSecurityService& serviceFeeSecurityService() const final { return *_serviceFeeSecurityService; }
  void setServiceFeeSecurityService(const ServiceFeeSecurityService* sfsCache);

  const TaxCodeTextService& taxCodeTextService() const final { return *_taxCodeTextService; }
  void setTaxCodeTextService(const TaxCodeTextService* taxCodeTextService);

  const TaxReissueService& taxReissueService() const final { return *_taxReissueService; }
  void setTaxReissueService(const TaxReissueService* taxReissueService);

  const TaxRoundingInfoService& taxRoundingInfoService() const final { return *_taxRoundingInfoService; }
  void setTaxRoundingInfoService(const TaxRoundingInfoService* roundingCache);

private:
  std::unique_ptr<const ActivationInfoService> _activationInfoService;
  std::unique_ptr<const AKHIFactorService> _AKHIFactorService;
  std::unique_ptr<const CarrierApplicationService> _carrierApplicationService;
  std::unique_ptr<const CarrierFlightService> _carrierFlightService;
  std::unique_ptr<const ConfigService> _configService;
  std::unique_ptr<const CurrencyService> _currencyService;
  std::unique_ptr<const CustomerService> _customerService;
  std::unique_ptr<const FallbackService> _fallbackService;
  std::unique_ptr<const LocService> _locService;
  std::unique_ptr<const LoggerService> _loggerService;
  std::unique_ptr<const MileageService> _mileageService;
  std::unique_ptr<const NationService> _nationService;
  std::unique_ptr<const PassengerMapper> _passengerMapper;
  std::unique_ptr<const PassengerTypesService> _passengerTypesService;
  std::unique_ptr<const PreviousTicketService> _previousTicketService;
  std::unique_ptr<const ReportingRecordService> _reportingRecordService;
  std::unique_ptr<const RepricingService> _repricingService;
  boost::optional<Handle<const RulesRecordsService>> _rulesRecordsService;
  std::unique_ptr<const SectorDetailService> _sectorDetailService;
  std::unique_ptr<const ServiceBaggageService> _serviceBaggageService;
  std::unique_ptr<const ServiceFeeSecurityService> _serviceFeeSecurityService;
  std::unique_ptr<const TaxCodeTextService> _taxCodeTextService;
  std::unique_ptr<const TaxReissueService> _taxReissueService;
  std::unique_ptr<const TaxRoundingInfoService> _taxRoundingInfoService;
};
}
