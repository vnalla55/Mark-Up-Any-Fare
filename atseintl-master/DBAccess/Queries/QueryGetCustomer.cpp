//----------------------------------------------------------------------------
//  File:           QueryGetCustomer.cpp
//  Description:    QueryGetCustomer
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetCustomer.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCustomerSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCustomer::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCustomer"));
std::string QueryGetCustomer::_baseSQL;
bool QueryGetCustomer::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCustomer> g_GetCustomer;

const char*
QueryGetCustomer::getQueryName() const
{
  return "GETCUSTOMER";
}

void
QueryGetCustomer::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCustomerSQLStatement<QueryGetCustomer> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCUSTOMER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCustomer::findCustomer(std::vector<tse::Customer*>& lstCust, PseudoCityCode pseudoCity)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(pseudoCity, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstCust.push_back(QueryGetCustomerSQLStatement<QueryGetCustomer>::mapRowToCustomer(row));
  }
  LOG4CXX_INFO(_logger,
               "GETCUSTOMER: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ") mSecs");
  res.freeResult();
} // findCustomer()
}
