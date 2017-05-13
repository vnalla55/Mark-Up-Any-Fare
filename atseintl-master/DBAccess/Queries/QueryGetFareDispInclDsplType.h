//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclDsplType.h
//          Description:    QueryGetFareDispInclDsplType
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
#include "DBAccess/FareDispInclDsplType.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispInclDsplType : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispInclDsplType(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispInclDsplType(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispInclDsplType() {};

  virtual const char* getQueryName() const override;

  void findFareDispInclDsplType(std::vector<tse::FareDispInclDsplType*>& fareDispInclDsplTypes,
                                const Indicator& userApplType,
                                const UserApplCode& userAppl,
                                const Indicator& pseudoCityType,
                                const PseudoCityCode& pseudoCity,
                                const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispInclDsplType& operator=(const QueryGetFareDispInclDsplType& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispInclDsplType& operator=(const std::string& Another)
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
}; // class QueryGetFareDispInclDsplType

class QueryGetAllFareDispInclDsplType : public QueryGetFareDispInclDsplType
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispInclDsplType(DBAdapter* dbAdapt)
    : QueryGetFareDispInclDsplType(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispInclDsplType*>& fareDispInclDsplTypes)
  {
    findAllFareDispInclDsplType(fareDispInclDsplTypes);
  }

  void findAllFareDispInclDsplType(std::vector<tse::FareDispInclDsplType*>& fareDispInclDsplTypes);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispInclDsplType
} // namespace tse

