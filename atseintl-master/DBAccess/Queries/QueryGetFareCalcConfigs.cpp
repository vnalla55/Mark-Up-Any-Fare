//----------------------------------------------------------------------------
//  File:           QueryGetFareCalcConfigs.cpp
//  Description:    QueryGetFareCalcConfigs
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetFareCalcConfigs.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareCalcConfigsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareCalcConfigs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareCalcConfigs"));
std::string QueryGetFareCalcConfigs::_baseSQL;
bool QueryGetFareCalcConfigs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareCalcConfigs> g_GetFareCalcConfigs;

const char*
QueryGetFareCalcConfigs::getQueryName() const
{
  return "GETFARECALCCONFIGS";
}

void
QueryGetFareCalcConfigs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareCalcConfigsSQLStatement<QueryGetFareCalcConfigs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARECALCCONFIGS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareCalcConfigs::findFareCalcConfigs(std::vector<tse::FareCalcConfig*>& fcCfgs,
                                             const Indicator userApplType,
                                             const UserApplCode& userAppl,
                                             const PseudoCityCode& pseudoCity)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, pseudoCity);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareCalcConfig* fcCfg = nullptr;
  tse::FareCalcConfig* fcCfgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    fcCfg = QueryGetFareCalcConfigsSQLStatement<QueryGetFareCalcConfigs>::mapRowToFareCalcConfig(
        row, fcCfgPrev);
    if (fcCfg != fcCfgPrev)
    {
      fcCfgs.push_back(fcCfg);
    }
    fcCfgPrev = fcCfg;
  }
  LOG4CXX_INFO(_logger,
               "GETFARECALCCONFIGS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareCalcConfigs()

int
QueryGetFareCalcConfigs::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger,
                  "stringToInteger - Null pointer to int data. LineNumber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // stringToInteger()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareCalcConfigs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareCalcConfigs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareCalcConfigs"));
std::string QueryGetAllFareCalcConfigs::_baseSQL;
bool QueryGetAllFareCalcConfigs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareCalcConfigs> g_GetAllFareCalcConfigs;

const char*
QueryGetAllFareCalcConfigs::getQueryName() const
{
  return "GETALLFARECALCCONFIGS";
}

void
QueryGetAllFareCalcConfigs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareCalcConfigsSQLStatement<QueryGetAllFareCalcConfigs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARECALCCONFIGS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareCalcConfigs::findAllFareCalcConfigs(std::vector<tse::FareCalcConfig*>& fcCfgs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareCalcConfig* fcCfg = nullptr;
  tse::FareCalcConfig* fcCfgPrev = nullptr;
  while ((row = res.nextRow()))
  {
    fcCfg =
        QueryGetAllFareCalcConfigsSQLStatement<QueryGetAllFareCalcConfigs>::mapRowToFareCalcConfig(
            row, fcCfgPrev);
    if (fcCfg != fcCfgPrev)
    {
      fcCfgs.push_back(fcCfg);
    }
    fcCfgPrev = fcCfg;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFARECALCCONFIGS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareCalcConfigs()
}
