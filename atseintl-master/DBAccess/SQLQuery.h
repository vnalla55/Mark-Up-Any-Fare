//----------------------------------------------------------------------------
//
//              File:           SQLQuery.h
//              Description:    SQLQuery
//              Created:        3/2/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//         Copyright 2009, Sabre Inc.  All rights reserved.  This software/documentation is the
// confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBAdapter.h"
#include "DBAccess/ParameterBinder.h"
#include "DBAccess/ParameterSubstitutor.h"

#include <boost/thread/mutex.hpp>
#include <list>
#include <set>
#include <typeinfo>

#include <boost/container/string.hpp>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#ifndef LOG4CXX_EXECUTECODE_INFO
#define LOG4CXX_EXECUTECODE_INFO(logger, codesnippet)                                              \
  {                                                                                                \
    if (logger->isInfoEnabled())                                                                   \
    {                                                                                              \
      codesnippet;                                                                                 \
    }                                                                                              \
  }
#endif

namespace DBAccess
{
class BoundParameter;
}

namespace tse
{

//////////////////////////////////////////////////////////////////////
//         Initialization helpers
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//  SQLQueryInitializer - base class for initializing queries
//
//  The only member function that is used externally is the
//  static initAllQueryClasses function to cause the query classes
//  to initialize.  The only other use of this class is meant to be
//  through the SQLQueryInitializer template.
/////////////////////////////////////////////////////////////////////

class SQLQueryInitializer;

class SQLQueryInitializer
{
public:
  static void initAllQueryClasses(bool isHistorical);

  virtual void classInit() = 0;
  virtual void classdeInit() = 0;

protected:
  SQLQueryInitializer(bool isHistorical);
  virtual ~SQLQueryInitializer();

private:
  void registerClass(SQLQueryInitializer* regClass, bool isHistorical);

  static boost::mutex& getMutex();
  static std::list<SQLQueryInitializer*>& getInitList(bool isHistorical);
};

///////////////////////////////////////////////////////////////////////
// SQLQueryInitializerHelper - template used to register a query
//                             class for initialization
//
//  Instantiating an instance of this template for a given query
//  class will cause the class "initialize()" function to get invoked.
///////////////////////////////////////////////////////////////////////

template <typename CLASS>
class SQLQueryInitializerHelper : SQLQueryInitializer
{
public:
  SQLQueryInitializerHelper(bool isHistorical = false) : SQLQueryInitializer(isHistorical) {};
  virtual ~SQLQueryInitializerHelper() {};

  void classInit() override { CLASS::initialize(); }
  void classdeInit() override { CLASS::deinitialize(); }
};

//////////////////////////////////////////////////////////////////////
//    Base class for queries
//////////////////////////////////////////////////////////////////////

class SQLQuery : public std::string, public DBAccess::ParameterBinder
{
  template <typename Query>
  friend class DBAccessTestHelper;

public:
  SQLQuery() = default;
  SQLQuery(DBAdapter* dbAdapt) : _dbAdapt(dbAdapt) {}
  SQLQuery(DBAdapter* dbAdapt, const std::string& sqlStatement) : _dbAdapt(dbAdapt)
  {
    const MallocContextDisabler context;
    std::string& str = *this;
    str = sqlStatement;
  }

  SQLQuery(const char* sqlStatement)
  {
    const MallocContextDisabler context;
    std::string& str = *this;
    str = sqlStatement;
  }

  SQLQuery(DBAdapter* dbAdapt, const char* sqlStatement) : _dbAdapt(dbAdapt)
  {
    const MallocContextDisabler context;
    std::string& str = *this;
    str = sqlStatement;
  }

  SQLQuery(const std::string& sqlStatement)
  {
    const MallocContextDisabler context;
    std::string& str = *this;
    str = sqlStatement;
  }

  virtual ~SQLQuery() = default;

  static bool classInit(ConfigMan& config, std::string& dbSection, std::string& key);
  static bool classInit(const std::string& CacheKey,
                        ConfigMan& config,
                        std::string& dbSection,
                        std::string& key);

  virtual const char* getQueryName() const;
  const char* getSQL() const;
  const SQLQuery& operator=(const SQLQuery& Another);
  const SQLQuery& operator=(const std::string& Another);

  log4cxx::LoggerPtr& logger() const { return _logger; }

  void substParm(const char* parm, int index, bool forceLiteral = false);
  void substParm(const std::string& parm, int index, bool forceLiteral = false);
  void substParm(const boost::container::string& parm, int index, bool forceLiteral = false);

  void substParm(int index, const char* parm, bool forceLiteral = false);
  void substParm(int index, const std::string& parm, bool forceLiteral = false);
  void substParm(int index, const boost::container::string& parm, bool forceLiteral = false);
  void substParm(int index, char parm);
  void substParm(int index, int parm);
  void substParm(int index, int64_t parm);
  void substParm(int index, const DateTime& parm);
  void substParm(int index, const std::vector<CarrierCode>& parm);
  void substParm(int index, const std::vector<int64_t>& parm);

  template <size_t n>
  void substParm(const Code<n>& parm, int index)
  {
    substParm(parm.c_str(), index);
  }
  void substCurrentDate(bool dateOnly = true);

  void clearBoundParameters();

  void bindAllParameters() const override;
  void bindParameter(const char* parm, int32_t index) const override;
  void bindParameter(const std::string& parm, int32_t index) const override;
  void bindParameter(int32_t parm, int32_t index) const override;
  void bindParameter(int64_t parm, int32_t index) const override;
  void bindParameter(float parm, int32_t index) const override;
  void bindParameter(const DateTime& parm, int32_t index, bool dateOnly = false) const override;

  void bindParameter(DBAccess::BoundParameter* parm) const override;

  static double adjustDecimal(long amount, int noDec); // rLyle
  static void registerBaseSQL(const std::string& rclassKey, const std::string& baseSQL);

  virtual uint32_t getArrayFetchSize() const;
  virtual uint32_t getMaxDefineBufferStackSize() const;
  void fillParameterString(std::string& displayString);
  void getSQLString(std::string& resultString);

protected:
  DBAdapter* _dbAdapt = nullptr;
  static void substTableDef(std::string*);
  static const std::set<std::string>& getQueryClassNames();
  static const std::string& getBaseSQLForClass(const std::string& queryClassName);

  char startTimer();
  int stopTimer();

  void startCPU();
  int stopCPU();

private:
  static boost::mutex _mutex; // Thread Safety
  static log4cxx::LoggerPtr _logger;

  typedef std::map<std::string, std::string> NameMapping;
  static std::map<std::string, NameMapping> _tableNames;

  // Members to facilitate query string validation
  static std::map<std::string, std::string> _tableDefs;
  static std::map<std::string, std::string> _classNameBaseSQL;
  static std::set<std::string> _queryClassName;

  // Timer Stuff to check Query performance
  long long _tmStart = 0;
  tms _cpuStart;
  static constexpr int clk_tck = 100;

  DBAccess::ParameterSubstitutor _parameterSubstitutor;

  static void loadTableDefs(const std::string& classKey,
                            ConfigMan& config,
                            std::string& dbSection,
                            std::string& key);
  static void loadTableDefs(ConfigMan& config, std::string& dbSection, std::string& key);
  static bool isValidTableName(const std::string& tabName);

// Query Options

#define DEFAULT_ARRAY_FETCH_SIZE 50
#define DEFAULT_MAX_DEFINE_BUFFER_STACK_SIZE 5

  class SQLQueryOptions
  {
  public:
    SQLQueryOptions()
      : _arrayFetchSize(DEFAULT_ARRAY_FETCH_SIZE),
        _maxDefineBufferStackSize(DEFAULT_MAX_DEFINE_BUFFER_STACK_SIZE)
    {
    }

    SQLQueryOptions(uint16_t arrayFetchSize, uint16_t maxDefineBufferStackSize)
      : _arrayFetchSize(arrayFetchSize), _maxDefineBufferStackSize(maxDefineBufferStackSize)
    {
    }

    SQLQueryOptions(const SQLQueryOptions& opt)
      : _arrayFetchSize(opt._arrayFetchSize),
        _maxDefineBufferStackSize(opt._maxDefineBufferStackSize)
    {
    }

    SQLQueryOptions& operator=(const SQLQueryOptions& rhs)
    {
      if (this == &rhs)
        return *this;

      _arrayFetchSize = rhs._arrayFetchSize;
      _maxDefineBufferStackSize = rhs._maxDefineBufferStackSize;
      return *this;
    }

    uint16_t getArrayFetchSize(void) { return _arrayFetchSize; }
    uint16_t getMaxDefineBufferStackSize(void) { return _maxDefineBufferStackSize; }

  private:
    uint16_t _arrayFetchSize;
    uint16_t _maxDefineBufferStackSize;
  };

  static std::map<std::string, SQLQueryOptions> _queryOptions;
  static void loadQueryOptions(ConfigMan& config);

  static uint16_t _defaultArrayFetchSize;
  static uint16_t _defaultMaxDefineBufferStackSize;
};
}
