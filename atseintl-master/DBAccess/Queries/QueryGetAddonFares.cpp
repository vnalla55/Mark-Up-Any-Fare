//----------------------------------------------------------------------------
//  File:           QueryGetAddonFares.cpp
//  Description:    QueryGetAddonFares
//  Created:        8/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetAddonFares.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAddonFaresSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAddonFares::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFares"));
std::string QueryGetAddonFares::_baseSQL;
bool QueryGetAddonFares::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonFares> g_GetAddonFares;

log4cxx::LoggerPtr
QueryGetSitaAddonQualCodes::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonQualCodes"));
std::string QueryGetSitaAddonQualCodes::_baseSQL;
bool QueryGetSitaAddonQualCodes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonQualCodes> g_GetSitaAddonQualCodes;

log4cxx::LoggerPtr
QueryGetSitaAddonDBEClasses::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonDBEClasses"));
std::string QueryGetSitaAddonDBEClasses::_baseSQL;
bool QueryGetSitaAddonDBEClasses::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonDBEClasses> g_GetSitaAddonDBEClasses;

log4cxx::LoggerPtr
QueryGetSitaAddonRules::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonRules"));
std::string QueryGetSitaAddonRules::_baseSQL;
bool QueryGetSitaAddonRules::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonRules> g_GetSitaAddonRules;

const char*
QueryGetAddonFares::getQueryName() const
{
  return "GETADDONFARES";
};

void
QueryGetAddonFares::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonFaresSQLStatement<QueryGetAddonFares> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONFARES");
    substTableDef(&_baseSQL);

    QueryGetSitaAddonQualCodes::initialize();
    QueryGetSitaAddonDBEClasses::initialize();
    QueryGetSitaAddonRules::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonFares::findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                                      const LocCode& interiorMarket,
                                      const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(interiorMarket, 1);
  substParm(carrier, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAOF.push_back(
        QueryGetAddonFaresSQLStatement<QueryGetAddonFares>::mapRowToAddonFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONFARES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSitaAddonQualCodes SQLQualCodes(_dbAdapt);
  QueryGetSitaAddonDBEClasses SQLDBEClasses(_dbAdapt);
  QueryGetSitaAddonRules SQLRules(_dbAdapt);
  std::vector<AddonFareInfo*>::iterator AOFIt;
  for (AOFIt = lstAOF.begin(); AOFIt != lstAOF.end(); AOFIt++)
  {
    if ((*AOFIt)->vendor() != "SITA")
      continue; // No Kids for this one

    SITAAddonFareInfo* pSITA = (SITAAddonFareInfo*)*AOFIt;

    SQLQualCodes.getQualCodes(pSITA);
    SQLDBEClasses.getDBEClasses(pSITA);
    SQLRules.getRules(pSITA);
  } // for (Iteration of potential Parents)
} // QueryGetAddonFares::findAddonFareInfo(#1)

///////////////////////////////////////////////////////////
//  QueryGetAddonFaresHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonFaresHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFaresHistorical"));
std::string QueryGetAddonFaresHistorical::_baseSQL;
bool QueryGetAddonFaresHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonFaresHistorical> g_GetAddonFaresHistorical;

const char*
QueryGetAddonFaresHistorical::getQueryName() const
{
  return "GETADDONFARESHISTORICAL";
};

void
QueryGetAddonFaresHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonFaresHistoricalSQLStatement<QueryGetAddonFaresHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONFARESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonFaresHistorical::findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                                                const LocCode& interiorMarket,
                                                const CarrierCode& carrier,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(interiorMarket, 1);
  substParm(carrier, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  substParm(5, endDate);
  substParm(interiorMarket, 6);
  substParm(carrier, 7);
  substParm(8, startDate);
  substParm(9, endDate);
  substParm(10, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAOF.push_back(QueryGetAddonFaresHistoricalSQLStatement<
        QueryGetAddonFaresHistorical>::mapRowToAddonFareInfo(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONFARESHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSitaAddonQualCodesHistorical SQLQualCodes(_dbAdapt);
  QueryGetSitaAddonDBEClassesHistorical SQLDBEClasses(_dbAdapt);
  QueryGetSitaAddonRulesHistorical SQLRules(_dbAdapt);
  std::vector<AddonFareInfo*>::iterator AOFIt;
  for (AOFIt = lstAOF.begin(); AOFIt != lstAOF.end(); AOFIt++)
  {
    if ((*AOFIt)->vendor() != "SITA")
      continue; // No Kids for this one

    SITAAddonFareInfo* pSITA = (SITAAddonFareInfo*)*AOFIt;

    SQLQualCodes.getQualCodes(pSITA);
    SQLDBEClasses.getDBEClasses(pSITA);
    SQLRules.getRules(pSITA);
  } // for (Iteration of potential Parents)
} // QueryGetAddonFares::findAddonFareInfo(#1)

///////////////////////////////////////////////////////////
//  QueryGetAddonFaresGW
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonFaresGW::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFaresGW"));
std::string QueryGetAddonFaresGW::_baseSQL;
bool QueryGetAddonFaresGW::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonFaresGW> g_GetAddonFaresGW;

const char*
QueryGetAddonFaresGW::getQueryName() const
{
  return "GETADDONFARESGW";
};

void
QueryGetAddonFaresGW::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonFaresGWSQLStatement<QueryGetAddonFaresGW> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONFARESGW");

    substTableDef(&_baseSQL);
    QueryGetSitaAddonQualCodes::initialize();
    QueryGetSitaAddonDBEClasses::initialize();
    QueryGetSitaAddonRules::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonFaresGW::findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                                        const LocCode& gatewayMarket,
                                        const LocCode& interiorMarket,
                                        const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(gatewayMarket, 1);
  substParm(interiorMarket, 2);
  substParm(carrier, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAOF.push_back(
        QueryGetAddonFaresGWSQLStatement<QueryGetAddonFaresGW>::mapRowToAddonFareInfoGW(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONFARESGW: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSitaAddonQualCodes SQLQualCodes(_dbAdapt);
  QueryGetSitaAddonDBEClasses SQLDBEClasses(_dbAdapt);
  QueryGetSitaAddonRules SQLRules(_dbAdapt);
  std::vector<tse::AddonFareInfo*>::iterator AOFIt;
  for (AOFIt = lstAOF.begin(); AOFIt != lstAOF.end(); AOFIt++)
  {
    if ((*AOFIt)->vendor() != "SITA")
      continue; // No Kids for this one

    SITAAddonFareInfo* pSITA = (SITAAddonFareInfo*)*AOFIt;

    SQLQualCodes.getQualCodes(pSITA);
    SQLDBEClasses.getDBEClasses(pSITA);
    SQLRules.getRules(pSITA);
  } // for (Iteration of potential Parents)
} // QueryGetAddonFaresGW::findAddonFareInfo(#2-GW)

///////////////////////////////////////////////////////////
//  QueryGetAddonFaresGWHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAddonFaresGWHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAddonFaresGWHistorical"));
std::string QueryGetAddonFaresGWHistorical::_baseSQL;
bool QueryGetAddonFaresGWHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAddonFaresGWHistorical> g_GetAddonFaresGWHistorical;

const char*
QueryGetAddonFaresGWHistorical::getQueryName() const
{
  return "GETADDONFARESGWHISTORICAL";
};

void
QueryGetAddonFaresGWHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAddonFaresGWHistoricalSQLStatement<QueryGetAddonFaresGWHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETADDONFARESGWHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAddonFaresGWHistorical::findAddonFareInfo(std::vector<tse::AddonFareInfo*>& lstAOF,
                                                  const LocCode& gatewayMarket,
                                                  const LocCode& interiorMarket,
                                                  const CarrierCode& carrier,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(gatewayMarket, 1);
  substParm(interiorMarket, 2);
  substParm(carrier, 3);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, endDate);
  substParm(gatewayMarket, 7);
  substParm(interiorMarket, 8);
  substParm(carrier, 9);
  substParm(10, startDate);
  substParm(11, endDate);
  substParm(12, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstAOF.push_back(QueryGetAddonFaresGWHistoricalSQLStatement<
        QueryGetAddonFaresGWHistorical>::mapRowToAddonFareInfoGW(row));
  }
  LOG4CXX_INFO(_logger,
               "GETADDONFARESGWHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ")");
  res.freeResult();

  // Get Children
  QueryGetSitaAddonQualCodesHistorical SQLQualCodes(_dbAdapt);
  QueryGetSitaAddonDBEClassesHistorical SQLDBEClasses(_dbAdapt);
  QueryGetSitaAddonRulesHistorical SQLRules(_dbAdapt);
  std::vector<tse::AddonFareInfo*>::iterator AOFIt;
  for (AOFIt = lstAOF.begin(); AOFIt != lstAOF.end(); AOFIt++)
  {
    if ((*AOFIt)->vendor() != "SITA")
      continue; // No Kids for this one

    SITAAddonFareInfo* pSITA = (SITAAddonFareInfo*)*AOFIt;

    SQLQualCodes.getQualCodes(pSITA);
    SQLDBEClasses.getDBEClasses(pSITA);
    SQLRules.getRules(pSITA);
  } // for (Iteration of potential Parents)
} // QueryGetAddonFaresGW::findAddonFareInfo(#2-GW)

//////////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonQualCodes -
//////////////////////////////////////////////////////////////////////////////////////
const char*
QueryGetSitaAddonQualCodes::getQueryName() const
{
  return "GETSITAADDONQUALCODES";
};

void
QueryGetSitaAddonQualCodes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonQualCodesSQLStatement<QueryGetSitaAddonQualCodes> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONQUALCODES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonQualCodes::getQualCodes(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->fareQualCodes().insert(
        QueryGetSitaAddonQualCodesSQLStatement<QueryGetSitaAddonQualCodes>::mapRowToChar(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONQUALCODES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonQualCodes::getQualCodes()

//////////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonQualCodesHistorical
//////////////////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSitaAddonQualCodesHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonQualCodesHistorical"));
std::string QueryGetSitaAddonQualCodesHistorical::_baseSQL;
bool QueryGetSitaAddonQualCodesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonQualCodesHistorical> g_GetSitaAddonQualCodesHistorical;

const char*
QueryGetSitaAddonQualCodesHistorical::getQueryName() const
{
  return "GETSITAADDONQUALCODESHISTORICAL";
};

void
QueryGetSitaAddonQualCodesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonQualCodesHistoricalSQLStatement<QueryGetSitaAddonQualCodesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONQUALCODESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonQualCodesHistorical::getQualCodes(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 4);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 5);
  substParm(a_pSITAAddonFareInfo->hashKey(), 6);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->fareQualCodes().insert(QueryGetSitaAddonQualCodesHistoricalSQLStatement<
        QueryGetSitaAddonQualCodesHistorical>::mapRowToChar(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONQUALCODESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonQualCodesHistorical::getQualCodes()

///////////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonDBEClasses
///////////////////////////////////////////////////////////////////////////////////////
const char*
QueryGetSitaAddonDBEClasses::getQueryName() const
{
  return "GETSITAADDONDBECLASSES";
};

void
QueryGetSitaAddonDBEClasses::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonDBEClassesSQLStatement<QueryGetSitaAddonDBEClasses> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONDBECLASSES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonDBEClasses::getDBEClasses(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->dbeClasses().insert(
        QueryGetSitaAddonDBEClassesSQLStatement<QueryGetSitaAddonDBEClasses>::mapRowToString(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONDBECLASSES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonDBEClasses::getDBEClasses()

///////////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonDBEClassesHistorical
///////////////////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSitaAddonDBEClassesHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonDBEClassesHistorical"));
std::string QueryGetSitaAddonDBEClassesHistorical::_baseSQL;
bool QueryGetSitaAddonDBEClassesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonDBEClassesHistorical> g_GetSitaAddonDBEClassesHistorical;

const char*
QueryGetSitaAddonDBEClassesHistorical::getQueryName() const
{
  return "GETSITAADDONDBECLASSESHISTORICAL";
};

void
QueryGetSitaAddonDBEClassesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonDBEClassesHistoricalSQLStatement<QueryGetSitaAddonDBEClassesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONDBECLASSESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonDBEClassesHistorical::getDBEClasses(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 4);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 5);
  substParm(a_pSITAAddonFareInfo->hashKey(), 6);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->dbeClasses().insert(QueryGetSitaAddonDBEClassesHistoricalSQLStatement<
        QueryGetSitaAddonDBEClassesHistorical>::mapRowToString(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONDBECLASSESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonDBEClassesHistorical::getDBEClasses()

//////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonRules - Used for Historical also because all dates are null
//////////////////////////////////////////////////////////////////////////////////
const char*
QueryGetSitaAddonRules::getQueryName() const
{
  return "GETSITAADDONRULES";
};

void
QueryGetSitaAddonRules::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonRulesSQLStatement<QueryGetSitaAddonRules> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONRULES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonRules::getRules(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->rules().insert(
        QueryGetSitaAddonRulesSQLStatement<QueryGetSitaAddonRules>::mapRowToString(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONRULES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonDBEClasses::getDBEClasses()

//////////////////////////////////////////////////////////////////////////////////
//  QueryGetSitaAddonRulesHistorical
//////////////////////////////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetSitaAddonRulesHistorical::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetAddonFares.GetSitaAddonRulesHistorical"));
std::string QueryGetSitaAddonRulesHistorical::_baseSQL;
bool QueryGetSitaAddonRulesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSitaAddonRulesHistorical> g_GetSitaAddonRuleHistorical;

const char*
QueryGetSitaAddonRulesHistorical::getQueryName() const
{
  return "GETSITAADDONRULESHISTORICAL";
};

void
QueryGetSitaAddonRulesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSitaAddonRulesHistoricalSQLStatement<QueryGetSitaAddonRulesHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSITAADDONRULESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSitaAddonRulesHistorical::getRules(SITAAddonFareInfo* a_pSITAAddonFareInfo)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 1);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 2);
  substParm(a_pSITAAddonFareInfo->hashKey(), 3);
  substParm(a_pSITAAddonFareInfo->gatewayMarket(), 4);
  substParm(a_pSITAAddonFareInfo->interiorMarket(), 5);
  substParm(a_pSITAAddonFareInfo->hashKey(), 6);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pSITAAddonFareInfo->rules().insert(QueryGetSitaAddonRulesHistoricalSQLStatement<
        QueryGetSitaAddonRulesHistorical>::mapRowToString(row));
  }
  LOG4CXX_INFO(_logger,
               "GETSITAADDONRULESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetSitaAddonDBEClassesHistorical::getDBEClasses()
}
