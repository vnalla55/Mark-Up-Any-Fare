//----------------------------------------------------------------------------
//          File:           QueryGetInterlineTicketCarrierSQLStatement.h
//          Description:    QueryGetInterlineTicketCarrierSQLStatement
//          Created:        10/1/2010
//          Authors:        Anna Kulig
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
#include "DBAccess/Queries/QueryGetInterlineTicketCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetInterlineTicketCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetInterlineTicketCarrierSQLStatement() {};
  virtual ~QueryGetInterlineTicketCarrierSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    LASTMODDATE,
    INTERLINECARRIER,
    HOSTINTERLINE,
    PSEUDOINTERLINE,
    SUPERPSEUDOINTERLINE,
    EMDINTERLINE
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select CARRIER,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  " LASTMODDATE,INTERLINECARRIER,HOSTINTERLINE,PSEUDOINTERLINE,"
                  " SUPERPSEUDOINTERLINE,EMDINTERLINE");
    this->From("=INTERLINETICKETCARRIER ");
    this->Where("CARRIER = %1q "
                "    and %cd <= EXPIREDATE ");
    this->OrderBy("carrier, createdate");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::InterlineTicketCarrierInfo* mapRowToInterlineTicketCarrier(Row* row)
  {
    tse::InterlineTicketCarrierInfo* interlineTC = new tse::InterlineTicketCarrierInfo;
    interlineTC->carrier() = row->getString(CARRIER);
    interlineTC->createDate() = row->getDate(CREATEDATE);
    interlineTC->expireDate() = row->getDate(EXPIREDATE);
    interlineTC->effDate() = row->getDate(EFFDATE);
    interlineTC->discDate() = row->getDate(DISCDATE);
    interlineTC->lastModDate() = row->getDate(LASTMODDATE);
    interlineTC->interlineCarrier() = row->getString(INTERLINECARRIER);
    interlineTC->hostInterline() = row->getChar(HOSTINTERLINE);
    interlineTC->pseudoInterline() = row->getChar(PSEUDOINTERLINE);
    interlineTC->superPseudoInterline() = row->getChar(SUPERPSEUDOINTERLINE);
    interlineTC->emdInterline() = row->getChar(EMDINTERLINE);
    return interlineTC;

  } // mapRowToInterlineTicketCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetInterlineTicketCarrierHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetInterlineTicketCarrierHistoricalSQLStatement
    : public QueryGetInterlineTicketCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q ");
    this->OrderBy("carrier, createdate");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllInterlineTicketCarrier
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllInterlineTicketCarrierSQLStatement
    : public QueryGetInterlineTicketCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");
    this->OrderBy("carrier, createdate");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllInterlineTicketCarrierHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllInterlineTicketCarrierHistoricalSQLStatement
    : public QueryGetInterlineTicketCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("carrier, createdate");
  };
};

} // tse
