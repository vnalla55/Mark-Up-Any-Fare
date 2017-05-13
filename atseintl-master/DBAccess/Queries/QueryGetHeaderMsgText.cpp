//----------------------------------------------------------------------------
//
//  File:           QueryGetHeaderMsgText.cpp
//  Description:    QueryGetHeaderMsgText
//  Created:        5/24/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
//  2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetHeaderMsgText.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHeaderMsgTextSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetHeaderMsgText::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetHeaderMsgText"));
std::string QueryGetHeaderMsgText::_baseSQL;
bool QueryGetHeaderMsgText::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetHeaderMsgText> g_GetHeaderMsgText;

const char*
QueryGetHeaderMsgText::getQueryName() const
{
  return "GETHEADERMSGTEXT";
}

void
QueryGetHeaderMsgText::initialize()
{
  if (!_isInitialized)
  {
    QueryGetHeaderMsgTextSQLStatement<QueryGetHeaderMsgText> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETHEADERMSGTEXT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetHeaderMsgText::findHeaderMsgText(std::vector<const std::string*>& lstSF,
                                              const uint64_t& itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strItemNo[12];
  sprintf(strItemNo, "%lu", itemNo);
  substParm(strItemNo, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstSF.push_back(
        QueryGetHeaderMsgTextSQLStatement<QueryGetHeaderMsgText>::mapRowToFDHeaderMsgText(row));
  }
  LOG4CXX_INFO(_logger,
               "GETHEADERMSGTEXT: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
} // findHeaderMsgText()
}
