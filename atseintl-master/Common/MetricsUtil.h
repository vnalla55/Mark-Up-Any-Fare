//----------------------------------------------------------------------------
//
//  Description: Common Metrics formatting functions for ATSE shopping/pricing.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/thread/mutex.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace tse
{
class Trx;
class PricingTrx;
class MultiExchangeTrx;

class MetricsTimerImpl;

class MetricsTimer
{
public:
  MetricsTimer();
  MetricsTimer(bool isTopLevel, bool recordMetrics, bool recordTopLevelOnly);
  MetricsTimer(const MetricsTimer&) = delete;
  MetricsTimer& operator=(const MetricsTimer&) = delete;

  double getElapsedTime();
  double getCpuUserTime();
  double getCpuSystemTime();

  void start();
  void stop();

  bool enabled() const;

  friend class MetricsTimerImpl;
  friend class CommonTimerImplBase;

private:
  class Data
  {
  public:
    double getElapsedTime() const;
    double getCpuUserTime() const;
    double getCpuSystemTime() const;
    bool isCpuValid() const { return _cpuValid; }
    void setCpuValid(bool valid) { _cpuValid = valid; }

    friend class MetricsTimer;
    friend class MetricsTimerImpl;
    friend class CommonTimerImplBase;

  private:
    long _elapsedTimeStartSec = 0;
    long _elapsedTimeStartUsec = 0;
    long _elapsedTimeEndSec = 0;
    long _elapsedTimeEndUsec = 0;
    long _cpuUserTimeStart = 0;
    long _cpuUserTimeEnd = 0;
    long _cpuSystemTimeStart = 0;
    long _cpuSystemTimeEnd = 0;
    bool _cpuValid = true;
  };

  Data _data;
  bool _endDataIsValid = false;
  const MetricsTimerImpl& _timerImpl;
};

class MetricsUtil
{
public:
  static const double getCPUTimeResolution() { return CPU_TIME_RESOLUTION; }
  static const size_t getElapsedTimeDecimalPlaces() { return ELAPSED_TIME_DECIMAL_PLACES; }
  static const size_t getCPUTimeDecimalPlaces() { return CPU_TIME_DECIMAL_PLACES; }
  static const size_t getElapsedTimeFieldWidth() { return ELAPSED_TIME_DECIMAL_PLACES + 5; }
  static const size_t getCPUTimeFieldWidth() { return CPU_TIME_DECIMAL_PLACES + 5; }

  static bool getThreadCPUTime(long& utime, long& stime);

  // NOTE: this enumeration is deprecated. It is recommended that instead
  // of using this, you just place your string straight into the location
  // where you're adding instrumentation
  enum MetricsFactor
  {
    // TO Metrics
    TO_PROCESS_CURRENCY = 0, // Elapsed time in TO.process(CurrencyTrx&)
    TO_PROCESS_METRICS, // Elapsed time in TO.process(MetricsTrx&)
    TO_PROCESS_MILEAGE, // Elapsed time in TO.process(MileageTrx&)
    TO_PROCESS_PRICING, // Elapsed time in TO.process(PricingTrx&)
    TO_PROCESS_SHOPPING, // Elapsed time in TO.process(ShoppingTrx&)
    TO_PROCESS_STATUS, // Elapsed time in TO.process(StatusTrx&)
    TO_SVC_FARES_C, // Elapsed time in TO.faresCollectionService
    TO_SVC_FARES_V, // Elapsed time in TO.faresValidationService
    TO_SVC_PRICING, // Elapsed time in TO.pricingService
    TO_SVC_SHOPPING, // Elapsed time in TO.shoppingService
    TO_SVC_ITIN, // Elapsed time in TO.itinAnalyzerService
    TO_SVC_TAX, // Elapsed time in TO.taxService
    TO_SVC_FARE_CALC, // Elapsed time in TO.fareCalcService
    TO_SVC_CURRENCY, // Elapsed time in TO.currencyService
    TO_SVC_MILEAGE, // Elapsed time in TO.mileageService
    TO_SVC_INTERNAL, // Elapsed time in TO.internalService
    TO_SVC_SERVICE_FEES, // Elapsed time in TO.serviceFeeService
    TO_SVC_TICKETING_FEES, // Elapsed time in TO.serviceFeeService
    TO_SVC_S8_BRAND, // Elapsed time in TO.s8BrandService
    TO_SVC_DECODE, // Elapsed time in TO.DecodeService
    // FCO Metrics
    FCO_PROCESS, // Elapsed time in FCO.process(Trx)
    FCO_GOV_CXR, // Elapsed time in FCO._goveringCarrier().process(Trx)
    FCO_FARE_CS, // Elapsed time in FCO._fareCurrencySelection().select(Trx)
    FCO_GLB_DIR, // Elapsed time in FCO._globalDirection().process(Trx)
    FCO_FLT_TRACKER, // Elapsed time in FCO._flightTracker().process(Trx)
    FCO_FIND_FARES, // Elapsed time in FCO.findFares()
    FCO_DIAG, // Elapsed time in FCO to write diagnostic object
    FCO_CFC_PROCESS, // Elapsed time in FCO call to CarrierFareController.process()
    FCO_AFC_PROCESS, // Elapsed time in FCO call to <add-on construction>.process()
    FCO_FBRC_PROCESS, // Elapsed time in FCO call to FareByRuleController.process()
    FCO_DFC_PROCESS, // Elapsed time in FCO call to DiscountedFareController.process()
    FCO_DFC_VALIDATION, // Elapsed time in FCO call to DiscountedFareController.validate()
    FCO_DFC_CALCAMT, // Elapsed time in FCO call to DiscountedFareController.calcAmount()
    FCO_IFC_PROCESS, // Elapsed time in FCO call to IndustryFareController.process()
    FCO_NFC_PROCESS, // Elapsed time in FCO call to NegotiatedFareController.process()
    FCO_FARE_SORT, // Elapsed time in FCO to sort the Fares
    FCO_FC_TVLDATE, // Elapsed time in FCO call to FareController.getTravelDate()
    FCO_FC_TXREF, // Elapsed time in FCO call to FareController.resolveTariffCrossRef()
    FCO_FC_FCA, // Elapsed time in FCO call to FareController.resolveFareClassApp()
    FCO_FC_FCASEG, // Elapsed time in FCO call to FareController.resolveFareClassAppSeg()
    FCO_FC_FTMATRIX, // Elapsed time in FCO call to FareController.resolveFareTypeMatrix()
    FCO_FC_MATCHLOC, // Elapsed time in FCO call to FareController.matchLocation()
    FCO_FC_CREATEFARES, // Elapsed time in FCO call to FareController.createFares()
    FCO_FC_CREATEPTFARES, // Elapsed time in FCO call to FareController.createPaxTypeFares()
    FCO_FC_CURRENCY, // Elapsed time in FCO call to FareController.convertCurrency()
    FCO_FC_INITFARE, // Elapsed time in FCO call to Fare.init()
    FCO_FC_GETFARE, // Elapsed time in FCO call to DataHandle.get(Fare)

    // FVO Metrics
    FVO_PROCESS, // Elapsed time in FVO.process(Trx)
    FVO_ROUTING, // Elapsed time in FVO.routingController.process(fareMarket)
    FVO_RTG_MAP_VALIDATION, // Elapsed time in routingController for MAP validation.
    FVO_RTG_MAP, // Elapsed time in routingController for MAP validation.
    FVO_RTG_DRV, // Elapsed time in routingController for MAP validation.
    FVO_RTG_RESTRICTION_VALIDATION, // Elapsed time in routingController for RestrictionValidation
    FVO_RTG_MILEAGE_VALIDATION, // Elapsed time in routingController for Mileage Validation
    FVO_RTG_TPD, // Elapsed time in routingController for TPD validation.
    FVO_RTG_PSR, // Elapsed time in tpm collection for PSR validation.
    FVO_RTG_TPM, // Elapsed time in tpm collection process.
    FVO_RTG_MPM, // Elapsed time in mpm collection process.
    FVO_RTG_SURCH_EXCEPT, // Elapsed time in mpm collection process.
    FVO_BKGCODE_VALIDATOR,
    FVO_RULE_CONTROLLER,
    FVO_MC_CONTROLLER,

    // PO Metrics
    PO_PROCESS, // Elapsed time in PO.process(Trx)
    PO_FMKT_MERGER, // Time to build Merged FareMarket
    PO_FMKT_PATH_MATRIX, // Time to build all FareMarketPath
    PO_PU_PATH_MATRIX, // Time to build all PUPath
    PO_INIT_PU_PQ, // Time spend to init
    PO_GET_PU, // Time spend in building PricingUnit
    PO_BUILDING_PU, // Time spend in building PricingUnit
    PO_PU_PQ_OPERATION, // Time spend on PU-PQ
    PO_INIT_FP_PQ, // Time spend to init
    PO_GET_FP, // Time spend in building PricingUnit
    PO_FP_PQ_OPERATION, // Time spend on FP-PQ
    PO_INIT_GFP_PQ, // Time spend to init
    PO_GET_GFP, // Time spend in building PricingUnit
    PO_GFP_PQ_OPERATION, // Time spend on GFP-PQ
    PO_COMB_PROCESS, // Elapsed time in PO call to Combinations process(Trx)
    PO_CXR_PREF_CHECK, // Elapsed time in PO call to cxr pref check
    PO_OJ_DEF_CHECK, // Elapsed time in PO call to cxr pref check
    PO_RT_DEF_CHECK, // Elapsed time in PO call to cxr pref check
    PO_LIMITATION, // Elapsed time in PO to call LimitationOnIndirectTravel
    PO_RT_TO_CT_CONVERSION, // Time to convert RT to CT for hip, etc
    PO_MINFARE_PROCESS, // Elapsed time in PO to call MinFare process(Trx)
    PO_RULE_VALIDATE, // Elapsed time in PO to call RuleController.validate(Trx)
    PO_BKGCODE_REVALIDATE, // Elapsed time in PO to call
    // FareBookingCodeValidator.revalidatePUBkgCode(Trx)
    PO_BKGCODE_MIXED_CLASS, // Elapsed time in PO to call MixedClassController.validate(Trx)
    PO_COLLECT_ENDORSEMENT, // Elapsed time in PO to call collectEndorsement
    PO_DETERMINE_MOST_REST_TKT_DT, // Elapsed time in PO to call MixedClassController.validate(Trx)

    // SO Metrics
    SO_PROCESS, // Elapsed time in SO.process(Trx)

    // ITIN Metrics
    ITIN_PROCESS, // Elapsed time in ITIN.process(Trx)
    ITIN_ATAE, // Elapsed time in ITIN for ATAE calls
    // TAX Metrics
    TAX_PROCESS, // Elapsed time in TAX.process(Trx)
    // Service Fees Metrics
    SERVICE_FEES_PROCESS, // Elapsed time in SERVICE_FEE.process(Trx)
    // Ticketing Fees Metrics
    TICKETING_FEES_PROCESS, // Elapsed time in TICKETING_FEE.process(Trx)
    // S8 Brand Metrics
    S8_BRAND_PROCESS,
    S8_BRAND_QUERY, // HTTP query to Branding Service
    // CBAS Brand Metrics
    CBAS_BRAND_QUERY, // HTTP query to legacy CBAS brands
    // Corba Request Manager Metrics
    CORBAMGR_PROCESS, // Elapsed time in CorbaRequestManager.process(request,response)
    CORBAMGR_REQ_XFORM, // Elapsed time in Xform.RequestToTrx()
    CORBAMGR_SERVICE, // Elapsed time in Service.process(Trx)
    CORBAMGR_RSP_XFORM, // Elapsed time in Xform.TrxToReponse()

    // Rules Metrics
    RC_VALIDATE_PAXFARETYPE, // Elapsed time in RuleConrtoller.validate for pax fare type scope
    RC_VALIDATE_PRICINGUNIT, // Elapsed time in RuleConrtoller.validate for pricing unit scope

    // Metrics for the rule validation phases
    RC_FC_PRE_VALIDATION,
    RC_FC_NORMAL_VALIDATION,
    RC_FC_PRECOMBINABILITY_VALIDATION,
    RC_FC_RE_VALIDATION,
    RC_FC_SHOPPING_VALIDATION,
    RC_FC_SHOPPING_ASO_VALIDATION,
    RC_FC_SHOPPING_WITH_FLIGHTS_VALIDATION,
    RC_FC_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION,
    RC_FC_DYNAMIC_VALIDATION,
    RC_FC_SHOPPING_VALIDATE_IF_CAT4,
    RC_FC_SHOPPING_ITIN_BASED_VALIDATION,
    RC_FC_SHOPPING_ITIN_BASED_VALIDATION_ALT,
    RC_FC_FARE_DISPLAY_VALIDATION,
    RC_FC_RULE_DISPLAY_VALIDATION,
    RC_PU_PRE_VALIDATION,
    RC_PU_NORMAL_VALIDATION,
    RC_PU_PRECOMBINABILITY_VALIDATION,
    RC_PU_RE_VALIDATION,
    RC_PU_SHOPPING_VALIDATION,
    RC_PU_SHOPPING_ASO_VALIDATION,
    RC_PU_SHOPPING_WITH_FLIGHTS_VALIDATION,
    RC_PU_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION,
    RC_PU_DYNAMIC_VALIDATION,
    RC_PU_SHOPPING_VALIDATE_IF_CAT4,
    RC_PU_SHOPPING_ITIN_BASED_VALIDATION,
    RC_PU_SHOPPING_ITIN_BASED_VALIDATION_ALT,

    // Metrics for Record 3 Category rules validation
    //
    RULE3CAT_ALL, // Elapsed time for all Rule record 3 categories
    RULE3CAT_1, //   Fare Application
    RULE3CAT_1_FC, //     Fare Component Validation
    RULE3CAT_1_PU, //     Pricing Unit Validation
    RULE3CAT_2, //   Day/Time Application
    RULE3CAT_2_FC,
    RULE3CAT_2_PU,
    RULE3CAT_3, //   Seasonal Application
    RULE3CAT_3_FC,
    RULE3CAT_3_PU,
    RULE3CAT_4, //   Flight Application
    RULE3CAT_4_FC,
    RULE3CAT_4_PU,
    RULE3CAT_5, //   Advanced Reservation/Ticketing
    RULE3CAT_5_FC,
    RULE3CAT_5_PU,
    RULE3CAT_6, //   Minimum Stay
    RULE3CAT_6_FC,
    RULE3CAT_6_PU,
    RULE3CAT_7, //   Maximum Stay
    RULE3CAT_7_FC,
    RULE3CAT_7_PU,
    RULE3CAT_8, //   Stopovers
    RULE3CAT_8_FC,
    RULE3CAT_8_PU,
    RULE3CAT_9, //   Transfers
    RULE3CAT_9_FC,
    RULE3CAT_9_PU,
    RULE3CAT_11, //   Blackout Dates
    RULE3CAT_11_FC,
    RULE3CAT_11_PU,
    RULE3CAT_12, //   Surcharges
    RULE3CAT_12_FC,
    RULE3CAT_12_PU,
    RULE3CAT_13, //   Accompanied Travel Requirements
    RULE3CAT_13_FC,
    RULE3CAT_13_PU,
    RULE3CAT_14, //   Travel Restrictions
    RULE3CAT_14_FC,
    RULE3CAT_14_PU,
    RULE3CAT_15, //   Sales Restrictions
    RULE3CAT_15_FC,
    RULE3CAT_15_PU,
    RULE3CAT_16, //   Penalties
    RULE3CAT_16_FC,
    RULE3CAT_16_PU,
    TO_PROCESS_FARE_DISPLAY,
    FARE_DISPLAY_SVC,
    FARE_DISPLAY_DSS,
    MAX_METRICS_ENUM
  };

  static bool header(std::ostream& os, const std::string& title);
  static bool lineItemHeader(std::ostream& os);
  static bool lineItem(std::ostream& os, const int factor);

  static bool
  trxLatency(std::ostream& os, PricingTrx& trx, double* totalElapsed = nullptr, double* totalCPU = nullptr);
  static bool
  trxLatency(std::ostream& os, const Trx& trx, double* totalElapsed = nullptr, double* totalCPU = nullptr);
  static bool trxLatency(std::ostream& os,
                         const MultiExchangeTrx& meTrx,
                         double* totalElapsed = nullptr,
                         double* totalCPU = nullptr);

  static const std::string& factorDesc(const int factor);

  static size_t getNumFM(PricingTrx& trx);

protected:
  static void getPricingTrxDetail(
      PricingTrx& trx, size_t& numTvlSegs, size_t& numFM, size_t& numPax, size_t& numPTF);
  static void dumpPricingTrxDetail(
      std::ostream& os, size_t numTvlSegs, size_t numFM, size_t numPax, size_t numPTF);
  static void dumpHeapMemDetail(std::ostream& os, const Trx& trx);

private:
  static const size_t NUM_WIDTH = 3;
  static const size_t DESC_WIDTH = 20;
  static const size_t HEAP_DESC_WIDTH = 47;
  static const size_t ITEM_WIDTH = 10;
  static const size_t ITEM_DECIMALS = 3;

  static const std::string ITEM_SEPARATOR;

  static double ELAPSED_TIME_RESOLUTION;
  static double CPU_TIME_RESOLUTION;

  static const double LORES_ELAPSED_TIME_RESOLUTION;
  static const double LORES_CPU_TIME_RESOLUTION;
  static const double HIRES_ELAPSED_TIME_RESOLUTION;
  static const double HIRES_CPU_TIME_RESOLUTION;

  static std::size_t ELAPSED_TIME_DECIMAL_PLACES;
  static std::size_t CPU_TIME_DECIMAL_PLACES;

  static const std::size_t LORES_ELAPSED_TIME_DECIMAL_PLACES = 2;
  static const std::size_t LORES_CPU_TIME_DECIMAL_PLACES = 2;
  static const std::size_t HIRES_ELAPSED_TIME_DECIMAL_PLACES = 3;
  static const std::size_t HIRES_CPU_TIME_DECIMAL_PLACES = 6;

  static boost::mutex _loadTimerImplsMutex;

  static const MetricsTimerImpl* _doNothingTimerImpl;
  static const MetricsTimerImpl* _defaultTopLevelTimerImpl;
  static const MetricsTimerImpl* _defaultTimerImpl;

  static bool _forceTopLevelOnly;
  static bool _forceLowLevel;
  static bool _timerImplsLoaded;

  static const MetricsTimerImpl&
  getTimerImpl(bool isTopLevel, bool recordMetrics, bool recordTopLevelOnly);
  static const MetricsTimerImpl& getTimerImpl();

  static void loadTimerImpls();

  // Function pointer for thread CPU time method. This pointer is
  //  set during initialization to point to the correct function
  //  based on the config file setting.
  //
  typedef bool getThreadCPUTimeFunc(long& utime, long& stime);
  static getThreadCPUTimeFunc* _getThreadCPUTimeFunc;

  friend class MetricsTimer;
};
} // tse namespace
