//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetEmdInterlineAgreement.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetEmdInterlineAgreementSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetEmdInterlineAgreement::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetEmdInterlineAgreement"));

std::string QueryGetEmdInterlineAgreement::_baseSQL;
bool QueryGetEmdInterlineAgreement::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetEmdInterlineAgreement> g_GetEmdInterlineAgreement;

const char*
QueryGetEmdInterlineAgreement::getQueryName() const
{
  return "GETEMDINTERLINEAGREEMENT";
}

void
QueryGetEmdInterlineAgreement::initialize()
{
  if (_isInitialized)
    return;

  QueryGetEmdInterlineAgreementSQLStatement<QueryGetEmdInterlineAgreement> sqlStatement;
  _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETEMDINTERLINEAGREEMENT");
  substTableDef(&_baseSQL);
  _isInitialized = true;
}

void
QueryGetEmdInterlineAgreement::findEmdInterlineAgreement(
    std::vector<EmdInterlineAgreementInfo*>& eiaList,
    const NationCode& country,
    const CrsCode& gds,
    const CarrierCode& carrier)
{
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  substParm(1, country); // Insert country code (%1q) parametr into SQL query string.
  substParm(2, gds); // Insert GDS parameter (%2q) into SQL query string.
  substParm(3, carrier); // Insert carrier code (%3q) parameter into SQL query string.

  DBResultSet res(_dbAdapt);
  res.executeQuery(this);

  Row* row;
  EmdInterlineAgreementInfo* eia;
  while ((row = res.nextRow()))
  {
    eia = QueryGetEmdInterlineAgreementSQLStatement<
        QueryGetEmdInterlineAgreement>::mapRowToEmdInterlineAgreementInfo(row);
    eiaList.push_back(eia);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = "
               << res.numRows() << " Time = "
               << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
}

const QueryGetEmdInterlineAgreement&
QueryGetEmdInterlineAgreement::
operator=(const QueryGetEmdInterlineAgreement& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = (SQLQuery&)rhs;
  }
  return *this;
}

const QueryGetEmdInterlineAgreement&
QueryGetEmdInterlineAgreement::
operator=(const std::string& rhs)
{
  if (this != &rhs)
  {
    *((SQLQuery*)this) = rhs;
  }
  return *this;
}

} // tse
