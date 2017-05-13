//----------------------------------------------------------------------------
//          File:           QueryGetPfcAbsorbSQLStatement.h
//          Description:    QueryGetPfcAbsorbSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcAbsorb.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcAbsorbSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcAbsorbSQLStatement() {}
  virtual ~QueryGetPfcAbsorbSQLStatement() {}

  enum ColumnIndexes
  {
    PFCAIRPORT = 0,
    LOCALCARRIER,
    EFFDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    GEOAPPL,
    ABSORBTYPE,
    FARETARIFF,
    OWRT,
    JOINTCARRIER,
    ABSORBCITY1,
    ABSORBCITY2,
    FARECLASS,
    ROUTING1,
    ROUTING2,
    RULENO,
    FLT1,
    FLT2,
    FLT3,
    FLT4,
    VENDOR,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PFCAIRPORT,LOCALCARRIER,EFFDATE,SEQNO,CREATEDATE,EXPIREDATE,"
                  "       DISDATE DISCDATE,GEOAPPL,ABSORBTYPE,FARETARIFF,OWRT,JOINTCARRIER,"
                  "       ABSORBCITY1,ABSORBCITY2,FARECLASS,ROUTING1,ROUTING2,RULENO,FLT1,"
                  "       FLT2,FLT3,FLT4,VENDOR,INHIBIT");
    this->From("=PFCABSORB ");
    this->Where("PFCAIRPORT = %1q"
                "    and LOCALCARRIER = %2q"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("PFCAIRPORT, LOCALCARRIER, EFFDATE, SEQNO, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcAbsorb* mapRowToPfcAbsorb(Row* row)
  {
    tse::PfcAbsorb* abs = new tse::PfcAbsorb;

    abs->pfcAirport() = row->getString(PFCAIRPORT);
    abs->localCarrier() = row->getString(LOCALCARRIER);
    abs->effDate() = row->getDate(EFFDATE);
    abs->seqNo() = row->getLong(SEQNO);
    abs->createDate() = row->getDate(CREATEDATE);
    abs->expireDate() = row->getDate(EXPIREDATE);
    abs->discDate() = row->getDate(DISCDATE);
    abs->geoAppl() = row->getChar(GEOAPPL);
    abs->absorbType() = row->getChar(ABSORBTYPE);
    abs->fareTariff() = row->getInt(FARETARIFF);
    abs->OwRt() = row->getChar(OWRT);
    abs->jointCarrier() = row->getString(JOINTCARRIER);
    abs->absorbCity1() = row->getString(ABSORBCITY1);
    abs->absorbCity2() = row->getString(ABSORBCITY2);
    abs->fareClass() = row->getString(FARECLASS);
    abs->routing1() = row->getString(ROUTING1);
    abs->routing2() = row->getString(ROUTING2);
    abs->ruleNo() = row->getString(RULENO);
    abs->flt1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
    abs->flt2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
    abs->flt3() = QUERYCLASS::checkFlightWildCard(row->getString(FLT3));
    abs->flt4() = QUERYCLASS::checkFlightWildCard(row->getString(FLT4));
    abs->vendor() = row->getString(VENDOR);
    abs->inhibit() = row->getChar(INHIBIT);

    return abs;
  } // mapRowToPfcAbsorb()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllPfcAbsorbSQLStatement : public QueryGetPfcAbsorbSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("VALIDITYIND = 'Y'"); }
};

template <class QUERYCLASS>
class QueryGetPfcAbsorbHistoricalSQLStatement : public QueryGetPfcAbsorbSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("PFCAIRPORT = %1q"
                "    and LOCALCARRIER = %2q"
                "    and VALIDITYIND = 'Y'"
                "    and %3n <= EXPIREDATE"
                "    and (%4n >= CREATEDATE"
                "     or %5n >= EFFDATE)");
  }
};

} // tse

