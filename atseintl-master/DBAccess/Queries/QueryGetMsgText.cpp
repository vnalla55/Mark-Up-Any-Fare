//----------------------------------------------------------------------------
//
//    File:           QueryGetMsgText.cpp
//    Description:    QueryGetMsgText
//    Created:        5/24/2006
//    Authors:        Mike Lillis
//
//    Updates:
//      07/26/2006  Quan Ta
//
//    2006, Sabre Inc.  All rights reserved.  This software/documentation is
//    the confidential and proprietary product of Sabre Inc. Any unauthorized
//    use, reproduction, or transfer of this software/documentation, in any
//    medium, or incorporation of this software/documentation into any system
//    or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetMsgText.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMsgTextSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMsgText::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMsgText"));
std::string QueryGetMsgText::_baseSQL;
bool QueryGetMsgText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMsgText> g_GetMsgText;

const char*
QueryGetMsgText::getQueryName() const
{
  return "GETMSGTEXT";
}

void
QueryGetMsgText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMsgTextSQLStatement<QueryGetMsgText> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMSGTEXT");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetMsgText::findMsgText(FareCalcConfigText::FCCTextMap& fccTextMap,
                                  const Indicator userApplType,
                                  const UserApplCode& userAppl,
                                  const PseudoCityCode& pseudoCity)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(userAppl, 2);
  substParm(pseudoCity, 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    QueryGetMsgTextSQLStatement<QueryGetMsgText>::mapRowToMsgText(fccTextMap, row);
  }
  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ") mSecs");
}
}
