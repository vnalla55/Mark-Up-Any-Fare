//----------------------------------------------------------------------------
//          File:           QueryGetDifferentialsSQLStatement.h
//          Description:    QueryGetDifferentialsSQLStatement
//          Created:        11/01/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDifferentials.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDifferentialsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDifferentialsSQLStatement() {};
  virtual ~QueryGetDifferentialsSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    VIALOCTYPE,
    VIALOC,
    GLOBALDIR,
    FARECLASS,
    FARETYPE,
    BOOKINGCODE,
    FLIGHTAPPL,
    CALCULATIONIND,
    HIPEXEMPTIND,
    INTERMEDCARRIER,
    INTERMEDLOC1ATYPE,
    INTERMEDLOC1A,
    INTERMEDLOC2ATYPE,
    INTERMEDLOC2A,
    INTERMEDLOC1BTYPE,
    INTERMEDLOC1B,
    INTERMEDLOC2BTYPE,
    INTERMEDLOC2B,
    INTERMEDFARETYPE,
    INTERMEDBOOKINGCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,SEQNO,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,DIRECTIONALITY,"
                  "       LOC1TYPE,LOC1,LOC2TYPE,LOC2,VIALOCTYPE,VIALOC,GLOBALDIR,FARECLASS,"
                  "       FARETYPE,BOOKINGCODE,FLIGHTAPPL,CALCULATIONIND,HIPEXEMPTIND,"
                  "       INTERMEDCARRIER,INTERMEDLOC1ATYPE,INTERMEDLOC1A,INTERMEDLOC2ATYPE,"
                  "       INTERMEDLOC2A,INTERMEDLOC1BTYPE,INTERMEDLOC1B,INTERMEDLOC2BTYPE,"
                  "       INTERMEDLOC2B,INTERMEDFARETYPE,INTERMEDBOOKINGCODE");
    this->From("=DIFFERENTIALS");
    this->Where("CARRIER = %1q "
                "    and %cd <= EXPIREDATE");
    this->OrderBy("SEQNO ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Differentials* mapRowToDifferentials(Row* row)
  {
    tse::Differentials* diffs = new tse::Differentials;

    diffs->carrier() = row->getString(CARRIER);
    diffs->seqNo() = row->getLong(SEQNO);
    diffs->createDate() = row->getDate(CREATEDATE);
    diffs->expireDate() = row->getDate(EXPIREDATE);
    diffs->effDate() = row->getDate(EFFDATE);
    diffs->discDate() = row->getDate(DISCDATE);
    diffs->directionality() = row->getChar(DIRECTIONALITY);
    diffs->loc1().locType() = row->getChar(LOC1TYPE);
    diffs->loc1().loc() = row->getString(LOC1);
    diffs->loc2().locType() = row->getChar(LOC2TYPE);
    diffs->loc2().loc() = row->getString(LOC2);
    diffs->viaLoc().locType() = row->getChar(VIALOCTYPE);
    diffs->viaLoc().loc() = row->getString(VIALOC);

    strToGlobalDirection(diffs->globalDir(), row->getString(GLOBALDIR));

    diffs->fareClass() = row->getString(FARECLASS);
    diffs->fareType() = row->getString(FARETYPE);
    diffs->bookingCode() = row->getString(BOOKINGCODE);
    diffs->flightAppl() = row->getChar(FLIGHTAPPL);
    diffs->calculationInd() = row->getChar(CALCULATIONIND);
    diffs->hipExemptInd() = row->getChar(HIPEXEMPTIND);
    diffs->intermedCarrier() = row->getString(INTERMEDCARRIER);

    diffs->intermedLoc1a().locType() = row->getChar(INTERMEDLOC1ATYPE);
    diffs->intermedLoc1a().loc() = row->getString(INTERMEDLOC1A);

    diffs->intermedLoc2a().locType() = row->getChar(INTERMEDLOC2ATYPE);
    diffs->intermedLoc2a().loc() = row->getString(INTERMEDLOC2A);

    diffs->intermedLoc1b().locType() = row->getChar(INTERMEDLOC1BTYPE);
    diffs->intermedLoc1b().loc() = row->getString(INTERMEDLOC1B);

    diffs->intermedLoc2b().locType() = row->getChar(INTERMEDLOC2BTYPE);
    diffs->intermedLoc2b().loc() = row->getString(INTERMEDLOC2B);

    diffs->intermedFareType() = row->getString(INTERMEDFARETYPE);
    diffs->intermedBookingCode() = row->getString(INTERMEDBOOKINGCODE);

    return diffs;
  } // mapRowToDifferentials

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetDifferentialsSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDifferentialsHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetDifferentialsHistoricalSQLStatement
    : public QueryGetDifferentialsSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO,CREATEDATE ");
  }
}; // class QueryGetDifferentialsHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllDifferentials
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllDifferentialsSQLStatement : public QueryGetDifferentialsSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("CARRIER,SEQNO ");
  }
}; // class QueryGetAllDifferentialsSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllDifferentialsHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllDifferentialsHistoricalSQLStatement
    : public QueryGetDifferentialsSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,SEQNO,CREATEDATE");
    else
      this->OrderBy("CARRIER,SEQNO ");
  }
}; // class QueryGetAllDifferentialsHistoricalSQLStatement
} // tse
