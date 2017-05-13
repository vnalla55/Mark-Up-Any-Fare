//----------------------------------------------------------------------------
//          File:           QueryGetPenaltyInfo.h
//          Description:    QueryGetPenaltyInfo
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
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetPenaltyInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPenaltyInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPenaltyInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPenaltyInfo() {}
  virtual const char* getQueryName() const override;

  void findPenaltyInfo(std::vector<tse::PenaltyInfo*>& lstPI, VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetPenaltyInfo& operator=(const QueryGetPenaltyInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPenaltyInfo& operator=(const std::string& Another)
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
}; // class QueryGetPenaltyInfo

class QueryGetPenaltyInfoHistorical : public QueryGetPenaltyInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPenaltyInfoHistorical(DBAdapter* dbAdapt) : QueryGetPenaltyInfo(dbAdapt, _baseSQL) {}
  virtual ~QueryGetPenaltyInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findPenaltyInfo(std::vector<tse::PenaltyInfo*>& lstPI,
                       VendorCode& vendor,
                       int itemNo,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPenaltyInfoHistorical
} // namespace tse

