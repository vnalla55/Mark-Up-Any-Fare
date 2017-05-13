//----------------------------------------------------------------------------
//          File:           QueryGetMinFareRule.h
//          Description:    QueryGetMinFareRule
//          Created:        3/2/2006
//          Authors:         Mike Lillis
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
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMinFareRuleFareClasses : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleFareClasses(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareRuleFareClasses(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareRuleFareClasses() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareClasses(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleFareClasses& operator=(const QueryGetMinFareRuleFareClasses& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleFareClasses& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleFareClasses

class QueryGetMinFareRuleFareClassesHistorical : public QueryGetMinFareRuleFareClasses
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleFareClassesHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareRuleFareClasses(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareRuleFareClassesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareClasses(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleFareClassesHistorical&
  operator=(const QueryGetMinFareRuleFareClassesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleFareClassesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleFareClassesHistorical

class QueryGetMinFareRuleFareTypes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleFareTypes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareRuleFareTypes(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareRuleFareTypes() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareTypes(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleFareTypes& operator=(const QueryGetMinFareRuleFareTypes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleFareTypes& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleFareTypes

class QueryGetMinFareRuleFareTypesHistorical : public QueryGetMinFareRuleFareTypes
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleFareTypesHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareRuleFareTypes(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareRuleFareTypesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareTypes(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleFareTypesHistorical&
  operator=(const QueryGetMinFareRuleFareTypesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleFareTypesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleFareTypesHistorical

class QueryGetMinFareRuleSameFareGroupChild : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleSameFareGroupChild(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareRuleSameFareGroupChild(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareRuleSameFareGroupChild() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareSet(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleSameFareGroupChild&
  operator=(const QueryGetMinFareRuleSameFareGroupChild& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleSameFareGroupChild& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleSameFareGroupChild

class QueryGetMinFareRuleSameFareGroupChildHistorical : public QueryGetMinFareRuleSameFareGroupChild
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleSameFareGroupChildHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareRuleSameFareGroupChild(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareRuleSameFareGroupChildHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getFareSet(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl);

  static void initialize();

  const QueryGetMinFareRuleSameFareGroupChildHistorical&
  operator=(const QueryGetMinFareRuleSameFareGroupChildHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleSameFareGroupChildHistorical& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleSameFareGroupChildHistorical

class QueryGetMinFareRuleLevExclBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleLevExclBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMinFareRuleLevExclBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMinFareRuleLevExclBase() {};
  virtual const char* getQueryName() const override;

  void findMinFareRuleLevelExcl(std::vector<tse::MinFareRuleLevelExcl*>& lstMFRLE,
                                VendorCode& vendor,
                                int textTblItemNo,
                                CarrierCode& govCxr,
                                TariffNumber rulTrf);

  static void initialize();

  const QueryGetMinFareRuleLevExclBase& operator=(const QueryGetMinFareRuleLevExclBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMinFareRuleLevExclBase& operator=(const std::string& Another)
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
}; // class QueryGetMinFareRuleLevExclBase

class QueryGetMinFareRuleLevExclBaseHistorical : public QueryGetMinFareRuleLevExclBase
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMinFareRuleLevExclBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetMinFareRuleLevExclBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMinFareRuleLevExclBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void findMinFareRuleLevelExcl(std::vector<tse::MinFareRuleLevelExcl*>& lstMFRLE,
                                VendorCode& vendor,
                                int textTblItemNo,
                                CarrierCode& govCxr,
                                TariffNumber rulTrf,
                                const DateTime& startDate,
                                const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMinFareRuleLevExclBaseHistorical
} // namespace tse
