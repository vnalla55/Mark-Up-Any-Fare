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
class AirlineInterlineAgreementInfo;

class QueryGetAirlineInterlineAgreement : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineInterlineAgreement(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetAirlineInterlineAgreement(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetAirlineInterlineAgreement() {}

  virtual const char* getQueryName() const override;

  void findAirlineInterlineAgreement(std::vector<tse::AirlineInterlineAgreementInfo*>& aiaList,
                                     const NationCode& country,
                                     const CrsCode& gds,
                                     const CarrierCode& carrier);

  static void initialize();

  const QueryGetAirlineInterlineAgreement& operator=(const QueryGetAirlineInterlineAgreement& rhs);

  const QueryGetAirlineInterlineAgreement& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

class QueryGetAirlineInterlineAgreementHistorical : public tse::QueryGetAirlineInterlineAgreement
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineInterlineAgreementHistorical(DBAdapter* dbAdapt)
     : QueryGetAirlineInterlineAgreement(dbAdapt, _baseSQL) {}

  virtual ~QueryGetAirlineInterlineAgreementHistorical() {}

  virtual const char* getQueryName() const override;

  void findAirlineInterlineAgreement(std::vector<tse::AirlineInterlineAgreementInfo*>& aiaList,
                                     const NationCode& country,
                                     const CrsCode& gds,
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

