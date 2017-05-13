//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMarkup.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarkupSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMarkup::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarkup"));
std::string QueryGetMarkup::_baseSQL;
bool QueryGetMarkup::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarkup> g_GetMarkup;

const char*
QueryGetMarkup::getQueryName() const
{
  return "GETMARKUP";
}

void
QueryGetMarkup::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarkupSQLStatement<QueryGetMarkup> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARKUP");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarkup::findMarkupControl(std::vector<tse::MarkupControl*>& lstMC,
                                  VendorCode& vendor,
                                  CarrierCode& carrier,
                                  TariffNumber ruleTariff,
                                  RuleNumber& rule,
                                  int seqNo,
                                  PseudoCityCode& pcc)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, ruleTariff);
  substParm(4, rule);
  substParm(5, seqNo);
  substParm(6, pcc);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MarkupControl* mc = nullptr;
  tse::MarkupControl* mcPrev = nullptr;
  while ((row = res.nextRow()))
  {
    mc = QueryGetMarkupSQLStatement<QueryGetMarkup>::mapRowToMarkupControl(row, mcPrev);
    if (mc != mcPrev)
      lstMC.push_back(mc);

    mcPrev = mc;
  }
  LOG4CXX_INFO(_logger,
               "GETMARKUP: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                     << stopCPU() << ") mSecs");
  res.freeResult();
} // findMarkupControl()

///////////////////////////////////////////////////////////
//  QueryGetMarkupHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMarkupHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMarkupHistorical"));
std::string QueryGetMarkupHistorical::_baseSQL;
bool QueryGetMarkupHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMarkupHistorical> g_GetMarkupHistorical;

const char*
QueryGetMarkupHistorical::getQueryName() const
{
  return "GETMARKUPHISTORICAL";
}

void
QueryGetMarkupHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMarkupHistoricalSQLStatement<QueryGetMarkupHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMARKUPHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMarkupHistorical::findMarkupControl(std::vector<tse::MarkupControl*>& lstMC,
                                            VendorCode& vendor,
                                            CarrierCode& carrier,
                                            TariffNumber ruleTariff,
                                            RuleNumber& rule,
                                            int seqNo,
                                            PseudoCityCode& pcc,
                                            const DateTime& startDate,
                                            const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, ruleTariff);
  substParm(4, rule);
  substParm(5, seqNo);
  substParm(6, pcc);
  substParm(7, startDate);
  substParm(8, endDate);
  substParm(9, vendor);
  substParm(10, carrier);
  substParm(11, ruleTariff);
  substParm(12, rule);
  substParm(13, seqNo);
  substParm(14, pcc);
  substParm(15, startDate);
  substParm(16, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MarkupControl* mc = nullptr;
  tse::MarkupControl* mcPrev = nullptr;
  while ((row = res.nextRow()))
  {
    mc = QueryGetMarkupHistoricalSQLStatement<QueryGetMarkupHistorical>::mapRowToMarkupControl(
        row, mcPrev);
    if (mc != mcPrev)
      lstMC.push_back(mc);

    mcPrev = mc;
  }
  LOG4CXX_INFO(_logger,
               "GETMARKUPHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
} // findMarkupControl()
}
