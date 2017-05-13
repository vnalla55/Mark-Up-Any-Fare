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
#include "DBAccess/SQLQuery.h"

namespace tse
{
class GenSalesAgentInfo;

class QueryGetGenSalesAgent : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGenSalesAgent(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetGenSalesAgent(DBAdapter* dbAdapt, const std::string& sqlStmt) : SQLQuery(dbAdapt, sqlStmt)
  {
  }

  virtual ~QueryGetGenSalesAgent() {};
  virtual const char* getQueryName() const override;

  void findGenSalesAgents(std::vector<GenSalesAgentInfo*>& col,
                          const CrsCode& gds,
                          const NationCode& country,
                          const SettlementPlanType& settlementPlan,
                          const CarrierCode& validatingCxr);

  static void initialize();

  const QueryGetGenSalesAgent& operator=(const QueryGetGenSalesAgent& rhs);
  const QueryGetGenSalesAgent& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

class QueryGetGenSalesAgentHistorical : public tse::QueryGetGenSalesAgent
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetGenSalesAgentHistorical(DBAdapter* dbAdapt)
      : QueryGetGenSalesAgent(dbAdapt, _baseSQL) {}

  virtual ~QueryGetGenSalesAgentHistorical() {};
  virtual const char* getQueryName() const override;

  void findGenSalesAgents(std::vector<GenSalesAgentInfo*>& col,
                          const CrsCode& gds,
                          const NationCode& country,
                          const SettlementPlanType& settlementPlan,
                          const CarrierCode& validatingCxr,
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
class QueryGetAllGenSalesAgents : public tse::QueryGetGenSalesAgent
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllGenSalesAgents(DBAdapter* dbAdapt)
    : QueryGetGenSalesAgent(dbAdapt, _baseSQL) {}

  virtual ~QueryGetAllGenSalesAgents() {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::GenSalesAgentInfo*>& gsaList)
      { findAllGenSalesAgents(gsaList); }

  void
  findAllGenSalesAgents(std::vector<tse::GenSalesAgentInfo*>& gsaList);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

}

