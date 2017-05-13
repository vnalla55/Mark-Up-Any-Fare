//----------------------------------------------------------------------------
//          File:           QueryGetJointCxr.h
//          Description:    QueryGetJointCxr
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
#include "DBAccess/JointCarrier.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetJointCxr : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetJointCxr(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetJointCxr(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetJointCxr() {};
  virtual const char* getQueryName() const override;

  void findJointCarrier(std::vector<const tse::JointCarrier*>& jointcxrs,
                        const VendorCode& vendor,
                        int itemNo);

  static void initialize();

  const QueryGetJointCxr& operator=(const QueryGetJointCxr& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetJointCxr& operator=(const std::string& Another)
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
}; // class QueryGetJointCxr

class QueryGetJointCxrHistorical : public QueryGetJointCxr
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetJointCxrHistorical(DBAdapter* dbAdapt) : QueryGetJointCxr(dbAdapt, _baseSQL) {}
  virtual ~QueryGetJointCxrHistorical() {}
  virtual const char* getQueryName() const override;

  void findJointCarrier(std::vector<const tse::JointCarrier*>& jointcxrs,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& startDate,
                        const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetJointCxrHistorical
} // namespace tse

