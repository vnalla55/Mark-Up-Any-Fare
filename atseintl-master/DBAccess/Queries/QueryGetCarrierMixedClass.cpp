//----------------------------------------------------------------------------
//  File:           QueryGetCarrierMixedClass.cpp
//  Description:    QueryGetCarrierMixedClass
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

#include "DBAccess/Queries/QueryGetCarrierMixedClass.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierMixedClassSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCarrierMixedClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierMixedClass"));
std::string QueryGetCarrierMixedClass::_baseSQL;
bool QueryGetCarrierMixedClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierMixedClass> g_GetCarrierMixedClass;

const char*
QueryGetCarrierMixedClass::getQueryName() const
{
  return "GETCARRIERMIXEDCLASS";
}

void
QueryGetCarrierMixedClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierMixedClassSQLStatement<QueryGetCarrierMixedClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERMIXEDCLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierMixedClass::findCarrierMixedClass(std::vector<tse::CarrierMixedClass*>& mixedClasses,
                                                 const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierMixedClass* carrierMix = nullptr;
  tse::CarrierMixedClass* carrierMixPrev = nullptr;
  DateTime prevDate;
  while ((row = res.nextRow()))
  {
    carrierMix =
        QueryGetCarrierMixedClassSQLStatement<QueryGetCarrierMixedClass>::mapRowToCarrierMixedClass(
            row, carrierMixPrev, prevDate);
    if (carrierMix != carrierMixPrev)
      mixedClasses.push_back(carrierMix);

    carrierMixPrev = carrierMix;
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERMIXEDCLASS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                  << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierMixedClass()

///////////////////////////////////////////////////////////
//
//  QueryGetCarrierMixedClassHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCarrierMixedClassHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierMixedClassHistorical"));
std::string QueryGetCarrierMixedClassHistorical::_baseSQL;
bool QueryGetCarrierMixedClassHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierMixedClassHistorical> g_GetCarrierMixedClassHistorical;

const char*
QueryGetCarrierMixedClassHistorical::getQueryName() const
{
  return "GETCARRIERMIXEDCLASSHISTORICAL";
}

void
QueryGetCarrierMixedClassHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierMixedClassHistoricalSQLStatement<QueryGetCarrierMixedClassHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERMIXEDCLASSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierMixedClassHistorical::findCarrierMixedClass(
    std::vector<tse::CarrierMixedClass*>& mixedClasses, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierMixedClass* carrierMix = nullptr;
  tse::CarrierMixedClass* carrierMixPrev = nullptr;
  DateTime prevDate;
  while ((row = res.nextRow()))
  {
    carrierMix = QueryGetCarrierMixedClassHistoricalSQLStatement<
        QueryGetCarrierMixedClassHistorical>::mapRowToCarrierMixedClass(row,
                                                                        carrierMixPrev,
                                                                        prevDate);
    if (carrierMix != carrierMixPrev)
      mixedClasses.push_back(carrierMix);

    carrierMixPrev = carrierMix;
  }
  LOG4CXX_INFO(_logger,
               "GETCARRIERMIXEDCLASSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findCarrierMixedClass()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCarrierMixedClass
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCarrierMixedClass::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCarrierMixedClass"));
std::string QueryGetAllCarrierMixedClass::_baseSQL;
bool QueryGetAllCarrierMixedClass::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCarrierMixedClass> g_GetAllCarrierMixedClass;

const char*
QueryGetAllCarrierMixedClass::getQueryName() const
{
  return "GETALLCARRIERMIXEDCLASS";
}

void
QueryGetAllCarrierMixedClass::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCarrierMixedClassSQLStatement<QueryGetAllCarrierMixedClass> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCARRIERMIXEDCLASS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCarrierMixedClass::findAllCarrierMixedClass(
    std::vector<tse::CarrierMixedClass*>& mixedClasses)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierMixedClass* carrierMix = nullptr;
  tse::CarrierMixedClass* carrierMixPrev = nullptr;
  DateTime prevDate;
  while ((row = res.nextRow()))
  {
    carrierMix = QueryGetAllCarrierMixedClassSQLStatement<
        QueryGetAllCarrierMixedClass>::mapRowToCarrierMixedClass(row, carrierMixPrev, prevDate);
    if (carrierMix != carrierMixPrev)
      mixedClasses.push_back(carrierMix);

    carrierMixPrev = carrierMix;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCARRIERMIXEDCLASS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCarrierMixedClass()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCarrierMixedClassHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCarrierMixedClassHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCarrierMixedClassHistorical"));
std::string QueryGetAllCarrierMixedClassHistorical::_baseSQL;
bool QueryGetAllCarrierMixedClassHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCarrierMixedClassHistorical>
g_GetAllCarrierMixedClassHistorical;

const char*
QueryGetAllCarrierMixedClassHistorical::getQueryName() const
{
  return "GETALLCARRIERMIXEDCLASSHISTORICAL";
}

void
QueryGetAllCarrierMixedClassHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCarrierMixedClassHistoricalSQLStatement<QueryGetAllCarrierMixedClassHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCARRIERMIXEDCLASSHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCarrierMixedClassHistorical::findAllCarrierMixedClass(
    std::vector<tse::CarrierMixedClass*>& mixedClasses)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierMixedClass* carrierMix = nullptr;
  tse::CarrierMixedClass* carrierMixPrev = nullptr;
  DateTime prevDate;
  while ((row = res.nextRow()))
  {
    carrierMix = QueryGetAllCarrierMixedClassHistoricalSQLStatement<
        QueryGetAllCarrierMixedClassHistorical>::mapRowToCarrierMixedClass(row,
                                                                           carrierMixPrev,
                                                                           prevDate);
    if (carrierMix != carrierMixPrev)
      mixedClasses.push_back(carrierMix);

    carrierMixPrev = carrierMix;
  }
  LOG4CXX_INFO(_logger,
               "GETALLCARRIERMIXEDCLASSHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllCarrierMixedClass()
}
