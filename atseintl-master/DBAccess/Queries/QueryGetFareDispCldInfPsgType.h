//----------------------------------------------------------------------------
//          File:           QueryGetFareDispCldInfPsgType.h
//          Description:    QueryGetFareDispCldInfPsgType
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
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispCldInfPsgType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispCldInfPsgType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispCldInfPsgType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispCldInfPsgType() {};

  virtual const char* getQueryName() const override;

  void findFareDispCldInfPsgType(std::vector<tse::FareDispCldInfPsgType*>& infos,
                                 const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const Indicator& pseudoCityType,
                                 const PseudoCityCode& pseudoCity,
                                 const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispCldInfPsgType& operator=(const QueryGetFareDispCldInfPsgType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispCldInfPsgType& operator=(const std::string& Another)
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
}; // class QueryGetFareDispCldInfPsgType

class QueryGetAllFareDispCldInfPsgType : public QueryGetFareDispCldInfPsgType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispCldInfPsgType(DBAdapter* dbAdapt)
    : QueryGetFareDispCldInfPsgType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispCldInfPsgType*>& infos)
  {
    findAllFareDispCldInfPsgType(infos);
  }

  void findAllFareDispCldInfPsgType(std::vector<tse::FareDispCldInfPsgType*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispCldInfPsgType
} // namespace tse

