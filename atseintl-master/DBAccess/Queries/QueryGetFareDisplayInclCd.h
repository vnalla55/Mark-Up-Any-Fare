//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayInclCd.h
//          Description:    QueryGetFareDisplayInclCd
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
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDisplayInclCd : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDisplayInclCd(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDisplayInclCd(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDisplayInclCd() {};

  virtual const char* getQueryName() const override;

  void findFareDisplayInclCd(std::vector<tse::FareDisplayInclCd*>& fareDisplayInclCds,
                             const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const Indicator& pseudoCityType,
                             const PseudoCityCode& pseudoCity,
                             const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetFareDisplayInclCd& operator=(const QueryGetFareDisplayInclCd& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDisplayInclCd& operator=(const std::string& Another)
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
}; // class QueryGetFareDisplayInclCd

class QueryGetAllFareDisplayInclCd : public QueryGetFareDisplayInclCd
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDisplayInclCd(DBAdapter* dbAdapt)
    : QueryGetFareDisplayInclCd(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDisplayInclCd*>& fareDisplayInclCds)
  {
    findAllFareDisplayInclCd(fareDisplayInclCds);
  }

  void findAllFareDisplayInclCd(std::vector<tse::FareDisplayInclCd*>& fareDisplayInclCds);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDisplayInclCd
} // namespace tse

