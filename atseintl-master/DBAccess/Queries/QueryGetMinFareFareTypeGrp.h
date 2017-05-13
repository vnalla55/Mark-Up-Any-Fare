//----------------------------------------------------------------------------
//          File:           QueryGetMinFareFareTypeGrp.h
//          Description:    QueryGetMinFareFareTypeGrp
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
#include "DBAccess/MinFareFareTypeGrp.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMinFareFareTypeGrp : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareFareTypeGrp(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareFareTypeGrp(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareFareTypeGrp() {};

  virtual const char* getQueryName() const override;

  void findMinFareFareTypeGrp(std::vector<tse::MinFareFareTypeGrp*>& lstFTG,
                              const std::string& specialProcessName);

  static void initialize();

  const QueryGetMinFareFareTypeGrp& operator=(const QueryGetMinFareFareTypeGrp& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareFareTypeGrp& operator=(const std::string& Another)
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
}; // class QueryGetMinFareFareTypeGrp

class QueryGetMinFareFareTypeGrpHistorical : public tse::QueryGetMinFareFareTypeGrp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareFareTypeGrpHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareFareTypeGrp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareFareTypeGrpHistorical() {};
  virtual const char* getQueryName() const override;

  void findMinFareFareTypeGrp(std::vector<tse::MinFareFareTypeGrp*>& lstFTG,
                              const std::string& specialProcessName);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMinFareFareTypeGrpHistorical

class QueryGetAllMinFareFareTypeGrp : public QueryGetMinFareFareTypeGrp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMinFareFareTypeGrp(DBAdapter* dbAdapt)
    : QueryGetMinFareFareTypeGrp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMinFareFareTypeGrp() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MinFareFareTypeGrp*>& lstFTG) { findAllMinFareFareTypeGrp(lstFTG); }

  void findAllMinFareFareTypeGrp(std::vector<tse::MinFareFareTypeGrp*>& lstFTG);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMinFareFareTypeGrp
class QueryGetAllMinFareFareTypeGrpHistorical : public QueryGetMinFareFareTypeGrp
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMinFareFareTypeGrpHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareFareTypeGrp(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllMinFareFareTypeGrpHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllMinFareFareTypeGrp(std::vector<tse::MinFareFareTypeGrp*>& lstFTG);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMinFareFareTypeGrpHistorical
} // namespace tse

