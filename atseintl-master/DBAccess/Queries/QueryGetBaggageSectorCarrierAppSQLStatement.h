//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/BaggageSectorCarrierApp.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBaggageSectorCarrierAppSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBaggageSectorCarrierAppSQLStatement() {};
  virtual ~QueryGetBaggageSectorCarrierAppSQLStatement() {};

  enum ColumnIndexes
  {
    MKTGCARRIER,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    OPTGCARRIER,
    INCLEXCLIND,
    BRDLOCTYPE,
    BRDLOC,
    OFFLOCTYPE,
    OFFLOC,
    FLT1NO,
    FLT2NO
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select MKTGCARRIER, SEQNO, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
        "OPTGCARRIER, INCLEXCLIND, BRDLOCTYPE, BRDLOC, OFFLOCTYPE, OFFLOC, FLT1NO, FLT2NO");
    this->From("=BAGGAGESECTORCARRIERAPP ");
    this->Where("MKTGCARRIER = %1q "
                "   and %cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("MKTGCARRIER,SEQNO,CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE, OPTGCARRIER, "
                    "INCLEXCLIND, BRDLOCTYPE, BRDLOC, OFFLOCTYPE, OFFLOC, FLT1NO, FLT2NO");
    else
      this->OrderBy("MKTGCARRIER,SEQNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static BaggageSectorCarrierApp* mapRowToBaggageSectorCarrierApp(Row* row)
  {
    BaggageSectorCarrierApp* crxApp = new BaggageSectorCarrierApp;

    crxApp->marketingCarrier() = row->getString(MKTGCARRIER);
    crxApp->seqNo() = row->getLong(SEQNO);
    crxApp->createDate() = row->getDate(CREATEDATE);
    crxApp->expireDate() = row->getDate(EXPIREDATE);
    crxApp->effDate() = row->getDate(EFFDATE);
    crxApp->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(OPTGCARRIER))
      crxApp->operatingCarrier() = row->getString(OPTGCARRIER);
    else
      crxApp->operatingCarrier() = "";
    crxApp->inclExclInd() = row->getChar(INCLEXCLIND);

    if (!row->isNull(BRDLOCTYPE) && row->getChar(BRDLOCTYPE) != ' ')
      crxApp->loc1().locType() = LocType(row->getChar(BRDLOCTYPE));
    else
      crxApp->loc1().locType() = LocType(UNKNOWN_LOC);

    if (!row->isNull(BRDLOC) && row->getChar(BRDLOC) != ' ')
      crxApp->loc1().loc() = row->getString(BRDLOC);
    else
      crxApp->loc1().loc() = "";

    if (!row->isNull(OFFLOCTYPE) && row->getChar(OFFLOCTYPE) != ' ')
      crxApp->loc2().locType() = LocType(row->getChar(OFFLOCTYPE));
    else
      crxApp->loc2().locType() = LocType(UNKNOWN_LOC);

    if (!row->isNull(OFFLOC) && row->getChar(OFFLOC) != ' ')
      crxApp->loc2().loc() = row->getString(OFFLOC);
    else
      crxApp->loc2().loc() = "";

    crxApp->flt1() = row->getInt(FLT1NO);
    crxApp->flt2() = row->getInt(FLT2NO);

    return crxApp;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBaggageSectorCarrierAppHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBaggageSectorCarrierAppHistoricalSQLStatement
    : public QueryGetBaggageSectorCarrierAppSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("MKTGCARRIER = %1q "); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBaggageSectorCarrierAppSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBaggageSectorCarrierAppSQLStatement
    : public QueryGetBaggageSectorCarrierAppSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBaggageSectorCarrierAppHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBaggageSectorCarrierAppHistoricalSQLStatement
    : public QueryGetBaggageSectorCarrierAppSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("1=1"); }
};
} // tse
