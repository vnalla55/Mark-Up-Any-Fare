//----------------------------------------------------------------------------
//          File:           QueryGetPaxTypeMatrix.h
//          Description:    QueryGetPaxTypeMatrix
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
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetPaxTypeMatrix : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPaxTypeMatrix(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetPaxTypeMatrix(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetPaxTypeMatrix() {}

  virtual const char* getQueryName() const override;

  void findPaxTypeMatrix(std::vector<const tse::PaxTypeMatrix*>& paxTypeMatrixs,
                         const PaxTypeCode& paxType);

  static void initialize();
  const QueryGetPaxTypeMatrix& operator=(const QueryGetPaxTypeMatrix& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetPaxTypeMatrix& operator=(const std::string& Another)
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
}; // class QueryGetPaxTypeMatrix

class QueryGetPaxTypeMatrixs : public QueryGetPaxTypeMatrix
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetPaxTypeMatrixs(DBAdapter* dbAdapt) : QueryGetPaxTypeMatrix(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;

  void execute(std::vector<const tse::PaxTypeMatrix*>& paxTypeMatrixs)
  {
    findAllPaxTypeMatrix(paxTypeMatrixs);
  }

  void findAllPaxTypeMatrix(std::vector<const tse::PaxTypeMatrix*>& paxTypeMatrixs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetPaxTypeMatrixs
} // namespace tse

