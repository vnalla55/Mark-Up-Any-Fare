//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayPrefSeg.h
//          Description:    QueryGetFareDisplayPrefSeg
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
#include "DBAccess/FareDisplayPrefSeg.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFareDisplayPrefSeg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFareDisplayPrefSeg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFareDisplayPrefSeg(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFareDisplayPrefSeg() {};

  virtual const char* getQueryName() const override;

  void findFareDisplayPrefSeg(std::vector<tse::FareDisplayPrefSeg*>& fareDisplayPrefSegs,
                              const Indicator& userApplType,
                              const UserApplCode& userAppl,
                              const Indicator& pseudoCityType,
                              const PseudoCityCode& pseudoCity,
                              const TJRGroup& tjrGroup);

  static void initialize();

  const QueryGetFareDisplayPrefSeg& operator=(const QueryGetFareDisplayPrefSeg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFareDisplayPrefSeg& operator=(const std::string& Another)
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
}; // class QueryGetFareDisplayPrefSeg

class QueryGetAllFareDisplayPrefSeg : public QueryGetFareDisplayPrefSeg
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFareDisplayPrefSeg(DBAdapter* dbAdapt)
    : QueryGetFareDisplayPrefSeg(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FareDisplayPrefSeg*>& fareDisplayPrefSegs)
  {
    findAllFareDisplayPrefSeg(fareDisplayPrefSegs);
  }

  void findAllFareDisplayPrefSeg(std::vector<tse::FareDisplayPrefSeg*>& fareDisplayPrefSegs);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFareDisplayPrefSeg
} // namespace tse

