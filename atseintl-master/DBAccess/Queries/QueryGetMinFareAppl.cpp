//----------------------------------------------------------------------------
//  File:           QueryGetMinFareAppl.cpp
//  Description:    QueryGetMinFareAppl
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
#include "DBAccess/Queries/QueryGetMinFareAppl.h"
#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMinFareApplSQLStatement.h"

namespace tse
{

log4cxx::LoggerPtr
QueryGetMinFareApplBase::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareApplBase"));
std::string QueryGetMinFareApplBase::_baseSQL;
bool QueryGetMinFareApplBase::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareApplBase> g_GetMinFareApplBase;

log4cxx::LoggerPtr
QueryGetMinFareRules::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareRules"));
std::string QueryGetMinFareRules::_baseSQL;
bool QueryGetMinFareRules::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareRules> g_GetMinFareRules;

log4cxx::LoggerPtr
QueryGetMinFareFareClasses::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareFareClasses"));
std::string QueryGetMinFareFareClasses::_baseSQL;
bool QueryGetMinFareFareClasses::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareFareClasses> g_GetMinFareFareClasses;

log4cxx::LoggerPtr
QueryGetMinFareFareTypes::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareFareTypes"));
std::string QueryGetMinFareFareTypes::_baseSQL;
bool QueryGetMinFareFareTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareFareTypes> g_GetMinFareFareTypes;

log4cxx::LoggerPtr
QueryGetMinFareDomFareTypes::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareDomFareTypes"));
std::string QueryGetMinFareDomFareTypes::_baseSQL;
bool QueryGetMinFareDomFareTypes::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareDomFareTypes> g_GetMinFareDomFareTypes;

log4cxx::LoggerPtr
QueryGetMinFareCxrFltRestrs::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareCxrFltRestrs"));
std::string QueryGetMinFareCxrFltRestrs::_baseSQL;
bool QueryGetMinFareCxrFltRestrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareCxrFltRestrs> g_GetMinFareCxrFltRestrs;

log4cxx::LoggerPtr
QueryGetMinFareSecCxrRestrs::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.GetMinFareApplBase.GetMinFareSecCxrRestrs"));
std::string QueryGetMinFareSecCxrRestrs::_baseSQL;
bool QueryGetMinFareSecCxrRestrs::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareSecCxrRestrs> g_GetMinFareSecCxrRestrs;

const char*
QueryGetMinFareApplBase::getQueryName() const
{
  return "GETMINFAREAPPLBASE";
}

void
QueryGetMinFareApplBase::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareApplBaseSQLStatement<QueryGetMinFareApplBase> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREAPPLBASE");
    substTableDef(&_baseSQL);

    QueryGetMinFareRules::initialize();
    QueryGetMinFareFareClasses::initialize();
    QueryGetMinFareFareTypes::initialize();
    QueryGetMinFareDomFareTypes::initialize();
    QueryGetMinFareCxrFltRestrs::initialize();
    QueryGetMinFareSecCxrRestrs::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareApplBase::findMinFareAppl(std::vector<tse::MinFareAppl*>& lstMFA,
                                         VendorCode& textTblVendor,
                                         int textTblItemNo,
                                         CarrierCode& govCxr)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[20];
  sprintf(strTIN, "%d", textTblItemNo);

  substParm(textTblVendor, 1);
  substParm(strTIN, 2);
  substParm(govCxr, 3);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareAppl* mfa = nullptr;
  tse::MinFareAppl* mfaPrev = nullptr;
  while ((row = res.nextRow()))
  {
    mfa = QueryGetMinFareApplBaseSQLStatement<QueryGetMinFareApplBase>::mapRowToMinFareApplBase(
        row, mfaPrev);
    if (mfa != mfaPrev)
      lstMFA.push_back(mfa);

    mfaPrev = mfa;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREAPPLBASE: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();

  // Instantiate child queries
  QueryGetMinFareRules SQLRules(_dbAdapt);
  QueryGetMinFareFareClasses SQLFareClasses(_dbAdapt);
  QueryGetMinFareFareTypes SQLFareTypes(_dbAdapt);
  QueryGetMinFareDomFareTypes SQLDomFareTypes(_dbAdapt);
  QueryGetMinFareCxrFltRestrs SQLCxrFltRestrs(_dbAdapt);
  QueryGetMinFareSecCxrRestrs SQLSecCxrRestrs(_dbAdapt);

  std::vector<tse::MinFareAppl*>::iterator MFAIt;
  for (MFAIt = lstMFA.begin(); MFAIt != lstMFA.end(); MFAIt++)
  { // Get Children
    SQLRules.getRulesAndFootnotes(*MFAIt);
    SQLFareClasses.getFareClasses(*MFAIt);
    SQLFareTypes.getFareTypes(*MFAIt);
    SQLDomFareTypes.getDomFareTypes(*MFAIt);
    SQLCxrFltRestrs.getCxrFltRestrs(*MFAIt);
    SQLSecCxrRestrs.getSecCxrRestrs(*MFAIt);
  } // for (Iteration thru Parents)
} // findMinFareAppl()

///////////////////////////////////////////////////////////
//  QueryGetMinFareApplBaseHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetMinFareApplBaseHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetMinFareApplBaseHistorical"));
std::string QueryGetMinFareApplBaseHistorical::_baseSQL;
bool QueryGetMinFareApplBaseHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetMinFareApplBaseHistorical> g_GetMinFareApplBaseHistorical;

const char*
QueryGetMinFareApplBaseHistorical::getQueryName() const
{
  return "GETMINFAREAPPLBASEHISTORICAL";
}

void
QueryGetMinFareApplBaseHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareApplBaseHistoricalSQLStatement<QueryGetMinFareApplBaseHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREAPPLBASEHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareApplBaseHistorical::findMinFareAppl(std::vector<tse::MinFareAppl*>& lstMFA,
                                                   VendorCode& textTblVendor,
                                                   int textTblItemNo,
                                                   CarrierCode& govCxr,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char strTIN[20];
  sprintf(strTIN, "%d", textTblItemNo);

  substParm(1, textTblVendor);
  substParm(2, strTIN);
  substParm(3, govCxr);
  substParm(4, startDate);
  substParm(5, endDate);
  substParm(6, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::MinFareAppl* mfa = nullptr;
  tse::MinFareAppl* mfaPrev = nullptr;
  while ((row = res.nextRow()))
  {
    mfa = QueryGetMinFareApplBaseHistoricalSQLStatement<
        QueryGetMinFareApplBaseHistorical>::mapRowToMinFareApplBase(row, mfaPrev);
    if (mfa != mfaPrev)
      lstMFA.push_back(mfa);

    mfaPrev = mfa;
  }
  LOG4CXX_INFO(_logger,
               "GETMINFAREAPPLBASEHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") mSecs");
  res.freeResult();

  // Instantiate child queries, Use Current because no date checks
  // other than createdate, which is part of join with parent
  QueryGetMinFareRules SQLRules(_dbAdapt);
  QueryGetMinFareFareClasses SQLFareClasses(_dbAdapt);
  QueryGetMinFareFareTypes SQLFareTypes(_dbAdapt);
  QueryGetMinFareDomFareTypes SQLDomFareTypes(_dbAdapt);
  QueryGetMinFareCxrFltRestrs SQLCxrFltRestrs(_dbAdapt);
  QueryGetMinFareSecCxrRestrs SQLSecCxrRestrs(_dbAdapt);

  std::vector<tse::MinFareAppl*>::iterator MFAIt;
  for (MFAIt = lstMFA.begin(); MFAIt != lstMFA.end(); MFAIt++)
  { // Get Children
    SQLRules.getRulesAndFootnotes(*MFAIt);
    SQLFareClasses.getFareClasses(*MFAIt);
    SQLFareTypes.getFareTypes(*MFAIt);
    SQLDomFareTypes.getDomFareTypes(*MFAIt);
    SQLCxrFltRestrs.getCxrFltRestrs(*MFAIt);
    SQLSecCxrRestrs.getSecCxrRestrs(*MFAIt);
  } // for (Iteration thru Parents)
} // findMinFareAppl()

///////////////////////////////////////////////////////////
//  QueryGetMinFareRules
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareRules::getQueryName() const
{
  return "GETMINFARERULES";
}

void
QueryGetMinFareRules::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareRulesSQLStatement<QueryGetMinFareRules> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARERULES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareRules::getRulesAndFootnotes(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    RuleNumber newRule = QueryGetMinFareRulesSQLStatement<QueryGetMinFareRules>::mapRowToRule(row);
	Footnote newFootnote =
              QueryGetMinFareRulesSQLStatement<QueryGetMinFareRules>::hasRowToFootnote(row) ?
                QueryGetMinFareRulesSQLStatement<QueryGetMinFareRules>::mapRowToFootnote(row) : "";
      a_pMinFareAppl->ruleFootnotes().push_back(std::make_pair(newRule, newFootnote));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARERULES: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareRules::getRulesAndFootnotes()

///////////////////////////////////////////////////////////
//  QueryGetMinFareFareClasses
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareFareClasses::getQueryName() const
{
  return "GETMINFAREFARECLASSES";
}

void
QueryGetMinFareFareClasses::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareFareClassesSQLStatement<QueryGetMinFareFareClasses> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREFARECLASSES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareFareClasses::getFareClasses(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareAppl->fareClasses().push_back(
        QueryGetMinFareFareClassesSQLStatement<QueryGetMinFareFareClasses>::mapRowToFareClass(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFAREFARECLASSES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareFareClasses::getFareClasses()

///////////////////////////////////////////////////////////
//  QueryGetMinFareFareTypes
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareFareTypes::getQueryName() const
{
  return "GETMINFAREFARETYPES";
}

void
QueryGetMinFareFareTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareFareTypesSQLStatement<QueryGetMinFareFareTypes> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREFARETYPES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareFareTypes::getFareTypes(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareAppl->fareTypes().push_back(
        QueryGetMinFareFareTypesSQLStatement<QueryGetMinFareFareTypes>::mapRowToFareType(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFAREFARETYPES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareFareTypes::getFareTypes()

///////////////////////////////////////////////////////////
//  QueryGetMinFareDomFareTypes
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareDomFareTypes::getQueryName() const
{
  return "GETMINFAREDOMFARETYPES";
}

void
QueryGetMinFareDomFareTypes::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareDomFareTypesSQLStatement<QueryGetMinFareDomFareTypes> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFAREDOMFARETYPES");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareDomFareTypes::getDomFareTypes(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareAppl->domFareTypes().push_back(
        QueryGetMinFareDomFareTypesSQLStatement<QueryGetMinFareDomFareTypes>::mapRowToFareType(
            row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFAREDOMFARETYPES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareDomFareTypes::getDomFareTypes()

///////////////////////////////////////////////////////////
//  QueryGetMinFareCxrFltRestrs
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareCxrFltRestrs::getQueryName() const
{
  return "GETMINFARECXRFLTRESTRS";
}

void
QueryGetMinFareCxrFltRestrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareCxrFltRestrsSQLStatement<QueryGetMinFareCxrFltRestrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARECXRFLTRESTRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareCxrFltRestrs::getCxrFltRestrs(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareAppl->cxrFltRestrs().push_back(QueryGetMinFareCxrFltRestrsSQLStatement<
        QueryGetMinFareCxrFltRestrs>::mapRowToMinFareCxrFltRestr(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARECXRFLTRESTRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareCxrFltRestrs::getCxrFltRestrs()

///////////////////////////////////////////////////////////
//  QueryGetMinFareSecCxrRestrs
///////////////////////////////////////////////////////////
const char*
QueryGetMinFareSecCxrRestrs::getQueryName() const
{
  return "GETMINFARESECCXRRESTRS";
}

void
QueryGetMinFareSecCxrRestrs::initialize()
{
  if (!_isInitialized)
  {
    QueryGetMinFareSecCxrRestrsSQLStatement<QueryGetMinFareSecCxrRestrs> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETMINFARESECCXRRESTRS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetMinFareSecCxrRestrs::getSecCxrRestrs(MinFareAppl* a_pMinFareAppl)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char strTIN[15];
  sprintf(strTIN, "%d", a_pMinFareAppl->textTblItemNo());
  char strSN[15];
  sprintf(strSN, "%d", a_pMinFareAppl->seqNo());

  resetSQL();

  substParm(a_pMinFareAppl->textTblVendor(), 1);
  substParm(strTIN, 2);
  substParm(a_pMinFareAppl->governingCarrier(), 3);
  substParm(4, a_pMinFareAppl->versionDate());
  substParm(strSN, 5);
  substParm(6, a_pMinFareAppl->createDate());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    a_pMinFareAppl->secondaryCxrs().push_back(
        QueryGetMinFareSecCxrRestrsSQLStatement<QueryGetMinFareSecCxrRestrs>::mapRowToCarrier(row));
  } // while
  LOG4CXX_INFO(_logger,
               "GETMINFARESECCXRRESTRS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetMinFareSecCxrRestrs::getSecCxrRestrs()
}
