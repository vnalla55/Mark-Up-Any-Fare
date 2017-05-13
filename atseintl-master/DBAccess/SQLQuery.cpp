//----------------------------------------------------------------------------
//
//              File:           SQLQuery.cpp
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

#include "DBAccess/SQLQuery.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DBAccessConsts.h"
#include "DBAccess/SQLStatementHelper.h"

#include <iostream>
#include <sstream>
#include <string>

#include <boost/tokenizer.hpp>
#include <errno.h>
#include <sys/timeb.h>
#include <time.h>

namespace tse
{
boost::mutex&
SQLQueryInitializer::getMutex()
{
  static boost::mutex _mutex;

  return _mutex;
}

std::list<SQLQueryInitializer*>&
SQLQueryInitializer::getInitList(bool isHistorical)
{
  static std::list<SQLQueryInitializer*> _list;
  static std::list<SQLQueryInitializer*> _listHistorical;

  return isHistorical ? _listHistorical : _list;
}

SQLQueryInitializer::SQLQueryInitializer(bool isHistorical) { registerClass(this, isHistorical); }
SQLQueryInitializer::~SQLQueryInitializer() {}

void
SQLQueryInitializer::registerClass(SQLQueryInitializer* regClass, bool isHistorical)
{
  boost::lock_guard<boost::mutex> g(getMutex());
  getInitList(isHistorical).push_back(regClass);
}

void
SQLQueryInitializer::initAllQueryClasses(bool isHistorical)
{
  boost::lock_guard<boost::mutex> g(getMutex());

  std::list<SQLQueryInitializer*>& l_list = getInitList(isHistorical);

  std::list<SQLQueryInitializer*>::const_iterator iter = l_list.begin();

  for (; iter != l_list.end(); ++iter)
  {
    (*iter)->classInit();
  }
};

//----------------------------------------
// Base class for queries
//----------------------------------------

log4cxx::LoggerPtr
SQLQuery::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery"));
boost::mutex SQLQuery::_mutex; // Thread Safety
std::map<std::string, std::string> SQLQuery::_tableDefs;
std::map<std::string, SQLQuery::NameMapping> SQLQuery::_tableNames;
std::map<std::string, std::string> SQLQuery::_classNameBaseSQL;
std::set<std::string> SQLQuery::_queryClassName;

uint16_t SQLQuery::_defaultArrayFetchSize = DEFAULT_ARRAY_FETCH_SIZE;
uint16_t SQLQuery::_defaultMaxDefineBufferStackSize = DEFAULT_MAX_DEFINE_BUFFER_STACK_SIZE;

std::map<std::string, SQLQuery::SQLQueryOptions> SQLQuery::_queryOptions;

bool
SQLQuery::classInit(ConfigMan& config, std::string& dbSection, std::string& key)
{
  loadTableDefs(config, dbSection, key);

  loadQueryOptions(config);
  return true;
}

bool
SQLQuery::classInit(const std::string& CacheKey,
                    ConfigMan& config,
                    std::string& dbSection,
                    std::string& key)
{
  loadTableDefs(CacheKey, config, dbSection, key);
  return true;
}

const char*
SQLQuery::getQueryName() const
{
  return "";
}

const char*
SQLQuery::getSQL() const
{
  return this->c_str();
}

//---------------------------------------------------------------------------
// loadQueryOptions function
// loads query options for arrayFetchSize and maxBufferCollectionSize
//---------------------------------------------------------------------------
void
SQLQuery::loadQueryOptions(ConfigMan& config)
{
  boost::lock_guard<boost::mutex> g(_mutex);

  //----------------------------------------------
  // QUERY_OPTIONS
  //----------------------------------------------

  enum eQueryOptions
  {
    eArrayFetchSize = 1,
    eMaxBufferStackSize,
  };

  _defaultArrayFetchSize = DEFAULT_ARRAY_FETCH_SIZE;
  _defaultMaxDefineBufferStackSize = DEFAULT_MAX_DEFINE_BUFFER_STACK_SIZE;

  // Read Overrides for Default Values

  std::string tempValue;

  if (config.getValue("DEFAULT_ARRAY_FETCH_SIZE", tempValue, "SQLQUERY_OPTIONS_DEFAULTS"))
  {
    errno = 0;
    long temp = strtol(tempValue.c_str(), nullptr, 10);
    if ((temp != 0) || (errno == 0))
    {
      _defaultArrayFetchSize = (temp > 65535) ? 65535 : static_cast<uint16_t>(temp);
    }
  }

  if (config.getValue(
          "DEFAULT_MAX_DEFINE_BUFFER_STACK_SIZE", tempValue, "SQLQUERY_OPTIONS_DEFAULTS"))
  {
    errno = 0;
    long temp = strtol(tempValue.c_str(), nullptr, 10);
    if ((temp != 0) || (errno == 0))
    {
      _defaultMaxDefineBufferStackSize = (temp > 65535) ? 65535 : static_cast<uint16_t>(temp);
    }
  }

  LOG4CXX_INFO(_logger,
               "Setting SQLQuery Options: default ARRAY_FETCH_SIZE to " << _defaultArrayFetchSize
                                                                        << ".");
  LOG4CXX_INFO(_logger,
               "Setting SQLQuery Options: default MAX_DEFINE_BUFFER_STACK_SIZE to "
                   << _defaultMaxDefineBufferStackSize << ".");

  // Read Overrides per Query

  std::vector<ConfigMan::NameValue> cfgValues;
  if (config.getValues(cfgValues, "SQLQUERY_OPTIONS"))
  {
    boost::char_separator<char> fieldSep("|", "", boost::keep_empty_tokens);

    uint16_t arrayFetchSize = _defaultArrayFetchSize;
    uint16_t maxBufferStackSize = _defaultMaxDefineBufferStackSize;

    for (auto& cfgValue : cfgValues)
    {
      std::string& queryName = cfgValue.name;

      boost::tokenizer<boost::char_separator<char>> fields(cfgValue.value, fieldSep);

      int num = 1;
      for (boost::tokenizer<boost::char_separator<char> >::const_iterator f = fields.begin();
           f != fields.end();
           ++f, ++num)
      {
        const std::string& field = (*f);
        switch (num)
        {
        case eArrayFetchSize:
        {
          long tempSize = atol(field.c_str());

          arrayFetchSize = (tempSize > 65535) ? 65535 : tempSize;

          break;
        }
        case eMaxBufferStackSize:
        {
          long tempSize = atol(field.c_str());

          maxBufferStackSize = (tempSize > 65535) ? 65535 : tempSize;
          break;
        }
        }
      }

      if (!queryName.empty() && (arrayFetchSize > 0) && (maxBufferStackSize > 0))
      {
        _queryOptions[queryName] = SQLQuery::SQLQueryOptions(arrayFetchSize, maxBufferStackSize);

        LOG4CXX_INFO(_logger,
                     "Setting SQLQuery Options: Query ["
                         << queryName << "], Array Fetch Size [" << arrayFetchSize
                         << "], Max Define Buffers [" << maxBufferStackSize << "].");
      }
    }
  }
}

//---------------------------------------------------------------------------
// loadTableDefs function
// loads table substitutions based on a
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
void
SQLQuery::loadTableDefs(const std::string& classKey,
                        ConfigMan& config,
                        std::string& dbSection,
                        std::string& key)
{
  boost::lock_guard<boost::mutex> g(_mutex);
  std::string iniFileName;

  // Get the name of the file that has the Database table and column mappings, etc
  //   (Look first in the section of the ini file for this database.  If it is not
  //    there, look in the root section of the file.)

  LOG4CXX_DEBUG(_logger, "Reading INI file from   " << key << "   DB Section is  " << dbSection);

  config.getValue(key, iniFileName, dbSection);
  if (iniFileName.empty())
  {
    config.getValue(key, iniFileName);
  }

  ConfigMan mysqlIni;
  std::vector<ConfigMan::NameValue> tables;
  if (!mysqlIni.read(iniFileName))
    LOG4CXX_WARN(_logger, "unable to read ini file " << iniFileName);
  mysqlIni.getValues(tables);

  NameMapping& l_rNameMap = _tableNames[classKey];

  for (auto& table : tables)
  {
    std::string& name = table.name;
    std::string& value = table.value;
    int n = name.find('\\');
    if (++n > 0)
    {
      if (l_rNameMap.count("=" + name.substr(n, name.length() - n)) == 0)
      {
        l_rNameMap["=" + name.substr(n, name.length() - n)] = value;
      }
    }
  }
}
//---------------------------------------------------------------------------
// registerBaseSQL  function
// register the baseSQL string for a query class
//---------------------------------------------------------------------------
void
SQLQuery::registerBaseSQL(const std::string& classKey, const std::string& sqlStatement)
{
  boost::lock_guard<boost::mutex> g(_mutex);

  if (_queryClassName.find(classKey) != _queryClassName.end())
    LOG4CXX_ERROR(_logger, "Duplicate class name: " << classKey);

  _queryClassName.insert(classKey);
  _classNameBaseSQL[classKey] = sqlStatement;
}

//---------------------------------------------------------------------------
// getArrayFetchSize  function
// return the size to use for array fetching
//---------------------------------------------------------------------------
uint32_t
SQLQuery::getArrayFetchSize() const
{
  std::string queryName = getQueryName();

  std::map<std::string, SQLQueryOptions>::iterator iter = _queryOptions.find(queryName);

  if (iter != _queryOptions.end())
  {
    return (*iter).second.getArrayFetchSize();
  }
  else
  {
    return _defaultArrayFetchSize;
  }
}

//---------------------------------------------------------------------------
// getMaxDefineBufferStackSize  function
// return the size to use for array fetching
//---------------------------------------------------------------------------
uint32_t
SQLQuery::getMaxDefineBufferStackSize() const
{
  std::string queryName = getQueryName();

  std::map<std::string, SQLQueryOptions>::iterator iter = _queryOptions.find(queryName);

  if (iter != _queryOptions.end())
  {
    return (*iter).second.getMaxDefineBufferStackSize();
  }
  else
  {
    return _defaultMaxDefineBufferStackSize;
  }
}

//---------------------------------------------------------------------------
// getQueryClassNames  function
// return the set of class names that have been registered.
//---------------------------------------------------------------------------
const std::set<std::string>&
SQLQuery::getQueryClassNames()
{
  return _queryClassName;
}

//---------------------------------------------------------------------------
// getBaseSQLForClass  function
// return the baseSQL string registered for a given query class
//---------------------------------------------------------------------------
const std::string&
SQLQuery::getBaseSQLForClass(const std::string& queryClassName)
{
  return _classNameBaseSQL[queryClassName];
}

//---------------------------------------------------------------------------
// loadTableDefs function
// loads table substitutions based on a
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
void
SQLQuery::loadTableDefs(ConfigMan& config, std::string& dbSection, std::string& key)
{
  boost::lock_guard<boost::mutex> g(_mutex);
  std::string iniFileName;

  // Get the name of the file that has the Database table and column mappings, etc
  //   (Look first in the section of the ini file for this database.  If it is not
  //    there, look in the root section of the file.)

  config.getValue(key, iniFileName, dbSection);

  if (iniFileName.empty())
  {
    config.getValue(key, iniFileName);
  }

  ConfigMan mysqlIni;
  std::vector<ConfigMan::NameValue> tables;
  if (!mysqlIni.read(iniFileName))
    LOG4CXX_WARN(_logger, "unable to read ini file " << iniFileName);
  mysqlIni.getValues(tables);

  for (auto& table : tables)
  {
    std::string& name = table.name;
    std::string& value = table.value;
    int n = name.find('\\');
    if (++n > 0)
    {
      if (_tableDefs.count("=" + name.substr(n, name.length() - n)) == 0)
      {
        _tableDefs["=" + name.substr(n, name.length() - n)] = value;
      }
    }
  }
}

//---------------------------------------------------------------------------
// substTableDef  function
// substitute table name in SQL (=tablename) for
// mapped table name in mysql.ini file.
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
void
SQLQuery::substTableDef(std::string* sqlCall)
{
  boost::lock_guard<boost::mutex> g(_mutex);
  DBAccess::SQLStatementHelper sqlHelper;

  int pos = 0;
  int prevWhere = 0;
  int limit = 0;
  do
  {
    pos = sqlCall->find("=", pos);
    limit = sqlCall->find("where", prevWhere);
    if (limit == -1)
      limit = sqlCall->size();

    if (pos >= limit || pos == -1)
    {
      limit = sqlCall->find("where", limit + 1);
      if (limit != -1)
      {
        prevWhere = limit + 1;
        continue;
      }
      else
      {
        break;
      }
    }

    int endPos = sqlCall->find(" ", pos);
    std::string tabName = sqlCall->substr(pos, endPos - pos);

    if (sqlHelper.ignoreTableDefReplacement())
    {
      if (isValidTableName(tabName))
      {
        sqlCall->erase(pos, 1);
      }
    }
    else
    {
      if (_tableDefs.find(tabName) != _tableDefs.end())
      {
        sqlCall->replace(pos, endPos - pos, _tableDefs[tabName]);
      }
      else
      {
        if (!sqlHelper.ignoreTableDefMissing() && tabName.size() > 1)
        {
          LOG4CXX_WARN(_logger, "table def not found" << tabName);
        }
      }
    }

    pos++;
  } while (true); // lint !e506
}

double
SQLQuery::adjustDecimal(long amount, int noDec)
{
  static double pow[] = { 1.0,    1.0E1,  1.0E2,  1.0E3,  1.0E4,  1.0E5,  1.0E6,
                          1.0E7,  1.0E8,  1.0E9,  1.0E10, 1.0E11, 1.0E12, 1.0E13,
                          1.0E14, 1.0E15, 1.0E16, 1.0E17, 1.0E18, 1.0E19, 1.0E20 };
  if (UNLIKELY(noDec >= (int)(sizeof(pow) / sizeof(double))))
    return 0;
  return amount / pow[noDec]; // lint !e661
} // adjustDecimal()

char
SQLQuery::startTimer()
{
  startCPU();
  timeb tmTmp;
  ftime(&tmTmp);
  _tmStart = (((long long)tmTmp.time) * 1000) + tmTmp.millitm;
  return ' '; // This function must return a value.  The macro it appears in is expecting one.
} // startTimer()

int
SQLQuery::stopTimer()
{
  timeb tmTmp;
  ftime(&tmTmp);
  long long tmStop = (((long long)tmTmp.time) * 1000) + tmTmp.millitm;
  return (int)(tmStop - _tmStart);
} // stopTimer()

void
SQLQuery::startCPU()
{
  times(&_cpuStart);
}

int
SQLQuery::stopCPU()
{
  tms cpuStop;
  times(&cpuStop);

  return (((cpuStop.tms_utime - _cpuStart.tms_utime) + (cpuStop.tms_stime - _cpuStart.tms_stime)) *
          1000) /
         clk_tck;
}

const SQLQuery&
SQLQuery::
operator=(const SQLQuery& Another)
{
  const MallocContextDisabler context;
  if (this != &Another)
  {
    *((std::string*)this) = (std::string&)Another;
    _tmStart = Another._tmStart;
    _cpuStart = Another._cpuStart;
    _dbAdapt = Another._dbAdapt;
  }
  return *this;
};
const SQLQuery&
SQLQuery::
operator=(const std::string& Another)
{
  const MallocContextDisabler context;
  if (LIKELY(this != &Another))
    *((std::string*)this) = Another;
  return *this;
};

//---------------------------------------------------------------------------
// substParm  function
// SQL paramameters are specified as %(num)([quote][numeric]:
// example :  %2q  "second parm, quote string
// This function substitutes values sent from Factory to the SQL
// Note: when MYSQL version has PreparedStatement we will change these to ?
//---------------------------------------------------------------------------
void
SQLQuery::substParm(const char* parm, int index, bool forceLiteral)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index, forceLiteral);
}

void
SQLQuery::substParm(const std::string& parm, int index, bool forceLiteral)
{
  substParm(parm.c_str(), index, forceLiteral);
}

void
SQLQuery::substParm(const boost::container::string& parm, int index, bool forceLiteral)
{
  substParm(parm.c_str(), index, forceLiteral);
}

/** @TODO remove q's and n's and use overloaded substParm methods like the following */
void
SQLQuery::substParm(int index, const char* parm, bool forceLiteral)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index, forceLiteral);
}

void
SQLQuery::substParm(int index, const std::string& parm, bool forceLiteral)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index, forceLiteral);
}

void
SQLQuery::substParm(int index, const boost::container::string& parm, bool forceLiteral)
{
  _parameterSubstitutor.substituteParameter(
      *((std::string*)this), parm.c_str(), index, forceLiteral);
}

void
SQLQuery::substParm(int index, char parm)
{
  char buf[2];
  buf[0] = parm;
  buf[1] = '\0';
  substParm(buf, index);
}

void
SQLQuery::substParm(int index, int parm)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index);
}

void
SQLQuery::substParm(int index, int64_t parm)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index);
}

void
SQLQuery::substParm(int index, const DateTime& parm)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index);
}

void
SQLQuery::substParm(int index, const std::vector<CarrierCode>& parm)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index);
}

void
SQLQuery::substParm(int index, const std::vector<int64_t>& parm)
{
  _parameterSubstitutor.substituteParameter(*((std::string*)this), parm, index);
}

//---------------------------------------------------------------------------
// substCurrentDate()
// Substitutes all occurrances of "%cd" in the query string with the
// current date in numeric form.
//---------------------------------------------------------------------------
void
SQLQuery::substCurrentDate(bool dateOnly)
{
  DateTime dt;
  if (dateOnly)
  {
    dt = DateTime::localTime();
  }
  else
  {
    time_t t;
    time(&t);
    tm ltime;
    localtime_r(&t, &ltime);
    dt = DateTime(static_cast<uint16_t>(ltime.tm_year + 1900),
                  static_cast<uint16_t>(ltime.tm_mon + 1),
                  static_cast<uint16_t>(ltime.tm_mday),
                  ltime.tm_hour,
                  ltime.tm_min,
                  ltime.tm_sec);
  }
  _parameterSubstitutor.substituteParameter(*((std::string*)this), dt, "%cd", dateOnly);
}

// A valid table name should never be null string or "="
bool
SQLQuery::isValidTableName(const std::string& tabName)
{
  return (!tabName.empty() && strcmp(tabName.c_str(), "=") != 0);
}

void
SQLQuery::clearBoundParameters()
{
  _parameterSubstitutor.clearBoundParameters();
}

void
SQLQuery::bindAllParameters() const
{
  _parameterSubstitutor.bindAllParameters(*this);
}

void
SQLQuery::bindParameter(const char* parm, int32_t index) const
{
  _dbAdapt->bindParameter(parm, index);
}

void
SQLQuery::bindParameter(const std::string& parm, int32_t index) const
{
  _dbAdapt->bindParameter(parm, index);
}

void
SQLQuery::bindParameter(int32_t parm, int32_t index) const
{
  _dbAdapt->bindParameter(parm, index);
}

void
SQLQuery::bindParameter(int64_t parm, int32_t index) const
{
  _dbAdapt->bindParameter(parm, index);
}

void
SQLQuery::bindParameter(float parm, int32_t index) const
{
  _dbAdapt->bindParameter(parm, index);
}

void
SQLQuery::bindParameter(const DateTime& parm, int32_t index, bool dateOnly) const
{
  _dbAdapt->bindParameter(parm, index, dateOnly);
}

void
SQLQuery::bindParameter(DBAccess::BoundParameter* parm) const
{
  _dbAdapt->bindParameter(parm);
}

void
SQLQuery::fillParameterString(std::string& displayString)
{
  _parameterSubstitutor.fillParameterString(displayString);
}

void
SQLQuery::getSQLString(std::string& resultString)
{
  _parameterSubstitutor.getSQLString(*this, resultString);
}
}
