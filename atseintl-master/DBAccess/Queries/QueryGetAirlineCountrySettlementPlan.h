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
class AirlineCountrySettlementPlanInfo;

class QueryGetAirlineCountrySettlementPlan : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineCountrySettlementPlan(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetAirlineCountrySettlementPlan(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetAirlineCountrySettlementPlan() {}

  virtual const char* getQueryName() const override;

  void
  findAirlineCountrySettlementPlans(std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList,
                                    const NationCode& country,
                                    const CrsCode& gds,
                                    const CarrierCode& airline,
                                    const SettlementPlanType& spType);

  static void initialize();

  const QueryGetAirlineCountrySettlementPlan&
  operator=(const QueryGetAirlineCountrySettlementPlan& rhs);

  const QueryGetAirlineCountrySettlementPlan& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------
class QueryGetAirlineCountrySettlementPlanHistorical : public tse::QueryGetAirlineCountrySettlementPlan
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAirlineCountrySettlementPlanHistorical(DBAdapter* dbAdapt)
    : QueryGetAirlineCountrySettlementPlan(dbAdapt, _baseSQL) {}

  virtual ~QueryGetAirlineCountrySettlementPlanHistorical() {}

  virtual const char* getQueryName() const override;

  void
  findAirlineCountrySettlementPlans(std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList,
                                    const NationCode& country,
                                    const CrsCode& gds,
                                    const CarrierCode& airline,
                                    const SettlementPlanType& spType,
                                    const DateTime& startDate,
                                    const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// GET ALL
//-----------------------------------------------------------------------------
class QueryGetAllAirlineCountrySettlementPlans : public tse::QueryGetAirlineCountrySettlementPlan
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllAirlineCountrySettlementPlans(DBAdapter* dbAdapt)
    : QueryGetAirlineCountrySettlementPlan(dbAdapt, _baseSQL) {}

  virtual ~QueryGetAllAirlineCountrySettlementPlans() {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList)
      { findAirlineCountrySettlementPlans(acspList); }

  void
  findAirlineCountrySettlementPlans(std::vector<tse::AirlineCountrySettlementPlanInfo*>& acspList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

}

