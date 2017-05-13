//----------------------------------------------------------------------------
//          File:           QueryGetFareDispTemplateSQLStatement.h
//          Description:    QueryGetFareDispTemplateSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDispTemplate.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispTemplateSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispTemplateSQLStatement() {};
  virtual ~QueryGetFareDispTemplateSQLStatement() {};

  enum ColumnIndexes
  {
    TEMPLATEID = 0,
    TEMPLATETYPE,
    LINESTYLE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command("select TEMPLATEID,TEMPLATETYPE,LINESTYLE");
    this->From("=FAREDISPTEMPLATE");
    this->Where("TEMPLATEID = %1q "
                "and TEMPLATETYPE = %2q");
    this->OrderBy("LINESTYLE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareDispTemplate* mapRowToFareDispTemplate(Row* row)
  {
    tse::FareDispTemplate* fdTemplate = new tse::FareDispTemplate;

    fdTemplate->templateID() = row->getInt(TEMPLATEID);
    fdTemplate->templateType() = row->getChar(TEMPLATETYPE);
    fdTemplate->lineStyle() = row->getChar(LINESTYLE);
    return fdTemplate;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDispTemplateSQLStatement
    : public QueryGetFareDispTemplateSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("TEMPLATEID,TEMPLATETYPE,LINESTYLE");
  };
};
}

