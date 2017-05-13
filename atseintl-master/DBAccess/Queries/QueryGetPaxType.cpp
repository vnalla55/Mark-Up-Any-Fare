//----------------------------------------------------------------------------
//  File:           QueryGetPaxType.cpp
//  Description:    QueryGetPaxType
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

#include "DBAccess/Queries/QueryGetPaxType.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPaxTypeSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPaxType::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPaxType"));
std::string QueryGetPaxType::_baseSQL;
bool QueryGetPaxType::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxType> g_GetPaxType;

const char*
QueryGetPaxType::getQueryName() const
{
  return "GETPAXTYPE";
}

void
QueryGetPaxType::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypeSQLStatement<QueryGetPaxType> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPaxType::findPaxType(std::vector<tse::PaxTypeInfo*>& paxTypes,
                             const PaxTypeCode& paxType,
                             const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, paxType);
  substParm(2, vendor);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypes.push_back(QueryGetPaxTypeSQLStatement<QueryGetPaxType>::mapRowToPaxType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPAXTYPE: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
} // findPaxType()
///////////////////////////////////////////////////////////
//
//  QueryGetPaxTypes
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPaxTypes::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPaxTypes"));
std::string QueryGetPaxTypes::_baseSQL;
bool QueryGetPaxTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxTypes> g_GetPaxTypes;

const char*
QueryGetPaxTypes::getQueryName() const
{
  return "GETPAXTYPES";
}

void
QueryGetPaxTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypesSQLStatement<QueryGetPaxTypes> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPaxTypes::findAllPaxType(std::vector<tse::PaxTypeInfo*>& paxTypes)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypes.push_back(QueryGetPaxTypesSQLStatement<QueryGetPaxTypes>::mapRowToPaxType(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPAXTYPES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findAllPaxType()
}
