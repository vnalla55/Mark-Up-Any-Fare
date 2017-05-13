//----------------------------------------------------------------------------
//  File:           QueryGetPaxTypeMatrix.cpp
//  Description:    QueryGetPaxTypeMatrix
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

#include "DBAccess/Queries/QueryGetPaxTypeMatrix.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPaxTypeMatrixSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetPaxTypeMatrix::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPaxTypeMatrix"));
std::string QueryGetPaxTypeMatrix::_baseSQL;
bool QueryGetPaxTypeMatrix::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxTypeMatrix> g_GetPaxTypeMatrix;

const char*
QueryGetPaxTypeMatrix::getQueryName() const
{
  return "GETPAXTYPEMATRIX";
}

void
QueryGetPaxTypeMatrix::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypeMatrixSQLStatement<QueryGetPaxTypeMatrix> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPEMATRIX");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPaxTypeMatrix::findPaxTypeMatrix(std::vector<const tse::PaxTypeMatrix*>& paxTypeMatrixs,
                                         const PaxTypeCode& paxType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, paxType);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeMatrixs.push_back(
        QueryGetPaxTypeMatrixSQLStatement<QueryGetPaxTypeMatrix>::mapRowToPaxTypeMatrix(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPAXTYPEMATRIX: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ")");
  res.freeResult();
} // findPaxTypeMatrix()
///////////////////////////////////////////////////////////
//
//  QueryGetPaxTypeMatrixs
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetPaxTypeMatrixs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetPaxTypeMatrixs"));
std::string QueryGetPaxTypeMatrixs::_baseSQL;
bool QueryGetPaxTypeMatrixs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetPaxTypeMatrixs> g_GetPaxTypeMatrixs;

const char*
QueryGetPaxTypeMatrixs::getQueryName() const
{
  return "GETPAXTYPEMATRIXS";
}

void
QueryGetPaxTypeMatrixs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetPaxTypeMatrixsSQLStatement<QueryGetPaxTypeMatrixs> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETPAXTYPEMATRIXS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetPaxTypeMatrixs::findAllPaxTypeMatrix(std::vector<const tse::PaxTypeMatrix*>& paxTypeMatrixs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    paxTypeMatrixs.push_back(
        QueryGetPaxTypeMatrixsSQLStatement<QueryGetPaxTypeMatrixs>::mapRowToPaxTypeMatrix(row));
  }
  LOG4CXX_INFO(_logger,
               "GETPAXTYPEMATRIXS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // findAllPaxTypeMatrix()
}
