//----------------------------------------------------------------------------
//          File:           QueryGetBkgCodeConvSQLStatement.h
//          Description:    QueryGetBkgCodeConvSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBkgCodeConv.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBkgCodeConvSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBkgCodeConvSQLStatement() {};
  virtual ~QueryGetBkgCodeConvSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    BOOKINGCODETBLITEMNO,
    CONVENTIONNO,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,RULETARIFF,RULE,SEQNO,CREATEDATE,EXPIREDATE,"
                  "       EFFDATE,DISCDATE,BOOKINGCODETBLITEMNO,CONVENTIONNO,INHIBIT");
    this->From(" =BOOKINGCODE");
    this->Where("%cd <= EXPIREDATE"
                "    and VENDOR = %1q"
                "    and CARRIER = %2q"
                "    and RULETARIFF = %3n"
                "    and RULE = %4q"
                "    and INHIBIT = 'N'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,CONVENTIONNO,SEQNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BookingCodeConv* mapRowToBookingCodeConv(Row* row)
  {
    tse::BookingCodeConv* bookConv = new tse::BookingCodeConv;

    bookConv->vendor() = row->getString(VENDOR);
    bookConv->carrier() = row->getString(CARRIER);
    bookConv->ruleTariff() = row->getInt(RULETARIFF);
    bookConv->rule() = row->getString(RULE);
    bookConv->seqNo() = row->getString(SEQNO);
    bookConv->createDate() = row->getDate(CREATEDATE);
    bookConv->expireDate() = row->getDate(EXPIREDATE);
    bookConv->effDate() = row->getDate(EFFDATE);
    bookConv->discDate() = row->getDate(DISCDATE);
    bookConv->bookingCodetblItemNo() = row->getInt(BOOKINGCODETBLITEMNO);
    bookConv->conventionNo() = row->getChar(CONVENTIONNO);
    bookConv->inhibit() = row->getChar(INHIBIT);

    return bookConv;
  } // mapRowToBookingCodeConv()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBkgCodeConv
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBkgCodeConvSQLStatement : public QueryGetBkgCodeConvSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE "
                "    and INHIBIT = 'N'");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,CONVENTIONNO,SEQNO,CREATEDATE");
    else
      this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,CONVENTIONNO ");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBkgCodeConvHist
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBkgCodeConvHistoricalSQLStatement : public QueryGetBkgCodeConvSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,CARRIER,RULETARIFF,RULE,SEQNO,CREATEDATE,EXPIREDATE,"
                             "       EFFDATE,DISCDATE,BOOKINGCODETBLITEMNO,CONVENTIONNO,INHIBIT");
    partialStatement.From(" =BOOKINGCODEH");
    partialStatement.Where("    VENDOR = %1q"
                           "    and CARRIER = %2q"
                           "    and RULETARIFF = %3n"
                           "    and RULE = %4q"
                           "    and %5n <= EXPIREDATE"
                           "    and %6n >= CREATEDATE"
                           "    and INHIBIT = 'N'"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR,CARRIER,RULETARIFF,RULE,SEQNO,CREATEDATE,EXPIREDATE,"
                             "       EFFDATE,DISCDATE,BOOKINGCODETBLITEMNO,CONVENTIONNO,INHIBIT");
    partialStatement.From(" =BOOKINGCODE");
    partialStatement.Where("    VENDOR = %7q"
                           "    and CARRIER = %8q"
                           "    and RULETARIFF = %9n"
                           "    and RULE = %10q"
                           "    and %11n <= EXPIREDATE"
                           "    and %12n >= CREATEDATE"
                           "    and INHIBIT = 'N'"
                           ")");
    partialStatement.OrderBy(" SEQNO, EFFDATE desc, CREATEDATE desc");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetBkgCodeConvHistoricalSQLStatement

} // tse
