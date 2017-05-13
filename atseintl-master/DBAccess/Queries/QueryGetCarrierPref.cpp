//----------------------------------------------------------------------------
//  File:           QueryGetCarrierPref.cpp
//  Description:    QueryGetCarrierPref
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

#include "DBAccess/Queries/QueryGetCarrierPref.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierPrefSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCarrierPref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierPref"));
std::string QueryGetCarrierPref::_baseSQL;
bool QueryGetCarrierPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierPref> g_GetCarrierPref;

log4cxx::LoggerPtr
QueryGetCarrierPrefFBRPref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCarrierPref.GetCarrierPrefFBRPref"));
std::string QueryGetCarrierPrefFBRPref::_baseSQL;
bool QueryGetCarrierPrefFBRPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierPrefFBRPref> g_GetCarrierPrefFBRPref;

const char*
QueryGetCarrierPref::getQueryName() const
{
  return "GETCARRIERPREF";
};

void
QueryGetCarrierPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERPREF");
    substTableDef(&_baseSQL);

    QueryGetCarrierPrefFBRPref::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierPref::findCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs,
                                     const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  fbrPref cxrFbrPref;
  CarrierPreference::CombPref combPref;

  std::vector<fbrPref> fbrPrefs;
  QueryGetCarrierPrefFBRPref SQLPrefFBRPref(_dbAdapt);
  SQLPrefFBRPref.getFbrPrefs(fbrPrefs, carrier);

  resetSQL();

  substParm(1, carrier);
  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierPreference* cxrPref = nullptr;
  CarrierCode prevCxr;
  DateTime prevDate, createDate;
  std::vector<fbrPref>::iterator i = fbrPrefs.begin();
  while ((row = res.nextRow()))
  {
    CarrierCode cxr = QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getCarrier(row);
    createDate = QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getCreateDate(row);
    if (cxrPref == nullptr || cxr != prevCxr || createDate.get64BitRep() != prevDate.get64BitRep())
    {
      cxrPref = QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::mapRowToCarrierPref(row);
      cxrprefs.push_back(cxrPref);
      for (; i != fbrPrefs.end(); ++i)
      {
        if (i->cxr > cxr)
          break;
        if (i->cxr == cxr)
        {
          if (i->createDate.get64BitRep() > createDate.get64BitRep())
            break;
          if (i->createDate.get64BitRep() == createDate.get64BitRep())
            cxrPref->fbrPrefs().push_back(i->vendor);
        }
      }
      prevCxr = cxr;
      prevDate = createDate;
    }
    if (!QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::isNullPublicPrivate1(row))
    {
      std::string vendor1 = QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getVendor1(row);
      std::string vendor2 = QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getVendor2(row);
      if (vendor1 <= vendor2)
      {
        combPref.publicPrivate1() =
            QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getPublicPrivate1(row);
        combPref.vendor1() = vendor1;
        combPref.publicPrivate2() =
            QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getPublicPrivate2(row);
        combPref.vendor2() = vendor2;
      }
      else
      {
        combPref.publicPrivate1() =
            QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getPublicPrivate2(row);
        combPref.vendor1() = vendor2;
        combPref.publicPrivate2() =
            QueryGetCarrierPrefSQLStatement<QueryGetCarrierPref>::getPublicPrivate1(row);
        combPref.vendor2() = vendor1;
      }
      cxrPref->combPrefs().push_back(combPref);
    }
  } // while ((row = res.nextRow()))

  LOG4CXX_INFO(_logger,
               "GETCARRIERPREF: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                            << stopCPU() << ")");
  res.freeResult();
} // QueryGetCarrierPref::findCarrierPref()
///////////////////////////////////////////////////////////
//
//  QueryGetCarrierPrefHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetCarrierPrefHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.QueryGetCarrierPrefHistorical"));
std::string QueryGetCarrierPrefHistorical::_baseSQL;
bool QueryGetCarrierPrefHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetCarrierPrefHistorical> g_GetCarrierPrefHistorical;

const char*
QueryGetCarrierPrefHistorical::getQueryName() const
{
  return "GETCARRIERPREFHISTORICAL";
};

void
QueryGetCarrierPrefHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierPrefHistoricalSQLStatement<QueryGetCarrierPrefHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERPREFHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetCarrierPrefFBRPref::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierPrefHistorical::findCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs,
                                               const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  fbrPref cxrFbrPref;
  CarrierPreference::CombPref combPref;

  std::vector<fbrPref> fbrPrefs;
  QueryGetCarrierPrefFBRPref SQLPrefFBRPref(_dbAdapt);
  SQLPrefFBRPref.getFbrPrefs(fbrPrefs, carrier);

  resetSQL();

  substParm(1, carrier);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierPreference* cxrPref = nullptr;
  CarrierCode prevCxr;
  DateTime prevDate, createDate;
  std::vector<fbrPref>::iterator i = fbrPrefs.begin();
  while ((row = res.nextRow()))
  {
    CarrierCode cxr =
        QueryGetCarrierPrefHistoricalSQLStatement<QueryGetCarrierPrefHistorical>::getCarrier(row);
    createDate =
        QueryGetCarrierPrefHistoricalSQLStatement<QueryGetCarrierPrefHistorical>::getCreateDate(
            row);
    if (cxrPref == nullptr || cxr != prevCxr || createDate.get64BitRep() != prevDate.get64BitRep())
    {
      cxrPref = QueryGetCarrierPrefHistoricalSQLStatement<
          QueryGetCarrierPrefHistorical>::mapRowToCarrierPref(row);
      cxrprefs.push_back(cxrPref);
      for (; i != fbrPrefs.end(); ++i)
      {
        if (i->cxr > cxr)
          break;
        if (i->cxr == cxr)
        {
          if (i->createDate.get64BitRep() > createDate.get64BitRep())
            break;
          if (i->createDate.get64BitRep() == createDate.get64BitRep())
            cxrPref->fbrPrefs().push_back(i->vendor);
        }
      }
      prevCxr = cxr;
      prevDate = createDate;
    }
    if (!QueryGetCarrierPrefHistoricalSQLStatement<
            QueryGetCarrierPrefHistorical>::isNullPublicPrivate1(row))
    {
      std::string vendor1 =
          QueryGetCarrierPrefHistoricalSQLStatement<QueryGetCarrierPrefHistorical>::getVendor1(row);
      std::string vendor2 =
          QueryGetCarrierPrefHistoricalSQLStatement<QueryGetCarrierPrefHistorical>::getVendor2(row);
      if (vendor1 <= vendor2)
      {
        combPref.publicPrivate1() = QueryGetCarrierPrefHistoricalSQLStatement<
            QueryGetCarrierPrefHistorical>::getPublicPrivate1(row);
        combPref.vendor1() = vendor1;
        combPref.publicPrivate2() = QueryGetCarrierPrefHistoricalSQLStatement<
            QueryGetCarrierPrefHistorical>::getPublicPrivate2(row);
        combPref.vendor2() = vendor2;
      }
      else
      {
        combPref.publicPrivate1() = QueryGetCarrierPrefHistoricalSQLStatement<
            QueryGetCarrierPrefHistorical>::getPublicPrivate2(row);
        combPref.vendor1() = vendor2;
        combPref.publicPrivate2() = QueryGetCarrierPrefHistoricalSQLStatement<
            QueryGetCarrierPrefHistorical>::getPublicPrivate1(row);
        combPref.vendor2() = vendor1;
      }
      cxrPref->combPrefs().push_back(combPref);
    }
  } // while ((row = res.nextRow()))

  LOG4CXX_INFO(_logger,
               "GETCARRIERPREFHISTORICAL: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetCarrierPref::findCarrierPref()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCarrierPref
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCarrierPref::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCarrierPref"));
std::string QueryGetAllCarrierPref::_baseSQL;
bool QueryGetAllCarrierPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCarrierPref> g_GetAllCarrierPref;

log4cxx::LoggerPtr
QueryGetAllCarrierPrefFBRPref::_logger(log4cxx::Logger::getLogger(
    "atseintl.DBAccess.SQLQuery.QueryGetAllCarrierPref.GetAllCarrierPrefFBRPref"));
std::string QueryGetAllCarrierPrefFBRPref::_baseSQL;
bool QueryGetAllCarrierPrefFBRPref::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCarrierPrefFBRPref> g_GetAllCarrierPrefFBRPref;

const char*
QueryGetAllCarrierPref::getQueryName() const
{
  return "GETALLCARRIERPREF";
};

void
QueryGetAllCarrierPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCARRIERPREF");
    substTableDef(&_baseSQL);

    QueryGetAllCarrierPrefFBRPref::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCarrierPref::findAllCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  fbrPref cxrFbrPref;
  CarrierPreference::CombPref combPref;

  std::vector<fbrPref> fbrPrefs;
  QueryGetAllCarrierPrefFBRPref SQLAllPrefFBRPref(_dbAdapt);
  SQLAllPrefFBRPref.getAllFbrPrefs(fbrPrefs);

  resetSQL();

  substCurrentDate();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierPreference* cxrPref = nullptr;
  CarrierCode prevCxr;
  DateTime prevDate, createDate;
  std::vector<fbrPref>::iterator i = fbrPrefs.begin();
  while ((row = res.nextRow()))
  {
    CarrierCode cxr = QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getCarrier(row);
    createDate = QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getCreateDate(row);
    if (cxrPref == nullptr || cxr != prevCxr || createDate.get64BitRep() != prevDate.get64BitRep())
    {
      cxrPref =
          QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::mapRowToCarrierPref(row);
      cxrprefs.push_back(cxrPref);
      for (; i != fbrPrefs.end(); ++i)
      {
        if (i->cxr > cxr)
          break;
        if (i->cxr == cxr)
        {
          if (i->createDate.get64BitRep() > createDate.get64BitRep())
            break;
          if (i->createDate.get64BitRep() == createDate.get64BitRep())
            cxrPref->fbrPrefs().push_back(i->vendor);
        }
      }
      prevCxr = cxr;
      prevDate = createDate;
    }
    if (!QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::isNullPublicPrivate1(row))
    {
      std::string vendor1 =
          QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getVendor1(row);
      std::string vendor2 =
          QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getVendor2(row);
      if (vendor1 <= vendor2)
      {
        combPref.publicPrivate1() =
            QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getPublicPrivate1(row);
        combPref.vendor1() = vendor1;
        combPref.publicPrivate2() =
            QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getPublicPrivate2(row);
        combPref.vendor2() = vendor2;
      }
      else
      {
        combPref.publicPrivate1() =
            QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getPublicPrivate2(row);
        combPref.vendor1() = vendor2;
        combPref.publicPrivate2() =
            QueryGetAllCarrierPrefSQLStatement<QueryGetAllCarrierPref>::getPublicPrivate1(row);
        combPref.vendor2() = vendor1;
      }
      cxrPref->combPrefs().push_back(combPref);
    }
  }

  LOG4CXX_INFO(_logger,
               "GETALLCARRIERPREF: NumRows = " << res.numRows() << " Time = " << stopTimer() << " ("
                                               << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllCarrierPref::findAllCarrierPref()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCarrierPrefHistorical
//
///////////////////////////////////////////////////////////
log4cxx::LoggerPtr
QueryGetAllCarrierPrefHistorical::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllCarrierPrefHistorical"));
std::string QueryGetAllCarrierPrefHistorical::_baseSQL;
bool QueryGetAllCarrierPrefHistorical::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllCarrierPrefHistorical> g_GetAllCarrierPrefHistorical;

const char*
QueryGetAllCarrierPrefHistorical::getQueryName() const
{
  return "GETALLCARRIERPREFHISTORICAL";
};

void
QueryGetAllCarrierPrefHistorical::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCarrierPrefHistoricalSQLStatement<QueryGetAllCarrierPrefHistorical> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCARRIERPREFHISTORICAL");
    substTableDef(&_baseSQL);

    QueryGetAllCarrierPrefFBRPref::initialize();
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCarrierPrefHistorical::findAllCarrierPref(std::vector<tse::CarrierPreference*>& cxrprefs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  fbrPref cxrFbrPref;
  CarrierPreference::CombPref combPref;

  std::vector<fbrPref> fbrPrefs;
  QueryGetAllCarrierPrefFBRPref SQLAllPrefFBRPref(_dbAdapt);
  SQLAllPrefFBRPref.getAllFbrPrefs(fbrPrefs);

  resetSQL();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  tse::CarrierPreference* cxrPref = nullptr;
  CarrierCode prevCxr;
  DateTime prevDate, createDate;
  std::vector<fbrPref>::iterator i = fbrPrefs.begin();
  while ((row = res.nextRow()))
  {
    CarrierCode cxr =
        QueryGetAllCarrierPrefHistoricalSQLStatement<QueryGetAllCarrierPrefHistorical>::getCarrier(
            row);
    createDate = QueryGetAllCarrierPrefHistoricalSQLStatement<
        QueryGetAllCarrierPrefHistorical>::getCreateDate(row);
    if (cxrPref == nullptr || cxr != prevCxr || createDate.get64BitRep() != prevDate.get64BitRep())
    {
      cxrPref = QueryGetAllCarrierPrefHistoricalSQLStatement<
          QueryGetAllCarrierPrefHistorical>::mapRowToCarrierPref(row);
      cxrprefs.push_back(cxrPref);
      for (; i != fbrPrefs.end(); ++i)
      {
        if (i->cxr > cxr)
          break;
        if (i->cxr == cxr)
        {
          if (i->createDate.get64BitRep() > createDate.get64BitRep())
            break;
          if (i->createDate.get64BitRep() == createDate.get64BitRep())
            cxrPref->fbrPrefs().push_back(i->vendor);
        }
      }
      prevCxr = cxr;
      prevDate = createDate;
    }
    if (!QueryGetAllCarrierPrefHistoricalSQLStatement<
            QueryGetAllCarrierPrefHistorical>::isNullPublicPrivate1(row))
    {
      std::string vendor1 = QueryGetAllCarrierPrefHistoricalSQLStatement<
          QueryGetAllCarrierPrefHistorical>::getVendor1(row);
      std::string vendor2 = QueryGetAllCarrierPrefHistoricalSQLStatement<
          QueryGetAllCarrierPrefHistorical>::getVendor2(row);
      if (vendor1 <= vendor2)
      {
        combPref.publicPrivate1() = QueryGetAllCarrierPrefHistoricalSQLStatement<
            QueryGetAllCarrierPrefHistorical>::getPublicPrivate1(row);
        combPref.vendor1() = vendor1;
        combPref.publicPrivate2() = QueryGetAllCarrierPrefHistoricalSQLStatement<
            QueryGetAllCarrierPrefHistorical>::getPublicPrivate2(row);
        combPref.vendor2() = vendor2;
      }
      else
      {
        combPref.publicPrivate1() = QueryGetAllCarrierPrefHistoricalSQLStatement<
            QueryGetAllCarrierPrefHistorical>::getPublicPrivate2(row);
        combPref.vendor1() = vendor2;
        combPref.publicPrivate2() = QueryGetAllCarrierPrefHistoricalSQLStatement<
            QueryGetAllCarrierPrefHistorical>::getPublicPrivate1(row);
        combPref.vendor2() = vendor1;
      }
      cxrPref->combPrefs().push_back(combPref);
    }
  }

  LOG4CXX_INFO(_logger,
               "GETALLCARRIERPREFHISTORICAL: NumRows = "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllCarrierPref::findAllCarrierPref()

///////////////////////////////////////////////////////////
//
//  QueryGetCarrierPrefFBRPref
//
///////////////////////////////////////////////////////////

const char*
QueryGetCarrierPrefFBRPref::getQueryName() const
{
  return "GETCARRIERPREFFBRPREF";
};

void
QueryGetCarrierPrefFBRPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCarrierPrefFBRPrefSQLStatement<QueryGetCarrierPrefFBRPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCARRIERPREFFBRPREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetCarrierPrefFBRPref::getFbrPrefs(std::vector<fbrPref>& fbrPrefs, const CarrierCode& carrier)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  substParm(carrier, 1);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fbrPref newFbrPref;
    QueryGetCarrierPrefFBRPrefSQLStatement<QueryGetCarrierPrefFBRPref>::mapRowToFbrPref(row,
                                                                                        newFbrPref);
    fbrPrefs.push_back(newFbrPref);
  } // while (Fetching Tax Carrier Flt Segs)
  LOG4CXX_INFO(_logger,
               "GETCARRIERPREFFBRPREF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetCarrierPrefFBRPref::getFbrPrefs()

///////////////////////////////////////////////////////////
//
//  QueryGetAllCarrierPrefFBRPref
//
///////////////////////////////////////////////////////////

const char*
QueryGetAllCarrierPrefFBRPref::getQueryName() const
{
  return "GETALLCARRIERPREFFBRPREF";
};

void
QueryGetAllCarrierPrefFBRPref::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllCarrierPrefFBRPrefSQLStatement<QueryGetAllCarrierPrefFBRPref> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLCARRIERPREFFBRPREF");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllCarrierPrefFBRPref::getAllFbrPrefs(std::vector<fbrPref>& fbrPrefs)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  resetSQL();

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    fbrPref newFbrPref;
    QueryGetAllCarrierPrefFBRPrefSQLStatement<QueryGetAllCarrierPrefFBRPref>::mapRowToFbrPref(
        row, newFbrPref);
    fbrPrefs.push_back(newFbrPref);
  } // while (Fetching Tax Carrier Flt Segs)
  LOG4CXX_INFO(_logger,
               "GETALLCARRIERPREFFBRPREF: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                      << " (" << stopCPU() << ")");
  res.freeResult();
} // QueryGetAllCarrierPrefFBRPref::getAllFbrPrefs()
}
