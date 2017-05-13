//----------------------------------------------------------------------------
//  File:           QueryGetFDSFareBasisComb.cpp
//  Description:    QueryGetFDSFareBasisComb
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
#include "DBAccess/Queries/QueryGetFDSFareBasisComb.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFDSFareBasisCombSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFDSFareBasisComb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFDSFareBasisComb"));
std::string QueryGetFDSFareBasisComb::_baseSQL;
bool QueryGetFDSFareBasisComb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFDSFareBasisComb> g_GetFDSFareBasisComb;

const char*
QueryGetFDSFareBasisComb::getQueryName() const
{
  return "GETFDSFAREBASISCOMB";
}

void
QueryGetFDSFareBasisComb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFDSFareBasisCombSQLStatement<QueryGetFDSFareBasisComb> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFDSFAREBASISCOMB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFDSFareBasisComb::findFDSFareBasisComb(std::vector<tse::FDSFareBasisComb*>& infos,
                                               const Indicator& userApplType,
                                               const UserApplCode& userAppl,
                                               const Indicator& pseudoCityType,
                                               const PseudoCityCode& pseudoCity,
                                               const TJRGroup& tjrGroup)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, pseudoCityType);
  substParm(4, pseudoCity);
  substParm(5, tjrGroup);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetFDSFareBasisCombSQLStatement<QueryGetFDSFareBasisComb>::mapRowToFDSFareBasisComb(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETFDSFAREBASISCOMB: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findFDSFareBasisComb()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFDSFareBasisComb()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFDSFareBasisComb::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFDSFareBasisComb"));
std::string QueryGetAllFDSFareBasisComb::_baseSQL;
bool QueryGetAllFDSFareBasisComb::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFDSFareBasisComb> g_GetAllFDSFareBasisComb;

const char*
QueryGetAllFDSFareBasisComb::getQueryName() const
{
  return "GETALLFDSFAREBASISCOMB";
}

void
QueryGetAllFDSFareBasisComb::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFDSFareBasisCombSQLStatement<QueryGetAllFDSFareBasisComb> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFDSFAREBASISCOMB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFDSFareBasisComb::findAllFDSFareBasisComb(std::vector<tse::FDSFareBasisComb*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFDSFareBasisCombSQLStatement<
        QueryGetAllFDSFareBasisComb>::mapRowToFDSFareBasisComb(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFDSFAREBASISCOMB: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFDSFareBasisComb()
}
