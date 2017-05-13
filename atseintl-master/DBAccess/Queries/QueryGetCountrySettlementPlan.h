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
class CountrySettlementPlanInfo;

class QueryGetCountrySettlementPlan : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCountrySettlementPlan(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetCountrySettlementPlan(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetCountrySettlementPlan() {}

  virtual const char* getQueryName() const override;

  void
  findCountrySettlementPlan(std::vector<tse::CountrySettlementPlanInfo*>& countrySettlementPlans,
                            const NationCode& countryCode);

  static void initialize();

  const QueryGetCountrySettlementPlan& operator=(const QueryGetCountrySettlementPlan& rhs);

  const QueryGetCountrySettlementPlan& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

class QueryGetCountrySettlementPlanHistorical : public tse::QueryGetCountrySettlementPlan
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCountrySettlementPlanHistorical(DBAdapter* dbAdapt)
     : QueryGetCountrySettlementPlan(dbAdapt, _baseSQL) {}

  virtual ~QueryGetCountrySettlementPlanHistorical() {}

  virtual const char* getQueryName() const override;

  void
  findCountrySettlementPlan(std::vector<tse::CountrySettlementPlanInfo*>& cspList,
                            const NationCode& countryCode,
                            const DateTime& startDate,
                            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

}

