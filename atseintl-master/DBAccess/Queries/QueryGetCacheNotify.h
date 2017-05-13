//----------------------------------------------------------------------------
//          File:           QueryGetCacheNotify.h
//          Description:    QueryGetCacheNotify
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/CacheNotifyInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


struct QueryCacheNotificationGlobals
{
  static uint32_t processingDelay();
};

class QueryGetAllFareCacheNotify : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareCacheNotify(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllFareCacheNotify(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllFareCacheNotify() {};

  const char* getQueryName() const override;

  void findAllFareCacheNotify(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

  const QueryGetAllFareCacheNotify& operator=(const QueryGetAllFareCacheNotify& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllFareCacheNotify& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareCacheNotify

class QueryGetAllRuleCacheNotify : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRuleCacheNotify(DBAdapter* dbAdapt) : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllRuleCacheNotify(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCacheNotify

class QueryGetAllRoutingCacheNotify : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRoutingCacheNotify(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void
  findAllRoutingCacheNotify(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRoutingCacheNotify

class QueryGetAllSupportCacheNotify : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSupportCacheNotify(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void
  findAllSupportCacheNotify(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSupportCacheNotify

class QueryGetAllIntlCacheNotify : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIntlCacheNotify(DBAdapter* dbAdapt) : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllIntlCacheNotify(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIntlCacheNotify

class QueryGetAllMerchandisingCacheNotify : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMerchandisingCacheNotify(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllMerchandisingCacheNotify(std::vector<tse::CacheNotifyInfo>& infos,
                                       int lastId,
                                       int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMerchandisingCacheNotify

class QueryGetAllCacheNotifyHistorical : public QueryGetAllFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCacheNotifyHistorical(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void
  findAllCacheNotifyHistorical(std::vector<tse::CacheNotifyInfo>& infos, int lastId, int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCacheNotifyHistorical

class QueryGetLastFareCacheNotify : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastFareCacheNotify(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetLastFareCacheNotify(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetLastFareCacheNotify() {};

  const char* getQueryName() const override;

  uint64_t findLastFareCacheNotify(const DateTime& startTime);

  static void initialize();

  const QueryGetLastFareCacheNotify& operator=(const QueryGetLastFareCacheNotify& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLastFareCacheNotify& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastFareCacheNotify

class QueryGetLastRuleCacheNotify : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastRuleCacheNotify(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastRuleCacheNotify(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastRuleCacheNotify

class QueryGetLastRoutingCacheNotify : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastRoutingCacheNotify(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastRoutingCacheNotify(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastRoutingCacheNotify

class QueryGetLastSupportCacheNotify : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastSupportCacheNotify(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastSupportCacheNotify(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastSupportCacheNotify

class QueryGetLastIntlCacheNotify : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastIntlCacheNotify(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastIntlCacheNotify(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastIntlCacheNotify

class QueryGetLastMerchandisingCacheNotify : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastMerchandisingCacheNotify(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastMerchandisingCacheNotify(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastMerchandisingCacheNotify

class QueryGetLastCacheNotifyHistorical : public QueryGetLastFareCacheNotify
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLastCacheNotifyHistorical(DBAdapter* dbAdapt)
    : QueryGetLastFareCacheNotify(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  uint64_t findLastCacheNotifyHistorical(const DateTime& startTime);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLastCacheNotifyHistorical

typedef std::vector<int64_t> orderNoList;

////////////////////////////////////////////////////////////////////////
// GetAll*CacheNotifyOrderNo
////////////////////////////////////////////////////////////////////////
class QueryGetAllFareCacheNotifyOrderNo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareCacheNotifyOrderNo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllFareCacheNotifyOrderNo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllFareCacheNotifyOrderNo() {};

  const char* getQueryName() const override;

  void findAllFareCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                     const orderNoList& lastOrderNos,
                                     DateTime& priorCutoff,
                                     const DateTime& lastRunTime,
                                     int pollSize);

  static void initialize();

  const QueryGetAllFareCacheNotifyOrderNo&
  operator=(const QueryGetAllFareCacheNotifyOrderNo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllFareCacheNotifyOrderNo& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareCacheNotifyOrderNo

class QueryGetAllRuleCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRuleCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllRuleCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                     const orderNoList& lastOrderNos,
                                     DateTime& priorCutoff,
                                     const DateTime& cutoff,
                                     int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCacheNotifyOrderNo

class QueryGetAllRoutingCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllRoutingCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllRoutingCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                        const orderNoList& lastOrderNos,
                                        DateTime& priorCutoff,
                                        const DateTime& cutoff,
                                        int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRoutingCacheNotifyOrderNo

class QueryGetAllSupportCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSupportCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllSupportCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                        const orderNoList& lastOrderNos,
                                        DateTime& priorCutoff,
                                        const DateTime& cutoff,
                                        int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSupportCacheNotifyOrderNo

class QueryGetAllIntlCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIntlCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllIntlCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                     const orderNoList& lastOrderNos,
                                     DateTime& priorCutoff,
                                     const DateTime& cutoff,
                                     int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIntlCacheNotifyOrderNo

class QueryGetAllMerchandisingCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMerchandisingCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllMerchandisingCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                              const orderNoList& lastOrderNos,
                                              DateTime& priorCutoff,
                                              const DateTime& cutoff,
                                              int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMerchandisingCacheNotifyOrderNo

class QueryGetAllCacheNotifyOrderNoHistorical : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCacheNotifyOrderNoHistorical(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findAllCacheNotifyOrderNoHistorical(std::vector<tse::CacheNotifyInfo>& infos,
                                           const orderNoList& lastOrderNos,
                                           DateTime& priorCutoff,
                                           const DateTime& cutoff,
                                           int pollSize);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCacheNotifyOrderNoHistorical

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Missing Orderno Pickup Queries
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Pick up out of sequence missing order numbers.

class QueryGetMissingFareCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingFareCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingFareCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                         const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCacheNotifyOrderNo

class QueryGetMissingRuleCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingRuleCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingRuleCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                         const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRuleCacheNotifyOrderNo

class QueryGetMissingRoutingCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingRoutingCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingRoutingCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                            const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllRoutingCacheNotifyOrderNo

class QueryGetMissingSupportCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingSupportCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingSupportCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                            const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSupportCacheNotifyOrderNo

class QueryGetMissingIntlCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingIntlCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingIntlCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                         const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIntlCacheNotifyOrderNo

class QueryGetMissingMerchandisingCacheNotifyOrderNo : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingMerchandisingCacheNotifyOrderNo(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingMerchandisingCacheNotifyOrderNo(std::vector<tse::CacheNotifyInfo>& infos,
                                                  const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMerchandisingCacheNotifyOrderNo

class QueryGetMissingCacheNotifyOrderNoHistorical : public QueryGetAllFareCacheNotifyOrderNo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMissingCacheNotifyOrderNoHistorical(DBAdapter* dbAdapt)
    : QueryGetAllFareCacheNotifyOrderNo(dbAdapt, _baseSQL) {};

  const char* getQueryName() const override;

  void findMissingCacheNotifyOrderNoHistorical(std::vector<tse::CacheNotifyInfo>& infos,
                                               const orderNoList& missing);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCacheNotifyOrderNoHistorical

} // namespace tse

