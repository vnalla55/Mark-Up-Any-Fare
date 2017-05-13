//----------------------------------------------------------------------------
//          File:           QueryGetStopoversInfoSQLStatement.h
//          Description:    QueryGetStopoversInfoSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetStopoversInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetStopoversInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetStopoversInfoSQLStatement() {};
  virtual ~QueryGetStopoversInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNOBTW,
    GEOTBLITEMNOAND,
    GEOTBLITEMNOGATEWAY,
    SAMEPOINTSTBLITEMNO,
    CHARGE1FIRSTAMT,
    CHARGE1ADDAMT,
    CHARGE2FIRSTAMT,
    CHARGE2ADDAMT,
    SEGCNT,
    MINSTAYTIME,
    MAXSTAYTIME,
    SAMEPNTSTOPS,
    SAMEPNTTRANSIT,
    SAMEPNTCONNECTIONS,
    CHARGE1NODEC,
    CHARGE2NODEC,
    NOSTOPSMIN,
    NOSTOPSMAX,
    NOSTOPSOUTBOUND,
    NOSTOPSINBOUND,
    OUTORRETURNIND,
    SAMECARRIERIND,
    OJSTOPOVERIND,
    CT2STOPOVERIND,
    CT2PLUSSTOPOVERIND,
    GTWYIND,
    MINSTAYTIMEUNIT,
    MAXSTAYTIMEUNIT,
    CHARGE1APPL,
    CHARGE1TOTAL,
    CHARGE1FIRST,
    CHARGE1ADDNO,
    CHARGE1CUR,
    CHARGE2APPL,
    CHARGE2TOTAL,
    CHARGE2FIRST,
    CHARGE2ADDNO,
    CHARGE2CUR,
    INHIBIT,
    ORDERNO,
    CARRIERIND,
    NOSTOPS,
    CARRIERIN,
    STOPOVERGEOAPPL,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    CARRIEROUT,
    STOPOVERINOUTIND,
    CHARGEIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select s.VENDOR,s.ITEMNO,s.CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
        "       OVERRIDEDATETBLITEMNO,GEOTBLITEMNOBTW,GEOTBLITEMNOAND,GEOTBLITEMNOGATEWAY,"
        "       SAMEPOINTSTBLITEMNO,CHARGE1FIRSTAMT,CHARGE1ADDAMT,CHARGE2FIRSTAMT,"
        "       CHARGE2ADDAMT,SEGCNT,MINSTAYTIME,MAXSTAYTIME,SAMEPNTSTOPS,SAMEPNTTRANSIT,"
        "       SAMEPNTCONNECTIONS,CHARGE1NODEC,CHARGE2NODEC,NOSTOPSMIN,NOSTOPSMAX,"
        "       NOSTOPSOUTBOUND,NOSTOPSINBOUND,OUTORRETURNIND,SAMECARRIERIND,OJSTOPOVERIND,"
        "       CT2STOPOVERIND,CT2PLUSSTOPOVERIND,GTWYIND,MINSTAYTIMEUNIT,MAXSTAYTIMEUNIT,"
        "       CHARGE1APPL,CHARGE1TOTAL,CHARGE1FIRST,CHARGE1ADDNO,CHARGE1CUR,"
        "       CHARGE2APPL,CHARGE2TOTAL,CHARGE2FIRST,CHARGE2ADDNO,CHARGE2CUR,"
        "       INHIBIT,ss.ORDERNO,CARRIERIND,NOSTOPS,CARRIERIN,STOPOVERGEOAPPL,"
        "       LOC1TYPE,LOC1,LOC2TYPE,LOC2,CARRIEROUT,STOPOVERINOUTIND,CHARGEIND");

    //		        this->From("=STOPOVERS s LEFT OUTER JOIN =STOPOVERSSEG ss"
    //		                   "                     USING (VENDOR,ITEMNO,CREATEDATE)");
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
        "=STOPOVERS", "s", "LEFT OUTER JOIN", "=STOPOVERSSEG", "ss", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("s.VENDOR = %1q"
                "    and s.ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("s.VENDOR, s.ITEMNO, s.CREATEDATE, ss.ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::StopoversInfo* mapRowToStopoversInfo(Row* row, StopoversInfo* siPrev)
  { // Load up Parent Determinant Fields
    VendorCode vendor = row->getString(VENDOR);
    uint itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    StopoversInfo* si;

    // If Parent hasn't changed, add to Segs
    if (siPrev != nullptr && siPrev->vendor() == vendor && siPrev->itemNo() == itemNo &&
        siPrev->createDate() == createDate)
    { // Just add to Prev
      si = siPrev;
    }
    else
    { // Time for a new Parent
      si = new tse::StopoversInfo;
      si->vendor() = vendor;
      si->itemNo() = itemNo;
      si->createDate() = createDate;

      si->expireDate() = row->getDate(EXPIREDATE);
      si->unavailTag() = row->getChar(UNAVAILTAG);
      si->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      si->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      si->geoTblItemNoBtw() = row->getInt(GEOTBLITEMNOBTW);
      si->geoTblItemNoAnd() = row->getInt(GEOTBLITEMNOAND);
      si->geoTblItemNoGateway() = row->getInt(GEOTBLITEMNOGATEWAY);
      si->samePointsTblItemNo() = row->getInt(SAMEPOINTSTBLITEMNO);

      si->charge1NoDec() = row->getInt(CHARGE1NODEC);
      si->charge1FirstAmt() =
          QUERYCLASS::adjustDecimal(row->getInt(CHARGE1FIRSTAMT), si->charge1NoDec());
      si->charge1AddAmt() =
          QUERYCLASS::adjustDecimal(row->getInt(CHARGE1ADDAMT), si->charge1NoDec());

      si->charge2NoDec() = row->getInt(CHARGE2NODEC);
      si->charge2FirstAmt() =
          QUERYCLASS::adjustDecimal(row->getInt(CHARGE2FIRSTAMT), si->charge2NoDec());
      si->charge2AddAmt() =
          QUERYCLASS::adjustDecimal(row->getInt(CHARGE2ADDAMT), si->charge2NoDec());

      si->segCnt() = row->getInt(SEGCNT);
      si->minStayTime() = row->getInt(MINSTAYTIME);
      si->maxStayTime() = row->getInt(MAXSTAYTIME);
      si->samePntStops() = row->getInt(SAMEPNTSTOPS);
      si->samePntTransit() = row->getInt(SAMEPNTTRANSIT);
      si->samePntConnections() = row->getInt(SAMEPNTCONNECTIONS);
      si->noStopsMin() = row->getString(NOSTOPSMIN);
      si->noStopsMax() = row->getString(NOSTOPSMAX);
      si->noStopsOutbound() = row->getString(NOSTOPSOUTBOUND);
      si->noStopsInbound() = row->getString(NOSTOPSINBOUND);
      si->outOrReturnInd() = row->getChar(OUTORRETURNIND);
      si->sameCarrierInd() = row->getChar(SAMECARRIERIND);
      si->ojStopoverInd() = row->getChar(OJSTOPOVERIND);
      si->ct2StopoverInd() = row->getChar(CT2STOPOVERIND);
      si->ct2PlusStopoverInd() = row->getChar(CT2PLUSSTOPOVERIND);
      si->gtwyInd() = row->getChar(GTWYIND);
      si->minStayTimeUnit() = row->getChar(MINSTAYTIMEUNIT);
      si->maxStayTimeUnit() = row->getChar(MAXSTAYTIMEUNIT);
      si->charge1Appl() = row->getChar(CHARGE1APPL);
      si->charge1Total() = row->getChar(CHARGE1TOTAL);
      si->charge1First() = row->getString(CHARGE1FIRST);
      si->charge1AddNo() = row->getString(CHARGE1ADDNO);
      si->charge1Cur() = row->getString(CHARGE1CUR);
      si->charge2Appl() = row->getChar(CHARGE2APPL);
      si->charge2Total() = row->getChar(CHARGE2TOTAL);
      si->charge2First() = row->getString(CHARGE2FIRST);
      si->charge2AddNo() = row->getString(CHARGE2ADDNO);
      si->charge2Cur() = row->getString(CHARGE2CUR);
      si->inhibit() = row->getChar(INHIBIT);
    } // New Parent

    if (!row->isNull(ORDERNO))
    { // Add new Segment & return
      StopoversInfoSeg* newSeg = new StopoversInfoSeg;

      newSeg->orderNo() = row->getInt(ORDERNO);
      newSeg->carrierInd() = row->getChar(CARRIERIND);
      newSeg->noStops() = row->getString(NOSTOPS);
      newSeg->carrierIn() = row->getString(CARRIERIN);
      newSeg->stopoverGeoAppl() = row->getChar(STOPOVERGEOAPPL);

      LocKey* loc = &newSeg->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &newSeg->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      newSeg->carrierOut() = row->getString(CARRIEROUT);
      newSeg->stopoverInOutInd() = row->getChar(STOPOVERINOUTIND);
      newSeg->chargeInd() = row->getChar(CHARGEIND);

      si->segs().push_back(newSeg);
    }
    return si;
  } // mapRowToStopoversInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetStopoversInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetStopoversInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetStopoversInfoHistoricalSQLStatement
    : public QueryGetStopoversInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("s.VENDOR = %1q "
                " and s.ITEMNO = %2n"
                " and s.VALIDITYIND = 'Y'"
                " and %3n <= s.EXPIREDATE"
                " and %4n >= s.CREATEDATE");
  }
}; // class QueryGetStopoversInfoHistoricalSQLStatement
} // tse
