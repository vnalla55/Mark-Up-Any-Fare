//     Description:    QueryGetETicketCxrs.cpp
//     Created:        11/8/2006
//     Author:         Adrienne A. Stipe
//
//     Updates:
//
//     2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetETicketCxrs.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetETicketCxrsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetETicketCxrs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetETicketCxrs"));
std::string QueryGetETicketCxrs::_baseSQL;
bool QueryGetETicketCxrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetETicketCxrs> g_GetETicketCxrs;

const char*
QueryGetETicketCxrs::getQueryName() const
{
  return "GETETICKETCARRIERS";
};

void
QueryGetETicketCxrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetETicketCxrsSQLStatement<QueryGetETicketCxrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETETICKETCARRIERS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetETicketCxrs::getETicketCarriers(std::vector<CarrierCode>& lstCxr,
                                        const PseudoCityCode& agentPCC,
                                        const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(agentPCC, 1);
  substParm(carrier, 2);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  CarrierCode cxr;
  while ((row = res.nextRow()))
  {
    cxr = QueryGetETicketCxrsSQLStatement<QueryGetETicketCxrs>::mapRowToCarrierCode(row);
    lstCxr.push_back(cxr);
  }
  LOG4CXX_INFO(_logger,
               "GETZONE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " msecs");
  res.freeResult();
}
}
