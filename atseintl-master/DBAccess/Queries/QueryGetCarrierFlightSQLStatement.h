//----------------------------------------------------------------------------
//          File:           QueryGetCarrierFlightSQLStatement.h
//          Description:    QueryGetCarrierFlightSQLStatement
//          Created:        10/29/2007
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
#include "DBAccess/Queries/QueryGetCarrierFlight.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCarrierFlightSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierFlightSQLStatement() {};
  virtual ~QueryGetCarrierFlightSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    SEGCNT,
    INHIBIT,
    ORDERNO,
    MARKETINGCARRIER,
    OPERATINGCARRIER,
    FLT1,
    FLT2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select cf.VENDOR,cf.ITEMNO,cf.CREATEDATE,EXPIREDATE,SEGCNT,INHIBIT,"
                  "       cfs.ORDERNO,MARKETINGCARRIER,OPERATINGCARRIER,FLT1,FLT2");

    //		        this->From("=CARRIERFLT cf LEFT OUTER JOIN =CARRIERFLTSEG cfs"
    //		                   "                       USING (VENDOR,ITEMNO,CREATEDATE)");
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
        "=CARRIERFLT", "cf", "LEFT OUTER JOIN", "=CARRIERFLTSEG", "cfs", joinFields, from);
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

  static tse::CarrierFlight* mapRowToCarrierFlight(Row* row, CarrierFlight* cfPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    CarrierFlight* cf;

    // If Parent hasn't changed, add segs to Prev
    if (cfPrev != nullptr && cfPrev->vendor() == vendor && cfPrev->itemNo() == itemNo &&
        cfPrev->createDate() == createDate)
    { // Just add to Prev
      cf = cfPrev;
    }
    else
    { // Time for a new Parent
      cf = new tse::CarrierFlight;
      cf->vendor() = vendor;
      cf->itemNo() = itemNo;
      cf->createDate() = createDate;

      cf->expireDate() = row->getDate(EXPIREDATE);
      cf->segCnt() = row->getInt(SEGCNT);
      cf->inhibit() = row->getChar(INHIBIT);
    } // New Parent

    // Add new CarrierFlightSeg & return
    if (LIKELY(!row->isNull(ORDERNO)))
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
  } // mapRowToCarrierFlight()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCarrierFlightSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCarrierFlightHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCarrierFlightHistoricalSQLStatement
    : public QueryGetCarrierFlightSQLStatement<QUERYCLASS>
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
}; // class QueryGetCarrierFlightHistoricalSQLStatement
} // tse
