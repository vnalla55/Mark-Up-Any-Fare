//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclRuleTrf.h
//          Description:    QueryGetFareDispInclRuleTrf
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
#include "DBAccess/FareDispInclRuleTrf.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispInclRuleTrf : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispInclRuleTrf(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispInclRuleTrf(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispInclRuleTrf() {};

  virtual const char* getQueryName() const override;

  void findFareDispInclRuleTrf(std::vector<tse::FareDispInclRuleTrf*>& fareDispInclRuleTrfs,
                               const Indicator& userApplType,
                               const UserApplCode& userAppl,
                               const Indicator& pseudoCityType,
                               const PseudoCityCode& pseudoCity,
                               const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDispInclRuleTrf& operator=(const QueryGetFareDispInclRuleTrf& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispInclRuleTrf& operator=(const std::string& Another)
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
}; // class QueryGetFareDispInclRuleTrf

class QueryGetAllFareDispInclRuleTrf : public QueryGetFareDispInclRuleTrf
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispInclRuleTrf(DBAdapter* dbAdapt)
    : QueryGetFareDispInclRuleTrf(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispInclRuleTrf*>& fareDispInclRuleTrfs)
  {
    findAllFareDispInclRuleTrf(fareDispInclRuleTrfs);
  }

  void findAllFareDispInclRuleTrf(std::vector<tse::FareDispInclRuleTrf*>& fareDispInclRuleTrfs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispInclRuleTrf
} // namespace tse
