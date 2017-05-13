//----------------------------------------------------------------------------
//          File:           QueryGetRuleCatAlphaCodeSQLStatement.h
//          Description:    QueryGetRuleCatAlphaCodeSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRuleCatAlphaCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetRuleCatAlphaCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRuleCatAlphaCodeSQLStatement() {};
  virtual ~QueryGetRuleCatAlphaCodeSQLStatement() {};

  enum ColumnIndexes
  {
    ALPHAREPRESENTATION = 0,
    DISPLAYCATEGORY,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ALPHAREPRESENTATION,DISPLAYCATEGORY,CREATEDATE");
    this->From("=RULECATALPHACD");
    this->Where("ALPHAREPRESENTATION = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("ALPHAREPRESENTATION, DISPLAYCATEGORY");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::RuleCatAlphaCode* mapRowToRuleCatAlphaCode(Row* row)
  {
    RuleCatAlphaCode* rcac = new RuleCatAlphaCode;
    rcac->alphaRepresentation() = row->getString(ALPHAREPRESENTATION);
    rcac->displayCategory() = row->getInt(DISPLAYCATEGORY);
    rcac->createDate() = row->getDate(CREATEDATE);
    return rcac;
  } // mapRowToRuleCatAlphaCode()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetRuleCatAlphaCodeSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllRuleCatAlphaCodeSQLStatement
    : public QueryGetRuleCatAlphaCodeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where(""); }; // adjustBaseSQL()
}; // class QueryGetAllRuleCatAlphaCodeSQLStatement
}
