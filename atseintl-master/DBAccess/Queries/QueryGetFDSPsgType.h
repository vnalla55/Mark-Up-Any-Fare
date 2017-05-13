//----------------------------------------------------------------------------
//          File:           QueryGetFDSPsgType.h
//          Description:    QueryGetFDSPsgType
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
#include "DBAccess/FDSPsgType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFDSPsgType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFDSPsgType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFDSPsgType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFDSPsgType() {};

  virtual const char* getQueryName() const override;

  void findFDSPsgType(std::vector<tse::FDSPsgType*>& infos,
                      const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const Indicator& pseudoCityType,
                      const PseudoCityCode& pseudoCity,
                      const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFDSPsgType& operator=(const QueryGetFDSPsgType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFDSPsgType& operator=(const std::string& Another)
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
}; // class QueryGetFDSPsgType

class QueryGetAllFDSPsgType : public QueryGetFDSPsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFDSPsgType(DBAdapter* dbAdapt) : QueryGetFDSPsgType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FDSPsgType*>& infos) { findAllFDSPsgType(infos); }

  void findAllFDSPsgType(std::vector<tse::FDSPsgType*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFDSPsgType
} // namespace tse

