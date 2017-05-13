//----------------------------------------------------------------------------
//  File:           QueryGetTpdPsr.cpp
//  Description:    QueryGetTpdPsr
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// (C) 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTpdPsr.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTpdPsrSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTpdPsrBase::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTpdPsrBase"));
std::string QueryGetTpdPsrBase::_baseSQL;

log4cxx::LoggerPtr
QueryGetTpdPsrViaCxrLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTpdPsrBase.GetTpdPsrViaCxrLocs"));
std::string QueryGetTpdPsrViaCxrLocs::_baseSQL;

log4cxx::LoggerPtr
QueryGetTpdPsrViaExcepts::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTpdPsrBase.GetTpdPsrViaExcepts"));
std::string QueryGetTpdPsrViaExcepts::_baseSQL;

log4cxx::LoggerPtr
QueryGetTpdPsrViaGeoLocs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTpdPsrBase.GetTpdPsrViaGeoLocs"));
std::string QueryGetTpdPsrViaGeoLocs::_baseSQL;

bool QueryGetTpdPsrBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTpdPsrBase> g_GetTpdPsrBase;

const char*
QueryGetTpdPsrBase::getQueryName() const
{
  return "GETTPDPSRBASE";
};

void
QueryGetTpdPsrBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTpdPsrBaseSQLStatement<QueryGetTpdPsrBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTPDPSRBASE");
    substTableDef(&_baseSQL);

    QueryGetTpdPsrViaCxrLocs::initialize();
    QueryGetTpdPsrViaExcepts::initialize();
    QueryGetTpdPsrViaGeoLocs::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTpdPsrBase::findTpdPsr(std::vector<tse::TpdPsr*>& lstTP,
                               Indicator applInd,
                               const CarrierCode& carrier,
                               Indicator area1,
                               Indicator area2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, applInd);
  substParm(2, carrier);
  substParm(3, area1);
  substParm(4, area2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TpdPsr* tp = nullptr;
  tse::TpdPsr* tpPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tp = QueryGetTpdPsrBaseSQLStatement<QueryGetTpdPsrBase>::mapRowToTpdPsrBase(row, tpPrev);
    if (tp != tpPrev)
      lstTP.push_back(tp);

    tpPrev = tp;
  }
  LOG4CXX_INFO(_logger,
               "GETTPDPSRBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();

  findTpdPsrChildren(lstTP);
} // QueryGetTpdPsrBase::findTpdPsr()

int
QueryGetTpdPsrBase::charToInt(const char* s)
{
  if (s == nullptr || *s < '0' || *s > '9')
  {
    return -1;
  }
  return *s - '0';
}

void
QueryGetTpdPsrBase::findTpdPsrChildren(std::vector<tse::TpdPsr*>& lstTP)
{
  QueryGetTpdPsrViaCxrLocs SQLViaCxrLocs(_dbAdapt);
  QueryGetTpdPsrViaExcepts SQLViaExcepts(_dbAdapt);
  QueryGetTpdPsrViaGeoLocs SQLViaGeoLocs(_dbAdapt);

  std::vector<tse::TpdPsr*>::iterator TPIt;
  for (TPIt = lstTP.begin(); TPIt != lstTP.end(); TPIt++)
  { // Get Children
    SQLViaCxrLocs.getViaCxrLocs(*TPIt);
    SQLViaExcepts.getViaExcepts(*TPIt);
    SQLViaGeoLocs.getViaGeoLocs(*TPIt);
  } // for (Iteration thru Parents)
} // QueryGetTpdPsrBase::findTpdPsrChildren()

///////////////////////////////////////////////////////////
//  QueryGetTpdPsrBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTpdPsrBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTpdPsrBaseHistorical"));
std::string QueryGetTpdPsrBaseHistorical::_baseSQL;
bool QueryGetTpdPsrBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTpdPsrBaseHistorical> g_GetTpdPsrBaseHistorical;

const char*
QueryGetTpdPsrBaseHistorical::getQueryName() const
{
  return "GETTPDPSRBASEHISTORICAL";
};

void
QueryGetTpdPsrBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTpdPsrBaseHistoricalSQLStatement<QueryGetTpdPsrBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTPDPSRBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTpdPsrBaseHistorical::findTpdPsr(std::vector<tse::TpdPsr*>& lstTP,
                                         Indicator applInd,
                                         const CarrierCode& carrier,
                                         Indicator area1,
                                         Indicator area2,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, applInd);
  substParm(2, carrier);
  substParm(3, area1);
  substParm(4, area2);
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TpdPsr* tp = nullptr;
  tse::TpdPsr* tpPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tp = QueryGetTpdPsrBaseHistoricalSQLStatement<QueryGetTpdPsrBaseHistorical>::mapRowToTpdPsrBase(
        row, tpPrev);
    if (tp != tpPrev)
      lstTP.push_back(tp);

    tpPrev = tp;
  }
  LOG4CXX_INFO(_logger,
               "GETTPDPSRBASEHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();

  findTpdPsrChildren(lstTP);
} // QueryGetTpdPsrBaseHistorical::findTpdPsr()

///////////////////////////////////////////////////////////
//  QueryGetAllTpdPsrBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTpdPsrBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTpdPsrBase"));
std::string QueryGetAllTpdPsrBase::_baseSQL;
bool QueryGetAllTpdPsrBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTpdPsrBase> g_GetAllTpdPsrBase;

const char*
QueryGetAllTpdPsrBase::getQueryName() const
{
  return "GETALLTPDPSRBASE";
};

void
QueryGetAllTpdPsrBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTpdPsrBaseSQLStatement<QueryGetAllTpdPsrBase> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTPDPSRBASE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTpdPsrBase::findAllTpdPsr(std::vector<tse::TpdPsr*>& lstTP)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TpdPsr* tp = nullptr;
  tse::TpdPsr* tpPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tp = QueryGetAllTpdPsrBaseSQLStatement<QueryGetAllTpdPsrBase>::mapRowToTpdPsrBase(row, tpPrev);
    if (tp != tpPrev)
      lstTP.push_back(tp);

    tpPrev = tp;
  }
  LOG4CXX_INFO(_logger,
               "GETALLTPDPSRBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  findTpdPsrChildren(lstTP);
} // QueryGetAllTpdPsrBase::findAllTpdPsr()

///////////////////////////////////////////////////////////
//  QueryGetTpdPsrViaCxrLocs
///////////////////////////////////////////////////////////
bool QueryGetTpdPsrViaCxrLocs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTpdPsrViaCxrLocs> g_GetTpdPsrViaCxrLocs;

const char*
QueryGetTpdPsrViaCxrLocs::getQueryName() const
{
  return "GETTPDPSRVIACXRLOCS";
}

void
QueryGetTpdPsrViaCxrLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTpdPsrViaCxrLocsSQLStatement<QueryGetTpdPsrViaCxrLocs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTPDPSRVIACXRLOCS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTpdPsrViaCxrLocs::getViaCxrLocs(TpdPsr* a_pTpdPsr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strApplInd[20];
  sprintf(strApplInd, "%c", a_pTpdPsr->applInd());
  char strArea1[20];
  sprintf(strArea1, "%c", a_pTpdPsr->area1());
  char strArea2[20];
  sprintf(strArea2, "%c", a_pTpdPsr->area2());
  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pTpdPsr->seqNo());

  resetSQL();

  substParm(strApplInd, 1);
  substParm(a_pTpdPsr->carrier(), 2);
  substParm(strArea1, 3);
  substParm(strArea2, 4);
  substParm(5, a_pTpdPsr->versionDate());
  substParm(strSeqNo, 6);
  substParm(7, a_pTpdPsr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    TpdPsrViaCxrLoc* nxtVCL = new TpdPsrViaCxrLoc;
    QueryGetTpdPsrViaCxrLocsSQLStatement<QueryGetTpdPsrViaCxrLocs>::mapRowToTpdPsrViaCxrLoc(row,
                                                                                            nxtVCL);
    a_pTpdPsr->viaCxrLocs().push_back(nxtVCL);
  } // while (Fetching ViaCxrLocs)
  LOG4CXX_INFO(_logger,
               "GETTPDPSRVIACXRLOCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTpdPsrViaCxrLocs::getViaCxrLocs()

///////////////////////////////////////////////////////////
//  QueryGetTpdPsrViaExcepts
///////////////////////////////////////////////////////////
bool QueryGetTpdPsrViaExcepts::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTpdPsrViaExcepts> g_GetTpdPsrViaExcepts;

const char*
QueryGetTpdPsrViaExcepts::getQueryName() const
{
  return "GETTPDPSRVIAEXCEPTS";
}

void
QueryGetTpdPsrViaExcepts::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTpdPsrViaExceptsSQLStatement<QueryGetTpdPsrViaExcepts> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTPDPSRVIAEXCEPTS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTpdPsrViaExcepts::getViaExcepts(TpdPsr* a_pTpdPsr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strApplInd[20];
  sprintf(strApplInd, "%c", a_pTpdPsr->applInd());
  char strArea1[20];
  sprintf(strArea1, "%c", a_pTpdPsr->area1());
  char strArea2[20];
  sprintf(strArea2, "%c", a_pTpdPsr->area2());
  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pTpdPsr->seqNo());

  resetSQL();

  substParm(strApplInd, 1);
  substParm(a_pTpdPsr->carrier(), 2);
  substParm(strArea1, 3);
  substParm(strArea2, 4);
  substParm(5, a_pTpdPsr->versionDate());
  substParm(strSeqNo, 6);
  substParm(7, a_pTpdPsr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    TpdPsrViaExcept* nxtVE = new TpdPsrViaExcept;
    QueryGetTpdPsrViaExceptsSQLStatement<QueryGetTpdPsrViaExcepts>::mapRowToTpdPsrViaExcept(row,
                                                                                            nxtVE);
    a_pTpdPsr->viaExcepts().push_back(nxtVE);
  } // while (Fetching ViaExcepts)
  LOG4CXX_INFO(_logger,
               "GETTPDPSRVIAEXCEPTS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTpdPsrViaExcepts::getViaExcepts()

///////////////////////////////////////////////////////////
//  QueryGetTpdPsrViaGeoLocs
///////////////////////////////////////////////////////////
bool QueryGetTpdPsrViaGeoLocs::_isInitialized = false;

SQLQueryInitializerHelper<QueryGetTpdPsrViaGeoLocs> g_GetTpdPsrViaGeoLocs;

const char*
QueryGetTpdPsrViaGeoLocs::getQueryName() const
{
  return "GETTPDPSRVIAGEOLOCS";
}

void
QueryGetTpdPsrViaGeoLocs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTpdPsrViaGeoLocsSQLStatement<QueryGetTpdPsrViaGeoLocs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTPDPSRVIAGEOLOCS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTpdPsrViaGeoLocs::getViaGeoLocs(TpdPsr* a_pTpdPsr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strApplInd[20];
  sprintf(strApplInd, "%c", a_pTpdPsr->applInd());
  char strArea1[20];
  sprintf(strArea1, "%c", a_pTpdPsr->area1());
  char strArea2[20];
  sprintf(strArea2, "%c", a_pTpdPsr->area2());
  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pTpdPsr->seqNo());

  resetSQL();

  substParm(strApplInd, 1);
  substParm(a_pTpdPsr->carrier(), 2);
  substParm(strArea1, 3);
  substParm(strArea2, 4);
  substParm(5, a_pTpdPsr->versionDate());
  substParm(strSeqNo, 6);
  substParm(7, a_pTpdPsr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    TpdPsrViaGeoLoc* nxtVGL = new TpdPsrViaGeoLoc;
    QueryGetTpdPsrViaGeoLocsSQLStatement<QueryGetTpdPsrViaGeoLocs>::mapRowToTpdPsrViaGeoLoc(row,
                                                                                            nxtVGL);
    a_pTpdPsr->viaGeoLocs().push_back(nxtVGL);
  } // while (Fetching ViaExcepts)
  LOG4CXX_INFO(_logger,
               "GETTPDPSRVIAGEOLOCS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetTpdPsrViaGeoLocs::getViaGeoLocs()
}
