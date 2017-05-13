
//----------------------------------------------------------------------------
//          File:           QueryGetDSTSQLStatement.h
//          Description:    QueryGetDSTSQLStatement
//          Created:        16/11/2007
//          Authors:        Leszek Banaszek
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
#include "DBAccess/Queries/QueryGetDST.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDSTSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDSTSQLStatement() {};
  virtual ~QueryGetDSTSQLStatement() {};

  enum ColumnIndexes
  {
    DSTGROUP = 0,
    UTCOFFSET,
    DSTSTART,
    DSTFINISH,
    DSTADJMIN,
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select D.DSTGROUP, D.UTCOFFSET, M.DSTSTART, M.DSTFINISH, M.DSTADJMIN ");

    //		        this->From("=DSTGROUP D LEFT OUTER JOIN =DSTADJMINUTES M USING(DSTGROUP) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(1);
    joinFields.push_back("DSTGROUP");
    this->generateJoinString(
        "=DSTGROUP", "D", "LEFT OUTER JOIN", "=DSTADJMINUTES", "M", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("D.DSTGROUP = %1q AND DSTFINISH > %2");
    // this->OrderBy("DSTGROUP");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToDST(Row* row, tse::DST& dst)
  {
    dst.dstgroup() = row->getString(DSTGROUP);
    dst.utcoffset() = row->getInt(UTCOFFSET);

    DSTAdjMinutes* dstAdjMinutes = new DSTAdjMinutes;
    dstAdjMinutes->_dstStart = row->getDate(DSTSTART);
    dstAdjMinutes->_dstFinish = row->getDate(DSTFINISH);
    dstAdjMinutes->_dstAdjmin = row->getInt(DSTADJMIN);
    dst.dstAdjMinutes().push_back(dstAdjMinutes);

  } // mapRowToDST()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
