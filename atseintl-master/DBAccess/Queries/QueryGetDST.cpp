//----------------------------------------------------------------------------
//  File:           QueryGetDST.cpp
//  Description:    QueryGetDST
//  Created:        15/11/2007
// Authors:         Leszek Banaszek
//
//  Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetDST.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDSTSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetDST::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetDST"));
std::string QueryGetDST::_baseSQL;
bool QueryGetDST::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetDST> g_GetDST;

const char*
QueryGetDST::getQueryName() const
{
  return "GETDST";
}

void
QueryGetDST::initialize()
{
  if (!_isInitialized)
  {
    QueryGetDSTSQLStatement<QueryGetDST> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETDST");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

bool
QueryGetDST::findDST(tse::DST& dst, const DSTGrpCode& dstGrp)
{
  bool found = false;
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(dstGrp, 1);
  static const Hours twoYearsAndOneDay(17568);
  DateTime maxDT = DateTime::localTime() - twoYearsAndOneDay;
  substParm(2, maxDT);

  // LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    found = true;
    QueryGetDSTSQLStatement<QueryGetDST>::mapRowToDST(row, dst);
  }
  // LOG4CXX_INFO(_logger,"GETDST:  Time = "
  //                                          << stopTimer() << " (" <<stopCPU() << ")");
  res.freeResult();
  return found;
} // findDST()
}
