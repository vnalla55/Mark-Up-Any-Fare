//----------------------------------------------------------------------------
//          File:           QueryGetRuleCatDescSQLStatement.h
//          Description:    QueryGetRuleCatDescSQLStatement
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
#include "DBAccess/Queries/QueryGetRuleCatDesc.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetRuleCatDescSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRuleCatDescSQLStatement() {};
  virtual ~QueryGetRuleCatDescSQLStatement() {};

  enum ColumnIndexes
  {
    DISPLAYCATEGORY = 0,
    DESCRIPTION,
    SHORTDESC,
    TEXT,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select d1.DISPLAYCATEGORY,d1.DESCRIPTION,d2.DESCRIPTION SHORTDESC,ft.TEXT,d1.CREATEDATE");
    this->From("=RULECATEGORYDESC d1 "
               "  inner join =RULECATEGORYDESC d2 "
               "     on d1.DISPLAYCATEGORY = d2.DISPLAYCATEGORY "
               "  left outer join =RULECATEGORYDESC d3 "
               "     on  d2.DISPLAYCATEGORY = d3.DISPLAYCATEGORY "
               "     and d3.DESCRIPTIONTYPE = 'A' "
               "  left outer join =FREETEXTSEG ft "
               "     on  d3.MESSAGETYPE = ft.MESSAGETYPE "
               "     and d3.ITEMNO = ft.ITEMNO ");
    this->Where("d1.DISPLAYCATEGORY = %1n "
                " and d1.DESCRIPTIONTYPE = 'L' "
                " and d2.DESCRIPTIONTYPE = 'M' ");
    if (DataManager::forceSortOrder())
      this->OrderBy("d3.DISPLAYCATEGORY, d3.DESCRIPTIONTYPE, d3.SYSTEMASSUMPTIONMSGAPPL, "
                    "d3.MESSAGETYPE, ft.ITEMNO, ft.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::RuleCategoryDescInfo* mapRowToRuleCatDesc(Row* row)
  {
    RuleCategoryDescInfo* rcd = new RuleCategoryDescInfo;
    rcd->category() = row->getInt(DISPLAYCATEGORY);
    rcd->description() = row->getString(DESCRIPTION);
    rcd->shortDescription() = row->getString(SHORTDESC);
    rcd->createDate() = row->getDate(CREATEDATE);

    if (!row->isNull(TEXT))
      rcd->defaultMsg() = row->getString(TEXT);

    return rcd;
  } // mapRowToRuleCatDesc()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetRuleCatDescSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllRuleCatDescSQLStatement : public QueryGetRuleCatDescSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("d1.DESCRIPTIONTYPE = 'L' "
                "   and d2.DESCRIPTIONTYPE = 'M' ");
  }; // adjustBaseSQL()
}; // class QueryGetAllRuleCatDescSQLStatement
}
