//----------------------------------------------------------------------------
// � 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxCodeText.h"

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCodeTextSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxCodeText::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCodeText"));

std::string QueryGetTaxCodeText::_baseSQL;
bool QueryGetTaxCodeText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeText> g_GetTaxCodeText;

QueryGetTaxCodeText::QueryGetTaxCodeText(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};

QueryGetTaxCodeText::QueryGetTaxCodeText(DBAdapter* dbAdapt, const std::string& sqlStatement)
  : SQLQuery(dbAdapt, sqlStatement) {};

QueryGetTaxCodeText::~QueryGetTaxCodeText() {};

const QueryGetTaxCodeText&
QueryGetTaxCodeText::
operator=(const QueryGetTaxCodeText& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
}

const QueryGetTaxCodeText&
QueryGetTaxCodeText::
operator=(const std::string& another)
{
  if (this != &another)
  {
    SQLQuery::operator=(another);
  }
  return *this;
};

void
QueryGetTaxCodeText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeTextSQLStatement<QueryGetTaxCodeText> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCODETEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

const char*
QueryGetTaxCodeText::getQueryName() const
{
  return "GETTAXCODETEXT";
}

void
QueryGetTaxCodeText::findTaxCodeTextInfo(std::vector<const TaxCodeTextInfo*>& data,
                                         const VendorCode& vendor,
                                         int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    data.push_back(
        QueryGetTaxCodeTextSQLStatement<QueryGetTaxCodeText>::mapRowToTaxCodeTextInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}

log4cxx::LoggerPtr
QueryGetTaxCodeTextHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTaxCodeTextHistorical"));
std::string QueryGetTaxCodeTextHistorical::_baseSQL;
bool QueryGetTaxCodeTextHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCodeTextHistorical> g_GetTaxCodeTextHistorical;

QueryGetTaxCodeTextHistorical::QueryGetTaxCodeTextHistorical(DBAdapter* dbAdapt)
  : QueryGetTaxCodeText(dbAdapt, _baseSQL)
{
}

QueryGetTaxCodeTextHistorical::~QueryGetTaxCodeTextHistorical() {}

const char*
QueryGetTaxCodeTextHistorical::getQueryName() const
{
  return "GETTAXCODETEXTHISTORICAL";
}

void
QueryGetTaxCodeTextHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCodeTextHistoricalSQLStatement<QueryGetTaxCodeTextHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCODETEXTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetTaxCodeTextHistorical::findTaxCodeTextInfo(std::vector<const TaxCodeTextInfo*>& data,
                                                   const VendorCode& vendor,
                                                   int itemNumber,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    data.push_back(QueryGetTaxCodeTextHistoricalSQLStatement<
        QueryGetTaxCodeTextHistorical>::mapRowToTaxCodeTextInfo(row));
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ")");
  res.freeResult();
}
}
