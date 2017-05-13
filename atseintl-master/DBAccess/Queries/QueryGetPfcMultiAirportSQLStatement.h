//----------------------------------------------------------------------------
//          File:           QueryGetPfcMultiAirportSQLStatement.h
//          Description:    QueryGetPfcMultiAirportSQLStatement
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
#include "DBAccess/Queries/QueryGetPfcMultiAirport.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcMultiAirportSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcMultiAirportSQLStatement() {}
  virtual ~QueryGetPfcMultiAirportSQLStatement() {}

  enum ColumnIndexes
  {
    LOC = 0,
    EFFDATE,
    CREATEDATE,
    LOCTYPE,
    EXPIREDATE,
    DISCDATE,
    SEGCNT,
    VENDOR,
    ORDERNO,
    COTERMLOC,
    VENDC,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.LOC,a.EFFDATE,a.CREATEDATE,LOCTYPE,EXPIREDATE,"
                  " DISDATE DISCDATE,SEGCNT,a.VENDOR,c.ORDERNO,COTERMLOC,"
                  " c.VENDOR VENDC,INHIBIT");

    //		        this->From(" =PFCMULTIAPT a LEFT OUTER JOIN =COTERMINAL c"
    //		                  " USING (LOC,EFFDATE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("LOC");
    joinFields.push_back("EFFDATE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=PFCMULTIAPT", "a", "LEFT OUTER JOIN", "=COTERMINAL", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("VALIDITYIND = 'Y'"
                "    and a.LOC = %1q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("a.LOC,a.EFFDATE,a.CREATEDATE,c.ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcMultiAirport* mapRowToPfcMultiAirport(Row* row, PfcMultiAirport* maPrev)
  {
    LocCode loc = row->getString(LOC);
    Indicator locType = row->getChar(LOCTYPE);
    DateTime createDate = row->getDate(CREATEDATE);
    DateTime effDate;
    effDate = row->getDate(EFFDATE);

    PfcMultiAirport* ma;

    // If Parent hasn't changed, add to Child (segs)
    if (maPrev != nullptr && maPrev->loc().loc() == loc && maPrev->loc().locType() == locType &&
        maPrev->createDate() == createDate && maPrev->effDate() == effDate)
    { // Add to Prev
      ma = maPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      ma = new tse::PfcMultiAirport;
      ma->loc().loc() = loc;
      ma->loc().locType() = locType;
      ma->createDate() = createDate;
      ma->effDate() = effDate;

      ma->expireDate() = row->getDate(EXPIREDATE);
      ma->discDate() = row->getDate(DISCDATE);
      ma->segCnt() = row->getInt(SEGCNT);
      ma->vendor() = row->getString(VENDOR);
      ma->inhibit() = row->getChar(INHIBIT);
    }
    if (!row->isNull(ORDERNO))
    {
      PfcCoterminal* newCT = new PfcCoterminal;
      newCT->orderNo() = row->getInt(ORDERNO);
      newCT->cotermLoc() = row->getString(COTERMLOC);
      newCT->vendor() = row->getString(VENDC);
      ma->coterminals().push_back(newCT);
    }

    return ma;
  } // mapRowToPfcMultiAirport()
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
class QueryGetAllPfcMultiAirportSQLStatement
    : public QueryGetPfcMultiAirportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'");
    this->OrderBy("a.LOC,a.EFFDATE,a.CREATEDATE,c.ORDERNO");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPfcMultiAirportHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetPfcMultiAirportHistoricalSQLStatement
    : public QueryGetPfcMultiAirportSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                "    and a.LOC = %1q "
                "    and %2n <= EXPIREDATE"
                "    and (%3n >= a.CREATEDATE"
                "     or %4n >= a.EFFDATE)");

    if (DataManager::forceSortOrder())
      this->OrderBy("a.LOC,a.EFFDATE,a.CREATEDATE,c.ORDERNO");
    else
      this->OrderBy("a.CREATEDATE");
  }
};
}

