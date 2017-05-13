//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCustomerActivationControl.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCustomerActivationControlSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCustomerActivationControl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCustomerActivationControl"));
std::string QueryGetCustomerActivationControl::_baseSQL;
bool QueryGetCustomerActivationControl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCustomerActivationControl> g_GetCustomerActivationControl;

log4cxx::LoggerPtr
QueryGetMultiHostActivationAppl::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetCustomerActivationControl.GetMultiHostActivationAppl"));
std::string QueryGetMultiHostActivationAppl::_baseSQL;
bool QueryGetMultiHostActivationAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiHostActivationAppl> g_GetMultiHostActivationAppl;

log4cxx::LoggerPtr
QueryGetCarrierActivationAppl::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetCustomerActivationControl.GetCarrierActivationAppl"));
std::string QueryGetCarrierActivationAppl::_baseSQL;
bool QueryGetCarrierActivationAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierActivationAppl> g_GetCarrierActivationAppl;

log4cxx::LoggerPtr
QueryGetGeoLocationAppl::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetCustomerActivationControl.GetGeoLocationAppl"));
std::string QueryGetGeoLocationAppl::_baseSQL;
bool QueryGetGeoLocationAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeoLocationAppl> g_GetGeoLocationAppl;

const char*
QueryGetCustomerActivationControl::getQueryName() const
{
  return "GETCUSTOMERACTIVATIONCONTROL";
}

void
QueryGetCustomerActivationControl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCustomerActivationControlSQLStatement<QueryGetCustomerActivationControl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCUSTOMERACTIVATIONCONTROL");
    substTableDef(&_baseSQL);

    QueryGetMultiHostActivationAppl::initialize();
    QueryGetCarrierActivationAppl::initialize();
    QueryGetGeoLocationAppl::initialize();
    _isInitialized = true;
  }
}

void
QueryGetCustomerActivationControl::findCustomerActivationControl(
    std::vector<CustomerActivationControl*>& customerActivationControls,
    const std::string& projectCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, projectCode);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {

    customerActivationControls.push_back(QueryGetCustomerActivationControlSQLStatement<
        QueryGetCustomerActivationControl>::mapRowToCustomerActivationControl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCUSTOMERACTIVATIONCONTROL: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  getDetailActivationData(customerActivationControls);
}

void
QueryGetCustomerActivationControl::getDetailActivationData(
    std::vector<CustomerActivationControl*>& customerActivationControls)
{
  QueryGetMultiHostActivationAppl SQLMultiHostActivationAppl(_dbAdapt);
  QueryGetCarrierActivationAppl SQLCarrierActivationAppl(_dbAdapt);
  QueryGetGeoLocationAppl SQLGeoLocationAppl(_dbAdapt);

  std::vector<CustomerActivationControl*>::iterator cacBegin = customerActivationControls.begin();
  std::vector<CustomerActivationControl*>::iterator cacEnd = customerActivationControls.end();

  for (; cacBegin != cacEnd; ++cacBegin)
  {
    SQLMultiHostActivationAppl.findMultiHostActivationAppl(*cacBegin);
    SQLCarrierActivationAppl.findCarrierActivationAppl(*cacBegin);
    SQLGeoLocationAppl.findGeoLocationAppl(*cacBegin);
  }
}

///////////////////////////////////////////////////////////
//  QueryGetCustomerActivationControlHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCustomerActivationControlHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetCustomerActivationControlHistorical"));
std::string QueryGetCustomerActivationControlHistorical::_baseSQL;
bool QueryGetCustomerActivationControlHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCustomerActivationControlHistorical>
g_GetCustomerActivationControlHistorical;

log4cxx::LoggerPtr
QueryGetMultiHostActivationApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCustomerActivationControlHistorical."
                               "GetMultiHostActivationApplHistorical"));
std::string QueryGetMultiHostActivationApplHistorical::_baseSQL;
bool QueryGetMultiHostActivationApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMultiHostActivationApplHistorical>
g_GetMultiHostActivationApplHistorical;

log4cxx::LoggerPtr
QueryGetCarrierActivationApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCustomerActivationControlHistorical."
                               "GetCarrierActivationApplHistorical"));
std::string QueryGetCarrierActivationApplHistorical::_baseSQL;
bool QueryGetCarrierActivationApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierActivationApplHistorical>
g_GetCarrierActivationApplHistorical;

log4cxx::LoggerPtr
QueryGetGeoLocationApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCustomerActivationControlHistorical."
                               "GetGeoLocationApplHistorical"));
std::string QueryGetGeoLocationApplHistorical::_baseSQL;
bool QueryGetGeoLocationApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetGeoLocationApplHistorical> g_GetGeoLocationApplHistorical;

const char*
QueryGetCustomerActivationControlHistorical::getQueryName() const
{
  return "GETCUSTOMERACTIVATIONCONTROLHISTORICAL";
}

void
QueryGetCustomerActivationControlHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCustomerActivationControlHistoricalSQLStatement<
        QueryGetCustomerActivationControlHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCUSTOMERACTIVATIONCONTROLHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetMultiHostActivationApplHistorical::initialize();
    QueryGetCarrierActivationApplHistorical::initialize();
    QueryGetGeoLocationApplHistorical::initialize();

    _isInitialized = true;
  }
}

void
QueryGetCustomerActivationControlHistorical::findCustomerActivationControl(
    std::vector<CustomerActivationControl*>& customerActivationControls,
    const std::string& projectCode,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, projectCode);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    customerActivationControls.push_back(QueryGetCustomerActivationControlHistoricalSQLStatement<
        QueryGetCustomerActivationControlHistorical>::mapRowToCustomerActivationControl(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCUSTOMERACTIVATIONCONTROLHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
  getDetailActivationDataHistorical(customerActivationControls);
}

void
QueryGetCustomerActivationControlHistorical::getDetailActivationDataHistorical(
    std::vector<CustomerActivationControl*>& customerActivationControls)
{
  QueryGetMultiHostActivationApplHistorical SQLMultiHostActivationApplHistorical(_dbAdapt);
  QueryGetCarrierActivationApplHistorical SQLCarrierActivationApplHistorical(_dbAdapt);
  QueryGetGeoLocationApplHistorical SQLGeoLocationApplHistorical(_dbAdapt);

  std::vector<CustomerActivationControl*>::iterator cacBegin = customerActivationControls.begin();
  std::vector<CustomerActivationControl*>::iterator cacEnd = customerActivationControls.end();

  for (; cacBegin != cacEnd; ++cacBegin)
  {
    SQLMultiHostActivationApplHistorical.findMultiHostActivationAppl(*cacBegin);
    SQLCarrierActivationApplHistorical.findCarrierActivationAppl(*cacBegin);
    SQLGeoLocationApplHistorical.findGeoLocationAppl(*cacBegin);
  }
}

///////////////////////////////////////////////////////////
//  QueryGetMultiHostActivationAppl
///////////////////////////////////////////////////////////

const char*
QueryGetMultiHostActivationAppl::getQueryName() const
{
  return "GETMULTIHOSTACTIVATIONAPPL";
}

void
QueryGetMultiHostActivationAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiHostActivationApplSQLStatement<QueryGetMultiHostActivationAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTIHOSTACTIVATIONAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMultiHostActivationAppl::findMultiHostActivationAppl(
    CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetMultiHostActivationApplSQLStatement<QueryGetMultiHostActivationAppl>::
        mapRowToMultiHostActivationAppl(row, customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETMULTIHOSTACTIVATIONAPPL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////////////////
//  QueryGetMultiHostActivationApplHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetMultiHostActivationApplHistorical::getQueryName() const
{
  return "GETMULTIHOSTACTIVATIONAPPLHISTORICAL";
}

void
QueryGetMultiHostActivationApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMultiHostActivationApplHistoricalSQLStatement<QueryGetMultiHostActivationApplHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMULTIHOSTACTIVATIONAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetMultiHostActivationApplHistorical::findMultiHostActivationAppl(
    CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetMultiHostActivationApplHistoricalSQLStatement<
        QueryGetMultiHostActivationApplHistorical>::
        mapRowToMultiHostActivationAppl(row, customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETMULTIHOSTACTIVATIONAPPL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetCarrierActivationAppl
///////////////////////////////////////////////////////////

const char*
QueryGetCarrierActivationAppl::getQueryName() const
{
  return "GETCARRIERACTIVATIONAPPL";
}

void
QueryGetCarrierActivationAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierActivationApplSQLStatement<QueryGetCarrierActivationAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERACTIVATIONAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetCarrierActivationAppl::findCarrierActivationAppl(
    CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetCarrierActivationApplSQLStatement<
        QueryGetCarrierActivationAppl>::mapRowToCarrierActivationAppl(row,
                                                                      customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERACTIVATIONAPPL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////////////////
//  QueryGetCarrierActivationApplHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetCarrierActivationApplHistorical::getQueryName() const
{
  return "GETCARRIERACTIVATIONAPPLHISTORICAL";
}

void
QueryGetCarrierActivationApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierActivationApplHistoricalSQLStatement<QueryGetCarrierActivationApplHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERACTIVATIONAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetCarrierActivationApplHistorical::findCarrierActivationAppl(
    CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetCarrierActivationApplHistoricalSQLStatement<QueryGetCarrierActivationApplHistorical>::
        mapRowToCarrierActivationAppl(row, customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERACTIVATIONAPPLHISTORICAL: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetGeoLocationAppl
///////////////////////////////////////////////////////////

const char*
QueryGetGeoLocationAppl::getQueryName() const
{
  return "GETGEOLOCATIONAPPL";
}

void
QueryGetGeoLocationAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeoLocationApplSQLStatement<QueryGetGeoLocationAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGEOLOCATIONAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetGeoLocationAppl::findGeoLocationAppl(CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetGeoLocationApplSQLStatement<QueryGetGeoLocationAppl>::mapRowToGeoLocationAppl(
        row, customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETGEOLOCATIONAPPL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////////////////
//  QueryGetGeoLocationApplHistorical
///////////////////////////////////////////////////////////////////////
const char*
QueryGetGeoLocationApplHistorical::getQueryName() const
{
  return "GETGEOLOCATIONAPPLHISTORICAL";
}

void
QueryGetGeoLocationApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetGeoLocationApplHistoricalSQLStatement<QueryGetGeoLocationApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETGEOLOCATIONAPPLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetGeoLocationApplHistorical::findGeoLocationAppl(
    CustomerActivationControl* customerActivationControl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(1, customerActivationControl->projCode());
  substParm(2, customerActivationControl->versionDate());
  substParm(3, customerActivationControl->seqNo());
  substParm(4, customerActivationControl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    QueryGetGeoLocationApplHistoricalSQLStatement<
        QueryGetGeoLocationApplHistorical>::mapRowToGeoLocationAppl(row, customerActivationControl);
  }
  LOG4CXX_INFO(_logger,
               "GETGEOLOCATIONAPPLHISTORICAL: NumRows "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

} // tse
