#include "DBAccess/Queries/QueryGetSpanishReferenceFare.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSpanishReferenceFareSQLStatement.h"

namespace tse
{
namespace
{
  Logger gLogger("atseintl.DBAccess.SQLQuery.QueryGetSpanishReferenceFare");
  Logger gLoggerH("atseintl.DBAccess.SQLQuery.QueryGetSpanishReferenceFareHistorical");
}

std::string QueryGetSpanishReferenceFare::_baseSQL;
bool QueryGetSpanishReferenceFare::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetSpanishReferenceFare> _getSpanishReferenceFare;

const char*
QueryGetSpanishReferenceFare::getQueryName() const
{
  return "GETSPANISHREFERENCEFARE";
}

void
QueryGetSpanishReferenceFare::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSpanishReferenceFareSQLStatement<QueryGetSpanishReferenceFare> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSPANISHREFERENCEFARE");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
}

void
QueryGetSpanishReferenceFare::findSpanishReferenceFare (
    std::vector<SpanishReferenceFareInfo*>& lst,
    const CarrierCode& tktCarrier, const CarrierCode& fareCarrier,
    const LocCode& sourceLoc, const LocCode& destLoc)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(tktCarrier, 1);
  substParm(fareCarrier, 2);
  substParm(sourceLoc, 3);
  substParm(destLoc, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(gLogger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lst.push_back(QueryGetSpanishReferenceFareSQLStatement<QueryGetSpanishReferenceFare>::mapRow(row));
  }
  LOG4CXX_INFO(gLogger, "GETSPANISHREFERENCEFARE: NumRows: " << res.numRows() << " Time = " << stopTimer()
               << " (" << stopCPU() << ") ms");
  res.freeResult();
}

std::string QueryGetSpanishReferenceFareHistorical::_baseSQL;
bool QueryGetSpanishReferenceFareHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSpanishReferenceFareHistorical> _getSpanishReferenceFareHistorical;

const char*
QueryGetSpanishReferenceFareHistorical::getQueryName() const
{
  return "GETSPANISHREFERENCEFAREHISTORICAL";
}

void
QueryGetSpanishReferenceFareHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSpanishReferenceFareHistoricalSQLStatement<QueryGetSpanishReferenceFareHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSPANISHREFERENCEFAREHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSpanishReferenceFareHistorical::findSpanishReferenceFare(
    std::vector<SpanishReferenceFareInfo*>& lst, const CarrierCode& tktCarrier,
    const CarrierCode& fareCarrier, const LocCode& sourceLoc, const LocCode& destLoc,
    const DateTime& startDate, const DateTime& endDate)
{
  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  substParm(tktCarrier, 1);
  substParm(fareCarrier, 2);
  substParm(sourceLoc, 3);
  substParm(destLoc, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(gLoggerH, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lst.push_back(
        QueryGetSpanishReferenceFareHistoricalSQLStatement<QueryGetSpanishReferenceFareHistorical>::mapRow(
            row));
  }
  LOG4CXX_INFO(gLoggerH,
               "GETSPANISHREFERENCEFAREHISTORICAL: NumRows = " << res.numRows() << " Time = "
               << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
} // tse
