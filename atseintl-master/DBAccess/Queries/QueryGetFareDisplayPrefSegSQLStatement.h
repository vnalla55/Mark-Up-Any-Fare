//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayPrefSegSQLStatement.h
//          Description:    QueryGetFareDisplayPrefSegSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDisplayPrefSeg.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDisplayPrefSegSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDisplayPrefSegSQLStatement() {};
  virtual ~QueryGetFareDisplayPrefSegSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    SSGGROUPNO,
    SURCHARGETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,SURCHARGETYPE");
    this->From("=FAREDISPLAYPREFSEG");
    this->Where("USERAPPLTYPE = %1q"
                "    and USERAPPL = %2q"
                "    and PSEUDOCITYTYPE = %3q"
                "    and PSEUDOCITY = %4q"
                "    and SSGGROUPNO = %5n ");

    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,SURCHARGETYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDisplayPrefSeg* mapRowToFareDisplayPrefSeg(Row* row)
  {
    tse::FareDisplayPrefSeg* fdps = new tse::FareDisplayPrefSeg;

    fdps->userApplType() = row->getChar(USERAPPLTYPE);
    fdps->userAppl() = row->getString(USERAPPL);
    fdps->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdps->pseudoCity() = row->getString(PSEUDOCITY);
    fdps->ssgGroupNo() = row->getInt(SSGGROUPNO);
    fdps->surchargeType() = row->getChar(SURCHARGETYPE);

    return fdps;
  } // mapRowToFareDisplayPrefSeg()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDisplayPrefSeg
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDisplayPrefSegSQLStatement
    : public QueryGetFareDisplayPrefSegSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,SURCHARGETYPE");
    else
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO");
  }
};
} // tse
