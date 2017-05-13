//----------------------------------------------------------------------------
//          File:           QueryGetOpenJawRestrictionSQLStatement.h
//          Description:    QueryGetOpenJawRestrictionSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetOpenJawRestriction.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetOpenJawRestrictionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOpenJawRestrictionSQLStatement() {};
  virtual ~QueryGetOpenJawRestrictionSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CATEGORY,
    ORDERNO,
    CREATEDATE,
    EXPIREDATE,
    SET1APPLIND,
    SET1LOC1TYPE,
    SET1LOC1,
    SET1LOC2TYPE,
    SET1LOC2,
    SET2APPLIND,
    SET2LOC1TYPE,
    SET2LOC1,
    SET2LOC2TYPE,
    SET2LOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select r.VENDOR,r.ITEMNO,r.CATEGORY,r.ORDERNO,r.CREATEDATE,m.EXPIREDATE, "
                  "       SET1APPLIND,SET1LOC1TYPE,"
                  "       SET1LOC1,SET1LOC2TYPE,SET1LOC2,SET2APPLIND,SET2LOC1TYPE,"
                  "       SET2LOC1,SET2LOC2TYPE,SET2LOC2");
    this->From("=OPENJAWSETSSEQ r join =MINORCOMBSUBCAT m "
               "   on  r.VENDOR = m.VENDOR "
               "   and r.ITEMNO = m.ITEMNO "
               "   and r.CATEGORY = m.CATEGORY "
               "   and r.CREATEDATE = m.CREATEDATE ");

    this->Where("r.VENDOR = %1q "
                "    and r.ITEMNO = %2n"
                "    and m.EXPIREDATE >= %cd ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::OpenJawRestriction* mapRowToOpenJawRestriction(Row* row)
  {
    tse::OpenJawRestriction* openJaw = new tse::OpenJawRestriction;

    openJaw->vendor() = row->getString(VENDOR);
    openJaw->itemNo() = row->getInt(ITEMNO);
    openJaw->category() = row->getInt(CATEGORY);
    openJaw->orderNo() = row->getInt(ORDERNO);
    openJaw->createDate() = row->getDate(CREATEDATE);
    openJaw->expireDate() = row->getDate(EXPIREDATE);
    openJaw->set1ApplInd() = row->getChar(SET1APPLIND);
    openJaw->set1Loc1().locType() = row->getChar(SET1LOC1TYPE);
    openJaw->set1Loc1().loc() = row->getString(SET1LOC1);
    openJaw->set1Loc2().locType() = row->getChar(SET1LOC2TYPE);
    openJaw->set1Loc2().loc() = row->getString(SET1LOC2);
    openJaw->set2ApplInd() = row->getChar(SET2APPLIND);
    openJaw->set2Loc1().locType() = row->getChar(SET2LOC1TYPE);
    openJaw->set2Loc1().loc() = row->getString(SET2LOC1);
    openJaw->set2Loc2().locType() = row->getChar(SET2LOC2TYPE);
    openJaw->set2Loc2().loc() = row->getString(SET2LOC2);

    return openJaw;
  } // mapRowToOpenJawRestriction()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetOpenJawRestrictionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOpenJawRestrictionHistoricalSQLStatement
    : public QueryGetOpenJawRestrictionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("r.VENDOR = %1q "
                " and r.ITEMNO = %2n"
                " and %3n <= m.EXPIREDATE"
                " and %4n >= r.CREATEDATE");
  }
}; // class QueryGetOpenJawRestrictionHistoricalSQLStatement
} // tse
