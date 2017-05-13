//----------------------------------------------------------------------------
//  File:           QueryGetMinFareRule.cpp
//  Description:    QueryGetMinFareRule
//  Created:        8/24/2006
//  Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetMinFareRule.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMinFareRuleSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetMinFareRuleLevExclBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBase"));
std::string QueryGetMinFareRuleLevExclBase::_baseSQL;
bool QueryGetMinFareRuleLevExclBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleLevExclBase> g_GetMinFareRuleLevExclBase;

log4cxx::LoggerPtr
QueryGetMinFareRuleFareClasses::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBase.GetMinFareRuleFareClasses"));
std::string QueryGetMinFareRuleFareClasses::_baseSQL;
bool QueryGetMinFareRuleFareClasses::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleFareClasses> g_GetMinFareRuleFareClasses;

log4cxx::LoggerPtr
QueryGetMinFareRuleFareTypes::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBase.GetMinFareRuleFareTypes"));
std::string QueryGetMinFareRuleFareTypes::_baseSQL;
bool QueryGetMinFareRuleFareTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleFareTypes> g_GetMinFareRuleFareTypes;

log4cxx::LoggerPtr
QueryGetMinFareRuleSameFareGroupChild::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBase.GetMinFareRuleSameFareGroupChild"));
std::string QueryGetMinFareRuleSameFareGroupChild::_baseSQL;
bool QueryGetMinFareRuleSameFareGroupChild::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleSameFareGroupChild>
g_QueryGetMinFareRuleSameFareGroupChild;

const char*
QueryGetMinFareRuleLevExclBase::getQueryName() const
{
  return "GETMINFARERULELEVEXCLBASE";
}

void
QueryGetMinFareRuleLevExclBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleLevExclBaseSQLStatement<QueryGetMinFareRuleLevExclBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULELEVEXCLBASE");
    substTableDef(&_baseSQL);

    QueryGetMinFareRuleFareClasses::initialize();
    QueryGetMinFareRuleFareTypes::initialize();
    QueryGetMinFareRuleSameFareGroupChild::initialize();

    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleLevExclBase::findMinFareRuleLevelExcl(
    std::vector<tse::MinFareRuleLevelExcl*>& lstMFRLE,
    VendorCode& vendor,
    int textTblItemNo,
    CarrierCode& govCxr,
    TariffNumber rulTrf)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[20], strRT[20];
  sprintf(strTIN, "%d", textTblItemNo);
  sprintf(strRT, "%d", rulTrf);

  substParm(vendor, 1);
  substParm(strTIN, 2);
  substParm(govCxr, 3);
  substParm(strRT, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareRuleLevelExcl* rle = nullptr;
  tse::MinFareRuleLevelExcl* rlePrev = nullptr;
  while ((row = res.nextRow()))
  {
    rle = QueryGetMinFareRuleLevExclBaseSQLStatement<
        QueryGetMinFareRuleLevExclBase>::mapRowToMinFareRuleLevelExclBase(row, rlePrev);
    if (rle != rlePrev)
      lstMFRLE.push_back(rle);

    rlePrev = rle;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFARERULELEVEXCLBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();

  // Instantiate child queries
  QueryGetMinFareRuleFareClasses SQLFareClasses(_dbAdapt);
  QueryGetMinFareRuleFareTypes SQLFareTypes(_dbAdapt);
  QueryGetMinFareRuleSameFareGroupChild SQLSameFareGroupChild(_dbAdapt);

  std::vector<tse::MinFareRuleLevelExcl*>::iterator MFRLEIt;
  for (MFRLEIt = lstMFRLE.begin(); MFRLEIt != lstMFRLE.end(); MFRLEIt++)
  {
    SQLFareClasses.getFareClasses(*MFRLEIt);
    SQLFareTypes.getFareTypes(*MFRLEIt);
    SQLSameFareGroupChild.getFareSet(*MFRLEIt);
  } // for (Iteration thru Parents)
} // findMinFareRuleLevelExcl()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleLevExclBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMinFareRuleLevExclBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBaseHistorical"));
std::string QueryGetMinFareRuleLevExclBaseHistorical::_baseSQL;
bool QueryGetMinFareRuleLevExclBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleLevExclBaseHistorical>
g_GetMinFareRuleLevExclBaseHistorical;

log4cxx::LoggerPtr
QueryGetMinFareRuleFareClassesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBaseHistorical."
                               "GetMinFareRuleFareClassesHistorical"));
std::string QueryGetMinFareRuleFareClassesHistorical::_baseSQL;
bool QueryGetMinFareRuleFareClassesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleFareClassesHistorical>
g_GetMinFareRuleFareClassesHistorical;

log4cxx::LoggerPtr
QueryGetMinFareRuleFareTypesHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBaseHistorical."
                               "GetMinFareRuleFareTypesHistorical"));
std::string QueryGetMinFareRuleFareTypesHistorical::_baseSQL;
bool QueryGetMinFareRuleFareTypesHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleFareTypesHistorical>
g_GetMinFareRuleFareTypesHistorical;

log4cxx::LoggerPtr
QueryGetMinFareRuleSameFareGroupChildHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareRuleLevExclBaseHistorical."
                               "GetMinFareRuleSameFareGroupChildHistorical"));
std::string QueryGetMinFareRuleSameFareGroupChildHistorical::_baseSQL;
bool QueryGetMinFareRuleSameFareGroupChildHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRuleSameFareGroupChildHistorical>
g_GetMinFareRuleSameFareGroupChildHistorical;

const char*
QueryGetMinFareRuleLevExclBaseHistorical::getQueryName() const
{
  return "GETMINFARERULELEVEXCLBASEHISTORICAL";
}

void
QueryGetMinFareRuleLevExclBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleLevExclBaseHistoricalSQLStatement<QueryGetMinFareRuleLevExclBaseHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULELEVEXCLBASEHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetMinFareRuleFareClassesHistorical::initialize();
    QueryGetMinFareRuleFareTypesHistorical::initialize();
    QueryGetMinFareRuleSameFareGroupChildHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleLevExclBaseHistorical::findMinFareRuleLevelExcl(
    std::vector<tse::MinFareRuleLevelExcl*>& lstMFRLE,
    VendorCode& vendor,
    int textTblItemNo,
    CarrierCode& govCxr,
    TariffNumber rulTrf,
    const DateTime& startDate,
    const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[20], strRT[20];
  sprintf(strTIN, "%d", textTblItemNo);
  sprintf(strRT, "%d", rulTrf);

  substParm(1, vendor);
  substParm(2, strTIN);
  substParm(3, govCxr);
  substParm(4, strRT);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(8, vendor);
  substParm(9, strTIN);
  substParm(10, govCxr);
  substParm(11, strRT);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareRuleLevelExcl* rle = nullptr;
  tse::MinFareRuleLevelExcl* rlePrev = nullptr;
  while ((row = res.nextRow()))
  {
    rle = QueryGetMinFareRuleLevExclBaseHistoricalSQLStatement<
        QueryGetMinFareRuleLevExclBaseHistorical>::mapRowToMinFareRuleLevelExclBase(row, rlePrev);
    if (rle != rlePrev)
      lstMFRLE.push_back(rle);

    rlePrev = rle;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFARERULELEVEXCLBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();

  // Instantiate child queries
  QueryGetMinFareRuleFareClassesHistorical SQLFareClasses(_dbAdapt);
  QueryGetMinFareRuleFareTypesHistorical SQLFareTypes(_dbAdapt);
  QueryGetMinFareRuleSameFareGroupChildHistorical SQLSameFareGroupChild(_dbAdapt);

  std::vector<tse::MinFareRuleLevelExcl*>::iterator MFRLEIt;
  for (MFRLEIt = lstMFRLE.begin(); MFRLEIt != lstMFRLE.end(); MFRLEIt++)
  {
    SQLFareClasses.getFareClasses(*MFRLEIt);
    SQLFareTypes.getFareTypes(*MFRLEIt);
    SQLSameFareGroupChild.getFareSet(*MFRLEIt);
  } // for (Iteration thru Parents)
} // findMinFareRuleLevelExcl()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleFareClasses
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleFareClasses::getQueryName() const
{
  return "GETMINFARERULEFARECLASSES";
}

void
QueryGetMinFareRuleFareClasses::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleFareClassesSQLStatement<QueryGetMinFareRuleFareClasses> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULEFARECLASSES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleFareClasses::getFareClasses(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareClasses().push_back(QueryGetMinFareRuleFareClassesSQLStatement<
        QueryGetMinFareRuleFareClasses>::mapRowToFareClass(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFAREFARECLASSES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleFareClasses::getFareClasses()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleFareClassesHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleFareClassesHistorical::getQueryName() const
{
  return "GETMINFARERULEFARECLASSESHISTORICAL";
}

void
QueryGetMinFareRuleFareClassesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleFareClassesHistoricalSQLStatement<QueryGetMinFareRuleFareClassesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULEFARECLASSESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleFareClassesHistorical::getFareClasses(
    MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  substParm(a_pMinFareRuleLevelExcl->vendor(), 8);
  substParm(strTIN, 9);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 10);
  substParm(strRT, 11);
  substParm(12, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 13);
  substParm(14, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareClasses().push_back(
        QueryGetMinFareRuleFareClassesHistoricalSQLStatement<
            QueryGetMinFareRuleFareClassesHistorical>::mapRowToFareClass(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULEFARECLASSESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleFareClassesHistorical::getFareClasses()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleFareTypes
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleFareTypes::getQueryName() const
{
  return "GETMINFARERULEFARETYPES";
}

void
QueryGetMinFareRuleFareTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleFareTypesSQLStatement<QueryGetMinFareRuleFareTypes> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULEFARETYPES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleFareTypes::getFareTypes(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareTypes().push_back(
        QueryGetMinFareRuleFareTypesSQLStatement<QueryGetMinFareRuleFareTypes>::mapRowToFareType(
            row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULEFARETYPES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleFareTypes::getFareTypes()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleFareTypesHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleFareTypesHistorical::getQueryName() const
{
  return "GETMINFARERULEFARETYPESHISTORICAL";
}

void
QueryGetMinFareRuleFareTypesHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleFareTypesHistoricalSQLStatement<QueryGetMinFareRuleFareTypesHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULEFARETYPESHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleFareTypesHistorical::getFareTypes(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  substParm(a_pMinFareRuleLevelExcl->vendor(), 8);
  substParm(strTIN, 9);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 10);
  substParm(strRT, 11);
  substParm(12, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 13);
  substParm(14, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareTypes().push_back(
        QueryGetMinFareRuleFareTypesHistoricalSQLStatement<
            QueryGetMinFareRuleFareTypesHistorical>::mapRowToFareType(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULEFARETYPESHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleFareTypesHistorical::getFareTypes()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleSameFareGroupChild
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleSameFareGroupChild::getQueryName() const
{
  return "GETMINFARERULESAMEFAREGROUPCHILD";
}

void
QueryGetMinFareRuleSameFareGroupChild::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleSameFareGroupChildSQLStatement<QueryGetMinFareRuleSameFareGroupChild>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULESAMEFAREGROUPCHILD");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleSameFareGroupChild::getFareSet(MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareSet().push_back(QueryGetMinFareRuleSameFareGroupChildSQLStatement<
        QueryGetMinFareRuleSameFareGroupChild>::mapRowToSetNumber(row));
    a_pMinFareRuleLevelExcl->sameFareGroupTariff().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupTariff(row));
    a_pMinFareRuleLevelExcl->sameFareGroupRules().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupRules(row));
    a_pMinFareRuleLevelExcl->sameFareGroupFareClasses().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupFareClasses(row));
    a_pMinFareRuleLevelExcl->sameFareGroupFareTypes().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupFareTypes(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULESAMEFAREGROUPCHILD: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleFareClasses::getFareClasses()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRuleFareClassesHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRuleSameFareGroupChildHistorical::getQueryName() const
{
  return "GETMINFARERULESAMEFAREGROUPCHILDHISTORICAL";
}

void
QueryGetMinFareRuleSameFareGroupChildHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRuleSameFareGroupChildHistoricalSQLStatement<
        QueryGetMinFareRuleSameFareGroupChildHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULESAMEFAREGROUPCHILDHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRuleSameFareGroupChildHistorical::getFareSet(
    MinFareRuleLevelExcl* a_pMinFareRuleLevelExcl)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareRuleLevelExcl->textTblItemNo());
  char strRT[15];
  sprintf(strRT, "%d", a_pMinFareRuleLevelExcl->ruleTariff());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareRuleLevelExcl->seqNo());

  resetSQL();

  substParm(a_pMinFareRuleLevelExcl->vendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 3);
  substParm(strRT, 4);
  substParm(5, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 6);
  substParm(7, a_pMinFareRuleLevelExcl->createDate());
  substParm(a_pMinFareRuleLevelExcl->vendor(), 8);
  substParm(strTIN, 9);
  substParm(a_pMinFareRuleLevelExcl->governingCarrier(), 10);
  substParm(strRT, 11);
  substParm(12, a_pMinFareRuleLevelExcl->versionDate());
  substParm(strSN, 13);
  substParm(14, a_pMinFareRuleLevelExcl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareRuleLevelExcl->fareSet().push_back(QueryGetMinFareRuleSameFareGroupChildSQLStatement<
        QueryGetMinFareRuleSameFareGroupChild>::mapRowToSetNumber(row));
    a_pMinFareRuleLevelExcl->sameFareGroupTariff().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupTariff(row));
    a_pMinFareRuleLevelExcl->sameFareGroupRules().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupRules(row));
    a_pMinFareRuleLevelExcl->sameFareGroupFareClasses().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupFareClasses(row));
    a_pMinFareRuleLevelExcl->sameFareGroupFareTypes().push_back(
        QueryGetMinFareRuleSameFareGroupChildSQLStatement<
            QueryGetMinFareRuleSameFareGroupChild>::mapRowToSameFareGroupFareTypes(row));

  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULESAMEFAREGROUPCHILDHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRuleSameFareGroupChildHistorical::getFareSet()
}
