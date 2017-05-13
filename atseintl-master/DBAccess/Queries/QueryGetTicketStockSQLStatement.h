//----------------------------------------------------------------------------
//          File:           QueryGetTicketStockSQLStatement.h
//          Description:    QueryGetTicketStockSQLStatement
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
#include "DBAccess/Queries/QueryGetTicketStock.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTicketStockSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTicketStockSQLStatement() {}
  virtual ~QueryGetTicketStockSQLStatement() {}

  enum ColumnIndexes
  {
    TKTSTOCKCODE = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    TICKETSTOCK,
    COUPONSPERBOOK,
    LINESPERMITTED,
    TOTALCHARACTERS,
    INHIBIT,
    LINENO,
    CHARSPERMITTED,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select t.TKTSTOCKCODE,t.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "TICKETSTOCK,COUPONSPERBOOK,LINESPERMITTED,TOTALCHARACTERS,"
                  "INHIBIT,LINENO,CHARSPERMITTED");

    //		        this->From("=TICKETSTOCK t LEFT OUTER JOIN =TICKETSTOCKSEG s"
    //		                  " USING (TKTSTOCKCODE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("TKTSTOCKCODE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TICKETSTOCK", "t", "LEFT OUTER JOIN", "=TICKETSTOCKSEG", "s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("VALIDITYIND = 'Y'"
                " and t.TKTSTOCKCODE = %1n "
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("t.TKTSTOCKCODE, t.CREATEDATE, s.LINENO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TicketStock* mapRowToTicketStock(Row* row, TicketStock* tsPrev)
  {
    int tktStockCode = row->getInt(TKTSTOCKCODE);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::TicketStock* ts;

    if (tsPrev != nullptr && tktStockCode == tsPrev->tktStockCode() && createDate == tsPrev->createDate())
    {
      ts = tsPrev;
    }
    else
    { // Create new parent
      ts = new tse::TicketStock;
      ts->tktStockCode() = tktStockCode;
      ts->createDate() = createDate;

      ts->expireDate() = row->getDate(EXPIREDATE);
      ts->effDate() = row->getDate(EFFDATE);
      ts->discDate() = row->getDate(DISCDATE);
      ts->ticketStock() = row->getString(TICKETSTOCK);
      ts->couponsPerBook() = row->getInt(COUPONSPERBOOK);
      ts->linesPermitted() = row->getInt(LINESPERMITTED);
      ts->totalCharacters() = row->getInt(TOTALCHARACTERS);
      ts->inhibit() = row->getChar(INHIBIT);
    }

    if (!row->isNull(LINENO))
    {
      ts->addSegInfo(row->getInt(LINENO), row->getInt(CHARSPERMITTED));
    }

    return ts;
  } // mapRowToTicketStock()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used for QueryGetTicketStockHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTicketStockHistoricalSQLStatement : public QueryGetTicketStockSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                " and t.TKTSTOCKCODE = %1n ");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTicketStockSQLStatement : public QueryGetTicketStockSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used for QueryGetAllTicketStockHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTicketStockHistoricalSQLStatement
    : public QueryGetTicketStockSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'");
    this->OrderBy("t.TKTSTOCKCODE");
  }
};
}

