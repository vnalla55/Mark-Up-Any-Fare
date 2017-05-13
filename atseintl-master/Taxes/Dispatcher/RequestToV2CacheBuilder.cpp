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
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/Global.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/XmlCache.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/ConfigService.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "Taxes/Dispatcher/RequestToV2CacheBuilder.h"
#include "Taxes/LegacyFacades/ActivationInfoServiceV2.h"
#include "Taxes/LegacyFacades/AKHIFactorServiceV2.h"
#include "Taxes/LegacyFacades/CarrierApplicationServiceV2.h"
#include "Taxes/LegacyFacades/CarrierFlightServiceV2.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"
#include "Taxes/LegacyFacades/CustomerServiceV2.h"
#include "Taxes/LegacyFacades/FallbackServiceV2.h"
#include "Taxes/LegacyFacades/LocServiceV2.h"
#include "Taxes/LegacyFacades/LoggerServiceV2.h"
#include "Taxes/LegacyFacades/MileageServiceV2.h"
#include "Taxes/LegacyFacades/NationServiceV2.h"
#include "Taxes/LegacyFacades/PassengerMapperV2.h"
#include "Taxes/LegacyFacades/PassengerTypesServiceV2.h"
#include "Taxes/LegacyFacades/PreviousTicketServiceV2.h"
#include "Taxes/LegacyFacades/ReportingRecordServiceV2.h"
#include "Taxes/LegacyFacades/RepricingServiceV2.h"
#include "Taxes/LegacyFacades/RulesRecordsServiceV2.h"
#include "Taxes/LegacyFacades/SectorDetailServiceV2.h"
#include "Taxes/LegacyFacades/ServiceBaggageServiceV2.h"
#include "Taxes/LegacyFacades/ServiceFeeSecurityServiceV2.h"
#include "Taxes/LegacyFacades/TaxRoundingInfoServiceV2.h"

#include <algorithm>
#include <vector>

namespace tax
{
namespace
{
tse::ConfigurableValue<tse::ConfigSet<std::string>>
tvlDepTaxNations("TAX_SVC", "TVL_DEP_TAX_NATIONS");
tse::ConfigurableValue<tse::ConfigSet<std::string>>
fltnoDepTaxNations("TAX_SVC", "FLTNO_DEP_TAX_NATIONS");
tse::ConfigurableValue<tse::ConfigSet<std::string>>
sameDayDepTaxNations("TAX_SVC", "SAME_DAY_DEP_TAX_NATIONS");
}

std::vector<type::Nation>
initNationsFromVariable(const char* listName)
{
  tse::ConfigMan& config = tse::Global::config();
  std::vector<type::Nation> nations;
  std::string allNationsString;

  config.getValue(listName, allNationsString, "TAX_SVC");

  std::stringstream stream(allNationsString);
  std::string nation;
  type::Nation nationCode;
  while (std::getline(stream, nation, '|'))
  {
    nationCode.fromString(nation.c_str(), nation.length());
    nations.insert(
        std::upper_bound(nations.begin(), nations.end(), nationCode, std::less<type::Nation>()),
        nationCode);
  }

  nations.erase(std::unique(nations.begin(), nations.end()), nations.end());
  return nations;
}

std::vector<type::Nation>
initNationsFromCv(tse::ConfigurableValue<tse::ConfigSet<std::string>>& cv)
{
  std::vector<type::Nation> nations;
  type::Nation nationCode;

  for (const auto nation : cv.getValue())
  {
    nationCode.fromString(nation.c_str(), nation.length());
    nations.push_back(nationCode);
  }

  return nations;
}

void
RequestToV2CacheBuilder::buildCache(DefaultServices& services,
                                    const XmlCache& /*xmlCache*/,
                                    const Request& request,
                                    tse::DataHandle& dataHandle)
{
  tse::PricingTrx* trx = dataHandle.create<tse::PricingTrx>();

  trx->setRequest(trx->dataHandle().create<tse::PricingRequest>());
  trx->setOptions(trx->dataHandle().create<tse::PricingOptions>());

  tax::type::Date ticketingDate = request.ticketingOptions().ticketingDate();
  trx->getRequest()->ticketingDT() =
      tse::DateTime(ticketingDate.year(), ticketingDate.month(), ticketingDate.day());

  trx->getRequest()->ticketingAgent() = trx->dataHandle().create<tse::Agent>();
  trx->getRequest()->ticketingAgent()->agentLocation() = trx->dataHandle().create<tse::Loc>();

  if (!request.pointsOfSale().empty())
  {
    const tse::Loc* agentLocation = trx->dataHandle().getLoc(
        tse::toTseAirportCode(request.pointsOfSale().front().loc()), time(nullptr));
    trx->getRequest()->ticketingAgent()->agentLocation() = agentLocation;
  }

  services.setFallbackService(new tse::FallbackServiceV2(*trx));
  services.setLoggerService(new tse::LoggerServiceV2());

  auto configService = std::make_unique<ConfigService>();
  configService->setHpsDomGroupingEnabled(tse::TrxUtil::isHpsDomGroupingEnabled(*trx));
  configService->setHpsIntlGroupingEnabled(tse::TrxUtil::isHpsIntlGroupingEnabled(*trx));
  configService->setHpsUseFlightRanges(tse::TrxUtil::useHPSFlightRanges(*trx));
  configService->travelDateDependantTaxNations() = initNationsFromCv(tax::tvlDepTaxNations);
  configService->flightNoDependantTaxNations() = initNationsFromCv(tax::fltnoDepTaxNations);
  configService->sameDayDepartureTaxNations() = initNationsFromCv(tax::sameDayDepTaxNations);

  services.setConfigService(configService.release());
  services.setCarrierApplicationService(new tse::CarrierApplicationServiceV2(trx->ticketingDate()));
  services.setCarrierFlightService(new tse::CarrierFlightServiceV2(trx->ticketingDate()));
  services.setCurrencyService(new tse::CurrencyServiceV2(*trx, trx->ticketingDate()));
  services.setLocService(new tse::LocServiceV2(trx->getRequest()->ticketingDT()));
  services.setMileageService(new tse::MileageServiceV2(*trx));
  services.setNationService(new tse::NationServiceV2());
  services.setAKHIFactorService(new tse::AKHIFactorServiceV2(trx->ticketingDate()));
  services.setSectorDetailService(new tse::SectorDetailServiceV2(trx->ticketingDate()));
  services.setServiceBaggageService(new tse::ServiceBaggageServiceV2(trx->ticketingDate()));
  services.setPassengerTypesService(new tse::PassengerTypesServiceV2(trx->ticketingDate()));
  services.setRepricingService(new tse::RepricingServiceV2(*trx, nullptr));
  services.setReportingRecordService(new tse::ReportingRecordServiceV2(trx->ticketingDate()));
  services.setRulesRecordsService(&tse::RulesRecordsServiceV2::instance());
  services.setPassengerMapper(new tse::PassengerMapperV2(*trx));
  services.setServiceFeeSecurityService(new tse::ServiceFeeSecurityServiceV2(trx->ticketingDate()));
  services.setTaxRoundingInfoService(new tse::TaxRoundingInfoServiceV2(*trx));
  services.setActivationInfoService(new tse::ActivationInfoServiceV2(*trx));
  services.setCustomerService(new tse::CustomerServiceV2(trx->dataHandle()));
  services.setPreviousTicketService(
      new tse::PreviousTicketServiceV2(*trx, new tse::UtcConfig(*trx)));
}

} // namespace tax
