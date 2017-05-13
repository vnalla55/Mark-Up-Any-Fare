//----------------------------------------------------------------------------
//          File:           QueryGetIndFareBasisModSQLStatement.h
//          Description:    QueryGetIndFareBasisModSQLStatement
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
#include "DBAccess/Queries/QueryGetIndFareBasisMod.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetIndFareBasisModSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetIndFareBasisModSQLStatement() {};
  virtual ~QueryGetIndFareBasisModSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    USERAPPLTYPE,
    USERAPPL,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    CHANGEFAREBASISIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select CARRIER,USERAPPLTYPE,USERAPPL,CREATEDATE,EXPIREDATE,"
                  " EFFDATE,DISCDATE,CHANGEFAREBASISIND");
    this->From("=INDFAREBASISMOD");
    this->Where("CARRIER = %1q"
                "    and USERAPPLTYPE = %2q"
                "    and USERAPPL = %3q"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER ,USERAPPLTYPE, USERAPPL, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::IndustryFareBasisMod* mapRowToIndustryFareBasisMod(Row* row)
  {
    tse::IndustryFareBasisMod* ifbm = new tse::IndustryFareBasisMod;

    ifbm->carrier() = row->getString(CARRIER);
    ifbm->userApplType() = row->getChar(USERAPPLTYPE);
    ifbm->userAppl() = row->getString(USERAPPL);
    ifbm->createDate() = row->getDate(CREATEDATE);
    ifbm->expireDate() = row->getDate(EXPIREDATE);
    ifbm->effDate() = row->getDate(EFFDATE);
    ifbm->discDate() = row->getDate(DISCDATE);
    ifbm->doNotChangeFareBasisInd() = row->getChar(CHANGEFAREBASISIND);

    return ifbm;
  } // mapRowToIndustryFareBasisMod()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetIndFareBasisModSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetIndFareBasisModHistoricalSQLStatement
    : public QueryGetIndFareBasisModSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q"
                "    and USERAPPLTYPE = %2q"
                "    and USERAPPL = %3q");
  }; // adjustBaseSQL()
}; // class QueryGetIndFareBasisModSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndFareBasisModSQLStatement
    : public QueryGetIndFareBasisModSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("1,2,3");
  }; // adjustBaseSQL()
}; // class QueryGetAllIndFareBasisModSQLStatement
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndFareBasisModHistoricalSQLStatement
    : public QueryGetIndFareBasisModSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("1,2,3");
  }; // adjustBaseSQL()
}; // class QueryGetAllIndFareBasisModSQLStatement
}

