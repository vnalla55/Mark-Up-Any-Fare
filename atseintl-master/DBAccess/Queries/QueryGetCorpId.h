//----------------------------------------------------------------------------
//          File:           QueryGetCorpId.h
//          Description:    QueryGetCorpId
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
#include "DBAccess/CorpId.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetCorpId : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCorpId(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCorpId(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCorpId() {};
  virtual const char* getQueryName() const override;

  void findCorpId(std::vector<tse::CorpId*>& corpIds, std::string& corpId);

  static void initialize();

  const QueryGetCorpId& operator=(const QueryGetCorpId& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCorpId& operator=(const std::string& Another)
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
}; // class QueryGetCorpId

class QueryGetCorpIdHistorical : public QueryGetCorpId
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCorpIdHistorical(DBAdapter* dbAdapt) : QueryGetCorpId(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCorpIdHistorical() {};
  virtual const char* getQueryName() const override;

  void findCorpId(std::vector<tse::CorpId*>& corpIds,
                  std::string& corpId,
                  const DateTime& startDate,
                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCorpIdHistorical
} // namespace tse
