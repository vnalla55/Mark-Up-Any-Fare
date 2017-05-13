//----------------------------------------------------------------------------
//          File:           QueryGetLimitations.h
//          Description:    QueryGetLimitations
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
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/LimitFareCxrLoc.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetLimitJrnyBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitJrnyBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetLimitJrnyBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetLimitJrnyBase() {};
  virtual const char* getQueryName() const override;

  void
  findLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys, const UserApplCode& userAppl);

  static void initialize();

  const QueryGetLimitJrnyBase& operator=(const QueryGetLimitJrnyBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitJrnyBase& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static int charToInt(const char* s);

protected:
  void getLimitationText(std::vector<tse::LimitationJrny*>& limJrnys);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitJrnyBase

class QueryGetLimitJrnyBaseHistorical : public tse::QueryGetLimitJrnyBase
{
public:
  QueryGetLimitJrnyBaseHistorical(DBAdapter* dbAdapt) : QueryGetLimitJrnyBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitJrnyBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void
  findLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys, const UserApplCode& userAppl);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitJrnyBaseHistorical

class QueryGetAllLimitJrnyBase : public tse::QueryGetLimitJrnyBase
{
public:
  QueryGetAllLimitJrnyBase(DBAdapter* dbAdapt) : QueryGetLimitJrnyBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitJrnyBase() {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::LimitationJrny*>& limJrnys) { findAllLimitationJrny(limJrnys); }

  void findAllLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitJrnyBase

class QueryGetAllLimitJrnyBaseHistorical : public tse::QueryGetLimitJrnyBase
{
public:
  QueryGetAllLimitJrnyBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetLimitJrnyBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitJrnyBaseHistorical() {};
  virtual const char* getQueryName() const override;
  void findAllLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitJrnyBaseHistorical

class QueryGetLimitJrnyTxtMsg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitJrnyTxtMsg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitJrnyTxtMsg() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getText(LimitationJrny* a_pLimitationJrny);
  static void initialize();

  const QueryGetLimitJrnyTxtMsg& operator=(const QueryGetLimitJrnyTxtMsg& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitJrnyTxtMsg& operator=(const std::string& Another)
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
}; // class QueryGetLimitJrnyTxtMsg

class QueryGetLimitationPU : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitationPU(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetLimitationPU(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetLimitationPU() {};
  virtual const char* getQueryName() const override;

  void findLimitationPU(std::vector<tse::LimitationCmn*>& limPUs, const UserApplCode& userAppl);

  static void initialize();

  const QueryGetLimitationPU& operator=(const QueryGetLimitationPU& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitationPU& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static int charToInt(const char* s);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitationPU

class QueryGetLimitationPUHistorical : public tse::QueryGetLimitationPU
{
public:
  QueryGetLimitationPUHistorical(DBAdapter* dbAdapt) : QueryGetLimitationPU(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitationPUHistorical() {};
  virtual const char* getQueryName() const override;

  void findLimitationPU(std::vector<tse::LimitationCmn*>& limPUs, const UserApplCode& userAppl);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitationPUHistorical

class QueryGetAllLimitationPU : public tse::QueryGetLimitationPU
{
public:
  QueryGetAllLimitationPU(DBAdapter* dbAdapt) : QueryGetLimitationPU(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitationPU() {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::LimitationCmn*>& limPUs) { findAllLimitationPU(limPUs); }

  void findAllLimitationPU(std::vector<tse::LimitationCmn*>& limPUs);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitationPU

class QueryGetAllLimitationPUHistorical : public tse::QueryGetLimitationPU
{
public:
  QueryGetAllLimitationPUHistorical(DBAdapter* dbAdapt)
    : QueryGetLimitationPU(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitationPUHistorical() {};
  virtual const char* getQueryName() const override;
  void findAllLimitationPU(std::vector<tse::LimitationCmn*>& limPUs);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitationPUHistorical

class QueryGetLimitFareBase : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitFareBase(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetLimitFareBase(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetLimitFareBase() {};

  virtual const char* getQueryName() const override;

  void
  findLimitationFare(std::vector<tse::LimitationFare*>& limFares, const UserApplCode& userAppl);

  static void initialize();

  const QueryGetLimitFareBase& operator=(const QueryGetLimitFareBase& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitFareBase& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

  static int charToInt(const char* s);

protected:
  void getLimitationCarriers(std::vector<tse::LimitationFare*>& limFares);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitFareBase

class QueryGetLimitFareBaseHistorical : public tse::QueryGetLimitFareBase
{
public:
  QueryGetLimitFareBaseHistorical(DBAdapter* dbAdapt) : QueryGetLimitFareBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitFareBaseHistorical() {};
  virtual const char* getQueryName() const override;

  void
  findLimitationFare(std::vector<tse::LimitationFare*>& limFares, const UserApplCode& userAppl);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetLimitFareBaseHistorical

class QueryGetAllLimitFareBase : public tse::QueryGetLimitFareBase
{
public:
  QueryGetAllLimitFareBase(DBAdapter* dbAdapt) : QueryGetLimitFareBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitFareBase() {};
  virtual const char* getQueryName() const override;
  void execute(std::vector<tse::LimitationFare*>& limFares) { findAllLimitationFare(limFares); }

  void findAllLimitationFare(std::vector<tse::LimitationFare*>& limFares);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitFareBase

class QueryGetAllLimitFareBaseHistorical : public tse::QueryGetLimitFareBase
{
public:
  QueryGetAllLimitFareBaseHistorical(DBAdapter* dbAdapt)
    : QueryGetLimitFareBase(dbAdapt, _baseSQL) {};
  virtual ~QueryGetAllLimitFareBaseHistorical() {};
  virtual const char* getQueryName() const override;
  void findAllLimitationFare(std::vector<tse::LimitationFare*>& limFares);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllLimitFareBaseHistorical

class QueryGetLimitFareGovCxrs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitFareGovCxrs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitFareGovCxrs() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getGovCxrs(LimitationFare* a_pLimitationFare);
  static void initialize();

  const QueryGetLimitFareGovCxrs& operator=(const QueryGetLimitFareGovCxrs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitFareGovCxrs& operator=(const std::string& Another)
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
}; // class QueryGetLimitFareGovCxrs

class QueryGetLimitFareCxrLocs : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitFareCxrLocs(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitFareCxrLocs() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getCxrLocs(LimitationFare* a_pLimitationFare);
  static void initialize();

  const QueryGetLimitFareCxrLocs& operator=(const QueryGetLimitFareCxrLocs& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitFareCxrLocs& operator=(const std::string& Another)
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
}; // class QueryGetLimitFareCxrLocs

class QueryGetLimitFareRoutings : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetLimitFareRoutings(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  virtual ~QueryGetLimitFareRoutings() {};
  void resetSQL()
  {
    *this = _baseSQL;
  };
  const char* getQueryName() const override;
  void getRoutings(LimitationFare* a_pLimitationFare);
  static void initialize();

  const QueryGetLimitFareRoutings& operator=(const QueryGetLimitFareRoutings& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetLimitFareRoutings& operator=(const std::string& Another)
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
}; // class QueryGetLimitFareRoutings
} // namespace tse
