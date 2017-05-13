//----------------------------------------------------------------------------
//          File:           QueryGetToursSQLStatement.h
//          Description:    QueryGetToursSQLStatement
//          Created:        10/5/2007
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
#include "DBAccess/Queries/QueryGetTours.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetToursSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetToursSQLStatement() {}
  virtual ~QueryGetToursSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    MINNIGHTPERCENTNODEC,
    MINNIGHTPERCENT,
    ADULTFAREPCTNODEC,
    ADULTFAREPERCENT,
    REFUND1PERCENTNODEC,
    REFUND1PERCENT,
    REFUND2PERCENTNODEC,
    REFUND2PERCENT,
    GROUPREFPERCENTNODEC,
    GROUPREFUNDPERCENT,
    NIGHTNODEC1,
    MINNIGHTAMT1,
    ADDLNIGHTAMT1,
    NIGHTNODEC2,
    MINNIGHTAMT2,
    ADDLNIGHTAMT2,
    MINTOURNODEC1,
    MINTOURPRICE1,
    MINTOURNODEC2,
    MINTOURPRICE2,
    REFUND1NODEC1,
    REFUND1AMT1,
    REFUND1NODEC2,
    REFUND1AMT2,
    REFUND1GEOTBLITEMNO,
    REFUND2NODEC1,
    REFUND2AMT1,
    REFUND2NODEC2,
    REFUND2AMT2,
    REFUND2GEOTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
    MINNIGHTS,
    MINAGE,
    MAXAGE,
    REFUND1NODAYS,
    REFUND2NODAYS,
    SEGCNT,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    TOURTYPE,
    TOURNO,
    CARRIER,
    MINSTAYIND,
    MINNIGHTAMTAPPLIND1,
    ADDLNIGHTAMTAPPLIND1,
    NIGHTCUR1,
    MINNIGHTAMTAPPLIND2,
    ADDLNIGHTAMTAPPLIND2,
    NIGHTCUR2,
    MINTOURCUR1,
    MINTOURCUR2,
    PREPAYIND,
    PSGRTYPEAPPL,
    PSGRTYPE,
    PSGRWAIVER,
    NONREFUNDABLEIND,
    REFUND1CUR1,
    REFUND1CUR2,
    REFUND1APPLIND,
    REFUND2CUR1,
    REFUND2CUR2,
    REFUND2APPLIND,
    TKTDIND,
    REFUNDMINGRPSIZEIND,
    REFUNDTVLIND,
    WAIVERAPPLIND,
    WAIVER1,
    WAIVER2,
    WAIVER3,
    WAIVER4,
    WAIVER5,
    WAIVER6,
    WAIVER7,
    WAIVER8,
    WAIVER9,
    WAIVER10,
    WAIVER11,
    WAIVER12,
    WAIVER13,
    WAIVER14,
    ORDERNO,
    MINGROUNDNODEC1,
    MINGROUNDAMT1,
    MINGROUNDNODEC2,
    MINGROUNDAMT2,
    TVLGEOTBLITEMNOBETW,
    TVLGEOTBLITEMNOAND,
    MINNOTRANSFERS,
    MINNOSKILIFTTKTS,
    MINNOCARRENTALDAYS,
    MINNOPARKDAYS,
    MINNORESORTDAYS,
    MINNOSHIPDAYS,
    MINNOOTHERACTIVITIES,
    MAXNOFREEDAYS,
    MINGROUNDCUR1,
    MINGROUNDCUR2,
    MINGROUNDAPPLIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select"
        "  r27.VENDOR,r27.ITEMNO,r27.CREATEDATE,EXPIREDATE,MINNIGHTPERCENTNODEC,"
        "  MINNIGHTPERCENT,ADULTFAREPCTNODEC,ADULTFAREPERCENT,REFUND1PERCENTNODEC,"
        "  REFUND1PERCENT,REFUND2PERCENTNODEC,REFUND2PERCENT,GROUPREFPERCENTNODEC,"
        "  GROUPREFUNDPERCENT,NIGHTNODEC1,MINNIGHTAMT1,ADDLNIGHTAMT1,NIGHTNODEC2,"
        "  MINNIGHTAMT2,ADDLNIGHTAMT2,MINTOURNODEC1,MINTOURPRICE1,MINTOURNODEC2,"
        "  MINTOURPRICE2,REFUND1NODEC1,REFUND1AMT1,REFUND1NODEC2,REFUND1AMT2,"
        "  REFUND1GEOTBLITEMNO,REFUND2NODEC1,REFUND2AMT1,REFUND2NODEC2,REFUND2AMT2,"
        "  REFUND2GEOTBLITEMNO,OVERRIDEDATETBLITEMNO,TEXTTBLITEMNO,MINNIGHTS,MINAGE,"
        "  MAXAGE,REFUND1NODAYS,REFUND2NODAYS,SEGCNT,VALIDITYIND,INHIBIT,UNAVAILTAG,"
        "  TOURTYPE,TOURNO,CARRIER,MINSTAYIND,MINNIGHTAMTAPPLIND1,ADDLNIGHTAMTAPPLIND1,"
        "  NIGHTCUR1,MINNIGHTAMTAPPLIND2,ADDLNIGHTAMTAPPLIND2,NIGHTCUR2,MINTOURCUR1,"
        "  MINTOURCUR2,PREPAYIND,PSGRTYPEAPPL,PSGRTYPE,PSGRWAIVER,NONREFUNDABLEIND,"
        "  REFUND1CUR1,REFUND1CUR2,REFUND1APPLIND,REFUND2CUR1,REFUND2CUR2,REFUND2APPLIND,"
        "  TKTDIND,REFUNDMINGRPSIZEIND,REFUNDTVLIND,WAIVERAPPLIND,WAIVER1,WAIVER2,WAIVER3,"
        "  WAIVER4,WAIVER5,WAIVER6,WAIVER7,WAIVER8,WAIVER9,WAIVER10,WAIVER11,WAIVER12,"
        "  WAIVER13,WAIVER14,ORDERNO,MINGROUNDNODEC1,MINGROUNDAMT1,MINGROUNDNODEC2,"
        "  MINGROUNDAMT2,TVLGEOTBLITEMNOBETW,TVLGEOTBLITEMNOAND,MINNOTRANSFERS,"
        "  MINNOSKILIFTTKTS,MINNOCARRENTALDAYS,MINNOPARKDAYS,MINNORESORTDAYS,MINNOSHIPDAYS,"
        "  MINNOOTHERACTIVITIES,MAXNOFREEDAYS,MINGROUNDCUR1,MINGROUNDCUR2,MINGROUNDAPPLIND");

    //		        this->From("=TOURS r27 left outer join =TOURSSEG using
    //(VENDOR,ITEMNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=TOURS", "r27", "left outer join", "=TOURSSEG", "", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" r27.VENDOR = %1q "
                "    and r27.ITEMNO = %2n "
                "    and EXPIREDATE >= %cd "
                "    and VALIDITYIND = 'Y' ");
    this->OrderBy("r27.VENDOR,r27.ITEMNO,r27.CREATEDATE,ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Tours* mapRowToTours(Row* row, Tours* prev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    Tours* rec;
    if (prev != nullptr && prev->vendor() == vendor && prev->itemNo() == itemNo &&
        prev->createDate() == createDate)
    {
      rec = prev;
    }
    else
    {
      rec = new Tours();
      rec->vendor() = vendor;
      rec->itemNo() = itemNo;
      rec->createDate() = createDate;
      rec->expireDate() = row->getDate(EXPIREDATE);
      rec->minNightPercentNoDec() = row->getInt(MINNIGHTPERCENTNODEC);
      rec->minNightPercent() =
          QUERYCLASS::adjustDecimal(row->getInt(MINNIGHTPERCENT), rec->minNightPercentNoDec());
      rec->adultFarePctNoDec() = row->getInt(ADULTFAREPCTNODEC);
      rec->adultFarePercent() =
          QUERYCLASS::adjustDecimal(row->getInt(ADULTFAREPERCENT), rec->adultFarePctNoDec());
      rec->refund1PercentNoDec() = row->getInt(REFUND1PERCENTNODEC);
      rec->refund1Percent() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND1PERCENT), rec->refund1PercentNoDec());
      rec->refund2PercentNoDec() = row->getInt(REFUND2PERCENTNODEC);
      rec->refund2Percent() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND2PERCENT), rec->refund2PercentNoDec());
      rec->groupRefPercentNoDec() = row->getInt(GROUPREFPERCENTNODEC);
      rec->groupRefundPercent() =
          QUERYCLASS::adjustDecimal(row->getInt(GROUPREFUNDPERCENT), rec->groupRefPercentNoDec());
      rec->nightNoDec1() = row->getInt(NIGHTNODEC1);
      rec->minNightAmt1() =
          QUERYCLASS::adjustDecimal(row->getInt(MINNIGHTAMT1), rec->nightNoDec1());
      rec->addlNightAmt1() =
          QUERYCLASS::adjustDecimal(row->getInt(ADDLNIGHTAMT1), rec->nightNoDec1());
      rec->nightNoDec2() = row->getInt(NIGHTNODEC2);
      rec->minNightAmt2() =
          QUERYCLASS::adjustDecimal(row->getInt(MINNIGHTAMT2), rec->nightNoDec2());
      rec->addlNightAmt2() =
          QUERYCLASS::adjustDecimal(row->getInt(ADDLNIGHTAMT2), rec->nightNoDec2());
      rec->minTourNoDec1() = row->getInt(MINTOURNODEC1);
      rec->minTourPrice1() =
          QUERYCLASS::adjustDecimal(row->getInt(MINTOURPRICE1), rec->minTourNoDec1());
      rec->minTourNoDec2() = row->getInt(MINTOURNODEC2);
      rec->minTourPrice2() =
          QUERYCLASS::adjustDecimal(row->getInt(MINTOURPRICE2), rec->minTourNoDec2());
      rec->refund1NoDec1() = row->getInt(REFUND1NODEC1);
      rec->refund1Amt1() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND1AMT1), rec->refund1NoDec1());
      rec->refund1NoDec2() = row->getInt(REFUND1NODEC2);
      rec->refund1Amt2() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND1AMT2), rec->refund1NoDec2());
      rec->refund1GeoTblItemNo() = row->getInt(REFUND1GEOTBLITEMNO);
      rec->refund2NoDec1() = row->getInt(REFUND2NODEC1);
      rec->refund2Amt1() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND2AMT1), rec->refund2NoDec1());
      rec->refund2NoDec2() = row->getInt(REFUND2NODEC2);
      rec->refund2Amt2() =
          QUERYCLASS::adjustDecimal(row->getInt(REFUND2AMT2), rec->refund2NoDec2());
      rec->refund2GeoTblItemNo() = row->getInt(REFUND2GEOTBLITEMNO);
      rec->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      rec->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      rec->minNights() = row->getInt(MINNIGHTS);
      rec->minAge() = row->getInt(MINAGE);
      rec->maxAge() = row->getInt(MAXAGE);
      rec->refund1NoDays() = row->getInt(REFUND1NODAYS);
      rec->refund2NoDays() = row->getInt(REFUND2NODAYS);
      rec->segCnt() = row->getInt(SEGCNT);
      rec->validityInd() = row->getChar(VALIDITYIND);
      rec->inhibit() = row->getChar(INHIBIT);
      rec->unavailTag() = row->getChar(UNAVAILTAG);
      rec->tourType() = row->getString(TOURTYPE);
      rec->tourNo() = row->getString(TOURNO);
      rec->carrier() = row->getString(CARRIER);
      rec->minStayInd() = row->getChar(MINSTAYIND);
      rec->minNightAmtApplInd1() = row->getChar(MINNIGHTAMTAPPLIND1);
      rec->addlNightAmtApplInd1() = row->getChar(ADDLNIGHTAMTAPPLIND1);
      rec->nightCur1() = row->getString(NIGHTCUR1);
      rec->minNightAmtApplInd2() = row->getChar(MINNIGHTAMTAPPLIND2);
      rec->addlNightAmtApplInd2() = row->getChar(ADDLNIGHTAMTAPPLIND2);
      rec->nightCur2() = row->getString(NIGHTCUR2);
      rec->minTourCur1() = row->getString(MINTOURCUR1);
      rec->minTourCur2() = row->getString(MINTOURCUR2);
      rec->prepayInd() = row->getChar(PREPAYIND);
      rec->psgrTypeAppl() = row->getChar(PSGRTYPEAPPL);
      rec->psgrType() = row->getString(PSGRTYPE);
      rec->psgrWaiver() = row->getChar(PSGRWAIVER);
      rec->nonrefundableInd() = row->getChar(NONREFUNDABLEIND);
      rec->refund1Cur1() = row->getString(REFUND1CUR1);
      rec->refund1Cur2() = row->getString(REFUND1CUR2);
      rec->refund1ApplInd() = row->getChar(REFUND1APPLIND);
      rec->refund2Cur1() = row->getString(REFUND2CUR1);
      rec->refund2Cur2() = row->getString(REFUND2CUR2);
      rec->refund2ApplInd() = row->getChar(REFUND2APPLIND);
      rec->tktdInd() = row->getChar(TKTDIND);
      rec->refundMinGrpSizeInd() = row->getChar(REFUNDMINGRPSIZEIND);
      rec->refundTvlInd() = row->getChar(REFUNDTVLIND);
      rec->waiverApplInd() = row->getChar(WAIVERAPPLIND);
      rec->waiver()[0] = row->getChar(WAIVER1);
      rec->waiver()[1] = row->getChar(WAIVER2);
      rec->waiver()[2] = row->getChar(WAIVER3);
      rec->waiver()[3] = row->getChar(WAIVER4);
      rec->waiver()[4] = row->getChar(WAIVER5);
      rec->waiver()[5] = row->getChar(WAIVER6);
      rec->waiver()[6] = row->getChar(WAIVER7);
      rec->waiver()[7] = row->getChar(WAIVER8);
      rec->waiver()[8] = row->getChar(WAIVER9);
      rec->waiver()[9] = row->getChar(WAIVER10);
      rec->waiver()[10] = row->getChar(WAIVER11);
      rec->waiver()[11] = row->getChar(WAIVER12);
      rec->waiver()[12] = row->getChar(WAIVER13);
      rec->waiver()[13] = row->getChar(WAIVER14);
    }

    if (!row->isNull(ORDERNO))
    {
      Tours::ToursSeg* seg = new Tours::ToursSeg();

      seg->orderNo() = row->getInt(ORDERNO);
      seg->minGroundNoDec1() = row->getInt(MINGROUNDNODEC1);
      seg->minGroundAmt1() =
          QUERYCLASS::adjustDecimal(row->getChar(MINGROUNDAMT1), seg->minGroundNoDec1());
      seg->minGroundNoDec2() = row->getInt(MINGROUNDNODEC2);
      seg->minGroundAmt2() =
          QUERYCLASS::adjustDecimal(row->getChar(MINGROUNDAMT2), seg->minGroundNoDec2());
      seg->tvlGeoTblItemNoBetw() = row->getInt(TVLGEOTBLITEMNOBETW);
      seg->tvlGeoTblItemNoAnd() = row->getInt(TVLGEOTBLITEMNOAND);
      seg->minNoTransfers() = row->getInt(MINNOTRANSFERS);
      seg->minNoSkiLiftTkts() = row->getInt(MINNOSKILIFTTKTS);
      seg->minNoCarRentaldays() = row->getInt(MINNOCARRENTALDAYS);
      seg->minNoParkDays() = row->getInt(MINNOPARKDAYS);
      seg->minNoResortDays() = row->getInt(MINNORESORTDAYS);
      seg->minNoShipDays() = row->getInt(MINNOSHIPDAYS);
      seg->minNoOtherActivities() = row->getInt(MINNOOTHERACTIVITIES);
      seg->maxNoFreeDays() = row->getInt(MAXNOFREEDAYS);
      seg->minGroundCur1() = row->getString(MINGROUNDCUR1);
      seg->minGroundCur2() = row->getString(MINGROUNDCUR2);
      seg->minGroundApplInd() = row->getChar(MINGROUNDAPPLIND);

      rec->segs().push_back(seg);
    }

    return rec;
  } // mapRowToTours()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetToursSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetToursHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetToursHistoricalSQLStatement : public QueryGetToursSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" r27.VENDOR = %1q"
                " and r27.ITEMNO = %2n "
                " and r27.VALIDITYIND = 'Y'"
                " and %3n <= r27.EXPIREDATE"
                " and %4n >= r27.CREATEDATE");
  }
}; // class QueryGetToursHistoricalSQLStatement
}
