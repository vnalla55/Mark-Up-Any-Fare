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

#include "Common/TseSrvStats.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/MemoryUsage.h"
#include "Common/TrxUtil.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalcCollector.h"
#include "Server/AppConsoleStats.h"

#include <typeinfo>

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

#ifndef TSE_DISABLE_ASAP
#include <AsapDomain.h>
#include <AsapDomainManager.h>

#endif

#include "Allocator/TrxMalloc.h"

// TODO: remove usage of kernel library, or refactor it from Common directory
#include <sys/sysinfo.h>

#ifdef TSE_DISABLE_ASAP
typedef long long asap_int64;
typedef short asap_int16;

namespace asap
{
class AsapDomain
{
public:
  enum Operation
  { ADD,
    REPLACE,
    REPLACE_TEXT,
    ASAP_MATH_MIN,
    MAX,
    AVG };

  bool updateItem(asap_int16 /*dataItem*/, asap_int64 /*value*/, Operation /*oper*/)
  {
    return true;
  }

  const char* getDomainName() { return "UNKNOWN DOMAIN"; }
};

class AsapDomainManager
{
public:
  static AsapDomainManager& getManager(const std::string& /*asapId*/)
  {
    static AsapDomainManager manager;
    return manager;
  }

  void setAppendProcessID(bool /*value*/) {}

  AsapDomain* getDomain(const std::string& /*domain*/)
  {
    static AsapDomain domain;
    return &domain;
  }

  void removeDomains() {}
  static void removeManagers() {}
};

} // namespace asap

#endif

namespace tse
{
AppConsoleStats* TseSrvStats::_acStats = nullptr;
TseSrvStats::ErrorCounts TseSrvStats::_errorCounts;
std::map<const Service*, TseSrvStats::LatencyData> TseSrvStats::_servicesLatencyData;
boost::mutex TseSrvStats::_mutex;
boost::mutex TseSrvStats::_asapMutex;

namespace
{
ConfigurableValue<std::string>
connLim("SERVER_SOCKET_ADP", "CONNLIMIT");
ConfigurableValue<uint32_t>
numListeners("SERVER_SOCKET_ADP", "NUM_LISTENERS", 1);

tse::ConfigMan _config;
log4cxx::LoggerPtr _logger = 0;

enum EVALUATION_STEPS
{ HOME_AGENCY_IATA = 1,
  PARTITION = 2,
  PCC = 3 };

// Used for sysinfo
const double SYSINFO_SCALE = 1 << SI_LOAD_SHIFT;
const double SYSINFO_KB_IN_BYTES = 1024;

std::vector<EVALUATION_STEPS> _evalOrder;

const char* ASAP_GROUP = "ASAP";
const char* PCC_GROUP = "PCC";
const char* HOME_AGENCY_IATA_GROUP = "HOME_AGENCY_IATA";
const char* PARTITION_GROUP = "PARTITION";

const char* EVAL_ORDER_KEY = "EVAL_ORDER";
const char* DEFAULT_USER_KEY = "DEFAULT_USER";
const char* ASAPID_KEY = "ID";
const char* APPEND_PID_KEY = "APPEND_PID";

std::string _defaultUser = "OTHER";

const std::string HISTORICAL_PRICING_SVC = "ATHPRC";
const std::string HISTORICAL_FAREDISPLAY_SVC = "ATHFRD";

const std::string PRICING_SVC = "AT2PRC";
const std::string IS_SVC = "AT2IS";
const std::string MIP_SVC = "AT2MIP";
const std::string FAREDISPLAY_SVC = "AT2FRD";
const std::string TAX_SVC = "AT2TAX";

const std::type_info& _pricingType = typeid(PricingTrx);
const std::type_info& _shoppingType = typeid(ShoppingTrx);
const std::type_info& _fareDisplayType = typeid(FareDisplayTrx);
const std::type_info& _taxType = typeid(TaxTrx);
const std::type_info& _altPricingType = typeid(AltPricingTrx);
const std::type_info& _portExcType = typeid(ExchangePricingTrx);
const std::type_info& _rexExcType = typeid(RexPricingTrx);

enum STATS
{
  // Note, the order (OK, ERROR, ELAPSED, REQ, RSP matters to the
  // recordCall method
  // TRX
  TRX_OK = 0,
  TRX_ERROR,
  TRX_ELAPSED,
  TRX_REQ_SIZE,
  TRX_RSP_SIZE,
  TRX_CPU,
  TRX_ITIN,
  // ASV2
  ASV2_OK,
  ASV2_ERROR,
  ASV2_ELAPSED,
  ASV2_REQ_SIZE,
  ASV2_RSP_SIZE,
  // DSSV2
  DSSV2_OK,
  DSSV2_ERROR,
  DSSV2_ELAPSED,
  DSSV2_REQ_SIZE,
  DSSV2_RSP_SIZE,
  // BAGG
  BAGG_OK,
  BAGG_ERROR,
  BAGG_ELAPSED,
  BAGG_REQ_SIZE,
  BAGG_RSP_SIZE,
  // BILL
  BILL_OK,
  BILL_ERROR,
  BILL_ELAPSED,
  BILL_REQ_SIZE,
  BILL_RSP_SIZE,
  // RQRS
  RQRS_OK,
  RQRS_ERROR,
  RQRS_ELAPSED,
  RQRS_REQ_SIZE,
  RQRS_RSP_SIZE,
  // RTG
  RTG_OK,
  RTG_ERROR,
  RTG_ELAPSED,
  RTG_REQ_SIZE,
  RTG_RSP_SIZE,
  // ABCC
  ABCC_OK,
  ABCC_ERROR,
  ABCC_ELAPSED,
  ABCC_REQ_SIZE,
  ABCC_RSP_SIZE,
  // S8BRAND
  S8BRAND_OK,
  S8BRAND_ERROR,
  S8BRAND_ELAPSED,
  S8BRAND_REQ_SIZE,
  S8BRAND_RSP_SIZE,
  // TicketingCxr
  TICKETINGCXR_OK,
  TICKETINGCXR_ERROR,
  TICKETINGCXR_ELAPSED,
  TICKETINGCXR_REQ_SIZE,
  TICKETINGCXR_RSP_SIZE,
  // SVC ELAPSED
  SVC_FARES_COLLECTION_ELAPSED,
  SVC_FARES_VALIDATION_ELAPSED,
  SVC_PRICING_ELAPSED,
  SVC_SHOPPING_ELAPSED,
  SVC_ITIN_ANALYZER_ELAPSED,
  SVC_TAX_ELAPSED,
  SVC_FARE_CALC_ELAPSED,
  SVC_CURRENCY_ELAPSED,
  SVC_MILEAGE_ELAPSED,
  SVC_INTERNAL_ELAPSED,
  SVC_FARE_DISPLAY_ELAPSED,
  SVC_FARE_SELECTOR_ELAPSED,
  SVC_REX_FARE_SELECTOR_ELAPSED,
  SVC_FREE_BAG_ELAPSED,
  SVC_SERVICE_FREES_ELAPSED,
  SVC_TICKETING_FEES_ELAPSED,
  SVC_S8_BRAND_ELAPSED,
  SVC_TICKETING_CXR_ELAPSED,
  SVC_TICKETING_CXR_DISPLAY_ELAPSED,
  SVC_DECODE_ELAPSED,
  // SVC THREADS
  SVC_FARES_COLLECTION_THREADS,
  SVC_FARES_VALIDATION_THREADS,
  SVC_PRICING_THREADS,
  SVC_SHOPPING_THREADS,
  SVC_ITIN_ANALYZER_THREADS,
  SVC_TAX_THREADS,
  SVC_FARE_CALC_THREADS,
  SVC_CURRENCY_THREADS,
  SVC_MILEAGE_THREADS,
  SVC_INTERNAL_THREADS,
  SVC_FARE_DISPLAY_THREADS,
  SVC_FARE_SELECTOR_THREADS,
  SVC_REX_FARE_SELECTOR_THREADS,
  SVC_FREE_BAG_THREADS,
  SVC_SERVICE_FREES_THREADS,
  SVC_TICKETING_FEES_THREADS,
  SVC_S8_BRAND_THREADS,
  SVC_TICKETING_CXR_THREADS,
  SVC_TICKETING_CXR_DISPLAY_THREADS,
  SVC_DECODE_THREADS,
  // Fare related instrumentation data
  SVC_TOTAL_NUMBER_FARES,
  SVC_NUMBER_CAT25_FARES,
  SVC_NUMBER_ADDON_FARES,
  SVC_NUMBER_VALIDATED_FARES,
  MAX_STAT_DATA_OFFSET
};

std::string _asapID;
std::string _host;
std::string _instanceName;
bool _appendPid = false;
std::string _currentDB = "";

tse::TseSrvStats::SERVER_TYPE _serverType = tse::TseSrvStats::SERVERTYPE_UNKNOWN;

// Asap functions (These arent in the header because otherwise we would of had to include
//                 the asap headers up there and we didnt want to do that)

asap_int64
asapValue(double d, bool convertTime = false)
{
  if (convertTime)
    return static_cast<asap_int64>(d * 1000000.0f); // Convert time to microseconds

  return static_cast<asap_int64>(d);
}

bool
publishAsapValue(asap::AsapDomain& domain,
                 short itemIndex,
                 asap_int64 value,
                 asap::AsapDomain::Operation op = asap::AsapDomain::ADD)
{
  const MallocContextDisabler context;
  // We throw away 0 add values, the OPS guys wanted it like that.
  // Its seems to make the asap collector happier
  if ((value == 0) && (op == asap::AsapDomain::ADD))
    return true; // Pretend that we worked :)

  if (UNLIKELY(!domain.updateItem(itemIndex, value, op)))
  {
    LOG4CXX_ERROR(_logger,
                  "Unable to update Asap value, domain=" << domain.getDomainName()
                                                         << " index=" << itemIndex);
    return false;
  }
  LOG4CXX_DEBUG(_logger,
                "Updated Asap value, domain=" << domain.getDomainName() << " index=" << itemIndex
                                              << " value=" << value);

  return true;
}
}

TseSrvStats::LatencyData::LatencyData() : elapsedTime(0.0), userTime(0.0), systemTime(0.0)
{
}

TseSrvStats::LatencyData::LatencyData(const Trx::Latency& latency)
  : elapsedTime(latency.wallTime), userTime(latency.userTime), systemTime(latency.systemTime)
{
}

TseSrvStats::LatencyData
TseSrvStats::LatencyData::
operator+=(const TseSrvStats::LatencyData& other)
{
  elapsedTime += other.elapsedTime;
  userTime += other.userTime;
  systemTime += other.systemTime;

  return *this;
}

const TseSrvStats::LatencyData
TseSrvStats::LatencyData::
operator+(const TseSrvStats::LatencyData& other) const
{
  return LatencyData(*this) += other;
}

//----------------------------------------------------------------------------
// recordTrxCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordTrxCall(bool successful,
                           uint64_t requestSize,
                           uint64_t responseSize,
                           double elapsedTime,
                           double cpuTime,
                           Trx& trx)
{
  const MallocContextDisabler context;
  recordCall(TRX_OK, successful, requestSize, responseSize, elapsedTime, trx);

  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());
  trx.statData()[TRX_CPU] += cpuTime;

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
  if (pricingTrx != nullptr)
    trx.statData()[TRX_ITIN] += double(pricingTrx->itin().size());
}

//----------------------------------------------------------------------------
// recordASv2Call()
//----------------------------------------------------------------------------
void
TseSrvStats::recordASv2Call(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(ASV2_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

//----------------------------------------------------------------------------
// recordDSSv2Call()
//----------------------------------------------------------------------------
void
TseSrvStats::recordDSSv2Call(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(DSSV2_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

void
TseSrvStats::recordBillingCall(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(BILL_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

//----------------------------------------------------------------------------
// recordRequestResponseCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordRequestResponseCall(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(RQRS_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

//----------------------------------------------------------------------------
// recordRTGCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordRTGCall(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(RTG_OK, successful, requestSize, responseSize, elapsedTime, trx);
}
//----------------------------------------------------------------------------
// recordABCCCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordABCCCall(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(ABCC_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

//----------------------------------------------------------------------------
// recordS8BRANDCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordS8BRANDCall(
    bool successful, uint64_t requestSize, uint64_t responseSize, double elapsedTime, Trx& trx)
{
  recordCall(S8BRAND_OK, successful, requestSize, responseSize, elapsedTime, trx);
}

//----------------------------------------------------------------------------
// recordCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordCall(size_t startIndex,
                        bool successful,
                        uint64_t requestSize,
                        uint64_t responseSize,
                        double elapsedTime,
                        Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());

  std::vector<double>& statData = trx.statData();
  if (statData.empty())
    setupTrxStats(statData);

  if (successful)
    statData[startIndex]++;
  else
    statData[startIndex + 1]++;

  startIndex += 2;

  statData[startIndex++] += elapsedTime;
  statData[startIndex++] += double(requestSize);
  statData[startIndex++] += double(responseSize);
}

//----------------------------------------------------------------------------
// reset()
//----------------------------------------------------------------------------
void
TseSrvStats::reset()
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(_mutex);

  // We just reset the counters, not the startTime
  long startTime = _acStats->startTime;

  memset(_acStats, 0, sizeof(AppConsoleStats));

  _acStats->startTime = startTime;

  _servicesLatencyData.clear();
}
//----------------------------------------------------------------------------
// dumpErrorCounts()
//----------------------------------------------------------------------------
void
TseSrvStats::dumpErrorCounts(std::ostream& os)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(_mutex);

  ErrorCounts::const_iterator iter = _errorCounts.begin();
  ErrorCounts::const_iterator iterEnd = _errorCounts.end();
  for (; iter != iterEnd; iter++)
  {
    if (iter->second)
    {
      os << iter->first << DELIM << iter->second << DELIM;
    }
  }
}

//----------------------------------------------------------------------------
// dumpStats()
//----------------------------------------------------------------------------
void
TseSrvStats::dumpStats(std::ostream& os)
{
  const MallocContextDisabler context;
  if (_acStats == nullptr)
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);

  if (!updateLoads())
    LOG4CXX_WARN(_logger, "Unable to update system load and free memory values");

  os << "<STATS"
     << " ST=\"" << _acStats->startTime << "\""
     << " CP=\"" << _acStats->totalCPUTime << "\""
     << " IT=\"" << _acStats->totalItins << "\""
     << " DB=\"" << _acStats->dbErrorCount << "\""
     << " TH=\"" << _errorCounts[ErrorResponseException::TRANSACTION_THRESHOLD_REACHED] << "\""
     << " SC=\"" << _acStats->socketClose << "\""
     << " CT=\"" << _acStats->concurrentTrx << "\""
     << " L1=\"" << _acStats->loadLast1Min << "\""
     << " L2=\"" << _acStats->loadLast5Min << "\""
     << " L3=\"" << _acStats->loadLast15Min << "\""
     //<< " FM=\""<<_acStats->freeMemKB<<"\""
     << " FM=\"" << MemoryUsage::getAvailableMemory() << "\""
     << " RS=\"" << MemoryUsage::getResidentMemorySize() << "\""
     << " DQ=\"" << _acStats->dbOkCount << "\""
     << " VM=\"" << MemoryUsage::getVirtualMemorySize() << "\"";

  { // Return Connection Weight for AppConsole
    const std::string& connlimit = connLim.getValue();
    const uint32_t listeners = numListeners.getValue();
    os << " WGHT=\"";
    if (listeners > 1)
    {
      os << connlimit << "x" << listeners;
    }
    else if (connlimit.empty())
    {
      os << "0";
    }
    else
    {
      os << connlimit;
    }
    os << "\"";
  }

  if (!_currentDB.empty())
    os << " DH=\"" << _currentDB << "\"";

  os << " >";

  TseSrvStats::dumpStats(os, _acStats->trx, "TRX");
  TseSrvStats::dumpStats(os, _acStats->ASv2, "AS2");
  TseSrvStats::dumpStats(os, _acStats->DSSv2, "DSS");
  TseSrvStats::dumpStats(os, _acStats->baggage, "BAG");
  TseSrvStats::dumpStats(os, _acStats->billing, "BIL");
  TseSrvStats::dumpStats(os, _acStats->requestResponse, "REQ");
  TseSrvStats::dumpStats(os, _acStats->RTG, "RTG");
  TseSrvStats::dumpStats(os, _acStats->ABCC, "ABC");
  TseSrvStats::dumpStats(os, _acStats->S8BRAND, "S8B");

  os << "</STATS>";
}

//----------------------------------------------------------------------------
// dumpStats()
//----------------------------------------------------------------------------
void
TseSrvStats::dumpStats(std::ostream& os, const ServicePoint& sp, const char* name)
{
  const MallocContextDisabler context;
  if (name == nullptr)
    return;

  os << "<SP"
     << " NM=\"" << name << "\""
     << " OK=\"" << sp.totalOkCount << "\""
     << " ER=\"" << sp.totalErrorCount << "\""
     << " ET=\"" << sp.totalElapsedTime << "\""
     << " RQ=\"" << sp.totalReqSize << "\""
     << " RS=\"" << sp.totalRspSize << "\""
     << " />";
}

void
TseSrvStats::dumpServicesLatency(std::ostream& os)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(_mutex);

  os << "<SLC "
     << "TOK=\"" << _acStats->trx.totalOkCount << "\" TER=\"" << _acStats->trx.totalErrorCount
     << "\">";

  typedef std::map<const Service*, LatencyData> ServiceLatencyMap;

  for (const ServiceLatencyMap::value_type& pair : _servicesLatencyData)
  {
    const LatencyData& latencyData = pair.second;

    os << "<SVC NAM=\"" << pair.first->name() << "\" ELP=\"" << latencyData.elapsedTime
       << "\" USR=\"" << latencyData.userTime << "\" SYS=\"" << latencyData.systemTime << "\" />";
  }
  os << "</SLC>";
}
//----------------------------------------------------------------------------
// recordFaresCollectionServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordFaresCollectionServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_FARES_COLLECTION_ELAPSED, elapsedTime, SVC_FARES_COLLECTION_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordFaresValidationServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordFaresValidationServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_FARES_VALIDATION_ELAPSED, elapsedTime, SVC_FARES_VALIDATION_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordPricingServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordPricingServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_PRICING_ELAPSED, elapsedTime, SVC_PRICING_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordShoppingServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordShoppingServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_SHOPPING_ELAPSED, elapsedTime, SVC_SHOPPING_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordItinAnalyzerServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordItinAnalyzerServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_ITIN_ANALYZER_ELAPSED, elapsedTime, SVC_ITIN_ANALYZER_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordTaxServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordTaxServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_TAX_ELAPSED, elapsedTime, SVC_TAX_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordFareCalcServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordFareCalcServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_FARE_CALC_ELAPSED, elapsedTime, SVC_FARE_CALC_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordCurrencyServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordCurrencyServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_CURRENCY_ELAPSED, elapsedTime, SVC_CURRENCY_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordMileageServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordMileageServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_MILEAGE_ELAPSED, elapsedTime, SVC_MILEAGE_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordInternalServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordInternalServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_INTERNAL_ELAPSED, elapsedTime, SVC_INTERNAL_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordFareDisplayServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordFareDisplayServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_FARE_DISPLAY_ELAPSED, elapsedTime, SVC_FARE_DISPLAY_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordFareSelectorServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordFareSelectorServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_FARE_SELECTOR_ELAPSED, elapsedTime, SVC_FARE_SELECTOR_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordFareSelectorServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordBaggageServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_FREE_BAG_ELAPSED, elapsedTime, SVC_FREE_BAG_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordServiceFeesServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordServiceFeesServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_SERVICE_FREES_ELAPSED, elapsedTime, SVC_SERVICE_FREES_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordTicketingFeesServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordTicketingFeesServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_TICKETING_FEES_ELAPSED, elapsedTime, SVC_TICKETING_FEES_THREADS, activeThreads, trx);
}

//----------------------------------------------------------------------------
// recordS8BrandServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordS8BrandServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_S8_BRAND_ELAPSED, elapsedTime, SVC_S8_BRAND_THREADS, activeThreads, trx);
}

void
TseSrvStats::recordTicketingCxrServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(
      SVC_TICKETING_CXR_ELAPSED, elapsedTime, SVC_TICKETING_CXR_THREADS, activeThreads, trx);
}

void
TseSrvStats::recordTicketingCxrDispServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_TICKETING_CXR_DISPLAY_ELAPSED,
                    elapsedTime,
                    SVC_TICKETING_CXR_DISPLAY_THREADS,
                    activeThreads,
                    trx);
}

void
TseSrvStats::recordDecodeServiceCall(double elapsedTime, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_DECODE_ELAPSED, elapsedTime, SVC_DECODE_THREADS, activeThreads, trx);
}

void
TseSrvStats::recordTotalNumberFares(Trx& trx, int numberFares)
{
  std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    setupTrxStats(statData);
  }
  statData[SVC_TOTAL_NUMBER_FARES] += numberFares;
}
int
TseSrvStats::getTotalNumberFares(const Trx& trx)
{
  const std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    return 0;
  }
  return static_cast<int>(statData[SVC_TOTAL_NUMBER_FARES] + .5);
}
void
TseSrvStats::recordNumberCat25Fares(Trx& trx, int numberFares)
{
  std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    setupTrxStats(statData);
  }
  statData[SVC_NUMBER_CAT25_FARES] += numberFares;
}
int
TseSrvStats::getNumberCat25Fares(const Trx& trx)
{
  const std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    return 0;
  }
  return static_cast<int>(statData[SVC_NUMBER_CAT25_FARES] + .5);
}
void
TseSrvStats::recordNumberAddOnFares(Trx& trx, int numberFares)
{
  std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    setupTrxStats(statData);
  }
  statData[SVC_NUMBER_ADDON_FARES] += numberFares;
}
int
TseSrvStats::getNumberAddOnFares(const Trx& trx)
{
  const std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    return 0;
  }
  return static_cast<int>(statData[SVC_NUMBER_ADDON_FARES] + .5);
}

void
TseSrvStats::recordNumberValidatedFares(Trx& trx, int numberValidatedFares)
{
  std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    setupTrxStats(statData);
  }
  statData[SVC_NUMBER_VALIDATED_FARES] += numberValidatedFares;
}

int
TseSrvStats::getNumberValidatedFares(const Trx& trx)
{
  const std::vector<double>& statData = trx.statData();
  if (statData.empty())
  {
    return 0;
  }
  return static_cast<int>(statData[SVC_NUMBER_VALIDATED_FARES] + .5);
}
//----------------------------------------------------------------------------
// recordFareDisplayServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordVoluntaryChangesRetrieverServiceCall(double elapsedTime,
                                                        uint32_t activeThreads,
                                                        Trx& trx)
{
  const MallocContextDisabler context;
  recordServiceCall(SVC_REX_FARE_SELECTOR_ELAPSED,
                    elapsedTime,
                    SVC_REX_FARE_SELECTOR_THREADS,
                    activeThreads,
                    trx);
}

//----------------------------------------------------------------------------
// dumpElapsed()
//----------------------------------------------------------------------------
void
TseSrvStats::dumpElapsed(std::ostream& oss)
{
  const MallocContextDisabler context;
  if (_acStats == nullptr)
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);

  oss << "<ELAPSED"
      << " FC=\"" << _acStats->faresCollectionServiceElapsed << "\""
      << " FV=\"" << _acStats->faresValidationServiceElapsed << "\""
      << " PO=\"" << _acStats->pricingServiceElapsed << "\""
      << " SS=\"" << _acStats->shoppingServiceElapsed << "\""
      << " IA=\"" << _acStats->itinAnalyzerServiceElapsed << "\""
      << " TX=\"" << _acStats->taxServiceElapsed << "\""
      << " CA=\"" << _acStats->fareCalcServiceElapsed << "\""
      << " CU=\"" << _acStats->currencyServiceElapsed << "\""
      << " MI=\"" << _acStats->mileageServiceElapsed << "\""
      << " IN=\"" << _acStats->internalServiceElapsed << "\""
      << " FD=\"" << _acStats->fareDisplayServiceElapsed << "\""
      << " FS=\"" << _acStats->fareSelectorServiceElapsed << "\""
      << " DS=\"" << _acStats->dbElapsedTime << "\""
      << " RF=\"" << _acStats->rexFareSelectorServiceElapsed << "\""
      << " FB=\"" << _acStats->freeBagServiceElapsed << "\""
      << " SF=\"" << _acStats->serviceFeesServiceElapsed << "\""
      << " TF=\"" << _acStats->ticketingFeesServiceElapsed << "\""
      << " S8=\"" << _acStats->s8BrandServiceElapsed << "\""
      << " TC=\"" << _acStats->ticketingCxrServiceElapsed << "\""
      << " />";
}

void
TseSrvStats::setServerInfo(const std::string& instanceName, const SERVER_TYPE serverType)
{
  const MallocContextDisabler context;
  _instanceName = instanceName;
  _serverType = serverType;
}

//----------------------------------------------------------------------------
// initialize()
//----------------------------------------------------------------------------
bool
TseSrvStats::initialize(const std::string& asapIni)
{
  const MallocContextDisabler context;
  _logger = log4cxx::Logger::getLogger("atseintl.Common.TseSrvStats");

  cleanupASAP();

  {
    char hostname[1024];

    if (::gethostname(hostname, 1023) < 0) // lint !e505 !e506
    {
      LOG4CXX_ERROR(_logger, "Unable to determine hostname");
      return false;
    }

    _host = hostname; // lint !e603

    for (auto& elem : _host)
      elem = char(::toupper(elem));
  }

  if (asapIni.empty())
  {
    LOG4CXX_ERROR(_logger, "No ASAP configuration file specified");
    return false;
  }

  if (!_config.read(asapIni))
  {
    LOG4CXX_ERROR(_logger, "Unable to read ASAP configuration file '" << asapIni << "'");
    return false;
  }

  // Determine evaluation order
  // EVAL_ORDER = HOME_AGENCY_IATA|PARTITION|PCC
  std::string eval;
  if (!_config.getValue(EVAL_ORDER_KEY, eval, ASAP_GROUP))
  {
    LOG4CXX_ERROR(_logger, "Unable to find config value " << ASAP_GROUP << "." << EVAL_ORDER_KEY);
    return false;
  }

  boost::char_separator<char> sep("|");
  boost::tokenizer<boost::char_separator<char>> tokens(eval, sep);

  _evalOrder.clear();

  boost::tokenizer<boost::char_separator<char>>::iterator i = tokens.begin();
  boost::tokenizer<boost::char_separator<char>>::iterator j = tokens.end();

  for (; i != j; ++i)
  {
    if (*i == PCC_GROUP)
      _evalOrder.push_back(PCC);
    else if (*i == HOME_AGENCY_IATA_GROUP)
      _evalOrder.push_back(HOME_AGENCY_IATA);
    else if (*i == PARTITION_GROUP)
      _evalOrder.push_back(PARTITION);
  }

  if (_evalOrder.empty())
  {
    LOG4CXX_ERROR(_logger, "No evaluation order for ASAP keys");
    return false;
  }

  std::string user;
  if (_config.getValue(DEFAULT_USER_KEY, user, ASAP_GROUP))
  {
    if (!user.empty())
      _defaultUser = user;
  }

  if (!_config.getValue(ASAPID_KEY, _asapID, ASAP_GROUP))
  {
    LOG4CXX_ERROR(_logger, "Unable to find config value " << ASAP_GROUP << "." << ASAPID_KEY);
    return false;
  }

  if (!_asapID.empty())
  {
    std::string pid;
    if (_config.getValue(APPEND_PID_KEY, pid, ASAP_GROUP))
    {
      _appendPid = (pid == "Y" || pid == "y");
    }
    LOG4CXX_DEBUG(_logger, "Setting AsapDomainManager::AppendProcessID to " << _appendPid);
    asap::AsapDomainManager& adm = asap::AsapDomainManager::getManager(_asapID);
    adm.setAppendProcessID(_appendPid);
  }
  return true;
}

void
TseSrvStats::shutdown()
{
  cleanupASAP();
}

const std::string
TseSrvStats::findCustomer(Trx& trx)
{
  const MallocContextDisabler context;
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
  if (pricingTrx == nullptr)
    return _defaultUser;

  // Use the evaluation order ot figure out the customer
  std::vector<EVALUATION_STEPS>::iterator i = _evalOrder.begin();
  std::vector<EVALUATION_STEPS>::iterator j = _evalOrder.end();

  std::string cust;

  for (; i != j; ++i)
  {
    switch (*i)
    {
    case HOME_AGENCY_IATA:
    {
      const std::string& iata = TrxUtil::getHomeAgencyIATA(*pricingTrx);
      if (!iata.empty())
        _config.getValue(iata, cust, HOME_AGENCY_IATA_GROUP);
    }
    break;

    case PARTITION:
    {
      const std::string& part = TrxUtil::getPartitionId(*pricingTrx);
      if (!part.empty())
        _config.getValue(part, cust, PARTITION_GROUP);
    }
    break;

    case PCC:
    {
      std::string pcc = TrxUtil::getPCC(*pricingTrx);
      if (!pcc.empty())
        _config.getValue(pcc, cust, PCC_GROUP);
    }
    break;

    default:
      break;
    }

    if (!cust.empty())
      return cust;
  }

  return _defaultUser;
}

//----------------------------------------------------------------------------
// findHost()
//----------------------------------------------------------------------------
const std::string&
TseSrvStats::findHost()
{
  return _host;
}

//----------------------------------------------------------------------------
// findService()
//----------------------------------------------------------------------------
const std::string&
TseSrvStats::findService(tse::Trx& trx)
{
  const MallocContextDisabler context;
  const std::type_info& trxType = typeid(trx);

  if ((trxType == _pricingType) || (trxType == _altPricingType) || (trxType == _portExcType) ||
      (trxType == _rexExcType))
  {
    if (_serverType == SERVERTYPE_UNKNOWN)
    {
      LOG4CXX_ERROR(_logger,
                    "serverType is not set, may not be correct determining MIP vs Pricing "
                    "ASAP transaction types");
    }
    else
    {
      if (_serverType == SERVERTYPE_SHOPPING)
        return MIP_SVC;
    }

    if (_serverType == SERVERTYPE_HISTORICAL)
      return HISTORICAL_PRICING_SVC;
    else
      return PRICING_SVC;
  }
  else if (trxType == _shoppingType) // Note we ignore SinglePass on purpose, those are
    return IS_SVC; // only supposed to be for testing
  else if (trxType == _fareDisplayType)
  {
    if (_serverType == SERVERTYPE_HISTORICAL)
      return HISTORICAL_FAREDISPLAY_SVC;
    else
      return FAREDISPLAY_SVC;
  }
  else if (trxType == _taxType)
    return TAX_SVC;

  return EMPTY_STRING();
}

//----------------------------------------------------------------------------
// findASAPDomain()
//----------------------------------------------------------------------------
const std::string
TseSrvStats::findASAPDomain(tse::Trx& trx, bool includeUser)
{
  const MallocContextDisabler context;
  // Domain looks like \host\user\port
  const std::string& host = findHost();
  if (host.empty())
    return EMPTY_STRING();

  std::string user;
  if (includeUser)
    user = findCustomer(trx);

  std::ostringstream oss;
  oss << host;

  if (!user.empty() && includeUser)
    oss << '\\' << user;

  // We only put the instanceName if appendPid is off
  // otherwise the pid works as our unique key
  if (!_instanceName.empty() && !_appendPid)
    oss << '\\' << _instanceName;

  return oss.str();
}

//----------------------------------------------------------------------------
// recordDBCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordDBCall(double elapsedTime, bool successful)
{
  const MallocContextDisabler context;
  if (_acStats == nullptr)
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);
  if (LIKELY(successful))
    _acStats->dbOkCount++;
  else
    _acStats->dbErrorCount++;

  _acStats->dbElapsedTime += elapsedTime;
}

//----------------------------------------------------------------------------
// recordCurrentDatabase()
//----------------------------------------------------------------------------
void
TseSrvStats::recordCurrentDatabase(const std::string& db)
{
  const MallocContextDisabler context;

  boost::lock_guard<boost::mutex> guard(_mutex);
  _currentDB = db;
}
//----------------------------------------------------------------------------
// getCurrentDatabase()
//----------------------------------------------------------------------------
std::string
TseSrvStats::getCurrentDatabase()
{
  boost::lock_guard<boost::mutex> guard(_mutex);
  return _currentDB;
}

//----------------------------------------------------------------------------
// recordSocketClose()
//----------------------------------------------------------------------------
void
TseSrvStats::recordSocketClose()
{
  if (_acStats == nullptr)
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);
  _acStats->socketClose++;
}

//----------------------------------------------------------------------------
// recordConcurrentTrx()
//----------------------------------------------------------------------------
void
TseSrvStats::recordConcurrentTrx(uint16_t concurrentTrx)
{
  if (_acStats == nullptr)
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);
  _acStats->concurrentTrx = concurrentTrx;
}

uint16_t
TseSrvStats::getConcurrency()
{
  boost::lock_guard<boost::mutex> guard(_mutex);
  return _acStats->concurrentTrx;
}

//----------------------------------------------------------------------------
// publishStats()
//----------------------------------------------------------------------------
void
TseSrvStats::publishStats(Trx& trx, bool publishASAP)
{
  processLatencyData(trx);

  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());
  std::vector<double>& statData = trx.statData();

  if (statData.empty())
    return; // We didnt keep any statistics

  if (_acStats != nullptr)
    publishAppConsoleStats(trx);

  if (!_asapID.empty() && publishASAP)
    publishASAPStats(trx);
}

void
TseSrvStats::processLatencyData(Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(_mutex);

  typedef std::map<size_t, std::deque<Trx::Latency>> LatencyMap;

  for (const LatencyMap& latencyMap : trx.latencyDataThread())
  {
    for (const LatencyMap::value_type& pair : latencyMap)
    {
      for (const Trx::Latency& latency : pair.second)
      {
        collectLatencyData(latency);
      }
    }
  }

  for (const Trx::Latency& latency : trx.latencyData())
  {
    collectLatencyData(latency);
  }
}

void
TseSrvStats::collectLatencyData(const Trx::Latency& latency)
{
  LatencyData taskLatency(latency);

  if (latency.toplevelTaskData && latency.service)
    _servicesLatencyData[latency.service] += taskLatency;
}
//----------------------------------------------------------------------------
// recordServiceCall()
//----------------------------------------------------------------------------
void
TseSrvStats::recordServiceCall(
    size_t elapsedIndex, double elapsedTime, size_t threadIndex, uint32_t activeThreads, Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());
  std::vector<double>& statData = trx.statData();

  if (statData.empty())
    setupTrxStats(statData);

  statData[elapsedIndex] += elapsedTime;
  statData[threadIndex] = activeThreads;
}

//----------------------------------------------------------------------------
// publishAppConsoleStats()
//----------------------------------------------------------------------------
void
TseSrvStats::publishAppConsoleStats(Trx& trx)
{
  const MallocContextDisabler context;
  // Already locked from publishStats()
  std::vector<double>& statData = trx.statData();

  if (statData.empty())
    return;

  boost::lock_guard<boost::mutex> guard(_mutex);

  // TRX
  _acStats->trx.totalOkCount += static_cast<uint64_t>(statData[TRX_OK]);
  _acStats->trx.totalErrorCount += static_cast<uint64_t>(statData[TRX_ERROR]);
  _acStats->trx.totalElapsedTime += statData[TRX_ELAPSED];
  _acStats->totalCPUTime += statData[TRX_CPU];
  _acStats->totalItins += static_cast<uint64_t>(statData[TRX_ITIN]);
  _acStats->trx.totalReqSize += static_cast<uint64_t>(statData[TRX_REQ_SIZE]);
  _acStats->trx.totalRspSize += static_cast<uint64_t>(statData[TRX_RSP_SIZE]);

  // ASV2
  _acStats->ASv2.totalOkCount += static_cast<uint64_t>(statData[ASV2_OK]);
  _acStats->ASv2.totalErrorCount += static_cast<uint64_t>(statData[ASV2_ERROR]);
  _acStats->ASv2.totalElapsedTime += statData[ASV2_ELAPSED];
  _acStats->ASv2.totalReqSize += static_cast<uint64_t>(statData[ASV2_REQ_SIZE]);
  _acStats->ASv2.totalRspSize += static_cast<uint64_t>(statData[ASV2_RSP_SIZE]);

  // DSSV2
  _acStats->DSSv2.totalOkCount += static_cast<uint64_t>(statData[DSSV2_OK]);
  _acStats->DSSv2.totalErrorCount += static_cast<uint64_t>(statData[DSSV2_ERROR]);
  _acStats->DSSv2.totalElapsedTime += statData[DSSV2_ELAPSED];
  _acStats->DSSv2.totalReqSize += static_cast<uint64_t>(statData[DSSV2_REQ_SIZE]);
  _acStats->DSSv2.totalRspSize += static_cast<uint64_t>(statData[DSSV2_RSP_SIZE]);

  // BAGG
  _acStats->baggage.totalOkCount += static_cast<uint64_t>(statData[BAGG_OK]);
  _acStats->baggage.totalErrorCount += static_cast<uint64_t>(statData[BAGG_ERROR]);
  _acStats->baggage.totalElapsedTime += statData[BAGG_ELAPSED];
  _acStats->baggage.totalReqSize += static_cast<uint64_t>(statData[BAGG_REQ_SIZE]);
  _acStats->baggage.totalRspSize += static_cast<uint64_t>(statData[BAGG_RSP_SIZE]);

  // BILL
  _acStats->billing.totalOkCount += static_cast<uint64_t>(statData[BILL_OK]);
  _acStats->billing.totalErrorCount += static_cast<uint64_t>(statData[BILL_ERROR]);
  _acStats->billing.totalElapsedTime += statData[BILL_ELAPSED];
  _acStats->billing.totalReqSize += static_cast<uint64_t>(statData[BILL_REQ_SIZE]);
  _acStats->billing.totalRspSize += static_cast<uint64_t>(statData[BILL_RSP_SIZE]);

  // RQRS
  _acStats->requestResponse.totalOkCount += static_cast<uint64_t>(statData[RQRS_OK]);
  _acStats->requestResponse.totalErrorCount += static_cast<uint64_t>(statData[RQRS_ERROR]);
  _acStats->requestResponse.totalElapsedTime += statData[RQRS_ELAPSED];
  _acStats->requestResponse.totalReqSize += static_cast<uint64_t>(statData[RQRS_REQ_SIZE]);
  _acStats->requestResponse.totalRspSize += static_cast<uint64_t>(statData[RQRS_RSP_SIZE]);

  // RTG
  _acStats->RTG.totalOkCount += static_cast<uint64_t>(statData[RTG_OK]);
  _acStats->RTG.totalErrorCount += static_cast<uint64_t>(statData[RTG_ERROR]);
  _acStats->RTG.totalElapsedTime += statData[RTG_ELAPSED];
  _acStats->RTG.totalReqSize += static_cast<uint64_t>(statData[RTG_REQ_SIZE]);
  _acStats->RTG.totalRspSize += static_cast<uint64_t>(statData[RTG_RSP_SIZE]);

  // ABCC
  _acStats->ABCC.totalOkCount += static_cast<uint64_t>(statData[ABCC_OK]);
  _acStats->ABCC.totalErrorCount += static_cast<uint64_t>(statData[ABCC_ERROR]);
  _acStats->ABCC.totalElapsedTime += statData[ABCC_ELAPSED];
  _acStats->ABCC.totalReqSize += static_cast<uint64_t>(statData[ABCC_REQ_SIZE]);
  _acStats->ABCC.totalRspSize += static_cast<uint64_t>(statData[ABCC_RSP_SIZE]);

  // S8BRAND
  _acStats->S8BRAND.totalOkCount += static_cast<uint64_t>(statData[S8BRAND_OK]);
  _acStats->S8BRAND.totalErrorCount += static_cast<uint64_t>(statData[S8BRAND_ERROR]);
  _acStats->S8BRAND.totalElapsedTime += statData[S8BRAND_ELAPSED];
  _acStats->S8BRAND.totalReqSize += static_cast<uint64_t>(statData[S8BRAND_REQ_SIZE]);
  _acStats->S8BRAND.totalRspSize += static_cast<uint64_t>(statData[S8BRAND_RSP_SIZE]);

  // TICKETINGCXR
  _acStats->TicketingCxr.totalOkCount += static_cast<uint64_t>(statData[TICKETINGCXR_OK]);
  _acStats->TicketingCxr.totalErrorCount += static_cast<uint64_t>(statData[TICKETINGCXR_ERROR]);
  _acStats->TicketingCxr.totalElapsedTime += statData[TICKETINGCXR_ELAPSED];
  _acStats->TicketingCxr.totalReqSize += static_cast<uint64_t>(statData[TICKETINGCXR_REQ_SIZE]);
  _acStats->TicketingCxr.totalRspSize += static_cast<uint64_t>(statData[TICKETINGCXR_RSP_SIZE]);

  // SVC ELAPSED
  _acStats->faresCollectionServiceElapsed += statData[SVC_FARES_COLLECTION_ELAPSED];
  _acStats->faresValidationServiceElapsed += statData[SVC_FARES_VALIDATION_ELAPSED];
  _acStats->pricingServiceElapsed += statData[SVC_PRICING_ELAPSED];
  _acStats->shoppingServiceElapsed += statData[SVC_SHOPPING_ELAPSED];
  _acStats->itinAnalyzerServiceElapsed += statData[SVC_ITIN_ANALYZER_ELAPSED];
  _acStats->taxServiceElapsed += statData[SVC_TAX_ELAPSED];
  _acStats->fareCalcServiceElapsed += statData[SVC_FARE_CALC_ELAPSED];
  _acStats->currencyServiceElapsed += statData[SVC_CURRENCY_ELAPSED];
  _acStats->mileageServiceElapsed += statData[SVC_MILEAGE_ELAPSED];
  _acStats->internalServiceElapsed += statData[SVC_INTERNAL_ELAPSED];
  _acStats->fareDisplayServiceElapsed += statData[SVC_FARE_DISPLAY_ELAPSED];
  _acStats->fareSelectorServiceElapsed += statData[SVC_FARE_SELECTOR_ELAPSED];
  _acStats->rexFareSelectorServiceElapsed += statData[SVC_REX_FARE_SELECTOR_ELAPSED];
  _acStats->freeBagServiceElapsed += statData[SVC_FREE_BAG_ELAPSED];
  _acStats->serviceFeesServiceElapsed += statData[SVC_SERVICE_FREES_ELAPSED];
  _acStats->ticketingFeesServiceElapsed += statData[SVC_TICKETING_FEES_ELAPSED];
  _acStats->s8BrandServiceElapsed += statData[SVC_S8_BRAND_ELAPSED];
  _acStats->ticketingCxrServiceElapsed += statData[SVC_TICKETING_CXR_ELAPSED];
  _acStats->ticketingCxrDispServiceElapsed += statData[SVC_TICKETING_CXR_DISPLAY_ELAPSED];

  // SVC Threads
  _acStats->faresCollectionServiceThreads =
      static_cast<uint32_t>(statData[SVC_FARES_COLLECTION_THREADS]);
  _acStats->faresValidationServiceThreads =
      static_cast<uint32_t>(statData[SVC_FARES_VALIDATION_THREADS]);
  _acStats->pricingServiceThreads = static_cast<uint32_t>(statData[SVC_PRICING_THREADS]);
  _acStats->shoppingServiceThreads = static_cast<uint32_t>(statData[SVC_SHOPPING_THREADS]);
  _acStats->itinAnalyzerServiceThreads = static_cast<uint32_t>(statData[SVC_ITIN_ANALYZER_THREADS]);
  _acStats->taxServiceThreads = static_cast<uint32_t>(statData[SVC_TAX_THREADS]);
  _acStats->fareCalcServiceThreads = static_cast<uint32_t>(statData[SVC_FARE_CALC_THREADS]);
  _acStats->currencyServiceThreads = static_cast<uint32_t>(statData[SVC_CURRENCY_THREADS]);
  _acStats->mileageServiceThreads = static_cast<uint32_t>(statData[SVC_MILEAGE_THREADS]);
  _acStats->internalServiceThreads = static_cast<uint32_t>(statData[SVC_INTERNAL_THREADS]);
  _acStats->fareDisplayServiceThreads = static_cast<uint32_t>(statData[SVC_FARE_DISPLAY_THREADS]);
  _acStats->fareSelectorServiceThreads = static_cast<uint32_t>(statData[SVC_FARE_SELECTOR_THREADS]);
  _acStats->rexFareSelectorServiceThreads =
      static_cast<uint32_t>(statData[SVC_REX_FARE_SELECTOR_THREADS]);
  _acStats->serviceFeesServiceThreads = static_cast<uint32_t>(statData[SVC_SERVICE_FREES_THREADS]);
  _acStats->ticketingFeesServiceThreads =
      static_cast<uint32_t>(statData[SVC_TICKETING_FEES_THREADS]);
  _acStats->s8BrandServiceThreads = static_cast<uint32_t>(statData[SVC_S8_BRAND_THREADS]);
  _acStats->ticketingCxrServiceThreads = static_cast<uint32_t>(statData[SVC_TICKETING_CXR_THREADS]);
  _acStats->ticketingCxrDispServiceThreads =
      static_cast<uint32_t>(statData[SVC_TICKETING_CXR_DISPLAY_THREADS]);

  // {
  //     ostringstream oss;
  //     dumpThreads(oss);
  //     std::cout<<oss.str()<<std::endl;
  // }
}

//----------------------------------------------------------------------------
// publishASAPStats()
//----------------------------------------------------------------------------
void
TseSrvStats::publishASAPStats(Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(_asapMutex);

  // Already locked from publishStats()
  std::vector<double>& statData = trx.statData();

  if (statData.empty())
    return;

  std::string domainRoot = findService(trx);
  if (domainRoot.empty())
  {
    LOG4CXX_INFO(_logger, "Unable to determine ASAP domain for Trx");
    return;
  }
  LOG4CXX_DEBUG(_logger, "domainRoot = '" << domainRoot << "'");

  std::string subdomain = findASAPDomain(trx);
  std::string subdomainNoUser = findASAPDomain(trx, false);
  if (subdomain.empty() || subdomainNoUser.empty())
  {
    LOG4CXX_INFO(_logger, "Unable to determine ASAP subdomain for Trx");
    return;
  }
  LOG4CXX_DEBUG(_logger, "subdomain = '" << subdomain << "'");
  LOG4CXX_DEBUG(_logger, "subdomainNoUser = '" << subdomainNoUser << "'");

  asap::AsapDomainManager& adm = asap::AsapDomainManager::getManager(_asapID);
  //  adm.setAppendProcessID(false);

  //
  // XXXXOP01
  //
  std::string domainName = domainRoot + "OP01\\" + subdomain;
  asap::AsapDomain* domain = adm.getDomain(domainName);
  if (domain != nullptr)
  {
    LOG4CXX_DEBUG(_logger, "Updating domain '" << domainName << "'");
    publishAsapOP01(*domain, statData, trx);
  }

  //
  // XXXXSV01
  //
  domainName = domainRoot + "SV01\\" + subdomainNoUser;
  domain = adm.getDomain(domainName);
  if (domain != nullptr)
  {
    LOG4CXX_DEBUG(_logger, "Updating domain '" << domainName << "'");
    publishAsapSV01(*domain, statData, domainRoot);
  }

  //
  // XXXXEX01
  //
  domainName = domainRoot + "EX01\\" + subdomainNoUser;
  domain = adm.getDomain(domainName);
  if (domain != nullptr)
  {
    LOG4CXX_DEBUG(_logger, "Updating domain '" << domainName << "'");
    publishAsapEX01(*domain, statData);
  }

  //
  // XXXXEX02
  //

  // AT2TAX does not have an EX02 segment
  if (domainRoot == TAX_SVC)
    return;

  domainName = domainRoot + "EX02\\" + subdomain;
  domain = adm.getDomain(domainName);
  if (domain != nullptr)
  {
    LOG4CXX_DEBUG(_logger, "Updating domain '" << domainName << "'");
    publishAsapEX02(*domain, statData, domainRoot);
  }
}

//----------------------------------------------------------------------------
// publishAsapOP01()
//----------------------------------------------------------------------------
bool
TseSrvStats::publishAsapOP01(asap::AsapDomain& domain,
                             const std::vector<double>& statData,
                             Trx& trx)
{
  {
    asap_int64 trxOk = static_cast<asap_int64>(statData[TRX_OK]);
    asap_int64 trxErr = static_cast<asap_int64>(statData[TRX_ERROR]);
    asap_int64 trxTot = trxOk + trxErr;

    publishAsapValue(domain, 0, trxTot);
    publishAsapValue(domain, 9, trxErr);
  }

  publishAsapValue(domain, 1, asapValue(statData[TRX_CPU], true));
  publishAsapValue(domain, 2, asapValue(statData[TRX_CPU], true), asap::AsapDomain::MAX);
  publishAsapValue(domain, 3, asapValue(statData[TRX_ELAPSED], true));
  publishAsapValue(domain, 4, asapValue(statData[TRX_ELAPSED], true), asap::AsapDomain::MAX);

  {
    asap_int64 fares = 0;

    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx);
    if (pricingTrx != nullptr)
    {
      std::vector<FareMarket*>::iterator i = pricingTrx->fareMarket().begin();
      std::vector<FareMarket*>::iterator j = pricingTrx->fareMarket().end();

      for (; i != j; ++i)
      {
        FareMarket* fm = *i;
        if (LIKELY(fm != nullptr))
          fares += fm->allPaxTypeFare().size();
      }
      publishAsapValue(domain, 5, fares);
    }
  }

  // For Pricing and MIP this is the number of itins,
  // For IS its the size of the flight matrix
  {
    asap_int64 solutions = getSolutionsFound(trx);
    publishAsapValue(domain, 6, solutions);
  }

  publishAsapValue(domain, 7, asapValue(statData[TRX_REQ_SIZE]));
  publishAsapValue(domain, 8, asapValue(statData[TRX_RSP_SIZE]));

  return true;
}

//----------------------------------------------------------------------------
// publishAsapSV01()
//----------------------------------------------------------------------------
bool
TseSrvStats::publishAsapSV01(asap::AsapDomain& domain,
                             const std::vector<double>& statData,
                             const std::string& domainRoot)
{
  if (_acStats != nullptr)
  {
    boost::lock_guard<boost::mutex> guard(_mutex);
    publishAsapValue(domain, 0, _acStats->concurrentTrx, asap::AsapDomain::MAX);
  }

  // Note these numbers are such a mess from one server type to another
  // we couldnt code anything in 'common'
  if (domainRoot == FAREDISPLAY_SVC || domainRoot == HISTORICAL_FAREDISPLAY_SVC)
  {
    publishAsapValue(
        domain, 1, asapValue(statData[SVC_FARES_COLLECTION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 2, asapValue(statData[SVC_FARES_COLLECTION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 3, asapValue(statData[SVC_FARE_SELECTOR_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 4, asapValue(statData[SVC_FARE_SELECTOR_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 5, asapValue(statData[SVC_FARES_VALIDATION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 6, asapValue(statData[SVC_FARES_VALIDATION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain, 7, asapValue(statData[SVC_TAX_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(domain, 8, asapValue(statData[SVC_TAX_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 9, asapValue(statData[SVC_FARE_DISPLAY_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 10, asapValue(statData[SVC_ITIN_ANALYZER_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain,
                     11,
                     asapValue(statData[SVC_REX_FARE_SELECTOR_ELAPSED], true),
                     asap::AsapDomain::AVG);
  }
  else if (domainRoot == IS_SVC)
  {
    publishAsapValue(
        domain, 1, asapValue(statData[SVC_FARES_COLLECTION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 2, asapValue(statData[SVC_FARES_COLLECTION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 3, asapValue(statData[SVC_FARES_VALIDATION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 4, asapValue(statData[SVC_FARES_VALIDATION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain, 5, asapValue(statData[SVC_PRICING_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 6, asapValue(statData[SVC_PRICING_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 7, asapValue(statData[SVC_SHOPPING_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 8, asapValue(statData[SVC_ITIN_ANALYZER_ELAPSED], true), asap::AsapDomain::AVG);
  }
  else if (domainRoot == MIP_SVC || domainRoot == PRICING_SVC ||
           domainRoot == HISTORICAL_PRICING_SVC)
  {
    publishAsapValue(
        domain, 1, asapValue(statData[SVC_FARES_COLLECTION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 2, asapValue(statData[SVC_FARES_COLLECTION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 3, asapValue(statData[SVC_FARES_VALIDATION_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 4, asapValue(statData[SVC_FARES_VALIDATION_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain, 5, asapValue(statData[SVC_PRICING_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 6, asapValue(statData[SVC_PRICING_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain, 7, asapValue(statData[SVC_TAX_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(domain, 8, asapValue(statData[SVC_TAX_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(domain, 9, asapValue(statData[SVC_FARE_CALC_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(
        domain, 10, asapValue(statData[SVC_FARE_CALC_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 11, asapValue(statData[SVC_ITIN_ANALYZER_ELAPSED], true), asap::AsapDomain::AVG);
  }
  else if (domainRoot == TAX_SVC)
  {
    publishAsapValue(domain, 1, asapValue(statData[SVC_TAX_THREADS]), asap::AsapDomain::MAX);
    publishAsapValue(domain, 2, asapValue(statData[SVC_TAX_ELAPSED], true), asap::AsapDomain::AVG);
    publishAsapValue(
        domain, 3, asapValue(statData[SVC_ITIN_ANALYZER_ELAPSED], true), asap::AsapDomain::AVG);
  }

  return true;
}

//----------------------------------------------------------------------------
// publishAsapEX01()
//----------------------------------------------------------------------------
bool
TseSrvStats::publishAsapEX01(asap::AsapDomain& domain, const std::vector<double>& statData)
{
  {
    asap_int64 billOk = static_cast<asap_int64>(statData[BILL_OK]);
    asap_int64 billErr = static_cast<asap_int64>(statData[BILL_ERROR]);
    asap_int64 billTot = billOk + billErr;

    publishAsapValue(domain, 0, billTot);
    publishAsapValue(domain, 1, billErr);

    publishAsapValue(domain, 2, asapValue(statData[BILL_ELAPSED], true));
    publishAsapValue(domain, 3, asapValue(statData[BILL_ELAPSED], true), asap::AsapDomain::MAX);
  }

  {
    asap_int64 rqrsOk = static_cast<asap_int64>(statData[RQRS_OK]);
    asap_int64 rqrsErr = static_cast<asap_int64>(statData[RQRS_ERROR]);
    asap_int64 rqrsTot = rqrsOk + rqrsErr;

    publishAsapValue(domain, 4, rqrsTot);
    publishAsapValue(domain, 5, rqrsErr);

    publishAsapValue(domain, 6, asapValue(statData[RQRS_ELAPSED], true));
    publishAsapValue(domain, 7, asapValue(statData[RQRS_ELAPSED], true), asap::AsapDomain::MAX);
  }

  if (_acStats != nullptr)
  {
    asap_int64 dbErr = 0;
    asap_int64 dbTot = 0;
    double dbElapsed = 0;

    {
      boost::lock_guard<boost::mutex> guard(_mutex);

      asap_int64 dbOk = static_cast<asap_int64>(_acStats->dbOkCount);
      dbErr = static_cast<asap_int64>(_acStats->dbErrorCount);
      dbTot = dbOk + dbErr;

      dbElapsed = _acStats->dbElapsedTime;
    }

    publishAsapValue(domain, 8, dbTot, asap::AsapDomain::REPLACE);
    publishAsapValue(domain, 9, dbErr, asap::AsapDomain::REPLACE);
    publishAsapValue(domain, 10, asapValue(dbElapsed, true), asap::AsapDomain::REPLACE);
  }

  return true;
}

//----------------------------------------------------------------------------
// publishAsapEX02()
//----------------------------------------------------------------------------
bool
TseSrvStats::publishAsapEX02(asap::AsapDomain& domain,
                             const std::vector<double>& statData,
                             const std::string& domainRoot)
{
  if (domainRoot == FAREDISPLAY_SVC || domainRoot == HISTORICAL_FAREDISPLAY_SVC)
  {
    // Fare Display puts RTG where ASv2 usually goes
    asap_int64 rtgOk = static_cast<asap_int64>(statData[RTG_OK]);
    asap_int64 rtgErr = static_cast<asap_int64>(statData[RTG_ERROR]);
    asap_int64 rtgTot = rtgOk + rtgErr;

    publishAsapValue(domain, 0, rtgTot);
    publishAsapValue(domain, 1, rtgErr);

    publishAsapValue(domain, 2, asapValue(statData[RTG_ELAPSED], true));
    publishAsapValue(domain, 3, asapValue(statData[RTG_ELAPSED], true), asap::AsapDomain::MAX);
  }
  else
  {
    asap_int64 ASv2Ok = static_cast<asap_int64>(statData[ASV2_OK]);
    asap_int64 ASv2Err = static_cast<asap_int64>(statData[ASV2_ERROR]);
    asap_int64 ASv2Tot = ASv2Ok + ASv2Err;

    publishAsapValue(domain, 0, ASv2Tot);
    publishAsapValue(domain, 1, ASv2Err);

    publishAsapValue(domain, 2, asapValue(statData[ASV2_ELAPSED], true));
    publishAsapValue(domain, 3, asapValue(statData[ASV2_ELAPSED], true), asap::AsapDomain::MAX);
  }

  if (domainRoot == FAREDISPLAY_SVC || domainRoot == HISTORICAL_FAREDISPLAY_SVC ||
      domainRoot == IS_SVC || domainRoot == PRICING_SVC || domainRoot == HISTORICAL_PRICING_SVC)
  {
    asap_int64 DSSv2Ok = static_cast<asap_int64>(statData[DSSV2_OK]);
    asap_int64 DSSv2Err = static_cast<asap_int64>(statData[DSSV2_ERROR]);
    asap_int64 DSSv2Tot = DSSv2Ok + DSSv2Err;

    publishAsapValue(domain, 4, DSSv2Tot);
    publishAsapValue(domain, 5, DSSv2Err);

    publishAsapValue(domain, 6, asapValue(statData[DSSV2_ELAPSED], true));
    publishAsapValue(domain, 7, asapValue(statData[DSSV2_ELAPSED], true), asap::AsapDomain::MAX);
  }

  // Only AT2PRC has baggage counters
  if (domainRoot == PRICING_SVC || domainRoot == HISTORICAL_PRICING_SVC)
  {
    asap_int64 baggOk = static_cast<asap_int64>(statData[BAGG_OK]);
    asap_int64 baggErr = static_cast<asap_int64>(statData[BAGG_ERROR]);
    asap_int64 baggTot = baggOk + baggErr;

    publishAsapValue(domain, 8, baggTot);
    publishAsapValue(domain, 9, baggErr);

    publishAsapValue(domain, 10, asapValue(statData[BAGG_ELAPSED], true));
    publishAsapValue(domain, 11, asapValue(statData[BAGG_ELAPSED], true), asap::AsapDomain::MAX);
  }

  return true;
}
//----------------------------------------------------------------------------
// updateErrorCounts()
//----------------------------------------------------------------------------
void
TseSrvStats::updateErrorCounts(ErrorResponseException::ErrorResponseCode errorCode)
{
  const MallocContextDisabler context;
  // Lock with mutex
  boost::lock_guard<boost::mutex> guard(_mutex);

  // initialize if not already existing in the map
  if (!_errorCounts.count(errorCode))
  {
    _errorCounts[errorCode] = 0;
  }
  // Increment the error count
  _errorCounts[errorCode]++;
}

//----------------------------------------------------------------------------
// cleanupASAP()
//----------------------------------------------------------------------------
void
TseSrvStats::cleanupASAP()
{
  const MallocContextDisabler context;
  if (_asapID.empty())
    return;

  asap::AsapDomainManager& adm = asap::AsapDomainManager::getManager(_asapID);

  adm.removeDomains();
  asap::AsapDomainManager::removeManagers();
}

//----------------------------------------------------------------------------
// dumpTrxStats()
//----------------------------------------------------------------------------
void
TseSrvStats::dumpTrxStats(std::ostream& oss, Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());
  std::vector<double>& statData = trx.statData();

  if (statData.empty())
    return; // We didnt keep any statistics

  oss << "\n"
      << "==================== External call statistics ======================"
      << "\n"
      << "Service    | Ok | Er | Elapsed  | CPU      | ReqSize    | RspSize "
      << "\n";

  boost::format fmter("%1$10s | %2$2d | %3$2d | %4$8.4f | %5$8.4f | %6$10d | %7$10d\n");

  oss << fmter % "Trx" % statData[TRX_OK] % statData[TRX_ERROR] % statData[TRX_ELAPSED] %
             statData[TRX_CPU] % statData[TRX_REQ_SIZE] % statData[TRX_RSP_SIZE];

  boost::format fmter2("%1$10s | %2$2d | %3$2d | %4$8.4f | %5$8s | %6$10d | %7$10d\n");

  oss << fmter2 % "ASv2" % statData[ASV2_OK] % statData[ASV2_ERROR] % statData[ASV2_ELAPSED] % "" %
             statData[ASV2_REQ_SIZE] % statData[ASV2_RSP_SIZE];

  oss << fmter2 % "DSSv2" % statData[DSSV2_OK] % statData[DSSV2_ERROR] % statData[DSSV2_ELAPSED] %
             "" % statData[DSSV2_REQ_SIZE] % statData[DSSV2_RSP_SIZE];

  oss << fmter2 % "Baggage" % statData[BAGG_OK] % statData[BAGG_ERROR] % statData[BAGG_ELAPSED] %
             "" % statData[BAGG_REQ_SIZE] % statData[BAGG_RSP_SIZE];

  oss << fmter2 % "Billing" % statData[BILL_OK] % statData[BILL_ERROR] % statData[BILL_ELAPSED] %
             "" % statData[BILL_REQ_SIZE] % statData[BILL_RSP_SIZE];

  oss << fmter2 % "Req/Rsp" % statData[RQRS_OK] % statData[RQRS_ERROR] % statData[RQRS_ELAPSED] %
             "" % statData[RQRS_REQ_SIZE] % statData[RQRS_RSP_SIZE];

  oss << fmter2 % "RTG" % statData[RTG_OK] % statData[RTG_ERROR] % statData[RTG_ELAPSED] % "" %
             statData[RTG_REQ_SIZE] % statData[RTG_RSP_SIZE];

  oss << fmter2 % "ABCC" % statData[ABCC_OK] % statData[ABCC_ERROR] % statData[ABCC_ELAPSED] % "" %
             statData[ABCC_REQ_SIZE] % statData[ABCC_RSP_SIZE];

  oss << fmter2 % "S8BRAND" % statData[S8BRAND_OK] % statData[S8BRAND_ERROR] %
             statData[S8BRAND_ELAPSED] % "" % statData[S8BRAND_REQ_SIZE] %
             statData[S8BRAND_RSP_SIZE];
}

//----------------------------------------------------------------------------
// setupTrxStats()
//----------------------------------------------------------------------------
void
TseSrvStats::setupTrxStats(std::vector<double>& statData)
{
  const MallocContextDisabler context;
  statData.reserve(MAX_STAT_DATA_OFFSET);
  for (size_t i = 0; i < MAX_STAT_DATA_OFFSET; ++i)
    statData.push_back(0);
}

double
TseSrvStats::getTrxCPUTime(Trx& trx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(trx.statDataMutex());
  return trx.statData()[TRX_CPU];
}

//----------------------------------------------------------------------------
// mergeTrxStats()
//----------------------------------------------------------------------------
void
TseSrvStats::mergeTrxStats(MultiExchangeTrx& multiTrx)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(multiTrx.statDataMutex());

  std::vector<double>& statData = multiTrx.statData();
  if (statData.empty())
    setupTrxStats(statData);

  if (!multiTrx.skipNewPricingTrx())
    mergeTrxStats(
        statData, multiTrx.newPricingTrx()->statData(), multiTrx.newPricingTrx()->statDataMutex());
  if (!multiTrx.skipExcPricingTrx1())
    mergeTrxStats(statData,
                  multiTrx.excPricingTrx1()->statData(),
                  multiTrx.excPricingTrx1()->statDataMutex());
  if (!multiTrx.skipExcPricingTrx2())
    mergeTrxStats(statData,
                  multiTrx.excPricingTrx2()->statData(),
                  multiTrx.excPricingTrx2()->statDataMutex());
}

//----------------------------------------------------------------------------
// mergeTrxStats()
//----------------------------------------------------------------------------
void
TseSrvStats::mergeTrxStats(std::vector<double>& toStatData,
                           std::vector<double>& fromStatData,
                           boost::mutex& fromMutex)
{
  const MallocContextDisabler context;
  boost::lock_guard<boost::mutex> guard(fromMutex);

  if (!fromStatData.empty())
  {
    // This should never happen, just wanted to be sure
    if (toStatData.size() != fromStatData.size())
      return;

    for (size_t i = 0, j = fromStatData.size(); i < j; ++i)
    {
      toStatData[i] += fromStatData[i];
    }

    fromStatData.clear();
  }
}

//----------------------------------------------------------------------------
// getSolutionsRequested()
//----------------------------------------------------------------------------
size_t
TseSrvStats::getSolutionsRequested(const Trx& trx)
{
  const MallocContextDisabler context;
  size_t rc = 1;

  const std::type_info& trxType = typeid(trx);
  if (trxType == _shoppingType)
  {
    const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
    if (shoppingTrx == nullptr)
      return rc;
    if (shoppingTrx->getRequestedNumOfEstimatedSolutions() > 0)
    {
      return shoppingTrx->getRequestedNumOfEstimatedSolutions();
    }
    return shoppingTrx->getOptions()->getRequestedNumberOfSolutions();
  }
  else if (trxType == _pricingType)
  {
    const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);
    if (pricingTrx == nullptr)
      return rc;

    return pricingTrx->itin().size();
  }
  else if (trxType == _altPricingType)
  {
    const AltPricingTrx* altPricingTrx = dynamic_cast<const AltPricingTrx*>(&trx);
    if (altPricingTrx == nullptr)
      return rc;

    if ((altPricingTrx->altTrxType() == AltPricingTrx::WPA) ||
        (altPricingTrx->altTrxType() == AltPricingTrx::WPA_NOMATCH))
    {
      // use the farecalc number if we have it, otherwise rc to itins
      const FareCalcConfig* fcc = altPricingTrx->fareCalcConfig();
      if (fcc != nullptr)
      {
        AltPricingTrx* altTrx = const_cast<AltPricingTrx*>(altPricingTrx);
        if (altTrx->getRequest()->ticketingAgent()->sabre1SUser() &&
            altTrx->getRequest()->isWpa50())
        {
          return AltPricingTrx::WPA_50_OPTIONS;
        }
        else
        {
          return fcc->wpaFareOptionMaxNo();
        }
      }
    }

    return altPricingTrx->itin().size();
  }
  else if (trxType == _fareDisplayType)
    return 1;
  else if (trxType == _taxType)
  {
    const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(&trx);
    if (taxTrx == nullptr)
      return rc;

    return taxTrx->itin().size();
  }

  return rc;
}

struct ItinIsValidSolution
{
  bool operator()(const Itin* itin) const
  {
    return itin->farePath().empty() == false &&
           itin->errResponseCode() == ErrorResponseException::NO_ERROR;
  }
};

size_t
countMIPSolutions(const std::vector<Itin*>& itins)
{
  size_t res = 0;
  ItinIsValidSolution isValid;
  for (const auto itin : itins)
  {
    if (!isValid(itin))
    {
      continue;
    }

    res += 1 + itin->getSimilarItins().size();
  }

  return res;
}

//----------------------------------------------------------------------------
// getSolutionsFound()
//----------------------------------------------------------------------------
size_t
TseSrvStats::getSolutionsFound(const Trx& trx)
{
  const MallocContextDisabler context;
  size_t rc = 1;

  const std::type_info& trxType = typeid(trx);
  if (trxType == _shoppingType)
  {
    const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
    if (shoppingTrx == nullptr)
      return rc;
    if (shoppingTrx->getTrxType() == PricingTrx::ESV_TRX)
    {
      if (shoppingTrx->getRequestedNumOfEstimatedSolutions() > 0)
      {
        return shoppingTrx->estimateMatrixESV().size() + shoppingTrx->flightMatrixESV().size();
      }
      return shoppingTrx->flightMatrixESV().size();
    }

    if (shoppingTrx->getRequestedNumOfEstimatedSolutions() > 0)
    {
      return shoppingTrx->estimateMatrix().size() + shoppingTrx->flightMatrix().size();
    }
    return shoppingTrx->flightMatrix().size();
  }
  else if ((trxType == _pricingType) || (trxType == _altPricingType))
  {
    const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);
    if (pricingTrx == nullptr)
      return rc;

    if (pricingTrx->getTrxType() == PricingTrx::MIP_TRX)
    {
      rc = countMIPSolutions(pricingTrx->itin());
      return rc;
    }

    const std::vector<FareCalcCollector*>& fccs = pricingTrx->fareCalcCollector();
    if (fccs.size() < 1)
    {
      return pricingTrx->itin().size();
    }
    rc = 0;
    for (const auto fcc : fccs)
    {
      rc += fcc->calcTotalsMap().size();
    }
    return rc;
  }
  else if (trxType == _fareDisplayType)
    return 1;
  else if (trxType == _taxType)
  {
    const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(&trx);
    if (taxTrx == nullptr)
      return rc;

    return taxTrx->itin().size();
  }

  return rc;
}

//----------------------------------------------------------------------------
// updateLoads()
//
// Note this must be called with a lock and nonnull acStats
//----------------------------------------------------------------------------
bool
TseSrvStats::updateLoads()
{
  struct sysinfo si;
  if (sysinfo(&si) != 0)
  {
    LOG4CXX_DEBUG(_logger, "sysinfo call failed");
    return false;
  }

  _acStats->loadLast1Min = (double)si.loads[0] / SYSINFO_SCALE;
  _acStats->loadLast5Min = (double)si.loads[1] / SYSINFO_SCALE;
  _acStats->loadLast15Min = (double)si.loads[2] / SYSINFO_SCALE;

  uint64_t free = (si.freeram + si.bufferram) * si.mem_unit;
  _acStats->freeMemKB = free / uint64_t(SYSINFO_KB_IN_BYTES);

  return true;
}

std::time_t
TseSrvStats::cacheUpdateTimestamp(bool update)
{
  static std::atomic<std::time_t> timestamp(std::time(nullptr));
  if (update)
  {
    timestamp = std::time(nullptr);
  }
  return timestamp;
}
}
