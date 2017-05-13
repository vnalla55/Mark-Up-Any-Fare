//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclFareType.h
//          Description:    QueryGetFareDispInclFareType
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
#include "DBAccess/FareDispInclFareType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispInclFareType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispInclFareType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispInclFareType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispInclFareType() {};

  virtual const char* getQueryName() const override;

  void findFareDispInclFareType(std::vector<tse::FareDispInclFareType*>& fareDispInclFareTypes,
                                const Indicator& userApplType,
                                const UserApplCode& userAppl,
                                const Indicator& pseudoCityType,
                                const PseudoCityCode& pseudoCity,
                                const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispInclFareType& operator=(const QueryGetFareDispInclFareType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispInclFareType& operator=(const std::string& Another)
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
}; // class QueryGetFareDispInclFareType

class QueryGetAllFareDispInclFareType : public QueryGetFareDispInclFareType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispInclFareType(DBAdapter* dbAdapt)
    : QueryGetFareDispInclFareType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispInclFareType*>& fareDispInclFareTypes)
  {
    findAllFareDispInclFareType(fareDispInclFareTypes);
  }

  void findAllFareDispInclFareType(std::vector<tse::FareDispInclFareType*>& fareDispInclFareTypes);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispInclFareType
} // namespace tse

