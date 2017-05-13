//----------------------------------------------------------------------------
//  File:           QueryGetVendXref.cpp
//  Description:    QueryGetVendXref
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

#include "DBAccess/Queries/QueryGetVendXref.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVendXrefSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetVendXref::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetVendXref"));
std::string QueryGetVendXref::_baseSQL;
bool QueryGetVendXref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetVendXref> g_GetVendXref;

const char*
QueryGetVendXref::getQueryName() const
{
  return "GETVENDXREF";
}

void
QueryGetVendXref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetVendXrefSQLStatement<QueryGetVendXref> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETVENDXREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetVendXref::findVendorCrossRef(std::vector<tse::VendorCrossRef*>& vxrs,
                                     const VendorCode& vendor)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(vendor, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    vxrs.push_back(QueryGetVendXrefSQLStatement<QueryGetVendXref>::mapRowToVendorCrossRef(row));
  }
  LOG4CXX_INFO(_logger,
               "GETVENDXREF: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
} // findVendorCrossRef()

///////////////////////////////////////////////////////////
//
//  QueryGetAllVendXref
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllVendXref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllVendXref"));
std::string QueryGetAllVendXref::_baseSQL;
bool QueryGetAllVendXref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllVendXref> g_GetAllVendXref;

const char*
QueryGetAllVendXref::getQueryName() const
{
  return "GETALLVENDXREF";
}

void
QueryGetAllVendXref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllVendXrefSQLStatement<QueryGetAllVendXref> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLVENDXREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllVendXref::findAllVendorCrossRef(std::vector<tse::VendorCrossRef*>& vxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    vxr.push_back(
        QueryGetAllVendXrefSQLStatement<QueryGetAllVendXref>::mapRowToVendorCrossRef(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLVENDXREF: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findAllVendorCrossRef()
}
