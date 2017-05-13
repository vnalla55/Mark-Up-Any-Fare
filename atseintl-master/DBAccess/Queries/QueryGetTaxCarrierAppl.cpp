//----------------------------------------------------------------------------
// 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxCarrierAppl.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCarrierApplSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxCarrierAppl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCarrierAppl"));
std::string QueryGetTaxCarrierAppl::_baseSQL;
bool QueryGetTaxCarrierAppl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCarrierAppl> g_GetTaxCarrierAppl;

const char*
QueryGetTaxCarrierAppl::getQueryName() const
{
  return "GETTAXCARRIERAPPL";
}

void
QueryGetTaxCarrierAppl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCarrierApplSQLStatement<QueryGetTaxCarrierAppl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCARRIERAPPL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCarrierAppl::findTaxCarrierAppl(std::vector<tse::TaxCarrierAppl*>& lstCF,
                                           VendorCode& vendor,
                                           int itemNo)
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

  tse::TaxCarrierAppl* cf = nullptr;
  tse::TaxCarrierAppl* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxCarrierApplSQLStatement<QueryGetTaxCarrierAppl>::mapRowToTaxCarrierAppl(row,
                                                                                            cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETAXCARRIERAPPL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findTaxCarrierAppl()

///////////////////////////////////////////////////////////
//  QueryGetTaxCarrierApplHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxCarrierApplHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTaxCarrierApplHistorical"));
std::string QueryGetTaxCarrierApplHistorical::_baseSQL;
bool QueryGetTaxCarrierApplHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCarrierApplHistorical> g_QueryGetTaxCarrierApplHistorical;

const char*
QueryGetTaxCarrierApplHistorical::getQueryName() const
{
  return "GETTAXCARRIERAPPLHISTORICAL";
}

void
QueryGetTaxCarrierApplHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCarrierApplHistoricalSQLStatement<QueryGetTaxCarrierApplHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXTTEXTINFOHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCarrierApplHistorical::findTaxCarrierAppl(std::vector<tse::TaxCarrierAppl*>& lstCF,
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

  tse::TaxCarrierAppl* cf = nullptr;
  tse::TaxCarrierAppl* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxCarrierApplHistoricalSQLStatement<
        QueryGetTaxCarrierApplHistorical>::mapRowToTaxCarrierAppl(row, cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXCARRIERAOOLHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTaxCarrierAppl()
}
