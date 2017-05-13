//----------------------------------------------------------------------------
//          File:           QueryGetFareDispRec8PsgTypeSQLStatement.h
//          Description:    QueryGetFareDispRec8PsgTypeSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispRec8PsgType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispRec8PsgTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispRec8PsgTypeSQLStatement() {};
  virtual ~QueryGetFareDispRec8PsgTypeSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    PSGTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,PSGTYPE");
    this->From("=FAREDISPINCLR8PSGTYP");
    this->Where("USERAPPLTYPE = %1q "
                "and USERAPPL = %2q "
                "and PSEUDOCITYTYPE = %3q "
                "and PSEUDOCITY = %4q "
                "and INCLUSIONCODE = %5q");

    // callback to allow for replacement of SQL clauses by a derived class/template
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, INCLUSIONCODE , PSGTYPE");

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareDispRec8PsgType* mapRowToFareDispRec8PsgType(Row* row)
  {
    tse::FareDispRec8PsgType* fdRec8PsgType = new tse::FareDispRec8PsgType;

    fdRec8PsgType->userApplType() = row->getChar(USERAPPLTYPE);
    fdRec8PsgType->userAppl() = row->getString(USERAPPL);
    fdRec8PsgType->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdRec8PsgType->pseudoCity() = row->getString(PSEUDOCITY);
    fdRec8PsgType->inclusionCode() = row->getString(INCLUSIONCODE);
    fdRec8PsgType->psgType() = row->getString(PSGTYPE);
    return fdRec8PsgType;
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
class QueryGetAllFareDispRec8PsgTypeSQLStatement
    : public QueryGetFareDispRec8PsgTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  };
};
}

