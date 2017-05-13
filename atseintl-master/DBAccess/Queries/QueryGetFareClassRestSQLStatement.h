//----------------------------------------------------------------------------
//          File:           QueryGetFareClassRestSQLStatement.h
//          Description:    QueryGetFareClassRestSQLStatement
//          Created:        11/01/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareClassRest.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareClassRestSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareClassRestSQLStatement() {};
  virtual ~QueryGetFareClassRestSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    CATEGORY,
    ORDERNO,
    FARECLASSTYPEAPPLIND,
    NORMALFARESIND,
    OWRT,
    SAMETRFRULEIND,
    SAMEDIFFIND,
    SAMEMINMAXIND,
    TYPEIND,
    TYPECODE,
    PENALTYSVCCHRGAPPLIND,
    PENALTYRESTIND,
    APPENDAGECODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select "
                  "r.VENDOR,r.ITEMNO,r.CREATEDATE,m.EXPIREDATE,r.CATEGORY,ORDERNO,"
                  "FARECLASSTYPEAPPLIND,"
                  "       NORMALFARESIND,OWRT,SAMETRFRULEIND,SAMEDIFFIND,SAMEMINMAXIND,"
                  "       TYPEIND,TYPECODE,PENALTYSVCCHRGAPPLIND,PENALTYRESTIND,APPENDAGECODE");
    this->From("=FARECLASSTYPESEQ r join =MINORCOMBSUBCAT m "
               "   on  r.VENDOR = m.VENDOR "
               "   and r.ITEMNO = m.ITEMNO "
               "   and r.CATEGORY = m.CATEGORY "
               "   and r.CREATEDATE = m.CREATEDATE ");
    this->Where("r.VENDOR = %1q "
                "    and r.ITEMNO = %2n "
                "    and m.EXPIREDATE >= %cd ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareClassRestRule* mapRowToFareClassRestRule(Row* row)
  {
    tse::FareClassRestRule* fareRest = new tse::FareClassRestRule;

    fareRest->itemNo() = row->getInt(ITEMNO);
    fareRest->createDate() = row->getDate(CREATEDATE);
    fareRest->expireDate() = row->getDate(EXPIREDATE);
    fareRest->orderNo() = row->getInt(ORDERNO);
    fareRest->fareClassTypeApplInd() = row->getChar(FARECLASSTYPEAPPLIND);
    fareRest->normalFaresInd() = row->getChar(NORMALFARESIND);
    fareRest->owrt() = row->getChar(OWRT);
    fareRest->sametrfRuleInd() = row->getChar(SAMETRFRULEIND);
    fareRest->samediffInd() = row->getChar(SAMEDIFFIND);
    fareRest->sameminMaxInd() = row->getChar(SAMEMINMAXIND);
    fareRest->typeInd() = row->getChar(TYPEIND);
    fareRest->typeCode() = row->getString(TYPECODE);
    fareRest->penaltysvcchrgApplInd() = row->getChar(PENALTYSVCCHRGAPPLIND);
    fareRest->penaltyRestInd() = row->getChar(PENALTYRESTIND);
    fareRest->appendageCode() = row->getString(APPENDAGECODE);

    return fareRest;
  } // mapRowToFareClassRestRule()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareClassRestHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareClassRestHistoricalSQLStatement
    : public QueryGetFareClassRestSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("r.VENDOR = %1q "
                " and r.ITEMNO = %2n "
                " and %3n <= m.EXPIREDATE"
                " and %4n >= r.CREATEDATE");
  }
};
} // tse
