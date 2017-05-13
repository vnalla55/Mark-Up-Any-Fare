//----------------------------------------------------------------------------
//          File:           QueryGetTicketEndorsementsInfoSQLStatement.h
//          Description:    QueryGetTicketEndorsementsInfoSQLStatement
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
#include "DBAccess/Queries/QueryGetTicketEndorsementsInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTicketEndorsementsInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTicketEndorsementsInfoSQLStatement() {}
  virtual ~QueryGetTicketEndorsementsInfoSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    PRIORITYCODE,
    ENDORSEMENTTXT,
    TKTLOCIND,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,PRIORITYCODE,ENDORSEMENTTXT,TKTLOCIND,"
                  "       INHIBIT");
    this->From("=TICKETENDORSEMENTS");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TicketEndorsementsInfo* mapRowToTicketEndorsementsInfo(Row* row)
  {
    tse::TicketEndorsementsInfo* tei = new tse::TicketEndorsementsInfo;

    tei->vendor() = row->getString(VENDOR);
    tei->itemNo() = row->getInt(ITEMNO);
    tei->createDate() = row->getDate(CREATEDATE);
    tei->expireDate() = row->getDate(EXPIREDATE);
    tei->unavailTag() = row->getChar(UNAVAILTAG);
    tei->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    tei->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    tei->priorityCode() = row->getInt(PRIORITYCODE);
    tei->endorsementTxt() = row->getString(ENDORSEMENTTXT);
    tei->tktLocInd() = row->getChar(TKTLOCIND);
    tei->inhibit() = row->getChar(INHIBIT);

    return tei;
  } // mapRowToTicketEndorsementsInfo()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTicketEndorsementsInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTicketEndorsementsInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTicketEndorsementsInfoHistoricalSQLStatement
    : public QueryGetTicketEndorsementsInfoSQLStatement<QUERYCLASS>
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
}; // class QueryGetTicketEndorsementsInfoHistoricalSQLStatement
}
