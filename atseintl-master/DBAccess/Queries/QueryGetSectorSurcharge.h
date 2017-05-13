//----------------------------------------------------------------------------
//          File:           QueryGetSectorSurcharge.h
//          Description:    QueryGetSectorSurcharge
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
#include "DBAccess/SectorSurcharge.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetSectSurchTktgCxrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSectSurchTktgCxrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSectSurchTktgCxrs() {};

  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getTktgCxrs(tse::SectorSurcharge* a_pSectorSurcharge);

  static void initialize();

  const QueryGetSectSurchTktgCxrs& operator=(const QueryGetSectSurchTktgCxrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSectSurchTktgCxrs& operator=(const std::string& Another)
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
}; // class QueryGetSectSurchTktgCxrs

class QueryGetSectorSurchargeBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSectorSurchargeBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSectorSurchargeBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSectorSurchargeBase() {};

  virtual const char* getQueryName() const override;

  void findSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS, const CarrierCode& carrier);

  static void initialize();

  const QueryGetSectorSurchargeBase& operator=(const QueryGetSectorSurchargeBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSectorSurchargeBase& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

protected:
  void findSectorSurchargeChildren(std::vector<tse::SectorSurcharge*>& lstSS);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSectorSurchargeBase

class QueryGetSectorSurchargeBaseHistorical : public QueryGetSectorSurchargeBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSectorSurchargeBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetSectorSurchargeBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSectorSurchargeBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS, const CarrierCode& carrier);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetSectorSurchargeBaseHistorical

class QueryGetAllSectorSurchargeBase : public QueryGetSectorSurchargeBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSectorSurchargeBase(DBAdapter* dbAdapt)
    : QueryGetSectorSurchargeBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSectorSurchargeBase() {};
  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::SectorSurcharge*>& lstSS) { findAllSectorSurcharge(lstSS); }

  void findAllSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSectorSurchargeBase

class QueryGetAllSectorSurchargeBaseHistorical : public QueryGetSectorSurchargeBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllSectorSurchargeBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetSectorSurchargeBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllSectorSurchargeBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findAllSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllSectorSurchargeBaseHistorical
} // namespace tse

