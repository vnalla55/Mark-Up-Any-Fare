//----------------------------------------------------------------------------
// 2014, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCTACarrier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCTACarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCTACarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCTACarrier"));
std::string QueryGetCTACarrier::_baseSQL;
bool QueryGetCTACarrier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCTACarrier> g_GetCTACarrier;

const char*
QueryGetCTACarrier::getQueryName() const
{
  return "GETCTACARRIER";
}

void
QueryGetCTACarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCTACarrierSQLStatement<QueryGetCTACarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCTACARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetCTACarrier::findCarrier(std::vector<tse::CTACarrier*>& lstCF,
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
        QueryGetCTACarrierSQLStatement<QueryGetCTACarrier>::mapRowToCTACarrier(row));
  }

  LOG4CXX_INFO(_logger,
               "GETCTACARRIER: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // findCarrier()

log4cxx::LoggerPtr
QueryGetCTACarrierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCTACarrierHistorical"));
std::string QueryGetCTACarrierHistorical::_baseSQL;
bool QueryGetCTACarrierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCTACarrierHistorical> g_QueryGetCTACarrierHistorical;

const char*
QueryGetCTACarrierHistorical::getQueryName() const
{
  return "GETCTACARRIERHISTORICAL";
}

void
QueryGetCTACarrierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCTACarrierHistoricalSQLStatement<QueryGetCTACarrierHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCTACARRIERHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCTACarrierHistorical::findCarrier(std::vector<tse::CTACarrier*>& lstCF,
                                          const CarrierCode& carrier)
{
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);

  Row* row;

  while ((row = res.nextRow()))
  {
    lstCF.push_back(QueryGetCTACarrierHistoricalSQLStatement<
        QueryGetCTACarrierHistorical>::mapRowToCTACarrier(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCTACARRIERHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrier()
}
