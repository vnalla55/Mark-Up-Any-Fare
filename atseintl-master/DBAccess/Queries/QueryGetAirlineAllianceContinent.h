// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetAirlineAllianceContinent : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineAllianceContinent(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAirlineAllianceContinent(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAirlineAllianceContinent() {};
  virtual const char* getQueryName() const override;

  void findAirlineAllianceContinentInfo(
      std::vector<tse::AirlineAllianceContinentInfo*>& AirlineAllianceContinentInfovec,
      const GenericAllianceCode& genericAllianceCode);

  static void initialize();

  const QueryGetAirlineAllianceContinent& operator=(const QueryGetAirlineAllianceContinent& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAirlineAllianceContinent& operator=(const std::string& Another)
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
};

class QueryGetAirlineAllianceContinentHistorical : public QueryGetAirlineAllianceContinent
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineAllianceContinentHistorical(DBAdapter* dbAdapt)
    : QueryGetAirlineAllianceContinent(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAirlineAllianceContinentHistorical() {}
  virtual const char* getQueryName() const override;

  void findAirlineAllianceContinentInfo(
      std::vector<tse::AirlineAllianceContinentInfo*>& AirlineAllianceContinentInfovec,
      const GenericAllianceCode& genericAllianceCode,
      const DateTime& startDate,
      const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

