//----------------------------------------------------------------------------
//          File:           QueryGetBasicBookingRequestSQLStatement.h
//          Description:    QueryGetBasicBookingRequestSQLStatement
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
#include "DBAccess/Queries/QueryGetBasicBookingRequest.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBasicBookingRequestSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBasicBookingRequestSQLStatement() {};
  virtual ~QueryGetBasicBookingRequestSQLStatement() {};

  enum ColumnIndexes
  {
    OWNERID = 0,
    CARRIER,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select OWNERID,CARRIER,EFFDATE,DISCDATE");
    this->From("=BASICBOOKINGREQUEST ");
    this->Where("  CARRIER = %1q "
                "  and EFFDATE <= %cd ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BasicBookingRequest* mapRowToBasicBookingRequest(Row* row)
  {
    tse::BasicBookingRequest* bbr = new tse::BasicBookingRequest;

    bbr->ownerId() = row->getString(OWNERID);
    bbr->carrier() = row->getString(CARRIER);
    bbr->effDate() = row->getDate(EFFDATE);
    bbr->discDate() = row->getDate(DISCDATE);

    return bbr;
  } // mapRowToBasicBookingRequest()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBasicBookingRequests
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBasicBookingRequestsSQLStatement
    : public QueryGetBasicBookingRequestSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("EFFDATE <= %cd "); }
};
} // tse
