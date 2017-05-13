// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBankIdentification.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBankIdentificationSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBankIdentification::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBankIdentification"));
std::string QueryGetBankIdentification::_baseSQL;
bool QueryGetBankIdentification::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBankIdentification> g_GetBankIdentification;

const char*
QueryGetBankIdentification::getQueryName() const
{
  return "GETBANKIDENTIFICATION";
}

void
QueryGetBankIdentification::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBankIdentificationSQLStatement<QueryGetBankIdentification> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBANKIDENTIFICATION");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetBankIdentification::findBankIdentificationInfo(
    std::vector<tse::BankIdentificationInfo*>& bins, const FopBinNumber& binNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(binNumber, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bins.push_back(QueryGetBankIdentificationSQLStatement<
        QueryGetBankIdentification>::mapRowToBankIdentificationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBANKIDENTIFICATION: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//  QueryGetBankIdentificationHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBankIdentificationHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetBankIdentificationHistorical"));
std::string QueryGetBankIdentificationHistorical::_baseSQL;
bool QueryGetBankIdentificationHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBankIdentificationHistorical> g_GetBankIdentificationHistorical;

const char*
QueryGetBankIdentificationHistorical::getQueryName() const
{
  return "GETBANKIDENTIFICATIONHISTORICAL";
}

void
QueryGetBankIdentificationHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBankIdentificationHistoricalSQLStatement<QueryGetBankIdentificationHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBANKIDENTIFICATIONHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetBankIdentificationHistorical::findBankIdentificationInfo(
    std::vector<tse::BankIdentificationInfo*>& bins,
    const FopBinNumber& binNumber,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(binNumber, 1);
  substParm(2, startDate);
  substParm(3, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    bins.push_back(QueryGetBankIdentificationHistoricalSQLStatement<
        QueryGetBankIdentificationHistorical>::mapRowToBankIdentificationInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBANKIDENTIFICATIONHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}
}
