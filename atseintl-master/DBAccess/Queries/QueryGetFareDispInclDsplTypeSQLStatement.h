//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclDsplTypeSQLStatement.h
//          Description:    QueryGetFareDispInclDsplTypeSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetFareDispInclDsplType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispInclDsplTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispInclDsplTypeSQLStatement() {};
  virtual ~QueryGetFareDispInclDsplTypeSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    DISPLAYCATTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "       DISPLAYCATTYPE");
    this->From(" =FAREDISPINCLDSPLTYPE ");
    this->Where("USERAPPLTYPE = %1q "
                "    and USERAPPL = %2q "
                "    and PSEUDOCITYTYPE = %3q "
                "    and PSEUDOCITY = %4q "
                "    and INCLUSIONCODE = %5q ");

    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,DISPLAYCATTYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDispInclDsplType* mapRowToFareDispInclDsplType(Row* row)
  {
    tse::FareDispInclDsplType* fdidt = new tse::FareDispInclDsplType;

    fdidt->userApplType() = row->getChar(USERAPPLTYPE);
    fdidt->userAppl() = row->getString(USERAPPL);
    fdidt->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdidt->pseudoCity() = row->getString(PSEUDOCITY);
    fdidt->inclusionCode() = row->getString(INCLUSIONCODE);
    fdidt->displayCatType() = row->getChar(DISPLAYCATTYPE);

    return fdidt;
  } // mapRowToFareDispInclDsplType()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDispInclDsplType
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDispInclDsplTypeSQLStatement
    : public QueryGetFareDispInclDsplTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  }
};
} // tse
