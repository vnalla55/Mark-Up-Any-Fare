//----------------------------------------------------------------------------
//  File:           QueryGetPfcEquipTypeExempt.cpp
//  Description:    QueryGetPfcEquipTypeExempt
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

#include "DBAccess/Queries/QueryGetPfcEquipTypeExempt.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcEquipTypeExemptSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPfcEquipTypeExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPfcEquipTypeExempt"));
std::string QueryGetPfcEquipTypeExempt::_baseSQL;
bool QueryGetPfcEquipTypeExempt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcEquipTypeExempt> g_GetPfcEquipTypeExempt;

const char*
QueryGetPfcEquipTypeExempt::getQueryName() const
{
  return "GETPFCEQUIPTYPEEXEMPT";
}

void
QueryGetPfcEquipTypeExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcEquipTypeExemptSQLStatement<QueryGetPfcEquipTypeExempt> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCEQUIPTYPEEXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcEquipTypeExempt::findPfcEquipTypeExempt(std::vector<tse::PfcEquipTypeExempt*>& lstETE,
                                                   const EquipmentType& equip,
                                                   const StateCode& state)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, equip);
  substParm(2, state);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstETE.push_back(QueryGetPfcEquipTypeExemptSQLStatement<
        QueryGetPfcEquipTypeExempt>::mapRowToPfcEquipTypeExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCEQUIPTYPEEXEMPT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcEquipTypeExempt()

///////////////////////////////////////////////////////////
//  QueryGetAllPfcEquipTypeExempt
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllPfcEquipTypeExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllPfcEquipTypeExempt"));
std::string QueryGetAllPfcEquipTypeExempt::_baseSQL;
bool QueryGetAllPfcEquipTypeExempt::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllPfcEquipTypeExempt> g_GetAllPfcEquipTypeExempt;

const char*
QueryGetAllPfcEquipTypeExempt::getQueryName() const
{
  return "GETALLPFCEQUIPTYPEEXEMPT";
}

void
QueryGetAllPfcEquipTypeExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllPfcEquipTypeExemptSQLStatement<QueryGetAllPfcEquipTypeExempt> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLPFCEQUIPTYPEEXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllPfcEquipTypeExempt::findAllPfcEquipTypeExempt(
    std::vector<tse::PfcEquipTypeExempt*>& lstETE)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstETE.push_back(QueryGetAllPfcEquipTypeExemptSQLStatement<
        QueryGetAllPfcEquipTypeExempt>::mapRowToPfcEquipTypeExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLPFCEQUIPTYPEEXEMPT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllPfcEquipTypeExempt()

///////////////////////////////////////////////////////////
//  QueryGetPfcEquipTypeExemptHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPfcEquipTypeExemptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetPfcEquipTypeExemptHistorical"));
std::string QueryGetPfcEquipTypeExemptHistorical::_baseSQL;
bool QueryGetPfcEquipTypeExemptHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPfcEquipTypeExemptHistorical> g_GetPfcEquipTypeExemptHistorical;

const char*
QueryGetPfcEquipTypeExemptHistorical::getQueryName() const
{
  return "GETPFCEQUIPTYPEEXEMPTHISTORICAL";
}

void
QueryGetPfcEquipTypeExemptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPfcEquipTypeExemptHistoricalSQLStatement<QueryGetPfcEquipTypeExemptHistorical>
    sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPFCEQUIPTYPEEXEMPTHISTORICAL");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetPfcEquipTypeExemptHistorical::findPfcEquipTypeExempt(
    std::vector<tse::PfcEquipTypeExempt*>& lstETE,
    const EquipmentType& equip,
    const StateCode& state,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, equip);
  substParm(2, state);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstETE.push_back(QueryGetPfcEquipTypeExemptHistoricalSQLStatement<
        QueryGetPfcEquipTypeExemptHistorical>::mapRowToPfcEquipTypeExempt(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPFCEQUIPTYPEEXEMPTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findPfcEquipTypeExempt()
}
