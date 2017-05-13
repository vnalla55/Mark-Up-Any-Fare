//----------------------------------------------------------------------------
//
//  Copyright Sabre 2005
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

#include "Common/ErrorResponseException.h"
#include "DataModel/Trx.h"
#include "Server/AppConsoleController.h"
#include "Server/AppConsoleStats.h"

#include <boost/thread/mutex.hpp>

#include <map>
#include <ostream>
#include <vector>

namespace asap
{
class AsapDomain;
}

namespace tse
{
class MultiExchangeTrx;

class TseSrvStats
{
  struct LatencyData
  {
    LatencyData();
    LatencyData(const Trx::Latency& latency);

    LatencyData operator+=(const LatencyData& other);
    const LatencyData operator+(const LatencyData& other) const;

    double elapsedTime;
    double userTime;
    double systemTime;
  };

public:
  enum SERVER_TYPE
  { SERVERTYPE_UNKNOWN,
    SERVERTYPE_PRICING,
    SERVERTYPE_SHOPPING,
    SERVERTYPE_FAREDISPLAY,
    SERVERTYPE_TAX,
    SERVERTYPE_HISTORICAL };

  typedef std::map<ErrorResponseException::ErrorResponseCode, uint64_t> ErrorCounts;

  static bool initialize(const std::string& asapIni);
  static void shutdown();
  static void setServerInfo(const std::string& instanceName, const SERVER_TYPE serverType);

  static void recordTrxCall(bool successful,
                            uint64_t requestSize,
                            uint64_t responseSize,
                            double elapsedTime,
                            double cpuTime,
                            Trx& trx);

  static void recordASv2Call(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);

  static void recordDSSv2Call(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);

  static void recordBillingCall(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);

  static void recordRequestResponseCall(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);
  static void recordRTGCall(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);
  static void recordABCCCall(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);
  static void recordS8BRANDCall(
      bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx);
  static void
  recordFaresCollectionServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void
  recordFaresValidationServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordPricingServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordShoppingServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordItinAnalyzerServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordTaxServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordFareCalcServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordCurrencyServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordMileageServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordInternalServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordFareDisplayServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordFareSelectorServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void
  recordVoluntaryChangesRetrieverServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordBaggageServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordServiceFeesServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordTicketingFeesServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordS8BrandServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordTicketingCxrServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordTicketingCxrDispServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);
  static void recordDecodeServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx);

  static void recordDBCall(double elapsedTime, bool successful);
  static void recordSocketClose();
  static void recordConcurrentTrx(uint16_t concurrentTrx);
  static uint16_t getConcurrency();
  static void recordCurrentDatabase(const std::string& db);
  static std::string getCurrentDatabase();

  static void updateErrorCounts(ErrorResponseException::ErrorResponseCode errorCode);

  static void publishStats(Trx& trx, bool publishASAP = true);
  static void dumpTrxStats(std::ostream& oss, Trx& trx);

  static void mergeTrxStats(MultiExchangeTrx& trx);

  static double getTrxCPUTime(Trx& trx);

  static size_t getSolutionsRequested(const Trx& trx);
  static size_t getSolutionsFound(const Trx& trx);
  // additional instrumentation data
  static void recordTotalNumberFares(Trx& trx, int numberFares);
  static int getTotalNumberFares(const Trx& trx);
  static void recordNumberCat25Fares(Trx& trx, int numberFares);
  static int getNumberCat25Fares(const Trx& trx);
  static void recordNumberAddOnFares(Trx& trx, int numberFares);
  static int getNumberAddOnFares(const Trx& trx);
  static void recordNumberValidatedFares(Trx& trx, int numberValidatedFares);
  static int getNumberValidatedFares(const Trx& trx);
  static std::time_t cacheUpdateTimestamp(bool update = false);

private:
  friend bool AppConsoleController::startup(const std::string& name, TseServer* srv);
  friend void AppConsoleController::shutdown();
  friend class TseAppConsole;
  friend class TseSrvStatsTest;

  static void reset();
  static void dumpErrorCounts(std::ostream& oss);
  static void dumpStats(std::ostream& oss);
  static void dumpStats(std::ostream& oss, const ServicePoint& sp, const char* name);

  static void dumpElapsed(std::ostream& oss);
  static void dumpServicesLatency(std::ostream& oss);

  static void recordCall(size_t index,
                         bool successful,
                         uint64_t requestSize,
                         uint64_t responseSize,
                         double elapsedTime,
                         Trx& trx);

  static void recordServiceCall(size_t elapsedIndex,
                                double elapsedTime,
                                size_t threadIndex,
                                uint32_t activeThreads,
                                Trx& trx);

  static void publishAppConsoleStats(Trx& trx);
  static void publishASAPStats(Trx& trx);
  static void cleanupASAP();

  static const std::string findCustomer(tse::Trx& trx);
  static const std::string& findHost();
  static const std::string& findService(tse::Trx& trx);
  static const std::string findASAPDomain(tse::Trx& trx, bool includeUser = true);

  static void setupTrxStats(std::vector<double>& statData);

  static void mergeTrxStats(std::vector<double>& toStatData,
                            std::vector<double>& fromStatData,
                            boost::mutex& fromMutex);

  static bool
  publishAsapOP01(asap::AsapDomain& domain, const std::vector<double>& statData, Trx& trx);
  static bool publishAsapSV01(asap::AsapDomain& domain,
                              const std::vector<double>& statData,
                              const std::string& domainRoot);
  static bool publishAsapEX01(asap::AsapDomain& domain, const std::vector<double>& statData);
  static bool publishAsapEX02(asap::AsapDomain& domain,
                              const std::vector<double>& statData,
                              const std::string& domainRoot);

  static bool updateLoads();

  static void processLatencyData(Trx& trx);
  static void collectLatencyData(const Trx::Latency& latency);

  static AppConsoleStats* _acStats;
  static ErrorCounts _errorCounts;
  static std::map<const Service*, LatencyData> _servicesLatencyData;
  static boost::mutex _mutex;
  static boost::mutex _asapMutex;

  static constexpr char DELIM = '|';

  //
  // Placed here so they cant be called
  //
  TseSrvStats(const TseSrvStats& rhs);
  TseSrvStats& operator=(const TseSrvStats& rhs);
};

} // end tse namespace


