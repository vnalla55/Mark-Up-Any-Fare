//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetFrequentFlyerStatusSeg.h"

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFrequentFlyerStatusSegSQLStatement.h"

namespace tse
{
namespace
{
struct LowerStatusPredicate
{
  bool operator()(const FreqFlyerStatusSeg* first, const FreqFlyerStatusSeg* second) const
  {
    if (first->createDate() != second->createDate())
      return first->createDate() < second->createDate();
    if (first->effDate() != second->effDate())
      return first->effDate() < second->effDate();
    if (first->expireDate() != second->expireDate())
      return first->expireDate() < second->expireDate();

    return first->discDate() < second->discDate();
  }
};

template <typename StatementT>
std::set<FreqFlyerStatusSeg*, LowerStatusPredicate>
parseRows(DBResultSet& res)
{
  Row* row;
  std::set<FreqFlyerStatusSeg*, LowerStatusPredicate> parsedRows;

  while ((row = res.nextRow()))
  {
    std::unique_ptr<FreqFlyerTierStatusRow> statusRow(StatementT::mapRowToStatus(row));

    FreqFlyerStatusSeg* status = new FreqFlyerStatusSeg;
    status->setCarrier(statusRow->_carrier);
    status->setCreateDate(statusRow->_createDate);
    status->setEffectiveDate(statusRow->_effectiveDate);
    status->setExpireDate(statusRow->_expireDate);
    status->setDiscDate(statusRow->_discDate);

    auto insertResult = parsedRows.insert(status);
    if (!insertResult.second)
      delete status;

    (*insertResult.first)
        ->addPartnerLevel(statusRow->_partnerCarrier, statusRow->_partnerLevel, statusRow->_level);
  }
  for (auto status : parsedRows)
    status->sortPartnerStatusMap();
  return parsedRows;
}
}

static const char* queryName = "GETFREQUENTFLYERSTATUSSEG";
static const char* queryNameHistorical = "GETFREQUENTFLYERSTATUSSEGHISTORICAL";
Logger
QueryGetFrequentFlyerStatusSeg::_logger("atseintl.DBAccess.SQLQuery.GetFrequentFlyerStatusSeg");
std::string QueryGetFrequentFlyerStatusSeg::_baseSQL;
bool QueryGetFrequentFlyerStatusSeg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFrequentFlyerStatusSeg> g_GetFrequentFlyerStatusSeg;

const char*
QueryGetFrequentFlyerStatusSeg::getQueryName() const
{
  return queryName;
}

void
QueryGetFrequentFlyerStatusSeg::findTierStatus(const CarrierCode carrier,
                                               std::vector<FreqFlyerStatusSeg*>& statuses)
{
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);
  LOG4CXX_INFO(_logger,
               "GETFREQFLYERSTATUSSEG: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  auto parsedRows =
      parseRows<QueryGetFrequentFlyerStatusSegSQLStatement<QueryGetFrequentFlyerStatusSeg>>(res);
  res.freeResult();
  for (auto status : parsedRows)
    statuses.push_back(status);
}

void
QueryGetFrequentFlyerStatusSeg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFrequentFlyerStatusSegSQLStatement<QueryGetFrequentFlyerStatusSeg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL(queryName);
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

Logger
QueryGetFrequentFlyerStatusSegHistorical::_logger(
    "atseintl.DBAccess.SQLQuery.GetFrequentFlyerStatusSegHistorical");
std::string QueryGetFrequentFlyerStatusSegHistorical::_baseSQL;
bool QueryGetFrequentFlyerStatusSegHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFrequentFlyerStatusSegHistorical>
g_GetFrequentFlyerStatusSegHistorical;

const char*
QueryGetFrequentFlyerStatusSegHistorical::getQueryName() const
{
  return queryNameHistorical;
}

void
QueryGetFrequentFlyerStatusSegHistorical::findTierStatusSegs(
    const CarrierCode carrier,
    const DateTime& startDate,
    const DateTime& endDate,
    std::vector<FreqFlyerStatusSeg*>& statuses)
{
  DBResultSet res(_dbAdapt);

  substParm(carrier, 1);
  substParm(2, startDate);
  substParm(3, endDate);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  LOG4CXX_INFO(_logger,
               "GETFREQFLYERSTATUSSEGHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  auto parsedRows = parseRows<QueryGetFrequentFlyerStatusSegHistoricalSQLStatement<
      QueryGetFrequentFlyerStatusSegHistorical>>(res);
  res.freeResult();
  for (auto status : parsedRows)
    statuses.push_back(status);
}

void
QueryGetFrequentFlyerStatusSegHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFrequentFlyerStatusSegHistoricalSQLStatement<QueryGetFrequentFlyerStatusSegHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL(queryNameHistorical);
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}
}
