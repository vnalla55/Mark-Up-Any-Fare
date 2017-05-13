//----------------------------------------------------------------------------
//          File:           QueryGetTravelRestriction.h
//          Description:    QueryGetTravelRestriction
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
#include "DBAccess/TravelRestriction.h"

namespace tse
{

class QueryGetTravelRestriction : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTravelRestriction(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTravelRestriction(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTravelRestriction() {}
  virtual const char* getQueryName() const override;

  void findTravelRestriction(std::vector<tse::TravelRestriction*>& lstTR,
                             VendorCode& vendor,
                             int itemNo);

  static void initialize();

  const QueryGetTravelRestriction& operator=(const QueryGetTravelRestriction& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTravelRestriction& operator=(const std::string& Another)
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
}; // class QueryGetTravelRestriction

class QueryGetTravelRestrictionHistorical : public QueryGetTravelRestriction
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTravelRestrictionHistorical(DBAdapter* dbAdapt)
    : QueryGetTravelRestriction(dbAdapt, _baseSQL)
  {
  }
  virtual ~QueryGetTravelRestrictionHistorical() {}
  virtual const char* getQueryName() const override;

  void findTravelRestriction(std::vector<tse::TravelRestriction*>& lstTR,
                             VendorCode& vendor,
                             int itemNo,
                             const DateTime& startDate,
                             const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTravelRestrictionHistorical
} // namespace tse

