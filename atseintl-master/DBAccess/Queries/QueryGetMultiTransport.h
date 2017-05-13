//----------------------------------------------------------------------------
//          File:           QueryGetMultiTransport.h
//          Description:    QueryGetMultiTransport
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
#include "DBAccess/MultiTransport.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMultiTransport : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransport(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMultiTransport(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMultiTransport() {};

  virtual const char* getQueryName() const override;

  void findMultiTransport(std::vector<tse::MultiTransport*>& multiTransport, const LocCode& loc);

  static void initialize();

  const QueryGetMultiTransport& operator=(const QueryGetMultiTransport& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetMultiTransport& operator=(const std::string& Another)
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
}; // class QueryGetMultiTransport

class QueryGetAllMultiTransport : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMultiTransport(DBAdapter* dbAdapt) : QueryGetMultiTransport(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MultiTransport*>& multiTransport)
  {
    findAllMultiTransport(multiTransport);
  }

  void findAllMultiTransport(std::vector<tse::MultiTransport*>& multiTransport);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMultiTransport

class QueryGetMultiTransportCity : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransportCity(DBAdapter* dbAdapt) : QueryGetMultiTransport(dbAdapt, _baseSQL) {};
  QueryGetMultiTransportCity(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : QueryGetMultiTransport(dbAdapt, sqlStatement) {};

  virtual const char* getQueryName() const override;

  void
  findMultiTransportCity(std::vector<tse::MultiTransport*>& multiTransport, const LocCode& loc);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMultiTransportCity

class QueryGetAllMultiTransportCity : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMultiTransportCity(DBAdapter* dbAdapt) : QueryGetMultiTransport(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MultiTransport*>& multiTransport)
  {
    findAllMultiTransportCity(multiTransport);
  }

  void findAllMultiTransportCity(std::vector<tse::MultiTransport*>& multiTransport);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetAllMultiTransportCity

class QueryGetMultiTransportLocs : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransportLocs(DBAdapter* dbAdapt) : QueryGetMultiTransport(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void
  findMultiTransportLocs(std::vector<tse::MultiTransport*>& multiTransport, const LocCode& loc);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMultiTransportLocs

class QueryGetAllMultiTransportLocs : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetAllMultiTransportLocs(DBAdapter* dbAdapt) : QueryGetMultiTransport(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void execute(std::vector<tse::MultiTransport*>& multiTransport)
  {
    findAllMultiTransportLocs(multiTransport);
  }

  void findAllMultiTransportLocs(std::vector<tse::MultiTransport*>& multiTransport);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

// --------------------------------------------------
// Historical DAO: QueryGetMultiTransportLocsHistorical
// --------------------------------------------------

class QueryGetMultiTransportLocsHistorical : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransportLocsHistorical(DBAdapter* dbAdapt)
    : QueryGetMultiTransport(dbAdapt, _baseSQL) {};

  virtual const char* getQueryName() const override;

  void findMultiTransportLocsHistorical(std::vector<tse::MultiTransport*>& multiTransport,
                                        const LocCode& loc,
                                        const DateTime& startDate,
                                        const DateTime& endDate);
  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

// --------------------------------------------------
// Historical DAO: QueryGetMultiTransportHistorical
// --------------------------------------------------

class QueryGetMultiTransportHistorical : public QueryGetMultiTransport
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransportHistorical(DBAdapter* dbAdapt)
    : QueryGetMultiTransport(dbAdapt, _baseSQL) {};
  virtual ~QueryGetMultiTransportHistorical() {};

  virtual const char* getQueryName() const override;

  void findMultiTransport(std::vector<tse::MultiTransport*>& multiTransport,
                          const LocCode& loc,
                          const DateTime& startDate,
                          const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMultiTransportHistorical

// --------------------------------------------------
// Historical DAO: QueryGetMultiTransportCityHistorical
// --------------------------------------------------

class QueryGetMultiTransportCityHistorical : public QueryGetMultiTransportCity
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMultiTransportCityHistorical(DBAdapter* dbAdapt)
    : QueryGetMultiTransportCity(dbAdapt, _baseSQL) {};
  ~QueryGetMultiTransportCityHistorical() {};

  virtual const char* getQueryName() const override;

  void findMultiTransportCity(std::vector<tse::MultiTransport*>& multiTransport,
                              const LocCode& loc,
                              const DateTime& startDate,
                              const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetMultiTransportCityHistorical

} // namespace tse

