//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/VoluntaryRefundsInfo.h"

namespace tse
{
class QueryGetVoluntaryRefundsInfo : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryRefundsInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}

  QueryGetVoluntaryRefundsInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual const char* getQueryName() const override { return "GETVOLUNTARYREFUNDSINFO"; }

  void find(std::vector<VoluntaryRefundsInfo*>& lst, const VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetVoluntaryRefundsInfo& operator=(const QueryGetVoluntaryRefundsInfo& Another)
  {
    if (this != &Another)
      SQLQuery::operator=(Another);

    return *this;
  }

  const QueryGetVoluntaryRefundsInfo& operator=(const std::string& Another)
  {
    if (this != &Another)
      SQLQuery::operator=(Another);

    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetVoluntaryRefundsInfoHistorical : public QueryGetVoluntaryRefundsInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetVoluntaryRefundsInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetVoluntaryRefundsInfo(dbAdapt, _baseSQL)
  {
  }

  virtual const char* getQueryName() const override { return "GETVOLUNTARYREFUNDSINFOHISTORICAL"; }

  void find(std::vector<VoluntaryRefundsInfo*>& lst,
            const VendorCode& vendor,
            int itemNo,
            const DateTime& startDate,
            const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // namespace tse

