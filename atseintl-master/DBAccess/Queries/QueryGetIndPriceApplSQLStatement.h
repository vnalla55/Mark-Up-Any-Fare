//----------------------------------------------------------------------------
//          File:           QueryGetIndPriceApplSQLStatement.h
//          Description:    QueryGetIndPriceApplSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetIndPriceAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetIndPriceApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetIndPriceApplSQLStatement() {};
  virtual ~QueryGetIndPriceApplSQLStatement() {};

  enum ColumnIndexes
  {
    CREATEDATE = 0,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    CARRIER,
    ORDERNO,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    PRIMEPRICINGAPPL,
    MINIMUMFAREAPPL,
    GLOBALDIR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select p.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,p.CARRIER,ORDERNO,"
                  " DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,PRIMEPRICINGAPPL,"
                  " MINIMUMFAREAPPL,GLOBALDIR");
    this->From("=INDPRICINGAPPL p, =INDPRICINGAPPLSEG c");
    this->Where("p.CARRIER = c.CARRIER "
                " and p.CREATEDATE = c.CREATEDATE "
                " and p.CARRIER = %1q "
                " and %cd <= EXPIREDATE ");
    this->OrderBy("GLOBALDIR desc, ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::IndustryPricingAppl* mapRowToIndustryPricingAppl(Row* row)
  {
    tse::IndustryPricingAppl* indPrice = new tse::IndustryPricingAppl;

    indPrice->createDate() = row->getDate(CREATEDATE);
    indPrice->expireDate() = row->getDate(EXPIREDATE);
    indPrice->effDate() = row->getDate(EFFDATE);
    indPrice->discDate() = row->getDate(DISCDATE);
    indPrice->carrier() = row->getString(CARRIER);
    indPrice->orderNo() = row->getInt(ORDERNO);

    std::string direct = row->getString(DIRECTIONALITY);
    if (direct == "F")
      indPrice->directionality() = FROM;
    else if (direct == "W")
      indPrice->directionality() = WITHIN;
    else if (direct == "O")
      indPrice->directionality() = ORIGIN;
    else if (direct == "X")
      indPrice->directionality() = TERMINATE;
    else if (direct.empty() || direct == " " || direct == "B")
      indPrice->directionality() = BETWEEN;

    LocKey* loc = &indPrice->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);

    loc = &indPrice->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);

    indPrice->primePricingAppl() = row->getChar(PRIMEPRICINGAPPL);
    indPrice->minimumFareAppl() = row->getChar(MINIMUMFAREAPPL);
    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(indPrice->globalDir(), gd);

    return indPrice;
  } // mapRowToIndustryPricingAppl()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetIndPriceApplSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndPriceApplSQLStatement : public QueryGetIndPriceApplSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("p.CARRIER = c.CARRIER"
                " and p.CREATEDATE = c.CREATEDATE"
                " and %cd <= EXPIREDATE");
    this->OrderBy("CARRIER,GLOBALDIR desc,ORDERNO");
  }; // adjustBaseSQL()
}; // class QueryGetAllIndPriceApplSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Adjust base query for historical
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetIndPriceApplHistoricalSQLStatement
    : public QueryGetIndPriceApplSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("p.CARRIER = c.CARRIER "
                " and p.CREATEDATE = c.CREATEDATE "
                " and p.CARRIER = %1q ");

    if (DataManager::forceSortOrder())
    {
      this->OrderBy("GLOBALDIR desc,ORDERNO,CREATEDATE");
    }

  }; // adjustBaseSQL()
}; // class QueryGetIndPriceApplHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Adjust base query for historical
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndPriceApplHistoricalSQLStatement
    : public QueryGetIndPriceApplSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("p.CARRIER = c.CARRIER"
                " and p.CREATEDATE = c.CREATEDATE");
    this->OrderBy("CARRIER,GLOBALDIR desc,ORDERNO,CREATEDATE");
  }; // adjustBaseSQL()
}; // class QueryGetAllIndPriceApplHistoricalSQLStatement
}

