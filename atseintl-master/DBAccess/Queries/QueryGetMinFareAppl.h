//----------------------------------------------------------------------------
//          File:           QueryGetMinFareAppl.h
//          Description:    QueryGetMinFareAppl
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
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetMinFareRules : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRules(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareRules(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareRules() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getRulesAndFootnotes(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareRules& operator=(const QueryGetMinFareRules& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRules& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRules

class QueryGetMinFareFareClasses : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareFareClasses(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareFareClasses(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareFareClasses() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareClasses(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareFareClasses& operator=(const QueryGetMinFareFareClasses& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareFareClasses& operator=(const std::string& Another)
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
}; // class QueryGetMinFareFareClasses

class QueryGetMinFareFareTypes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareFareTypes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareFareTypes(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareFareTypes() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareTypes(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareFareTypes& operator=(const QueryGetMinFareFareTypes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareFareTypes& operator=(const std::string& Another)
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
}; // class QueryGetMinFareFareTypes

class QueryGetMinFareDomFareTypes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareDomFareTypes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareDomFareTypes(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareDomFareTypes() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getDomFareTypes(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareDomFareTypes& operator=(const QueryGetMinFareDomFareTypes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareDomFareTypes& operator=(const std::string& Another)
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
}; // class QueryGetMinFareDomFareTypes

class QueryGetMinFareCxrFltRestrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareCxrFltRestrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareCxrFltRestrs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareCxrFltRestrs() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getCxrFltRestrs(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareCxrFltRestrs& operator=(const QueryGetMinFareCxrFltRestrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareCxrFltRestrs& operator=(const std::string& Another)
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
}; // class QueryGetMinFareCxrFltRestrs

class QueryGetMinFareSecCxrRestrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareSecCxrRestrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareSecCxrRestrs(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareSecCxrRestrs() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getSecCxrRestrs(MinFareAppl* a_pMinFareAppl);

  static void initialize();

  const QueryGetMinFareSecCxrRestrs& operator=(const QueryGetMinFareSecCxrRestrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareSecCxrRestrs& operator=(const std::string& Another)
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
}; // class QueryGetMinFareSecCxrRestrs

class QueryGetMinFareApplBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareApplBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareApplBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareApplBase() {};
  virtual const char* getQueryName() const override;

  void findMinFareAppl(std::vector<tse::MinFareAppl*>& lstMFA,
                       VendorCode& textTblVendor,
                       int textTblItemNo,
                       CarrierCode& govCxr);

  static void initialize();

  const QueryGetMinFareApplBase& operator=(const QueryGetMinFareApplBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareApplBase& operator=(const std::string& Another)
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
}; // class QueryGetMinFareApplBase

class QueryGetMinFareApplBaseHistorical : public QueryGetMinFareApplBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareApplBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareApplBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareApplBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findMinFareAppl(std::vector<tse::MinFareAppl*>& lstMFA,
                       VendorCode& textTblVendor,
                       int textTblItemNo,
                       CarrierCode& govCxr,
                       const DateTime& startDate,
                       const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMinFareApplBaseHist
} // namespace tse
