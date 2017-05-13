//----------------------------------------------------------------------------
//          File:           QueryGetFltTrk.h
//          Description:    QueryGetFltTrk
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
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetFltTrk : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFltTrk(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetFltTrk(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetFltTrk() {};

  virtual const char* getQueryName() const override;

  void findFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks, const CarrierCode& cxr);

  static void initialize();

  const QueryGetFltTrk& operator=(const QueryGetFltTrk& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetFltTrk& operator=(const std::string& Another)
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
}; // class QueryGetFltTrk

class QueryGetAllFltTrk : public QueryGetFltTrk
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFltTrk(DBAdapter* dbAdapt) : QueryGetFltTrk(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::FltTrkCntryGrp*>& trks) { findAllFltTrkCntryGrp(trks); }

  void findAllFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFltTrk

class QueryGetFltTrkHistorical : public QueryGetFltTrk
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetFltTrkHistorical(DBAdapter* dbAdapt) : QueryGetFltTrk(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks, const CarrierCode& cxr);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetFltTrkHistorical

class QueryGetAllFltTrkHistorical : public QueryGetFltTrk
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllFltTrkHistorical(DBAdapter* dbAdapt) : QueryGetFltTrk(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findAllFltTrkCntryGrp(std::vector<tse::FltTrkCntryGrp*>& trks);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllFltTrkHistorical

} // namespace tse

