//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
// class Row;
class NeutralValidatingAirline;

class QueryGetNeutralValidatingAirline : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNeutralValidatingAirline(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetNeutralValidatingAirline(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetNeutralValidatingAirline() {}

  virtual const char* getQueryName() const override;

  void findNeutralValidatingAirlineInfo(std::vector<tse::NeutralValidatingAirlineInfo*>& nvaList,
                                        const NationCode& country,
                                        const CrsCode& gds,
                                        const SettlementPlanType& spType);

  static void initialize();

  const QueryGetNeutralValidatingAirline& operator=(const QueryGetNeutralValidatingAirline& rhs);

  const QueryGetNeutralValidatingAirline& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

class QueryGetNeutralValidatingAirlineHistorical : public tse::QueryGetNeutralValidatingAirline
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetNeutralValidatingAirlineHistorical(DBAdapter* dbAdapt)
      : QueryGetNeutralValidatingAirline(dbAdapt, _baseSQL) {}

  virtual ~QueryGetNeutralValidatingAirlineHistorical() {}

  virtual const char* getQueryName() const override;

  void findNeutralValidatingAirlineInfo(std::vector<tse::NeutralValidatingAirlineInfo*>& nvaList,
                                        const NationCode& country,
                                        const CrsCode& gds,
                                        const SettlementPlanType& spType,
                                        const DateTime& startDate,
                                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

}

