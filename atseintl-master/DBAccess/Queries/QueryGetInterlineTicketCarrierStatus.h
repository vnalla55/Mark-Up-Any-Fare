//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrierStatus.h
//          Description:    QueryGetInterlineTicketCarrierStatus
//          Created:        02/27/2012
//          Authors:        M Dantas
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
#include "DBAccess/InterlineTicketCarrierStatus.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetInterlineTicketCarrierStatus : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetInterlineTicketCarrierStatus(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetInterlineTicketCarrierStatus(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetInterlineTicketCarrierStatus() {};

  virtual const char* getQueryName() const override;

  void findInterlineTicketCarrierStatus(
      std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus,
      const CarrierCode& carrier,
      const CrsCode& crscode);

  static void initialize();

  const QueryGetInterlineTicketCarrierStatus&
  operator=(const QueryGetInterlineTicketCarrierStatus& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetInterlineTicketCarrierStatus& operator=(const std::string& Another)
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
}; // class QueryGetInterlineTicketCarrierStatus

class QueryGetInterlineTicketCarrierStatusHistorical
    : public tse::QueryGetInterlineTicketCarrierStatus
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetInterlineTicketCarrierStatusHistorical(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrierStatus(dbAdapt, _baseSQL) {};
  virtual ~QueryGetInterlineTicketCarrierStatusHistorical() {};

  virtual const char* getQueryName() const override;

  void findInterlineTicketCarrierStatus(
      std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus,
      const CarrierCode& carrier,
      const CrsCode& crscode);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetInterlineTicketCarrierStatusHistorical

class QueryGetAllInterlineTicketCarrierStatus : public QueryGetInterlineTicketCarrierStatus
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineTicketCarrierStatus(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrierStatus(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllInterlineTicketCarrierStatus() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::InterlineTicketCarrierStatus*>& InterlineTicketCarrierStatus)
  {
    findAllInterlineTicketCarrierStatus(InterlineTicketCarrierStatus);
  }

  void findAllInterlineTicketCarrierStatus(
      std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllInterlineTicketCarrier

class QueryGetAllInterlineTicketCarrierStatusHistorical
    : public QueryGetInterlineTicketCarrierStatus
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllInterlineTicketCarrierStatusHistorical(DBAdapter* dbAdapt)
    : QueryGetInterlineTicketCarrierStatus(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllInterlineTicketCarrierStatusHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllInterlineTicketCarrierStatus(
      std::vector<tse::InterlineTicketCarrierStatus*>& interlineTicketCarriersStatus);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllInterlineTicketCarrierStatusHistorical

} // namespace tse

