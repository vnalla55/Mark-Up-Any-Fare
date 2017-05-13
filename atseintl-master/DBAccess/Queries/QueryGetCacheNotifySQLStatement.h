//----------------------------------------------------------------------------
//          File:           QueryGetCacheNotifySQLStatement.h
//          Description:    QueryGetCacheNotifySQLStatement
//          Created:        10/29/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCacheNotify.h"
#include "DBAccess/SQLStatement.h"


namespace
{
// Event Query requires the following guarantees.
// 1. order numbers are uniq [ in a cache notification table ]
// 2. ids are uniq [ in a cache notification table ]
// 3. order numbers arrive in sequence within entitytype
// 4. ids are committed in sequence by entitytype

// This query or  similar mechanics are required because  ids can be
// committed out  of sequence  and order numbers  can arrive  out of
// sequence within  one notification table, but by  entity type they
// are unique and in sequence.

const std::string
CurrentInterval("=CURRENT_INTERVAL");
const std::string
TableReplace("=TABLE");
const std::string
TableFareCacheNotifyOrderNo("FareCacheNotify");
const std::string
TableSupportCacheNotifyOrderNo("SupportCacheNotify");
const std::string
TableIntlCacheNotifyOrderNo("IntlCacheNotify");
const std::string
TableHistoricalCacheNotifyOrderNo("HistoricalCacheNotify");
const std::string
TableRuleCacheNotifyOrderNo("RuleCacheNotify");
const std::string
TableRoutingCacheNotifyOrderNo("RoutingCacheNotify");
const std::string
TableMerchandisingCacheNotifyOrderNo("MerchandisingCacheNotify");

const std::string
originalEventQuery(
    " SELECT "
    "   events.id, events.orderno, UPPER(events.entitytype) entitytype, events.onsitetimestamp, "
    "events.keystring "
    " FROM "
    " ( "
    // Build an ordered sub-query so that Oracle's ROWNUM post
    // select processing executes properly. The limit statement is
    // dynamically appended to this text.  Collect all ids by
    // entity type >= the resync ids
    "   SELECT "
    "     events.id, events.orderno, events.entitytype, events.onsitetimestamp, events.keystring "
    "   FROM "
    "     =TABLE events, "
    "   (     "
    // The next row to process may be in the set of already
    // processed. Filter already processed items now.
    "         SELECT next_row.id, next_row.orderno, next_row.entitytype "
    "         FROM =TABLE next_row, "
    "         ( "
    // The current min(ids) [ resync point ] is the
    // minimum of the union of the last id's processed and
    // the currently available maximum ids by entity type.
    "           SELECT "
    "                MIN(sync_point.id) id, sync_point.entitytype "
    "           FROM "
    "             =TABLE sync_point, "
    "             ( "
    // The next id to process or the last id
    // processed by entitytype.
    "                SELECT MIN(event_info.id) id, event_info.entitytype "
    "                FROM "
    "                    =TABLE event_info, "
    "                    ( "
    // The last id for an order number
    // processed by entitytype.
    "                      SELECT MIN(id) id, entitytype "
    "                      FROM =TABLE "
    "                      WHERE orderno IN ( %1n ) "
    "                      GROUP BY entitytype "
    "                    ) processed "
    "                WHERE "
    "                    event_info.entitytype = processed.entitytype "
    "                AND event_info.id > processed.id "
    "                GROUP BY event_info.entitytype "
    // The current max id by entity type
    "                UNION "
    "                SELECT "
    "                    MAX(id) id, entitytype "
    "                FROM "
    "                    =TABLE "
    "                WHERE onsitetimestamp <= %2n "
    "                GROUP BY entitytype "
    "                UNION "
    // Pick up any entitytype not existing in the db
    // prior to the last execution.
    "                SELECT "
    "                    MIN(id) id, entitytype "
    "                FROM "
    "                    =TABLE "
    "                WHERE entitytype NOT IN "
    "                ( "
    "                    SELECT entitytype FROM =TABLE WHERE onsitetimestamp <= %3n GROUP BY "
    "entitytype "
    "                ) "
    "                GROUP BY entitytype "
    "             ) last_or_max "
    "           WHERE "
    "               sync_point.entitytype = last_or_max.entitytype "
    "           AND "
    "               sync_point.id         = last_or_max.id "
    "           GROUP BY "
    "               sync_point.entitytype "
    "        ) last_or_max "
    "        WHERE "
    "            next_row.id = last_or_max.id "
    "   ) criteria "
    "   WHERE "
    "       events.entitytype = criteria.entitytype "
    "   AND "
    "       events.id         >= criteria.id "
    "   ORDER BY entitytype, id "
    " ) events ");

const std::string
originalEventQueryWithProcessingDelay(
    " SELECT "
    "   events.id, events.orderno, UPPER(events.entitytype) entitytype, events.onsitetimestamp, "
    "events.keystring "
    " FROM "
    " ( "
    // Build an ordered sub-query so that Oracle's ROWNUM post
    // select processing executes properly. The limit statement is
    // dynamically appended to this text.  Collect all ids by
    // entity type >= the resync ids
    "   SELECT "
    "     events.id, events.orderno, events.entitytype, events.onsitetimestamp, events.keystring "
    "   FROM "
    "     =TABLE events, "
    "   (     "
    // The next row to process may be in the set of already
    // processed. Filter already processed items now.
    "         SELECT next_row.id, next_row.orderno, next_row.entitytype "
    "         FROM =TABLE next_row, "
    "         ( "
    // The current min(ids) [ resync point ] is the
    // minimum of the union of the last id's processed and
    // the currently available maximum ids by entity type.
    "           SELECT "
    "                MIN(sync_point.id) id, sync_point.entitytype "
    "           FROM "
    "             =TABLE sync_point, "
    "             ( "
    // The next id to process or the last id
    // processed by entitytype.
    "                SELECT MIN(event_info.id) id, event_info.entitytype "
    "                FROM "
    "                    =TABLE event_info, "
    "                    ( "
    // The last id for an order number
    // processed by entitytype.
    "                      SELECT MIN(id) id, entitytype "
    "                      FROM =TABLE "
    "                      WHERE orderno IN ( %1n ) "
    "                      GROUP BY entitytype "
    "                    ) processed "
    "                WHERE "
    "                    event_info.entitytype = processed.entitytype "
    "                AND event_info.id > processed.id "
    "                AND event_info.onsitetimestamp <= %2n "
    "                GROUP BY event_info.entitytype "
    // The current max id by entity type
    "                UNION "
    "                SELECT "
    "                    MAX(id) id, entitytype "
    "                FROM "
    "                    =TABLE "
    "                WHERE onsitetimestamp <= %3n "
    "                GROUP BY entitytype "
    "                UNION "
    // Pick up any entitytype not existing in the db
    // prior to the last execution.
    "                SELECT "
    "                    MIN(id) id, entitytype "
    "                FROM "
    "                    =TABLE "
    "                WHERE entitytype NOT IN "
    "                ( "
    "                    SELECT entitytype FROM =TABLE WHERE onsitetimestamp <= %4n GROUP BY "
    "entitytype "
    "                ) "
    "                GROUP BY entitytype "
    "             ) last_or_max "
    "           WHERE "
    "               sync_point.entitytype = last_or_max.entitytype "
    "           AND "
    "               sync_point.id         = last_or_max.id "
    "           GROUP BY "
    "               sync_point.entitytype "
    "        ) last_or_max "
    "        WHERE "
    "            next_row.id = last_or_max.id "
    "   ) criteria "
    "   WHERE "
    "       events.entitytype = criteria.entitytype "
    "   AND "
    "       events.id         >= criteria.id "
    "   AND "
    "       events.onsitetimestamp <= %5n "
    "   ORDER BY entitytype, id "
    " ) events ");

std::string
eventQueryPartitionedLookup(
    "  SELECT events.id,events.orderno,UPPER(events.entitytype) entitytype,events.onsitetimestamp, "
    "events.keystring  "
    "  FROM "
    "  ( "
    "     SELECT "
    "events.id,events.orderno,events.entitytype,events.onsitetimestamp,events.keystring "
    "     FROM "
    "         ( SELECT * from =TABLE WHERE =CURRENT_INTERVAL %1n ) events "
    "         inner join ( SELECT next_row.id,next_row.orderno,next_row.entitytype FROM ( SELECT * "
    "from =TABLE WHERE =CURRENT_INTERVAL %2n ) next_row "
    "           inner join ( SELECT MIN(sync_point.id) id,sync_point.entitytype FROM ( SELECT * "
    "from =TABLE WHERE =CURRENT_INTERVAL %3n ) sync_point "
    "             inner join ( SELECT MIN(event_info.id) id,event_info.entitytype FROM ( SELECT * "
    "from =TABLE WHERE =CURRENT_INTERVAL %4n ) event_info "
    "               inner join ( "
    "                             SELECT MIN(id) id,entitytype "
    "                             FROM =TABLE "
    "                             WHERE "
    "                                 orderno IN "
    "                                 ( %5n ) "
    "                             GROUP BY entitytype "
    "                           ) processed "
    "                           ON event_info.entitytype = processed.entitytype AND event_info.id "
    "> processed.id "
    "                           GROUP BY event_info.entitytype "
    "                           UNION SELECT MAX(id) id,entitytype FROM ( SELECT * FROM =TABLE "
    "WHERE =CURRENT_INTERVAL %6n ) GROUP BY entitytype "
    // current minute's new entitytypes not seen in the last hour
    // If there's more than one row, then return the first id for that entitytype.
    "                           UNION SELECT MIN(id) id,entitytype FROM ( SELECT * FROM =TABLE "
    "WHERE ONSITETIMESTAMP BETWEEN %7n AND %8n ) "
    "                                        WHERE entitytype NOT IN "
    "                                          ( SELECT entitytype FROM ( SELECT * from =TABLE "
    "WHERE =CURRENT_INTERVAL %9n and ONSITETIMESTAMP < %10n ) GROUP BY entitytype ) "
    "                                      GROUP BY entitytype  "
    "                         ) last_max_or_new "
    "                         ON    sync_point.entitytype = last_max_or_new.entitytype AND "
    "sync_point.id = last_max_or_new.id "
    "                         GROUP BY sync_point.entitytype "
    "                       ) resync_point "
    "                       ON next_row.id = resync_point.id "
    "                     ) criteria "
    "            ON events.entitytype = criteria.entitytype AND events.id >= criteria.id  "
    "     ORDER BY "
    "     entitytype, id, onsitetimestamp  "
    "  ) events  ");
////////////////////////////////////////////////////////////////////////
// If out of sequence commits can cause orderno lossage, then we
// need to track the orderno's from a sequence that have been
// dropped and re-request them periodically
////////////////////////////////////////////////////////////////////////
const std::string
missingOrdernoLookupQuery(
    " SELECT DISTINCT id, orderno, UPPER(entitytype) entitytype, onsitetimestamp, keystring"
    " FROM"
    " ("
    "    SELECT * "
    "    FROM"
    "    ("
    "         SELECT lhs.id, lhs.orderno, lhs.entitytype, lhs.onsitetimestamp, lhs.keystring"
    "         FROM =TABLE lhs INNER JOIN"
    "             ("
    "                SELECT id, entitytype"
    "                FROM =TABLE"
    "                WHERE orderno IN ( %1n )"
    "             ) rhs ON lhs.id = rhs.id"
    "    ) lhs"
    "    ORDER BY entitytype, id"
    " )");

log4cxx::LoggerPtr&
getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyAdapter"));
  return logger;
}

bool
entry(const std::string& entryName)
{
  std::string _name("CACHE_ADP");
  tse::ConfigMan& config = tse::Global::config();
  std::string value("F");
  config.getValue(entryName, value, _name);
  LOG4CXX_INFO(getLogger(), "CACHE_ADP/" << entryName << "[" << value << "]");
  bool rc = false;
  if ((value == "Y") || (value == "y") || (value == "1") || (value == "T") || (value == "t") ||
      (value == "true") || (value == "TRUE") || (value == "yes") || (value == "YES"))
  {
    rc = true;
  }
  return rc;
}

bool
entry(const std::string& entryName, std::string& text)
{
  std::string _name("CACHE_ADP");
  tse::ConfigMan& config = tse::Global::config();
  bool rc = config.getValue(entryName, text, _name);
  LOG4CXX_INFO(getLogger(), "CACHE_ADP/" << entryName << "[" << text << "]");
  return rc;
}

std::string
replaceAll(const std::string& s, const std::string& from, const std::string& to)
{
  std::string original(s);
  size_t position(original.find(from, 0));
  size_t last(0);
  std::string result;
  while (position != std::string::npos)
  {
    result += original.substr(last, position - last);
    result += to;
    last = position + from.length();
    position = original.find(from, last);
  }
  if (last < original.length())
    result += original.substr(last);
  return result;
}

class QueryOrderNoIntervalOptions
{
  static std::string _text;
  static std::string _partitionCurrentIntervalText;
  static bool _partitioned;
  static bool _useProcessingDelay;
  static uint16_t _processingDelay;
  static const std::string& replaceSweepText();
  static unsigned int initialize(bool partitioned);

public:
  static const std::string& sweepInterval(bool partitioned);
  static const std::string& sweepIntervalQuery();
  static bool partitioned() { return _partitioned; }
  static bool useProcessingDelay() { return _useProcessingDelay; }
  static uint16_t processingDelay() { return _processingDelay; }
};

std::string QueryOrderNoIntervalOptions::_text;
std::string QueryOrderNoIntervalOptions::_partitionCurrentIntervalText;

bool QueryOrderNoIntervalOptions::_partitioned = false;
bool QueryOrderNoIntervalOptions::_useProcessingDelay = false;
uint16_t QueryOrderNoIntervalOptions::_processingDelay = 0;

unsigned int
QueryOrderNoIntervalOptions::initialize(bool partitioned)
{
  std::ostringstream o;
  unsigned int _sweepInterval = 0;
  std::string _name("CACHE_ADP");

  std::string text;
  ////////////////////////////////////////////////////////////////////////
  // New query format.
  ////////////////////////////////////////////////////////////////////////
  if (entry("SWEEP_INTERVAL", text))
  {
    _sweepInterval = atoi(text.c_str());
  }

  if (_sweepInterval < 1 || _sweepInterval > 48)
  {
    LOG4CXX_INFO(getLogger(),
                 "Config entry for 'SWEEP_INTERVAL' out of range [1 - 48]: reset to 1 hour.");
    _sweepInterval = 1;
  }

  if (partitioned)
  {
    o.str("");
    o << "ONSITETIMESTAMP BETWEEN SYSTIMESTAMP - " << _sweepInterval << "/24 AND ";
    _partitionCurrentIntervalText = o.str();
  }

  return _sweepInterval;
}

const std::string&
QueryOrderNoIntervalOptions::sweepInterval(bool partitioned)
{
  static unsigned int _sweepInterval = initialize(partitioned);
  (void)_sweepInterval;
  return _text;
}

const std::string&
QueryOrderNoIntervalOptions::replaceSweepText()
{
  _useProcessingDelay = entry("USE_PROCESSING_DELAY");
  std::string text;
  if (useProcessingDelay())
  {
    _processingDelay = tse::QueryCacheNotificationGlobals::processingDelay();
    if (!_processingDelay)
    {
      _useProcessingDelay = false;
      LOG4CXX_ERROR(getLogger(),
                    "Configured for 'USE_PROCESSING_DELAY' but 'PROCESSING_DELAY' not set.");
    }
  }

  if (entry("USE_ORDERNO"))
  {
    if (entry("USE_PARTITIONED_ORDERNO"))
    {
      QueryOrderNoIntervalOptions::sweepInterval(true);
      _text =
          replaceAll(eventQueryPartitionedLookup, CurrentInterval, _partitionCurrentIntervalText);
      _partitioned = true;
      LOG4CXX_DEBUG(getLogger(), "Configured for 'USE_PARTITIONED_ORDERNO'");
      LOG4CXX_DEBUG(getLogger(), "'USE_PARTITIONED_ORDERNO'" << _text);
    }
    else // if ( entry("USE_ORIGINAL_ORDERNO" ) )
    {
      if (!useProcessingDelay())
      {
        _text = originalEventQuery;
        LOG4CXX_INFO(getLogger(), "Configured for 'USE_ORIGINAL_ORDERNO'");
      }
      else
      {
        _text = originalEventQueryWithProcessingDelay;
        LOG4CXX_INFO(getLogger(), "Configured for 'USE_PROCESSING_DELAY'");
      }
    }
  }
  return _text;
}

const std::string&
QueryOrderNoIntervalOptions::sweepIntervalQuery()
{
  static std::string _sweepIntervalText = replaceSweepText();
  return _sweepIntervalText;
}
}

namespace tse
{

class Row;

////////////////////////////////////////////////////////////////////////
// QueryGetAllFareCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareCacheNotifySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllFareCacheNotifySQLStatement() {};
  virtual ~QueryGetAllFareCacheNotifySQLStatement() {};

  enum ColumnIndexes
  {
    ID = 0,
    ORDERNO,
    ENTITYTYPE,
    CREATEDATE,
    KEYSTRING,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ID,ORDERNO,upper(ENTITYTYPE) ENTITYTYPE,CREATEDATE,KEYSTRING");
    this->From("=FARECACHENOTIFY ");
    // this->Where("ID > %1n limit %2n");

    std::string where("ID > %1n");
    this->Where(where);

    std::string limit;
    this->generateLimitString("%2n", limit);
    this->Limit(limit);

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapResultToCacheNotify(std::vector<CacheNotifyInfo>& infos, DBResultSet& res)
  {
    Row* row;
    while ((row = res.nextRow()))
    {
      infos.resize(infos.size() + 1);
      CacheNotifyInfo& info = infos.back();
      info.id() = row->getInt(ID);
      info.orderno() = row->getInt(ORDERNO);
      info.entityType() = row->getString(ENTITYTYPE);
      if (info.entityType()[0] == '=')
        info.entityType().erase(0, 1);
      info.createDate() = row->getDate(CREATEDATE);
      info.keyString() = row->getString(KEYSTRING);
    }
  } // mapResultToCacheNotify()

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
// QueryGetLastFareCacheNotify
//
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastFareCacheNotifySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetLastFareCacheNotifySQLStatement() {};
  virtual ~QueryGetLastFareCacheNotifySQLStatement() {};

  enum ColumnIndexes
  {
    MAX = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select max(ID) MAX");
    this->From("=FARECACHENOTIFY");
    this->Where("CREATEDATE < %1n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static uint64_t getLastNotifyId(DBResultSet& res)
  {
    Row* row = res.nextRow();
    if (row == nullptr)
      return 0;

    if (row->isNull(MAX))
      return 0;

    return row->getLongLong(MAX);
  } // getLastNotifyId()

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRuleCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllRuleCacheNotifySQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=RULECACHENOTIFY "); };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRoutingCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllRoutingCacheNotifySQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=ROUTINGCACHENOTIFY "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSupportCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSupportCacheNotifySQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{

protected:
  void adjustBaseSQL() override { this->From("=SUPPORTCACHENOTIFY "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllIntlCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllIntlCacheNotifySQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=INTLCACHENOTIFY "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCacheNotifyHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMerchandisingCacheNotifySQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=MERCHANDISINGCACHENOTIFY "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCacheNotifyHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCacheNotifyHistoricalSQLStatement
    : public QueryGetAllFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=HISTORICALCACHENOTIFY "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastRuleCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastRuleCacheNotifySQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=RULECACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastRoutingCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastRoutingCacheNotifySQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=ROUTINGCACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastSupportCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastSupportCacheNotifySQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=SUPPORTCACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastIntlCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastIntlCacheNotifySQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=INTLCACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastIntlCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastMerchandisingCacheNotifySQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=MERCHANDISINGCACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetLastCacheNotifyHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetLastCacheNotifyHistoricalSQLStatement
    : public QueryGetLastFareCacheNotifySQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override { this->From("=HISTORICALCACHENOTIFY"); }
};

////////////////////////////////////////////////////////////////////////
// QueryGetAllFareCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareCacheNotifyOrderNoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllFareCacheNotifyOrderNoSQLStatement() {};
  virtual ~QueryGetAllFareCacheNotifyOrderNoSQLStatement() {};

  const std::string& generateEventQuery()
  {
    static std::string text(QueryOrderNoIntervalOptions::sweepIntervalQuery());
    return text;
  }

  const std::string& eventQuery()
  {
    static std::string _eventQuery = generateEventQuery();
    return _eventQuery;
  }

  enum ColumnIndexes
  {
    ID = 0,
    ORDERNO,
    ENTITYTYPE,
    ONSITETIMESTAMP,
    KEYSTRING,
    NUMBEROFCOLUMNS
  }; // enum

  virtual const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapResult(std::vector<CacheNotifyInfo>& infos, DBResultSet& res)
  {
    Row* row;
    while ((row = res.nextRow()))
    {
      infos.resize(infos.size() + 1);
      CacheNotifyInfo& info = infos.back();
      info.id() = row->getLongLong(ID);
      info.orderno() = row->getLongLong(ORDERNO);
      info.entityType() = row->getString(ENTITYTYPE);
      if (info.entityType()[0] == '=')
        info.entityType().erase(0, 1);
      info.createDate() = row->getDate(ONSITETIMESTAMP);
      info.keyString() = row->getString(KEYSTRING);
    }
  } // mapResultToCacheNotify()

  static void parameterSubstitution(tse::SQLQuery& query,
                                    const orderNoList& lastOrderNos,
                                    DateTime& priorCutoff,
                                    const DateTime& cutoff,
                                    int pollSize)
  {
    if (partitioned())
    {
      query.substParm(1, cutoff);
      query.substParm(2, cutoff);
      query.substParm(3, cutoff);
      query.substParm(4, cutoff);
      query.substParm(5, lastOrderNos);
      query.substParm(6, cutoff);
      query.substParm(7, priorCutoff);
      query.substParm(8, cutoff);
      query.substParm(9, priorCutoff);
      query.substParm(10, priorCutoff);
      query.substParm(11, pollSize);
      priorCutoff = cutoff;
    }
    else
    {
      tse::DateTime when(cutoff);
      if (useProcessingDelay())
      {
        when = cutoff.subtractSeconds(processingDelay());
      }
      query.substParm(1, lastOrderNos);
      query.substParm(2, when);
      query.substParm(3, when);
      if (!useProcessingDelay())
      {
        query.substParm(4, pollSize);
      }
      else
      {
        query.substParm(4, when);
        query.substParm(5, when);
        query.substParm(6, pollSize);
      }
    }
  }
  static bool partitioned() { return QueryOrderNoIntervalOptions::partitioned(); }
  static bool useProcessingDelay() { return QueryOrderNoIntervalOptions::useProcessingDelay(); }
  static uint16_t processingDelay() { return QueryOrderNoIntervalOptions::processingDelay(); }

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction

  virtual void adjustBaseSQL()
  {
    this->Command(replaceAll(eventQuery(), TableReplace, TableFareCacheNotifyOrderNo));
    this->appendLimit();
  }

  void appendLimit()
  {
    std::string limit;
    if (partitioned())
      this->generateLimitString("%11n", limit);
    else
    {
      if (!useProcessingDelay())
      {
        this->generateLimitString("%4n", limit);
      }
      else
      {
        this->generateLimitString("%6n", limit);
      }
    }
    this->Limit(limit);
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRuleCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllRuleCacheNotifyOrderNoSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(this->eventQuery(), TableReplace, TableRuleCacheNotifyOrderNo));
    this->appendLimit();
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRoutingCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllRoutingCacheNotifyOrderNoSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(this->eventQuery(), TableReplace, TableRoutingCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSupportCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSupportCacheNotifyOrderNoSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{

protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(this->eventQuery(), TableReplace, TableSupportCacheNotifyOrderNo));
    this->appendLimit();
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllIntlCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllIntlCacheNotifyOrderNoSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(this->eventQuery(), TableReplace, TableIntlCacheNotifyOrderNo));
    this->appendLimit();
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllMerchandisingCacheNotifyOrderNoSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMerchandisingCacheNotifyOrderNoSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(
        replaceAll(this->eventQuery(), TableReplace, TableMerchandisingCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCacheNotifyHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCacheNotifyOrderNoHistoricalSQLStatement
    : public QueryGetAllFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(this->eventQuery(), TableReplace, TableHistoricalCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for Missing order numbers
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingFareCacheNotifyOrderNoSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    ID = 0,
    ORDERNO,
    ENTITYTYPE,
    ONSITETIMESTAMP,
    KEYSTRING,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& eventQuery()
  {
    static std::string _eventQuery = missingOrdernoLookupQuery;
    return _eventQuery;
  }

  virtual const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapResult(std::vector<CacheNotifyInfo>& infos, DBResultSet& res)
  {
    Row* row;
    while ((row = res.nextRow()))
    {
      infos.resize(infos.size() + 1);
      CacheNotifyInfo& info = infos.back();
      info.id() = row->getLongLong(ID);
      info.orderno() = row->getLongLong(ORDERNO);
      info.entityType() = row->getString(ENTITYTYPE);
      if (info.entityType()[0] == '=')
        info.entityType().erase(0, 1);
      info.createDate() = row->getDate(ONSITETIMESTAMP);
      info.keyString() = row->getString(KEYSTRING);
    }
  } // mapResultToCacheNotify()

  static void parameterSubstitutionMissingOrderno(tse::SQLQuery& query, const orderNoList& missing)
  {
    query.substParm(1, missing);
  }

protected:
  virtual void adjustBaseSQL()
  {
    this->Command(replaceAll(eventQuery(), TableReplace, TableFareCacheNotifyOrderNo));
    this->appendLimit();
  };

  void appendLimit()
  {
    //     std::string limit;
    //     this->generateLimitString("%2n", limit);
    //     this->Limit(limit);
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRuleCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingRuleCacheNotifyOrderNoSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(missingOrdernoLookupQuery, TableReplace, TableRuleCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllRoutingCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingRoutingCacheNotifyOrderNoSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(
        replaceAll(missingOrdernoLookupQuery, TableReplace, TableRoutingCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSupportCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingSupportCacheNotifyOrderNoSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{

protected:
  void adjustBaseSQL() override
  {
    this->Command(
        replaceAll(missingOrdernoLookupQuery, TableReplace, TableSupportCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllIntlCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingIntlCacheNotifyOrderNoSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(replaceAll(missingOrdernoLookupQuery, TableReplace, TableIntlCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllMerchandisingCacheNotify
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingMerchandisingCacheNotifyOrderNoSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(
        replaceAll(missingOrdernoLookupQuery, TableReplace, TableMerchandisingCacheNotifyOrderNo));
    this->appendLimit();
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCacheNotifyHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMissingCacheNotifyOrderNoHistoricalSQLStatement
    : public QueryGetMissingFareCacheNotifyOrderNoSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->Command(
        replaceAll(missingOrdernoLookupQuery, TableReplace, TableHistoricalCacheNotifyOrderNo));
    this->appendLimit();
  };
};

} // namespace tse
