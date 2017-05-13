//----------------------------------------------------------------------------
//     2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCarrierFlight.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTaxCarrierFlightSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCarrierFlightSQLStatement() {};
  virtual ~QueryGetTaxCarrierFlightSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    SEGCNT,
    ORDERNO,
    MARKETINGCARRIER,
    OPERATINGCARRIER,
    FLT1,
    FLT2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select cf.VENDOR,cf.ITEMNO,cf.CREATEDATE,EXPIREDATE,SEGCNT,"
                  "       cfs.ORDERNO,MARKETINGCARRIER,OPERATINGCARRIER,FLT1,FLT2");

    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TAXCARRIERFLT", "cf", "LEFT OUTER JOIN", "=TAXCARRIERFLTSEG", "cfs", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("cf.VENDOR = %1q"
                "    and cf.ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxCarrierFlightInfo* mapRowToTaxCarrierFlight(Row* row, TaxCarrierFlightInfo* cfPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    TaxCarrierFlightInfo* cf;

    // If Parent hasn't changed, add segs to Prev
    if (cfPrev != nullptr && cfPrev->vendor() == vendor && cfPrev->itemNo() == itemNo &&
        cfPrev->createDate() == createDate)
    { // Just add to Prev
      cf = cfPrev;
    }
    else
    { // Time for a new Parent
      cf = new tse::TaxCarrierFlightInfo;
      cf->vendor() = vendor;
      cf->itemNo() = itemNo;
      cf->createDate() = createDate;

      cf->expireDate() = row->getDate(EXPIREDATE);
      cf->segCnt() = row->getInt(SEGCNT);
    } // New Parent

    // Add new CarrierFlightSeg & return
    if (!row->isNull(ORDERNO))
    {
      CarrierFlightSeg* s = new CarrierFlightSeg;
      s->orderNo() = row->getInt(ORDERNO);
      s->marketingCarrier() = row->getString(MARKETINGCARRIER);
      s->operatingCarrier() = row->getString(OPERATINGCARRIER);
      s->flt1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
      s->flt2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
      cf->segs().push_back(s);
    }
    return cf;
  } // mapRowToTaxCarrierFlight()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTaxCarrierFlightSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTaxCarrierFlightHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTaxCarrierFlightHistoricalSQLStatement
    : public QueryGetTaxCarrierFlightSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("cf.VENDOR = %1q"
                "  and cf.ITEMNO = %2n"
                "  and cf.VALIDITYIND = 'Y'"
                "  and %3n <= cf.EXPIREDATE"
                "  and %4n >= cf.CREATEDATE");
  }
}; // class QueryGetTaxCarrierFlightHistoricalSQLStatement
} // tse
