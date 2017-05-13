//----------------------------------------------------------------------------
//
//  File:               TransactionOrchestrator.C
//  Description:        Service class to Transaction Orchestrator
//                      Adapter
//  Created:            11/16/2003
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Service/TransactionOrchestrator.h"

#include "Common/AltPricingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/SettlementTypesTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NoPNROptions.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag690Collector.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diag870Collector.h"
#include "Diagnostic/Diag983Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diagnostic.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Manager/TseManagerUtil.h"
#include "Server/TseServer.h"
#include "Service/RefundOrchestrator.h"

#include <sstream>

namespace tse
{
static Logger
logger("atseintl.Service.TransactionOrchestrator");

FALLBACK_DECL(obFeesWpaOption1);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(Cat33_Diag)
FALLBACK_DECL(fixDiag817)
FALLBACK_DECL(taxRexPricingTxType)

namespace
{
ConfigurableValue<std::string>
faresCollectionServiceName("TO_SVC", "FARES_COLLECTION_SERVICE");
ConfigurableValue<std::string>
faresValidationServiceName("TO_SVC", "FARES_VALIDATION_SERVICE");
ConfigurableValue<std::string>
pricingServiceName("TO_SVC", "PRICING_SERVICE");
ConfigurableValue<std::string>
shoppingServiceName("TO_SVC", "SHOPPING_SERVICE");
ConfigurableValue<std::string>
itinAnalyzerServiceName("TO_SVC", "ITIN_SERVICE");
ConfigurableValue<std::string>
taxServiceName("TO_SVC", "TAX_SERVICE");
ConfigurableValue<std::string>
fareCalcServiceName("TO_SVC", "FARE_CALC_SERVICE");
ConfigurableValue<std::string>
currencyServiceName("TO_SVC", "CURRENCY_SERVICE");
ConfigurableValue<std::string>
mileageServiceName("TO_SVC", "MILEAGE_SERVICE");
ConfigurableValue<std::string>
internalServiceName("TO_SVC", "INTERNAL_SERVICE");
ConfigurableValue<std::string>
fareDisplayServiceName("TO_SVC", "FAREDISPLAY_SERVICE");
ConfigurableValue<std::string>
fareSelectorServiceName("TO_SVC", "FARE_SELECTOR_SERVICE");
ConfigurableValue<std::string>
rexFareSelectorServiceName("TO_SVC", "REX_FARE_SELECTOR_SERVICE");
ConfigurableValue<std::string>
freeBagServiceName("TO_SVC", "FREE_BAG_SERVICE");
ConfigurableValue<std::string>
serviceFeesServiceName("TO_SVC", "SERVICE_FEES_SERVICE");
ConfigurableValue<std::string>
ticketingFeesServiceName("TO_SVC", "TICKETING_FEES_SERVICE");
ConfigurableValue<std::string>
s8BrandServiceName("TO_SVC", "S8_BRAND_SERVICE");
ConfigurableValue<std::string>
ticketingCxrServiceName("TO_SVC", "TICKETING_CXR_SERVICE");
ConfigurableValue<std::string>
ticketingCxrDispServiceName("TO_SVC", "TICKETING_CXR_DISPLAY_SERVICE");
ConfigurableValue<std::string>
decodeServiceName("TO_SVC", "DECODE_SERVICE");

inline std::ostream&
TOPATH_INFO(bool shouldLog, std::ostream& os, const char* msg)
{
  if (LIKELY(!shouldLog))
    return os;

  return os << msg << "\n";
}

inline std::ostream&
TOPATH_INFO(bool shouldLog, std::ostream& os, const std::string& msg)
{
  if (!shouldLog)
    return os;

  return os << msg << "\n";
}

void
TOPATH_PRINT(bool shouldLog, std::ostringstream& oss, PricingTrx* trx)
{
  if (!shouldLog || (trx == nullptr))
    return;

  DCFactory* factory = DCFactory::instance();
  if (factory == nullptr)
    return;

  DiagCollector* dc = factory->create(*trx);
  if (dc == nullptr)
    return;

  dc->enable(trx->diagnostic().diagnosticType());

  *dc << oss.str();

  dc->flushMsg();
}

inline bool
isFreeBagServiceNeeded(const PricingTrx& trx)
{
  return (trx.getTrxType() != PricingTrx::MIP_TRX ||
          (trx.getTrxType() == PricingTrx::MIP_TRX &&
           trx.getRequest()->isProcessBaggageForMIP())) &&
         !trx.getRequest()->isSFR();
}

class TOLatencyData : public tse::TSELatencyData
{
public:
  TOLatencyData(Trx& trx,
                const char* displayName,
                tse::to::StatFnc* statFnc,
                uint32_t activeThreads)
    : TSELatencyData(trx, displayName, true), _statFnc(statFnc), _activeThreads(activeThreads)
  {
  }

  virtual ~TOLatencyData()
  {
    if (LIKELY(_statFnc != nullptr))
    {
      _timer.stop();
      (*_statFnc)(_timer.getElapsedTime(), _activeThreads, _trx);
    }
  }

private:
  tse::to::StatFnc* _statFnc;
  uint32_t _activeThreads;

  TOLatencyData(const TOLatencyData& rhs);
  TOLatencyData& operator=(const TOLatencyData& rhs);
};

class CurrentServiceSetter
{
public:
  CurrentServiceSetter(Trx& trx, const Service* service) : _trx(trx)
  {
    _trx.setCurrentService(service);
  }

  ~CurrentServiceSetter() { _trx.setCurrentService(nullptr); }

private:
  Trx& _trx;
};
}

bool
TransactionOrchestrator::initialize(int argc, char* argv[])
{
  _faresCollectionServiceName = faresCollectionServiceName.getValue();
  _faresValidationServiceName = faresValidationServiceName.getValue();
  _pricingServiceName = pricingServiceName.getValue();
  _shoppingServiceName = shoppingServiceName.getValue();
  _itinAnalyzerServiceName = itinAnalyzerServiceName.getValue();
  _taxServiceName = taxServiceName.getValue();
  _fareCalcServiceName = fareCalcServiceName.getValue();
  _currencyServiceName = currencyServiceName.getValue();
  _mileageServiceName = mileageServiceName.getValue();
  _internalServiceName = internalServiceName.getValue();
  _fareDisplayServiceName = fareDisplayServiceName.getValue();
  _fareSelectorServiceName = fareSelectorServiceName.getValue();
  _rexFareSelectorServiceName = rexFareSelectorServiceName.getValue();
  _freeBagServiceName = freeBagServiceName.getValue();
  _serviceFeesServiceName = serviceFeesServiceName.getValue();
  _ticketingFeesServiceName = ticketingFeesServiceName.getValue();
  _s8BrandServiceName = s8BrandServiceName.getValue();
  _ticketingCxrServiceName = ticketingCxrServiceName.getValue();
  _ticketingCxrDispServiceName = ticketingCxrDispServiceName.getValue();
  _decodeServiceName = decodeServiceName.getValue();
  return true; // Success
}

bool
TransactionOrchestrator::process(PricingTrx& trx)
{
  try
  {
    return process(trx, ALL_SERVICES);
  }
  catch (ErrorResponseException& e)
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      throw;
    }
    // Check if No-Match should be attempted:
    if (e.code() == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
        e.code() == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
        e.code() == ErrorResponseException::NO_FARE_FOR_CLASS ||
        e.code() == ErrorResponseException::NO_FARES_RBD_CARRIER ||
        e.code() == ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS)
    {
      if (trx.isRequestedFareBasisInfoUseful())
      {
        throw;
      }
      else if (!trx.getRequest()->turnOffNoMatch() && trx.altTrxType() == PricingTrx::WP &&
               (!trx.getRequest()->isLowFareRequested() &&
                !trx.getRequest()->isLowFareNoAvailability()) &&
               trx.fareCalcConfig()->wpNoMatchPermitted() == 'Y')
      {
        AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(&trx);
        if (altTrx && process(*altTrx))
          return true;
      }
    }

    // If the No-Match doesn't apply, or failed, rethrow
    throw;
  }
}

const char* latencyTOPricing = "TO PROCESS PRICING";
const char* latencyTOMENew = "TO PROCESS ME NEW ";
const char* latencyTOMEExc1 = "TO PROCESS ME EXC1";
const char* latencyTOMEExc2 = "TO PROCESS ME EXC2";

//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
bool
TransactionOrchestrator::fixLegacyDiagnostic(DiagnosticTypes& diagType)
{
  switch (diagType)
  {
  case 8:
    diagType = Diagnostic610;
    return true;
  case 10:
    diagType = Diagnostic631;
    return true;
  case 11:
    diagType = Diagnostic634;
    return true;
  case 12:
    diagType = Diagnostic608;
    return true;
  case 74:
    diagType = Diagnostic660;
    return true;
  default:
    break;
  }
  return false;
}

uint64_t
TransactionOrchestrator::getServicesForDiagnostic(const DiagnosticTypes& diagType,
                                                  const PricingTrx& trx) const
{
  uint64_t services = 0;

  // This section is only for exceptions to the general range based logic below
  // Do not any new diagnostics to this switch statement unless its absolutely necessary
  switch (diagType)
  {
  case Diagnostic203:
  case Diagnostic853:
  case Diagnostic856:
    services = ITIN_ANALYZER | FARE_CALC;
    break;

  case Diagnostic852:
    services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | FREE_BAG;
    break;

  case AllFareDiagnostic:
  case Diagnostic319:
  case Diagnostic550:
    services = ITIN_ANALYZER | FARE_COLLECTOR;
    break;

  case Diagnostic863:
  case Diagnostic868:
    services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
    break;

  case Diagnostic202:
  case Diagnostic372:
  case Diagnostic400:
  case Diagnostic411:
  case Diagnostic405:
  case Diagnostic413:
  case Diagnostic416:
  case Diagnostic420:
  case Diagnostic430:
  case Diagnostic450:
  case Diagnostic460:
    services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
    break;

  default:
    break;
  }

  if (services == 0)
  {
    if (diagType >= FARES_DIAG_RANGE_BEGIN && diagType <= FARES_DIAG_RANGE_END)
    {
      // Fares diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR;
    }
    else if (diagType >= PRICING_DIAG_RANGE_BEGIN && diagType <= PRICING_DIAG_RANGE_END)
    {
      // Pricing diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
    }
    else if (diagType >= TAXES_DIAG_RANGE_BEGIN && diagType <= TAXES_DIAG_RANGE_END)
    {
      // Taxes diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES;
    }
    else if (diagType >= FARE_CALC_DIAG_RANGE_BEGIN && diagType <= FARE_CALC_DIAG_RANGE_END)
    {
      // FareCalc diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC;
    }
    else if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END)
    {
      // Internal diagnostics
      services = ITIN_ANALYZER | INTERNAL;
    }
    else if (diagType >= RULES_DIAG_RANGE_BEGIN && diagType <= RULES_DIAG_RANGE_END)
    {
      // Rules diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
    }
    else if (diagType >= BOOKING_CODE_DIAG_RANGE_BEGIN && diagType <= BOOKING_CODE_DIAG_RANGE_END)
    {
      // Booking codes diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
    }
    else if (diagType >= ROUTING_DIAG_RANGE_BEGIN && diagType <= ROUTING_DIAG_RANGE_END)
    {
      // Routing diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
    }
    else if (diagType >= RULECNTL_DIAG_RANGE_BEGIN && diagType <= RULECNTL_DIAG_RANGE_END)
    {
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
    }
    else if (diagType >= MIPS_DIAG_RANGE_BEGIN && diagType <= MIPS_DIAG_RANGE_END)
    {
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING | FARE_CALC;
    }
    else if (diagType >= SERVICE_FEES_DIAG_RANGE_BEGIN && diagType <= SERVICE_FEES_DIAG_RANGE_END)
    {
      // Service Fees diagnostics
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SERVICE_FEES;

      if (diagType == Diagnostic877)
        services |= TAXES;
    }
    else if (diagType >= BRANDED_FARES_DIAG_RANGE_BEGIN && diagType <= BRANDED_FARES_DIAG_RANGE_END)
    {
      services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND;
    }
  }

  if (trx.isPbbRequest())
  {
    services |= S8_BRAND;
  }

  return services;
}

//----------------------------------------------------------------------------
// process(PricingTrx, validServices)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(PricingTrx& trx, const uint64_t& validServices)
{
  const char* latencyTitle = latencyTOPricing;

  if (trx.excTrxType() == PricingTrx::NEW_WITHIN_ME)
    latencyTitle = latencyTOMENew;
  else if (trx.excTrxType() == PricingTrx::EXC1_WITHIN_ME)
    latencyTitle = latencyTOMEExc1;
  else if (trx.excTrxType() == PricingTrx::EXC2_WITHIN_ME)
    latencyTitle = latencyTOMEExc2;

  TSELatencyData latency(trx, latencyTitle);
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processPricing()");

  Diagnostic& diag = trx.diagnostic();

  if (diag.diagnosticType() == UpperBound)
  {
    // Internal Master Pricing diagnostics
    return filterAndInvokeServices(trx, INTERNAL, validServices);
  }

  uint64_t serviceBits = getServicesForDiagnostic(trx);

  if (Diagnostic975 == diag.diagnosticType() && !diag.processAllServices())
  {
    getServiceBitsForDiag975(diag, serviceBits);
  }

  if (serviceBits == 0 || diag.processAllServices())
  {
    if (trx.getRequest()->isSFR())
    {
      serviceBits =
          ITIN_ANALYZER | FARE_COLLECTOR | FARE_SELECTOR | FARE_VALIDATOR | PRICING | FARE_CALC;
      serviceBits |= (trx.isPbbRequest() ? S8_BRAND : 0);
    }
    else
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | SERVICE_FEES |
                    TICKETING_FEES | TAXES | PRICING | FARE_CALC;
    }

    if (isFreeBagServiceNeeded(trx))
    {
      serviceBits |= FREE_BAG;
    }
  }

  if (trx.getRequest()->isSFR() && (serviceBits & FARE_COLLECTOR))
    serviceBits |= FARE_SELECTOR;

  return filterAndInvokeServices(trx, serviceBits, validServices);
}

uint64_t
TransactionOrchestrator::getServicesForDiagnostic(PricingTrx& trx)
{
  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  if (diag.diagnosticType() == DiagnosticNone)
    return 0;

  fixLegacyDiagnostic(diagType);
  diag.diagnosticType() = diagType;

  if (trx.displayOnly())
  {
    if (diagType == AllFareDiagnostic)
      return ITIN_ANALYZER | FARE_COLLECTOR;
    else
      return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
  }

  // ** Note **
  //
  // This section is only for exceptions to the general range based logic below
  // Do not any new diagnostics to this switch statement unless its absolutely
  // necessary

  switch (diagType)
  {
  case Diagnostic997:
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SERVICE_FEES | TAXES | PRICING |
           FARE_CALC | INTERNAL;

  case Diagnostic892:
  case Diagnostic894: // Fallthrough on purpose
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | PRICING | SERVICE_FEES |
           TAXES | TICKETING_FEES | FARE_CALC;

  case Diagnostic853:
  case Diagnostic203:
  {
    return ITIN_ANALYZER | FARE_CALC;
  }

  case AllFareDiagnostic: // Fallthrough on purpose
  case Diagnostic550:
  {
    return ITIN_ANALYZER | FARE_COLLECTOR;
  }

  case Diagnostic202:
  case Diagnostic405: // Fallthrough on purpose
  case Diagnostic416: // Fallthrough on purpose
  case Diagnostic430:
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;

  case Diagnostic413: // Fallthrough on purpose
  case Diagnostic420: // Fallthrough on purpose
  case Diagnostic450: // Fallthrough on purpose
  case Diagnostic460: // Fallthrough on purpose
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;

  case Diagnostic319:
    return ITIN_ANALYZER | FARE_COLLECTOR;

  // Diagnostic 804
  case LegacyTaxDiagnostic24:
  {
    Diag804Collector* diag804Collector =
        dynamic_cast<Diag804Collector*>(DCFactory::instance()->create(trx));

    const std::string& modifiedTaxes =
        diag804Collector->rootDiag()->diagParamMapItem("MODIFIED_TAXES");

    if ("Y" == modifiedTaxes)
    {
      // FareCalc diagnostics
      return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SERVICE_FEES | TAXES | PRICING |
             FARE_CALC;
    }
  }

  break;

  case Diagnostic863: // Fallthrough on purpose
  case Diagnostic868: // Fallthrough on purpose
  case Diagnostic299:
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;

  default: // Handled below
    break;
  }

  if (diagType == Diagnostic411 && trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | SERVICE_FEES |
           TICKETING_FEES | TAXES | PRICING |
           ((trx.getTrxType() != PricingTrx::MIP_TRX) ? FREE_BAG : 0) | FARE_CALC;
  }

  if (diagType == Diagnostic676)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | SERVICE_FEES |
           TICKETING_FEES | TAXES | PRICING |
           ((trx.getTrxType() != PricingTrx::MIP_TRX) ? FREE_BAG : 0) | FARE_CALC;
  }

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187)
  {
    if (trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
      return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SERVICE_FEES |
             ((trx.getTrxType() != PricingTrx::MIP_TRX) ? FREE_BAG : 0);
  }

  if (diagType >= FARES_DIAG_RANGE_BEGIN && diagType <= FARES_DIAG_RANGE_END)
  {
    // Fares diagnostics
    return ITIN_ANALYZER | FARE_COLLECTOR;
  }
  else if (diagType >= PRICING_DIAG_RANGE_BEGIN && diagType <= PRICING_DIAG_RANGE_END)
  {
    // Pricing diagnostics
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
  }
  else if (diagType >= TAXES_DIAG_RANGE_BEGIN && diagType <= TAXES_DIAG_RANGE_END)
  {
    // Taxes diagnostics
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SERVICE_FEES | TAXES;
  }
  else if (diagType >= SERVICE_FEES_DIAG_RANGE_BEGIN && diagType <= SERVICE_FEES_DIAG_RANGE_END)
  {
    if (diagType == Diagnostic877)
      return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SERVICE_FEES | TAXES;

    // Service Fees  diagnostics
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SERVICE_FEES;
  }
  else if (diagType >= FARE_CALC_DIAG_RANGE_BEGIN && diagType <= FARE_CALC_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | SERVICE_FEES |
           TICKETING_FEES | TAXES | PRICING | (isFreeBagServiceNeeded(trx) ? FREE_BAG : 0) |
           FARE_CALC;
  }
  else if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END)
  {
    // Internal diagnostics
    return ITIN_ANALYZER | INTERNAL;
  }
  else if (diagType >= RULES_DIAG_RANGE_BEGIN && diagType <= RULES_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
  }
  else if (diagType >= BOOKING_CODE_DIAG_RANGE_BEGIN && diagType <= BOOKING_CODE_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
  }
  else if (diagType >= ROUTING_DIAG_RANGE_BEGIN && diagType <= ROUTING_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
  }
  else if (diagType >= RULECNTL_DIAG_RANGE_BEGIN && diagType <= RULECNTL_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
  }
  else if (diagType >= BRANDED_FARES_DIAG_RANGE_BEGIN && diagType <= BRANDED_FARES_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND;
  }
  else if (diagType >= MIPS_DIAG_RANGE_BEGIN && diagType <= MIPS_DIAG_RANGE_END)
  {
    return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | S8_BRAND | SERVICE_FEES |
           TICKETING_FEES | TAXES | PRICING | FARE_CALC;
  }
  else if (diagType >= ALT_PRICING_DIAG_RANGE_BEGIN && diagType <= ALT_PRICING_DIAG_RANGE_END)
  {
    // FIXME: service bits
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);
    if (fcConfig->wpNoMatchPermitted() == 'Y')
    {
      // FareCalc diagnostics
      return ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING |
             ((trx.getTrxType() != PricingTrx::MIP_TRX) ? FREE_BAG : 0) | FARE_CALC;
    }
    if (!diag.processAllServices())
      throw tse::ErrorResponseException(ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY);
  }
  else
  {
    LOG4CXX_ERROR(logger,
                  "TransactionOrchestrator() - processPricing() unsupport diagnostic " << diagType);
    if (!diag.processAllServices())
      throw tse::ErrorResponseException(ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY);
  }

  // LOG4CXX_DEBUG(logger,"TransactionOrchestrator() - Leaving processPricing()");

  return 0;
}

//----------------------------------------------------------------------------
// process(ExchangePricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(ExchangePricingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS PRICING");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processPricing()");

  uint64_t services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING |
                      FARE_CALC | INTERNAL | FREE_BAG;

  services |= (trx.isPbbRequest() ? S8_BRAND : 0);

  return process(trx, services);
}
//----------------------------------------------------------------------------
// process(StructuredRuleTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(StructuredRuleTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS STRUCTUREDRULE");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processStructuredRuleTrx()");

  uint64_t services = ITIN_ANALYZER | FARE_COLLECTOR | FARE_SELECTOR | FARE_VALIDATOR | PRICING |
                      FARE_CALC | S8_BRAND;

  return process(trx, services);
}

bool
TransactionOrchestrator::process(FrequentFlyerTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS FREQUENTFLYER");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processFrequentFlyerTrx()");

  const uint64_t services = DECODE;

  return invokeServices(trx, services);
}

//----------------------------------------------------------------------------
// process(MultiExchangeTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(MultiExchangeTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS MULTI_EXC");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processMultiExchangeTrx");

  Code<4> trxName = " NEW";

  const bool diagNewPricingTrx = trx.newPricingTrx()->excTrxType() == PricingTrx::ME_DIAG_TRX;

  try
  {
    bool result = true;
    if (!trx.skipNewPricingTrx())
    {
      trx.newPricingTrx()->setExcTrxType(PricingTrx::PORT_EXC_TRX);

      result = process(*(trx.newPricingTrx()));

      if (diagNewPricingTrx)
        trx.newPricingTrx()->setExcTrxType(PricingTrx::ME_DIAG_TRX);
      else
        trx.newPricingTrx()->setExcTrxType(PricingTrx::NEW_WITHIN_ME);

      if (!result)
        return result;
    }

    if (!trx.skipExcPricingTrx1())
    {
      trxName = " EX1";
      const PricingTrx::ExcTrxType savedType = trx.excPricingTrx1()->excTrxType();
      trx.excPricingTrx1()->setExcTrxType(PricingTrx::PORT_EXC_TRX);

      result = process(*(trx.excPricingTrx1()));
      trx.excPricingTrx1()->setExcTrxType(savedType);
      if (!result)
        return result;
    }

    if (!trx.skipExcPricingTrx2())
    {
      trxName = " EX2";
      const PricingTrx::ExcTrxType savedType = trx.excPricingTrx2()->excTrxType();
      trx.excPricingTrx2()->setExcTrxType(PricingTrx::PORT_EXC_TRX);

      result = process(*(trx.excPricingTrx2()));
      trx.excPricingTrx2()->setExcTrxType(savedType);
      if (!result)
        return result;
    }
    return true;
  }
  catch (ErrorResponseException& e)
  {
    std::string newMsg = e.message() + trxName;

    if (diagNewPricingTrx)
      trx.newPricingTrx()->setExcTrxType(PricingTrx::ME_DIAG_TRX);
    else
      trx.newPricingTrx()->setExcTrxType(PricingTrx::NEW_WITHIN_ME);

    throw ErrorResponseException(e.code(), newMsg.c_str());
  }
}

//----------------------------------------------------------------------------
// process(ShoppingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(ShoppingTrx& trx)
{
  if (PricingTrx::ESV_TRX == trx.getTrxType())
  {
    return processESV(trx);
  }

  TSELatencyData latency(trx, "TO PROCESS SHOPPING");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processShopping()");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  uint64_t serviceBits = 0;

  if (trx.startShortCutPricingItin())
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TICKETING_FEES |
                  TAXES | FARE_CALC | SHOPPING;
  }
  else
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | SHOPPING;
  }

  if (!diag.processAllServices())
  {
    switch (diagType)
    {
    case Diagnostic900:
    case Diagnostic902:
    case Diagnostic922:
    {
      serviceBits = ITIN_ANALYZER | SHOPPING;
      break;
    }

    case Diagnostic901:
    case Diagnostic903:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | SHOPPING;
      break;
    }

    case Diagnostic905:
    {
      serviceBits = SHOPPING;
      break;
    }

    case Diagnostic904:
    case Diagnostic906:
    case Diagnostic907:
    case Diagnostic908:
    case Diagnostic909:
    case Diagnostic911:
    // case Diagnostic912 :
    case Diagnostic953:
    case Diagnostic954:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SHOPPING;
      break;
    }

    case Diagnostic910:
    case Diagnostic912:
    case Diagnostic914:
    case Diagnostic930:
    case Diagnostic985:
    case Diagnostic959:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | SHOPPING;
      break;
    }

    case Diagnostic319:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
      break;
    }

    case Diagnostic975:
    {
      getServiceBitsForDiag975(diag, serviceBits);
      break;
    }

    default:
    { // default is set before switch
      break;
    }
    } // switch
  }

  switch (diagType)
  {
  case Diagnostic910:
  case Diagnostic912:
  case Diagnostic914:
  case Diagnostic930:
  case Diagnostic985:
  case Diagnostic959:
  {
    diag.deActivate();
    const bool res = invokeServices(trx, serviceBits);
    diag.activate();
    return res;
  }

  case Diagnostic319:
  {
    diag.activate();
    return invokeServices(trx, serviceBits);
  }

  default:
  {
    return invokeServices(trx, serviceBits);
  }
  }
}

//----------------------------------------------------------------------------
// processESV(ShoppingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::processESV(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "TO PROCESS SHOPPING ESV");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator::processESV(ShoppingTrx&)");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  uint64_t serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SHOPPING;
  if (!diag.processAllServices())
  {
    switch (diagType)
    {
    case Diagnostic900:
    case Diagnostic902:
    case Diagnostic952:
    {
      serviceBits = ITIN_ANALYZER | SHOPPING;
      break;
    }

    case Diagnostic903:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | SHOPPING;
      break;
    }

    case Diagnostic905:
    {
      serviceBits = SHOPPING;
      break;
    }

    case Diagnostic953:
    case Diagnostic954:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SHOPPING;
      break;
    }

    case Diagnostic956:
    case Diagnostic957:
    case Diagnostic958:
    case Diagnostic959:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | SHOPPING;
      break;
    }
    default:
      break; // defaul is set before switch
    }
  }

  return invokeServices(trx, serviceBits);
}

//----------------------------------------------------------------------------
// process(FlightFinder)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(FlightFinderTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS FLIGHT FINDER");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(FLIGHTFINDER)");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  uint64_t serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SHOPPING;

  if (!diag.processAllServices() && !trx.ignoreDiagReq())
  {
    switch (diagType)
    {
    case Diagnostic900:
    case Diagnostic902:
    case Diagnostic905:
    case Diagnostic914:
    {
      serviceBits = ITIN_ANALYZER | SHOPPING;
      break;
    }
    case Diagnostic901:
    case Diagnostic903:
    {
      serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | SHOPPING;
      break;
    }
    default:
    {
      // default is set before switch
    }
    }
  }

  return invokeServices(trx, serviceBits);
}

//----------------------------------------------------------------------------
// process(RexShoppingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(RexShoppingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS RexShoppingTrx");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(RexShoppingTrx&)");

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  uint64_t services = 0;
  trx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
  RexPricingTrx::DoubleQualifierWrapper dQW(*trx.getOptions());

  if (diagType == Diagnostic905)
  {
    dQW.restore();
    services = SHOPPING;
  }
  else if (diagType == Diagnostic194)
  {
    services = ITIN_ANALYZER | SHOPPING;
  }
  else if (diagType == AllFareDiagnostic)
  {
    services = ITIN_ANALYZER | FARE_COLLECTOR;
  }
  else if (diagType == Diagnostic231)
  {
    services = ITIN_ANALYZER | FARE_COLLECTOR | REX_FARE_SELECTOR;
  }
  else
  {
    services = ITIN_ANALYZER | FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING;
  }

  bool result = invokeServices(trx, services);

  if (result && (services & PRICING))
  {
    trx.trxPhase() = RexPricingTrx::MATCH_EXC_RULE_PHASE;
    services = REX_FARE_SELECTOR | SHOPPING;
    result = invokeServices(trx, services);
  }

  return result;
}

class CSOThreadTask : public TseCallableTrxTask
{
public:
  CSOThreadTask(TransactionOrchestrator& to, RexPricingTrx& trx, uint64_t services)
    : _to(to), _trx(trx), _services(services)
  {
    TseCallableTrxTask::trx(&_trx);
    trx.createPricingTrxForCSO(false);
  }

  void performTask() override { _to.processUflItin(_trx, _services); }

private:
  TransactionOrchestrator& _to;
  RexPricingTrx& _trx;
  uint64_t _services;
};

bool
TransactionOrchestrator::isErrorEnforceRedirection(const ErrorResponseException& e) const
{
  return e.code() == ErrorResponseException::UNABLE_TO_MATCH_FARE ||
         e.code() == ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES ||
         e.code() == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
         e.code() == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS;
}

bool
TransactionOrchestrator::isCSOTrxApplicable(const RexPricingTrx& trx) const
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    return trx.billing()->actionCode() == "WFR.C";

  return !trx.isSecondaryReqExist();
}

uint64_t
TransactionOrchestrator::getExcItinServices(const PricingTrx& trx)
{
  return FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING | TAXES;
}

bool
TransactionOrchestrator::rexPricingMainProcess(RexPricingTrx& trx)
{
  trx.diagnostic().deActivate();

  uint64_t services = FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC;

  if (isFreeBagServiceNeeded(trx))
    services |= FREE_BAG;

  services |= (trx.isPbbRequest() ? S8_BRAND : 0);

  // don't change these two lines order!
  CSOThreadTask task(*this, trx, ITIN_ANALYZER | services);
  TseRunnableExecutor taskExecutor(TseThreadingConst::TO_TASK);

  if (isCSOTrxApplicable(trx))
    taskExecutor.execute(task);

  bool result = false;
  try
  {
    result = processExcItin(trx, getExcItinServices(trx));

    if (trx.isSecondaryReqExist())
      taskExecutor.execute(task);

    trx.getRequest()->lowFareRequested() = 'Y';

    result = result && processNewItin(trx, services);

    const TSELatencyData latency(trx, "WAITING FOR CSO TO FINISH");
    taskExecutor.wait();
  }
  catch (const ErrorResponseException& e)
  {
    if (trx.isSecondaryReqExist())
    {
      if (isErrorEnforceRedirection(e))
      {
        trx.redirectReasonError() = e;
        return processEftItin(trx, ITIN_ANALYZER | services, false);
      }
      else
        processUflItin(trx, ITIN_ANALYZER | services);
    }
    throw;
  }
  catch (...)
  {
    if (trx.isSecondaryReqExist())
      processUflItin(trx, ITIN_ANALYZER | services);
    throw;
  }

  return result;
}

//----------------------------------------------------------------------------
// process(RexPricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(RexPricingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS RexPricingTrx");

  LOG4CXX_DEBUG(logger, "Process(RexPricingTrx&)");

  if (trx.reqType() == TX_TAX_INFO_REQUEST)
  {
    trx.setExcTrxType(PricingTrx::TAX_INFO_TRX);
    if (trx.originalTktIssueDT().isValid())
      trx.setFareApplicationDT(trx.originalTktIssueDT());
    return fallback::taxRexPricingTxType(&trx) ? false : invokeServices(trx, TAXES);
  };

  trx.getRequest()->lowFareRequested() = 'Y';

  bool result = invokeServices(trx, ITIN_ANALYZER);

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  DiagnosticQualifier qualifier = diagnosticQualifierProcess(trx);

  switch (qualifier)
  {
  case DQ_NONE:
  {
    result = result && rexPricingMainProcess(trx);
    break;
  }
  case DQ_ITEXC:
  {
    trx.diagnostic().activate();

    uint64_t services = 0;
    if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END)
      services = INTERNAL;
    else if (diagType == Diagnostic231)
      services = FARE_COLLECTOR | REX_FARE_SELECTOR;
    else
      services = getExcItinServices(trx);

    result = result && processExcItin(trx, services);

    break;
  }
  case DQ_ITALL:
  case DQ_ITNEW:
  {
    if (qualifier == DQ_ITALL)
      trx.diagnostic().activate();
    else
      trx.diagnostic().deActivate();

    result = result && processExcItin(trx, getExcItinServices(trx));

    if (qualifier == DQ_ITALL)
    {
      if (!fallback::Cat33_Diag(&trx))
      {
        if (diagType != Diagnostic817 && diagType != Diagnostic825)
        {
          DiagManager diagC(trx, diagType);
          diagC << "- BEGIN NEW ITIN DIAGNOSTIC -\n";
        }
      }
      else
      {
        if (diagType != Diagnostic817)
        {
          DiagManager diagC(trx, diagType);
          diagC << "- BEGIN NEW ITIN DIAGNOSTIC -\n";
        }
      }
    }
    else
      trx.diagnostic().activate();

    bool onlyAsBooked =
        diagType == Diagnostic690 &&
        trx.diagnostic().diagParamMapItem(Diag690Collector::DISPLAY_SOLUTION) == "BOOKED";

    if (!onlyAsBooked)
      trx.getRequest()->lowFareRequested() = 'Y';

    if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) &&
        (diagType == Diagnostic187 &&
         trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD"))
    {
      result =
          result &&
          processNewItin(trx, (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR) ^ ITIN_ANALYZER);
    }
    else
      result = result &&
               processNewItin(
                   trx, (getServicesForDiagnostic(diagType, trx) | ITIN_ANALYZER) ^ ITIN_ANALYZER);

    break;
  }
  case DQ_ITUFL:
  {
    if (trx.isSecondaryReqExist())
    {
      trx.diagnostic().deActivate();
      try
      {
        processExcItin(trx, FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING);
      }
      catch (const ErrorResponseException& e)
      {
        if (isErrorEnforceRedirection(e))
          break;
      }
    }

    trx.diagnostic().activate();
    trx.createPricingTrxForCSO(true);

    result = result && processUflItin(trx, getServicesForDiagnostic(diagType, trx));

    break;
  }
  case DQ_ITEFT:
  {
    if (!trx.isSecondaryReqExist())
      break;

    trx.diagnostic().deActivate();
    try
    {
      processExcItin(trx, FARE_COLLECTOR | REX_FARE_SELECTOR | PRICING);
    }
    catch (const ErrorResponseException& e)
    {
      if (isErrorEnforceRedirection(e))
      {
        trx.diagnostic().activate();
        result = result && processEftItin(trx, getServicesForDiagnostic(diagType, trx), true);
      }
    }

    break;
  }
  default:
    ;
  }

  if (!trx.reissuePricingErrorMsg().empty() && !trx.isRebookedSolutionValid())
  {
    throw ErrorResponseException(trx.reissuePricingErrorCode(),
                                 trx.reissuePricingErrorMsg().c_str());
  }

  if (qualifier == DQ_ITUFL && !trx.csoPricingErrorMsg().empty())
  {
    throw ErrorResponseException(trx.csoPricingErrorCode(), trx.csoPricingErrorMsg().c_str());
  }

  return result;
}

//----------------------------------------------------------------------------
// process(RexExchangeTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Process(RexExchangeTrx&)");

  return process(static_cast<RexPricingTrx&>(trx));
}

//----------------------------------------------------------------------------
// process(RexPricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(RefundPricingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS RefundPricingTrx");

  LOG4CXX_DEBUG(logger, "Process(RefundPricingTrx&)");

  RefundOrchestrator orchestrator(trx, *this);

  return orchestrator.process();
}

TransactionOrchestrator::DiagnosticQualifier
TransactionOrchestrator::diagnosticQualifierProcess(const RexPricingTrx& trx) const
{
  const DiagnosticTypes& diagType = trx.diagnostic().diagnosticType();

  const std::string& diagItin = trx.diagnostic().diagParamMapItem(Diagnostic::ITIN_TYPE);

  if (!fallback::fixDiag817(&trx))
  {
    switch (diagType)
    {
    case DiagnosticNone:
      return DQ_NONE;
    case Diagnostic194:
    case Diagnostic231:
    case Diagnostic331:
    case Diagnostic688:
      return diagItin == "RED" ? DQ_ITEFT : DQ_ITEXC;
    case Diagnostic689:
      return diagItin == "RED" ? DQ_ITEFT : DQ_ITNEW;
      return DQ_ITALL;
    default:
      ;
    }
  }
  else
  {
    switch (diagType)
    {
    case DiagnosticNone:
      return DQ_NONE;
    case Diagnostic194:
    case Diagnostic231:
    case Diagnostic331:
    case Diagnostic688:
      return diagItin == "RED" ? DQ_ITEFT : DQ_ITEXC;
    case Diagnostic689:
      return diagItin == "RED" ? DQ_ITEFT : DQ_ITNEW;
    case Diagnostic817:
    case Diagnostic825:
    case Diagnostic832:
    case Diagnostic833:
      return DQ_ITALL;
    default:
      ;
    }
  }

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
  {
  }
  else if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END &&
           diagItin != "UFL" && diagItin != "RED")
    return DQ_ITEXC;

  if (diagItin == "EXC")
    return DQ_ITEXC;
  if (diagItin == "ALL")
    return DQ_ITALL;
  if (diagItin == "UFL")
    return DQ_ITUFL;
  if (diagItin == "RED")
    return DQ_ITEFT;

  return DQ_ITNEW;
}

bool
TransactionOrchestrator::processExcItin(RexPricingTrx& trx, uint64_t services)
{
  trx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;

  RexPricingTrx::DoubleQualifierWrapper dQW(*trx.getOptions());

  if (trx.diagnostic().diagnosticType() == Diagnostic198)
  {
    dQW.restore();
  }

  LOG4CXX_DEBUG(logger, "Process(RexPricingTrx&) - EXC part");

  bool result = false;
  result = invokeServices(trx, services);

  if (result && (services & PRICING))
  {
    trx.trxPhase() = RexPricingTrx::MATCH_EXC_RULE_PHASE;
    result = invokeServices(trx, REX_FARE_SELECTOR);
  }

  return result;
}

bool
TransactionOrchestrator::processNewItin(RexPricingTrx& trx, uint64_t services)
{
  trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;

  trx.setAnalyzingExcItin(false);

  LOG4CXX_DEBUG(logger, "Process(RexPricingTrx&) - NEW part");

  if (trx.tag1PricingSvcCallStatus() && (services & PRICING))
  {
    if (!oldProcessNewItin(trx, FARE_COLLECTOR | FARE_VALIDATOR | PRICING) &&
        trx.reissuePricingErrorCode() != ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS &&
        trx.reissuePricingErrorCode() != ErrorResponseException::NO_FARE_FOR_CLASS_USED &&
        trx.reissuePricingErrorCode() != ErrorResponseException::REISSUE_RULES_FAIL)
      return false;

    if (trx.shouldCallPricingSvcSecondTime())
    {
      std::pair<FarePath*, FarePath*> rebookedBooked =
          std::make_pair(trx.lowestRebookedFarePath(), trx.lowestBookedFarePath());

      trx.prepareForSecondPricingSvcCall();

      if (!oldProcessNewItin(trx, PRICING))
        return false;

      trx.validSolutionsPlacement(rebookedBooked);
    }

    services &= ~FARE_COLLECTOR & ~FARE_VALIDATOR & ~PRICING;
  }

  return oldProcessNewItin(trx, services);
}

bool
TransactionOrchestrator::oldProcessNewItin(RexPricingTrx& trx, uint64_t services)
{
  bool result = false;
  try
  {
    result = invokeServices(trx, services);
  }
  catch (ErrorResponseException& ex)
  {
    ErrorResponseException rrf(ErrorResponseException::REISSUE_RULES_FAIL);
    ErrorResponseException& e =
        (ex.code() == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS &&
         (trx.rsvCounterStatus() || (!trx.rsvCounterStatus() && trx.fareFailFareBytes())))
            ? rrf
            : ex;
    trx.reissuePricingErrorCode() = e.code();
    trx.reissuePricingErrorMsg() = e.message();
    LOG4CXX_WARN(logger, "Exception:" << e.message() << " - Reissue pricing failed");
  }
  catch (std::exception& e)
  {
    trx.reissuePricingErrorCode() = ErrorResponseException::UNKNOWN_EXCEPTION;
    trx.reissuePricingErrorMsg() = e.what();
    LOG4CXX_WARN(logger, "Exception:" << e.what() << " - Reissue pricing failed");
  }
  catch (...)
  {
    trx.reissuePricingErrorCode() = ErrorResponseException::UNKNOWN_EXCEPTION;
    trx.reissuePricingErrorMsg() = "UNKNOWN EXCEPTION - REISSUE PRICING FAILED";
    LOG4CXX_WARN(logger, trx.reissuePricingErrorMsg());
  }

  return result;
}

bool
TransactionOrchestrator::processUflItin(RexPricingTrx& trx, uint64_t services)
{
  if (!trx.pricingTrxForCSO())
    return false;

  TSELatencyData latency(trx, "CANCEL AND START OVER PRICING TASK");
  LOG4CXX_DEBUG(logger, "Process(RexPricingTrx&) - UFL part");

  try
  {
    if ((trx.csoTransSuccessful() = invokeServices(*trx.pricingTrxForCSO(), services)))
    {
      if (!trx.pricingTrxForCSO()->itin().empty() &&
          !trx.pricingTrxForCSO()->itin().front()->farePath().empty())
      {
        trx.lowestCSOFarePath() = trx.pricingTrxForCSO()->itin().front()->farePath().front();
      }
    }
  }
  catch (ErrorResponseException& ex)
  {
    trx.csoPricingErrorCode() = ex.code();
    trx.csoPricingErrorMsg() = ex.message();
    LOG4CXX_WARN(logger, "Exception:" << ex.message() << " - Cancel and start over pricing failed");
  }
  catch (std::exception& e)
  {
    trx.csoPricingErrorCode() = ErrorResponseException::UNKNOWN_EXCEPTION;
    trx.csoPricingErrorMsg() = e.what();
    LOG4CXX_WARN(logger, "Exception:" << e.what() << " - Cancel and start over pricing failed");
  }
  catch (...)
  {
    trx.csoPricingErrorMsg() = "UNKNOWN EXCEPTION - CANCEL AND START OVER PRICING FAILED";
    trx.csoPricingErrorCode() = ErrorResponseException::UNKNOWN_EXCEPTION;
    LOG4CXX_WARN(logger, trx.csoPricingErrorMsg());
  }

  return trx.csoTransSuccessful();
}

bool
TransactionOrchestrator::processEftItin(RexBaseTrx& trx, uint64_t services, bool diag)
{
  trx.redirected() = true;
  trx.setActionCode();

  trx.createExchangePricingTrxForRedirect(diag);
  if (!trx.exchangePricingTrxForRedirect())
  {
    LOG4CXX_WARN(logger, "Invalid transaction for redirect to Port Exchange - redirect failed");

    throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                 "INVALID TRANSACTION FOR REDIRECT TO PORT EXCHANGE");
  }

  TSELatencyData latency(trx, "EXCHANGE FAST TRACK REDIRECTION TASK");
  LOG4CXX_DEBUG(logger, "Process(RexPricingTrx&) - redirect to EFT");

  trx.redirectResult() = invokeServices(*trx.exchangePricingTrxForRedirect(), services);

  return trx.redirectResult();
}

//----------------------------------------------------------------------------
// NoPNRPricingTrx
//---------------------------------------------------------------------------
void
TransactionOrchestrator::changeMaxNoOptions(NoPNRPricingTrx& trx, int maxOptions)
{
  const NoPNROptions* options = trx.noPNROptions();

  // new options
  NoPNROptions* newOptions;
  trx.dataHandle().get(newOptions);
  newOptions->userApplType() = options->userApplType();
  newOptions->userAppl() = options->userAppl();
  newOptions->loc1() = options->loc1();
  newOptions->wqNotPermitted() = options->wqNotPermitted();
  newOptions->maxNoOptions() = maxOptions;
  newOptions->wqSort() = options->wqSort();
  newOptions->wqDuplicateAmounts() = options->wqDuplicateAmounts();
  newOptions->fareLineHeaderFormat() = options->fareLineHeaderFormat();
  newOptions->passengerDetailLineFormat() = options->passengerDetailLineFormat();
  newOptions->fareLinePTC() = options->fareLinePTC();
  newOptions->primePTCRefNo() = options->primePTCRefNo();
  newOptions->secondaryPTCRefNo() = options->secondaryPTCRefNo();
  newOptions->fareLinePTCBreak() = options->fareLinePTCBreak();
  newOptions->passengerDetailPTCBreak() = options->passengerDetailPTCBreak();
  newOptions->negPassengerTypeMapping() = options->negPassengerTypeMapping();
  newOptions->noMatchOptionsDisplay() = options->noMatchOptionsDisplay();
  newOptions->allMatchTrailerMessage() = options->allMatchTrailerMessage();
  newOptions->matchIntegratedTrailer() = options->matchIntegratedTrailer();
  newOptions->accompaniedTvlTrailerMsg() = options->accompaniedTvlTrailerMsg();
  newOptions->rbdMatchTrailerMsg() = options->rbdMatchTrailerMsg();
  newOptions->rbdNoMatchTrailerMsg() = options->rbdNoMatchTrailerMsg();
  newOptions->rbdNoMatchTrailerMsg2() = options->rbdNoMatchTrailerMsg2();
  newOptions->displayFareRuleWarningMsg() = options->displayFareRuleWarningMsg();
  newOptions->displayFareRuleWarningMsg2() = options->displayFareRuleWarningMsg2();
  newOptions->displayFinalWarningMsg() = options->displayFinalWarningMsg();
  newOptions->displayFinalWarningMsg2() = options->displayFinalWarningMsg2();
  newOptions->displayTaxWarningMsg() = options->displayTaxWarningMsg();
  newOptions->displayTaxWarningMsg2() = options->displayTaxWarningMsg2();
  newOptions->displayPrivateFareInd() = options->displayPrivateFareInd();
  newOptions->displayNonCOCCurrencyInd() = options->displayNonCOCCurrencyInd();
  newOptions->displayTruePTCInFareLine() = options->displayTruePTCInFareLine();
  newOptions->applyROInFareDisplay() = options->applyROInFareDisplay();
  newOptions->alwaysMapToADTPsgrType() = options->alwaysMapToADTPsgrType();
  newOptions->noMatchRBDMessage() = options->noMatchRBDMessage();
  newOptions->noMatchNoFaresErrorMsg() = options->noMatchNoFaresErrorMsg();

  int oldMax = options->maxNoOptions();
  NoPNRPricingTrx::FareTypes::FTGroup fareTypeGroup =
      trx.mapFTtoFTG(trx.diagnostic().diagParamMapItem(Diagnostic::FARE_TYPE_DESIGNATOR));

  const std::vector<NoPNROptionsSeg*>& segs = options->segs();
  std::vector<NoPNROptionsSeg*>::const_iterator iterEnd = segs.end();
  std::vector<NoPNROptionsSeg*>::const_iterator iter = segs.begin();

  int nbOptions = 0;
  int newVal = 0;
  while (iter != iterEnd)
  {
    // new segment
    // simulate that segment comes from database - can't use dataHandle, using new instead
    NoPNROptionsSeg* seg = *iter, *newSeg = new NoPNROptionsSeg;
    newSeg->seqNo() = seg->seqNo();
    newSeg->fareTypeGroup() = seg->fareTypeGroup();

    if (fareTypeGroup == NoPNRPricingTrx::FareTypes::FTG_NONE)
    {
      newVal = std::max((int)((double)seg->noDisplayOptions() * maxOptions / oldMax + 0.5), 1);
      newSeg->noDisplayOptions() = newVal;
    }
    else
    {
      newVal = (seg->fareTypeGroup() == fareTypeGroup) ? maxOptions : 0;
      newSeg->noDisplayOptions() = newVal;
    }
    nbOptions += newVal;
    newOptions->segs().push_back(newSeg);
    ++iter;
  }
  newOptions->totalNoOptions() = nbOptions;
  trx.changeOptions(newOptions);
}

bool
TransactionOrchestrator::process(NoPNRPricingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS NOPNRPRICING");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(NoPNRPricingTrx&)");

  trx.setFullFBCItin();

  if (trx.isFullFBCItin() && trx.getOptions()->isNoMatch())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT);

  bool result = false;

  // electronic ticket always on
  trx.getRequest()->electronicTicket() = 'Y';

  // initialize
  trx.loadNoPNROptions();

  // if WQ is not permitted for agent - break processing
  if (trx.noPNROptions()->wqNotPermitted() == 'Y')
    throw ErrorResponseException(ErrorResponseException::NO_FARES_RBD_CARRIER);

  trx.fareTypes().loadFareTypes();
  trx.initializeOpenSegmentDates();
  NoPNRPricingTrx::Solutions& solutions = trx.solutions();
  solutions.initialize();

  // check if low fare request
  bool lowestFare = trx.getRequest()->isLowFareRequested();
  trx.lowestFare() = lowestFare;
  trx.getRequest()->lowFareRequested() = 'N';

  // diagnostics
  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes orgDiagType = diag.diagnosticType(), diagType = orgDiagType;
  bool noMatchDiag;
  if (diagType >= ALT_PRICING_DIAG_RANGE_BEGIN && diagType <= ALT_PRICING_DIAG_RANGE_END)
  {
    diagType = (DiagnosticTypes)(diagType - ALT_PRICING_DIAG_RANGE_BEGIN);
    noMatchDiag = true;
  }
  else
    noMatchDiag = false;

  const bool isDiag = diagType != DiagnosticNone;
  if (isDiag && !trx.isFullFBCItin())
  {
    std::string diagMax = diag.diagParamMapItem("NO");
    if (!diagMax.empty())
    {
      int maxOptions = std::max(std::atoi(diagMax.c_str()), 0);
      changeMaxNoOptions(trx, maxOptions);
    }
  }
  if (diagType == Diagnostic853 || diagType == Diagnostic856)
    return invokeServices(trx, getServicesForDiagnostic(diagType, trx));

  // services
  uint64_t services = (isDiag && !noMatchDiag) ? getServicesForDiagnostic(diagType, trx)
                                               : ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR |
                                                     PRICING | TAXES | FARE_CALC | FREE_BAG;
  if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END)
    services |= INTERNAL;

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
  {
    services =
        ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES | FARE_CALC | FREE_BAG;
  }

  bool fareCalc = (services & FARE_CALC) != 0;
  bool taxes = (services & TAXES) != 0;
  bool baggage = (services & FREE_BAG) != 0;
  services = services & ~(FARE_CALC | TAXES | FREE_BAG);

  // processing
  bool reprocess = false;

  if (!trx.isNoMatch())
  {
    try
    {
      LOG4CXX_DEBUG(logger,
                    "TransactionOrchestrator::process(NoPNRPricingTrx&) - Run services for MATCH");
      result = invokeServices(trx, services);
    }
    catch (const ErrorResponseException& exc)
    {
      const ErrorResponseException::ErrorResponseCode& code = exc.code();

      if (!trx.isFullFBCItin() &&
          (code == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
           code == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
           code == ErrorResponseException::NO_FARE_FOR_CLASS ||
           code == ErrorResponseException::NO_FARES_RBD_CARRIER ||
           code == ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS ||
           code == ErrorResponseException::NO_PUBLIC_FARES_VALID_FOR_PASSENGER ||
           code == ErrorResponseException::NO_PRIVATE_FARES_VALID_FOR_PASSENGER))
      {
        reprocess = true;
      }
      else
        throw;
    }
    catch (...)
    {
      throw;
    }

    if (!trx.isFullFBCItin())
    {
      if (trx.isLowestFareOverride() && !lowestFare)
      {
        if (!solutions.all())
        {
          trx.integrated() = true;
          reprocess = true;
        }
      }
      else
      {
        const std::vector<PaxType*>& paxTypes = trx.paxType();
        std::vector<PaxType*>::const_iterator iter = paxTypes.begin();
        std::vector<PaxType*>::const_iterator iterEnd = paxTypes.end();
        while (iter != iterEnd)
        {
          PaxType* paxType = *iter;

          if (solutions.found(paxType) != 0)
            solutions.process(paxType) = false;
          ++iter;
        }
      }

      if (reprocess)
      {
        AltPricingUtil::prepareToReprocess(trx);
        trx.reprocess() = true;
        services &= FARE_VALIDATOR | PRICING | TAXES | INTERNAL;
      }
      else if (isDiag && noMatchDiag)
      {
        trx.diagDisplay() = false;
        return true;
      }
    }
  }

  bool diagChanged = false;
  if (trx.isNoMatch())
  {
    if (isDiag)
    {
      const bool isXM = trx.getOptions()->isNoMatch();
      if (isXM)
      {
        if (noMatchDiag)
          throw ErrorResponseException(ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY);
      }
      else
      {
        if (noMatchDiag)
        {
          diag.diagnosticType() = diagType;
          diagChanged = true;
        }
        else
        {
          if (!reprocess)
            trx.diagDisplay() = false;
          return true;
        }
      }
      uint64_t diagServices = getServicesForDiagnostic(diagType, trx);
      if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
          trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
      {
        diagServices = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES |
                       FARE_CALC | FREE_BAG;
      }
      fareCalc = (diagServices & FARE_CALC) != 0;
      taxes = (diagServices & TAXES) != 0;
      baggage = (diagServices & FREE_BAG) != 0;
      services = services & diagServices & ~(FARE_CALC | FREE_BAG);
    }

    LOG4CXX_DEBUG(logger,
                  "TransactionOrchestrator::process(NoPNRPricingTrx&) - Run services for NO-MATCH");
    result = invokeServices(trx, services);
  }

  if (result)
  {
    if (taxes)
    {
      if (trx.isNoMatch())
      {
        trx.getRequest()->lowFareRequested() = 'Y';
        result = invokeServices(trx, TAXES);
        trx.getRequest()->lowFareRequested() = 'N';
      }
      else
      {
        result = invokeServices(trx, TAXES);
      }
    }

    if (lowestFare && trx.isNoMatch())
      AltPricingUtil::keepLowestSolutions(trx, *trx.itin().front());
    AltPricingUtil::finalProcessing(trx);

    if (result && baggage)
      result = invokeServices(trx, FREE_BAG);

    if (result && fareCalc)
      result = invokeServices(trx, FARE_CALC);
  }

  if (diagChanged)
    diag.diagnosticType() = orgDiagType;

  return result;
}

//----------------------------------------------------------------------------
// process(StatusTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(StatusTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS STATUS");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processStatusCheckOnly()");

  bool rc = validateServicePointers(ALL_SERVICES);

  statusLine(trx, rc);

  return invokeServices(trx, ALL_SERVICES);
}

//----------------------------------------------------------------------------
// process(CurrencyTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(CurrencyTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS CURRENCY");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processCurrencyConversion()");

  return invokeServices(trx, CURRENCY);
}

bool
TransactionOrchestrator::process(TicketingCxrTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS TICKETING_CXR");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered TicketingCxr process()");
  return invokeServices(trx, TICKETING_CXR);
}

bool
TransactionOrchestrator::process(TicketingCxrDisplayTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS TICKETING_CXR DISPLAY");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered TicketingCxrDisplay process()");
  return invokeServices(trx, TICKETING_CXR_DISPLAY);
}

//----------------------------------------------------------------------------
// process(MileageTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(MileageTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS MILEAGE");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processMileageInfo()");

  return invokeServices(trx, MILEAGE);
}

//----------------------------------------------------------------------------
// process(FareDisplayTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(FareDisplayTrx& trx)
{
  TSELatencyData latency(trx, " TO FARE DISPLAY PROCESS ");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process FareDisplayTrx()");
  std::string& inclusionCode = trx.getRequest()->inclusionCode(); // lint !e530

  if (trx.getRequest()->diagnosticNumber() == Diagnostic198)
    return invokeServices(trx, ITIN_ANALYZER | INTERNAL);

  if (trx.isFQ() && trx.getRequest()->diagnosticNumber() == DIAG_999_ID)
  {
    return true;
  }
  if ((inclusionCode == FD_ADDON) || (trx.getRequest()->diagnosticNumber() == DIAG_216_ID) ||
      (trx.getRequest()->diagnosticNumber() == DIAG_220_ID))
  {
    return invokeServices(trx, ITIN_ANALYZER | FARE_DISPLAY);
  }
  else if (trx.getRequest()->diagnosticNumber() == DIAG_212_ID ||
           trx.getRequest()->diagnosticNumber() == DIAG_207_ID ||
           trx.getRequest()->diagnosticNumber() == DIAG_209_ID)
  {
    return invokeServices(trx, ITIN_ANALYZER);
  }
  else if (trx.getRequest()->requestType() == MP_REQUEST &&
           FareDisplayUtil::determineMPType(trx) == NO_MARKET_MP)
  {
    return invokeServices(trx, FARE_DISPLAY);
  }

  return invokeServices(trx,
                        ITIN_ANALYZER | FARE_COLLECTOR | FARE_SELECTOR | FARE_VALIDATOR | S8_BRAND |
                            TAXES | FARE_DISPLAY);
}
//----------------------------------------------------------------------------
// process(MetricsTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(MetricsTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS METRICS");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processMetrics()");

  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "TO Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_CURRENCY);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_METRICS);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_MILEAGE);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_PRICING);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_SHOPPING);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_PROCESS_STATUS);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_FARES_C);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_FARES_V);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_PRICING);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_SHOPPING);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_ITIN);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_TAX);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_FARE_CALC);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_CURRENCY);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_MILEAGE);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_INTERNAL);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_SERVICE_FEES);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_TICKETING_FEES);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_S8_BRAND);
  MetricsUtil::lineItem(oss, MetricsUtil::TO_SVC_DECODE);

  return invokeServices(trx, ALL_SERVICES);
}

//----------------------------------------------------------------------------
// process(RepricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(RepricingTrx& trx)
{
  // @todo add metrics for repricing
  //  TSELatencyData latency( trx, "TO PROCESS CURRENCY");

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(RepricingTrx)");

  uint64_t services = ITIN_ANALYZER | FARE_COLLECTOR;

  if (trx.validate())
    services = services | FARE_VALIDATOR;

  return invokeServices(trx, services);
}

//----------------------------------------------------------------------------
// process(PricingDetailTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(PricingDetailTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic870)
      DiagnosticUtil::displayObFeesNotRequested(trx);

    return true;
  }

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(PricingDetailTrx)");
  return process(dynamic_cast<TktFeesPricingTrx&>(trx));
}

bool
TransactionOrchestrator::process(AltPricingDetailObFeesTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
    return true;

  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(AltPricingDetailObFeesTrx)");
  return process(dynamic_cast<TktFeesPricingTrx&>(trx));
}

//----------------------------------------------------------------------------
// process(TaxTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(TaxTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processTaxes()");

  if (trx.requestType() == DISPLAY_REQUEST || trx.requestType() == ATPCO_DISPLAY_REQUEST)
  {
    return invokeServices(trx, TAX_DISPLAY);
  }
  else if (trx.requestType() == PFC_DISPLAY_REQUEST)
  {
    return invokeServices(trx, PFC_DISPLAY);
  }
  else if (trx.requestType() == TAX_INFO_REQUEST)
  {
    return invokeServices(trx, TAX_INFO);
  }
  else
  {
    return invokeServices(trx, ITIN_ANALYZER | TAXES);
  }
}

bool
TransactionOrchestrator::process(DecodeTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered processDecode()");

  return invokeServices(trx, DECODE);
}

//----------------------------------------------------------------------------
// process(AltPricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(AltPricingTrx& trx)
{
  TSELatencyData latency(trx, "TO PROCESS PRICING");
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(AltPricingTrx)");

  // If there is AccompaniedRestrictionData populated this is a WTFR not a WPA or WPn
  if (trx.isPriceSelectionEntry())
  {
    return invokeServices(trx, PRICING);
  }

  // @TODO: Check for the XM option once support for it is coded in PSS
  //
  bool isXMrequest = AltPricingUtil::isXMrequest(trx);
  AltPricingUtil::prepareToProcess(trx);

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  uint64_t processServiceBits(0); // First pass for WPA transaction
  uint64_t finalServiceBits(0); // Final pass for WPA Match/Partial Match
  uint64_t reprocessServiceBits(0); // Reprocessing for WPA No Match

  bool ret = false;
  bool isAltPricingDiag = false;

  if ((diagType > ALT_PRICING_DIAG_RANGE_BEGIN) && (diagType <= ALT_PRICING_DIAG_RANGE_END))
  {
    isAltPricingDiag = true;

    diagType = (DiagnosticTypes)(diagType - ALT_PRICING_DIAG_RANGE_BEGIN);

    // diag.diagnosticType() = DiagnosticNone;
    diag.deActivate();
  }

  if (diagType == DiagnosticNone || !diag.isActive())
  {
    processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING);

    reprocessServiceBits = (FARE_VALIDATOR | TAXES | PRICING);

    finalServiceBits = FARE_CALC | FREE_BAG;
  }
  else
  {
    fixLegacyDiagnostic(diagType);

    if (!isAltPricingDiag)
    {
      diag.diagnosticType() = diagType;
    }

    if (trx.displayOnly())
    {
      if (diagType == AllFareDiagnostic)
      {
        processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR);
      }
      else
      {
        processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR);
      }
    }

    if ((diagType >= FARES_DIAG_RANGE_BEGIN) && (diagType <= FARES_DIAG_RANGE_END))
    {
      // Fares diagnostics
      processServiceBits = ITIN_ANALYZER | FARE_COLLECTOR;
    }
    else if ((diagType >= PRICING_DIAG_RANGE_BEGIN) && (diagType <= PRICING_DIAG_RANGE_END))
    {
      // Pricing diagnostics
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING);
    }
    else if ((diagType >= TAXES_DIAG_RANGE_BEGIN) && (diagType <= TAXES_DIAG_RANGE_END))
    {
      // Taxes diagnostics
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES);
    }
    else if ((diagType >= FARE_CALC_DIAG_RANGE_BEGIN) && (diagType <= FARE_CALC_DIAG_RANGE_END))
    {
      // FareCalc diagnostics
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES);
      finalServiceBits = FARE_CALC | FREE_BAG;
    }
    else if ((diagType >= INTERNAL_DIAG_RANGE_BEGIN) && (diagType <= INTERNAL_DIAG_RANGE_END))
    {
      // Internal diagnostics
      if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
          trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
      {
        processServiceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | SERVICE_FEES |
                             ((trx.getTrxType() != PricingTrx::MIP_TRX) ? FREE_BAG : 0);
      }
      else
        processServiceBits = (ITIN_ANALYZER | INTERNAL);
    }
    else if ((diagType >= RULES_DIAG_RANGE_BEGIN) && (diagType <= RULES_DIAG_RANGE_END))
    {
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING);
    }
    else if ((diagType >= BOOKING_CODE_DIAG_RANGE_BEGIN) &&
             (diagType <= BOOKING_CODE_DIAG_RANGE_END))
    {
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR);
    }
    else if ((diagType >= ROUTING_DIAG_RANGE_BEGIN) && (diagType <= ROUTING_DIAG_RANGE_END))
    {
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR);
    }
    else if ((diagType >= RULECNTL_DIAG_RANGE_BEGIN) && (diagType <= RULECNTL_DIAG_RANGE_END))
    {
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING);
    }
    else if ((diagType >= MIPS_DIAG_RANGE_BEGIN) && (diagType <= MIPS_DIAG_RANGE_END))
    {
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING);
      finalServiceBits = FARE_CALC;
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "TransactionOrchestrator() - processPricing() unsupported diagnostic "
                        << diagType);
      throw tse::ErrorResponseException(ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY);
    }

    // ** Note **
    //
    // This section is only for exceptions to the general range based
    //  logic below
    // Do not any new diagnostics to this switch statement unless its
    //  absolutely necessary

    switch (diagType)
    {
    case Diagnostic853:
      processServiceBits = ITIN_ANALYZER;
      finalServiceBits = FARE_CALC;
      break;

    case AllFareDiagnostic: // Fallthrough on purpose
    case Diagnostic550:
      processServiceBits = ITIN_ANALYZER | FARE_COLLECTOR;
      break;

    case Diagnostic405: // Fallthrough on purpose
    case Diagnostic416: // Fallthrough on purpose
    case Diagnostic430:
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING);
      break;

    case Diagnostic413: // Fallthrough on purpose
    case Diagnostic420:
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING);
      break;

    case Diagnostic319:
      processServiceBits = ITIN_ANALYZER | FARE_COLLECTOR;
      break;

    case Diagnostic863:
    case Diagnostic868:
      processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR);
      break;
    default: // Handled below
      break;
    }
  }

  if (isAltPricingDiag && !isXMrequest)
  {
    if (diagType != DiagnosticNone) // to make sure
    {
      diag.diagnosticType() = diagType;
      diag.activate();
    }

    if (processServiceBits & FARE_VALIDATOR)
    {
      reprocessServiceBits |= FARE_VALIDATOR;
    }
    if (processServiceBits & TAXES)
    {
      reprocessServiceBits |= TAXES;
    }
    if (processServiceBits & PRICING)
    {
      reprocessServiceBits |= PRICING;
    }

    processServiceBits = (ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | TAXES | PRICING);
  }

  bool noCombinableFares = false;
  ErrorResponseException ex(ErrorResponseException::NO_ERROR);

  if (trx.altTrxType() == PricingTrx::WPA)
  {
    try
    {
      ret = invokeServices(trx, processServiceBits);
    }
    catch (ErrorResponseException& e)
    {
      if (e.code() == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
          e.code() == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
          e.code() == ErrorResponseException::NO_FARE_FOR_CLASS ||
          e.code() == ErrorResponseException::NO_FARES_RBD_CARRIER ||
          e.code() == ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS)
      {
        if (!isXMrequest)
        {
          noCombinableFares = true;
          ex = e;
        }
        else
        {
          throw;
        }
      }
      else
      {
        throw;
      }
    }
    catch (...)
    {
      throw;
    }
  }
  else
  {
    // We got here by mean of a failed WP attemp, setup to do WP-No-Match
    if (!isXMrequest)
    {
      noCombinableFares = true;
    }
  }

  if (noCombinableFares)
  {
    if (!AltPricingUtil::needToReprocess(trx))
    {
      if (ex.code() != ErrorResponseException::NO_ERROR)
        throw ex;
      else
        return false;
    }

    if (!AltPricingUtil::prepareToReprocess(trx))
    {
      if (ex.code() != ErrorResponseException::NO_ERROR)
        throw ex;
      else
        return false;
    }

    if (isAltPricingDiag)
    {
      diag.diagnosticType() = diagType;
    }

    ret = invokeServices(trx, reprocessServiceBits);

    if (!AltPricingUtil::finalProcessing(trx))
    {
      if (ex.code() != ErrorResponseException::NO_ERROR)
        throw ex;
      else
        return false;
    }

    if (finalServiceBits)
    {
      ret = invokeServices(trx, finalServiceBits);
    }
  }
  else
  {
    if (isAltPricingDiag)
    {
      LOG4CXX_DEBUG(logger,
                    "TransactionOrchestrator() - did not reprocess for Alt Pricing. No data to "
                    "display for diagnostic "
                        << diagType);
      throw ErrorResponseException(ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY);
    }

    if (!AltPricingUtil::finalProcessing(trx))
    {
      throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
    }
    addTktServiceIfNeeded(trx, finalServiceBits);
    ret = invokeServices(trx, finalServiceBits);
  }

  return ret;
}

//----------------------------------------------------------------------------
// process(AncillaryPricingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(AncillaryPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(AncillaryPricingTrx)");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
  {
  }
  else if ((diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END) ||
           diagType == UpperBound)
  {
    return invokeServices(trx, ITIN_ANALYZER | INTERNAL | TAXES);
  }

  const AncRequest* req = static_cast<AncRequest*>(trx.getRequest());

  if (trx.activationFlags().isAB240())
  {
    // Run SERVICE_FEES and TAXES only if Ancillary data is requested
    if (trx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData) &&
        trx.isBaggageRequest())
      return invokeServices(trx, ITIN_ANALYZER | SERVICE_FEES | FREE_BAG | TAXES);
    else if (trx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData))
      return invokeServices(trx, ITIN_ANALYZER | SERVICE_FEES | TAXES);
    else if (trx.isBaggageRequest())
      return invokeServices(trx, ITIN_ANALYZER | FREE_BAG | TAXES);
    else
      return false;
  }

  if ((req->majorSchemaVersion() >= 2) &&
      (trx.billing()->requestPath() == ACS_PO_ATSE_PATH || req->isWPBGRequest() ||
       trx.billing()->actionCode().substr(0, 5) == "MISC6"))
  {
    std::vector<ServiceGroup> groupCodes;
    trx.getOptions()->getGroupCodes(trx.getOptions()->serviceGroupsVec(), groupCodes);

    if (groupCodes.empty() ||
        (std::find_if(groupCodes.begin(),
                      groupCodes.end(),
                      ServiceFeeUtil::checkServiceGroupForAcs) != groupCodes.end()))
    {
      if (req->isWPBGRequest())
        return invokeServices(trx, ITIN_ANALYZER | FREE_BAG);

      return invokeServices(trx, ITIN_ANALYZER | SERVICE_FEES | FREE_BAG | TAXES);
    }
  }

  return invokeServices(trx, ITIN_ANALYZER | SERVICE_FEES | TAXES);
}

bool
TransactionOrchestrator::process(BaggageTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(BaggageTrx)");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
    return invokeServices(trx, ITIN_ANALYZER | FREE_BAG);

  if ((diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END) ||
      diagType == UpperBound)
  {
    return invokeServices(trx, ITIN_ANALYZER | INTERNAL);
  }
  return invokeServices(trx, ITIN_ANALYZER | FREE_BAG);
}

//----------------------------------------------------------------------------
// process(TktFeesPricingTrx
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(TktFeesPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(TktFeesPricingTrx)");

  Diagnostic& diag = trx.diagnostic();
  DiagnosticTypes diagType = diag.diagnosticType();

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
    return invokeServices(trx, ITIN_ANALYZER | TICKETING_FEES);

  if (diagType >= INTERNAL_DIAG_RANGE_BEGIN && diagType <= INTERNAL_DIAG_RANGE_END)
  {
    return invokeServices(trx, ITIN_ANALYZER | INTERNAL);
  }

  uint64_t serviceBits = ITIN_ANALYZER | TICKETING_FEES;
  AltPricingTrxData* altTrx = dynamic_cast<AltPricingTrxData*>(&trx);
  if (altTrx && altTrx->isPriceSelectionEntry())
  {
    serviceBits |= PRICING;
  }
  return invokeServices(trx, serviceBits);
}

//----------------------------------------------------------------------------
// process(BrandingTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(BrandingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(BrandingTrx)");

  // To enable diagnostics 100 up to 199, we
  // must turn on INTERNAL service in addition to ITIN_ANALYZER
  const Diagnostic& diag = trx.diagnostic();
  const DiagnosticTypes diagType = diag.diagnosticType();
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx) && diagType == Diagnostic187 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::RBD_ALL) != "LRBD")
    return invokeServices(trx, ITIN_ANALYZER);

  if ((diagType != DiagnosticNone) &&
      ((diagType >= INTERNAL_DIAG_RANGE_BEGIN) && (diagType <= INTERNAL_DIAG_RANGE_END)))
  {
    return invokeServices(trx, ITIN_ANALYZER | INTERNAL);
  }

  return invokeServices(trx, ITIN_ANALYZER);
}

//----------------------------------------------------------------------------
// process(SettlementTypesTrx)
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::process(SettlementTypesTrx& trx)
{
  LOG4CXX_DEBUG(logger, "TransactionOrchestrator() - Entered process(SettlementTypesTrx)");

  // We need to determine only settlement plans
  ValidatingCxrUtil::determineCountrySettlementPlan(trx, nullptr, nullptr);

  return true;
}

//----------------------------------------------------------------------------
// validPtr()
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::validPtr(const std::string& name, Service*& service)
{
  if (name.empty())
    return false;

  if (service != nullptr)
    return true; // Success

  service = _server.service(name);

  if (service == nullptr)
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - No valid '" << name << "' service");
    return false; // Failure
  }

  return true;
}

//----------------------------------------------------------------------------
// validateServicePointers()
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::validateServicePointers(const uint64_t& serviceBits)
{
  if ((serviceBits & ITIN_ANALYZER) && !validItinAnalyzerPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid ITIN_ANALYZER service");
    return false;
  }

  if ((serviceBits & FARE_COLLECTOR) && !validFaresCollectionPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FARE_COLLECTOR service");
    return false;
  }

  if ((serviceBits & FARE_VALIDATOR) && !validFaresValidationPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FARE_VALIDATOR service");
    return false;
  }

  if ((serviceBits & TAXES) && !validTaxPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid TAXES service");
    return false;
  }

  if ((serviceBits & TAX_DISPLAY) && !validTaxPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid TAX DISPLAY service");
    return false;
  }

  if ((serviceBits & PFC_DISPLAY) && !validTaxPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid PFC DISPLAY service");
    return false;
  }

  if ((serviceBits & TAX_INFO) && !validTaxPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid TAX INFO service");
    return false;
  }

  if ((serviceBits & PRICING) && !validPricingPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid PRICING service");
    return false;
  }

  if ((serviceBits & FARE_CALC) && !validFareCalcPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FARE_CALC service");
    return false;
  }

  if ((serviceBits & CURRENCY) && !validCurrencyPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid CURRENCY service");
    return false;
  }

  if ((serviceBits & MILEAGE) && !validMileagePtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid MILEAGE service");
    return false;
  }

  if ((serviceBits & INTERNAL) && !validInternalPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid INTERNAL service");
    return false;
  }

  if ((serviceBits & SHOPPING) && !validShoppingPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid SHOPPING service");
    return false;
  }

  if ((serviceBits & FARE_DISPLAY) && !validFareDisplayPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FARE_DISPLAY service");
    return false;
  }

  if ((serviceBits & FARE_SELECTOR) && !validFareSelectorPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FARE_SELECTOR service");
    return false;
  }

  if ((serviceBits & REX_FARE_SELECTOR) && !validRexFareSelectorPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid REX_FARE_SELECTOR service");
    return false;
  }

  if ((serviceBits & SERVICE_FEES) && !validServiceFeesPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid SERVICE FEES service");
    return false;
  }

  if ((serviceBits & TICKETING_FEES) && !validTicketingFeesPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid TICKETING FEES service");
    return false;
  }

  if ((serviceBits & S8_BRAND) && !validS8BrandPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid S8 Brand service");
    return false;
  }

  if ((serviceBits & TICKETING_CXR) && !validTicketingCxrPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid Ticketing Carrier service");
    return false;
  }

  if ((serviceBits & TICKETING_CXR_DISPLAY) && !validTicketingCxrDispPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid Ticketing Carrier Display service");
    return false;
  }

  if ((serviceBits & FREE_BAG) && !validFreeBagPtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid FREE BAGGAGE service");
    return false;
  }

  if ((serviceBits & DECODE) && !validDecodePtr())
  {
    LOG4CXX_ERROR(logger, "TransactionOrchestrator() - no valid DECODE service");
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// invokeServices()
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::invokeServices(Trx& trx, const uint64_t& serviceBits)
{
  bool continueOnFailure = (serviceBits & CONTINUE_ON_FAILURE);

  // For the transactions that dont have continueOnFailure set we want to validate
  // that we have all the necessary service pointers up front, so we dont get
  // halfway through the transaction and they bail because we cant call something
  if (!continueOnFailure)
  {
    if (!validateServicePointers(serviceBits))
    {
      LOG4CXX_ERROR(logger,
                    "TransactionOrchestrator() - Unable to process transaction, a "
                    "required service pointer was missing");
      return false;
    }
  }

  // for logging purpose only
  bool logPath = false;
  std::string trxID;
  std::ostringstream tooss;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
  if (pricingTrx != nullptr)
  {
    std::ostringstream oss;
    oss << "TXN:" << TrxUtil::getTransId(*pricingTrx);
    trxID = oss.str();

    // TODO Add Constant!
    logPath = pricingTrx->diagnostic().diagParamMapItemPresent(Diagnostic::TO_ROUTING) &&
              (pricingTrx->diagnostic().diagnosticType() != DiagnosticNone);
  }
  else
  {
    std::ostringstream oss;
    oss << "TXN:" << trx.transactionId();
    trxID = oss.str();
  }

  ServiceBundle sb(continueOnFailure, trx, logPath, tooss, trxID);

  TOPATH_INFO(logPath, tooss, "\n \n** TRANSACTION ORCHESTRATOR TRX ROUTING **\n \n");

  if (!invokeService(sb,
                     serviceBits & ITIN_ANALYZER,
                     "ITIN ANALYZER SERVICE",
                     _itinAnalyzerServiceName,
                     _itinAnalyzerService,
                     TseSrvStats::recordItinAnalyzerServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FARE_COLLECTOR,
                     "FARE COLLECTOR SERVICE",
                     _faresCollectionServiceName,
                     _faresCollectionService,
                     TseSrvStats::recordFaresCollectionServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FARE_SELECTOR,
                     "FARE SELECTOR SERVICE",
                     _fareSelectorServiceName,
                     _fareSelectorService,
                     TseSrvStats::recordFareSelectorServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FARE_VALIDATOR,
                     "FARE VALIDATOR SERVICE",
                     _faresValidationServiceName,
                     _faresValidationService,
                     TseSrvStats::recordFaresValidationServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & S8_BRAND,
                     "S8 BRAND SERVICE",
                     _s8BrandServiceName,
                     _s8BrandService,
                     TseSrvStats::recordS8BrandServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & REX_FARE_SELECTOR,
                     "VOLUNTARY CHANGES - FARE SELECTOR SERVICE ",
                     _rexFareSelectorServiceName,
                     _rexFareSelectorService,
                     TseSrvStats::recordVoluntaryChangesRetrieverServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & PRICING,
                     "PRICING SERVICE",
                     _pricingServiceName,
                     _pricingService,
                     TseSrvStats::recordPricingServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & SERVICE_FEES,
                     "SERVICE FEES SERVICE",
                     _serviceFeesServiceName,
                     _serviceFeesService,
                     TseSrvStats::recordServiceFeesServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FREE_BAG,
                     "FREE BAGGAGE SERVICE",
                     _freeBagServiceName,
                     _freeBagService,
                     TseSrvStats::recordBaggageServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TAXES,
                     "TAX SERVICE",
                     _taxServiceName,
                     _taxService,
                     TseSrvStats::recordTaxServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TICKETING_FEES,
                     "TICKETING FEES SERVICE",
                     _ticketingFeesServiceName,
                     _ticketingFeesService,
                     TseSrvStats::recordTicketingFeesServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TAX_DISPLAY,
                     "TAX DISPLAY SERVICE",
                     _taxServiceName,
                     _taxService,
                     TseSrvStats::recordTaxServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & PFC_DISPLAY,
                     "PFC DISPLAY SERVICE",
                     _taxServiceName,
                     _taxService,
                     TseSrvStats::recordTaxServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TAX_INFO,
                     "TAX INFO SERVICE",
                     _taxServiceName,
                     _taxService,
                     TseSrvStats::recordTaxServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FARE_CALC,
                     "FARE CALC SERVICE",
                     _fareCalcServiceName,
                     _fareCalcService,
                     TseSrvStats::recordFareCalcServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & CURRENCY,
                     "CURRENCY SERVICE",
                     _currencyServiceName,
                     _currencyService,
                     TseSrvStats::recordCurrencyServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & MILEAGE,
                     "MILEAGE SERVICE",
                     _mileageServiceName,
                     _mileageService,
                     TseSrvStats::recordMileageServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & DECODE,
                     "DECODE SERVICE",
                     _decodeServiceName,
                     _decodeService,
                     TseSrvStats::recordDecodeServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & INTERNAL,
                     "INTERNAL SERVICE",
                     _internalServiceName,
                     _internalService,
                     TseSrvStats::recordInternalServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & SHOPPING,
                     "SHOPPING SERVICE",
                     _shoppingServiceName,
                     _shoppingService,
                     TseSrvStats::recordShoppingServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & FARE_DISPLAY,
                     "FARE DISPLAY SERVICE",
                     _fareDisplayServiceName,
                     _fareDisplayService,
                     TseSrvStats::recordFareDisplayServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TICKETING_CXR,
                     "TICKETING CXR SERVICE",
                     _ticketingCxrServiceName,
                     _ticketingCxrService,
                     TseSrvStats::recordTicketingCxrServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  if (!invokeService(sb,
                     serviceBits & TICKETING_CXR_DISPLAY,
                     "TICKETING CXR DISPLAY SERVICE",
                     _ticketingCxrDispServiceName,
                     _ticketingCxrDispService,
                     TseSrvStats::recordTicketingCxrDispServiceCall))
  {
    TOPATH_PRINT(logPath, tooss, pricingTrx);
    return false;
  }

  TOPATH_PRINT(logPath, tooss, pricingTrx);
  return true;
}

//----------------------------------------------------------------------------
// invokeService()
//---------------------------------------------------------------------------
bool
TransactionOrchestrator::invokeService(const ServiceBundle& sb,
                                       const uint64_t bitPresent,
                                       const char* displayName,
                                       const std::string& serviceName,
                                       Service*& service,
                                       to::StatFnc* statFnc)
{
  if (fallback::reworkTrxAborter(&sb._trx))
    checkTrxAborted(sb._trx);
  else
    sb._trx.checkTrxAborted();

  if (!bitPresent)
    return true; // Nothing to do this service is off

  // We failed, return false if we need to stop
  if (!validPtr(serviceName, service))
  {
    LOG4CXX_ERROR(logger, "Unable to resolve service '" << serviceName << "'");
    return sb._continueOnFailure;
  }

  // We cant work without these pointers
  if ((displayName == nullptr) || (service == nullptr))
  {
    LOG4CXX_ERROR(logger, "Null displayName or service pointer passed to invokeService()");
    return sb._continueOnFailure;
  }

  TOPATH_INFO(sb._logPath, sb._tooss, displayName);
  LOG4CXX_INFO(logger, sb._trxID << " - Calling " << displayName);

  bool rc = false;
  {
    CurrentServiceSetter serviceSetter(sb._trx, service);
    TOLatencyData latency(sb._trx, displayName, statFnc, service->getActiveThreads());
    rc = sb._trx.process(*service);
  }

  LOG4CXX_INFO(logger, sb._trxID << " - " << displayName << " returned " << rc);

  if (LIKELY(rc))
    return rc;
  else
  {
    return sb._continueOnFailure;
  }
}

bool
TransactionOrchestrator::filterAndInvokeServices(Trx& trx,
                                                 const uint64_t& serviceBits,
                                                 const uint64_t& validServiceBits)
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
  if (pricingTrx != nullptr && (pricingTrx->getTrxType() == PricingTrx::MIP_TRX) &&
      pricingTrx->snapRequest())
  {
    return snapProcessingPath(*pricingTrx, serviceBits, validServiceBits);
  }

  return invokeServices(trx, serviceBits & validServiceBits);
}

// ******************************** WN SNAP  ******************************** //
bool
TransactionOrchestrator::snapProcessingPath(PricingTrx& trx,
                                            const uint64_t& serviceBits,
                                            const uint64_t& validServiceBits)
{
  bool processedPath1 = false;
  bool processedPath2 = false;

  buildSubItinVectors(trx);

  const uint64_t processingBits = serviceBits & validServiceBits;
  uint64_t finalServiceBits = const_cast<uint64_t&>(processingBits);
  finalServiceBits = finalServiceBits & ~FARE_CALC;

  if (trx.subItinVecFirstCxr().size())
  {
    trx.itin().swap(trx.subItinVecFirstCxr());

    AirSeg* airSeg = dynamic_cast<AirSeg*>((trx.itin().front())->travelSeg().front());

    if ((airSeg != nullptr) && ((trx.diagnostic().diagnosticType() != Diagnostic983) &&
                                !(trx.diagnostic().diagnosticType() >= FARE_CALC_DIAG_RANGE_BEGIN &&
                                  trx.diagnostic().diagnosticType() <= FARE_CALC_DIAG_RANGE_END)))
    {
      printDiagHeader(trx, airSeg->carrier());
    }

    processedPath1 = invokeServices(trx, finalServiceBits);

    trx.itin().swap(trx.subItinVecFirstCxr()); // back to original order
  }
  else
  {
    LOG4CXX_ERROR(logger, "WN_SNAP trx.subItinVecFirstCxr() empty!");
  }

  if (trx.subItinVecSecondCxr().size())
  {
    trx.itin().swap(trx.subItinVecSecondCxr());

    AirSeg* airSeg = dynamic_cast<AirSeg*>((trx.itin().front())->travelSeg().front());

    if ((airSeg != nullptr) && ((trx.diagnostic().diagnosticType() != Diagnostic983) &&
                                !(trx.diagnostic().diagnosticType() >= FARE_CALC_DIAG_RANGE_BEGIN &&
                                  trx.diagnostic().diagnosticType() <= FARE_CALC_DIAG_RANGE_END)))
    {
      printDiagHeader(trx, airSeg->carrier());
    }

    processedPath1 = invokeServices(trx, finalServiceBits);

    trx.itin().swap(trx.subItinVecSecondCxr()); // back to original order
  }
  else
  {
    LOG4CXX_ERROR(logger, "WN_SNAP trx.subItinVecSecondCxr() empty!");
  }

  if (serviceBits & FARE_CALC)
  {
    // Run Fare Calc Service for each direction
    WnSnapUtil::splitItinsByDirection(trx);
    WnSnapUtil::createFarePathsForOutIn(trx);

    if (!trx.subItinVecOutbound().empty())
    {
      trx.itin().swap(trx.subItinVecOutbound());

      if (trx.diagnostic().diagnosticType() >= FARE_CALC_DIAG_RANGE_BEGIN &&
          trx.diagnostic().diagnosticType() <= FARE_CALC_DIAG_RANGE_END)
      {
        printDiagHeader(trx, "OUTBOUND");
      }

      invokeServices(trx, FARE_CALC);

      trx.itin().swap(trx.subItinVecOutbound()); // back to original order
    }

    if (!trx.subItinVecInbound().empty())
    {
      trx.itin().swap(trx.subItinVecInbound());

      if (trx.diagnostic().diagnosticType() >= FARE_CALC_DIAG_RANGE_BEGIN &&
          trx.diagnostic().diagnosticType() <= FARE_CALC_DIAG_RANGE_END)
      {
        printDiagHeader(trx, "INBOUND");
      }

      invokeServices(trx, FARE_CALC);

      trx.itin().swap(trx.subItinVecInbound()); // back to original order
    }
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic983)
  {
    DCFactory* factory = DCFactory::instance();
    Diag983Collector* diagPtr = dynamic_cast<Diag983Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic983);
    diagPtr->activate();

    (*diagPtr) << trx;

    diagPtr->flushMsg();
  }

  if (processedPath1 || processedPath2)
    return true;
  else
    return false;
}

void
TransactionOrchestrator::buildSubItinVectors(PricingTrx& trx)
{
  std::vector<Itin*>::iterator itinIter = trx.itin().begin();
  std::vector<Itin*>::iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);

    CarrierCode firstCarrier = "";
    CarrierCode secondCarrier = "";

    Itin* subItinFirstCxr = nullptr;
    Itin* subItinSecondCxr = nullptr;

    trx.dataHandle().get(subItinFirstCxr);
    trx.dataHandle().get(subItinSecondCxr);

    std::vector<TravelSeg*>::iterator travelSegIter = itin->travelSeg().begin();
    std::vector<TravelSeg*>::iterator travelSegIterEnd = itin->travelSeg().end();

    for (; travelSegIter != travelSegIterEnd; ++travelSegIter)
    {
      TravelSeg* travelSeg = (*travelSegIter);
      AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSeg);
      int carrierNo = 0;

      if (airSeg != nullptr)
      {
        carrierNo = determineCarrierNo(firstCarrier, secondCarrier, airSeg);

        switch (carrierNo)
        {
        case 1:
          subItinFirstCxr->travelSeg().push_back(travelSeg);
          break;

        case 2:
          subItinSecondCxr->travelSeg().push_back(travelSeg);
          break;

        default:
          LOG4CXX_ERROR(logger, "WN_SNAP more than 2 carriers!");
          break;
        }
      }
    }

    PricingTrx::SubItinValue subItinValue;

    if (subItinFirstCxr->travelSeg().size())
    {
      WnSnapUtil::addArunkSegments(trx, subItinFirstCxr);
      trx.subItinVecFirstCxr().push_back(subItinFirstCxr);
      subItinValue.firstCxrItin = subItinFirstCxr;
    }

    if (subItinSecondCxr->travelSeg().size())
    {
      WnSnapUtil::addArunkSegments(trx, subItinSecondCxr);
      trx.subItinVecSecondCxr().push_back(subItinSecondCxr);
      subItinValue.secondCxrItin = subItinSecondCxr;
    }

    trx.primeSubItinMap()[itin] = subItinValue;
  }
}

int
TransactionOrchestrator::determineCarrierNo(CarrierCode& firstCarrier,
                                            CarrierCode& secondCarrier,
                                            const AirSeg* airSeg)
{
  int carrierNo = 0;

  if (firstCarrier.empty())
  {
    firstCarrier = airSeg->carrier();
    carrierNo = 1;
  }
  else
  {
    if (airSeg->carrier() == firstCarrier)
    {
      carrierNo = 1;
    }
    else
    {
      if (secondCarrier.empty())
      {
        secondCarrier = airSeg->carrier();
        carrierNo = 2;
      }
      else
      {
        if (airSeg->carrier() == secondCarrier)
        {
          carrierNo = 2;
        }
      }
    }
  }

  return carrierNo;
}

void
TransactionOrchestrator::printDiagHeader(PricingTrx& trx, const std::string& str)
{
  if (trx.diagnostic().isActive() && (trx.diagnostic().diagnosticType() != DiagnosticNone))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector& dc = *(factory->create(trx));

    dc.activate();
    dc << std::endl;
    dc << "**********************************************************\n"
       << "* PROCESSING PATH OF " << str << " ITIN CONTENT\n"
       << "**********************************************************\n";
    dc << std::endl;
    dc.flushMsg();
  }
}

void
TransactionOrchestrator::getServiceBitsForDiag975(const Diagnostic& diag, uint64_t& serviceBits)
{
  const std::string& diagArg = diag.diagParamMapItem("ORC");

  if ("IAO" == diagArg)
  {
    serviceBits = ITIN_ANALYZER;
  }
  else if ("FCO" == diagArg)
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR;
  }
  else if ("FVO" == diagArg)
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR;
  }
  else if ("PO" == diagArg)
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING;
  }
  else if ("TAX" == diagArg)
  {
    serviceBits = ITIN_ANALYZER | FARE_COLLECTOR | FARE_VALIDATOR | PRICING | TAXES;
  }
}

void
TransactionOrchestrator::addTktServiceIfNeeded(const PricingTrx& trx, uint64_t& finalServiceBits)
    const
{
  if (trx.isSingleMatch() || !fallback::obFeesWpaOption1(&trx))
  {
    finalServiceBits |= TICKETING_FEES;
  }
}

// ******************************** WN SNAP  ******************************** //

static LoadableModuleRegister<Service, TransactionOrchestrator>
_("libTO.so");

} // end of tse
