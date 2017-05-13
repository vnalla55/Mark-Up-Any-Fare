//----------------------------------------------------------------------------
//          File:           QueryGetTable988.h
//          Description:    QueryGetTable988
//          Created:        4/3/2007
// Authors:         Grzegorz Cholewiak
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetTable988 : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable988(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetTable988(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetTable988() {};
  virtual const char* getQueryName() const override;

  void
  findReissue(std::vector<tse::ReissueSequence*>& table988s, const VendorCode& vendor, int itemNo);

  static void initialize();

  const QueryGetTable988& operator=(const QueryGetTable988& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetTable988& operator=(const std::string& Another)
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
}; // class QueryGetTable988

class QueryGetTable988Historical : public QueryGetTable988
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTable988Historical(DBAdapter* dbAdapt) : QueryGetTable988(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTable988Historical() {}
  virtual const char* getQueryName() const override;

  void findReissue(std::vector<tse::ReissueSequence*>& table988s,
                   const VendorCode& vendor,
                   int itemNo,
                   const DateTime& startDate,
                   const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTable988Historical
} // namespace tse

