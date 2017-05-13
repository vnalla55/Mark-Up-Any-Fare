//----------------------------------------------------------------------------
//          File:           QueryGetTaxReissue.h
//          Description:    QueryGetTaxReissue
//          Created:        3/2/2006
// Authors:         Dean Van Decker
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TaxReissue.h"

namespace tse
{

class QueryGetTaxReissue : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxReissue(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTaxReissue(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTaxReissue() {}

  virtual const char* getQueryName() const override;

  void findTaxReissue(std::vector<TaxReissue*>& taxReissue, const TaxCode& code);

  static void initialize();

  const QueryGetTaxReissue& operator=(const QueryGetTaxReissue& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTaxReissue& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxReissue

class QueryGetTaxReissueHistorical : public QueryGetTaxReissue
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTaxReissueHistorical(DBAdapter* dbAdapt) : QueryGetTaxReissue(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTaxReissueHistorical() {}
  const char* getQueryName() const override;

  void findTaxReissue(std::vector<TaxReissue*>& taxReissue, const TaxCode& code);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTaxReissueHistorical

class QueryGetAllTaxReissueHistorical : public QueryGetTaxReissue
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTaxReissueHistorical(DBAdapter* dbAdapt) : QueryGetTaxReissue(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllTaxReissueHistorical() {}
  const char* getQueryName() const override;

  void execute(std::vector<TaxReissue*>& taxReissue) { findAllTaxReissues(taxReissue); }

  void findAllTaxReissues(std::vector<TaxReissue*>& taxReissue);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTaxReissueHistorical
} // namespace tse
