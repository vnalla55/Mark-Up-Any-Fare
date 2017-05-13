//----------------------------------------------------------------------------
//          File:           QueryGetFareDispRec8PsgType.h
//          Description:    QueryGetFareDispRec8PsgType
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
#include "DBAccess/FareDispRec8PsgType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispRec8PsgType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispRec8PsgType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispRec8PsgType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispRec8PsgType() {};

  virtual const char* getQueryName() const override;

  void findFareDispRec8PsgType(std::vector<tse::FareDispRec8PsgType*>& infos,
                               const Indicator& userApplType,
                               const UserApplCode& userAppl,
                               const Indicator& pseudoCityType,
                               const PseudoCityCode& pseudoCity,
                               const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispRec8PsgType& operator=(const QueryGetFareDispRec8PsgType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispRec8PsgType& operator=(const std::string& Another)
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
}; // class QueryGetFareDispRec8PsgType

class QueryGetAllFareDispRec8PsgType : public QueryGetFareDispRec8PsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispRec8PsgType(DBAdapter* dbAdapt)
    : QueryGetFareDispRec8PsgType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispRec8PsgType*>& infos) { findAllFareDispRec8PsgType(infos); }

  void findAllFareDispRec8PsgType(std::vector<tse::FareDispRec8PsgType*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispRec8PsgType
} // namespace tse

