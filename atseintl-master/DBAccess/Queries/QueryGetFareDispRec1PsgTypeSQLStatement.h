//----------------------------------------------------------------------------
//          File:           QueryGetFareDispRec1PsgTypeSQLStatement.h
//          Description:    QueryGetFareDispRec1PsgTypeSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispRec1PsgType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispRec1PsgTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispRec1PsgTypeSQLStatement() {};
  virtual ~QueryGetFareDispRec1PsgTypeSQLStatement() {};

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

    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,PSGTYPE ");
    this->From("=FAREDISPINCLR1PSGTYP");
    this->Where("USERAPPLTYPE = %1q "
                "and USERAPPL = %2q "
                "and PSEUDOCITYTYPE = %3q "
                "and PSEUDOCITY = %4q "
                "and INCLUSIONCODE = %5q");

    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, INCLUSIONCODE, PSGTYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareDispRec1PsgType* mapRowToFareDispRec1PsgType(Row* row)
  {
    tse::FareDispRec1PsgType* fdRec1PsgType = new tse::FareDispRec1PsgType;

    fdRec1PsgType->userApplType() = row->getChar(USERAPPLTYPE);
    fdRec1PsgType->userAppl() = row->getString(USERAPPL);
    fdRec1PsgType->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdRec1PsgType->pseudoCity() = row->getString(PSEUDOCITY);
    fdRec1PsgType->inclusionCode() = row->getString(INCLUSIONCODE);
    fdRec1PsgType->psgType() = row->getString(PSGTYPE);
    return fdRec1PsgType;
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
class QueryGetAllFareDispRec1PsgTypeSQLStatement
    : public QueryGetFareDispRec1PsgTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  };
};
}

