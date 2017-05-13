//----------------------------------------------------------------------------
//          File:           QueryGetTicketEndorsementsInfo.h
//          Description:    QueryGetTicketEndorsementsInfo
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TicketEndorsementsInfo.h"

namespace tse
{

class QueryGetTicketEndorsementsInfo : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTicketEndorsementsInfo(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTicketEndorsementsInfo(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTicketEndorsementsInfo() {}
  virtual const char* getQueryName() const override;

  void findTicketEndorsementsInfo(std::vector<tse::TicketEndorsementsInfo*>& lstTEI,
                                  VendorCode& vendor,
                                  int itemNo);

  static void initialize();

  const QueryGetTicketEndorsementsInfo& operator=(const QueryGetTicketEndorsementsInfo& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTicketEndorsementsInfo& operator=(const std::string& Another)
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
}; // class QueryGetTicketEndorsementsInfo

class QueryGetTicketEndorsementsInfoHistorical : public QueryGetTicketEndorsementsInfo
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTicketEndorsementsInfoHistorical(DBAdapter* dbAdapt)
    : QueryGetTicketEndorsementsInfo(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTicketEndorsementsInfoHistorical() {}
  virtual const char* getQueryName() const override;

  void findTicketEndorsementsInfo(std::vector<tse::TicketEndorsementsInfo*>& lstTEI,
                                  VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTicketEndorsementsInfoHistorical
} // namespace tse

