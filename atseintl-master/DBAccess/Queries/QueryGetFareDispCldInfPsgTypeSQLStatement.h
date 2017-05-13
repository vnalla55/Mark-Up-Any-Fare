//----------------------------------------------------------------------------
//          File:           QueryGetFareDispCldInfPsgTypeSQLStatement.h
//          Description:    QueryGetFareDispCldInfPsgTypeSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispCldInfPsgType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispCldInfPsgTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispCldInfPsgTypeSQLStatement() {};
  virtual ~QueryGetFareDispCldInfPsgTypeSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    PSGTYPEIND,
    PSGTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "       PSGTYPEIND,PSGTYPE");
    this->From(" =FAREDISPINCLCHDINF");
    this->Where("USERAPPLTYPE = %1q"
                "    and USERAPPL = %2q"
                "    and PSEUDOCITYTYPE = %3q"
                "    and PSEUDOCITY = %4q"
                "    and INCLUSIONCODE = %5q");
    if (DataManager::forceSortOrder())
      this->OrderBy(
          "USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, INCLUSIONCODE, PSGTYPEIND, PSGTYPE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareDispCldInfPsgType* mapRowToFareDispCldInfPsg(Row* row)
  {
    FareDispCldInfPsgType* info = new FareDispCldInfPsgType;
    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->pseudoCity() = row->getString(PSEUDOCITY);
    info->inclusionCode() = row->getString(INCLUSIONCODE);
    info->psgTypeInd() = row->getChar(PSGTYPEIND);
    info->psgType() = row->getString(PSGTYPE);
    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDispCldInfPsgType
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDispCldInfPsgTypeSQLStatement
    : public QueryGetFareDispCldInfPsgTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "          PSGTYPEIND,PSGTYPE");
  }
};
} // tse
