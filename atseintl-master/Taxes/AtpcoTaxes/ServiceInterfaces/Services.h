// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

namespace tax
{
class ActivationInfoService;
class AKHIFactorService;
class CarrierApplicationService;
class CarrierFlightService;
class ConfigService;
class CurrencyService;
class CustomerService;
class FallbackService;
class LocService;
class LoggerService;
class MileageService;
class NationService;
class PassengerMapper;
class PassengerTypesService;
class PreviousTicketService;
class ReportingRecordService;
class RepricingService;
class RulesRecordsService;
class SectorDetailService;
class ServiceBaggageService;
class ServiceFeeSecurityService;
class TaxCodeTextService;
class TaxReissueService;
class TaxRoundingInfoService;

class Services
{
public:
  virtual ~Services() = default;

  virtual const ActivationInfoService& activationInfoService() const = 0;
  virtual const AKHIFactorService& aKHIFactorService() const = 0;
  virtual const CarrierApplicationService& carrierApplicationService() const = 0;
  virtual const CarrierFlightService& carrierFlightService() const = 0;
  virtual const ConfigService& configService() const = 0;
  virtual const CurrencyService& currencyService() const = 0;
  virtual const CustomerService& customerService() const = 0;
  virtual const FallbackService& fallbackService() const = 0;
  virtual const LocService& locService() const = 0;
  virtual const LoggerService& loggerService() const = 0;
  virtual const MileageService& mileageService() const = 0;
  virtual const NationService& nationService() const = 0;
  virtual const PassengerMapper& passengerMapper() const = 0;
  virtual const PassengerTypesService& passengerTypesService() const = 0;
  virtual const PreviousTicketService& previousTicketService() const = 0;
  virtual const ReportingRecordService& reportingRecordService() const = 0;
  virtual const RepricingService& repricingService() const = 0;
  virtual const RulesRecordsService& rulesRecordsService() const = 0;
  virtual const SectorDetailService& sectorDetailService() const = 0;
  virtual const ServiceBaggageService& serviceBaggageService() const = 0;
  virtual const ServiceFeeSecurityService& serviceFeeSecurityService() const = 0;
  virtual const TaxCodeTextService& taxCodeTextService() const = 0;
  virtual const TaxReissueService& taxReissueService() const = 0;
  virtual const TaxRoundingInfoService& taxRoundingInfoService() const = 0;
};
}
