//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNvbNva.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNvbNvaSQLStatement.h"

namespace tse
{

///////////////////////////////////////////////////////////
//  QueryGetNvbNva
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNvbNva::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNvbNva"));
std::string QueryGetNvbNva::_baseSQL;
bool QueryGetNvbNva::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNvbNva> g_GetNvbNva;

const char*
QueryGetNvbNva::getQueryName() const
{
  return "GETNVBNVA";
}

void
QueryGetNvbNva::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNvbNvaSQLStatement<QueryGetNvbNva> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNVBNVA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetNvbNva::findNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const RuleNumber& rule)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, ruleTariff);
  substParm(rule, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  NvbNvaInfo* info = nullptr;
  NvbNvaInfo* infoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    info = QueryGetNvbNvaSQLStatement<QueryGetNvbNva>::mapRowToNvbNvaInfo(row, infoPrev);
    if (info != infoPrev)
    {
      vNvbNvaInfo.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETNVBNVA: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                     << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetNvbNvaHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNvbNvaHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNvbNvaHistorical"));
std::string QueryGetNvbNvaHistorical::_baseSQL;
bool QueryGetNvbNvaHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNvbNvaHistorical> g_GetNvbNvaHistorical;

const char*
QueryGetNvbNvaHistorical::getQueryName() const
{
  return "GETNVBNVAHISTORICAL";
}

void
QueryGetNvbNvaHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNvbNvaHistoricalSQLStatement<QueryGetNvbNvaHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNVBNVAHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetNvbNvaHistorical::findNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(3, ruleTariff);
  substParm(rule, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(vendor, 7);
  substParm(carrier, 8);
  substParm(9, ruleTariff);
  substParm(rule, 10);
  substParm(11, startDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  NvbNvaInfo* info = nullptr;
  NvbNvaInfo* infoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    info = QueryGetNvbNvaHistoricalSQLStatement<QueryGetNvbNvaHistorical>::mapRowToNvbNvaInfo(
        row, infoPrev);
    if (info != infoPrev)
    {
      vNvbNvaInfo.push_back(info);
    }
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETNVBNVAHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetAllNvbNva
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNvbNva::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllNvbNva"));
std::string QueryGetAllNvbNva::_baseSQL;
bool QueryGetAllNvbNva::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNvbNva> g_GetAllNvbNva;

const char*
QueryGetAllNvbNva::getQueryName() const
{
  return "GETALLNVBNVA";
}

void
QueryGetAllNvbNva::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNvbNvaSQLStatement<QueryGetAllNvbNva> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNVBNVA");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllNvbNva::findAllNvbNvaInfo(std::vector<NvbNvaInfo*>& vNvbNvaInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  NvbNvaInfo* info = nullptr;
  NvbNvaInfo* infoPrev = nullptr;
  while ((row = res.nextRow()))
  {
    info = QueryGetAllNvbNvaSQLStatement<QueryGetAllNvbNva>::mapRowToNvbNvaInfo(row, infoPrev);
    if (info != infoPrev)
      vNvbNvaInfo.push_back(info);
    infoPrev = info;
  }
  LOG4CXX_INFO(_logger,
               "GETALLNVBNVA: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllNvbNva()

} // namespace tse
