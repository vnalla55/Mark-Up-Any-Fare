//----------------------------------------------------------------------------
//  File:           QueryGetMarketCarriers.cpp
//  Description:    QueryGetMarketCarriers
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// (C) 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMarketCarriers.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarketCarriersSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMarketCarriers::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarketCarriers"));
std::string QueryGetMarketCarriers::_baseSQL;
bool QueryGetMarketCarriers::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarketCarriers> g_GetMarketCarriers;

const char*
QueryGetMarketCarriers::getQueryName() const
{
  return "GETMARKETCARRIERS";
}

void
QueryGetMarketCarriers::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarketCarriersSQLStatement<QueryGetMarketCarriers> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARKETCARRIERS");
    substTableDef(&_baseSQL);

    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarketCarriers::findMarketCarriers(std::vector<const tse::MarketCarrier*>& lstMC,
                                           const LocCode& market1,
                                           const LocCode& market2)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(market1, 1);
  substParm(market2, 2);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    MarketCarrier* mc =
        QueryGetMarketCarriersSQLStatement<QueryGetMarketCarriers>::mapRowToMarketCarrier(row);
    if (mc)
      lstMC.push_back(mc);
  }
  LOG4CXX_INFO(_logger,
               "GETMARKETCARRIERS: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();

} // QueryGetMarketCarriers::getMarketCarriers()
}
