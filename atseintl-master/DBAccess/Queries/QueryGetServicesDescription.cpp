#include "DBAccess/Queries/QueryGetServicesDescription.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServicesDescriptionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetServicesDescription::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetServicesDescrption"));
std::string QueryGetServicesDescription::_baseSQL;
bool QueryGetServicesDescription::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServicesDescription> g_GetServicesDescription;

const char*
QueryGetServicesDescription::getQueryName() const
{
  return "GETSERVICESDESCRIPTION";
}

void
QueryGetServicesDescription::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServicesDescriptionSQLStatement<QueryGetServicesDescription> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICESDESCRIPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServicesDescription::findServicesDescription(
    std::vector<ServicesDescription*>& servicesDescription, const ServiceGroupDescription& value)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(value, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    servicesDescription.push_back(QueryGetServicesDescriptionSQLStatement<
        QueryGetServicesDescription>::mapRowToServicesDescription(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSERVICESDESCRIPTION: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetServicesDescriptionHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetServicesDescriptionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryServicesDescriptionHistorical"));
std::string QueryGetServicesDescriptionHistorical::_baseSQL;
bool QueryGetServicesDescriptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServicesDescriptionHistorical> g_GetServicesDescriptionHistorical;

const char*
QueryGetServicesDescriptionHistorical::getQueryName() const
{
  return "GETSERVICESDESCRIPTIONHISTORICAL";
}

void
QueryGetServicesDescriptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServicesDescriptionHistoricalSQLStatement<QueryGetServicesDescriptionHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICESDESCRIPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServicesDescriptionHistorical::findServicesDescription(
    std::vector<ServicesDescription*>& servicesDescription,
    const ServiceGroupDescription& value,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(value, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    servicesDescription.push_back(QueryGetServicesDescriptionHistoricalSQLStatement<
        QueryGetServicesDescriptionHistorical>::mapRowToServicesDescription(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSERVICESDESCRIPTIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
