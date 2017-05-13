//----------------------------------------------------------------------------
//          File:           QueryGetFareDispRec1PsgType.h
//          Description:    QueryGetFareDispRec1PsgType
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
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispRec1PsgType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispRec1PsgType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispRec1PsgType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispRec1PsgType() {};

  virtual const char* getQueryName() const override;

  void findFareDispRec1PsgType(std::vector<tse::FareDispRec1PsgType*>& infos,
                               const Indicator& userApplType,
                               const UserApplCode& userAppl,
                               const Indicator& pseudoCityType,
                               const PseudoCityCode& pseudoCity,
                               const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispRec1PsgType& operator=(const QueryGetFareDispRec1PsgType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispRec1PsgType& operator=(const std::string& Another)
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
}; // class QueryGetFareDispRec1PsgType

class QueryGetAllFareDispRec1PsgType : public QueryGetFareDispRec1PsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispRec1PsgType(DBAdapter* dbAdapt)
    : QueryGetFareDispRec1PsgType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispRec1PsgType*>& infos) { findAllFareDispRec1PsgType(infos); }

  void findAllFareDispRec1PsgType(std::vector<tse::FareDispRec1PsgType*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispRec1PsgType
} // namespace tse

