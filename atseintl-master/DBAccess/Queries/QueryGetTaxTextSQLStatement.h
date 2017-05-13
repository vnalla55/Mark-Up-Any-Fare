//----------------------------------------------------------------------------
//     2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxText.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTaxTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxTextSQLStatement() {};
  virtual ~QueryGetTaxTextSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    TEXT
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, TEXT");
    this->From("=TAXTEXT");
    this->Where("%cd <= EXPIREDATE"
                " and VENDOR = %1q"
                " and ITEMNO = %2q"
                " and VALIDITYIND = 'Y'");

    this->OrderBy("VENDOR,ITEMNO, SEQNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxText* mapRowToTaxText(Row* row, TaxText* cfPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime expireDate = row->getDate(EXPIREDATE);
    std::string text = row->getString(TEXT);

    TaxText* cf;

    if (cfPrev != nullptr)
    {
      cf = cfPrev;
    }
    else
    { // Time for a new Parent
      cf = new tse::TaxText;
      cf->vendor() = vendor;
      cf->itemNo() = itemNo;
      cf->expireDate() = expireDate;
    } // New Parent

    cf->txtMsgs().push_back(text);

    return cf;
  } // mapRowToTaxText()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTaxTextSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTaxTextHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTaxTextHistoricalSQLStatement : public QueryGetTaxTextSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                "  and ITEMNO = %2n"
                "  and VALIDITYIND = 'Y'"
                "  and %3n <= EXPIREDATE"
                "  and %4n >= CREATEDATE");
  }
}; // class QueryGetTaxTextHistoricalSQLStatement
} // tse
