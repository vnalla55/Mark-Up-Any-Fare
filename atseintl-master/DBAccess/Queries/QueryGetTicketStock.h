//----------------------------------------------------------------------------
//          File:           QueryGetTicketStock.h
//          Description:    QueryGetTicketStock
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TicketStock.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetTicketStock : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTicketStock(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTicketStock(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTicketStock() {}

  virtual const char* getQueryName() const override;

  void findTicketStock(std::vector<tse::TicketStock*>& lstTS, int ticketStockCode);

  static void initialize();

  const QueryGetTicketStock& operator=(const QueryGetTicketStock& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTicketStock& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTicketStock

class QueryGetTicketStockHistorical : public tse::QueryGetTicketStock
{
public:
  QueryGetTicketStockHistorical(DBAdapter* dbAdapt) : QueryGetTicketStock(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTicketStockHistorical() {}

  virtual const char* getQueryName() const override;

  void findTicketStock(std::vector<tse::TicketStock*>& lstTS, int ticketStockCode);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTicketStockHistorical

class QueryGetAllTicketStock : public QueryGetTicketStock
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTicketStock(DBAdapter* dbAdapt) : QueryGetTicketStock(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllTicketStock() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::TicketStock*>& lstTS) { findAllTicketStock(lstTS); }

  void findAllTicketStock(std::vector<tse::TicketStock*>& lstTS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTicketStock

class QueryGetAllTicketStockHistorical : public QueryGetTicketStock
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTicketStockHistorical(DBAdapter* dbAdapt) : QueryGetTicketStock(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllTicketStockHistorical() {}
  virtual const char* getQueryName() const override;

  void findAllTicketStock(std::vector<tse::TicketStock*>& lstTS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTicketStockHistorical
} // namespace tse

