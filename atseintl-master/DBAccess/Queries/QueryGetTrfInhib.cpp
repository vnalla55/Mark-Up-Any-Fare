//----------------------------------------------------------------------------
//  File:           QueryGetTrfInhib.cpp
//  Description:    QueryGetTrfInhib
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

#include "DBAccess/Queries/QueryGetTrfInhib.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTrfInhibSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetTrfInhib::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetTrfInhib"));
std::string QueryGetTrfInhib::_baseSQL;
bool QueryGetTrfInhib::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetTrfInhib> g_GetTrfInhib;

const char*
QueryGetTrfInhib::getQueryName() const
{
  return "GETTRFINHIB";
}

void
QueryGetTrfInhib::initialize()
{
  if (!_isInitialized)
  {
    QueryGetTrfInhibSQLStatement<QueryGetTrfInhib> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETTRFINHIB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetTrfInhib::findTariffInhibits(std::vector<tse::TariffInhibits*>& lstTI,
                                     const VendorCode& vendor,
                                     const Indicator tariffCrossRefType,
                                     const CarrierCode& carrier,
                                     const TariffNumber& fareTariff,
                                     const TariffCode& ruleTariffCode)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, vendor);
  substParm(2, tariffCrossRefType);
  substParm(3, carrier);
  substParm(4, fareTariff);
  substParm(5, ruleTariffCode);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTI.push_back(QueryGetTrfInhibSQLStatement<QueryGetTrfInhib>::mapRowToTariffInhibits(row));
  }
  LOG4CXX_INFO(_logger,
               "GETTRFINHIB: NumRows " << res.numRows() << " Time = " << stopTimer() << " ("
                                       << stopCPU() << ") mSecs");
  res.freeResult();
} // findTariffInhibits()

///////////////////////////////////////////////////////////
//
//  QueryGetAllTrfInhib
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllTrfInhib::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllTrfInhib"));
std::string QueryGetAllTrfInhib::_baseSQL;
bool QueryGetAllTrfInhib::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllTrfInhib> g_GetAllTrfInhib;

const char*
QueryGetAllTrfInhib::getQueryName() const
{
  return "GETALLTRFINHIB";
}

void
QueryGetAllTrfInhib::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllTrfInhibSQLStatement<QueryGetAllTrfInhib> sqlStatement;

    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLTRFINHIB");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
tse::QueryGetAllTrfInhib::findAllTariffInhibits(std::vector<tse::TariffInhibits*>& lstTI)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    lstTI.push_back(
        QueryGetAllTrfInhibSQLStatement<QueryGetAllTrfInhib>::mapRowToTariffInhibits(row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLTRFINHIB: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // findAllTariffInhibits()
}
