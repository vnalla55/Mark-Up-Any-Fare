//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Adapter/Adapter.h"
#include "Common/TseSynchronizingValue.h"
#include "DataModel/CacheTrx.h"
#include "DBAccess/CacheNotifyInfo.h"
#include "Server/TseServer.h"
#include "Service/Service.h"

#include <log4cxx/helpers/objectptr.h>

#include <set>

#include <stdint.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

// [entitytype]( orderno )
typedef std::tr1::unordered_map<std::string, int64_t> OrderNosByEntityType;
// All order numbers for one cache table
typedef std::tr1::unordered_set<int64_t> ProcessedOrderNos;
typedef ProcessedOrderNos::iterator ProcessedOrderNosIterator;
typedef ProcessedOrderNos::const_iterator ProcessedOrderNosConstIterator;
// All Cache table's order numbers
typedef std::tr1::unordered_map<std::string, ProcessedOrderNos> AllCacheProcessed;
typedef AllCacheProcessed::iterator AllCacheProcessedIterator;
typedef AllCacheProcessed::const_iterator AllCacheProcessedConstIterator;

namespace tse
{
class CacheNotifyAdapter;

class CacheNotifyAdapter final : public Adapter
{
public:
  CacheNotifyAdapter(const std::string&, TseServer& tseServer) : _tseServer(&tseServer) {}

  ~CacheNotifyAdapter() override { shutdown(); }

  bool run(Service& srv);
  bool runIdNo(Service& srv);
  bool runOrderNo(Service& srv);
  bool runOrderNoRefactor(Service& srv);

  void shutdown() override;

  void preShutdown() override;

  static std::vector<int64_t> map2Vector(const OrderNosByEntityType& rhs);
  static std::vector<int64_t> set2Vector(const ProcessedOrderNos& rhs);
  const DateTime nextCutoff(DateTime& last, const bool resync);

  static log4cxx::LoggerPtr getLogger() { return _logger; }

  void processByOrderNo(Service& srv,
                        const std::string& table,
                        std::vector<CacheNotifyInfo>& infos,
                        OrderNosByEntityType& lastOrderNosByEntityType,
                        bool isHistorical = false
                        // Save the last orderno processed for resyncs
                        ,
                        bool recordLast = true);

private:
  bool initialize() override;

  TseServer* _tseServer;
  volatile bool _exiting = false;
  uint64_t _lastFareNotify = 0;
  uint64_t _lastRoutingNotify = 0;
  uint64_t _lastRuleNotify = 0;
  uint64_t _lastSupportNotify = 0;
  uint64_t _lastIntlNotify = 0;
  uint64_t _lastMerchNotify = 0;
  uint64_t _lastHistNotify = 0;
  uint64_t _lastFareHistNotify = 0;
  uint64_t _lastRoutingHistNotify = 0;
  uint64_t _lastRuleHistNotify = 0;
  uint64_t _lastSupportHistNotify = 0;
  uint64_t _lastIntlHistNotify = 0;
  uint64_t _lastMerchHistNotify = 0;

  OrderNosByEntityType _lastFareOrderNosByEntity;
  OrderNosByEntityType _lastRoutingOrderNosByEntity;
  OrderNosByEntityType _lastRuleOrderNosByEntity;
  OrderNosByEntityType _lastSupportOrderNosByEntity;
  OrderNosByEntityType _lastIntlOrderNosByEntity;
  OrderNosByEntityType _lastMerchOrderNosByEntity;
  OrderNosByEntityType _lastHistOrderNosByEntity;
  OrderNosByEntityType _lastFareHistOrderNosByEntity;
  OrderNosByEntityType _lastRoutingHistOrderNosByEntity;
  OrderNosByEntityType _lastRuleHistOrderNosByEntity;
  OrderNosByEntityType _lastSupportHistOrderNosByEntity;
  OrderNosByEntityType _lastIntlHistOrderNosByEntity;
  OrderNosByEntityType _lastMerchHistOrderNosByEntity;

  OrderNosByEntityType _directInjectionOrderNos;

  enum
  { INTERVAL_INSTANCES = 10 };
  // All Cache table's order numbers for the current and prior interval
  AllCacheProcessed _allEvents[INTERVAL_INSTANCES];

  DateTime _cutoff;

  uint32_t _pollInterval = 0;
  uint32_t _processingDelay = 0;
  int _pollSize = 1000;
  TseSynchronizingValue _dbSyncToken;
  std::map<std::string, std::vector<std::string>> _keyFields;
  std::map<std::string, std::vector<std::string>> _cacheIds;
  std::map<std::string, std::vector<std::string>> _historicalKeys;
  std::map<std::string, std::vector<std::string>> _historicalIds;

  class GetFlushIntervals;

  std::map<std::string, DateTime> _nextFlushTime;

  static log4cxx::LoggerPtr _logger;
  static log4cxx::LoggerPtr _ordernologger;
  static log4cxx::LoggerPtr _ordernoSkippedLogger;

  uint64_t process(Service& srv,
                   const std::string& table,
                   std::vector<CacheNotifyInfo>& infos,
                   OrderNosByEntityType& lastOrderNosByEntityType,
                   bool isHistorical = false);

  bool populateTrx(CacheTrx& cacheTrx,
                   CacheUpdateEventPtrSet& uniqueEvents,
                   std::string& entityType,
                   std::string& keyString,
                   bool isHistorical);

  bool clearStaleCaches(Service& srv);

  void updateLastNotifyIDs(const DateTime& cutoff);

  void resyncLastNotifyIDs();

  void nextEventSet(bool resync);
  bool processedPriorEvent(const std::string& cache, size_t orderno);

  const std::vector<std::string>* getKeyFields(std::string& entityType, bool isHistorical);
  const std::vector<std::string>* getCacheIds(std::string& entityType, bool isHistorical);

  int touchThreadAliveFile();

  std::string _threadAliveFile;
  bool _useOrderNo = false;
  size_t _currentInterval = 0;
  uint64_t _intervals = 0;
  bool _usePartitionedQuery = false;
  bool _redoMissingOrderNos = false;
  bool _resyncFromLast = false;
  std::string _resyncFromLastFile;
  std::string _redoMissingOrderNoFile;
  bool _useRefactorOrderNo = false;
};

typedef std::map<std::string, std::vector<std::string>> CacheNotifyControlMap;

class CacheNotifyControlInitializer
{
public:
  CacheNotifyControlInitializer();
  bool isValid() const { return _valid; }
  bool inUse(const std::string& entityType) const;
  const std::vector<std::string>* getCacheIds(const std::string& entityType,
                                              bool isHistorical) const;
  static const CacheNotifyControlInitializer& instance();
private:
  bool _valid{false};
  CacheNotifyControlMap _keyFields;
  CacheNotifyControlMap _cacheIds;
  CacheNotifyControlMap _historicalKeys;
  CacheNotifyControlMap _historicalIds;
};

} // namespace tse
