//----------------------------------------------------------------------------
//          File:           QueryGetMiscFareTag.h
//          Description:    QueryGetMiscFareTag
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
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMiscFareTag : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMiscFareTag(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMiscFareTag(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMiscFareTag() {};
  virtual const char* getQueryName() const override;

  void
  findMiscFareTag(std::vector<MiscFareTag*>& fareTags, const VendorCode& vendor, int itemNumber);

  static void initialize();

  const QueryGetMiscFareTag& operator=(const QueryGetMiscFareTag& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMiscFareTag& operator=(const std::string& Another)
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
}; // class QueryGetMiscFareTag

class QueryGetMiscFareTagHistorical : public QueryGetMiscFareTag
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMiscFareTagHistorical(DBAdapter* dbAdapt) : QueryGetMiscFareTag(dbAdapt, _baseSQL) {}
  virtual ~QueryGetMiscFareTagHistorical() {}
  virtual const char* getQueryName() const override;

  void findMiscFareTag(std::vector<MiscFareTag*>& fareTags,
                       const VendorCode& vendor,
                       int itemNumber,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMiscFareTagHistorical
} // namespace tse

