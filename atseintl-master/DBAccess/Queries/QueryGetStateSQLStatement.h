//----------------------------------------------------------------------------
//          File:           QueryGetStateSQLStatement.h
//          Description:    QueryGetStateSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetState.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetStateSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetStateSQLStatement() {};
  virtual ~QueryGetStateSQLStatement() {};

  enum ColumnIndexes
  {
    NATION = 0,
    STATE,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    SUBAREA,
    AREA,
    DESCRIPTION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select NATION,STATE,CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,SUBAREA,AREA,DESCRIPTION ");
    this->From("=STATE ");
    this->Where("NATION = %1q "
                " and STATE = %2q "
                " and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("NATION, STATE, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::State* mapRowToState(Row* row)
  {
    tse::State* state = new tse::State;

    state->nation() = row->getString(NATION);
    state->state() = row->getString(STATE);
    state->effDate() = row->getDate(EFFDATE);
    state->discDate() = row->getDate(DISCDATE);
    state->createDate() = row->getDate(CREATEDATE);
    state->expireDate() = row->getDate(EXPIREDATE);
    state->subArea() = row->getString(SUBAREA);
    state->area() = row->getString(AREA);
    state->description() = row->getString(DESCRIPTION);

    return state;
  } // mapRowToState()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetStateSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetStateHistoricalSQLStatement : public QueryGetStateSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("NATION = %1q"
                " and STATE = %2q");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetStates
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetStatesSQLStatement : public QueryGetStateSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("NATION, STATE, CREATEDATE");
    else
      this->OrderBy("NATION,STATE");
  }
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetStatesHistoricalSQLStatement : public QueryGetStateSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("NATION,STATE,CREATEDATE");
  };
};
} // tse
