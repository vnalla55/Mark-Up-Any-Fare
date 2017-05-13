//----------------------------------------------------------------------------
//          File:           QueryGetPFCSQLStatement.h
//          Description:    QueryGetPFCSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetPFC.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPFCSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPFCSQLStatement() {}
  virtual ~QueryGetPFCSQLStatement() {}

  enum ColumnIndexes
  {
    PFCAIRPORT = 0,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    PFCAMT1,
    PFCNODEC1,
    PFCAIRTAXEXCP,
    PFCCHARTEREXCP,
    FREQFLYERIND,
    SEGCNT,
    PFCCUR1,
    VENDOR,
    ORDERNO,
    EXCPCARRIER,
    FLT1,
    FLT2,
    VENDC,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select p.PFCAIRPORT,p.EFFDATE,p.CREATEDATE,EXPIREDATE,"
                  " DISDATE DISCDATE,PFCAMT1,PFCNODEC1,PFCAIRTAXEXCP,"
                  " PFCCHARTEREXCP,FREQFLYERIND,SEGCNT,PFCCUR1,p.VENDOR,"
                  " c.ORDERNO,EXCPCARRIER,FLT1,FLT2,c.VENDOR VENDC,INHIBIT");

    //		        this->From("=PFC p LEFT OUTER JOIN =PFCCARRIEREXCP c"
    //		                  " USING (PFCAIRPORT,EFFDATE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("PFCAIRPORT");
    joinFields.push_back("EFFDATE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=PFC", "p", "LEFT OUTER JOIN", "=PFCCARRIEREXCP", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("VALIDITYIND = 'Y'"
                " and p.PFCAIRPORT = %1q "
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.PFCAIRPORT,p.EFFDATE,p.CREATEDATE,c.ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcPFC* mapRowToPfcPFC(Row* row, PfcPFC* pfcPrev)
  {
    LocCode pfcAirport = row->getString(PFCAIRPORT);
    DateTime effDate;
    effDate = row->getDate(EFFDATE);
    DateTime createDate = row->getDate(CREATEDATE);

    PfcPFC* pfc;

    // If Parent hasn't changed, add to Child (segs)
    if (pfcPrev != nullptr && pfcPrev->pfcAirport() == pfcAirport && pfcPrev->effDate() == effDate &&
        pfcPrev->createDate() == createDate)
    { // Add to Prev
      pfc = pfcPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      pfc = new tse::PfcPFC;
      pfc->pfcAirport() = pfcAirport;
      pfc->effDate() = effDate;
      pfc->createDate() = createDate;
      pfc->expireDate() = row->getDate(EXPIREDATE);
      pfc->discDate() = row->getDate(DISCDATE);
      int noDec = row->getInt(PFCNODEC1);
      int pfcAmt1 = row->getInt(PFCAMT1);
      pfc->pfcAmt1() = QUERYCLASS::adjustDecimal(pfcAmt1, noDec);

      pfc->pfcAirTaxExcp() = row->getChar(PFCAIRTAXEXCP);
      pfc->pfcCharterExcp() = row->getChar(PFCCHARTEREXCP);
      pfc->freqFlyerInd() = row->getChar(FREQFLYERIND);
      pfc->segCnt() = row->getInt(SEGCNT);
      pfc->pfcCur1() = row->getString(PFCCUR1);
      pfc->vendor() = row->getString(VENDOR);
      pfc->inhibit() = row->getChar(INHIBIT);
    }
    if (!row->isNull(ORDERNO))
    {
      PfcCxrExcpt* newCE = new PfcCxrExcpt;

      newCE->orderNo() = row->getInt(ORDERNO);
      newCE->excpCarrier() = row->getString(EXCPCARRIER);
      newCE->flt1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
      newCE->flt2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
      newCE->vendor() = row->getString(VENDC);

      pfc->cxrExcpts().push_back(newCE);
    }

    return pfc;
  } // mapRowToPfcPFC()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllPFCSQLStatement : public QueryGetPFCSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y' ");
    this->OrderBy("p.PFCAIRPORT,p.EFFDATE,p.CREATEDATE,c.ORDERNO");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPFCHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetPFCHistoricalSQLStatement : public QueryGetPFCSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'"
                " and p.PFCAIRPORT = %1q "
                " and %2n <= p.EXPIREDATE"
                " and (%3n >= p.CREATEDATE"
                "  or %4n >= p.EFFDATE)");
    this->OrderBy("p.PFCAIRPORT,p.EFFDATE,p.CREATEDATE,c.ORDERNO");
  }
};
}
