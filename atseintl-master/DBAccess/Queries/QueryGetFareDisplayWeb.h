//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayWeb.h
//          Description:    QueryGetFareDisplayWeb
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
#include "DBAccess/FareDisplayWeb.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDisplayWeb : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDisplayWeb(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDisplayWeb(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDisplayWeb() {};

  virtual const char* getQueryName() const override;

  void findFareDisplayWeb(std::vector<tse::FareDisplayWeb*>& fareDisplayWebs,
                          const CarrierCode& carrier);

  static void initialize();

  const QueryGetFareDisplayWeb& operator=(const QueryGetFareDisplayWeb& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDisplayWeb& operator=(const std::string& Another)
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
}; // class QueryGetFareDisplayWeb

class QueryGetAllFareDisplayWeb : public QueryGetFareDisplayWeb
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDisplayWeb(DBAdapter* dbAdapt) : QueryGetFareDisplayWeb(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDisplayWeb*>& fareDisplayWebs)
  {
    findAllFareDisplayWeb(fareDisplayWebs);
  }

  void findAllFareDisplayWeb(std::vector<tse::FareDisplayWeb*>& fareDisplayWebs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDisplayWeb
} // namespace tse

