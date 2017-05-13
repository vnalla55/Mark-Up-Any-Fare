//----------------------------------------------------------------------------
//          File:           QueryGetDBEGlobalClassSQLStatement.h
//          Description:    QueryGetDBEGlobalClassSQLStatement
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
#include "DBAccess/Queries/QueryGetDBEGlobalClass.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDBEGlobalClassSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDBEGlobalClassSQLStatement() {};
  virtual ~QueryGetDBEGlobalClassSQLStatement() {};

  enum ColumnIndexes
  {
    DBEGLOBALCLASS = 0,
    DBECLASS,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select DBEGLOBALCLASS,DBECLASS");
    this->From("=DBEGLOBALCLASS");
    this->Where("DBEGLOBALCLASS = %1q ");
    this->OrderBy("DBECLASS");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::DBEGlobalClass* mapRowToDBEGlobalClass(Row* row, DBEGlobalClass* dbePrev)
  {
    DBEClass dbeGlbCls = row->getString(DBEGLOBALCLASS);
    tse::DBEGlobalClass* dbe;
    if (dbePrev != nullptr && dbeGlbCls == dbePrev->dbeGlobalClass())
    {
      dbe = dbePrev;
    }
    else
    { // Create new parent
      dbe = new tse::DBEGlobalClass;
      dbe->dbeGlobalClass() = dbeGlbCls;
    }

    if (!row->isNull(DBECLASS))
    { // Create new seg and hook it in
      DBEClass dbeCls = row->getString(DBECLASS);
      dbe->dbeClasses().push_back(dbeCls);
    }

    return dbe;
  } // mapRowToDBEGlobalClass()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllDBEGlobalClass
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllDBEGlobalClassSQLStatement : public QueryGetDBEGlobalClassSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("DBEGLOBALCLASS,DBECLASS");
  }
};
} // tse
