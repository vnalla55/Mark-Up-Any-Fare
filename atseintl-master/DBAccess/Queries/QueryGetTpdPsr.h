//----------------------------------------------------------------------------
//          File:           QueryGetTpdPsr.h
//          Description:    QueryGetTpdPsr
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
#include "DBAccess/SQLQuery.h"
#include "DBAccess/TpdPsr.h"

namespace tse
{

class QueryGetTpdPsrViaCxrLocs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTpdPsrViaCxrLocs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTpdPsrViaCxrLocs() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getViaCxrLocs(TpdPsr* a_pTpdPsr);

  static void initialize();
  const QueryGetTpdPsrViaCxrLocs& operator=(const QueryGetTpdPsrViaCxrLocs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTpdPsrViaCxrLocs& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTpdPsrViaCxrLocs

class QueryGetTpdPsrViaExcepts : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTpdPsrViaExcepts(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTpdPsrViaExcepts() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };

  const char* getQueryName() const override;

  void getViaExcepts(TpdPsr* a_pTpdPsr);

  static void initialize();
  const QueryGetTpdPsrViaExcepts& operator=(const QueryGetTpdPsrViaExcepts& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTpdPsrViaExcepts& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTpdPsrViaExcepts

class QueryGetTpdPsrViaGeoLocs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTpdPsrViaGeoLocs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTpdPsrViaGeoLocs() {}
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getViaGeoLocs(TpdPsr* a_pTpdPsr);
  static void initialize();
  const QueryGetTpdPsrViaGeoLocs& operator=(const QueryGetTpdPsrViaGeoLocs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTpdPsrViaGeoLocs& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTpdPsrViaGeoLocs

class QueryGetTpdPsrBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTpdPsrBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}
  QueryGetTpdPsrBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }
  virtual ~QueryGetTpdPsrBase() {}

  virtual const char* getQueryName() const override;

  void findTpdPsr(std::vector<tse::TpdPsr*>& lstTP,
                  Indicator applInd,
                  const CarrierCode& carrier,
                  Indicator area1,
                  Indicator area2);

  static void initialize();
  const QueryGetTpdPsrBase& operator=(const QueryGetTpdPsrBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }
  const QueryGetTpdPsrBase& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

  static int charToInt(const char* s);
  void findTpdPsrChildren(std::vector<tse::TpdPsr*>& lstTP);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTpdPsrBase

class QueryGetTpdPsrBaseHistorical : public QueryGetTpdPsrBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetTpdPsrBaseHistorical(DBAdapter* dbAdapt) : QueryGetTpdPsrBase(dbAdapt, _baseSQL) {}
  virtual ~QueryGetTpdPsrBaseHistorical() {}

  virtual const char* getQueryName() const override;

  void findTpdPsr(std::vector<tse::TpdPsr*>& lstTP,
                  Indicator applInd,
                  const CarrierCode& carrier,
                  Indicator area1,
                  Indicator area2,
                  const DateTime& startDate,
                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetTpdPsrBaseHistorical

class QueryGetAllTpdPsrBase : public QueryGetTpdPsrBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllTpdPsrBase(DBAdapter* dbAdapt) : QueryGetTpdPsrBase(dbAdapt, _baseSQL) {}

  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::TpdPsr*>& lstTP) { findAllTpdPsr(lstTP); }

  void findAllTpdPsr(std::vector<tse::TpdPsr*>& lstTP);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllTpdPsrBase
} // namespace tse
