//----------------------------------------------------------------------------
//          File:           QueryGetPfcTktDesigExceptSQLStatement.h
//          Description:    QueryGetPfcTktDesigExceptSQLStatement
//          Created:        11/09/2007
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
#include "DBAccess/Queries/QueryGetPfcTktDesigExcept.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcTktDesigExceptSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcTktDesigExceptSQLStatement() {}
  virtual ~QueryGetPfcTktDesigExceptSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER = 0,
    TKTDESIGNATOR,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,TKTDESIGNATOR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE ");
    this->From("=PFCTKTDESIGEXCEPT ");
    this->Where(" CARRIER = %1q"
                "    and %cd <= EXPIREDATE");
    this->OrderBy(" TKTDESIGNATOR");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcTktDesigExcept* mapRowToPfcTktDesigExcept(Row* row)
  {
    tse::PfcTktDesigExcept* tde = new tse::PfcTktDesigExcept;

    tde->carrier() = row->getString(CARRIER);
    tde->tktDesignator() = row->getString(TKTDESIGNATOR);
    tde->createDate() = row->getDate(CREATEDATE);
    tde->expireDate() = row->getDate(EXPIREDATE);
    tde->effDate() = row->getDate(EFFDATE);
    tde->discDate() = row->getDate(DISCDATE);

    return tde;
  } // mapRowToPfcTktDesigExcept()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPfcTktDesigExceptHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetPfcTktDesigExceptHistoricalSQLStatement
    : public QueryGetPfcTktDesigExceptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" CARRIER = %1q"
                "  and %2n <= EXPIREDATE"
                "  and (%3n >= CREATEDATE"
                "   or %4n >= EFFDATE)");
  }
};

} // tse
