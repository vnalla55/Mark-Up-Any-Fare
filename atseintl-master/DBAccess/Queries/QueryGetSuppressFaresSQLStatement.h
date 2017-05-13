//----------------------------------------------------------------------------
//          File:           QueryGetSuppressFaresSQLStatement.h
//          Description:    QueryGetSuppressFaresSQLStatement
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
#include "DBAccess/Queries/QueryGetSuppressFares.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSuppressFarePccSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSuppressFarePccSQLStatement() {};
  virtual ~QueryGetSuppressFarePccSQLStatement() {};

  enum ColumnIndexes
  {
    PSEUDOCITYTYPE = 0,
    PSEUDOCITY,
    CARRIER,
    CREATEDATE,
    FAREDISPLAYTYPE,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    LOCKDATE,
    SSGGROUPNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    std::string configVal("N");
    (Global::config()).getValue("TJR_GROUP", configVal, "FAREDISPLAY_SVC");

    if (configVal == "N")
    {
      this->Command("select PSEUDOCITYTYPE,PSEUDOCITY,CARRIER,CREATEDATE,"
                    "       FAREDISPLAYTYPE,DIRECTIONALITY,LOC1TYPE,LOC1,"
                    "       LOC2TYPE,LOC2,LOCKDATE, 0");
      this->From("=FAREDISPSUPPRESS");
      this->Where("PSEUDOCITY = %1q"
                  "    and PSEUDOCITYTYPE = %2q"
                  "    and 0 = %3n");
    }
    else
    {
      this->Command("select PSEUDOCITYTYPE,PSEUDOCITY,CARRIER,CREATEDATE,"
                    "       FAREDISPLAYTYPE,DIRECTIONALITY,LOC1TYPE,LOC1,"
                    "       LOC2TYPE,LOC2,LOCKDATE,SSGGROUPNO");
      this->From("=FAREDISPSUPPRESS");
      this->Where("PSEUDOCITY = %1q"
                  "    and PSEUDOCITYTYPE = %2q"
                  "    and SSGGROUPNO = %3n");
    }

    if (DataManager::forceSortOrder())
      this->OrderBy("PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,CARRIER,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL(configVal);

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FDSuppressFare* mapRowToFDSuppressFare(Row* row)
  {
    tse::FDSuppressFare* sf = new tse::FDSuppressFare;

    sf->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    sf->pseudoCityCode() = row->getString(PSEUDOCITY);
    sf->carrier() = row->getString(CARRIER);

    // VERIFY THIS DATE..GIVING TROUBLE - Date out of valid range
    sf->createDate() = row->getDate(CREATEDATE);
    sf->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);

    std::string dir = row->getString(DIRECTIONALITY);

    if (dir == "F")
      sf->directionality() = FROM;
    else if (dir == "W")
      sf->directionality() = WITHIN;
    else if (dir == "O")
      sf->directionality() = ORIGIN;
    else if (dir == "X")
      sf->directionality() = TERMINATE;
    else if (dir.empty() || dir == " " || dir == "B")
      sf->directionality() = BETWEEN;

    sf->loc1().locType() = row->getChar(LOC1TYPE);
    sf->loc1().loc() = row->getString(LOC1);

    sf->loc2().locType() = row->getChar(LOC2TYPE);
    sf->loc2().loc() = row->getString(LOC2);

    sf->ssgGroupNo() = row->getInt(SSGGROUPNO);

    return sf;
  } // mapRowToFDSuppressFare()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL(std::string& configVal) {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSuppressFarePccCc
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSuppressFarePccCcSQLStatement : public QueryGetSuppressFarePccSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL(std::string& configVal) override
  {
    if (configVal == "N")
    {
      this->Where("PSEUDOCITY         = %1q"
                  "    and CARRIER        = %2q"
                  "    and PSEUDOCITYTYPE = %3q"
                  "    and 0              = %4n");
    }
    else
    {
      this->Where("PSEUDOCITY         = %1q"
                  "    and CARRIER        = %2q"
                  "    and PSEUDOCITYTYPE = %3q"
                  "    and SSGGROUPNO     = %4n");
    }
  }
};
} // tse
