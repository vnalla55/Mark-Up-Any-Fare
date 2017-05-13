//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclFareTypeSQLStatement.h
//          Description:    QueryGetFareDispInclFareTypeSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispInclFareType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispInclFareTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispInclFareTypeSQLStatement() {};
  virtual ~QueryGetFareDispInclFareTypeSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    FARETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "       FARETYPE");
    this->From(" =FAREDISPINCLFARETYPE ");
    this->Where("USERAPPLTYPE = %1q "
                "    and USERAPPL = %2q "
                "    and PSEUDOCITYTYPE = %3q "
                "    and PSEUDOCITY = %4q "
                "    and INCLUSIONCODE = %5q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, INCLUSIONCODE, FARETYPE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDispInclFareType* mapRowToFareDispInclFareType(Row* row)
  {
    tse::FareDispInclFareType* fdift = new tse::FareDispInclFareType;

    fdift->userApplType() = row->getChar(USERAPPLTYPE);
    fdift->userAppl() = row->getString(USERAPPL);
    fdift->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdift->pseudoCity() = row->getString(PSEUDOCITY);
    fdift->inclusionCode() = row->getString(INCLUSIONCODE);
    fdift->fareType() = row->getString(FARETYPE);

    return fdift;
  } // mapRowToFareDispInclFareType()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDispInclFareType
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDispInclFareTypeSQLStatement
    : public QueryGetFareDispInclFareTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->From(" =FAREDISPINCLFARETYPE fdincl ");
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  }
};
} // tse
