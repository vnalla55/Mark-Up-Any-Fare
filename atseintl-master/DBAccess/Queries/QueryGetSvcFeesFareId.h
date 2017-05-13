//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesFareId.h
//          Description:    QueryGetSvcFeesFareId
//          Created:        04/08/2013
//
//     Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is
//     confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesFareIdInfo.h"

namespace tse
{
class QueryGetSvcFeesFareId : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesFareId(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetSvcFeesFareId(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetSvcFeesFareId() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst,
                         const VendorCode& vendor,
                         long long iemNo);

  static void initialize();

  const QueryGetSvcFeesFareId& operator=(const QueryGetSvcFeesFareId& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetSvcFeesFareId& operator=(const std::string& Another)
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

class QueryGetSvcFeesFareIdHistorical : public QueryGetSvcFeesFareId
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesFareIdHistorical(DBAdapter* dbAdapt) : QueryGetSvcFeesFareId(dbAdapt, _baseSQL) {}
  virtual ~QueryGetSvcFeesFareIdHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst,
                         const VendorCode& vendor,
                         long long itemNo,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetAllSvcFeesFareId : public QueryGetSvcFeesFareId
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSvcFeesFareId(DBAdapter* dbAdapt) : QueryGetSvcFeesFareId(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllSvcFeesFareId() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<SvcFeesFareIdInfo*>& lst) { findAllSvcFeesFareId(lst); }
  void findAllSvcFeesFareId(std::vector<SvcFeesFareIdInfo*>& lst);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

