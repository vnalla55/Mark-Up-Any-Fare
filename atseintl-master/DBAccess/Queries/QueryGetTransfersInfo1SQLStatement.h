//----------------------------------------------------------------------------
//          File:           QueryGetTransfersInfo1SQLStatement.h
//          Description:    QueryGetTransfersInfo1SQLStatement
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
#include "DBAccess/Queries/QueryGetTransfersInfo1.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTransfersInfo1SQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTransfersInfo1SQLStatement() {}
  virtual ~QueryGetTransfersInfo1SQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    NOTRANSFERSMIN,
    NOTRANSFERSMAX,
    PRIMECXRPRIMECXR,
    PRIMEPRIMEMAXTRANSFERS,
    SAMECXRSAMECXR,
    SAMESAMEMAXTRANSFERS,
    PRIMECXROTHERCXR,
    PRIMEOTHERMAXTRANSFERS,
    OTHERCXROTHERCXR,
    OTHEROTHERMAXTRANSFERS,
    NOOFTRANSFERSOUT,
    NOOFTRANSFERSIN,
    NOOFTRANSFERSAPPL,
    FAREBREAKSURFACEIND,
    FAREBREAKSURFACETBLITEMNO,
    EMBEDDEDSURFACEIND,
    EMBEDDEDSURFACETBLITEMNO,
    TRANSFERSCHARGEAPPL,
    MAXNOTRANSFERSCHARGE1,
    MAXNOTRANSFERSCHARGE2,
    CHARGE1CUR1AMT,
    CHARGE2CUR1AMT,
    CUR1,
    NODEC1,
    CHARGE1CUR2AMT,
    CHARGE2CUR2AMT,
    CUR2,
    NODEC2,
    SEGCNT,
    INHIBIT,
    ORDERNO,
    TRANSFERAPPL,
    NOTRANSFERSPERMITTED,
    PRIMEONLINE,
    SAMEONLINE,
    PRIMEINTERLINE,
    OTHERINTERLINE,
    STOPOVERCONNECTIND,
    CARRIERAPPL,
    CARRIERIN,
    CARRIEROUT,
    INCARRIERAPPLTBLITEMNO,
    OUTCARRIERAPPLTBLITEMNO,
    TSI,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    ZONETBLITEMNO,
    BETWEENAPPL,
    GATEWAY,
    RESTRICTION,
    OUTINPORTION,
    CHARGEAPPL,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select t.VENDOR,t.ITEMNO,t.CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "OVERRIDEDATETBLITEMNO,NOTRANSFERSMIN,NOTRANSFERSMAX,PRIMECXRPRIMECXR,"
                  "PRIMEPRIMEMAXTRANSFERS,SAMECXRSAMECXR,SAMESAMEMAXTRANSFERS,"
                  "PRIMECXROTHERCXR,PRIMEOTHERMAXTRANSFERS,OTHERCXROTHERCXR,"
                  "OTHEROTHERMAXTRANSFERS,NOOFTRANSFERSOUT,NOOFTRANSFERSIN,"
                  "NOOFTRANSFERSAPPL,FAREBREAKSURFACEIND,FAREBREAKSURFACETBLITEMNO,"
                  "EMBEDDEDSURFACEIND,EMBEDDEDSURFACETBLITEMNO,TRANSFERSCHARGEAPPL,"
                  "MAXNOTRANSFERSCHARGE1,MAXNOTRANSFERSCHARGE2,CHARGE1CUR1AMT,"
                  "CHARGE2CUR1AMT,CUR1,NODEC1,CHARGE1CUR2AMT,CHARGE2CUR2AMT,CUR2,NODEC2,"
                  "SEGCNT,INHIBIT,s.ORDERNO,TRANSFERAPPL,NOTRANSFERSPERMITTED,"
                  "PRIMEONLINE,SAMEONLINE,PRIMEINTERLINE,OTHERINTERLINE,"
                  "STOPOVERCONNECTIND,CARRIERAPPL,CARRIERIN,CARRIEROUT,"
                  "INCARRIERAPPLTBLITEMNO,OUTCARRIERAPPLTBLITEMNO,TSI,LOC1TYPE,LOC1,"
                  "LOC2TYPE,LOC2,ZONETBLITEMNO,BETWEENAPPL,GATEWAY,RESTRICTION,"
                  "OUTINPORTION,CHARGEAPPL");

    //		        this->From(" =TRANSFERSA t LEFT OUTER JOIN =TRANSFERSASEG s"
    //		                   " USING (VENDOR,ITEMNO,CREATEDATE)");
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
        "=TRANSFERSA", "t", "LEFT OUTER JOIN", "=TRANSFERSASEG", "s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" t.VENDOR = %1q"
                " and t.ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("t.VENDOR, t.ITEMNO, t.CREATEDATE, HASHCODE, s.ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TransfersInfo1* mapRowToTransfersInfo1(Row* row, TransfersInfo1* tiPrev)
  { // Load up Parent Determinant Fields
    VendorCode vendor = row->getString(VENDOR);
    uint itemNo = row->getInt(ITEMNO);
    DateTime createDate = row->getDate(CREATEDATE);

    TransfersInfo1* ti;

    // If Parent hasn't changed, add to Children
    if (tiPrev != nullptr && tiPrev->vendor() == vendor && tiPrev->itemNo() == itemNo &&
        tiPrev->createDate() == createDate)
    { // Just add to Prev
      ti = tiPrev;
    }
    else
    { // Time for a new Parent
      ti = new tse::TransfersInfo1;
      ti->vendor() = vendor;
      ti->itemNo() = itemNo;
      ti->createDate() = createDate;

      ti->expireDate() = row->getDate(EXPIREDATE);
      ti->unavailTag() = row->getChar(UNAVAILTAG);
      ti->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      ti->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      ti->noTransfersMin() = row->getString(NOTRANSFERSMIN);
      ti->noTransfersMax() = row->getString(NOTRANSFERSMAX);
      ti->primeCxrPrimeCxr() = row->getChar(PRIMECXRPRIMECXR);
      ti->primePrimeMaxTransfers() = row->getString(PRIMEPRIMEMAXTRANSFERS);
      ti->sameCxrSameCxr() = row->getChar(SAMECXRSAMECXR);
      ti->sameSameMaxTransfers() = row->getString(SAMESAMEMAXTRANSFERS);
      ti->primeCxrOtherCxr() = row->getChar(PRIMECXROTHERCXR);
      ti->primeOtherMaxTransfers() = row->getString(PRIMEOTHERMAXTRANSFERS);
      ti->otherCxrOtherCxr() = row->getChar(OTHERCXROTHERCXR);
      ti->otherOtherMaxTransfers() = row->getString(OTHEROTHERMAXTRANSFERS);
      ti->noOfTransfersOut() = row->getString(NOOFTRANSFERSOUT);
      ti->noOfTransfersIn() = row->getString(NOOFTRANSFERSIN);
      ti->noOfTransfersAppl() = row->getChar(NOOFTRANSFERSAPPL);
      ti->fareBreakSurfaceInd() = row->getChar(FAREBREAKSURFACEIND);
      ti->fareBreakSurfaceTblItemNo() = row->getInt(FAREBREAKSURFACETBLITEMNO);
      ti->embeddedSurfaceInd() = row->getChar(EMBEDDEDSURFACEIND);
      ti->embeddedSurfaceTblItemNo() = row->getInt(EMBEDDEDSURFACETBLITEMNO);
      ti->transfersChargeAppl() = row->getChar(TRANSFERSCHARGEAPPL);
      ti->maxNoTransfersCharge1() = row->getString(MAXNOTRANSFERSCHARGE1);
      ti->maxNoTransfersCharge2() = row->getString(MAXNOTRANSFERSCHARGE2);
      ti->noDec1() = row->getInt(NODEC1);
      ti->charge1Cur1Amt() = QUERYCLASS::adjustDecimal(row->getInt(CHARGE1CUR1AMT), ti->noDec1());
      ti->charge2Cur1Amt() = QUERYCLASS::adjustDecimal(row->getInt(CHARGE2CUR1AMT), ti->noDec1());
      ti->cur1() = row->getString(CUR1);
      ti->noDec2() = row->getInt(NODEC2);
      ti->charge1Cur2Amt() = QUERYCLASS::adjustDecimal(row->getInt(CHARGE1CUR2AMT), ti->noDec2());
      ti->charge2Cur2Amt() = QUERYCLASS::adjustDecimal(row->getInt(CHARGE2CUR2AMT), ti->noDec2());
      ti->cur2() = row->getString(CUR2);
      ti->segCnt() = row->getInt(SEGCNT);
      ti->inhibit() = row->getChar(INHIBIT);
    } // New Parent

    if (!row->isNull(ORDERNO))
    { // Add new Segment & return
      TransfersInfoSeg1* newSeg = new TransfersInfoSeg1;
      newSeg->orderNo() = row->getInt(ORDERNO);
      newSeg->transferAppl() = row->getChar(TRANSFERAPPL);
      newSeg->noTransfersPermitted() = row->getString(NOTRANSFERSPERMITTED);
      newSeg->primeOnline() = row->getChar(PRIMEONLINE);
      newSeg->sameOnline() = row->getChar(SAMEONLINE);
      newSeg->primeInterline() = row->getChar(PRIMEINTERLINE);
      newSeg->otherInterline() = row->getChar(OTHERINTERLINE);
      newSeg->stopoverConnectInd() = row->getChar(STOPOVERCONNECTIND);
      newSeg->carrierAppl() = row->getChar(CARRIERAPPL);
      newSeg->carrierIn() = row->getString(CARRIERIN);
      newSeg->carrierOut() = row->getString(CARRIEROUT);
      newSeg->inCarrierApplTblItemNo() = row->getInt(INCARRIERAPPLTBLITEMNO);
      newSeg->outCarrierApplTblItemNo() = row->getInt(OUTCARRIERAPPLTBLITEMNO);
      newSeg->tsi() = row->getInt(TSI);

      LocKey* loc = &newSeg->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &newSeg->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      newSeg->zoneTblItemNo() = row->getInt(ZONETBLITEMNO);
      newSeg->betweenAppl() = row->getChar(BETWEENAPPL);
      newSeg->gateway() = row->getChar(GATEWAY);
      newSeg->restriction() = row->getChar(RESTRICTION);
      newSeg->outInPortion() = row->getChar(OUTINPORTION);
      newSeg->chargeAppl() = row->getChar(CHARGEAPPL);

      ti->segs().push_back(newSeg);
    }

    return ti;
  } // mapRowToTransfersInfo1()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTransfersInfo1SQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTransfersInfo1Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTransfersInfo1HistoricalSQLStatement
    : public QueryGetTransfersInfo1SQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" t.VENDOR = %1q"
                " and t.ITEMNO = %2n "
                " and t.VALIDITYIND = 'Y'"
                " and %3n <= t.EXPIREDATE"
                " and %4n >= t.CREATEDATE");
  }
}; // class QueryGetTransfersInfo1HistoricalSQLStatement
}

