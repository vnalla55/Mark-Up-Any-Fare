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
#include "AtpcoTaxes/DomainDataObjects/XmlCache.h"
#include "AtpcoTaxes/Factories/OutputConverter.h"
#include "AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "TestServer/Facades/AKHIFactorServiceServer.h"
#include "TestServer/Facades/CarrierApplicationServiceServer.h"
#include "TestServer/Facades/CarrierFlightServiceServer.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "TestServer/Facades/CustomerServiceServer.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/LoggerServiceServer.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "TestServer/Facades/MileageServiceServer.h"
#include "TestServer/Facades/PassengerMapperServer.h"
#include "TestServer/Facades/PassengerTypesServiceServer.h"
#include "TestServer/Facades/RepricingServiceServer.h"
#include "TestServer/Facades/ReportingRecordServiceServer.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"
#include "TestServer/Facades/SectorDetailServiceServer.h"
#include "TestServer/Facades/ServiceBaggageServiceServer.h"
#include "TestServer/Facades/ServiceFeeSecurityServiceServer.h"
#include "TestServer/Facades/TaxRoundingInfoServiceServer.h"
#include "TestServer/Xform/TestCacheBuilder.h"
#include "TestServer/Facades/ActivationInfoServiceServer.h"


namespace tax
{

void
TestCacheBuilder::buildCache(DefaultServices& services, const XmlCache& xmlCache)
{
  // TODO: the memory management of rulesRecordsServiceServer can definately be implmented
  //       more elegantly.
  RulesRecordsServiceServer * rulesRecordsServiceServer = new RulesRecordsServiceServer();
  rulesRecordsServiceServer->rulesRecords() = xmlCache.rulesRecords();
  services.setRulesRecordsService(rulesRecordsServiceServer);
  rulesRecordsServiceServer->updateKeys();

  LocServiceServer* locServiceServer = new LocServiceServer();
  locServiceServer->nations() = xmlCache.nations();
  locServiceServer->isInLocs() = xmlCache.isInLocs();
  services.setLocService(locServiceServer);

  MileageServiceServer* mileageServiceServer = new MileageServiceServer();
  mileageServiceServer->mileages() = xmlCache.mileages();
  services.setMileageService(mileageServiceServer);

  CarrierFlightServiceServer* carrierFlightServiceServer = new CarrierFlightServiceServer();
  carrierFlightServiceServer->carrierFlights() = xmlCache.carrierFlights();
  services.setCarrierFlightService(carrierFlightServiceServer);

  CarrierApplicationServiceServer* carrierApplicationServiceServer =
    new CarrierApplicationServiceServer();
  carrierApplicationServiceServer->carrierApplications() = xmlCache.carrierApplications();
  services.setCarrierApplicationService(carrierApplicationServiceServer);

  CurrencyServiceServer* currencyServiceServer = new CurrencyServiceServer();
  currencyServiceServer->currencyConversions() = xmlCache.currencyConversions();
  currencyServiceServer->currencies() = xmlCache.currencies();
  services.setCurrencyService(currencyServiceServer);

  CustomerServiceServer* customerServiceServer =
      new CustomerServiceServer(xmlCache.customers());
  services.setCustomerService(customerServiceServer);

  AKHIFactorServiceServer* aKHIFactorServiceServer = new AKHIFactorServiceServer();
  aKHIFactorServiceServer->aKHIFactor() = xmlCache.aKHIFactor();
  services.setAKHIFactorService(aKHIFactorServiceServer);

  services.setFallbackService(new FallbackServiceServer());
  services.setLoggerService(new LoggerServiceServer());

  SectorDetailServiceServer* sectorDetailServiceServer = new SectorDetailServiceServer();
  sectorDetailServiceServer->sectorDetail() = xmlCache.sectorDetail();
  services.setSectorDetailService(sectorDetailServiceServer);

  ServiceBaggageServiceServer* serviceBaggageServiceServer = new ServiceBaggageServiceServer();
  serviceBaggageServiceServer->serviceBaggage() = xmlCache.serviceBaggage();
  services.setServiceBaggageService(serviceBaggageServiceServer);

  PassengerTypesServiceServer* passengerTypesServiceServer =
    new PassengerTypesServiceServer(xmlCache.passengerTypeCodes());
  services.setPassengerTypesService(passengerTypesServiceServer);

  RepricingServiceServer* repricingServiceServer = new RepricingServiceServer();
  repricingServiceServer->repricingEntries() = xmlCache.repricingEntries();
  services.setRepricingService(repricingServiceServer);

  ReportingRecordServiceServer* reportingRecordServiceServer = new ReportingRecordServiceServer();
  for (const ReportingRecord& record : xmlCache.reportingRecords())
  {
    reportingRecordServiceServer->reportingRecords().push_back(
        std::make_shared<ReportingRecord>(record));
  }
  services.setReportingRecordService(reportingRecordServiceServer);

  PassengerMapper* passengerMapper = new PassengerMapperServer(xmlCache.paxTypeMapping());
  services.setPassengerMapper(passengerMapper);

  ServiceFeeSecurityServiceServer* serviceFeeSecurityServiceServer =
    new ServiceFeeSecurityServiceServer(xmlCache.serviceFeeSecurity());
  services.setServiceFeeSecurityService(serviceFeeSecurityServiceServer);

  TaxRoundingInfoServiceServer* roundingCache = new TaxRoundingInfoServiceServer();
  roundingCache->initialize(xmlCache.taxRoundings());
  services.setTaxRoundingInfoService(roundingCache);

  ActivationInfoServiceServer* activationInfoCache = new ActivationInfoServiceServer();
  services.setActivationInfoService(activationInfoCache);
}

} // namespace tax
