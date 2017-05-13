//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SeasonalityDOW.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetSeasonalityDOW : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeasonalityDOW(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSeasonalityDOW(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSeasonalityDOW() {};
  virtual const char* getQueryName() const override;

  void findSeasonalityDOW(std::vector<SeasonalityDOW*>& SeasonalityDOWVec,
                          const VendorCode& vendor,
                          int itemNo);

  static void initialize();

  const QueryGetSeasonalityDOW& operator=(const QueryGetSeasonalityDOW& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSeasonalityDOW& operator=(const std::string& Another)
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
};

class QueryGetAllSeasonalityDOW : public QueryGetSeasonalityDOW
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSeasonalityDOW(DBAdapter* dbAdapt) : QueryGetSeasonalityDOW(dbAdapt, _baseSQL) {}
  virtual ~QueryGetAllSeasonalityDOW() {}
  virtual const char* getQueryName() const override;

  void execute(std::vector<SeasonalityDOW*>& seasonalityDOWVec)
  {
    findAllSeasonalityDOW(seasonalityDOWVec);
  }

  void findAllSeasonalityDOW(std::vector<SeasonalityDOW*>& seasonalityDOWVec);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetSeasonalityDOWHistorical : public QueryGetSeasonalityDOW
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSeasonalityDOWHistorical(DBAdapter* dbAdapt) : QueryGetSeasonalityDOW(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetSeasonalityDOWHistorical() {}
  virtual const char* getQueryName() const override;

  void findSeasonalityDOW(std::vector<SeasonalityDOW*>& SeasonalityDOWVec,
                          const VendorCode& vendor,
                          int itemNo);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}
