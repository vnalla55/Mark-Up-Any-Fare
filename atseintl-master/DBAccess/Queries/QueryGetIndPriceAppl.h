//----------------------------------------------------------------------------
//          File:           QueryGetIndPriceAppl.h
//          Description:    QueryGetIndPriceAppl
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
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetIndPriceAppl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIndPriceAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetIndPriceAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetIndPriceAppl() {};

  virtual const char* getQueryName() const override;

  void findIndustryPricingAppl(std::vector<const tse::IndustryPricingAppl*>& indPriceAppls,
                               const CarrierCode& carrier);

  static void initialize();

  const QueryGetIndPriceAppl& operator=(const QueryGetIndPriceAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetIndPriceAppl& operator=(const std::string& Another)
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
}; // class QueryGetIndPriceAppl

class QueryGetAllIndPriceAppl : public QueryGetIndPriceAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndPriceAppl(DBAdapter* dbAdapt) : QueryGetIndPriceAppl(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<const tse::IndustryPricingAppl*>& indPriceAppls)
  {
    findAllIndustryPricingAppl(indPriceAppls);
  }

  void findAllIndustryPricingAppl(std::vector<const tse::IndustryPricingAppl*>& indPriceAppls);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndPriceAppl

class QueryGetIndPriceApplHistorical : public QueryGetIndPriceAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetIndPriceApplHistorical(DBAdapter* dbAdapt) : QueryGetIndPriceAppl(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findIndustryPricingAppl(std::vector<tse::IndustryPricingAppl*>& indPriceAppls,
                               const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetIndPriceApplHistorical

class QueryGetAllIndPriceApplHistorical : public QueryGetIndPriceAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllIndPriceApplHistorical(DBAdapter* dbAdapt)
    : QueryGetIndPriceAppl(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findAllIndustryPricingAppl(std::vector<tse::IndustryPricingAppl*>& indPriceAppls);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllIndPriceApplHistorical

} // namespace tse

