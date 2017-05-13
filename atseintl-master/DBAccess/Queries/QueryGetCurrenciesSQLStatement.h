//----------------------------------------------------------------------------
//          File:           QueryGetCurrenciesSQLStatement.h
//          Description:    QueryGetCurrenciesSQLStatement
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
#include "DBAccess/Queries/QueryGetCurrencies.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCurrenciesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCurrenciesSQLStatement() {};
  virtual ~QueryGetCurrenciesSQLStatement() {};

  enum ColumnIndexes
  {
    CUR = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DOMROUNDINGFACTOR,
    ROUNDINGFACTORNODEC,
    NODEC,
    CURNO,
    CURNAME,
    NUCROUNDINGPROCESSNO,
    CONTROLLINGENTITYDESC,
    TAXOVERRIDEROUNDINGUNITNODEC,
    TAXOVERRIDEROUNDINGUNIT,
    TAXOVERRIDEROUNDINGRULE,
    NATION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select cur.CUR,cur.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "       DOMROUNDINGFACTOR,ROUNDINGFACTORNODEC,NODEC,CURNO,CURNAME,"
                  "       NUCROUNDINGPROCESSNO,CONTROLLINGENTITYDESC,"
                  "       TAXOVERRIDEROUNDINGUNITNODEC,TAXOVERRIDEROUNDINGUNIT,"
                  "       TAXOVERRIDEROUNDINGRULE,NATION");

    //		        this->From("=CURRENCY cur LEFT OUTER JOIN =CURRENCYNATIONREST"
    //		                   "                      USING (CUR, CREATEDATE) ");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("CUR");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=CURRENCY", "cur", "LEFT OUTER JOIN", "=CURRENCYNATIONREST", "", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("cur.CUR = %1q "
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("cur.CUR,cur.CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Currency* mapRowToCurrency(Row* row, tse::Currency* curPrev)
  {
    CurrencyCode cur = row->getString(CUR);
    DateTime createDate = row->getDate(CREATEDATE);

    Currency* curr;

    // If Parent hasn't changed, add to Children
    if (curPrev != nullptr && curPrev->cur() == cur && curPrev->createDate() == createDate)
    { // Just add to Prev
      curr = curPrev;
    }
    else
    { // Time for a new Parent
      curr = new tse::Currency;
      curr->cur() = cur;
      curr->createDate() = createDate;

      curr->expireDate() = row->getDate(EXPIREDATE);
      curr->effDate() = row->getDate(EFFDATE);
      curr->discDate() = row->getDate(DISCDATE);

      curr->roundingFactorNoDec() = row->getInt(ROUNDINGFACTORNODEC);
      curr->domRoundingFactor() = row->getInt(DOMROUNDINGFACTOR);
      if (curr->domRoundingFactor() != -1)
      {
        curr->domRoundingFactor() =
            QUERYCLASS::adjustDecimal((int)curr->domRoundingFactor(), curr->roundingFactorNoDec());
      }

      curr->noDec() = row->getInt(NODEC);
      curr->curNo() = row->getInt(CURNO);

      curr->curName() = row->getString(CURNAME);
      curr->nucRoundingProcessNo() = row->getString(NUCROUNDINGPROCESSNO);
      curr->controllingEntityDesc() = row->getString(CONTROLLINGENTITYDESC);

      curr->taxOverrideRoundingUnitNoDec() = row->getInt(TAXOVERRIDEROUNDINGUNITNODEC);
      curr->taxOverrideRoundingUnit() = QUERYCLASS::adjustDecimal(
          row->getInt(TAXOVERRIDEROUNDINGUNIT), curr->taxOverrideRoundingUnitNoDec());

      std::string roundRule = row->getString(TAXOVERRIDEROUNDINGRULE);
      curr->taxOverrideRoundingRule() = NONE;
      if (roundRule == "D")
        curr->taxOverrideRoundingRule() = DOWN;
      else if (roundRule == "N")
        curr->taxOverrideRoundingRule() = NEAREST;
      else if (roundRule == "U")
        curr->taxOverrideRoundingRule() = UP;
    }
    if (!row->isNull(NATION))
    {
      std::string nation = row->getString(NATION);
      curr->nationRes().push_back(nation);
    }

    return curr;
  } // mapRowToCurrency()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCurrenciesHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCurrenciesHistoricalSQLStatement : public QueryGetCurrenciesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("cur.CUR = %1q ");
    this->OrderBy("cur.CREATEDATE");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCurrencies
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCurrenciesSQLStatement : public QueryGetCurrenciesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("cur.CUR,cur.CREATEDATE");
    else
      this->OrderBy("1");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCurrenciesHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCurrenciesHistoricalSQLStatement
    : public QueryGetCurrenciesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("cur.CUR,cur.CREATEDATE");
  }
};
} // tse
