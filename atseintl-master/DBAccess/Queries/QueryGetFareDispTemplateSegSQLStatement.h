//----------------------------------------------------------------------------
//          File:           QueryGetFareDispTemplateSegSQLStatement.h
//          Description:    QueryGetFareDispTemplateSegSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispTemplateSeg.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispTemplateSegSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispTemplateSegSQLStatement() {};
  virtual ~QueryGetFareDispTemplateSegSQLStatement() {};

  enum ColumnIndexes
  {
    TEMPLATEID = 0,
    TEMPLATETYPE,
    COLUMNELEMENT,
    ELEMENTSTART,
    ELEMENTLENGTH,
    ELEMENTJUSTIFY,
    ELEMENTDATEFORMAT,
    HEADER,
    HEADERSTART,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TEMPLATEID,TEMPLATETYPE,COLUMNELEMENT,ELEMENTSTART,ELEMENTLENGTH,"
                  "ELEMENTJUSTIFY,ELEMENTDATEFORMAT,HEADER,HEADERSTART");
    this->From("=FAREDISPTEMPLATESEG");
    this->Where("TEMPLATEID = %1q "
                "and TEMPLATETYPE = %2q");
    this->OrderBy("COLUMNELEMENT");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareDispTemplateSeg* mapRowToFareDispTemplateSeg(Row* row)
  {
    FareDispTemplateSeg* fdTemplateSeg = new FareDispTemplateSeg;

    fdTemplateSeg->templateID() = row->getInt(TEMPLATEID);
    fdTemplateSeg->templateType() = row->getChar(TEMPLATETYPE);
    fdTemplateSeg->columnElement() = row->getInt(COLUMNELEMENT);
    fdTemplateSeg->elementStart() = row->getInt(ELEMENTSTART);
    fdTemplateSeg->elementLength() = row->getInt(ELEMENTLENGTH);
    fdTemplateSeg->elementJustify() = row->getChar(ELEMENTJUSTIFY);
    fdTemplateSeg->elementDateFormat() = row->getChar(ELEMENTDATEFORMAT);
    fdTemplateSeg->header() = row->getString(HEADER);
    fdTemplateSeg->headerStart() = row->getInt(HEADERSTART);

    return fdTemplateSeg;
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
class QueryGetAllFareDispTemplateSegSQLStatement
    : public QueryGetFareDispTemplateSegSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("TEMPLATEID,TEMPLATETYPE,COLUMNELEMENT");
  };
};
}

