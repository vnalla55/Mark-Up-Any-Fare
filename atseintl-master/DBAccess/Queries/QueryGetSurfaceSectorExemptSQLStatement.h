//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceSectorExemptSQLStatement.h
//          Description:    QueryGetSurfaceSectorExemptSQLStatement
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
#include "DBAccess/Queries/QueryGetSurfaceSectorExempt.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSurfaceSectorExemptSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSurfaceSectorExemptSQLStatement() {};
  virtual ~QueryGetSurfaceSectorExemptSQLStatement() {};

  enum ColumnIndexes
  {
    ORIGLOC = 0,
    DESTLOC,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ORIGLOC,DESTLOC,EFFDATE,CREATEDATE,EXPIREDATE,DISCDATE");
    this->From("=SURFACESECTOREXEMPT");
    this->Where("ORIGLOC = %1q "
                "    and DESTLOC = %2q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("ORIGLOC, DESTLOC, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SurfaceSectorExempt* mapRowToSurfaceSectorExempt(Row* row)
  {
    tse::SurfaceSectorExempt* sse = new tse::SurfaceSectorExempt;

    sse->origLoc() = row->getString(ORIGLOC);
    sse->destLoc() = row->getString(DESTLOC);
    sse->effDate() = row->getDate(EFFDATE);
    sse->createDate() = row->getDate(CREATEDATE);
    sse->expireDate() = row->getDate(EXPIREDATE);
    sse->discDate() = row->getDate(DISCDATE);

    return sse;
  } // mapRowToSurfaceSectorExempt()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSurfaceSectorExemptHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSurfaceSectorExemptHistoricalSQLStatement
    : public QueryGetSurfaceSectorExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("ORIGLOC = %1q "
                "    and DESTLOC = %2q ");
    this->OrderBy("CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSurfaceSectorExempt
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSurfaceSectorExemptSQLStatement
    : public QueryGetSurfaceSectorExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("ORIGLOC, DESTLOC, CREATEDATE");
    else
      this->OrderBy("1,2");
  }
};

//////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllSurfaceSectorExemptHistorical
//////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSurfaceSectorExemptHistoricalSQLStatement
    : public QueryGetSurfaceSectorExemptSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("ORIGLOC,DESTLOC,CREATEDATE");
  }
};
} // tse
