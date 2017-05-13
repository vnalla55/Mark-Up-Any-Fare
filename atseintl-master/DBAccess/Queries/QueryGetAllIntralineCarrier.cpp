//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetAllIntralineCarrier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllIntralineCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllIntralineCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIntralineCarrier"));
std::string QueryGetAllIntralineCarrier::_baseSQL;
bool QueryGetAllIntralineCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIntralineCarrier> g_GetAllIntralineCarrier;

const char*
QueryGetAllIntralineCarrier::getQueryName() const
{
  return "GETALLINTRALINECARRIER";
}

void
QueryGetAllIntralineCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIntralineCarrierSQLStatement<QueryGetAllIntralineCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTRALINECARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIntralineCarrier::findAllIntralineCarrier(
    std::vector<tse::IntralineCarrierInfo*>& intralineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    intralineCarriers.push_back(QueryGetAllIntralineCarrierSQLStatement<
        QueryGetAllIntralineCarrier>::mapRowToIntralineCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTRALINECARRIER: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIntralineCarrier()

///////////////////////////////////////////////////////////
//
//  QueryGetAllIntralineCarrierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllIntralineCarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllIntralineCarrierHistorical"));
std::string QueryGetAllIntralineCarrierHistorical::_baseSQL;
bool QueryGetAllIntralineCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllIntralineCarrierHistorical>
g_GetAllIntralineCarrierHistorical;

const char*
QueryGetAllIntralineCarrierHistorical::getQueryName() const
{
  return "GETALLINTRALINECARRIERHISTORICAL";
}

void
QueryGetAllIntralineCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllIntralineCarrierHistoricalSQLStatement<QueryGetAllIntralineCarrierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLINTRALINECARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllIntralineCarrierHistorical::findAllIntralineCarrier(
    std::vector<tse::IntralineCarrierInfo*>& intralineCarriers)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    intralineCarriers.push_back(QueryGetAllIntralineCarrierHistoricalSQLStatement<
        QueryGetAllIntralineCarrierHistorical>::mapRowToIntralineCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETALLINTRALINECARRIERHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllIntralineCarrier()
}
