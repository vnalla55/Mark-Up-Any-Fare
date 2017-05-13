//----------------------------------------------------------------------------
//  File:           QueryGetCombCtrl.cpp
//  Description:    QueryGetCombCtrl
//  Created:        5/24/2006
//  Authors:        Mike Lillis
//                  Simon Li
//  Updates:
//
// � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetCombCtrl.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCombCtrlSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCombCtrl::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCombCtrl"));
std::string QueryGetCombCtrl::_baseSQL;
bool QueryGetCombCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCombCtrl> g_GetCombCtrl;

const char*
QueryGetCombCtrl::getQueryName() const
{
  return "GETCOMBCTRL";
}

void
QueryGetCombCtrl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCombCtrlSQLStatement<QueryGetCombCtrl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMBCTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCombCtrl::findCombinabilityRule(std::vector<tse::CombinabilityRuleInfo*>& combs,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        int ruleTariff,
                                        const RuleNumber& ruleNo,
                                        int cat)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  uint16_t dataSetNumber = 0;

  char ruleTarStr[15];
  char catStr[4];

  sprintf(ruleTarStr, "%d", ruleTariff);
  sprintf(catStr, "%d", cat);

  std::map<int, tse::ScoreSummary> scoreSum;

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substParm(catStr, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CombinabilityRuleInfo* comb = nullptr;
  tse::CombinabilityRuleInfo* combPrev = nullptr;
  while ((row = res.nextRow()))
  {
    comb = QueryGetCombCtrlSQLStatement<QueryGetCombCtrl>::mapRowToCombination(
        row, combPrev, scoreSum, dataSetNumber);
    if (comb != combPrev)
    {
      combs.push_back(comb);
    }
    combPrev = comb;
  }
  if (comb != nullptr)
  {
    QueryGetCombCtrlSQLStatement<QueryGetCombCtrl>::mapScoreSumToComb(
        comb, scoreSum, dataSetNumber);
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBCTRL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                         << stopCPU() << ")");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::CombinabilityRuleInfo* i){i->sync_with_cache();});
} // findCombinabilityRule()

///////////////////////////////////////////////////////////
//  QueryGetCombCtrlHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCombCtrlHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCombCtrlHistorical"));
std::string QueryGetCombCtrlHistorical::_baseSQL;
bool QueryGetCombCtrlHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCombCtrlHistorical> g_GetCombCtrlHistorical;

const char*
QueryGetCombCtrlHistorical::getQueryName() const
{
  return "GETCOMBCTRLHISTORICAL";
}

void
QueryGetCombCtrlHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCombCtrlHistoricalSQLStatement<QueryGetCombCtrlHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCOMBCTRLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCombCtrlHistorical::findCombinabilityRule(std::vector<tse::CombinabilityRuleInfo*>& combs,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  int ruleTariff,
                                                  const RuleNumber& ruleNo,
                                                  int cat,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  uint16_t dataSetNumber = 0;

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", ruleTariff);

  char catStr[4];
  sprintf(catStr, "%d", cat);

  std::map<int, tse::ScoreSummary> scoreSum;

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substParm(catStr, 5);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(vendor, 8);
  substParm(carrier, 9);
  substParm(ruleTarStr, 10);
  substParm(ruleNo, 11);
  substParm(catStr, 12);
  substParm(13, startDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CombinabilityRuleInfo* comb = nullptr;
  tse::CombinabilityRuleInfo* combPrev = nullptr;
  while ((row = res.nextRow()))
  {
    comb = QueryGetCombCtrlHistoricalSQLStatement<QueryGetCombCtrlHistorical>::mapRowToCombination(
        row, combPrev, scoreSum, dataSetNumber);
    if (comb != combPrev)
    {
      combs.push_back(comb);
    }
    combPrev = comb;
  }
  if (comb != nullptr)
  {
    QueryGetCombCtrlHistoricalSQLStatement<QueryGetCombCtrlHistorical>::mapScoreSumToComb(
        comb, scoreSum, dataSetNumber);
  }
  LOG4CXX_INFO(_logger,
               "GETCOMBCTRLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::CombinabilityRuleInfo* i){i->sync_with_cache();});
} // findCombinabilityRule()
}
