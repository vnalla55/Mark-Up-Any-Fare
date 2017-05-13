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
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetAirlineAllianceCarrier : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineAllianceCarrier(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAirlineAllianceCarrier(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAirlineAllianceCarrier() {};
  virtual const char* getQueryName() const override;

  void findAirlineAllianceCarrierInfo(
      std::vector<tse::AirlineAllianceCarrierInfo*>& AirlineAllianceCarrierInfovec,
      const CarrierCode& carrier);

  static void initialize();

  const QueryGetAirlineAllianceCarrier& operator=(const QueryGetAirlineAllianceCarrier& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAirlineAllianceCarrier& operator=(const std::string& Another)
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

class QueryGetAirlineAllianceCarrierHistorical : public QueryGetAirlineAllianceCarrier
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineAllianceCarrierHistorical(DBAdapter* dbAdapt)
    : QueryGetAirlineAllianceCarrier(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAirlineAllianceCarrierHistorical() {}
  virtual const char* getQueryName() const override;

  void findAirlineAllianceCarrierInfo(
      std::vector<tse::AirlineAllianceCarrierInfo*>& AirlineAllianceCarrierInfovec,
      const CarrierCode& carrier,
      const DateTime& startDate,
      const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

