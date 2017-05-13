//----------------------------------------------------------------------------
//          File:           QueryGetPfcEssAirSvc.h
//          Description:    QueryGetPfcEssAirSvc
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
#include "DBAccess/PfcEssAirSvc.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPfcEssAirSvc : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcEssAirSvc(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPfcEssAirSvc(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPfcEssAirSvc() {}

  virtual const char* getQueryName() const override;

  void findPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS,
                        const LocCode& easHubArpt,
                        const LocCode& easArpt);

  static void initialize();
  const QueryGetPfcEssAirSvc& operator=(const QueryGetPfcEssAirSvc& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPfcEssAirSvc& operator=(const std::string& Another)
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
}; // class QueryGetPfcEssAirSvc

class QueryGetAllPfcEssAirSvc : public QueryGetPfcEssAirSvc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllPfcEssAirSvc(DBAdapter* dbAdapt) : QueryGetPfcEssAirSvc(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::PfcEssAirSvc*>& lstEAS) { findAllPfcEssAirSvc(lstEAS); }

  void findAllPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcEssAirSvc

class QueryGetPfcEssAirSvcHistorical : public QueryGetPfcEssAirSvc
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPfcEssAirSvcHistorical(DBAdapter* dbAdapt) : QueryGetPfcEssAirSvc(dbAdapt, _baseSQL) {}
  virtual ~QueryGetPfcEssAirSvcHistorical() {}

  virtual const char* getQueryName() const override;

  void findPfcEssAirSvc(std::vector<tse::PfcEssAirSvc*>& lstEAS,
                        const LocCode& easHubArpt,
                        const LocCode& easArpt,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllPfcEssAirSvcHistorical

} // namespace tse

