//----------------------------------------------------------------------------
//
//  File:               TransactionOrchestrator.h
//  Description:        Service class for the Transaction Orchestrator
//  Created:            11/16/2003
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
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/Diagnostic.h"
#include "Service/Service.h"

#include <string>

namespace tse
{
class AirSeg;
class AltPricingDetailObFeesTrx;
class AltPricingTrx;
class AncillaryPricingTrx;
class BaggageTrx;
class CurrencyTrx;
class DecodeTrx;
class ErrorResponseException;
class ExchangePricingTrx;
class FareDisplayTrx;
class FlightFinderTrx;
class MetricsTrx;
class MileageTrx;
class MultiExchangeTrx;
class NoPNRPricingTrx;
class PricingDetailTrx;
class PricingTrx;
class RefundOrchestrator;
class RefundPricingTrx;
class RepricingTrx;
class RexBaseTrx;
class RexExchangeTrx;
class RexPricingTrx;
class RexShoppingTrx;
class ShoppingTrx;
class StatusTrx;
class StructuredRuleTrx;
class FrequentFlyerTrx;
class TaxTrx;
class TktFeesPricingTrx;
class TransactionOrchestrator;
class Trx;
class TseServer;
class TicketingCxrTrx;
class TicketingCxrDisplayTrx;

namespace to
{
typedef void
StatFnc(double elapsedTime, uint32_t activeThreads, Trx& trx);
}

class TransactionOrchestrator : public Service
{
  friend class RefundOrchestrator;
  friend class TransactionOrchestratorTest;
  friend class RefundOrchestratorTest;

private:
  enum
  { CONTINUE_ON_FAILURE = 1, // 0 0000 0000 0000 0001
    ITIN_ANALYZER = 2, // 0 0000 0000 0000 0010
    FARE_COLLECTOR = 4, // 0 0000 0000 0000 0100
    FARE_VALIDATOR = 8, // 0 0000 0000 0000 1000
    TAXES = 16, // 0 0000 0000 0001 0000
    PRICING = 32, // 0 0000 0000 0010 0000
    FARE_CALC = 64, // 0 0000 0000 0100 0000
    CURRENCY = 128, // 0 0000 0000 1000 0000
    MILEAGE = 256, // 0 0000 0001 0000 0000
    INTERNAL = 512, // 0 0000 0010 0000 0000
    SHOPPING = 1024, // 0 0000 0100 0000 0000
    FARE_DISPLAY = 2048, // 0 0000 1000 0000 0000
    FARE_SELECTOR = 4096, // 0 0001 0000 0000 0000
    REX_FARE_SELECTOR = 8192, // 0 0010 0000 0000 0000
    TAX_DISPLAY = 16384, // 0 0100 0000 0000 0000
    PFC_DISPLAY = 0x8000, // 0 1000 0000 0000 0000
    TAX_INFO = 0x10000, // 1 0000 0000 0000 0000
    FREE_BAG = 0x20000, // 10 0000 0000 0000 0000
    SERVICE_FEES = 0x40000, // 100 0000 0000 0000 0000
    //        ALL_SERVICES        = 0x7FFFF,
    TICKETING_FEES = 0x80000, // 00 1000 0000 0000 0000 0000
    S8_BRAND = 0x100000, // 01 0000 0000 0000 0000 0000
    TICKETING_CXR = 0x200000, // 10 0000 0000 0000 0000 0000
    TICKETING_CXR_DISPLAY = 0x400000, // 100 0000 0000 0000 0000 0000
    DECODE = 0x800000, // 1000 0000 0000 0000 0000 0000
    ALL_SERVICES = 0xFFFFFF };

  std::string _faresCollectionServiceName;
  std::string _faresValidationServiceName;
  std::string _pricingServiceName;
  std::string _shoppingServiceName;
  std::string _itinAnalyzerServiceName;
  std::string _taxServiceName;
  std::string _fareCalcServiceName;
  std::string _currencyServiceName;
  std::string _mileageServiceName;
  std::string _internalServiceName;
  std::string _fareDisplayServiceName;
  std::string _fareSelectorServiceName;
  std::string _rexFareSelectorServiceName;
  std::string _freeBagServiceName;
  std::string _serviceFeesServiceName;
  std::string _ticketingFeesServiceName;
  std::string _s8BrandServiceName;
  std::string _ticketingCxrServiceName;
  std::string _ticketingCxrDispServiceName;
  std::string _decodeServiceName;

  Service* _faresCollectionService = nullptr;
  Service* _faresValidationService = nullptr;
  Service* _pricingService = nullptr;
  Service* _shoppingService = nullptr;
  Service* _itinAnalyzerService = nullptr;
  Service* _taxService = nullptr;
  Service* _fareCalcService = nullptr;
  Service* _currencyService = nullptr;
  Service* _mileageService = nullptr;
  Service* _internalService = nullptr;
  Service* _fareDisplayService = nullptr;
  Service* _fareSelectorService = nullptr;
  Service* _rexFareSelectorService = nullptr;
  Service* _freeBagService = nullptr;
  Service* _serviceFeesService = nullptr;
  Service* _ticketingFeesService = nullptr;
  Service* _s8BrandService = nullptr;
  Service* _ticketingCxrService = nullptr;
  Service* _ticketingCxrDispService = nullptr;
  Service* _decodeService = nullptr;

  bool validPtr(const std::string& name, Service*& service);
  void getServiceBitsForDiag975(const Diagnostic& diag, uint64_t& serviceBits);

  bool validFaresCollectionPtr()
  {
    if (_faresCollectionService != nullptr)
      return true;

    return validPtr(_faresCollectionServiceName, _faresCollectionService);
  }

  bool validFaresValidationPtr()
  {
    if (_faresValidationService != nullptr)
      return true;

    return validPtr(_faresValidationServiceName, _faresValidationService);
  }

  bool validPricingPtr()
  {
    if (_pricingService != nullptr)
      return true;

    return validPtr(_pricingServiceName, _pricingService);
  }

  bool validShoppingPtr()
  {
    if (_shoppingService != nullptr)
      return true;

    return validPtr(_shoppingServiceName, _shoppingService);
  }

  bool validItinAnalyzerPtr()
  {
    if (_itinAnalyzerService != nullptr)
      return true;

    return validPtr(_itinAnalyzerServiceName, _itinAnalyzerService);
  }

  bool validTaxPtr()
  {
    if (_taxService != nullptr)
      return true;

    return validPtr(_taxServiceName, _taxService);
  }

  bool validFareCalcPtr()
  {
    if (_fareCalcService != nullptr)
      return true;

    return validPtr(_fareCalcServiceName, _fareCalcService);
  }

  bool validCurrencyPtr()
  {
    if (_currencyService != nullptr)
      return true;

    return validPtr(_currencyServiceName, _currencyService);
  }

  bool validMileagePtr()
  {
    if (_mileageService != nullptr)
      return true;

    return validPtr(_mileageServiceName, _mileageService);
  }

  bool validInternalPtr()
  {
    if (_internalService != nullptr)
      return true;

    return validPtr(_internalServiceName, _internalService);
  }

  bool validFareDisplayPtr()
  {
    if (_fareDisplayService != nullptr)
      return true;

    return validPtr(_fareDisplayServiceName, _fareDisplayService);
  }

  bool validFareSelectorPtr()
  {
    if (_fareSelectorService != nullptr)
      return true;

    return validPtr(_fareSelectorServiceName, _fareSelectorService);
  }

  bool validRexFareSelectorPtr()
  {
    if (_rexFareSelectorService != nullptr)
      return true;

    return validPtr(_rexFareSelectorServiceName, _rexFareSelectorService);
  }

  bool validFreeBagPtr()
  {
    if (_freeBagService != nullptr)
      return true;

    return validPtr(_freeBagServiceName, _freeBagService);
  }

  bool validServiceFeesPtr()
  {
    if (_serviceFeesService != nullptr)
      return true;

    return validPtr(_serviceFeesServiceName, _serviceFeesService);
  }

  bool validTicketingFeesPtr()
  {
    if (_ticketingFeesService != nullptr)
      return true;

    return validPtr(_ticketingFeesServiceName, _ticketingFeesService);
  }

  bool validS8BrandPtr()
  {
    if (_s8BrandService != nullptr)
      return true;

    return validPtr(_s8BrandServiceName, _s8BrandService);
  }

  bool validTicketingCxrPtr()
  {
    if (_ticketingCxrService != nullptr)
      return true;
    return validPtr(_ticketingCxrServiceName, _ticketingCxrService);
  }

  bool validTicketingCxrDispPtr()
  {
    if (_ticketingCxrDispService != nullptr)
      return true;
    return validPtr(_ticketingCxrDispServiceName, _ticketingCxrDispService);
  }

  bool validDecodePtr()
  {
    if (_decodeService != nullptr)
      return true;

    return validPtr(_decodeServiceName, _decodeService);
  }

  /**
   * Process a Currency Conversion transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(CurrencyTrx& trx) override;

  /**
   * Process a Metrics transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(MetricsTrx& trx) override;

  /**
   * Process a Mileage Info transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(MileageTrx& trx) override;

  /**
   * Process a Fare Display transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(FareDisplayTrx& trx) override;

  /**
   * Process a pricing transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(PricingTrx& trx) override;
  bool process(RexPricingTrx& trx) override;
  bool process(RexExchangeTrx& trx) override;
  bool process(RefundPricingTrx& trx) override;

  /**
   * Process No PNR Pricing transaction
   *
   * @param trx transaction to process
   *
   * @ return true if successful, false otherwie
   */
  bool process(NoPNRPricingTrx& trx) override;
  void changeMaxNoOptions(NoPNRPricingTrx& trx, int maxOptions);

  bool process(ExchangePricingTrx& trx) override;

  /**
   * Process a shopping transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(ShoppingTrx& trx) override;
  virtual bool processESV(ShoppingTrx& trx);

  bool process(RexShoppingTrx& trx) override;

  /**
   * Process a FlightFinder transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(FlightFinderTrx& trx) override;

  /**
   * Process a Status Check transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(StatusTrx& trx) override;

  /**
   * Process a Repricing transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(RepricingTrx& trx) override;

  /**
   * Process a ME exchange transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(MultiExchangeTrx& trx) override;

  /**
   * Process a Pricing Detail transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(PricingDetailTrx& trx) override;

  virtual bool process(AltPricingDetailObFeesTrx& trx);

  bool process(TaxTrx& trx) override;

  bool process(DecodeTrx& trx) override;

  /**
   * Process an AltPricingTrx object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(AltPricingTrx& trx) override;

  /**
   * Process an AncillaryPricingTrx object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(AncillaryPricingTrx& trx) override;
  bool process(BaggageTrx& trx) override;
  bool process(TktFeesPricingTrx& trx) override;
  bool process(BrandingTrx& trx) override;
  bool process(TicketingCxrTrx& trx) override;
  bool process(TicketingCxrDisplayTrx& trx) override;

  bool process(StructuredRuleTrx& trx) override;
  bool process(FrequentFlyerTrx& trx) override;

  bool process(SettlementTypesTrx& trx) override;

protected:
  struct ServiceBundle
  {
  public:
    ServiceBundle(bool continueOnFailure,
                  Trx& trx,
                  bool logPath,
                  std::ostream& tooss,
                  const std::string& trxID)
      : _continueOnFailure(continueOnFailure),
        _trx(trx),
        _logPath(logPath),
        _tooss(tooss),
        _trxID(trxID)
    {
    }

    bool _continueOnFailure;
    Trx& _trx;
    bool _logPath;
    std::ostream& _tooss;
    const std::string& _trxID;
  };

  // ******************************** WN SNAP  ******************************** //
  void buildSubItinVectors(PricingTrx& trx) override;
  int
  determineCarrierNo(CarrierCode& firstCarrier, CarrierCode& secondCarrier, const AirSeg* airSeg);
  bool snapProcessingPath(PricingTrx& trx,
                          const uint64_t& serviceBits,
                          const uint64_t& validServiceBits);
  void printDiagHeader(PricingTrx& trx, const std::string& str);
  // ******************************** WN SNAP  ******************************** //

  virtual bool invokeServices(Trx&, const uint64_t& serviceBits);
  inline bool invokeService(const ServiceBundle& sb,
                            const uint64_t bitPresent,
                            const char* displayName,
                            const std::string& serviceName,
                            Service*& service,
                            to::StatFnc* statFnc);

  bool
  filterAndInvokeServices(Trx& trx, const uint64_t& serviceBits, const uint64_t& validServiceBits);

  bool validateServicePointers(const uint64_t& serviceBits);

  bool process(PricingTrx& trx, const uint64_t& validServices);

  bool fixLegacyDiagnostic(DiagnosticTypes& diagType);
  uint64_t getServicesForDiagnostic(const DiagnosticTypes& diagType , const PricingTrx& trx) const;
  uint64_t getServicesForDiagnostic(PricingTrx& trx);

  enum DiagnosticQualifier
  {
    DQ_NONE = 0,
    DQ_ITEXC,
    DQ_ITALL,
    DQ_ITNEW,
    DQ_ITUFL,
    DQ_ITEFT
  };

  DiagnosticQualifier diagnosticQualifierProcess(const RexPricingTrx& trx) const;
  bool processExcItin(RexPricingTrx& trx, uint64_t services);
  bool processNewItin(RexPricingTrx& trx, uint64_t services);
  bool oldProcessNewItin(RexPricingTrx& trx, uint64_t services);
  bool processUflItin(RexPricingTrx& trx, uint64_t services);
  bool processEftItin(RexBaseTrx& trx, uint64_t services, bool diag);
  bool isCSOTrxApplicable(const RexPricingTrx& trx) const;

  static uint64_t getExcItinServices(const PricingTrx& trx);
  bool rexPricingMainProcess(RexPricingTrx& trx);

  bool isErrorEnforceRedirection(const ErrorResponseException& e) const;
  void addTktServiceIfNeeded(const PricingTrx& trx,
                             uint64_t& finalServiceBits) const;

  friend class CSOThreadTask;
  class RefundOrchestrator;

public:
  TransactionOrchestrator(const std::string& name, TseServer& server) : Service(name, server) {}

  bool initialize(int argc = 0, char* argv[] = nullptr) override;
  // bool portExchangeTask(RexPricingTrx& trx, uint64_t& services);
};
} /* end namespace tse */
