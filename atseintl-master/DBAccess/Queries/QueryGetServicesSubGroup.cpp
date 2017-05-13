// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "DBAccess/Queries/QueryGetServicesSubGroup.h"

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServicesSubGroupSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetServicesSubGroup::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetServicesSubGroup"));
std::string QueryGetServicesSubGroup::_baseSQL;
bool QueryGetServicesSubGroup::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetServicesSubGroup> g_GetServicesSubGroup;

const char*
QueryGetServicesSubGroup::getQueryName() const
{
  return "GETSERVICESSUBGROUP";
}

void
QueryGetServicesSubGroup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetServicesSubGroupSQLStatement<QueryGetServicesSubGroup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSERVICESSUBGROUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetServicesSubGroup::findServicesSubGroup(std::vector<ServicesSubGroup*>& servicesSubGroup,
                                               const ServiceGroup& serviceGroup,
                                               const ServiceGroup& serviceSubGroup)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substParm(serviceGroup, 1);
  substParm(serviceSubGroup, 2);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    servicesSubGroup.push_back(
        QueryGetServicesSubGroupSQLStatement<QueryGetServicesSubGroup>::mapRowToServicesSubGroup(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETSERVICESSUBGROUP: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
}
} // tse
