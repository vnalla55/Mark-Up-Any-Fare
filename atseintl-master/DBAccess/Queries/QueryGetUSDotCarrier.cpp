//----------------------------------------------------------------------------
// 2012, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetUSDotCarrier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetUSDotCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetUSDotCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetUSDotCarrier"));
std::string QueryGetUSDotCarrier::_baseSQL;
bool QueryGetUSDotCarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetUSDotCarrier> g_GetUSDotCarrier;

const char*
QueryGetUSDotCarrier::getQueryName() const
{
  return "GETUSDOTCARRIER";
}

void
QueryGetUSDotCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetUSDotCarrierSQLStatement<QueryGetUSDotCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETUSDOTCARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetUSDotCarrier::findCarrier(std::vector<tse::USDotCarrier*>& lstCF,
                                  const CarrierCode& carrier)
{
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);

  Row* row;

  while ((row = res.nextRow()))
  {
    lstCF.push_back(
        QueryGetUSDotCarrierSQLStatement<QueryGetUSDotCarrier>::mapRowToUSDotCarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETUSDOTCARRIER: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findCarrier()

///////////////////////////////////////////////////////////
//  QueryGetUSDotCarrierHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetUSDotCarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetUSDotCarrierHistorical"));
std::string QueryGetUSDotCarrierHistorical::_baseSQL;
bool QueryGetUSDotCarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetUSDotCarrierHistorical> g_QueryGetUSDotCarrierHistorical;

const char*
QueryGetUSDotCarrierHistorical::getQueryName() const
{
  return "GETUSDOTCARRIERHISTORICAL";
}

void
QueryGetUSDotCarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetUSDotCarrierHistoricalSQLStatement<QueryGetUSDotCarrierHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETUSDOTCARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetUSDotCarrierHistorical::findCarrier(std::vector<tse::USDotCarrier*>& lstCF,
                                            const CarrierCode& carrier)
{
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);

  Row* row;

  while ((row = res.nextRow()))
  {
    lstCF.push_back(QueryGetUSDotCarrierHistoricalSQLStatement<
        QueryGetUSDotCarrierHistorical>::mapRowToUSDotCarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETUSDOTCARRIERHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrier()
}
