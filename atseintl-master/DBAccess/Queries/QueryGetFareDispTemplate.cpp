//----------------------------------------------------------------------------
//  File:           QueryGetFareDispTemplate.cpp
//  Description:    QueryGetFareDispTemplate
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
#include "DBAccess/Queries/QueryGetFareDispTemplate.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispTemplateSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareDispTemplate::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareDispTemplate"));
std::string QueryGetFareDispTemplate::_baseSQL;
bool QueryGetFareDispTemplate::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareDispTemplate> g_GetFareDispTemplate;

const char*
QueryGetFareDispTemplate::getQueryName() const
{
  return "GETFAREDISPTEMPLATE";
}

void
QueryGetFareDispTemplate::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareDispTemplateSQLStatement<QueryGetFareDispTemplate> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFAREDISPTEMPLATE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFareDispTemplate::findFareDispTemplate(std::vector<tse::FareDispTemplate*>& infos,
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
    infos.push_back(
        QueryGetFareDispTemplateSQLStatement<QueryGetFareDispTemplate>::mapRowToFareDispTemplate(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETFAREDISPTEMPLATE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // findFareDispTemplate()

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareDispTemplate()
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareDispTemplate::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareDispTemplate"));
std::string QueryGetAllFareDispTemplate::_baseSQL;
bool QueryGetAllFareDispTemplate::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareDispTemplate> g_GetAllFareDispTemplate;

const char*
QueryGetAllFareDispTemplate::getQueryName() const
{
  return "GETALLFAREDISPTEMPLATE";
}

void
QueryGetAllFareDispTemplate::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareDispTemplateSQLStatement<QueryGetAllFareDispTemplate> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFAREDISPTEMPLATE");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFareDispTemplate::findAllFareDispTemplate(std::vector<tse::FareDispTemplate*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(QueryGetAllFareDispTemplateSQLStatement<
        QueryGetAllFareDispTemplate>::mapRowToFareDispTemplate(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLFAREDISPTEMPLATE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllFareDispTemplate()
}
