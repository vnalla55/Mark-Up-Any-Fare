//----------------------------------------------------------------------------
//  File:           QueryGetCurrencySelection.cpp
//  Description:    QueryGetCurrencySelection
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

#include "DBAccess/Queries/QueryGetCurrencySelection.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCurrencySelectionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetOneCurSelBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBase"));
std::string QueryGetOneCurSelBase::_baseSQL;
bool QueryGetOneCurSelBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneCurSelBase> g_GetOneCurSelBase;

log4cxx::LoggerPtr
QueryGetCurSelRestrCur::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBase.GetCurSelRestrCur"));
std::string QueryGetCurSelRestrCur::_baseSQL;
bool QueryGetCurSelRestrCur::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurSelRestrCur> g_GetCurSelRestrCur;

log4cxx::LoggerPtr
QueryGetCurSelPsgrType::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBase.GetCurSelPsgrType"));
std::string QueryGetCurSelPsgrType::_baseSQL;
bool QueryGetCurSelPsgrType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurSelPsgrType> g_GetCurSelPsgrType;

log4cxx::LoggerPtr
QueryGetCurSelAseanCur::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBase.GetCurSelAseanCur"));
std::string QueryGetCurSelAseanCur::_baseSQL;
bool QueryGetCurSelAseanCur::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurSelAseanCur> g_GetCurSelAseanCur;

log4cxx::LoggerPtr
QueryGetCurSelTextMsg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBase.GetCurSelTextMsg"));
std::string QueryGetCurSelTextMsg::_baseSQL;
bool QueryGetCurSelTextMsg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCurSelTextMsg> g_GetCurSelTextMsg;

const char*
QueryGetOneCurSelBase::getQueryName() const
{
  return "GETONECURSELBASE";
};

void
QueryGetOneCurSelBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneCurSelBaseSQLStatement<QueryGetOneCurSelBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONECURSELBASE");
    substTableDef(&_baseSQL);

    QueryGetCurSelRestrCur::initialize();
    QueryGetCurSelPsgrType::initialize();
    QueryGetCurSelAseanCur::initialize();
    QueryGetCurSelTextMsg::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneCurSelBase::findCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS,
                                             NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CurrencySelection* cs = nullptr;
  tse::CurrencySelection* csPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cs = QueryGetOneCurSelBaseSQLStatement<QueryGetOneCurSelBase>::mapRowToCurrencySelection(
        row, csPrev);
    if (cs != csPrev)
      lstCS.push_back(cs);

    csPrev = cs;
  }
  LOG4CXX_INFO(_logger,
               "GETONECURSELBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  getCurSelChildren(lstCS);
} // QueryGetOneCurSelBase::findCurrencySelection()

void
QueryGetOneCurSelBase::getCurSelChildren(std::vector<tse::CurrencySelection*>& lstCS)
{
  QueryGetCurSelRestrCur SQLRestrCur(_dbAdapt);
  QueryGetCurSelPsgrType SQLPsgrType(_dbAdapt);
  QueryGetCurSelAseanCur SQLAseanCur(_dbAdapt);
  QueryGetCurSelTextMsg SQLTextMsg(_dbAdapt);

  std::vector<tse::CurrencySelection*>::iterator CSIt;
  for (CSIt = lstCS.begin(); CSIt != lstCS.end(); CSIt++)
  { // Get Children
    SQLRestrCur.getRestrCurs(*CSIt);
    SQLPsgrType.getPsgrTypes(*CSIt);
    SQLAseanCur.getAseanCurs(*CSIt);
    SQLTextMsg.getTextMsg(*CSIt);
  } // for (Iteration thru Parents)
} // QueryGetOneCurSelBase::getCurSelChildren()

///////////////////////////////////////////////////////////
//  QueryGetOneCurSelBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetOneCurSelBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOneCurSelBaseHistorical"));
std::string QueryGetOneCurSelBaseHistorical::_baseSQL;
bool QueryGetOneCurSelBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOneCurSelBaseHistorical> g_GetOneCurSelBaseHistorical;

const char*
QueryGetOneCurSelBaseHistorical::getQueryName() const
{
  return "GETONECURSELBASEHISTORICAL";
};

void
QueryGetOneCurSelBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOneCurSelBaseHistoricalSQLStatement<QueryGetOneCurSelBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETONECURSELBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetCurSelRestrCur::initialize();
    QueryGetCurSelPsgrType::initialize();
    QueryGetCurSelAseanCur::initialize();
    QueryGetCurSelTextMsg::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOneCurSelBaseHistorical::findCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS,
                                                       NationCode& nation)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(nation, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CurrencySelection* cs = nullptr;
  tse::CurrencySelection* csPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cs = QueryGetOneCurSelBaseHistoricalSQLStatement<
        QueryGetOneCurSelBaseHistorical>::mapRowToCurrencySelection(row, csPrev);
    if (cs != csPrev)
      lstCS.push_back(cs);

    csPrev = cs;
  }
  LOG4CXX_INFO(_logger,
               "GETONECURSELBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getCurSelChildren(lstCS);
} // QueryGetOneCurSelBase::findCurrencySelection()

///////////////////////////////////////////////////////////
//  QueryGetAllCurSelBase
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCurSelBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCurSelBase"));
std::string QueryGetAllCurSelBase::_baseSQL;
bool QueryGetAllCurSelBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCurSelBase> g_GetAllCurSelBase;

const char*
QueryGetAllCurSelBase::getQueryName() const
{
  return "GETALLCURSELBASE";
};

void
QueryGetAllCurSelBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCurSelBaseSQLStatement<QueryGetAllCurSelBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCURSELBASE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCurSelBase::findAllCurrencySelection(std::vector<tse::CurrencySelection*>& lstCS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CurrencySelection* cs = nullptr;
  tse::CurrencySelection* csPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cs = QueryGetAllCurSelBaseSQLStatement<QueryGetAllCurSelBase>::mapRowToCurrencySelection(
        row, csPrev);
    if (cs != csPrev)
      lstCS.push_back(cs);

    csPrev = cs;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCURSELBASE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();

  getCurSelChildren(lstCS);
} // QueryGetAllCurSelBase::findAllCurrencySelection()

///////////////////////////////////////////////////////////
//  QueryGetAllCurSelBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCurSelBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCurSelBaseHistorical"));
std::string QueryGetAllCurSelBaseHistorical::_baseSQL;
bool QueryGetAllCurSelBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCurSelBaseHistorical> g_GetAllCurSelBaseHistorical;

const char*
QueryGetAllCurSelBaseHistorical::getQueryName() const
{
  return "GETALLCURSELBASEHISTORICAL";
};

void
QueryGetAllCurSelBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCurSelBaseHistoricalSQLStatement<QueryGetAllCurSelBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCURSELBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCurSelBaseHistorical::findAllCurrencySelection(
    std::vector<tse::CurrencySelection*>& lstCS)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CurrencySelection* cs = nullptr;
  tse::CurrencySelection* csPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cs = QueryGetAllCurSelBaseHistoricalSQLStatement<
        QueryGetAllCurSelBaseHistorical>::mapRowToCurrencySelection(row, csPrev);
    if (cs != csPrev)
      lstCS.push_back(cs);

    csPrev = cs;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCURSELBASEHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

  getCurSelChildren(lstCS);
} // QueryGetAllCurSelBaseHistorical::findAllCurrencySelection()

///////////////////////////////////////////////////////////
//  QueryGetCurSelRestrCur
///////////////////////////////////////////////////////////
const char*
QueryGetCurSelRestrCur::getQueryName() const
{
  return "GETCURSELRESTRCUR";
}

void
QueryGetCurSelRestrCur::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurSelRestrCurSQLStatement<QueryGetCurSelRestrCur> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURSELRESTRCUR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurSelRestrCur::getRestrCurs(CurrencySelection* a_pCurSel)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pCurSel->seqNo());

  resetSQL();

  substParm(1, a_pCurSel->versionDate());
  substParm(a_pCurSel->nation(), 2);
  substParm(strSeqNo, 3);
  substParm(4, a_pCurSel->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pCurSel->restrictedCurs().push_back(
        QueryGetCurSelRestrCurSQLStatement<QueryGetCurSelRestrCur>::mapRowToRestrictedCur(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCURSELRESTRCUR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // QueryGetCurSelRestrCur::getRestrCurs()

///////////////////////////////////////////////////////////
//  QueryGetCurSelPsgrType
///////////////////////////////////////////////////////////
const char*
QueryGetCurSelPsgrType::getQueryName() const
{
  return "GETCURSELPSGRTYPE";
}

void
QueryGetCurSelPsgrType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurSelPsgrTypeSQLStatement<QueryGetCurSelPsgrType> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURSELPSGRTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurSelPsgrType::getPsgrTypes(CurrencySelection* a_pCurSel)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pCurSel->seqNo());

  resetSQL();

  substParm(1, a_pCurSel->versionDate());
  substParm(a_pCurSel->nation(), 2);
  substParm(strSeqNo, 3);
  substParm(4, a_pCurSel->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pCurSel->passengerTypes().push_back(
        QueryGetCurSelPsgrTypeSQLStatement<QueryGetCurSelPsgrType>::mapRowToPassengerType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCURSELPSGRTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // QueryGetCurSelPsgrType::getPsgrTypes()

///////////////////////////////////////////////////////////
//  QueryGetCurSelAseanCur
///////////////////////////////////////////////////////////
const char*
QueryGetCurSelAseanCur::getQueryName() const
{
  return "GETCURSELASEANCUR";
}

void
QueryGetCurSelAseanCur::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurSelAseanCurSQLStatement<QueryGetCurSelAseanCur> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURSELASEANCUR");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurSelAseanCur::getAseanCurs(CurrencySelection* a_pCurSel)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pCurSel->seqNo());

  resetSQL();

  substParm(1, a_pCurSel->versionDate());
  substParm(a_pCurSel->nation(), 2);
  substParm(strSeqNo, 3);
  substParm(4, a_pCurSel->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pCurSel->aseanCurs().push_back(
        QueryGetCurSelAseanCurSQLStatement<QueryGetCurSelAseanCur>::mapRowToCurrencySelection(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCURSELASEANCUR: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // QueryGetCurSelAseanCur::getAseanCurs()

///////////////////////////////////////////////////////////
//  QueryGetCurSelTextMsg
///////////////////////////////////////////////////////////
const char*
QueryGetCurSelTextMsg::getQueryName() const
{
  return "GETCURSELTEXTMSG";
}

void
QueryGetCurSelTextMsg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCurSelTextMsgSQLStatement<QueryGetCurSelTextMsg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCURSELTEXTMSG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCurSelTextMsg::getTextMsg(CurrencySelection* a_pCurSel)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strSeqNo[20];
  sprintf(strSeqNo, "%d", a_pCurSel->seqNo());

  resetSQL();

  substParm(1, a_pCurSel->versionDate());
  substParm(a_pCurSel->nation(), 2);
  substParm(strSeqNo, 3);
  substParm(4, a_pCurSel->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pCurSel->txtSegs().push_back(
        QueryGetCurSelTextMsgSQLStatement<QueryGetCurSelTextMsg>::mapRowToText(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCURSELTEXTMSG: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // QueryGetCurSelTextMsg::getTextMsg()
}
