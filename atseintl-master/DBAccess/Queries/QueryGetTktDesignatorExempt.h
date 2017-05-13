//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesFareId.h
//          Description:    QueryGetTktDesignatorExempt
//          Created:        12/12/2013
//
//     Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is
//     confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TktDesignatorExemptInfo.h"

namespace tse
{
class QueryGetTktDesignatorExempt : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTktDesignatorExempt(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTktDesignatorExempt(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTktDesignatorExempt() {}
  virtual const char* getQueryName() const override;

  void
  findTktDesignatorExempt(std::vector<TktDesignatorExemptInfo*>& lst, const CarrierCode& carrier);

  static void initialize();

  const QueryGetTktDesignatorExempt& operator=(const QueryGetTktDesignatorExempt& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTktDesignatorExempt& operator=(const std::string& Another)
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
};

class QueryGetTktDesignatorExemptHistorical : public QueryGetTktDesignatorExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTktDesignatorExemptHistorical(DBAdapter* dbAdapt)
    : QueryGetTktDesignatorExempt(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTktDesignatorExemptHistorical() {}
  virtual const char* getQueryName() const override;

  void findTktDesignatorExempt(std::vector<TktDesignatorExemptInfo*>& lst,
                               const CarrierCode& carrier,
                               const DateTime& startDate,
                               const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllTktDesignatorExempt : public QueryGetTktDesignatorExempt
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTktDesignatorExempt(DBAdapter* dbAdapt)
    : QueryGetTktDesignatorExempt(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetAllTktDesignatorExempt() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<TktDesignatorExemptInfo*>& lst) { findAllTktDesignatorExempt(lst); }
  void findAllTktDesignatorExempt(std::vector<TktDesignatorExemptInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

