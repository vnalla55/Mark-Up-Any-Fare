//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayInclCdSQLStatement.h
//          Description:    QueryGetFareDisplayInclCdSQLStatement
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
#include "DBAccess/Queries/QueryGetFareDisplayInclCd.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDisplayInclCdSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDisplayInclCdSQLStatement() {};
  virtual ~QueryGetFareDisplayInclCdSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    INCLUSIONCODE,
    CREATEDATE,
    LOCKDATE,
    MEMONO,
    DESCRIPTION,
    EXCEPTFARETYPE,
    EXCEPTPSGTYPE,
    DISPLTYPEANDORFARETYPE,
    FARETYPEANDORPSGTYPE,
    DISPLTYPEANDORPSGTYPE,
    CREATORBUSINESSUNIT,
    CREATORID,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE,"
                  "       CREATEDATE,LOCKDATE,MEMONO,DESCRIPTION,EXCEPTFARETYPE,EXCEPTPSGTYPE,"
                  "       DISPLTYPEANDORFARETYPE,FARETYPEANDORPSGTYPE,DISPLTYPEANDORPSGTYPE,"
                  "       CREATORBUSINESSUNIT,CREATORID");
    this->From(" =FAREDISPLAYINCLCD");
    this->Where("USERAPPLTYPE = %1q"
                "    and USERAPPL = %2q"
                "    and PSEUDOCITYTYPE = %3q"
                "    and PSEUDOCITY = %4q"
                "    and INCLUSIONCODE = %5q");
    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE, USERAPPL, PSEUDOCITYTYPE, PSEUDOCITY, INCLUSIONCODE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDisplayInclCd* mapRowToFareDisplayInclCd(Row* row)
  {
    tse::FareDisplayInclCd* fdic = new tse::FareDisplayInclCd;

    fdic->userApplType() = row->getChar(USERAPPLTYPE);
    fdic->userAppl() = row->getString(USERAPPL);
    fdic->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdic->pseudoCity() = row->getString(PSEUDOCITY);
    fdic->inclusionCode() = row->getString(INCLUSIONCODE);
    fdic->createDate() = row->getDate(CREATEDATE);
    fdic->lockDate() = row->getDate(LOCKDATE);
    fdic->memoNo() = row->getInt(MEMONO);
    fdic->description() = row->getString(DESCRIPTION);
    fdic->exceptFareType() = row->getChar(EXCEPTFARETYPE);
    fdic->exceptPsgType() = row->getChar(EXCEPTPSGTYPE);
    fdic->displTypeAndOrFareType() = row->getChar(DISPLTYPEANDORFARETYPE);
    fdic->fareTypeAndOrPsgType() = row->getChar(FARETYPEANDORPSGTYPE);
    fdic->displTypeAndOrPsgType() = row->getChar(DISPLTYPEANDORPSGTYPE);
    fdic->creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);
    fdic->creatorId() = row->getString(CREATORID);

    return fdic;
  } // mapRowToFareDisplayInclCd()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDisplayInclCd
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDisplayInclCdSQLStatement
    : public QueryGetFareDisplayInclCdSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,INCLUSIONCODE");
  }
};
} // tse
