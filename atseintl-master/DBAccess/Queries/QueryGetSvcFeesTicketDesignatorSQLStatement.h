//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesTicketDesignatorSQLStatement.h
//          Description:    QueryGetSvcFeesTicketDesignatorSQLStatement
//          Created:        3/19/2009
// Authors:
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesTicketDesignator.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesTicketDesignatorSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesTicketDesignatorSQLStatement() {};
  virtual ~QueryGetSvcFeesTicketDesignatorSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    TICKETDESIGNATOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO,  "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, TICKETDESIGNATOR");

    this->From("=SVCFEESTICKETDESIGNATOR");

    this->Where("VENDOR      = %1q  "
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("CREATEDATE, VENDOR, ITEMNO, SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::SvcFeesTktDesignatorInfo* mapRowToSvcFeesTktDesignatorInfo(Row* row)
  {
    tse::SvcFeesTktDesignatorInfo* tktD = new tse::SvcFeesTktDesignatorInfo;

    tktD->vendor() = row->getString(VENDOR);
    tktD->itemNo() = row->getInt(ITEMNO);
    tktD->seqNo() = row->getLong(SEQNO);
    tktD->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      tktD->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(TICKETDESIGNATOR))
      tktD->tktDesignator() = row->getString(TICKETDESIGNATOR);

    return tktD;
  }; // mapRowToSvcFeesTktDesignatorInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesTicketDesignatorSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesTicketDesignatorHistoricalSQLStatement
    : public QueryGetSvcFeesTicketDesignatorSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and ITEMNO= %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  };
};
}
