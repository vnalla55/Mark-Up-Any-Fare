//----------------------------------------------------------------------------
//          File:           QueryGetCopParticipatingNation.h
//          Description:    QueryGetCopParticipatingNation
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
#include "DBAccess/CopParticipatingNation.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetCopParticipatingNation : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetCopParticipatingNation(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetCopParticipatingNation(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetCopParticipatingNation() {};

  virtual const char* getQueryName() const override;

  void findCopParticipatingNation(std::vector<tse::CopParticipatingNation*>& lstCPN,
                                  const NationCode& nation,
                                  const NationCode& copNation);

  static void initialize();

  const QueryGetCopParticipatingNation& operator=(const QueryGetCopParticipatingNation& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetCopParticipatingNation& operator=(const std::string& Another)
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
}; // class QueryGetCopParticipatingNation
class QueryGetCopParticipatingNationHistorical : public tse::QueryGetCopParticipatingNation
{
public:
  QueryGetCopParticipatingNationHistorical(DBAdapter* dbAdapt)
    : QueryGetCopParticipatingNation(dbAdapt, _baseSQL) {};
  virtual ~QueryGetCopParticipatingNationHistorical() {};
  virtual const char* getQueryName() const override;

  void findCopParticipatingNation(std::vector<tse::CopParticipatingNation*>& lstCPN,
                                  const NationCode& nation,
                                  const NationCode& copNation);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetCopParticipatingNationHistorical

class QueryGetAllCopParticipatingNation : public QueryGetCopParticipatingNation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCopParticipatingNation(DBAdapter* dbAdapt)
    : QueryGetCopParticipatingNation(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCopParticipatingNation() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::CopParticipatingNation*>& lstCPN)
  {
    findAllCopParticipatingNation(lstCPN);
  }

  void findAllCopParticipatingNation(std::vector<tse::CopParticipatingNation*>& lstCPN);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCopParticipatingNation
class QueryGetAllCopParticipatingNationHistorical : public QueryGetCopParticipatingNation
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllCopParticipatingNationHistorical(DBAdapter* dbAdapt)
    : QueryGetCopParticipatingNation(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllCopParticipatingNationHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllCopParticipatingNation(std::vector<tse::CopParticipatingNation*>& lstCPN);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllCopParticipatingNationHistorical
} // namespace tse

