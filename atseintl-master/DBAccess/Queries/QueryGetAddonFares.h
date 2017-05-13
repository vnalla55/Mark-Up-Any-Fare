//----------------------------------------------------------------------------
//          File:           QueryGetAddonFares.h
//          Description:    QueryGetAddonFares
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
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetSitaAddonQualCodes : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonQualCodes(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSitaAddonQualCodes(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSitaAddonQualCodes() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getQualCodes(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonQualCodes& operator=(const QueryGetSitaAddonQualCodes& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonQualCodes& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonQualCodes

class QueryGetSitaAddonQualCodesHistorical : public QueryGetSitaAddonQualCodes
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonQualCodesHistorical(DBAdapter* dbAdapt)
    : QueryGetSitaAddonQualCodes(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSitaAddonQualCodesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getQualCodes(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonQualCodesHistorical&
  operator=(const QueryGetSitaAddonQualCodesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonQualCodesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonQualCodesHistorical

class QueryGetSitaAddonDBEClasses : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonDBEClasses(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSitaAddonDBEClasses(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSitaAddonDBEClasses() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getDBEClasses(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonDBEClasses& operator=(const QueryGetSitaAddonDBEClasses& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonDBEClasses& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonDBEClasses

class QueryGetSitaAddonDBEClassesHistorical : public QueryGetSitaAddonDBEClasses
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonDBEClassesHistorical(DBAdapter* dbAdapt)
    : QueryGetSitaAddonDBEClasses(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSitaAddonDBEClassesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getDBEClasses(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonDBEClassesHistorical&
  operator=(const QueryGetSitaAddonDBEClassesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonDBEClassesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonDBEClassesHistorical

class QueryGetSitaAddonRules : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonRules(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetSitaAddonRules(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetSitaAddonRules() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getRules(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonRules& operator=(const QueryGetSitaAddonRules& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonRules& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonRules

class QueryGetSitaAddonRulesHistorical : public QueryGetSitaAddonRules
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetSitaAddonRulesHistorical(DBAdapter* dbAdapt)
    : QueryGetSitaAddonRules(dbAdapt, _baseSQL) {};
  virtual ~QueryGetSitaAddonRulesHistorical() {};
  const char* getQueryName() const override;
  void resetSQL()
  {
    *this = _baseSQL;
  };

  void getRules(SITAAddonFareInfo* a_pSITAAddonFareInfo);

  static void initialize();

  const QueryGetSitaAddonRulesHistorical& operator=(const QueryGetSitaAddonRulesHistorical& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetSitaAddonRulesHistorical& operator=(const std::string& Another)
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
}; // class QueryGetSitaAddonRulesHistorical

class QueryGetAddonFares : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonFares(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAddonFares(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAddonFares() {};
  virtual const char* getQueryName() const override;

  void findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                         const LocCode& interiorMarket,
                         const CarrierCode& carrier);

  static void initialize();

  const QueryGetAddonFares& operator=(const QueryGetAddonFares& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAddonFares& operator=(const std::string& Another)
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
}; // class QueryGetAddonFares

class QueryGetAddonFaresHistorical : public QueryGetAddonFares
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonFaresHistorical(DBAdapter* dbAdapt) : QueryGetAddonFares(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonFaresHistorical() {};
  virtual const char* getQueryName() const override;

  void findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                         const LocCode& interiorMarket,
                         const CarrierCode& carrier,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAddonFaresHistorical

class QueryGetAddonFaresGW : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonFaresGW(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetAddonFaresGW(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetAddonFaresGW() {};

  const char* getQueryName() const override;

  void findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                         const LocCode& gatewayMarket,
                         const LocCode& interiorMarket,
                         const CarrierCode& carrier);

  static void initialize();

  const QueryGetAddonFaresGW& operator=(const QueryGetAddonFaresGW& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetAddonFaresGW& operator=(const std::string& Another)
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
}; // class QueryGetAddonFaresGW

class QueryGetAddonFaresGWHistorical : public QueryGetAddonFaresGW
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAddonFaresGWHistorical(DBAdapter* dbAdapt) : QueryGetAddonFaresGW(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAddonFaresGWHistorical() {};
  const char* getQueryName() const override;

  void findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                         const LocCode& gatewayMarket,
                         const LocCode& interiorMarket,
                         const CarrierCode& carrier,
                         const DateTime& startDate,
                         const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAddonFaresGWHistorical
} // namespace tse
