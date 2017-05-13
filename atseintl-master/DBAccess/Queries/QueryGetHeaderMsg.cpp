//----------------------------------------------------------------------------
//
//  File:           QueryGetHeaderMsg.cpp
//  Description:    QueryGetHeaderMsg
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
//  ----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetHeaderMsg.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHeaderMsgSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetHeaderMsg::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetHeaderMsg"));
std::string QueryGetHeaderMsg::_baseSQL;
bool QueryGetHeaderMsg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetHeaderMsg> g_GetHeaderMsg;

const char*
QueryGetHeaderMsg::getQueryName() const
{
  return "GETHEADERMSG";
}

void
QueryGetHeaderMsg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetHeaderMsgSQLStatement<QueryGetHeaderMsg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETHEADERMSG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetHeaderMsg::findHeaderMsgs(std::vector<const tse::FDHeaderMsg*>& lstSF,
                                       const PseudoCityCode& pseudoCityCode,
                                       const PseudoCityType& pseudoCityType,
                                       const Indicator& userApplType,
                                       const std::string& userAppl,
                                       const TJRGroup& tjrGroup,
                                       const InclusionCode& inclusionCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(pseudoCityCode.substr(0, 4), 1);
  substParm(2, pseudoCityType);
  substParm(3, userApplType);
  substParm(userAppl, 4);
  substCurrentDate();
  substParm(5, tjrGroup);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    lstSF.push_back(QueryGetHeaderMsgSQLStatement<QueryGetHeaderMsg>::mapRowToFDHeaderMsg(row));
  }
  LOG4CXX_INFO(_logger,
               "GETHEADERMSG: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                          << stopCPU() << ") mSecs");
  res.freeResult();
} // findHeaderMsgs()
}
