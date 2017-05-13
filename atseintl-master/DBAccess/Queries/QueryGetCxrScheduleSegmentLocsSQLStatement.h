//----------------------------------------------------------------------------
//          File:           QueryGetCxrScheduleSegmentLocsSQLStatement.h
//          Description:    QueryGetCxrScheduleSegmentLocsSQLStatement
//          Created:        11/01/2010
//          Authors:
//
//          Updates:
//
//     (C) 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCxrScheduleSegmentLocs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCxrScheduleSegmentLocsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCxrScheduleSegmentLocsSQLStatement() {};
  virtual ~QueryGetCxrScheduleSegmentLocsSQLStatement() {};

  enum ColumnIndexes
  {
    ORIGAIRPORT = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct ORIGAIRPORT");
    this->From("=SCHEDULESEGMENT");
    this->Where("CARRIER = %1q");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToLocInfo(Row* row, std::vector<LocCode>& codes)
  {
    LocCode code = row->getString(ORIGAIRPORT);
    codes.push_back(code);
  } // mapRowToLocInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCxrScheduleSegmentLocsSQLStatement

} // tse
