// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxExemption.h"

namespace tse
{

class DBResultSet;

class QueryGetTaxExemption : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExemption(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxExemption(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  { }

  ~QueryGetTaxExemption() = default;

  const char* getQueryName() const override;

  void findTaxExemption(std::vector<TaxExemption*>& taxExemption,
      const TaxCode& code, const PseudoCityCode& channelId);

  static void initialize();

protected:
  void findTaxExemption(std::vector<TaxExemption*>& taxExemption, DBResultSet& res);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetTaxExemptionHistorical : public QueryGetTaxExemption
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxExemptionHistorical(DBAdapter* dbAdapt) : QueryGetTaxExemption(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxExemptionHistorical() = default;
  const char* getQueryName() const override;

  void findTaxExemption(std::vector<TaxExemption*>& taxExemption,
      const TaxCode& code, const PseudoCityCode& channelId);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} //namespace
