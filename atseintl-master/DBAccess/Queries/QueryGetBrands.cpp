//----------------------------------------------------------------------------
//  File:           QueryGetBrands.cpp
//  Description:    QueryGetBrands
//  Created:        1/10/2007
//  Authors:        Mike Lillis / Marco Cartolano
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBrands.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBrandsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBrands::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBrands"));
std::string QueryGetBrands::_baseSQL;
bool QueryGetBrands::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBrands> g_GetBrands;

const char*
QueryGetBrands::getQueryName() const
{
  return "GETBRANDS";
}

void
QueryGetBrands::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBrandsSQLStatement<QueryGetBrands> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBRANDS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBrands::findBrands(std::vector<tse::Brand*>& infos,
                           const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetBrandsSQLStatement<QueryGetBrands>::mapRowToBrand(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBRANDS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ")");
  res.freeResult();
} // findBrands()

///////////////////////////////////////////////////////////
//
//  QueryGetBrandsHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetBrandsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBrandsHistorical"));
std::string QueryGetBrandsHistorical::_baseSQL;
bool QueryGetBrandsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBrandsHistorical> g_GetBrandsHistorical;

const char*
QueryGetBrandsHistorical::getQueryName() const
{
  return "GETBRANDSHISTORICAL";
}

void
QueryGetBrandsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBrandsHistoricalSQLStatement<QueryGetBrandsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBRANDSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBrandsHistorical::findBrands(std::vector<tse::Brand*>& infos,
                                     const Indicator& userApplType,
                                     const UserApplCode& userAppl,
                                     const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetBrandsHistoricalSQLStatement<QueryGetBrandsHistorical>::mapRowToBrand(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBRANDSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findBrands()

///////////////////////////////////////////////////////////
//
//  QueryGetAllBrands()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBrands::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllBrands"));
std::string QueryGetAllBrands::_baseSQL;
bool QueryGetAllBrands::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBrands> g_GetAllBrands;

const char*
QueryGetAllBrands::getQueryName() const
{
  return "GETALLBRANDS";
}
void
QueryGetAllBrands::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBrandsSQLStatement<QueryGetAllBrands> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLBRANDS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBrands::findAllBrands(std::vector<tse::Brand*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllBrandsSQLStatement<QueryGetAllBrands>::mapRowToBrand(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLBRANDS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ")");
  res.freeResult();
} // findAllBrands()

///////////////////////////////////////////////////////////
//
//  QueryGetAllBrandsHistorical()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllBrandsHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllBrandsHistorical"));
std::string QueryGetAllBrandsHistorical::_baseSQL;
bool QueryGetAllBrandsHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBrandsHistorical> g_GetAllBrandsHistorical;

const char*
QueryGetAllBrandsHistorical::getQueryName() const
{
  return "GETALLBRANDSHISTORICAL";
}

void
QueryGetAllBrandsHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBrandsHistoricalSQLStatement<QueryGetAllBrandsHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLBRANDSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBrandsHistorical::findAllBrands(std::vector<tse::Brand*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllBrandsHistoricalSQLStatement<QueryGetAllBrandsHistorical>::mapRowToBrand(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLBRANDSHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllBrands()
}
