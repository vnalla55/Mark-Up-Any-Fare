//----------------------------------------------------------------------------
//          File:           QueryGetCopParticipatingNationSQLStatement.h
//          Description:    QueryGetCopParticipatingNationSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetCopParticipatingNation.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCopParticipatingNationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCopParticipatingNationSQLStatement() {};
  virtual ~QueryGetCopParticipatingNationSQLStatement() {};

  enum ColumnIndexes
  {
    NATION = 0,
    COPNATION,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select NATION,COPNATION,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE");
    this->From("=COPPARTICIPATINGNATION");
    this->Where("NATION = %1q "
                "    and COPNATION = %2q "
                "    and %cd <= EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CopParticipatingNation* mapRowToCopParticipatingNation(Row* row)
  {
    tse::CopParticipatingNation* cpn = new tse::CopParticipatingNation;

    cpn->nation() = row->getString(NATION);
    cpn->copNation() = row->getString(COPNATION);
    cpn->createDate() = row->getDate(CREATEDATE);
    cpn->expireDate() = row->getDate(EXPIREDATE);
    cpn->effDate() = row->getDate(EFFDATE);
    cpn->discDate() = row->getDate(DISCDATE);

    return cpn;
  } // mapRowToCopParticipatingNation()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCopParticipatingNationHistoricalSQLStatement
    : public QueryGetCopParticipatingNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("NATION = %1q "
                "    and COPNATION = %2q ");
    this->OrderBy("CREATEDATE");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCopParticipatingNation
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCopParticipatingNationSQLStatement
    : public QueryGetCopParticipatingNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for QueryGetAllCopParticipatingNationHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCopParticipatingNationHistoricalSQLStatement
    : public QueryGetCopParticipatingNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("NATION,COPNATION,CREATEDATE");
  }
};
} // tse
