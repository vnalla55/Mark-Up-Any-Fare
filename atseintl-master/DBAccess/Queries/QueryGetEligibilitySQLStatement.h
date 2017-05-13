//----------------------------------------------------------------------------
//          File:           QueryGetEligibilitySQLStatement.h
//          Description:    QueryGetEligibilitySQLStatement
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
#include "DBAccess/Queries/QueryGetEligibility.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetEligibilitySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetEligibilitySQLStatement() {};
  virtual ~QueryGetEligibilitySQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    MINAGE,
    MAXAGE,
    FIRSTOCCUR,
    LASTOCCUR,
    UNAVAILTAG,
    PSGTYPE,
    PSGID,
    PSGAPPL,
    PSGSTATUS,
    LOC1TYPE,
    LOC1,
    ACCTCODE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,MINAGE,MAXAGE,FIRSTOCCUR,LASTOCCUR,"
                  "       UNAVAILTAG,PSGTYPE,PSGID,PSGAPPL,PSGSTATUS,LOC1TYPE,LOC1,"
                  "       ACCTCODE,INHIBIT");
    this->From("=ELIGIBILITY");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::EligibilityInfo* mapRowToEligibilityInfo(Row* row)
  {
    tse::EligibilityInfo* elg = new tse::EligibilityInfo;

    elg->vendor() = row->getString(VENDOR);
    elg->itemNo() = row->getInt(ITEMNO);
    elg->createDate() = row->getDate(CREATEDATE);
    elg->expireDate() = row->getDate(EXPIREDATE);
    elg->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    elg->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    elg->minAge() = row->getInt(MINAGE);
    elg->maxAge() = row->getInt(MAXAGE);
    elg->firstOccur() = row->getInt(FIRSTOCCUR);
    elg->lastOccur() = row->getInt(LASTOCCUR);
    elg->unavailTag() = row->getChar(UNAVAILTAG);
    elg->psgType() = row->getString(PSGTYPE);
    elg->psgId() = row->getChar(PSGID);
    elg->psgAppl() = row->getChar(PSGAPPL);
    elg->psgStatus() = row->getChar(PSGSTATUS);
    elg->loc1().locType() = row->getChar(LOC1TYPE);
    elg->loc1().loc() = row->getString(LOC1);
    elg->acctCode() = row->getString(ACCTCODE);
    elg->inhibit() = row->getChar(INHIBIT);

    return elg;
  } // mapRowToEligibilityInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetEligibilitySQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetEligibilityHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetEligibilityHistoricalSQLStatement : public QueryGetEligibilitySQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                "  and ITEMNO = %2n"
                "  and VALIDITYIND = 'Y'"
                "  and %3n <= EXPIREDATE"
                "  and %4n >= CREATEDATE");
  }
}; // class QueryGetEligibilityHistoricalSQLStatement
} // tse
