#include "DBAccess/Queries/QueryGetTaxSpecConfig.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxSpecConfigSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTaxSpecConfig::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTaxSpecConfig"));
std::string QueryGetTaxSpecConfig::_baseSQL;

log4cxx::LoggerPtr
QueryGetAllTaxSpecConfigRegs::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetAllTaxSpecConfigRegs"));
std::string QueryGetAllTaxSpecConfigRegs::_baseSQL;

log4cxx::LoggerPtr
QueryGetTaxSpecConfigHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetTaxSpecConfigHistorical"));
std::string QueryGetTaxSpecConfigHistorical::_baseSQL;

bool QueryGetTaxSpecConfig::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxSpecConfig> g_GetTaxSpecConfig;

const char*
QueryGetTaxSpecConfig::getQueryName() const
{
  return "GETTAXSPECCONFIG";
};

void
QueryGetTaxSpecConfig::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxSpecConfigSQLStatement<QueryGetTaxSpecConfig> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXSPECCONFIG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxSpecConfig::findTaxSpecConfigReg(std::vector<tse::TaxSpecConfigReg*>& taxC,
                                            const TaxSpecConfigName& configName)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, configName);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxSpecConfigReg* tcr = nullptr;
  tse::TaxSpecConfigReg* tcrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tcrPrev = tcr;
    tcr = QueryGetTaxSpecConfigSQLStatement<QueryGetTaxSpecConfig>::mapRowToTaxSpecConfigReg(
        row, tcrPrev);
    if (tcr == tcrPrev)
      continue;

    taxC.push_back(tcr);
  }
  LOG4CXX_INFO(_logger,
               "GETTAXSPECCONF: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllTaxSpecConfigRegs
//
///////////////////////////////////////////////////////////
bool QueryGetAllTaxSpecConfigRegs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTaxSpecConfigRegs> g_GetAllTaxSpecConfigRegs;

const char*
QueryGetAllTaxSpecConfigRegs::getQueryName() const
{
  return "GETALLTAXSPECCONFIGREGS";
};

void
QueryGetAllTaxSpecConfigRegs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTaxSpecConfigRegsSQLStatement<QueryGetAllTaxSpecConfigRegs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTAXSPECCONFIGREGS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllTaxSpecConfigRegs::findAllTaxSpecConfigReg(
    std::vector<tse::TaxSpecConfigReg*>& vecTaxSpecConfigReg)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::TaxSpecConfigReg* tcr = nullptr;
  tse::TaxSpecConfigReg* tcrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tcrPrev = tcr;
    tcr = QueryGetAllTaxSpecConfigRegsSQLStatement<
        QueryGetAllTaxSpecConfigRegs>::mapRowToTaxSpecConfigReg(row, tcrPrev);
    if (tcr == tcrPrev)
      continue;

    vecTaxSpecConfigReg.push_back(tcr);
  }
  LOG4CXX_INFO(_logger,
               "GETALLTAXSPECCONFIGREGS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
} // findAllTaxSpecConfigReg()

///////////////////////////////////////////////////////////
//
//  QueryGetTaxSpecConfigHistorical
//
///////////////////////////////////////////////////////////

bool QueryGetTaxSpecConfigHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTaxSpecConfigHistorical> g_GetTaxSpecConfigHistorical;

const char*
QueryGetTaxSpecConfigHistorical::getQueryName() const
{
  return "GETTAXSPECCONFHISTORICAL";
};

void
QueryGetTaxSpecConfigHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTaxSpecConfigHistoricalSQLStatement<QueryGetTaxSpecConfigHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTAXSPECCONFHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTaxSpecConfigHistorical::findTaxSpecConfigRegHistorical(
    std::vector<tse::TaxSpecConfigReg*>& taxC,
    const TaxSpecConfigName& name,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, name);
  substParm(2, startDate);
  substParm(3, endDate);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());

  res.executeQuery(this);

  tse::TaxSpecConfigReg* tcr = nullptr;
  tse::TaxSpecConfigReg* tcrPrev = nullptr;
  while ((row = res.nextRow()))
  {
    tcrPrev = tcr;
    tcr = QueryGetTaxSpecConfigHistoricalSQLStatement<
        QueryGetTaxSpecConfigHistorical>::mapRowToTaxSpecConfigReg(row, tcrPrev);
    if (tcr == tcrPrev)
      continue;

    taxC.push_back(tcr);
  }
  LOG4CXX_INFO(_logger,
               "GETTAXSPECCONFIGHISTORICAL: NumRows = " << res.numRows() << " Time = "
                                                        << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();

} // QueryGetTaxSpecConfig::findTaxSpecConfigReg()
}
