//----------------------------------------------------------------------------
//          File:           QueryGetSeasonalAppl.h
//          Description:    QueryGetSeasonalAppl
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
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetSeasonalAppl : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeasonalAppl(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSeasonalAppl(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSeasonalAppl() {};
  virtual const char* getQueryName() const override;

  void findSeasonalAppl(std::vector<tse::SeasonalAppl*>& seasappls, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetSeasonalAppl& operator=(const QueryGetSeasonalAppl& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSeasonalAppl& operator=(const std::string& Another)
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
}; // class QueryGetSeasonalAppl

class QueryGetSeasonalApplHistorical : public QueryGetSeasonalAppl
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeasonalApplHistorical(DBAdapter* dbAdapt) : QueryGetSeasonalAppl(dbAdapt, _baseSQL) {}
  virtual ~QueryGetSeasonalApplHistorical() {}
  virtual const char* getQueryName() const override;

  void findSeasonalAppl(std::vector<tse::SeasonalAppl*>& seasappls,
                        VendorCode& vendor,
                        int itemNo,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSeasonalApplHistorical
} // namespace tse
