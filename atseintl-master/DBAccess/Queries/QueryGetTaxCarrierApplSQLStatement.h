//----------------------------------------------------------------------------
//     2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCarrierAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTaxCarrierApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCarrierApplSQLStatement() {};
  virtual ~QueryGetTaxCarrierApplSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    APPLIND,
    CARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select f.VENDOR, f.ITEMNO, f.CREATEDATE, f.EXPIREDATE,"
                  " ca.APPLIND APPLIND, ca.CARRIER CARRIER, ca.ORDERNO");

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CREATEDATE");
    joinFields.push_back("ITEMNO");
    this->generateJoinString(
        "=TAXCARRIERAPPL", "f", "LEFT OUTER JOIN", "=TAXCARRIERAPPLSEG", "ca", joinFields, from);
    this->From(from);

    this->Where("%cd <= f.EXPIREDATE"
                " and f.VENDOR = %1q"
                " and f.ITEMNO = %2q"
                " and f.VALIDITYIND = 'Y'");

    this->OrderBy("f.CREATEDATE,ca.ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxCarrierAppl* mapRowToTaxCarrierAppl(Row* row, TaxCarrierAppl* cfPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);
    DateTime expireDate = row->getDate(EXPIREDATE);

    TaxCarrierAppl* cf;

    // If Parent hasn't changed, add msg line to Prev
    if (cfPrev != nullptr && cfPrev->vendor() == vendor && cfPrev->itemNo() == itemNo &&
        cfPrev->createDate() == createDate)
    { // Just add to Prev
      cf = cfPrev;
    }
    else
    { // Time for a new Parent
      cf = new tse::TaxCarrierAppl;
      cf->vendor() = vendor;
      cf->itemNo() = itemNo;
      cf->createDate() = createDate;
      cf->expireDate() = expireDate;

    } // New Parent

    if (!row->isNull(APPLIND))
    {
      TaxCarrierApplSeg* ca = new TaxCarrierApplSeg;
      ca->applInd() = row->getChar(APPLIND);
      ca->carrier() = row->getString(CARRIER);
      cf->segs().push_back(ca);
    }

    return cf;
  } // mapRowToTaxCarrierAppl()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTaxCarrierApplSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTaxCarrierApplHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTaxCarrierApplHistoricalSQLStatement
    : public QueryGetTaxCarrierApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("f.VENDOR = %1q"
                "  and f.ITEMNO = %2n"
                "  and f.VALIDITYIND = 'Y'"
                "  and %3n <= f.EXPIREDATE"
                "  and %4n >= f.CREATEDATE");
  }
}; // class QueryGetTaxCarrierApplHistoricalSQLStatement
} // tse
