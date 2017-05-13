//----------------------------------------------------------------------------
//          File:           QueryGetNegFareRestExtSQLStatement.h
//          Description:    QueryGetNegFareRestExtSQLStatement
//          Created:        9/9/2010
//          Authors:        Artur Krezel
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
#include "DBAccess/Queries/QueryGetNegFareRestExt.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNegFareRestExtSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNegFareRestExtSQLStatement() {};
  virtual ~QueryGetNegFareRestExtSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    STATICVALUECODE,
    STATICVALUECODECOMBIND,
    TOURCODECOMBIND,
    FAREBASISAMTIND,
    TKTFAREDATASEGEXISTIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,STATICVALUECODE,"
                  "STATICVALUECODECOMBIND,TOURCODECOMBIND,FAREBASISAMTIND,TKTFAREDATASEGEXISTIND");
    this->From("=NEGFARERESTEXT");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NegFareRestExt* mapRowToNegFareRestExt(Row* row)
  {
    tse::NegFareRestExt* nfrt = new tse::NegFareRestExt;

    nfrt->vendor() = row->getString(VENDOR);
    nfrt->itemNo() = row->getInt(ITEMNO);
    nfrt->createDate() = row->getDate(CREATEDATE);
    nfrt->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(STATICVALUECODE))
      nfrt->staticValueCode() = row->getString(STATICVALUECODE);
    if (!row->isNull(STATICVALUECODECOMBIND))
      nfrt->staticValueCodeCombInd() = row->getChar(STATICVALUECODECOMBIND);
    if (!row->isNull(TOURCODECOMBIND))
      nfrt->tourCodeCombInd() = row->getChar(TOURCODECOMBIND);
    if (!row->isNull(FAREBASISAMTIND))
      nfrt->fareBasisAmtInd() = row->getChar(FAREBASISAMTIND);
    if (!row->isNull(TKTFAREDATASEGEXISTIND))
      nfrt->tktFareDataSegExistInd() = row->getChar(TKTFAREDATASEGEXISTIND);

    return nfrt;
  } // mapRowToNegFareRestExt()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetNegFareRestExtSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetNegFareRestExtHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNegFareRestExtHistoricalSQLStatement
    : public QueryGetNegFareRestExtSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,STATICVALUECODE,"
        "       STATICVALUECODECOMBIND,TOURCODECOMBIND,FAREBASISAMTIND,TKTFAREDATASEGEXISTIND");
    partialStatement.From("=NEGFARERESTEXTH");
    partialStatement.Where("VENDOR = %1q"
                           " and ITEMNO = %2n"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,STATICVALUECODE,"
        "       STATICVALUECODECOMBIND,TOURCODECOMBIND,FAREBASISAMTIND,TKTFAREDATASEGEXISTIND");
    partialStatement.From("=NEGFARERESTEXT");
    partialStatement.Where("VENDOR = %5q"
                           " and ITEMNO = %6n"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE"
                           ")");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetNegFareRestExtHistoricalSQLStatement
} // tse
