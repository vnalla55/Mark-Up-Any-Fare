// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------


#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxExemption.h"
#include "DBAccess/Queries/QueryGetTaxExemptionSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxExemption::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxExemption"));
std::string QueryGetTaxExemption::_baseSQL;
bool QueryGetTaxExemption::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExemption> g_GetTaxExemption;

log4cxx::LoggerPtr
QueryGetTaxExemptionHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxExemptionHistorical"));
std::string QueryGetTaxExemptionHistorical::_baseSQL;
bool QueryGetTaxExemptionHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxExemptionHistorical> g_GetTaxExemptionHistorical;

const char*
QueryGetTaxExemption::getQueryName() const
{
  return "GETTAXEXEMPTION";
}

void
QueryGetTaxExemption::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExemptionSQLStatement<QueryGetTaxExemption> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPTION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetTaxExemptionHistorical::getQueryName() const
{
  return "GETTAXEXEMPTIONHISTORICAL";
}

void
QueryGetTaxExemptionHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxExemptionHistoricalSQLStatement<QueryGetTaxExemptionHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXEXEMPTIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTaxExemption::findTaxExemption(std::vector<TaxExemption*>& taxExemption, DBResultSet& res)
{
  Row* row = nullptr;

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  taxExemption.reserve(res.numRows());
  TaxExemption* prev = nullptr;
  while ((row = res.nextRow()))
  {
    TaxExemption* rec =
      QueryGetTaxExemptionSQLStatement<QueryGetTaxExemption>::mapRowToTaxExemption(row, prev);
    if (prev != rec)
    {
      taxExemption.push_back(rec);
      prev = rec;
    }
  }
  LOG4CXX_INFO(_logger, "" << getQueryName()
               << ": NumRows = " << res.numRows() << " Time = "
               << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

void
QueryGetTaxExemption::findTaxExemption(std::vector<TaxExemption*>& taxExemption,
    const TaxCode& code, const PseudoCityCode& channelId)
{
  DBResultSet res(_dbAdapt);

  substParm(code, 1);
  substParm(channelId, 2);
  substCurrentDate();

  findTaxExemption(taxExemption, res);
}

void
QueryGetTaxExemptionHistorical::findTaxExemption(std::vector<TaxExemption*>& taxExemption,
    const TaxCode& code, const PseudoCityCode& channelId)
{
  DBResultSet res(_dbAdapt);

  substParm(code, 1);
  substParm(channelId, 2);

  QueryGetTaxExemption::findTaxExemption(taxExemption, res);
}

} //namespace
