//----------------------------------------------------------------------------
//
//  File:           QueryGetBrandedFareApp.cpp
//  Description:    QueryGetBrandedFareApp
//  Created:        1/11/2007
//  Authors:        Mike Lillis / Marco Cartolano
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetBrandedFareApp.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBrandedFareAppSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetBrandedFareApp::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetBrandedFareApp"));
std::string QueryGetBrandedFareApp::_baseSQL;
bool QueryGetBrandedFareApp::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetBrandedFareApp> g_GetBrandedFareApp;

const char*
QueryGetBrandedFareApp::getQueryName() const
{
  return "GETBRANDEDFAREAPP";
}

void
QueryGetBrandedFareApp::initialize()
{
  if (!_isInitialized)
  {
    QueryGetBrandedFareAppSQLStatement<QueryGetBrandedFareApp> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETBRANDEDFAREAPP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetBrandedFareApp::findBrandedFareApp(std::vector<tse::BrandedFareApp*>& infos,
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
    infos.push_back(
        QueryGetBrandedFareAppSQLStatement<QueryGetBrandedFareApp>::mapRowToBrandedFareApp(row));
  }
  LOG4CXX_INFO(_logger,
               "GETBRANDEDFAREAPP: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
} // findBrandedFareApp()
}
