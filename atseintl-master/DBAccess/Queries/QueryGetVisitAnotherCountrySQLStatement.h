//----------------------------------------------------------------------------
//          File:           QueryGetVisitAnotherCountrySQLStatement.h
//          Description:    QueryGetVisitAnotherCountrySQLStatement
//          Created:        10/5/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVisitAnotherCountry.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetVisitAnotherCountrySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetVisitAnotherCountrySQLStatement() {}
  virtual ~QueryGetVisitAnotherCountrySQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    PSGTYPE,
    IDREQUIREDIND,
    MINAGE,
    MAXAGE,
    RESIDENTIND,
    RESGEOTBLITEMNO,
    TRAVELIND,
    MILES,
    TICKET,
    TIMEPERIOD,
    TIMEUNIT,
    ISSUEIND,
    ISSUEGEOTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,VALIDITYIND,INHIBIT,UNAVAILTAG,"
                  "       PSGTYPE,IDREQUIREDIND,MINAGE,MAXAGE,RESIDENTIND,RESGEOTBLITEMNO,"
                  "       TRAVELIND,MILES,TICKET,TIMEPERIOD,TIMEUNIT,ISSUEIND,"
                  "       ISSUEGEOTBLITEMNO,OVERRIDEDATETBLITEMNO,TEXTTBLITEMNO");
    this->From("=VISITANOTHERCNTRY");
    this->Where(" VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and EXPIREDATE >= %cd "
                "    and VALIDITYIND = 'Y' ");
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::VisitAnotherCountry* mapRowToVisitAnotherCountry(Row* row)
  {
    VisitAnotherCountry* rec = new VisitAnotherCountry();

    rec->vendor() = row->getString(VENDOR);
    rec->itemNo() = row->getInt(ITEMNO);
    rec->createDate() = row->getDate(CREATEDATE);
    rec->expireDate() = row->getDate(EXPIREDATE);
    rec->validityInd() = row->getChar(VALIDITYIND);
    rec->inhibit() = row->getChar(INHIBIT);
    rec->unavailTag() = row->getChar(UNAVAILTAG);
    rec->psgType() = row->getString(PSGTYPE);
    rec->idRequiredInd() = row->getChar(IDREQUIREDIND);
    rec->minAge() = row->getInt(MINAGE);
    rec->maxAge() = row->getInt(MAXAGE);
    rec->residentInd() = row->getChar(RESIDENTIND);
    rec->resGeoTblItemNo() = row->getInt(RESGEOTBLITEMNO);
    rec->travelInd() = row->getChar(TRAVELIND);
    rec->miles() = row->getInt(MILES);
    rec->ticket() = row->getChar(TICKET);
    rec->timePeriod() = row->getInt(TIMEPERIOD);
    rec->timeUnit() = row->getChar(TIMEUNIT);
    rec->issueInd() = row->getChar(ISSUEIND);
    rec->issueGeoTblItemNo() = row->getInt(ISSUEGEOTBLITEMNO);
    rec->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    rec->textTblItemNo() = row->getInt(TEXTTBLITEMNO);

    return rec;
  } // mapRowToVisitAnotherCountry()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetVisitAnotherCountrySQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetVisitAnotherCountryHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetVisitAnotherCountryHistoricalSQLStatement
    : public QueryGetVisitAnotherCountrySQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" VENDOR = %1q"
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetVisitAnotherCountryHistoricalSQLStatement
}

