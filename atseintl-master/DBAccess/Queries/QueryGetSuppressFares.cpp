//----------------------------------------------------------------------------
//  File:           QueryGetSuppressFares.cpp
//  Description:    QueryGetSuppressFares
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

#include "DBAccess/Queries/QueryGetSuppressFares.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSuppressFaresSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetSuppressFarePccCc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSuppressFarePccCc"));
std::string QueryGetSuppressFarePccCc::_baseSQL;
bool QueryGetSuppressFarePccCc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSuppressFarePccCc> g_GetSuppressFarePccCc;

log4cxx::LoggerPtr
QueryGetSuppressFarePcc::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetSuppressFarePcc"));
std::string QueryGetSuppressFarePcc::_baseSQL;
bool QueryGetSuppressFarePcc::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetSuppressFarePcc> g_GetSuppressFarePcc;

const char*
QueryGetSuppressFarePccCc::getQueryName() const
{
  return "GETSUPPRESSFARE_PCC_CC";
};

void
QueryGetSuppressFarePccCc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSuppressFarePccCcSQLStatement<QueryGetSuppressFarePccCc> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSUPPRESSFARE_PCC_CC");
    substTableDef(&_baseSQL);

    QueryGetSuppressFarePcc::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSuppressFarePccCc::findSuppressFares(std::vector<const tse::FDSuppressFare*>& lstSF,
                                             const PseudoCityCode& pseudoCityCode,
                                             const Indicator pseudoCityType,
                                             const TJRGroup& ssgGroupNo,
                                             const CarrierCode& carrier)
{
  if (carrier.empty())
  { // No carrier, so use alternate find()
    QueryGetSuppressFarePcc altQry(_dbAdapt);
    altQry.findSuppressFares(lstSF, pseudoCityCode, pseudoCityType, ssgGroupNo);
    return;
  }

  // Got a carrier, so going with original query
  Row* row;
  DBResultSet res(_dbAdapt);
  char pccTypeStr[2];
  sprintf(pccTypeStr, "%c", pseudoCityType);
  substParm(1, pseudoCityCode.substr(0, 4));
  substParm(2, carrier.substr(0, 2));
  substParm(3, pccTypeStr);
  substParm(4, ssgGroupNo);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::FDSuppressFare* sf =
        QueryGetSuppressFarePccCcSQLStatement<QueryGetSuppressFarePccCc>::mapRowToFDSuppressFare(
            row);
    LOG4CXX_INFO(_logger,
                 "pcc:" << sf->pseudoCityCode() << ",pccType:" << sf->pseudoCityType()
                        << ",Carrier:" << sf->carrier());
    LOG4CXX_INFO(_logger,
                 "FareDisplayType:" << sf->fareDisplayType() << ",Dir:" << sf->directionality());
    LOG4CXX_INFO(_logger, "loc1:" << sf->loc1().loc() << ",loc1type:" << sf->loc1().locType());
    LOG4CXX_INFO(_logger, "loc2:" << sf->loc2().loc() << ",loc2type:" << sf->loc2().locType());

    LOG4CXX_INFO(_logger, "Leaving QueryGetSuppressFarePccCc::mapRowToFDSuppressFare");
    lstSF.push_back(sf);
  }

  LOG4CXX_INFO(_logger,
               "GETSUPPRESSFARE_PCC_CC: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                    << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  return;
} // QueryGetSuppressFarePccCc::findSuppressFares()

///////////////////////////////////////////////////////////
//
//  QueryGetSuppressFarePcc
//
///////////////////////////////////////////////////////////
const char*
QueryGetSuppressFarePcc::getQueryName() const
{
  return "GETSUPPRESSFARE_PCC";
}

void
QueryGetSuppressFarePcc::initialize()
{
  if (!_isInitialized)
  {
    QueryGetSuppressFarePccSQLStatement<QueryGetSuppressFarePcc> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETSUPPRESSFARE_PCC");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetSuppressFarePcc::findSuppressFares(std::vector<const tse::FDSuppressFare*>& lstSF,
                                           const PseudoCityCode& pseudoCityCode,
                                           const Indicator pseudoCityType,
                                           const TJRGroup& ssgGroupNo)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  char pccTypeStr[2];
  sprintf(pccTypeStr, "%c", pseudoCityType);

  resetSQL();

  substParm(1, pseudoCityCode.substr(0, 4));
  substParm(2, pccTypeStr);
  substParm(3, ssgGroupNo);
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    tse::FDSuppressFare* sf =
        QueryGetSuppressFarePccSQLStatement<QueryGetSuppressFarePcc>::mapRowToFDSuppressFare(row);
    LOG4CXX_INFO(_logger,
                 "pcc:" << sf->pseudoCityCode() << ",pccType:" << sf->pseudoCityType()
                        << ",Carrier:" << sf->carrier());
    LOG4CXX_INFO(_logger,
                 "FareDisplayType:" << sf->fareDisplayType() << ",Dir:" << sf->directionality());
    LOG4CXX_INFO(_logger, "loc1:" << sf->loc1().loc() << ",loc1type:" << sf->loc1().locType());
    LOG4CXX_INFO(_logger, "loc2:" << sf->loc2().loc() << ",loc2type:" << sf->loc2().locType());
    lstSF.push_back(sf);
  }

  LOG4CXX_INFO(_logger,
               "GETSUPPRESSFARE_PCC: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                 << " (" << stopCPU() << ") mSecs");
  res.freeResult();
  return;
} // QueryGetSuppressFarePcc::findSuppressFares()
}
