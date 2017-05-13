//----------------------------------------------------------------------------
//          File:           QueryGetFareDispInclRuleTrfSQLStatement.h
//          Description:    QueryGetFareDispInclRuleTrfSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDispInclRuleTrf.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDispInclRuleTrfSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDispInclRuleTrfSQLStatement() {};
  virtual ~QueryGetFareDispInclRuleTrfSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    VENDOR,
    RULETARIFF,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "       VENDOR,RULETARIFF");
    this->From("=FAREDISPINCLRULETRF");
    this->Where("USERAPPLTYPE = %1q"
                "    and USERAPPL = %2q"
                "    and PSEUDOCITYTYPE = %3q"
                "    and PSEUDOCITY = %4q"
                "    and INCLUSIONCODE = %5q");
    if (DataManager::forceSortOrder())
      this->OrderBy(
          "USERAPPLTYPE, USERAPPL,PSEUDOCITYTYPE,  PSEUDOCITY, INCLUSIONCODE, VENDOR, RULETARIFF");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDispInclRuleTrf* mapRowToFareDispInclRuleTrf(Row* row)
  {
    tse::FareDispInclRuleTrf* fdirt = new tse::FareDispInclRuleTrf;

    fdirt->userApplType() = row->getChar(USERAPPLTYPE);
    fdirt->userAppl() = row->getString(USERAPPL);
    fdirt->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdirt->pseudoCity() = row->getString(PSEUDOCITY);
    fdirt->inclusionCode() = row->getString(INCLUSIONCODE);
    fdirt->vendorCode() = row->getString(VENDOR);
    fdirt->ruleTariff() = row->getInt(RULETARIFF);

    return fdirt;
  } // mapRowToFareDispInclRuleTrf()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDispInclRuleTrf
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDispInclRuleTrfSQLStatement
    : public QueryGetFareDispInclRuleTrfSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  }
};
} // tse
