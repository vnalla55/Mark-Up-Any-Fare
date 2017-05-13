//----------------------------------------------------------------------------
//  File:           QueryGetSectorSurcharge.cpp
//  Description:    QueryGetSectorSurcharge
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

#include "DBAccess/Queries/QueryGetSectorSurcharge.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSectorSurchargeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSectorSurchargeBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSectorSurchargeBase"));
std::string QueryGetSectorSurchargeBase::_baseSQL;
bool QueryGetSectorSurchargeBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSectorSurchargeBase> g_GetSectorSurchargeBase;

log4cxx::LoggerPtr
QueryGetSectSurchTktgCxrs::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSectorSurchargeBase.GetSectSurchTktgCxrs"));
std::string QueryGetSectSurchTktgCxrs::_baseSQL;
bool QueryGetSectSurchTktgCxrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSectSurchTktgCxrs> g_GetSectSurchTktgCxrs;

const char*
QueryGetSectorSurchargeBase::getQueryName() const
{
  return "GETSECTORSURCHARGEBASE";
};

void
QueryGetSectorSurchargeBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSectorSurchargeBaseSQLStatement<QueryGetSectorSurchargeBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSECTORSURCHARGEBASE");
    substTableDef(&_baseSQL);

    QueryGetSectSurchTktgCxrs::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSectorSurchargeBase::findSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS,
                                                 const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SectorSurcharge* ss = nullptr;
  tse::SectorSurcharge* ssPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ss = QueryGetSectorSurchargeBaseSQLStatement<
        QueryGetSectorSurchargeBase>::mapRowToSectorSurchBase(row, ssPrev);
    if (ss != ssPrev)
      lstSS.push_back(ss);

    ssPrev = ss;
  }
  LOG4CXX_INFO(_logger,
               "GETSECTORSURCHARGEBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();

  findSectorSurchargeChildren(lstSS);
} // QueryGetSectorSurchargeBase::findSectorSurcharge()

void
QueryGetSectorSurchargeBase::findSectorSurchargeChildren(std::vector<tse::SectorSurcharge*>& lstSS)
{
  QueryGetSectSurchTktgCxrs SQLSectSurchTktgCxrs(_dbAdapt);

  std::vector<tse::SectorSurcharge*>::iterator SectSurchIt;
  for (SectSurchIt = lstSS.begin(); SectSurchIt != lstSS.end(); SectSurchIt++)
  { // Get Children
    SQLSectSurchTktgCxrs.getTktgCxrs(*SectSurchIt);
  } // for (Iteration thru Parents)
} // QueryGetSectorSurchargeBase::findSectorSurchargeChildren()

///////////////////////////////////////////////////////////
//
//  QueryGetSectorSurchargeBaseHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSectorSurchargeBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSectorSurchargeBaseHistorical"));
std::string QueryGetSectorSurchargeBaseHistorical::_baseSQL;
bool QueryGetSectorSurchargeBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSectorSurchargeBaseHistorical> g_GetSectorSurchargeBaseHistorical;

const char*
QueryGetSectorSurchargeBaseHistorical::getQueryName() const
{
  return "GETSECTORSURCHARGEBASEHISTORICAL";
};

void
QueryGetSectorSurchargeBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSectorSurchargeBaseHistoricalSQLStatement<QueryGetSectorSurchargeBaseHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSECTORSURCHARGEBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetSectSurchTktgCxrs::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSectorSurchargeBaseHistorical::findSectorSurcharge(
    std::vector<tse::SectorSurcharge*>& lstSS, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SectorSurcharge* ss = nullptr;
  tse::SectorSurcharge* ssPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ss = QueryGetSectorSurchargeBaseHistoricalSQLStatement<
        QueryGetSectorSurchargeBaseHistorical>::mapRowToSectorSurchBase(row, ssPrev);
    if (ss != ssPrev)
      lstSS.push_back(ss);

    ssPrev = ss;
  }
  LOG4CXX_INFO(_logger,
               "GETSECTORSURCHARGEBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  findSectorSurchargeChildren(lstSS);
} // QueryGetSectorSurchargeBase::findSectorSurcharge()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSectorSurchargeBase
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSectorSurchargeBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSectorSurchargeBase"));
std::string QueryGetAllSectorSurchargeBase::_baseSQL;
bool QueryGetAllSectorSurchargeBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSectorSurchargeBase> g_GetAllSectorSurchargeBase;

const char*
QueryGetAllSectorSurchargeBase::getQueryName() const
{
  return "GETALLSECTORSURCHARGEBASE";
};

void
QueryGetAllSectorSurchargeBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSectorSurchargeBaseSQLStatement<QueryGetAllSectorSurchargeBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSECTORSURCHARGEBASE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSectorSurchargeBase::findAllSectorSurcharge(std::vector<tse::SectorSurcharge*>& lstSS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SectorSurcharge* ss = nullptr;
  tse::SectorSurcharge* ssPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ss = QueryGetAllSectorSurchargeBaseSQLStatement<
        QueryGetAllSectorSurchargeBase>::mapRowToSectorSurchBase(row, ssPrev);
    if (ss != ssPrev)
      lstSS.push_back(ss);

    ssPrev = ss;
  }
  LOG4CXX_INFO(_logger,
               "GETALLSECTORSURCHARGEBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();

  findSectorSurchargeChildren(lstSS);
} // findAllSectorSurcharge()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSectorSurchargeBaseHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSectorSurchargeBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSectorSurchargeBaseHistorical"));
std::string QueryGetAllSectorSurchargeBaseHistorical::_baseSQL;
bool QueryGetAllSectorSurchargeBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSectorSurchargeBaseHistorical>
g_GetAllSectorSurchargeBaseHistorical;

const char*
QueryGetAllSectorSurchargeBaseHistorical::getQueryName() const
{
  return "GETALLSECTORSURCHARGEBASEHISTORICAL";
};

void
QueryGetAllSectorSurchargeBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSectorSurchargeBaseHistoricalSQLStatement<QueryGetAllSectorSurchargeBaseHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSECTORSURCHARGEBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSectorSurchargeBaseHistorical::findAllSectorSurcharge(
    std::vector<tse::SectorSurcharge*>& lstSS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SectorSurcharge* ss = nullptr;
  tse::SectorSurcharge* ssPrev = nullptr;
  while ((row = res.nextRow()))
  {
    ss = QueryGetAllSectorSurchargeBaseHistoricalSQLStatement<
        QueryGetAllSectorSurchargeBaseHistorical>::mapRowToSectorSurchBase(row, ssPrev);
    if (ss != ssPrev)
      lstSS.push_back(ss);

    ssPrev = ss;
  }
  LOG4CXX_INFO(_logger,
               "GETALLSECTORSURCHARGEBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  findSectorSurchargeChildren(lstSS);
} // findAllSectorSurcharge()

///////////////////////////////////////////////////////////
//
//  QueryGetSectSurchTktgCxrs
//
///////////////////////////////////////////////////////////
const char*
QueryGetSectSurchTktgCxrs::getQueryName() const
{
  return "GETSECTSURCHTKTGCXRS";
}

void
QueryGetSectSurchTktgCxrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSectSurchTktgCxrsSQLStatement<QueryGetSectSurchTktgCxrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSECTSURCHTKTGCXRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSectSurchTktgCxrs::getTktgCxrs(tse::SectorSurcharge* a_pSectorSurcharge)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char snStr[15];
  sprintf(snStr, "%d", a_pSectorSurcharge->seqNo());

  resetSQL();

  substParm(a_pSectorSurcharge->carrier(), 1);
  substParm(2, a_pSectorSurcharge->versionDate());
  substParm(snStr, 3);
  substParm(4, a_pSectorSurcharge->createDate());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSectorSurcharge->tktgCxrs().push_back(
        QueryGetSectSurchTktgCxrsSQLStatement<QueryGetSectSurchTktgCxrs>::mapRowToCarrier(row));
  } // while (Fetchin Tktg Carrier Excepts)
  res.freeResult();

} // QueryGetSectSurchTktgCxrs::getTktgCxrs()
}
