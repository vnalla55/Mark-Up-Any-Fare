//----------------------------------------------------------------------------
//          File:           QueryGetTaxSegAbsorbSQLStatement.h
//          Description:    QueryGetTaxSegAbsorbSQLStatement
//          Created:        10/5/2007
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
#include "DBAccess/Queries/QueryGetTaxSegAbsorb.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTaxSegAbsorbSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxSegAbsorbSQLStatement() {}
  virtual ~QueryGetTaxSegAbsorbSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER = 0,
    EFFDATE,
    SEQNO,
    DISCDATE,
    CREATEDATE,
    EXPIREDATE,
    FARETARIFFNO,
    NOSEG1,
    NOSEG2,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    ABSORPTIONIND,
    BETWANDVIALOC1TYPE,
    BETWANDVIALOC1,
    BETWANDVIALOC2TYPE,
    BETWANDVIALOC2,
    OWRT,
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
    this->Command("select CARRIER,EFFDATE,SEQNO,DISDATE DISCDATE,CREATEDATE,EXPIREDATE,"
                  "       FARETARIFFNO,NOSEG1,NOSEG2,LOC1TYPE,LOC1,LOC2TYPE,LOC2,ABSORPTIONIND,"
                  "       BETWANDVIALOC1TYPE,BETWANDVIALOC1,BETWANDVIALOC2TYPE,BETWANDVIALOC2,OWRT,"
                  "       FARECLASS,ROUTING1,ROUTING2,RULENO,FLT1,FLT2,FLT3,FLT4,VENDOR,INHIBIT");
    this->From("=TAXSEGABSORP");
    this->Where("VALIDITYIND = 'Y'"
                "    and CARRIER = %1q "
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER, EFFDATE, SEQNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxSegAbsorb* mapRowToTaxSegAbsorb(Row* row)
  {
    tse::TaxSegAbsorb* tsa = new tse::TaxSegAbsorb;

    tsa->carrier() = row->getString(CARRIER);
    tsa->effDate() = row->getDate(EFFDATE);
    tsa->seqNo() = row->getLong(SEQNO);
    tsa->discDate() = row->getDate(DISCDATE);
    tsa->createDate() = row->getDate(CREATEDATE);
    tsa->expireDate() = row->getDate(EXPIREDATE);
    tsa->fareTariffNo() = row->getInt(FARETARIFFNO);
    tsa->noSeg1() = row->getInt(NOSEG1);
    tsa->noSeg2() = row->getInt(NOSEG2);

    LocKey* loc = &tsa->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);

    loc = &tsa->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);

    tsa->absorptionInd() = row->getChar(ABSORPTIONIND);

    loc = &tsa->betwAndViaLoc1();
    loc->locType() = row->getChar(BETWANDVIALOC1TYPE);
    loc->loc() = row->getString(BETWANDVIALOC1);

    loc = &tsa->betwAndViaLoc2();
    loc->locType() = row->getChar(BETWANDVIALOC2TYPE);
    loc->loc() = row->getString(BETWANDVIALOC2);

    tsa->owRt() = row->getChar(OWRT);
    tsa->fareClass() = row->getString(FARECLASS);
    tsa->routing1() = row->getString(ROUTING1);
    tsa->routing2() = row->getString(ROUTING2);
    tsa->ruleNo() = row->getString(RULENO);
    tsa->flt1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
    tsa->flt2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
    tsa->flt3() = QUERYCLASS::checkFlightWildCard(row->getString(FLT3));
    tsa->flt4() = QUERYCLASS::checkFlightWildCard(row->getString(FLT4));
    tsa->vendor() = row->getString(VENDOR);
    tsa->inhibit() = row->getChar(INHIBIT);

    return tsa;
  } // mapRowToTaxSegAbsorb()
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
class QueryGetAllTaxSegAbsorbSQLStatement : public QueryGetTaxSegAbsorbSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y' and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER, EFFDATE, SEQNO, CREATEDATE");
    else
      this->OrderBy("1");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTaxSegAbsorbHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTaxSegAbsorbHistoricalSQLStatement
    : public QueryGetTaxSegAbsorbSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                " and CARRIER = %1q "
                " and %2n <= EXPIREDATE"
                " and (%3n >= CREATEDATE"
                "  or %4n >= EFFDATE)");
  }
};
}

