//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrier.h
//          Description:    QueryGetInterlineTicketCarrier
//          Created:        10/1/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetInterlineTicketCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetInterlineTicketCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetInterlineTicketCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetInterlineTicketCarrier() {};

  virtual const char* getQueryName() const override;

  void
  findInterlineTicketCarrier(std::vector<tse::InterlineTicketCarrierInfo*>& interlineTicketCarriers,
                             const CarrierCode& carrier);

  static void initialize();

  const QueryGetInterlineTicketCarrier& operator=(const QueryGetInterlineTicketCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetInterlineTicketCarrier& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetInterlineTicketCarrier

class QueryGetInterlineTicketCarrierHistorical : public tse::QueryGetInterlineTicketCarrier
{
public:
  QueryGetInterlineTicketCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetInterlineTicketCarrierHistorical() {};

  virtual const char* getQueryName() const override;

  void
  findInterlineTicketCarrier(std::vector<tse::InterlineTicketCarrierInfo*>& interlineTicketCarriers,
                             const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetInterlineTicketCarrierHistorical

class QueryGetAllInterlineTicketCarrier : public QueryGetInterlineTicketCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineTicketCarrier(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllInterlineTicketCarrier() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::InterlineTicketCarrierInfo*>& InterlineTicketCarrier)
  {
    findAllInterlineTicketCarrier(InterlineTicketCarrier);
  }

  void findAllInterlineTicketCarrier(
      std::vector<tse::InterlineTicketCarrierInfo*>& interlineTicketCarriers);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllInterlineTicketCarrier

class QueryGetAllInterlineTicketCarrierHistorical : public QueryGetInterlineTicketCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineTicketCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllInterlineTicketCarrierHistorical() {};
  virtual const char* getQueryName() const override;

  void
  findAllInterlineTicketCarrier(std::vector<tse::InterlineTicketCarrierInfo*>& interlineCarriers);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllInterlineTicketCarrierHistorical

} // namespace tse

