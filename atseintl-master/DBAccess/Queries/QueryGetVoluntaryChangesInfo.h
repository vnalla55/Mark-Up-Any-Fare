//----------------------------------------------------------------------------
//          File:           QueryGetVoluntaryChangesInfo.h
//          Description:    QueryGetVoluntaryChangesInfo
//          Created:        3/29/2007
// Authors:         Artur Krezel
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
#include "DBAccess/VoluntaryChangesInfo.h"

namespace tse
{

class QueryGetVoluntaryChangesInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryChangesInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetVoluntaryChangesInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetVoluntaryChangesInfo() {}
  virtual const char* getQueryName() const override;

  void findVoluntaryChangesInfo(std::vector<tse::VoluntaryChangesInfo*>& lstVCI,
                                VendorCode& vendor,
                                int itemNo);

  static void initialize();

  const QueryGetVoluntaryChangesInfo& operator=(const QueryGetVoluntaryChangesInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetVoluntaryChangesInfo& operator=(const std::string& Another)
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
}; // class QueryGetVoluntaryChangesInfo

class QueryGetVoluntaryChangesInfoHistorical : public QueryGetVoluntaryChangesInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryChangesInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetVoluntaryChangesInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetVoluntaryChangesInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findVoluntaryChangesInfo(std::vector<tse::VoluntaryChangesInfo*>& lstVCI,
                                VendorCode& vendor,
                                int itemNo,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetVoluntaryChangesInfoHistorical
} // namespace tse

