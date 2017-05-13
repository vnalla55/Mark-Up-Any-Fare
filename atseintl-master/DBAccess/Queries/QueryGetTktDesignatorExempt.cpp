//----------------------------------------------------------------------------
//  File:           QueryGetTktDesignatorExempt.cpp
//  Description:    QueryGetTktDesignatorExempt
//  Created:        12/12/2013
//
// Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is the
// confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetTktDesignatorExempt.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTktDesignatorExemptSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetTktDesignatorExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTktDesignatorExempt"));
std::string QueryGetTktDesignatorExempt::_baseSQL;
bool
QueryGetTktDesignatorExempt::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetTktDesignatorExempt> _getTktDesignatorExempt;

const char*
QueryGetTktDesignatorExempt::getQueryName() const
{
  return "GETTKTDESIGNATOREXEMPT";
}

void
QueryGetTktDesignatorExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTktDesignatorExemptSQLStatement<QueryGetTktDesignatorExempt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTKTDESIGNATOREXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTktDesignatorExempt::findTktDesignatorExempt(std::vector<TktDesignatorExemptInfo*>& lst,
                                                     const CarrierCode& carrier)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);
  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TktDesignatorExemptInfo* info(nullptr);
  TktDesignatorExemptInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info =
        QueryGetTktDesignatorExemptSQLStatement<QueryGetTktDesignatorExempt>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETTKTDESIGNATOREXEMPT: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetTktDesignatorExemptHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTktDesignatorExemptHistorical"));
std::string QueryGetTktDesignatorExemptHistorical::_baseSQL;
bool
QueryGetTktDesignatorExemptHistorical::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetTktDesignatorExemptHistorical> _getTktDesignatorExemptHistorical;

const char*
QueryGetTktDesignatorExemptHistorical::getQueryName() const
{
  return "GETTKTDESIGNATOREXEMPTHISTORICAL";
}

void
QueryGetTktDesignatorExemptHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTktDesignatorExemptHistoricalSQLStatement<QueryGetTktDesignatorExemptHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTKTDESIGNATOREXEMPTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTktDesignatorExemptHistorical::findTktDesignatorExempt(
    std::vector<TktDesignatorExemptInfo*>& lst,
    const CarrierCode& carrier,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  substParm(carrier, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TktDesignatorExemptInfo* info(nullptr);
  TktDesignatorExemptInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetTktDesignatorExemptHistoricalSQLStatement<
        QueryGetTktDesignatorExemptHistorical>::mapRow(row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETTKTDESIGNATOREXEMPTHISTORICAL: NumRows: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetAllTktDesignatorExempt::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTktDesignatorExempt"));
std::string QueryGetAllTktDesignatorExempt::_baseSQL;
bool
QueryGetAllTktDesignatorExempt::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetAllTktDesignatorExempt> _getAllTktDesignatorExempt;

const char*
QueryGetAllTktDesignatorExempt::getQueryName() const
{
  return "GETALLTKTDESIGNATOREXEMPT";
}

void
QueryGetAllTktDesignatorExempt::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTktDesignatorExemptSQLStatement<QueryGetAllTktDesignatorExempt> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTKTDESIGNATOREXEMPT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllTktDesignatorExempt::findAllTktDesignatorExempt(
    std::vector<TktDesignatorExemptInfo*>& lst)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  TktDesignatorExemptInfo* info(nullptr);
  TktDesignatorExemptInfo* infoPrev(nullptr);

  while ((row = res.nextRow()))
  {
    info = QueryGetAllTktDesignatorExemptSQLStatement<QueryGetAllTktDesignatorExempt>::mapRow(
        row, infoPrev);
    if (info != infoPrev)
    {
      lst.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLTKTDESIGNATOREXEMPT: NumRows: " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") ms");
  res.freeResult();
}
} // tse
