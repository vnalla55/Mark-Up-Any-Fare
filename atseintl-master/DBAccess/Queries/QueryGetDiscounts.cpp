//----------------------------------------------------------------------------
//  File:           QueryGetDiscounts.cpp
//  Description:    QueryGetDiscounts
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
#include "DBAccess/Queries/QueryGetDiscounts.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDiscountsSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetChdDiscount::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetChdDiscount"));
std::string QueryGetChdDiscount::_baseSQL;
bool QueryGetChdDiscount::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetChdDiscount> g_GetChdDiscount;

log4cxx::LoggerPtr
QueryGetTourDiscount::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTourDiscount"));
std::string QueryGetTourDiscount::_baseSQL;
bool QueryGetTourDiscount::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTourDiscount> g_GetTourDiscount;

log4cxx::LoggerPtr
QueryGetAgentDiscount::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAgentDiscount"));
std::string QueryGetAgentDiscount::_baseSQL;
bool QueryGetAgentDiscount::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAgentDiscount> g_GetAgentDiscount;

log4cxx::LoggerPtr
QueryGetOthersDiscount::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOthersDiscount"));
std::string QueryGetOthersDiscount::_baseSQL;
bool QueryGetOthersDiscount::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOthersDiscount> g_GetOthersDiscount;

const char*
QueryGetChdDiscount::getQueryName() const
{
  return "GETCHDDISCOUNT";
};

void
QueryGetChdDiscount::initialize()
{
  if (!_isInitialized)
  {
    QueryGetChdDiscountSQLStatement<QueryGetChdDiscount> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCHDDISCOUNT");
    substTableDef(&_baseSQL);

    QueryGetTourDiscount::initialize();
    QueryGetAgentDiscount::initialize();
    QueryGetOthersDiscount::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetChdDiscount::findDiscountInfo(std::vector<tse::DiscountInfo*>& discount,
                                      const VendorCode& vendor,
                                      int itemNumber,
                                      int category)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  if (category == tse::DiscountInfo::CHILD)
  {
    substParm(vendor, 1);
    substParm(itemStr, 2);
    substCurrentDate();
    LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
    res.executeQuery(this);
    tse::DiscountInfo* disc = nullptr;
    tse::DiscountInfo* discPrev = nullptr;
    while ((row = res.nextRow()))
    {
      disc =
          QueryGetChdDiscountSQLStatement<QueryGetChdDiscount>::mapRowToDiscountInfo(row, discPrev);
      if (disc != discPrev)
      {
        disc->category() = tse::DiscountInfo::CHILD;
        discount.push_back(disc);
      }
      discPrev = disc;
    }
    LOG4CXX_INFO(_logger,
                 "GETCHDDISCOUNT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                              << " mSecs");
    res.freeResult();
  }
  else if (category == tse::DiscountInfo::TOUR)
  {
    QueryGetTourDiscount SQLTourDiscounts(_dbAdapt);
    SQLTourDiscounts.getTourDiscounts(discount, vendor, itemNumber);
  }
  else if (category == tse::DiscountInfo::AGENT)
  {
    QueryGetAgentDiscount SQLAgentDiscounts(_dbAdapt);
    SQLAgentDiscounts.getAgentDiscounts(discount, vendor, itemNumber);
  }
  else if (category == tse::DiscountInfo::OTHERS)
  {
    QueryGetOthersDiscount SQLOthersDiscounts(_dbAdapt);
    SQLOthersDiscounts.getOthersDiscounts(discount, vendor, itemNumber);
  }
} // QueryGetChdDiscount::findDiscountInfo()

///////////////////////////////////////////////////////////
//  QueryGetChdDiscountHistorical
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetChdDiscountHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetChdDiscountHistorical"));
std::string QueryGetChdDiscountHistorical::_baseSQL;
bool QueryGetChdDiscountHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetChdDiscountHistorical> g_GetChdDiscountHistorical;

log4cxx::LoggerPtr
QueryGetTourDiscountHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTourDiscountHistorical"));
std::string QueryGetTourDiscountHistorical::_baseSQL;
bool QueryGetTourDiscountHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTourDiscountHistorical> g_GetTourDiscountHistorical;

log4cxx::LoggerPtr
QueryGetAgentDiscountHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAgentDiscountHistorical"));
std::string QueryGetAgentDiscountHistorical::_baseSQL;
bool QueryGetAgentDiscountHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAgentDiscountHistorical> g_GetAgentDiscountHistorical;

log4cxx::LoggerPtr
QueryGetOthersDiscountHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetOthersDiscountHistorical"));
std::string QueryGetOthersDiscountHistorical::_baseSQL;
bool QueryGetOthersDiscountHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetOthersDiscountHistorical> g_GetOthersDiscountHistorical;

const char*
QueryGetChdDiscountHistorical::getQueryName() const
{
  return "GETCHDDISCOUNTHISTORICAL";
};

void
QueryGetChdDiscountHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetChdDiscountHistoricalSQLStatement<QueryGetChdDiscountHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCHDDISCOUNTHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetTourDiscountHistorical::initialize();
    QueryGetAgentDiscountHistorical::initialize();
    QueryGetOthersDiscountHistorical::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetChdDiscountHistorical::findDiscountInfo(std::vector<tse::DiscountInfo*>& discount,
                                                const VendorCode& vendor,
                                                int itemNumber,
                                                int category,
                                                const DateTime& startDate,
                                                const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  if (category == tse::DiscountInfo::CHILD)
  {
    substParm(vendor, 1);
    substParm(itemStr, 2);
    substParm(3, startDate);
    substParm(4, endDate);
    LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
    res.executeQuery(this);
    tse::DiscountInfo* disc = nullptr;
    tse::DiscountInfo* discPrev = nullptr;
    while ((row = res.nextRow()))
    {
      disc = QueryGetChdDiscountHistoricalSQLStatement<
          QueryGetChdDiscountHistorical>::mapRowToDiscountInfo(row, discPrev);
      if (disc != discPrev)
      {
        disc->category() = tse::DiscountInfo::CHILD;
        discount.push_back(disc);
      }
      discPrev = disc;
    }
    LOG4CXX_INFO(_logger,
                 "GETCHDDISCOUNTHISTORICAL: NumRows = " << res.numRows()
                                                        << " Time = " << stopTimer() << " mSecs");
    res.freeResult();
  }
  else if (category == tse::DiscountInfo::TOUR)
  {
    QueryGetTourDiscountHistorical SQLTourDiscounts(_dbAdapt);
    SQLTourDiscounts.getTourDiscounts(discount, vendor, itemNumber, startDate, endDate);
  }
  else if (category == tse::DiscountInfo::AGENT)
  {
    QueryGetAgentDiscountHistorical SQLAgentDiscounts(_dbAdapt);
    SQLAgentDiscounts.getAgentDiscounts(discount, vendor, itemNumber, startDate, endDate);
  }
  else if (category == tse::DiscountInfo::OTHERS)
  {
    QueryGetOthersDiscountHistorical SQLOthersDiscounts(_dbAdapt);
    SQLOthersDiscounts.getOthersDiscounts(discount, vendor, itemNumber, startDate, endDate);
  }
} // QueryGetChdDiscountHistorical::findDiscountInfo()

///////////////////////////////////////////////////////////
//  QueryGetTourDiscount
///////////////////////////////////////////////////////////
const char*
QueryGetTourDiscount::getQueryName() const
{
  return "GETTOURDISCOUNT";
};

void
QueryGetTourDiscount::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTourDiscountSQLStatement<QueryGetTourDiscount> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTOURDISCOUNT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTourDiscount::getTourDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                       const VendorCode& vendor,
                                       int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc =
        QueryGetTourDiscountSQLStatement<QueryGetTourDiscount>::mapRowToDiscountInfo(row, discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::TOUR;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETTOURDISCOUNT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                             << " mSecs");
  res.freeResult();
} // QueryGetTourDiscount::getTourDiscounts()

///////////////////////////////////////////////////////////
//  QueryGetTourDiscountHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetTourDiscountHistorical::getQueryName() const
{
  return "GETTOURDISCOUNTHISTORICAL";
};

void
QueryGetTourDiscountHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTourDiscountHistoricalSQLStatement<QueryGetTourDiscountHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTOURDISCOUNTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTourDiscountHistorical::getTourDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                                 const VendorCode& vendor,
                                                 int itemNumber,
                                                 const DateTime& startDate,
                                                 const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc = QueryGetTourDiscountHistoricalSQLStatement<
        QueryGetTourDiscountHistorical>::mapRowToDiscountInfo(row, discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::TOUR;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETTOURDISCOUNTHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                       << " mSecs");
  res.freeResult();
} // QueryGetTourDiscountHistorical::getTourDiscounts()

///////////////////////////////////////////////////////////
//  QueryGetAgentDiscount
///////////////////////////////////////////////////////////
const char*
QueryGetAgentDiscount::getQueryName() const
{
  return "GETAGENTDISCOUNT";
};

void
QueryGetAgentDiscount::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAgentDiscountSQLStatement<QueryGetAgentDiscount> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAGENTDISCOUNT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAgentDiscount::getAgentDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                         const VendorCode& vendor,
                                         int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc = QueryGetAgentDiscountSQLStatement<QueryGetAgentDiscount>::mapRowToDiscountInfo(row,
                                                                                          discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::AGENT;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETAGENTDISCOUNT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                              << " mSecs");
  res.freeResult();
} // QueryGetAgentDiscount::getAgentDiscounts()

///////////////////////////////////////////////////////////
//  QueryGetAgentDiscountHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetAgentDiscountHistorical::getQueryName() const
{
  return "GETAGENTDISCOUNTHISTORICAL";
};

void
QueryGetAgentDiscountHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAgentDiscountHistoricalSQLStatement<QueryGetAgentDiscountHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETAGENTDISCOUNTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAgentDiscountHistorical::getAgentDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                                   const VendorCode& vendor,
                                                   int itemNumber,
                                                   const DateTime& startDate,
                                                   const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc = QueryGetAgentDiscountHistoricalSQLStatement<
        QueryGetAgentDiscountHistorical>::mapRowToDiscountInfo(row, discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::AGENT;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETAGENTDISCOUNTHISTORICAL: NumRows = " << res.numRows()
                                                        << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetAgentDiscountHistorical::getAgentDiscounts()

///////////////////////////////////////////////////////////
//  QueryGetOthersDiscount
///////////////////////////////////////////////////////////
const char*
QueryGetOthersDiscount::getQueryName() const
{
  return "GETOTHERSDISCOUNT";
};

void
QueryGetOthersDiscount::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOthersDiscountSQLStatement<QueryGetOthersDiscount> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOTHERSDISCOUNT");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOthersDiscount::getOthersDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                           const VendorCode& vendor,
                                           int itemNumber)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc = QueryGetOthersDiscountSQLStatement<QueryGetOthersDiscount>::mapRowToDiscountInfo(
        row, discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::OTHERS;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETOTHERSDISCOUNT: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                               << " mSecs");
  res.freeResult();
} // QueryGetOthersDiscount::getOthersDiscounts()

///////////////////////////////////////////////////////////
//  QueryGetOthersDiscountHistorical
///////////////////////////////////////////////////////////
const char*
QueryGetOthersDiscountHistorical::getQueryName() const
{
  return "GETOTHERSDISCOUNTHISTORICAL";
};

void
QueryGetOthersDiscountHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetOthersDiscountHistoricalSQLStatement<QueryGetOthersDiscountHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETOTHERSDISCOUNTHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetOthersDiscountHistorical::getOthersDiscounts(std::vector<tse::DiscountInfo*>& discounts,
                                                     const VendorCode& vendor,
                                                     int itemNumber,
                                                     const DateTime& startDate,
                                                     const DateTime& endDate)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  char itemStr[15];
  sprintf(itemStr, "%d", itemNumber);

  resetSQL();

  substParm(vendor, 1);
  substParm(itemStr, 2);
  substParm(3, startDate);
  substParm(4, endDate);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  tse::DiscountInfo* disc = nullptr;
  tse::DiscountInfo* discPrev = nullptr;
  while ((row = res.nextRow()))
  {
    disc = QueryGetOthersDiscountHistoricalSQLStatement<
        QueryGetOthersDiscountHistorical>::mapRowToDiscountInfo(row, discPrev);
    if (disc != discPrev)
    {
      disc->category() = tse::DiscountInfo::OTHERS;
      discounts.push_back(disc);
    }
    discPrev = disc;
  }
  LOG4CXX_INFO(_logger,
               "GETOTHERSDISCOUNTHISTORICAL: NumRows = " << res.numRows()
                                                         << " Time = " << stopTimer() << " mSecs");
  res.freeResult();
} // QueryGetOthersDiscountHistorical::getOthersDiscounts()
}
