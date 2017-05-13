//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrierStatusSQLStatement.h
//          Description:    QueryGetInterlineTicketCarrierStatusSQLStatement
//          Created:        02/27/2012
//          Authors:        M Dantas
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetInterlineTicketCarrierStatus.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetInterlineTicketCarrierStatusSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetInterlineTicketCarrierStatusSQLStatement() {};
  virtual ~QueryGetInterlineTicketCarrierStatusSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CRSCODE,
    STATUS,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    LASTMODDATE,
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select CARRIER,CRSCODE,STATUS,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,LASTMODDATE ");
    this->From("=INTERLINETKTCARRIERSTATUS ");
    this->Where("CARRIER = %1q"
                " and "
                "CRSCODE = %2q "
                " and %cd <= EXPIREDATE ");

    this->OrderBy("carrier, createdate");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::InterlineTicketCarrierStatus* mapRowToInterlineTicketCarrierStatus(Row* row)
  {
    tse::InterlineTicketCarrierStatus* interlineTC = new tse::InterlineTicketCarrierStatus;
    interlineTC->carrier() = row->getString(CARRIER);
    interlineTC->crsCode() = row->getString(CRSCODE);
    interlineTC->status() = row->getChar(STATUS);
    interlineTC->createDate() = row->getDate(CREATEDATE);
    interlineTC->expireDate() = row->getDate(EXPIREDATE);
    interlineTC->effDate() = row->getDate(EFFDATE);
    interlineTC->discDate() = row->getDate(DISCDATE);
    interlineTC->lastModDate() = row->getDate(LASTMODDATE);
    return interlineTC;

  } // mapRowToInterlineTicketCarrierStatus()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

///////////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetInterlineTicketCarriertatusHistorical
///////////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetInterlineTicketCarrierStatusHistoricalSQLStatement
    : public QueryGetInterlineTicketCarrierStatusSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q "
                "CRSCODE = %2q ");
    this->OrderBy("carrier, createdate");
  };
};

//////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllInterlineTicketCarrierStatus
//////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllInterlineTicketCarrierStatusSQLStatement
    : public QueryGetInterlineTicketCarrierStatusSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");
    this->OrderBy("carrier, createdate");
  };
};

//////////////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllInterlineTicketCarriertatusHistorical
//////////////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllInterlineTicketCarrierStatusHistoricalSQLStatement
    : public QueryGetInterlineTicketCarrierStatusSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("carrier, createdate");
  };
};

} // tse
