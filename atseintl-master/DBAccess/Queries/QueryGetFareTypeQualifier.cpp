//----------------------------------------------------------------------------
//          File:           QueryGetFareTypeQualifier.cpp
//          Description:    QueryGetFareTypeQualifier
//          Created:        03/06/2007
//          Authors:        Quan Ta
//
//          Updates:
//
// � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
// the confidential and proprietary product of Sabre Inc. Any unauthorized
// use, reproduction, or transfer of this software/documentation, in any
// medium, or incorporation of this software/documentation into any system or
// publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetFareTypeQualifier.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeQualifierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetFareTypeQualPsg::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareTypeQualPsg"));
std::string QueryGetFareTypeQualPsg::_baseSQL;
bool QueryGetFareTypeQualPsg::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeQualPsg> g_GetFareTypeQualPsg;

const char*
QueryGetFareTypeQualPsg::getQueryName() const
{
  return "GETFARETYPEQUALPSG";
}

void
QueryGetFareTypeQualPsg::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeQualPsgSQLStatement<QueryGetFareTypeQualPsg> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEQUALPSG");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareTypeQualPsg::findFareTypeQualPsg(FareTypeQualifier& ftq)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, ftq.userApplType());
  substParm(2, ftq.userAppl());
  substParm(3, ftq.fareTypeQualifier().c_str());
  substParm(4, ftq.journeyTypeDom());
  substParm(5, ftq.journeyTypeIntl());
  substParm(6, ftq.journeyTypeEoe());
  substParm(7, ftq.pricingUnitDom());
  substParm(8, ftq.pricingUnitIntl());
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    QueryGetFareTypeQualPsgSQLStatement<QueryGetFareTypeQualPsg>::mapRowToFareTypeQualPsg(ftq, row);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ") mSecs");

  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetFareTypeQualifier
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareTypeQualifier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareTypeQualifier"));
std::string QueryGetFareTypeQualifier::_baseSQL;
bool QueryGetFareTypeQualifier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeQualifier> g_GetFareTypeQualifier;

const char*
QueryGetFareTypeQualifier::getQueryName() const
{
  return "GETFARETYPEQUALIFIER";
}

void
QueryGetFareTypeQualifier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEQUALIFIER");
    substTableDef(&_baseSQL);
    QueryGetFareTypeQualPsg::initialize();
    _isInitialized = true;
  }
}

void
QueryGetFareTypeQualifier::findFareTypeQualifier(std::vector<tse::FareTypeQualifier*>& ftqList,
                                                 const Indicator userApplType,
                                                 const UserApplCode& userAppl,
                                                 const FareType& qualifier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, qualifier.c_str());
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareTypeQualifier* prev = nullptr;
  while ((row = res.nextRow()))
  {
    FareTypeQualifier fareTypeQualifier;
    QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier>::mapRowToFareTypeQualifier(
        fareTypeQualifier, row);

    if (!prev || *prev != fareTypeQualifier)
    {
      prev = new FareTypeQualifier(fareTypeQualifier);
      ftqList.push_back(prev);
    }

    // Adding Msg
    FareType fareType =
        row->getString(QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier>::FARETYPE);

    FareTypeQualMsg qualifierMsg;
    qualifierMsg.fareTypeReqInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier>::FARETYPEREQIND);
    qualifierMsg.groupTrailerMsgInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier>::GROUPTRAILERMSGIND);
    qualifierMsg.itTrailerMsgInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetFareTypeQualifier>::ITTRAILERMSGIND);

    prev->addQualMsg(fareType, qualifierMsg);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ") mSecs");

  res.freeResult();

  findFareTypeQualPsg(ftqList);

} // findFareTypeQualifier()

void
QueryGetFareTypeQualifier::findFareTypeQualPsg(std::vector<FareTypeQualifier*>& ftqList)
{
  for (const auto elem : ftqList)
  {
    QueryGetFareTypeQualPsg queryFtqPsg(_dbAdapt);
    queryFtqPsg.findFareTypeQualPsg(*elem);
  }
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareTypeQualifier
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareTypeQualifier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareTypeQualifier"));
std::string QueryGetAllFareTypeQualifier::_baseSQL;
bool QueryGetAllFareTypeQualifier::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareTypeQualifier> g_GetAllFareTypeQualifier;

const char*
QueryGetAllFareTypeQualifier::getQueryName() const
{
  return "GETALLFARETYPEQUALIFIER";
}

void
QueryGetAllFareTypeQualifier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareTypeQualifierSQLStatement<QueryGetAllFareTypeQualifier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARETYPEQUALIFIER");
    substTableDef(&_baseSQL);
    QueryGetFareTypeQualPsg::initialize();
    _isInitialized = true;
  }
}

void
QueryGetAllFareTypeQualifier::findAllFareTypeQualifier(
    std::vector<tse::FareTypeQualifier*>& ftqList)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareTypeQualifier* prev = nullptr;
  while ((row = res.nextRow()))
  {
    FareTypeQualifier fareTypeQualifier;
    QueryGetAllFareTypeQualifierSQLStatement<
        QueryGetAllFareTypeQualifier>::mapRowToFareTypeQualifier(fareTypeQualifier, row);

    if (!prev || *prev != fareTypeQualifier)
    {
      prev = new FareTypeQualifier(fareTypeQualifier);
      ftqList.push_back(prev);
    }

    // Adding Msg
    FareType fareType = row->getString(
        QueryGetFareTypeQualifierSQLStatement<QueryGetAllFareTypeQualifier>::FARETYPE);

    FareTypeQualMsg qualifierMsg;
    qualifierMsg.fareTypeReqInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetAllFareTypeQualifier>::FARETYPEREQIND);
    qualifierMsg.groupTrailerMsgInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetAllFareTypeQualifier>::GROUPTRAILERMSGIND);
    qualifierMsg.itTrailerMsgInd() = row->getChar(
        QueryGetFareTypeQualifierSQLStatement<QueryGetAllFareTypeQualifier>::ITTRAILERMSGIND);

    prev->addQualMsg(fareType, qualifierMsg);
  }

  LOG4CXX_INFO(_logger,
               "GetAllFareTypeQualifier: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");

  res.freeResult();

  findFareTypeQualPsg(ftqList);

} // findAllFareTypeQualifier()

///////////////////////////////////////////////////////////
//
//  QueryGetFareTypeQualPsgHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareTypeQualPsgHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetFareTypeQualPsgHistorical"));
std::string QueryGetFareTypeQualPsgHistorical::_baseSQL;
bool QueryGetFareTypeQualPsgHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeQualPsgHistorical> g_GetFareTypeQualPsgHistorical;

const char*
QueryGetFareTypeQualPsgHistorical::getQueryName() const
{
  return "GETFARETYPEQUALPSGHISTORICAL";
}

void
QueryGetFareTypeQualPsgHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeQualPsgHistoricalSQLStatement<QueryGetFareTypeQualPsgHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEQUALPSGHISTORICAL");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}

void
QueryGetFareTypeQualPsgHistorical::findFareTypeQualPsg(FareTypeQualifier& ftq)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, ftq.userApplType());
  substParm(2, ftq.userAppl());
  substParm(3, ftq.fareTypeQualifier().c_str());
  substParm(4, ftq.journeyTypeDom());
  substParm(5, ftq.journeyTypeIntl());
  substParm(6, ftq.journeyTypeEoe());
  substParm(7, ftq.pricingUnitDom());
  substParm(8, ftq.pricingUnitIntl());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  while ((row = res.nextRow()))
  {
    QueryGetFareTypeQualPsgSQLStatement<QueryGetFareTypeQualPsg>::mapRowToFareTypeQualPsg(ftq, row);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ") mSecs");

  res.freeResult();
}

///////////////////////////////////////////////////////////
//
//  QueryGetFareTypeQualifierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetFareTypeQualifierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetFareTypeQualifierHistorical"));
std::string QueryGetFareTypeQualifierHistorical::_baseSQL;
bool QueryGetFareTypeQualifierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetFareTypeQualifierHistorical> g_GetFareTypeQualifierHistorical;

const char*
QueryGetFareTypeQualifierHistorical::getQueryName() const
{
  return "GETFARETYPEQUALIFIERHISTORICAL";
}

void
QueryGetFareTypeQualifierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetFareTypeQualifierHistoricalSQLStatement<QueryGetFareTypeQualifierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETFARETYPEQUALIFIERHISTORICAL");
    substTableDef(&_baseSQL);
    QueryGetFareTypeQualPsgHistorical::initialize();
    _isInitialized = true;
  }
}

void
QueryGetFareTypeQualifierHistorical::findFareTypeQualifier(
    std::vector<tse::FareTypeQualifier*>& ftqList,
    const Indicator userApplType,
    const UserApplCode& userAppl,
    const FareType& qualifier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  substParm(1, userApplType);
  substParm(2, userAppl);
  substParm(3, qualifier.c_str());
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareTypeQualifier* prev = nullptr;
  while ((row = res.nextRow()))
  {
    FareTypeQualifier fareTypeQualifier;
    QueryGetFareTypeQualifierHistoricalSQLStatement<
        QueryGetFareTypeQualifierHistorical>::mapRowToFareTypeQualifier(fareTypeQualifier, row);

    if (!prev || *prev != fareTypeQualifier)
    {
      prev = new FareTypeQualifier(fareTypeQualifier);
      ftqList.push_back(prev);
    }

    // Adding Msg
    FareType fareType = row->getString(QueryGetFareTypeQualifierHistoricalSQLStatement<
        QueryGetFareTypeQualifierHistorical>::FARETYPE);

    FareTypeQualMsg qualifierMsg;
    qualifierMsg.fareTypeReqInd() = row->getChar(QueryGetFareTypeQualifierHistoricalSQLStatement<
        QueryGetFareTypeQualifierHistorical>::FARETYPEREQIND);
    qualifierMsg.groupTrailerMsgInd() =
        row->getChar(QueryGetFareTypeQualifierHistoricalSQLStatement<
            QueryGetFareTypeQualifierHistorical>::GROUPTRAILERMSGIND);
    qualifierMsg.itTrailerMsgInd() = row->getChar(QueryGetFareTypeQualifierHistoricalSQLStatement<
        QueryGetFareTypeQualifierHistorical>::ITTRAILERMSGIND);

    prev->addQualMsg(fareType, qualifierMsg);
  }

  LOG4CXX_INFO(_logger,
               getQueryName() << ": NumRows = " << res.numRows() << " Time = " << stopTimer()
                              << " (" << stopCPU() << ") mSecs");

  res.freeResult();

  findFareTypeQualPsg(ftqList);

} // findFareTypeQualifier()

void
QueryGetFareTypeQualifierHistorical::findFareTypeQualPsg(std::vector<FareTypeQualifier*>& ftqList)
{
  for (const auto elem : ftqList)
  {
    QueryGetFareTypeQualPsgHistorical queryFtqPsg(_dbAdapt);
    queryFtqPsg.findFareTypeQualPsg(*elem);
  }
}

///////////////////////////////////////////////////////////
//
//  QueryGetAllFareTypeQualifierHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllFareTypeQualifierHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllFareTypeQualifierHistorical"));
std::string QueryGetAllFareTypeQualifierHistorical::_baseSQL;
bool QueryGetAllFareTypeQualifierHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllFareTypeQualifierHistorical>
g_GetAllFareTypeQualifierHistorical;

const char*
QueryGetAllFareTypeQualifierHistorical::getQueryName() const
{
  return "GETALLFARETYPEQUALIFIERHISTORICAL";
}

void
QueryGetAllFareTypeQualifierHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllFareTypeQualifierHistoricalSQLStatement<QueryGetAllFareTypeQualifierHistorical>
    sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLFARETYPEQUALIFIERHISTORICAL");
    substTableDef(&_baseSQL);
    QueryGetFareTypeQualPsgHistorical::initialize();
    _isInitialized = true;
  }
}

void
QueryGetAllFareTypeQualifierHistorical::findAllFareTypeQualifier(
    std::vector<tse::FareTypeQualifier*>& ftqList)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  FareTypeQualifier* prev = nullptr;
  while ((row = res.nextRow()))
  {
    FareTypeQualifier fareTypeQualifier;
    QueryGetAllFareTypeQualifierHistoricalSQLStatement<
        QueryGetAllFareTypeQualifierHistorical>::mapRowToFareTypeQualifier(fareTypeQualifier, row);

    if (!prev || *prev != fareTypeQualifier)
    {
      prev = new FareTypeQualifier(fareTypeQualifier);
      ftqList.push_back(prev);
    }

    // Adding Msg
    FareType fareType = row->getString(QueryGetAllFareTypeQualifierHistoricalSQLStatement<
        QueryGetAllFareTypeQualifierHistorical>::FARETYPE);

    FareTypeQualMsg qualifierMsg;
    qualifierMsg.fareTypeReqInd() = row->getChar(QueryGetAllFareTypeQualifierHistoricalSQLStatement<
        QueryGetAllFareTypeQualifierHistorical>::FARETYPEREQIND);
    qualifierMsg.groupTrailerMsgInd() =
        row->getChar(QueryGetAllFareTypeQualifierHistoricalSQLStatement<
            QueryGetAllFareTypeQualifierHistorical>::GROUPTRAILERMSGIND);
    qualifierMsg.itTrailerMsgInd() =
        row->getChar(QueryGetAllFareTypeQualifierHistoricalSQLStatement<
            QueryGetAllFareTypeQualifierHistorical>::ITTRAILERMSGIND);

    prev->addQualMsg(fareType, qualifierMsg);
  }

  LOG4CXX_INFO(_logger,
               "GetAllFareTypeQualifier: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");

  res.freeResult();

  findFareTypeQualPsg(ftqList);

} // findAllFareTypeQualifier()
}
