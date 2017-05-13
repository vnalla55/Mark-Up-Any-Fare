//----------------------------------------------------------------------------
//          File:           QueryGetCircleTripProvision.h
//          Description:    QueryGetCircleTripProvision
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
#include "DBAccess/CircleTripProvision.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCircleTripProvision : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCircleTripProvision(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCircleTripProvision(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCircleTripProvision() {};

  virtual const char* getQueryName() const override;

  void findCircleTripProvision(std::vector<tse::CircleTripProvision*>& lstCTP,
                               LocCode& market1,
                               LocCode& market2);

  static void initialize();

  const QueryGetCircleTripProvision& operator=(const QueryGetCircleTripProvision& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCircleTripProvision& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCircleTripProvision

class QueryGetCircleTripProvisionHistorical : public tse::QueryGetCircleTripProvision
{
public:
  QueryGetCircleTripProvisionHistorical(DBAdapter* dbAdapt)
    : QueryGetCircleTripProvision(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCircleTripProvisionHistorical() {};
  virtual const char* getQueryName() const override;

  void findCircleTripProvision(std::vector<tse::CircleTripProvision*>& lstCTP,
                               LocCode& market1,
                               LocCode& market2);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCircleTripProvisionHistorical

class QueryGetAllCircleTripProvision : public QueryGetCircleTripProvision
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCircleTripProvision(DBAdapter* dbAdapt)
    : QueryGetCircleTripProvision(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCircleTripProvision() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::CircleTripProvision*>& lstCTP)
  {
    findAllCircleTripProvision(lstCTP);
  }

  void findAllCircleTripProvision(std::vector<tse::CircleTripProvision*>& lstCTP);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCircleTripProvision

class QueryGetAllCircleTripProvisionHistorical : public QueryGetCircleTripProvision
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCircleTripProvisionHistorical(DBAdapter* dbAdapt)
    : QueryGetCircleTripProvision(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCircleTripProvisionHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllCircleTripProvision(std::vector<tse::CircleTripProvision*>& lstCTP);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCircleTripProvisionHistorical
} // namespace tse

