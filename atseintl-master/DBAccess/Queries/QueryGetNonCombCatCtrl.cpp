//----------------------------------------------------------------------------
//  File:           QueryGetNonCombCatCtrl.cpp
//  Description:    QueryGetNonCombCatCtrl
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrlSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetNonCombCatCtrl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNonCombCatCtrl"));
std::string QueryGetNonCombCatCtrl::_baseSQL;
bool QueryGetNonCombCatCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNonCombCatCtrl> g_GetNonCombCatCtrl;

const char*
QueryGetNonCombCatCtrl::getQueryName() const
{
  return "GETNONCOMBCATCTRL";
}

void
QueryGetNonCombCatCtrl::initialize()
{
  if (UNLIKELY(!_isInitialized))
  {
    QueryGetNonCombCatCtrlSQLStatement<QueryGetNonCombCatCtrl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNONCOMBCATCTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetNonCombCatCtrl::findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
                                            const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            int ruleTariff,
                                            const RuleNumber& ruleNo,
                                            int cat)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ruleTarStr[15];
  char catStr[4];

  sprintf(ruleTarStr, "%d", ruleTariff);
  sprintf(catStr, "%d", cat);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substParm(catStr, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::GeneralFareRuleInfo* catCtrl = nullptr;
  tse::GeneralFareRuleInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl = QueryGetNonCombCatCtrlSQLStatement<QueryGetNonCombCatCtrl>::mapRowToGeneralFareRule(
        row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      combs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }

  LOG4CXX_INFO(_logger,
               "GETNONCOMBCATCTRL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                             << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::GeneralFareRuleInfo* i){i->sync_with_cache();});
} // QueryGetNonCombCatCtrl::findGeneralFareRule()

///////////////////////////////////////////////////////////
//  QueryGetNonCombCatCtrlHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNonCombCatCtrlHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNonCombCatCtrlHistorical"));
std::string QueryGetNonCombCatCtrlHistorical::_baseSQL;
bool QueryGetNonCombCatCtrlHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNonCombCatCtrlHistorical> g_GetNonCombCatCtrlHistorical;

const char*
QueryGetNonCombCatCtrlHistorical::getQueryName() const
{
  return "GETNONCOMBCATCTRLHISTORICAL";
}

void
QueryGetNonCombCatCtrlHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNonCombCatCtrlHistoricalSQLStatement<QueryGetNonCombCatCtrlHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNONCOMBCATCTRLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetNonCombCatCtrlHistorical::findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
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

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", ruleTariff);

  char catStr[4];
  sprintf(catStr, "%d", cat);

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

  tse::GeneralFareRuleInfo* catCtrl = nullptr;
  tse::GeneralFareRuleInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl = QueryGetNonCombCatCtrlHistoricalSQLStatement<
        QueryGetNonCombCatCtrlHistorical>::mapRowToGeneralFareRule(row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      combs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETNONCOMBCATCTRLHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                       << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::GeneralFareRuleInfo* i){i->sync_with_cache();});
} // QueryGetNonCombCatCtrlHistorical::findGeneralFareRule()

///////////////////////////////////////////////////////////
//  QueryGetNonCombCtrlBackDating
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetNonCombCtrlBackDating::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetNonCombCtrlBackDating"));
std::string QueryGetNonCombCtrlBackDating::_baseSQL;
bool QueryGetNonCombCtrlBackDating::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetNonCombCtrlBackDating> g_GetNonCombCtrlBackDating;

const char*
QueryGetNonCombCtrlBackDating::getQueryName() const
{
  return "GETNONCOMBCAT31CTRL";
}

void
QueryGetNonCombCtrlBackDating::initialize()
{
  if (!_isInitialized)
  {
    QueryGetNonCombCtrlBackDatingSQLStatement<QueryGetNonCombCtrlBackDating> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETNONCOMBCAT31CTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetNonCombCtrlBackDating::findGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   int ruleTariff,
                                                   const RuleNumber& ruleNo,
                                                   int cat,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  DBResultSet res(_dbAdapt);
  char ruleTarStr[15];
  char catStr[4];

  sprintf(ruleTarStr, "%d", ruleTariff);
  sprintf(catStr, "%d", cat);

  substParm(1, vendor);
  substParm(2, carrier);
  substParm(3, ruleTarStr);
  substParm(4, ruleNo);
  substParm(5, catStr);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(8, startDate);
  substParm(9, vendor);
  substParm(10, carrier);
  substParm(11, ruleTarStr);
  substParm(12, ruleNo);
  substParm(13, catStr);
  substParm(14, startDate);
  substParm(15, endDate);
  substParm(16, startDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::GeneralFareRuleInfo* catCtrl = nullptr;
  tse::GeneralFareRuleInfo* catCtrlPrev = nullptr;

  combs.reserve(res.numRows());

  while (Row* row = res.nextRow())
  {
    catCtrl = QueryGetNonCombCtrlBackDatingSQLStatement<
        QueryGetNonCombCtrlBackDating>::mapRowToGeneralFareRule(row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      combs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }

  LOG4CXX_INFO(_logger,
               "GETNONCOMBCAT31CTRL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::GeneralFareRuleInfo* i){i->sync_with_cache();});
} // QueryGetNonCombCtrlBackDating::findGeneralFareRule()

///////////////////////////////////////////////////////////
//  QueryGetAllNonCombCatCtrl
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllNonCombCatCtrl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllNonCombCatCtrl"));
std::string QueryGetAllNonCombCatCtrl::_baseSQL;
bool QueryGetAllNonCombCatCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllNonCombCatCtrl> g_GetAllNonCombCatCtrl;

const char*
QueryGetAllNonCombCatCtrl::getQueryName() const
{
  return "GETALLNONCOMBCATCTRL";
}

void
QueryGetAllNonCombCatCtrl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllNonCombCatCtrlSQLStatement<QueryGetAllNonCombCatCtrl> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLNONCOMBCATCTRL");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetAllNonCombCatCtrl::findAllGeneralFareRule(std::vector<tse::GeneralFareRuleInfo*>& combs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::GeneralFareRuleInfo* catCtrl = nullptr;
  tse::GeneralFareRuleInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl =
        QueryGetAllNonCombCatCtrlSQLStatement<QueryGetAllNonCombCatCtrl>::mapRowToGeneralFareRule(
            row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      combs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETALLNONCOMBCATCTRL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(combs.begin(), combs.end(), [](tse::GeneralFareRuleInfo* i){i->sync_with_cache();});
} // QueryGetAllNonCombCatCtrl::findAllGeneralFareRule()

///////////////////////////////////////////////////////////
//  QueryGetFBRCtrl
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFBRCtrl::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFBRCtrl"));
std::string QueryGetFBRCtrl::_baseSQL;
bool QueryGetFBRCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFBRCtrl> g_GetFBRCtrl;

const char*
QueryGetFBRCtrl::getQueryName() const
{
  return "GETFBRCTRL";
}

void
QueryGetFBRCtrl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFBRCtrlSQLStatement<QueryGetFBRCtrl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFBRCTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFBRCtrl::findFareByRuleCtrlInfo(std::vector<tse::FareByRuleCtrlInfo*>& fbrs,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        int ruleTariff,
                                        const RuleNumber& ruleNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char ruleTarStr[15];

  sprintf(ruleTarStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareByRuleCtrlInfo* catCtrl = nullptr;
  tse::FareByRuleCtrlInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl =
        QueryGetFBRCtrlSQLStatement<QueryGetFBRCtrl>::mapRowToFareByRuleCtrlInfo(row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      fbrs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETFBRCTRL: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                        << stopCPU() << ")");
  res.freeResult();
  std::for_each(fbrs.begin(), fbrs.end(), [](tse::FareByRuleCtrlInfo* i){i->sync_with_cache();});
} // QueryGetFBRCtrl::findFareByRuleCtrlInfo()

///////////////////////////////////////////////////////////
//  QueryGetFBRCtrlHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFBRCtrlHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFBRCtrlHistorical"));
std::string QueryGetFBRCtrlHistorical::_baseSQL;
bool QueryGetFBRCtrlHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFBRCtrlHistorical> g_GetFBRCtrlHistorical;

const char*
QueryGetFBRCtrlHistorical::getQueryName() const
{
  return "GETFBRCTRLHISTORICAL";
}

void
QueryGetFBRCtrlHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFBRCtrlHistoricalSQLStatement<QueryGetFBRCtrlHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFBRCTRLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFBRCtrlHistorical::findFareByRuleCtrlInfo(std::vector<tse::FareByRuleCtrlInfo*>& fbrs,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  int ruleTariff,
                                                  const RuleNumber& ruleNo,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", ruleTariff);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(ruleNo, 4);
  substParm(5, startDate);
  substParm(6, endDate);
  substParm(7, endDate);
  substParm(vendor, 8);
  substParm(carrier, 9);
  substParm(ruleTarStr, 10);
  substParm(ruleNo, 11);
  substParm(12, startDate);
  substParm(13, endDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FareByRuleCtrlInfo* catCtrl = nullptr;
  tse::FareByRuleCtrlInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl = QueryGetFBRCtrlHistoricalSQLStatement<
        QueryGetFBRCtrlHistorical>::mapRowToFareByRuleCtrlInfo(row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      fbrs.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETFBRCTRLHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
  std::for_each(fbrs.begin(), fbrs.end(), [](tse::FareByRuleCtrlInfo* i){i->sync_with_cache();});
} // QueryGetFBRCtrlHistorical::findFareByRuleCtrlInfo()

///////////////////////////////////////////////////////////
//  QueryGetFootNoteCtrl
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFootNoteCtrl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFootNoteCtrl"));
std::string QueryGetFootNoteCtrl::_baseSQL;
bool QueryGetFootNoteCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFootNoteCtrl> g_GetFootNoteCtrl;

const char*
QueryGetFootNoteCtrl::getQueryName() const
{
  return "GETFOOTNOTECTRL";
}

void
QueryGetFootNoteCtrl::initialize()
{
  if (UNLIKELY(!_isInitialized))
  {
    QueryGetFootNoteCtrlSQLStatement<QueryGetFootNoteCtrl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFOOTNOTECTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFootNoteCtrl::findFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           int fareTariff,
                                           const Footnote& footNote,
                                           int cat)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", fareTariff);

  char catStr[4];
  sprintf(catStr, "%d", cat);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(footNote, 4);
  substParm(catStr, 5);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FootNoteCtrlInfo* catCtrl = nullptr;
  tse::FootNoteCtrlInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl = QueryGetFootNoteCtrlSQLStatement<QueryGetFootNoteCtrl>::mapRowToFootNoteCtrlInfo(
        row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      foots.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }

  LOG4CXX_INFO(_logger,
               "GETFOOTNOTECTRL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                           << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(foots.begin(), foots.end(), [](tse::FootNoteCtrlInfo* i){i->sync_with_cache();});
} // QueryGetFootNoteCtrl::findFootNoteCtrlInfo()

///////////////////////////////////////////////////////////
//  QueryGetFootNoteCtrlHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFootNoteCtrlHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFootNoteCtrlHistorical"));
std::string QueryGetFootNoteCtrlHistorical::_baseSQL;
bool QueryGetFootNoteCtrlHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFootNoteCtrlHistorical> g_GetFootNoteCtrlHistorical;

const char*
QueryGetFootNoteCtrlHistorical::getQueryName() const
{
  return "GETFOOTNOTECTRLHISTORICAL";
}

void
QueryGetFootNoteCtrlHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFootNoteCtrlHistoricalSQLStatement<QueryGetFootNoteCtrlHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFOOTNOTECTRLHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetFootNoteCtrlHistorical::findFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots,
                                                     const VendorCode& vendor,
                                                     const CarrierCode& carrier,
                                                     int fareTariff,
                                                     const Footnote& footNote,
                                                     int cat,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char ruleTarStr[15];
  sprintf(ruleTarStr, "%d", fareTariff);

  char catStr[4];
  sprintf(catStr, "%d", cat);

  substParm(vendor, 1);
  substParm(carrier, 2);
  substParm(ruleTarStr, 3);
  substParm(footNote, 4);
  substParm(catStr, 5);
  substParm(6, startDate);
  substParm(7, endDate);
  substParm(vendor, 8);
  substParm(carrier, 9);
  substParm(ruleTarStr, 10);
  substParm(footNote, 11);
  substParm(catStr, 12);
  substParm(13, startDate);
  substParm(14, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FootNoteCtrlInfo* catCtrl = nullptr;
  tse::FootNoteCtrlInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl = QueryGetFootNoteCtrlHistoricalSQLStatement<
        QueryGetFootNoteCtrlHistorical>::mapRowToFootNoteCtrlInfo(row, catCtrlPrev);
    if (catCtrl != catCtrlPrev)
      foots.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETFOOTNOTECTRLHISTORICAL: NumRows " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(foots.begin(), foots.end(), [](tse::FootNoteCtrlInfo* i){i->sync_with_cache();});
} // QueryGetFootNoteCtrlHistorical::findFootNoteCtrlInfo()

///////////////////////////////////////////////////////////
//  QueryGetAllFootNoteCtrl
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFootNoteCtrl::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFootNoteCtrl"));
std::string QueryGetAllFootNoteCtrl::_baseSQL;
bool QueryGetAllFootNoteCtrl::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFootNoteCtrl> g_GetAllFootNoteCtrl;

const char*
QueryGetAllFootNoteCtrl::getQueryName() const
{
  return "GETALLFOOTNOTECTRL";
}

void
QueryGetAllFootNoteCtrl::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFootNoteCtrlSQLStatement<QueryGetAllFootNoteCtrl> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFOOTNOTECTRL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllFootNoteCtrl::findAllFootNoteCtrlInfo(std::vector<tse::FootNoteCtrlInfo*>& foots)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::FootNoteCtrlInfo* catCtrl = nullptr;
  tse::FootNoteCtrlInfo* catCtrlPrev = nullptr;
  while ((row = res.nextRow()))
  {
    catCtrl =
        QueryGetAllFootNoteCtrlSQLStatement<QueryGetAllFootNoteCtrl>::mapRowToFootNoteCtrlInfo(
            row, catCtrlPrev);

    if (catCtrl != catCtrlPrev)
      foots.push_back(catCtrl);

    catCtrlPrev = catCtrl;
  }
  LOG4CXX_INFO(_logger,
               "GETALLFOOTNOTECTRL: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                              << stopCPU() << ") mSecs");
  res.freeResult();
  std::for_each(foots.begin(), foots.end(), [](tse::FootNoteCtrlInfo* i){i->sync_with_cache();});
} // QueryGetAllFootNoteCtrl::findAllFootNoteCtrlInfo()
}
