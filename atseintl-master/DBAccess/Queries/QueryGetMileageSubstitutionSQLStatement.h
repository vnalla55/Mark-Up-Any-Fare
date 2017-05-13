//----------------------------------------------------------------------------
//          File:           QueryGetMileageSubstitutionSQLStatement.h
//          Description:    QueryGetMileageSubstitutionSQLStatement
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
#include "DBAccess/Queries/QueryGetMileageSubstitution.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMileageSubstitutionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMileageSubstitutionSQLStatement() {};
  virtual ~QueryGetMileageSubstitutionSQLStatement() {};

  enum ColumnIndexes
  {
    SUBSTITUTIONLOC = 0,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    PUBLISHEDLOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SUBSTITUTIONLOC,EFFDATE,CREATEDATE,EXPIREDATE,"
                  "       DISCDATE,PUBLISHEDLOC");
    this->From("=MILEAGESUBSTITUTION");
    this->Where("SUBSTITUTIONLOC = %1q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("SUBSTITUTIONLOC, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MileageSubstitution* mapRowToMileageSubstitution(Row* row)
  {
    tse::MileageSubstitution* ms = new tse::MileageSubstitution;

    ms->substitutionLoc() = row->getString(SUBSTITUTIONLOC);
    ms->effDate() = row->getDate(EFFDATE);
    ms->createDate() = row->getDate(CREATEDATE);
    ms->expireDate() = row->getDate(EXPIREDATE);
    ms->discDate() = row->getDate(DISCDATE);
    ms->publishedLoc() = row->getString(PUBLISHEDLOC);

    return ms;
  } // mapRowToMileageSubstitution()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMileageSubstitutionHistoricalSQLStatement
    : public QueryGetMileageSubstitutionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("SUBSTITUTIONLOC = %1q ");
    this->OrderBy("CREATEDATE");
  };
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMileageSubstitutionSQLStatement
    : public QueryGetMileageSubstitutionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("1");
  };
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMileageSubstitutionHistoricalSQLStatement
    : public QueryGetMileageSubstitutionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("SUBSTITUTIONLOC,CREATEDATE");
  };
};
} // tse
