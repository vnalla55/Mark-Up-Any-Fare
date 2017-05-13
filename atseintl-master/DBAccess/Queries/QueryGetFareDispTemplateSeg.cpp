//----------------------------------------------------------------------------
//  File:           QueryGetFareDispTemplateSeg.cpp
//  Description:    QueryGetFareDispTemplateSeg
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
#include "DBAccess/Queries/QueryGetFareDispTemplateSeg.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispTemplateSegSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispTemplateSeg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispTemplateSeg"));
std::string QueryGetFareDispTemplateSeg::_baseSQL;
bool QueryGetFareDispTemplateSeg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispTemplateSeg> g_GetFareDispTemplateSeg;

const char*
QueryGetFareDispTemplateSeg::getQueryName() const
{
  return "GETFAREDISPTEMPLATESEG";
}

void
QueryGetFareDispTemplateSeg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispTemplateSegSQLStatement<QueryGetFareDispTemplateSeg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPTEMPLATESEG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispTemplateSeg::findFareDispTemplateSeg(std::vector<tse::FareDispTemplateSeg*>& infos,
                                                     const int& templateID,
                                                     const Indicator& templateType)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  // since tmplateID is varchar
  char strTemplateID[16];
  snprintf(strTemplateID, sizeof(strTemplateID), "%d", templateID);
  substParm(strTemplateID, 1);
  substParm(2, templateType);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetFareDispTemplateSegSQLStatement<
        QueryGetFareDispTemplateSeg>::mapRowToFareDispTemplateSeg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPTEMPLATESEG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispTemplateSeg()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispTemplateSeg()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispTemplateSeg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispTemplateSeg"));
std::string QueryGetAllFareDispTemplateSeg::_baseSQL;
bool QueryGetAllFareDispTemplateSeg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispTemplateSeg> g_GetAllFareDispTemplateSeg;

const char*
QueryGetAllFareDispTemplateSeg::getQueryName() const
{
  return "GETALLFAREDISPTEMPLATESEG";
}

void
QueryGetAllFareDispTemplateSeg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispTemplateSegSQLStatement<QueryGetAllFareDispTemplateSeg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPTEMPLATESEG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispTemplateSeg::findAllFareDispTemplateSeg(
    std::vector<tse::FareDispTemplateSeg*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFareDispTemplateSegSQLStatement<
        QueryGetAllFareDispTemplateSeg>::mapRowToFareDispTemplateSeg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPTEMPLATESEG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispTemplateSeg()
}
