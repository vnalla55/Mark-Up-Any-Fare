//----------------------------------------------------------------------------
//  File:           QueryGetNegFareRestExtSeq.cpp
//  Description:    QueryGetNegFareRestExtSeq
//  Created:        9/9/2010
// Authors:         Artur Krezel
//
//  Updates:
//
// � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNegFareRestExtSeq.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareRestExtSeqSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNegFareRestExtSeq::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNegFareRestExtSeq"));
std::string QueryGetNegFareRestExtSeq::_baseSQL;
bool QueryGetNegFareRestExtSeq::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRestExtSeq> g_GetNegFareRestExtSeq;

const char*
QueryGetNegFareRestExtSeq::getQueryName() const
{
  return "GETNEGFARERESTEXTSEQ";
}

void
QueryGetNegFareRestExtSeq::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestExtSeqSQLStatement<QueryGetNegFareRestExtSeq> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARERESTEXTSEQ");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRestExtSeq::findNegFareRestExtSeq(
    std::vector<tse::NegFareRestExtSeq*>& negFareRestExtSeqs, const VendorCode& vendor, int itemNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    negFareRestExtSeqs.push_back(
        QueryGetNegFareRestExtSeqSQLStatement<QueryGetNegFareRestExtSeq>::mapRowToNegFareRestExtSeq(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARERESTEXTSEQ: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareRestExtSeq()

///////////////////////////////////////////////////////////
//  QueryGetNegFareRestExtSeqHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNegFareRestExtSeqHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetNegFareRestExtSeqHistorical"));
std::string QueryGetNegFareRestExtSeqHistorical::_baseSQL;
bool QueryGetNegFareRestExtSeqHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNegFareRestExtSeqHistorical> g_GetNegFareRestExtSeqHistorical;

const char*
QueryGetNegFareRestExtSeqHistorical::getQueryName() const
{
  return "GETNEGFARERESTEXTSEQHISTORICAL";
}

void
QueryGetNegFareRestExtSeqHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNegFareRestExtSeqHistoricalSQLStatement<QueryGetNegFareRestExtSeqHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNEGFARERESTEXTSEQHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetNegFareRestExtSeqHistorical::findNegFareRestExtSeq(
    std::vector<tse::NegFareRestExtSeq*>& negFareRestExtSeqs,
    const VendorCode& vendor,
    int itemNo,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char itemStr[15];
  sprintf(itemStr, "%d", itemNo);

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(vendor, 5);
  substParm(itemStr, 6);
  substParm(7, startDate);
  substParm(8, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    negFareRestExtSeqs.push_back(QueryGetNegFareRestExtSeqHistoricalSQLStatement<
        QueryGetNegFareRestExtSeqHistorical>::mapRowToNegFareRestExtSeq(row));
  }
  LOG4CXX_INFO(_logger,
               "GETNEGFARERESTEXTSEQHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findNegFareRestExtSeq()
}
