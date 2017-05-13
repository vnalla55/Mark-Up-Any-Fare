//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
// class Row;
class PassengerAirlineInfo;

class QueryGetPassengerAirline : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPassengerAirline(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPassengerAirline(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetPassengerAirline() {}

  virtual const char* getQueryName() const override;

  void findPassengerAirline(std::vector<PassengerAirlineInfo*>& paiList,
                             const CarrierCode& airlineCode);

  static void initialize();

  const QueryGetPassengerAirline& operator=(const QueryGetPassengerAirline& rhs);

  const QueryGetPassengerAirline& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

class QueryGetPassengerAirlineHistorical : public tse::QueryGetPassengerAirline
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPassengerAirlineHistorical(DBAdapter* dbAdapt)
    : QueryGetPassengerAirline(dbAdapt, _baseSQL) {}

  virtual ~QueryGetPassengerAirlineHistorical() {}

  virtual const char* getQueryName() const override;

  void findPassengerAirline(std::vector<PassengerAirlineInfo*>& paiList,
                            const CarrierCode& airlineCode,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

}

