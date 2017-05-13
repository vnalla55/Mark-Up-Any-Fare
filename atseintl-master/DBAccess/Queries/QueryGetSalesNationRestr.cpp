//----------------------------------------------------------------------------
//  File:           QueryGetSalesNationRestr.cpp
//  Description:    QueryGetSalesNationRestr
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

#include "DBAccess/Queries/QueryGetSalesNationRestr.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSalesNationRestrSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSalesNationRestrBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSalesNationRestrBase"));
std::string QueryGetSalesNationRestrBase::_baseSQL;
bool QueryGetSalesNationRestrBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesNationRestrBase> g_GetSalesNationRestrBase;

log4cxx::LoggerPtr
QueryGetSalesNatRestrGovCxrs::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSalesNationRestrBase.GetSalesNatRestrGovCxrs"));
std::string QueryGetSalesNatRestrGovCxrs::_baseSQL;
bool QueryGetSalesNatRestrGovCxrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesNatRestrGovCxrs> g_GetSalesNatRestrGovCxrs;

log4cxx::LoggerPtr
QueryGetSalesNatRestrTktgCxrs::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSalesNationRestrBase.QueryGetSalesNatRestrTktgCxrs"));
std::string QueryGetSalesNatRestrTktgCxrs::_baseSQL;
bool QueryGetSalesNatRestrTktgCxrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesNatRestrTktgCxrs> g_GetSalesNatRestrTktgCxrs;

log4cxx::LoggerPtr
QueryGetSalesNatRestrText::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetSalesNationRestrBase.QueryGetSalesNatRestrText"));
std::string QueryGetSalesNatRestrText::_baseSQL;
bool QueryGetSalesNatRestrText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesNatRestrText> g_GetSalesNatRestrText;

const char*
QueryGetSalesNationRestrBase::getQueryName() const
{
  return "GETSALESNATIONRESTRBASE";
};

void
QueryGetSalesNationRestrBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesNationRestrBaseSQLStatement<QueryGetSalesNationRestrBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESNATIONRESTRBASE");
    substTableDef(&_baseSQL);

    QueryGetSalesNatRestrGovCxrs::initialize();
    QueryGetSalesNatRestrTktgCxrs::initialize();
    QueryGetSalesNatRestrText::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesNationRestrBase::findSalesNationRestr(std::vector<tse::SalesNationRestr*>& lstSNR,
                                                   NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SalesNationRestr* snr = nullptr;
  tse::SalesNationRestr* snrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    snr = QueryGetSalesNationRestrBaseSQLStatement<
        QueryGetSalesNationRestrBase>::mapRowToSalesNationRestrBase(row, snrPrev);
    if (snr != snrPrev)
      lstSNR.push_back(snr);

    snrPrev = snr;
  }
  LOG4CXX_INFO(_logger,
               "GETSALESNATIONRESTRBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSalesNatRestrGovCxrs SQLGovCxrs(_dbAdapt);
  QueryGetSalesNatRestrTktgCxrs SQLTktgCxrs(_dbAdapt);
  QueryGetSalesNatRestrText SQLText(_dbAdapt);
  std::vector<tse::SalesNationRestr*>::iterator i;
  for (i = lstSNR.begin(); i != lstSNR.end(); i++)
  {
    SQLGovCxrs.getGovCxrs(*i);
    SQLTktgCxrs.getTktgCxrs(*i);
    SQLText.getText(*i);
  }
} // QueryGetSalesNationRestrBase::findSalesNationRestr()

///////////////////////////////////////////////////////////
//
//  QueryGetSalesNationRestrBaseHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSalesNationRestrBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSalesNationRestrBaseHistorical"));
std::string QueryGetSalesNationRestrBaseHistorical::_baseSQL;
bool QueryGetSalesNationRestrBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSalesNationRestrBaseHistorical>
g_GetSalesNationRestrBaseHistorical;

const char*
QueryGetSalesNationRestrBaseHistorical::getQueryName() const
{
  return "GETSALESNATIONRESTRBASEHISTORICAL";
};

void
QueryGetSalesNationRestrBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesNationRestrBaseHistoricalSQLStatement<QueryGetSalesNationRestrBaseHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESNATIONRESTRBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetSalesNatRestrGovCxrs::initialize();
    QueryGetSalesNatRestrTktgCxrs::initialize();
    QueryGetSalesNatRestrText::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesNationRestrBaseHistorical::findSalesNationRestr(
    std::vector<tse::SalesNationRestr*>& lstSNR, NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SalesNationRestr* snr = nullptr;
  tse::SalesNationRestr* snrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    snr = QueryGetSalesNationRestrBaseHistoricalSQLStatement<
        QueryGetSalesNationRestrBaseHistorical>::mapRowToSalesNationRestrBase(row, snrPrev);
    if (snr != snrPrev)
      lstSNR.push_back(snr);

    snrPrev = snr;
  }
  LOG4CXX_INFO(_logger,
               "GETSALESNATIONRESTRBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSalesNatRestrGovCxrs SQLGovCxrs(_dbAdapt);
  QueryGetSalesNatRestrTktgCxrs SQLTktgCxrs(_dbAdapt);
  QueryGetSalesNatRestrText SQLText(_dbAdapt);
  std::vector<tse::SalesNationRestr*>::iterator i;
  for (i = lstSNR.begin(); i != lstSNR.end(); i++)
  {
    SQLGovCxrs.getGovCxrs(*i);
    SQLTktgCxrs.getTktgCxrs(*i);
    SQLText.getText(*i);
  }
} // QueryGetSalesNationRestrBaseHistorical::findSalesNationRestr()

///////////////////////////////////////////////////////////
//
//  QueryGetAllSalesNationRestrBaseHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllSalesNationRestrBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllSalesNationRestrBaseHistorical"));
std::string QueryGetAllSalesNationRestrBaseHistorical::_baseSQL;
bool QueryGetAllSalesNationRestrBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllSalesNationRestrBaseHistorical>
g_GetAllSalesNationRestrBaseHistorical;

const char*
QueryGetAllSalesNationRestrBaseHistorical::getQueryName() const
{
  return "GETALLSALESNATIONRESTRBASEHISTORICAL";
};

void
QueryGetAllSalesNationRestrBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllSalesNationRestrBaseHistoricalSQLStatement<QueryGetAllSalesNationRestrBaseHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLSALESNATIONRESTRBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetSalesNatRestrGovCxrs::initialize();
    QueryGetSalesNatRestrTktgCxrs::initialize();
    QueryGetSalesNatRestrText::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllSalesNationRestrBaseHistorical::findAllSalesNationRestr(
    std::vector<tse::SalesNationRestr*>& lstSNR)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::SalesNationRestr* snr = nullptr;
  tse::SalesNationRestr* snrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    snr = QueryGetAllSalesNationRestrBaseHistoricalSQLStatement<
        QueryGetAllSalesNationRestrBaseHistorical>::mapRowToSalesNationRestrBase(row, snrPrev);
    if (snr != snrPrev)
      lstSNR.push_back(snr);

    snrPrev = snr;
  }
  LOG4CXX_INFO(_logger,
               "GETALLSALESNATIONRESTRBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSalesNatRestrGovCxrs SQLGovCxrs(_dbAdapt);
  QueryGetSalesNatRestrTktgCxrs SQLTktgCxrs(_dbAdapt);
  QueryGetSalesNatRestrText SQLText(_dbAdapt);
  std::vector<tse::SalesNationRestr*>::iterator i;
  for (i = lstSNR.begin(); i != lstSNR.end(); i++)
  {
    SQLGovCxrs.getGovCxrs(*i);
    SQLTktgCxrs.getTktgCxrs(*i);
    SQLText.getText(*i);
  }
} // QueryGetSalesNationRestrBase::findAllSalesNationRestr()

///////////////////////////////////////////////////////////
//
//  QueryGetSalesNatRestrGovCxrs
//
///////////////////////////////////////////////////////////
const char*
QueryGetSalesNatRestrGovCxrs::getQueryName() const
{
  return "GETSALESNATRESTRGOVCXRS";
}

void
QueryGetSalesNatRestrGovCxrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesNatRestrGovCxrsSQLStatement<QueryGetSalesNatRestrGovCxrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESNATRESTRGOVCXRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesNatRestrGovCxrs::getGovCxrs(SalesNationRestr* a_pSalesNationRestr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pSalesNationRestr->seqNo());

  resetSQL();

  substParm(a_pSalesNationRestr->nation(), 1);
  substParm(2, a_pSalesNationRestr->versionDate());
  substParm(strSeqNo, 3);
  substParm(4, a_pSalesNationRestr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSalesNationRestr->govCxrs().push_back(
        QueryGetSalesNatRestrGovCxrsSQLStatement<QueryGetSalesNatRestrGovCxrs>::mapRowToCarrier(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETSALESNATRESTRGOVCXRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSalesNatRestrGovCxrs::getGovCxrs()

///////////////////////////////////////////////////////////
//
//  QueryGetSalesNatRestrTktgCxrs
//
///////////////////////////////////////////////////////////
const char*
QueryGetSalesNatRestrTktgCxrs::getQueryName() const
{
  return "GETSALESNATRESTRTKTGCXRS";
}

void
QueryGetSalesNatRestrTktgCxrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesNatRestrTktgCxrsSQLStatement<QueryGetSalesNatRestrTktgCxrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESNATRESTRTKTGCXRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesNatRestrTktgCxrs::getTktgCxrs(SalesNationRestr* a_pSalesNationRestr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pSalesNationRestr->seqNo());

  resetSQL();

  substParm(a_pSalesNationRestr->nation(), 1);
  substParm(2, a_pSalesNationRestr->versionDate());
  substParm(strSeqNo, 3);
  substParm(4, a_pSalesNationRestr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSalesNationRestr->tktgCxrs().push_back(
        QueryGetSalesNatRestrTktgCxrsSQLStatement<QueryGetSalesNatRestrTktgCxrs>::mapRowToCarrier(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETSALESNATRESTRTKTGCXRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSalesNatRestrTktgCxrs::getTktgCxrs()

///////////////////////////////////////////////////////////
//
//  QueryGetSalesNatRestrText
//
///////////////////////////////////////////////////////////
const char*
QueryGetSalesNatRestrText::getQueryName() const
{
  return "GETSALESNATRESTRTEXT";
}

void
QueryGetSalesNatRestrText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSalesNatRestrTextSQLStatement<QueryGetSalesNatRestrText> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSALESNATRESTRTEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSalesNatRestrText::getText(SalesNationRestr* a_pSalesNationRestr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pSalesNationRestr->seqNo());

  resetSQL();

  substParm(a_pSalesNationRestr->nation(), 1);
  substParm(2, a_pSalesNationRestr->versionDate());
  substParm(strSeqNo, 3);
  substParm(4, a_pSalesNationRestr->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSalesNationRestr->txtSegs().push_back(
        QueryGetSalesNatRestrTextSQLStatement<QueryGetSalesNatRestrText>::mapRowToText(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSALESNATRESTRTEXT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSalesNatRestrText::getText()
}
