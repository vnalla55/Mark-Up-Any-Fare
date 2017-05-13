//----------------------------------------------------------------------------
//          File:           QueryGetFareTypeMatrixs.h
//          Description:    QueryGetFareTypeMatrixs
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
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareTypeMatrixs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeMatrixs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareTypeMatrixs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareTypeMatrixs() {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareTypeMatrix*>& FareTypeMatrixs)
  {
    findAllFareTypeMatrix(FareTypeMatrixs);
  }

  void findAllFareTypeMatrix(std::vector<tse::FareTypeMatrix*>& FareTypeMatrixs);

  static void initialize();

  const QueryGetFareTypeMatrixs& operator=(const QueryGetFareTypeMatrixs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareTypeMatrixs& operator=(const std::string& Another)
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
}; // class QueryGetFareTypeMatrixs

class QueryGetFareTypeMatrixsHistorical : public QueryGetFareTypeMatrixs
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareTypeMatrixsHistorical(DBAdapter* dbAdapt)
    : QueryGetFareTypeMatrixs(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findAllFareTypeMatrix(std::vector<tse::FareTypeMatrix*>& FareTypeMatrixs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFareTypeMatrixsHistorical
} // namespace tse

