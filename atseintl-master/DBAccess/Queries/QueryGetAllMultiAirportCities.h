//----------------------------------------------------------------------------
//          File:           QueryGetAllMultiAirportCities.h
//          Description:    QueryGetAllMultiAirportCities
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
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/MultiAirportsByCity.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMultiAirportCity : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiAirportCity(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMultiAirportCity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMultiAirportCity() {};

  virtual const char* getQueryName() const override;

  void findMultiAirportCity(std::vector<tse::MultiAirportCity*>& multiAirportCities,
                            const LocCode& airportCode);

  static void initialize();

  const QueryGetMultiAirportCity& operator=(const QueryGetMultiAirportCity& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMultiAirportCity& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMultiAirportCity

class QueryGetAllMultiAirportCities : public QueryGetMultiAirportCity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMultiAirportCities(DBAdapter* dbAdapt)
    : QueryGetMultiAirportCity(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMultiAirportCities() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MultiAirportCity*>& multiAirportCities)
  {
    findAllMultiAirportCities(multiAirportCities);
  }

  void findAllMultiAirportCities(std::vector<tse::MultiAirportCity*>& multiAirportCities);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMultiAirportCities

class QueryGetAllMultiAirportsByCity : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMultiAirportsByCity(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllMultiAirportsByCity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllMultiAirportsByCity() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MultiAirportsByCity*>& multiAirportsByCity)
  {
    findAllMultiAirportsByCity(multiAirportsByCity);
  }

  void findAllMultiAirportsByCity(std::vector<tse::MultiAirportsByCity*>& multiAirportsByCity);

  static void initialize();

  const QueryGetAllMultiAirportsByCity& operator=(const QueryGetAllMultiAirportsByCity& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllMultiAirportsByCity& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMultiAirportsByCity
} // namespace tse

