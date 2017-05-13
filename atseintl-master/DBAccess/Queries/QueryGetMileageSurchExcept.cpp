//----------------------------------------------------------------------------
//  File:           QueryGetMileageSurchExcept.cpp
//  Description:    QueryGetMileageSurchExcept
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMileageSurchExcept.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMileageSurchExceptSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOneMileageSurchExceptBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneMileageSurchExceptBase"));
std::string QueryGetOneMileageSurchExceptBase::_baseSQL;
bool QueryGetOneMileageSurchExceptBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneMileageSurchExceptBase> g_GetOneMileageSurchExceptBase;

log4cxx::LoggerPtr
QueryGetMlgSurchPsgTypes::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetOneMileageSurchExceptBase.GetMlgSurchPsgTypes"));
std::string QueryGetMlgSurchPsgTypes::_baseSQL;
bool QueryGetMlgSurchPsgTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMlgSurchPsgTypes> g_GetMlgSurchPsgTypes;

const char*
QueryGetOneMileageSurchExceptBase::getQueryName() const
{
  return "GETONEMILEAGESURCHEXCEPTBASE";
}

void
QueryGetOneMileageSurchExceptBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneMileageSurchExceptBaseSQLStatement<QueryGetOneMileageSurchExceptBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONEMILEAGESURCHEXCEPTBASE");

    substTableDef(&_baseSQL);

    QueryGetMlgSurchPsgTypes::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneMileageSurchExceptBase::findMileageSurchExcept(
    std::vector<tse::MileageSurchExcept*>& lstMSE,
    VendorCode& vendor,
    int textTblItemNo,
    CarrierCode& governingCarrier,
    TariffNumber ruleTariff,
    RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ttiStr[20];
  sprintf(ttiStr, "%d", textTblItemNo);
  char rtStr[20];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(ttiStr, 2);
  substParm(governingCarrier, 3);
  substParm(rtStr, 4);
  substParm(rule, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MileageSurchExcept* mse = nullptr;
  tse::MileageSurchExcept* msePrev = nullptr;
  while ((row = res.nextRow()))
  {
    mse = QueryGetOneMileageSurchExceptBaseSQLStatement<
        QueryGetOneMileageSurchExceptBase>::mapRowToMileageSurchExceptBase(row, msePrev);
    if (mse != msePrev)
      lstMSE.push_back(mse);

    msePrev = mse;
  }
  LOG4CXX_INFO(_logger,
               "GETONEMILEAGESURCHEXCEPTBASE: NumRows = " << res.numRows()
                                                          << " Time = " << stopTimer() << " mSecs");
  res.freeResult();

  // Get PsgTypes
  QueryGetMlgSurchPsgTypes SQLMlgSurchPsgTypes(_dbAdapt);

  std::vector<tse::MileageSurchExcept*>::iterator MSEIt;
  for (MSEIt = lstMSE.begin(); MSEIt != lstMSE.end(); MSEIt++)
  {
    SQLMlgSurchPsgTypes.getPsgTypes(*MSEIt);
  } // for (Iteration)
} // findMileageSurchExcept()

///////////////////////////////////////////////////////////
//  QueryGetOneMileageSurchExceptBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOneMileageSurchExceptBaseHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetOneMileageSurchExceptBaseHistorical"));
std::string QueryGetOneMileageSurchExceptBaseHistorical::_baseSQL;
bool QueryGetOneMileageSurchExceptBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneMileageSurchExceptBaseHistorical>
g_GetOneMileageSurchExceptBaseHistorical;

const char*
QueryGetOneMileageSurchExceptBaseHistorical::getQueryName() const
{
  return "GETONEMILEAGESURCHEXCEPTBASEHISTORICAL";
}

void
QueryGetOneMileageSurchExceptBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneMileageSurchExceptBaseHistoricalSQLStatement<
        QueryGetOneMileageSurchExceptBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONEMILEAGESURCHEXCEPTBASEHISTORICAL");
    substTableDef(&_baseSQL);
    QueryGetMlgSurchPsgTypes::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneMileageSurchExceptBaseHistorical::findMileageSurchExcept(
    std::vector<tse::MileageSurchExcept*>& lstMSE,
    VendorCode& vendor,
    int textTblItemNo,
    CarrierCode& governingCarrier,
    TariffNumber ruleTariff,
    RuleNumber& rule,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ttiStr[20];
  sprintf(ttiStr, "%d", textTblItemNo);
  char rtStr[20];
  sprintf(rtStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(ttiStr, 2);
  substParm(governingCarrier, 3);
  substParm(rtStr, 4);
  substParm(rule, 5);
  substParm(6, startDate);
  substParm(7, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MileageSurchExcept* mse = nullptr;
  tse::MileageSurchExcept* msePrev = nullptr;
  while ((row = res.nextRow()))
  {
    mse = QueryGetOneMileageSurchExceptBaseHistoricalSQLStatement<
        QueryGetOneMileageSurchExceptBaseHistorical>::mapRowToMileageSurchExceptBase(row, msePrev);
    if (mse != msePrev)
      lstMSE.push_back(mse);

    msePrev = mse;
  }
  LOG4CXX_INFO(_logger,
               "GETONEMILEAGESURCHEXCEPTBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                                    << stopTimer() << " mSecs");
  res.freeResult();

  // Get PsgTypes
  QueryGetMlgSurchPsgTypes SQLMlgSurchPsgTypes(_dbAdapt);

  std::vector<tse::MileageSurchExcept*>::iterator MSEIt;
  for (MSEIt = lstMSE.begin(); MSEIt != lstMSE.end(); MSEIt++)
  {
    SQLMlgSurchPsgTypes.getPsgTypes(*MSEIt);
  } // for (Iteration)
} // findMileageSurchExcept()

///////////////////////////////////////////////////////////
//  QueryGetAllMileageSurchExceptBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllMileageSurchExceptBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllMileageSurchExceptBase"));
std::string QueryGetAllMileageSurchExceptBase::_baseSQL;
bool QueryGetAllMileageSurchExceptBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllMileageSurchExceptBase> g_GetAllMileageSurchExceptBase;

const char*
QueryGetAllMileageSurchExceptBase::getQueryName() const
{
  return "GETALLMILEAGESURCHEXCEPTBASE";
}

void
QueryGetAllMileageSurchExceptBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllMileageSurchExceptBaseSQLStatement<QueryGetAllMileageSurchExceptBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLMILEAGESURCHEXCEPTBASE");

    substTableDef(&_baseSQL);
    QueryGetMlgSurchPsgTypes::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllMileageSurchExceptBase::findAllMileageSurchExcept(
    std::vector<tse::MileageSurchExcept*>& lstMSE)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MileageSurchExcept* mse = nullptr;
  tse::MileageSurchExcept* msePrev = nullptr;
  while ((row = res.nextRow()))
  {
    mse = QueryGetAllMileageSurchExceptBaseSQLStatement<
        QueryGetAllMileageSurchExceptBase>::mapRowToMileageSurchExceptBase(row, msePrev);
    if (mse != msePrev)
      lstMSE.push_back(mse);

    msePrev = mse;
  }
  LOG4CXX_INFO(_logger,
               "GETALLMILEAGESURCHEXCEPTBASE: NumRows = " << res.numRows()
                                                          << " Time = " << stopTimer() << " mSecs");
  res.freeResult();

  // Get PsgTypes
  QueryGetMlgSurchPsgTypes SQLMlgSurchPsgTypes(_dbAdapt);

  std::vector<tse::MileageSurchExcept*>::iterator MSEIt;
  for (MSEIt = lstMSE.begin(); MSEIt != lstMSE.end(); MSEIt++)
  {
    SQLMlgSurchPsgTypes.getPsgTypes(*MSEIt);
  } // for (Iteration)
} // findAllMileageSurchExcept()

///////////////////////////////////////////////////////////
//  QueryGetMlgSurchPsgTypes
///////////////////////////////////////////////////////////
const char*
QueryGetMlgSurchPsgTypes::getQueryName() const
{
  return "GETMLGSURCHPSGTYPES";
}

void
QueryGetMlgSurchPsgTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMlgSurchPsgTypesSQLStatement<QueryGetMlgSurchPsgTypes> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMLGSURCHPSGTYPES");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMlgSurchPsgTypes::getPsgTypes(MileageSurchExcept* a_pMileageSurchExcept)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ttiStr[15];
  sprintf(ttiStr, "%d", a_pMileageSurchExcept->textTblItemNo());
  char rtStr[15];
  sprintf(rtStr, "%d", a_pMileageSurchExcept->ruleTariff());

  resetSQL();

  substParm(a_pMileageSurchExcept->vendor(), 1);
  substParm(ttiStr, 2);
  substParm(a_pMileageSurchExcept->governingCarrier(), 3);
  substParm(rtStr, 4);
  substParm(a_pMileageSurchExcept->rule(), 5);
  substParm(6, a_pMileageSurchExcept->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMileageSurchExcept->paxTypes().push_back(
        QueryGetMlgSurchPsgTypesSQLStatement<QueryGetMlgSurchPsgTypes>::mapRowToPsgType(row));
  } // while (Fetchin Psg Types)
  LOG4CXX_INFO(_logger,
               "GETMLGSURCHPSGTYPES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // getPsgTypes()

///////////////////////////////////////////////////////////
//  QueryCxrInMlgSurchExcpt
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCxrInMlgSurchExcpt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CxrInMlgSurchExcpt"));
std::string QueryCxrInMlgSurchExcpt::_baseSQL;
bool QueryCxrInMlgSurchExcpt::_isInitialized = false;
SQLQueryInitializerHelper<QueryCxrInMlgSurchExcpt> g_CxrInMlgSurchExcpt;

const char*
QueryCxrInMlgSurchExcpt::getQueryName() const
{
  return "CXRINMLGSURCHEXCPT";
}

void
QueryCxrInMlgSurchExcpt::initialize()
{
  if (!_isInitialized)
  {
    QueryCxrInMlgSurchExcptSQLStatement<QueryCxrInMlgSurchExcpt> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CXRINMLGSURCHEXCPT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

///////////////////////////////////////////////////////////
//  QueryCxrInMlgSurchExcptHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryCxrInMlgSurchExcptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.CxrInMlgSurchExcptHistorical"));
std::string QueryCxrInMlgSurchExcptHistorical::_baseSQL;
bool QueryCxrInMlgSurchExcptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryCxrInMlgSurchExcptHistorical> g_CxrInMlgSurchExcptHistorical;

const char*
QueryCxrInMlgSurchExcptHistorical::getQueryName() const
{
  return "CXRINMLGSURCHEXCPTHISTORICAL";
}

void
QueryCxrInMlgSurchExcptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryCxrInMlgSurchExcptHistoricalSQLStatement<QueryCxrInMlgSurchExcptHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("CXRINMLGSURCHEXCPTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

}
