//----------------------------------------------------------------------------
// 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxText.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxTextSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxText::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxText"));
std::string QueryGetTaxText::_baseSQL;
bool QueryGetTaxText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxText> g_GetTaxText;

const char*
QueryGetTaxText::getQueryName() const
{
  return "GETTAXTEXT";
}

void
QueryGetTaxText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxTextSQLStatement<QueryGetTaxText> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXTEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxText::findTaxText(std::vector<tse::TaxText*>& lstCF, VendorCode& vendor, int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxText* cf = nullptr;
  tse::TaxText* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxTextSQLStatement<QueryGetTaxText>::mapRowToTaxText(row, cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXTEXT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findTaxText()

///////////////////////////////////////////////////////////
//  QueryGetTaxTextHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxTextHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTaxTextHistorical"));
std::string QueryGetTaxTextHistorical::_baseSQL;
bool QueryGetTaxTextHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxTextHistorical> g_QueryGetTaxTextHistorical;

const char*
QueryGetTaxTextHistorical::getQueryName() const
{
  return "GETTAXTEXTHISTORICAL";
}

void
QueryGetTaxTextHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxTextHistoricalSQLStatement<QueryGetTaxTextHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXTTEXTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxTextHistorical::findTaxText(std::vector<tse::TaxText*>& lstCF,
                                       VendorCode& vendor,
                                       int itemNo,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxText* cf = nullptr;
  tse::TaxText* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxTextHistoricalSQLStatement<QueryGetTaxTextHistorical>::mapRowToTaxText(row,
                                                                                           cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXTEXTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findTaxText()
}
