//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesResBkgDesig.h
//          Description:    QueryGetSvcFeesResBkgDesig.h
//          Created:        11/12/2009
// Authors:
//
//          Updates:
//
//      2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"

namespace tse
{

class QueryGetSvcFeesResBkgDesig : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesResBkgDesig(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesResBkgDesig(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesResBkgDesig() {};
  virtual const char* getQueryName() const override;

  void findSvcFeesResBkgDesigInfo(std::vector<tse::SvcFeesResBkgDesigInfo*>& resBkgDesig,
                                  const VendorCode& vendor,
                                  int itemNo);

  static void initialize();

  const QueryGetSvcFeesResBkgDesig& operator=(const QueryGetSvcFeesResBkgDesig& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesResBkgDesig& operator=(const std::string& Another)
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
}; // class QueryGetSvcFeesResBkgDesig

class QueryGetSvcFeesResBkgDesigHistorical : public QueryGetSvcFeesResBkgDesig
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesResBkgDesigHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesResBkgDesig(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesResBkgDesigHistorical() {}
  virtual const char* getQueryName() const override;

  void findSvcFeesResBkgDesigInfo(std::vector<tse::SvcFeesResBkgDesigInfo*>& resBkgDesig,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesResBkgDesigHistorical
} // namespace tse

