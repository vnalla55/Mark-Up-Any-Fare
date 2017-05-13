//----------------------------------------------------------------------------
// 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetTaxCarrierFlight.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCarrierFlightSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxCarrierFlight::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxCarrierFlight"));
std::string QueryGetTaxCarrierFlight::_baseSQL;
bool QueryGetTaxCarrierFlight::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCarrierFlight> g_GetTaxCarrierFlight;

const char*
QueryGetTaxCarrierFlight::getQueryName() const
{
  return "GETTAXCARRIERFLIGHT";
}

void
QueryGetTaxCarrierFlight::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCarrierFlightSQLStatement<QueryGetTaxCarrierFlight> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCARRIERFLIGHT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCarrierFlight::findCarrierFlight(std::vector<tse::TaxCarrierFlightInfo*>& lstCF,
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

  tse::TaxCarrierFlightInfo* cf = nullptr;
  tse::TaxCarrierFlightInfo* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxCarrierFlightSQLStatement<QueryGetTaxCarrierFlight>::mapRowToTaxCarrierFlight(
        row, cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXCARRIERFLIGHT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findTaxCarrierFlight()

int
QueryGetTaxCarrierFlight::checkFlightWildCard(const char* fltStr)
{
  if (fltStr[0] == '*')
    return -1;
  else
    return stringToInteger(fltStr, __LINE__); // lint !e668
} // checkFlightWildCard()

int
QueryGetTaxCarrierFlight::stringToInteger(const char* stringVal, int lineNumber)
{
  if (stringVal == nullptr)
  {
    LOG4CXX_FATAL(_logger, "stringToInteger - Null pointer to int data. Linber: " << lineNumber);
    throw std::runtime_error("Null pointer to int data");
  }
  else if (*stringVal == '-' || *stringVal == '+')
  {
    if (stringVal[1] < '0' || stringVal[1] > '9')
      return 0;
  }
  else if (*stringVal < '0' || *stringVal > '9')
  {
    return 0;
  }
  return atoi(stringVal);
} // stringToInteger()

///////////////////////////////////////////////////////////
//  QueryGetTaxCarrierFlightHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetTaxCarrierFlightHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTaxCarrierFlightHistorical"));
std::string QueryGetTaxCarrierFlightHistorical::_baseSQL;
bool QueryGetTaxCarrierFlightHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxCarrierFlightHistorical> g_QueryGetTaxCarrierFlightHistorical;

const char*
QueryGetTaxCarrierFlightHistorical::getQueryName() const
{
  return "GETTAXCARRIERFLIGHTHISTORICAL";
}

void
QueryGetTaxCarrierFlightHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxCarrierFlightHistoricalSQLStatement<QueryGetTaxCarrierFlightHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXCARRIERFLIGHTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxCarrierFlightHistorical::findCarrierFlight(
    std::vector<tse::TaxCarrierFlightInfo*>& lstCF,
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

  tse::TaxCarrierFlightInfo* cf = nullptr;
  tse::TaxCarrierFlightInfo* cfPrev = nullptr;
  while ((row = res.nextRow()))
  {
    cf = QueryGetTaxCarrierFlightHistoricalSQLStatement<
        QueryGetTaxCarrierFlightHistorical>::mapRowToTaxCarrierFlight(row, cfPrev);
    if (cf != cfPrev)
      lstCF.push_back(cf);

    cfPrev = cf;
  }
  LOG4CXX_INFO(_logger,
               "GETTAXCARRIERFLIGHTHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findTaxCarrierFlight()
}
