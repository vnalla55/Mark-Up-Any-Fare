//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetAllInterlineCarrier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllInterlineCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllInterlineCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllInterlineCarrier"));
std::string QueryGetAllInterlineCarrier::_baseSQL;
bool QueryGetAllInterlineCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineCarrier> g_GetAllInterlineCarrier;

const char*
QueryGetAllInterlineCarrier::getQueryName() const
{
  return "GETALLINTERLINECARRIER";
}

void
QueryGetAllInterlineCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineCarrierSQLStatement<QueryGetAllInterlineCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINECARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllInterlineCarrier::findAllInterlineCarrier(
    std::vector<tse::InterlineCarrierInfo*>& interlineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetAllInterlineCarrierSQLStatement<
        QueryGetAllInterlineCarrier>::mapRowToInterlineCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINECARRIER: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineCarrier()

///////////////////////////////////////////////////////////
//
//  QueryGetAllInterlineCarrierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllInterlineCarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllInterlineCarrierHistorical"));
std::string QueryGetAllInterlineCarrierHistorical::_baseSQL;
bool QueryGetAllInterlineCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllInterlineCarrierHistorical>
g_GetAllInterlineCarrierHistorical;

const char*
QueryGetAllInterlineCarrierHistorical::getQueryName() const
{
  return "GETALLINTERLINECARRIERHISTORICAL";
}

void
QueryGetAllInterlineCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllInterlineCarrierHistoricalSQLStatement<QueryGetAllInterlineCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTERLINECARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllInterlineCarrierHistorical::findAllInterlineCarrier(
    std::vector<tse::InterlineCarrierInfo*>& interlineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    interlineCarriers.push_back(QueryGetAllInterlineCarrierHistoricalSQLStatement<
        QueryGetAllInterlineCarrierHistorical>::mapRowToInterlineCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTERLINECARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllInterlineCarrier()
}
