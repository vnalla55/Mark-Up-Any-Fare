//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesAccountCode.h
//          Description:    QueryGetSvcFeesAccountCode.h
//          Created:        3/6/2009
// Authors:
//
//          Updates:
//
//      2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"

namespace tse
{

class QueryGetSvcFeesAccountCode : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesAccountCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesAccountCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesAccountCode() {};
  virtual const char* getQueryName() const override;

  void findSvcFeesAccCodeInfo(std::vector<tse::SvcFeesAccCodeInfo*>& accCode,
                              const VendorCode& vendor,
                              int itemNo);

  static void initialize();

  const QueryGetSvcFeesAccountCode& operator=(const QueryGetSvcFeesAccountCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesAccountCode& operator=(const std::string& Another)
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
}; // class QueryGetSvcFeesAccountCode

class QueryGetSvcFeesAccountCodeHistorical : public QueryGetSvcFeesAccountCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesAccountCodeHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesAccountCode(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesAccountCodeHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesAccCodeInfo(std::vector<tse::SvcFeesAccCodeInfo*>& accCode,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesAccountCodeHistorical
} // namespace tse

