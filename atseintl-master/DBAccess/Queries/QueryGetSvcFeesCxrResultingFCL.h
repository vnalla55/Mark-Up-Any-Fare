//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesCxrResultingFCL.h
//          Description:    QueryGetSvcFeesCxrResultingFCL.h
//          Created:        11/10/2009
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
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"

namespace tse
{

class QueryGetSvcFeesCxrResultingFCL : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesCxrResultingFCL(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSvcFeesCxrResultingFCL(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSvcFeesCxrResultingFCL() {};
  virtual const char* getQueryName() const override;

  void
  findSvcFeesCxrResultingFCLInfo(std::vector<tse::SvcFeesCxrResultingFCLInfo*>& cxrResultingFCL,
                                 const VendorCode& vendor,
                                 int itemNo);

  static void initialize();

  const QueryGetSvcFeesCxrResultingFCL& operator=(const QueryGetSvcFeesCxrResultingFCL& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSvcFeesCxrResultingFCL& operator=(const std::string& Another)
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
}; // class QueryGetSvcFeesCxrResultingFCL

class QueryGetSvcFeesCxrResultingFCLHistorical : public QueryGetSvcFeesCxrResultingFCL
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSvcFeesCxrResultingFCLHistorical(DBAdapter* dbAdapt)
    : QueryGetSvcFeesCxrResultingFCL(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSvcFeesCxrResultingFCLHistorical() {}
  virtual const char* getQueryName() const override;

  void
  findSvcFeesCxrResultingFCLInfo(std::vector<tse::SvcFeesCxrResultingFCLInfo*>& cxrResultingFCL,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& startDate,
                                 const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSvcFeesCxrResultingFCLHistorical
} // namespace tse

