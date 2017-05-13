//----------------------------------------------------------------------------
//          File:           QueryGetAllGenericTaxCode.h
//          Description:    QueryGetAllGenericTaxCode
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
#include "DBAccess/GenericTaxCode.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetAllGenericTaxCode : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllGenericTaxCode(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAllGenericTaxCode(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllGenericTaxCode() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::GenericTaxCode*>& taxCodes) { findAllGenericTaxCode(taxCodes); }

  void findAllGenericTaxCode(std::vector<tse::GenericTaxCode*>& taxCodes);

  static void initialize();

  const QueryGetAllGenericTaxCode& operator=(const QueryGetAllGenericTaxCode& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAllGenericTaxCode& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllGenericTaxCode

class QueryGetAllGenericTaxCodeHistorical : public QueryGetAllGenericTaxCode
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllGenericTaxCodeHistorical(DBAdapter* dbAdapt)
    : QueryGetAllGenericTaxCode(dbAdapt, _baseSQL) {};
  QueryGetAllGenericTaxCodeHistorical(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetAllGenericTaxCode(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAllGenericTaxCodeHistorical() {};

  virtual const char* getQueryName() const override;

  void findAllGenericTaxCode(std::vector<tse::GenericTaxCode*>& taxCodes,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllGenericTaxCodeHistorical

} // namespace tse
