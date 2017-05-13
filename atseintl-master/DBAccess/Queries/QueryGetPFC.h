//----------------------------------------------------------------------------
//          File:           QueryGetPFC.h
//          Description:    QueryGetPFC
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
#include "DBAccess/PfcPFC.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPFC : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPFC(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPFC(DBAdapter* dbAdapt, const std::string& sqlStatement) : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPFC() {}

  virtual const char* getQueryName() const override;

  void findPfcPFC(std::vector<tse::PfcPFC*>& lstPFC, const LocCode& loc);

  static void initialize();
  const QueryGetPFC& operator=(const QueryGetPFC& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPFC& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

  static int checkFlightWildCard(const char* fltStr);
  static int stringToInteger(const char* stringVal, int lineNumber);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPFC

class QueryGetAllPFC : public QueryGetPFC
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPFC(DBAdapter* dbAdapt) : QueryGetPFC(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcPFC*>& lstPFC) { findAllPfcPFC(lstPFC); }

  void findAllPfcPFC(std::vector<tse::PfcPFC*>& lstPFC);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPFC

class QueryGetPFCHistorical : public QueryGetPFC
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPFCHistorical(DBAdapter* dbAdapt) : QueryGetPFC(dbAdapt, _baseSQL) {}
  QueryGetPFCHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetPFC(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPFCHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcPFC(std::vector<tse::PfcPFC*>& lstPFC,
                  const LocCode& loc,
                  const DateTime& startDate,
                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPFCHistorical

} // namespace tse

