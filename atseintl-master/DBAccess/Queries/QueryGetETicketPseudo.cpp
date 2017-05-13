//     Description:    QueryGetETicketPseudo.cpp
//     Created:        11/13/2006
//     Author:         Adrienne A. Stipe
//
//     Updates:
//
//     2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetETicketPseudo.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetETicketPseudoSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetETicketPseudo::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetETicketPseudo"));
std::string QueryGetETicketPseudo::_baseSQL;
bool QueryGetETicketPseudo::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetETicketPseudo> g_GetETicketPseudo;

const char*
QueryGetETicketPseudo::getQueryName() const
{
  return "GETETICKETPSEUDO";
};

void
QueryGetETicketPseudo::initialize()
{
  if (!_isInitialized)
  {
    QueryGetETicketPseudoSQLStatement<QueryGetETicketPseudo> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETETICKETPSEUDO");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

CarrierCode
QueryGetETicketPseudo::getETicketPseudo(const PseudoCityCode& agentPCC, const CarrierCode& carrier)
{
  CarrierCode retVal;
  DBResultSet res(_dbAdapt);
  Row* row;

  substParm(agentPCC, 1);
  substParm(carrier, 2);
  res.executeQuery(this);
  if ((row = res.nextRow()))
    retVal = QueryGetETicketPseudoSQLStatement<QueryGetETicketPseudo>::mapRowToCarrierCode(row);
  else
    retVal = carrier;
  res.freeResult();

  return retVal;
} // getETicketPseudo()
}
