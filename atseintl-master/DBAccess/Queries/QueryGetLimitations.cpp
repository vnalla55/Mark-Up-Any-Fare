//----------------------------------------------------------------------------
//  File:           QueryGetLimitations.cpp
//  Description:    QueryGetLimitations
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
#include "DBAccess/Queries/QueryGetLimitations.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetLimitationsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetLimitJrnyBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitJrnyBase"));
std::string QueryGetLimitJrnyBase::_baseSQL;
bool QueryGetLimitJrnyBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitJrnyBase> g_GetLimitJrnyBase;

log4cxx::LoggerPtr
QueryGetLimitJrnyTxtMsg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitJrnyBase.GetLimitJrnyTxtMsg"));
std::string QueryGetLimitJrnyTxtMsg::_baseSQL;
bool QueryGetLimitJrnyTxtMsg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitJrnyTxtMsg> g_GetLimitJrnyTxtMsg;

const char*
QueryGetLimitJrnyBase::getQueryName() const
{
  return "GETLIMITJRNYBASE";
}

void
QueryGetLimitJrnyBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitJrnyBaseSQLStatement<QueryGetLimitJrnyBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITJRNYBASE");

    substTableDef(&_baseSQL);

    QueryGetLimitJrnyTxtMsg::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitJrnyBase::findLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys,
                                          const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationJrny* limJrny = nullptr;
  tse::LimitationJrny* limJrnyPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limJrny = QueryGetLimitJrnyBaseSQLStatement<QueryGetLimitJrnyBase>::mapRowToLimitJrnyBase(
        row, limJrnyPrev);
    if (limJrny != limJrnyPrev)
      limJrnys.push_back(limJrny);

    limJrnyPrev = limJrny;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITJRNYBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  getLimitationText(limJrnys);
} // QueryGetLimitJrnyBase::findLimitationJrny()

int
QueryGetLimitJrnyBase::charToInt(const char* s)
{
  if (s == nullptr || *s < '0' || *s > '9')
  {
    return -1;
  }
  return *s - '0';
} // QueryGetLimitJrnyBase::charToInt()

void
QueryGetLimitJrnyBase::getLimitationText(std::vector<tse::LimitationJrny*>& limJrnys)
{
  QueryGetLimitJrnyTxtMsg SQLLimitJrnyTxtMsg(_dbAdapt);

  std::vector<tse::LimitationJrny*>::iterator LimJnyIt;
  for (LimJnyIt = limJrnys.begin(); LimJnyIt != limJrnys.end(); LimJnyIt++)
  { // Get Children
    SQLLimitJrnyTxtMsg.getText(*LimJnyIt);
  } // for (Iteration thru Parents)
} // QueryGetLimitJrnyBase::getLimitationText()

///////////////////////////////////////////////////////////
//  QueryGetLimitJrnyBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLimitJrnyBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitJrnyBaseHistorical"));
std::string QueryGetLimitJrnyBaseHistorical::_baseSQL;
bool QueryGetLimitJrnyBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitJrnyBaseHistorical> g_GetLimitJrnyBaseHistorical;

const char*
QueryGetLimitJrnyBaseHistorical::getQueryName() const
{
  return "GETLIMITJRNYBASEHISTORICAL";
}

void
QueryGetLimitJrnyBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitJrnyBaseHistoricalSQLStatement<QueryGetLimitJrnyBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITJRNYBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitJrnyBaseHistorical::findLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys,
                                                    const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationJrny* limJrny = nullptr;
  tse::LimitationJrny* limJrnyPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limJrny = QueryGetLimitJrnyBaseHistoricalSQLStatement<
        QueryGetLimitJrnyBaseHistorical>::mapRowToLimitJrnyBase(row, limJrnyPrev);
    if (limJrny != limJrnyPrev)
      limJrnys.push_back(limJrny);

    limJrnyPrev = limJrny;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITJRNYBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationText(limJrnys);
} // QueryGetLimitJrnyBase::findLimitationJrny()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitJrnyBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitJrnyBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitJrnyBase"));
std::string QueryGetAllLimitJrnyBase::_baseSQL;
bool QueryGetAllLimitJrnyBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitJrnyBase> g_GetAllLimitJrnyBase;

const char*
QueryGetAllLimitJrnyBase::getQueryName() const
{
  return "GETALLLIMITJRNYBASE";
}

void
QueryGetAllLimitJrnyBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitJrnyBaseSQLStatement<QueryGetAllLimitJrnyBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITJRNYBASE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitJrnyBase::findAllLimitationJrny(std::vector<tse::LimitationJrny*>& limJrnys)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationJrny* limJrny = nullptr;
  tse::LimitationJrny* limJrnyPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limJrny = QueryGetAllLimitJrnyBaseSQLStatement<QueryGetAllLimitJrnyBase>::mapRowToLimitJrnyBase(
        row, limJrnyPrev);
    if (limJrny != limJrnyPrev)
      limJrnys.push_back(limJrny);

    limJrnyPrev = limJrny;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITJRNYBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationText(limJrnys);
} // findAllLimitationJrny()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitJrnyBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitJrnyBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitJrnyBaseHistorical"));
std::string QueryGetAllLimitJrnyBaseHistorical::_baseSQL;
bool QueryGetAllLimitJrnyBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitJrnyBaseHistorical> g_GetAllLimitJrnyBaseHistorical;

const char*
QueryGetAllLimitJrnyBaseHistorical::getQueryName() const
{
  return "GETALLLIMITJRNYBASEHISTORICAL";
}

void
QueryGetAllLimitJrnyBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitJrnyBaseHistoricalSQLStatement<QueryGetAllLimitJrnyBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITJRNYBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitJrnyBaseHistorical::findAllLimitationJrny(
    std::vector<tse::LimitationJrny*>& limJrnys)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationJrny* limJrny = nullptr;
  tse::LimitationJrny* limJrnyPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limJrny = QueryGetAllLimitJrnyBaseHistoricalSQLStatement<
        QueryGetAllLimitJrnyBaseHistorical>::mapRowToLimitJrnyBase(row, limJrnyPrev);
    if (limJrny != limJrnyPrev)
      limJrnys.push_back(limJrny);

    limJrnyPrev = limJrny;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITJRNYBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationText(limJrnys);
} // findAllLimitationJrny()

///////////////////////////////////////////////////////////
//  QueryGetLimitJrnyTxtMsg
///////////////////////////////////////////////////////////
const char*
QueryGetLimitJrnyTxtMsg::getQueryName() const
{
  return "GETLIMITJRNYTXTMSG";
}

void
QueryGetLimitJrnyTxtMsg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitJrnyTxtMsgSQLStatement<QueryGetLimitJrnyTxtMsg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITJRNYTXTMSG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitJrnyTxtMsg::getText(tse::LimitationJrny* a_pLimitationJrny)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strSeqNo[15];
  sprintf(strSeqNo, "%d", a_pLimitationJrny->seqNo());

  resetSQL();

  substParm(1, a_pLimitationJrny->versionDate());
  substParm(strSeqNo, 2);
  substParm(3, a_pLimitationJrny->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pLimitationJrny->textMsg().push_back(
        QueryGetLimitJrnyTxtMsgSQLStatement<QueryGetLimitJrnyTxtMsg>::mapRowToText(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETLIMITJRNYTXTMSG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitJrnyTxtMsg::getText()

///////////////////////////////////////////////////////////
//  QueryGetLimitationPU
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLimitationPU::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitationPU"));
std::string QueryGetLimitationPU::_baseSQL;
bool QueryGetLimitationPU::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitationPU> g_GetLimitationPU;

const char*
QueryGetLimitationPU::getQueryName() const
{
  return "GETLIMITATIONPU";
}

void
QueryGetLimitationPU::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitationPUSQLStatement<QueryGetLimitationPU> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITATIONPU");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitationPU::findLimitationPU(std::vector<tse::LimitationCmn*>& limPUs,
                                       const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationCmn* limPU = nullptr;
  tse::LimitationCmn* limPuPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limPU = QueryGetLimitationPUSQLStatement<QueryGetLimitationPU>::mapRowToLimitationPU(row,
                                                                                         limPuPrev);
    if (limPU != limPuPrev)
      limPUs.push_back(limPU);

    limPuPrev = limPU;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITATIONPU: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitationPU::findLimitationPU()

int
QueryGetLimitationPU::charToInt(const char* s)
{
  if (s == nullptr || *s < '0' || *s > '9')
  {
    return -1;
  }
  return *s - '0';
} // QueryGetLimitationPU::charToInt()

///////////////////////////////////////////////////////////
//  QueryGetLimitationPUHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLimitationPUHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitationPUHistorical"));
std::string QueryGetLimitationPUHistorical::_baseSQL;
bool QueryGetLimitationPUHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitationPUHistorical> g_GetLimitationPUHistorical;

const char*
QueryGetLimitationPUHistorical::getQueryName() const
{
  return "GETLIMITATIONPUHISTORICAL";
}

void
QueryGetLimitationPUHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitationPUHistoricalSQLStatement<QueryGetLimitationPUHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITATIONPUHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitationPUHistorical::findLimitationPU(std::vector<tse::LimitationCmn*>& limPUs,
                                                 const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationCmn* limPU = nullptr;
  tse::LimitationCmn* limPuPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limPU = QueryGetLimitationPUHistoricalSQLStatement<
        QueryGetLimitationPUHistorical>::mapRowToLimitationPU(row, limPuPrev);
    if (limPU != limPuPrev)
      limPUs.push_back(limPU);

    limPuPrev = limPU;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITATIONPUHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitationPUHistorical::findLimitationPU()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitationPU
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitationPU::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitationPU"));
std::string QueryGetAllLimitationPU::_baseSQL;
bool QueryGetAllLimitationPU::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitationPU> g_GetAllLimitationPU;

const char*
QueryGetAllLimitationPU::getQueryName() const
{
  return "GETALLLIMITATIONPU";
}

void
QueryGetAllLimitationPU::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitationPUSQLStatement<QueryGetAllLimitationPU> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITATIONPU");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitationPU::findAllLimitationPU(std::vector<tse::LimitationCmn*>& limPUs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationCmn* limPU = nullptr;
  tse::LimitationCmn* limPuPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limPU = QueryGetAllLimitationPUSQLStatement<QueryGetAllLimitationPU>::mapRowToLimitationPU(
        row, limPuPrev);
    if (limPU != limPuPrev)
      limPUs.push_back(limPU);

    limPuPrev = limPU;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITATIONPU: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllLimitationPU::findAllLimitationPU()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitationPUHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitationPUHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitationPUHistorical"));
std::string QueryGetAllLimitationPUHistorical::_baseSQL;
bool QueryGetAllLimitationPUHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitationPUHistorical> g_GetAllLimitationPUHistorical;

const char*
QueryGetAllLimitationPUHistorical::getQueryName() const
{
  return "GETALLLIMITATIONPUHISTORICAL";
}

void
QueryGetAllLimitationPUHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitationPUHistoricalSQLStatement<QueryGetAllLimitationPUHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITATIONPUHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitationPUHistorical::findAllLimitationPU(std::vector<tse::LimitationCmn*>& limPUs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationCmn* limPU = nullptr;
  tse::LimitationCmn* limPuPrev = nullptr;
  while ((row = res.nextRow()))
  {
    limPU = QueryGetAllLimitationPUHistoricalSQLStatement<
        QueryGetAllLimitationPUHistorical>::mapRowToLimitationPU(row, limPuPrev);
    if (limPU != limPuPrev)
      limPUs.push_back(limPU);

    limPuPrev = limPU;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITATIONPUHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllLimitationPUHistorical::findAllLimitationPU()

///////////////////////////////////////////////////////////
//  QueryGetLimitFareBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLimitFareBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitFareBase"));
std::string QueryGetLimitFareBase::_baseSQL;
bool QueryGetLimitFareBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitFareBase> g_GetLimitFareBase;

log4cxx::LoggerPtr
QueryGetLimitFareGovCxrs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitFareBase.GetLimitFareGovCxrs"));
std::string QueryGetLimitFareGovCxrs::_baseSQL;
bool QueryGetLimitFareGovCxrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitFareGovCxrs> g_GetLimitFareGovCxrs;

log4cxx::LoggerPtr
QueryGetLimitFareCxrLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitFareBase.GetLimitFareCxrLocs"));
std::string QueryGetLimitFareCxrLocs::_baseSQL;
bool QueryGetLimitFareCxrLocs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitFareCxrLocs> g_GetLimitFareCxrLocs;

log4cxx::LoggerPtr
QueryGetLimitFareRoutings::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitFareBase.GetLimitFareRoutings"));
std::string QueryGetLimitFareRoutings::_baseSQL;
bool QueryGetLimitFareRoutings::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitFareRoutings> g_GetLimitFareRoutings;

const char*
QueryGetLimitFareBase::getQueryName() const
{
  return "GETLIMITFAREBASE";
}

void
QueryGetLimitFareBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitFareBaseSQLStatement<QueryGetLimitFareBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITFAREBASE");

    substTableDef(&_baseSQL);

    QueryGetLimitFareGovCxrs::initialize();
    QueryGetLimitFareCxrLocs::initialize();
    QueryGetLimitFareRoutings::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitFareBase::findLimitationFare(std::vector<tse::LimitationFare*>& limFares,
                                          const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationFare* limFare = nullptr;
  tse::LimitationFare* limFarePrev = nullptr;
  while ((row = res.nextRow()))
  { // Get the base records
    limFare = QueryGetLimitFareBaseSQLStatement<QueryGetLimitFareBase>::mapRowToLimitFareBase(
        row, limFarePrev);
    if (limFare != limFarePrev)
      limFares.push_back(limFare);

    limFarePrev = limFare;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITFAREBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  getLimitationCarriers(limFares);
} // QueryGetLimitFareBase::findLimitationFare()

int
QueryGetLimitFareBase::charToInt(const char* s)
{
  if (s == nullptr || *s < '0' || *s > '9')
  {
    return -1;
  }
  return *s - '0';
} // QueryGetLimitFareBase::charToInt()

void
QueryGetLimitFareBase::getLimitationCarriers(std::vector<tse::LimitationFare*>& limFares)
{
  QueryGetLimitFareGovCxrs SQLLimitFareGovCxrs(_dbAdapt);
  QueryGetLimitFareCxrLocs SQLLimitFareCxrLocs(_dbAdapt);
  QueryGetLimitFareRoutings SQLLimitFareRoutings(_dbAdapt);

  std::vector<tse::LimitationFare*>::iterator LimFareIt;
  for (LimFareIt = limFares.begin(); LimFareIt != limFares.end(); LimFareIt++)
  { // Get Children
    SQLLimitFareGovCxrs.getGovCxrs(*LimFareIt);
    SQLLimitFareCxrLocs.getCxrLocs(*LimFareIt);
    SQLLimitFareRoutings.getRoutings(*LimFareIt);
  } // for (Iteration thru Parents)
} // QueryGetLimitFareBase::getLimitationCarriers()

///////////////////////////////////////////////////////////
//  QueryGetLimitFareBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetLimitFareBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetLimitFareBaseHistorical"));
std::string QueryGetLimitFareBaseHistorical::_baseSQL;
bool QueryGetLimitFareBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetLimitFareBaseHistorical> g_GetLimitFareBaseHistorical;

const char*
QueryGetLimitFareBaseHistorical::getQueryName() const
{
  return "GETLIMITFAREBASEHISTORICAL";
}

void
QueryGetLimitFareBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitFareBaseHistoricalSQLStatement<QueryGetLimitFareBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITFAREBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitFareBaseHistorical::findLimitationFare(std::vector<tse::LimitationFare*>& limFares,
                                                    const UserApplCode& userAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userAppl);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationFare* limFare = nullptr;
  tse::LimitationFare* limFarePrev = nullptr;
  while ((row = res.nextRow()))
  { // Get the base records
    limFare = QueryGetLimitFareBaseHistoricalSQLStatement<
        QueryGetLimitFareBaseHistorical>::mapRowToLimitFareBase(row, limFarePrev);
    if (limFare != limFarePrev)
      limFares.push_back(limFare);

    limFarePrev = limFare;
  }
  LOG4CXX_INFO(_logger,
               "GETLIMITFAREBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationCarriers(limFares);
} // QueryGetLimitFareBaseHistorical::findAllLimitationFare()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitFareBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitFareBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitFareBase"));
std::string QueryGetAllLimitFareBase::_baseSQL;
bool QueryGetAllLimitFareBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitFareBase> g_GetAllLimitFareBase;

const char*
QueryGetAllLimitFareBase::getQueryName() const
{
  return "GETALLLIMITFAREBASE";
}

void
QueryGetAllLimitFareBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitFareBaseSQLStatement<QueryGetAllLimitFareBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITFAREBASE");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitFareBase::findAllLimitationFare(std::vector<tse::LimitationFare*>& limFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationFare* limFare = nullptr;
  tse::LimitationFare* limFarePrev = nullptr;
  while ((row = res.nextRow()))
  { // Get the base records
    limFare = QueryGetAllLimitFareBaseSQLStatement<QueryGetAllLimitFareBase>::mapRowToLimitFareBase(
        row, limFarePrev);
    if (limFare != limFarePrev)
      limFares.push_back(limFare);

    limFarePrev = limFare;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITFAREBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationCarriers(limFares);
} // QueryGetAllLimitFareBase::findAllLimitationFare()

///////////////////////////////////////////////////////////
//  QueryGetAllLimitFareBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllLimitFareBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllLimitFareBaseHistorical"));
std::string QueryGetAllLimitFareBaseHistorical::_baseSQL;
bool QueryGetAllLimitFareBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllLimitFareBaseHistorical> g_GetAllLimitFareBaseHistorical;

const char*
QueryGetAllLimitFareBaseHistorical::getQueryName() const
{
  return "GETALLLIMITFAREBASEHISTORICAL";
}

void
QueryGetAllLimitFareBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllLimitFareBaseHistoricalSQLStatement<QueryGetAllLimitFareBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLLIMITFAREBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllLimitFareBaseHistorical::findAllLimitationFare(
    std::vector<tse::LimitationFare*>& limFares)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::LimitationFare* limFare = nullptr;
  tse::LimitationFare* limFarePrev = nullptr;
  while ((row = res.nextRow()))
  { // Get the base records
    limFare = QueryGetAllLimitFareBaseHistoricalSQLStatement<
        QueryGetAllLimitFareBaseHistorical>::mapRowToLimitFareBase(row, limFarePrev);
    if (limFare != limFarePrev)
      limFares.push_back(limFare);

    limFarePrev = limFare;
  }
  LOG4CXX_INFO(_logger,
               "GETALLLIMITFAREBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getLimitationCarriers(limFares);
} // QueryGetAllLimitFareBase::findAllLimitationFare()

///////////////////////////////////////////////////////////
//  QueryGetLimitFareGovCxrs
///////////////////////////////////////////////////////////
const char*
QueryGetLimitFareGovCxrs::getQueryName() const
{
  return "GETLIMITFAREGOVCXRS";
}

void
QueryGetLimitFareGovCxrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitFareGovCxrsSQLStatement<QueryGetLimitFareGovCxrs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITFAREGOVCXRS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitFareGovCxrs::getGovCxrs(LimitationFare* a_pLimitationFare)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strSeqNo[15];
  sprintf(strSeqNo, "%d", a_pLimitationFare->seqNo());

  resetSQL();

  substParm(1, a_pLimitationFare->versionDate());
  substParm(strSeqNo, 2);
  substParm(3, a_pLimitationFare->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pLimitationFare->govCarriers().push_back(
        QueryGetLimitFareGovCxrsSQLStatement<QueryGetLimitFareGovCxrs>::mapRowToGovCarrier(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETLIMITFAREGOVCXRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitFareGovCxrs::getGovCxrs()

///////////////////////////////////////////////////////////
//  QueryGetLimitFareCxrLocs
///////////////////////////////////////////////////////////
const char*
QueryGetLimitFareCxrLocs::getQueryName() const
{
  return "GETLIMITFARECXRLOCS";
}

void
QueryGetLimitFareCxrLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitFareCxrLocsSQLStatement<QueryGetLimitFareCxrLocs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITFARECXRLOCS");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitFareCxrLocs::getCxrLocs(LimitationFare* a_pLimitationFare)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strSeqNo[15];
  sprintf(strSeqNo, "%d", a_pLimitationFare->seqNo());

  resetSQL();

  substParm(1, a_pLimitationFare->versionDate());
  substParm(strSeqNo, 2);
  substParm(3, a_pLimitationFare->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pLimitationFare->exceptViaCxrs().push_back(
        QueryGetLimitFareCxrLocsSQLStatement<QueryGetLimitFareCxrLocs>::mapRowToLimitFareCxrLoc(
            row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETLIMITFARECXRLOCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitFareCxrLocs::getCxrlocs()

///////////////////////////////////////////////////////////
//  QueryGetLimitFareRoutings
///////////////////////////////////////////////////////////
const char*
QueryGetLimitFareRoutings::getQueryName() const
{
  return "GETLIMITFAREROUTINGS";
}

void
QueryGetLimitFareRoutings::initialize()
{
  if (!_isInitialized)
  {
    QueryGetLimitFareRoutingsSQLStatement<QueryGetLimitFareRoutings> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETLIMITFAREROUTINGS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetLimitFareRoutings::getRoutings(LimitationFare* a_pLimitationFare)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strSeqNo[15];
  sprintf(strSeqNo, "%d", a_pLimitationFare->seqNo());

  resetSQL();

  substParm(1, a_pLimitationFare->versionDate());
  substParm(strSeqNo, 2);
  substParm(3, a_pLimitationFare->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pLimitationFare->routings().push_back(
        QueryGetLimitFareRoutingsSQLStatement<QueryGetLimitFareRoutings>::mapRowToRouting(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETLIMITFAREGOVCXRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetLimitFareRoutings::getRoutings()
}
