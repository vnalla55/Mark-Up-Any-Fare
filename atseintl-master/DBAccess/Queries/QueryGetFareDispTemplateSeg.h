//----------------------------------------------------------------------------
//          File:           QueryGetFareDispTemplateSeg.h
//          Description:    QueryGetFareDispTemplateSeg
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
#include "DBAccess/FareDispTemplateSeg.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDispTemplateSeg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDispTemplateSeg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDispTemplateSeg(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDispTemplateSeg() {};

  virtual const char* getQueryName() const override;

  void findFareDispTemplateSeg(std::vector<tse::FareDispTemplateSeg*>& infos,
                               const int& templateID,
                               const Indicator& templateType);

  static void initialize();

  const QueryGetFareDispTemplateSeg& operator=(const QueryGetFareDispTemplateSeg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDispTemplateSeg& operator=(const std::string& Another)
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
}; // class QueryGetFareDispTemplateSeg

class QueryGetAllFareDispTemplateSeg : public QueryGetFareDispTemplateSeg
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDispTemplateSeg(DBAdapter* dbAdapt)
    : QueryGetFareDispTemplateSeg(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDispTemplateSeg*>& infos) { findAllFareDispTemplateSeg(infos); }

  void findAllFareDispTemplateSeg(std::vector<tse::FareDispTemplateSeg*>& infos);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDispTemplateSeg
} // namespace tse

