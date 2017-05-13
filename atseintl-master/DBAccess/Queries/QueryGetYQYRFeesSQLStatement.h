//----------------------------------------------------------------------------
//          File:           QueryGetYQYRFeesSQLStatement.h
//          Description:    QueryGetYQYRFeesSQLStatement
//          Created:        10/6/2007
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
#include "DBAccess/Queries/QueryGetYQYRFees.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetYQYRFeesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetYQYRFeesSQLStatement() {}
  virtual ~QueryGetYQYRFeesSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    TAXCODE,
    SUBCODE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FIRSTTKTDATE,
    LASTTKTDATE,
    RETURNTOORIGIN,
    PSGTYPE,
    POSLOCTYPE,
    POSLOC,
    POSZONETBLITEMNO,
    POSLOCALETYPE,
    POSAGENCYPCC,
    POSIATATVLAGENCYNO,
    POTLOCTYPE,
    POTLOC,
    POTZONETBLITEMNO,
    JOURNEYLOC1IND,
    JOURNEYLOCTYPE1,
    JOURNEYLOC1,
    JOURNEYZONETBLITEMNO1,
    JOURNEYLOCTYPE2,
    JOURNEYLOC2,
    JOURNEYZONETBLITEMNO2,
    VIALOCTYPE,
    VIALOC,
    VIAZONETBLITEMNO,
    WHOLLYWITHINLOCTYPE,
    WHOLLYWITHINLOC,
    WHOLLYWITHINZONETBLITEMNO,
    SECTORPORTIONOFTVLIND,
    DIRECTIONALITY,
    SECTORPORTIONLOCTYPE1,
    SECTORPORTIONLOC1,
    SECTORPORTIONZONETBLITEMNO1,
    SECTORPORTIONLOCTYPE2,
    SECTORPORTIONLOC2,
    SECTORPORTIONZONETBLITEMNO2,
    SECTORPORTIONVIALOCTYPE,
    SECTORPORTIONVIALOC,
    SECTORPORTIONVIAZONETBLITEMNO,
    STOPCONNECTIND,
    STOPCONNECTTIME,
    STOPCONNECTUNIT,
    CONNECTEXEMPTIND,
    INTLDOMIND,
    BOOKINGCODE1,
    BOOKINGCODE2,
    BOOKINGCODE3,
    EQUIPCODE,
    FAREBASIS,
    BATCHNO,
    BATCHCI,
    FEEAMOUNT,
    CUR,
    NODEC,
    PERCENT,
    PERCENTNODEC,
    TAXINCLUDEDIND,
    FEEAPPLIND,
    TAXCARRIERAPPLTBLITEMNO,
    TAXCARRIERFLTTBLITEMNO,
    TAXTEXTTBLITEMNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select f.VENDOR,f.CARRIER,TAXCODE,SUBCODE,SEQNO,f.CREATEDATE,f.EXPIREDATE,"
                  " EFFDATE,DISCDATE,FIRSTTKTDATE,LASTTKTDATE,RETURNTOORIGIN,PSGTYPE,POSLOCTYPE,"
                  " POSLOC,POSZONETBLITEMNO,POSLOCALETYPE,POSAGENCYPCC,POSIATATVLAGENCYNO,"
                  " POTLOCTYPE,POTLOC,POTZONETBLITEMNO,JOURNEYLOC1IND,JOURNEYLOCTYPE1,JOURNEYLOC1,"
                  " JOURNEYZONETBLITEMNO1,JOURNEYLOCTYPE2,JOURNEYLOC2,JOURNEYZONETBLITEMNO2,"
                  " VIALOCTYPE,VIALOC,VIAZONETBLITEMNO,WHOLLYWITHINLOCTYPE,WHOLLYWITHINLOC,"
                  " WHOLLYWITHINZONETBLITEMNO,SECTORPORTIONOFTVLIND,DIRECTIONALITY,SECTORPORTIONLOCTYPE1,"
                  " SECTORPORTIONLOC1,SECTORPORTIONZONETBLITEMNO1,SECTORPORTIONLOCTYPE2,SECTORPORTIONLOC2,"
                  " SECTORPORTIONZONETBLITEMNO2,SECTORPORTIONVIALOCTYPE,SECTORPORTIONVIALOC,"
                  " SECTORPORTIONVIAZONETBLITEMNO,STOPCONNECTIND,STOPCONNECTTIME,STOPCONNECTUNIT,"
                  " CONNECTEXEMPTIND,INTLDOMIND,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,EQUIPCODE,"
                  " FAREBASIS,BATCHNO,BATCHCI,FEEAMOUNT,CUR,NODEC,PERCENT,PERCENTNODEC,TAXINCLUDEDIND,"
                  " FEEAPPLIND,TAXCARRIERAPPLTBLITEMNO,TAXCARRIERFLTTBLITEMNO,TAXTEXTTBLITEMNO");
    this->From("=YQYRFEES f");
    this->Where("%cd <= f.EXPIREDATE"
                " and f.LASTTKTDATE >= SYSDATE - 1"
                " and f.CARRIER = %1q "
                " and f.VALIDITYIND = 'Y'");
    if (DataManager::forceSortOrder())
      this->OrderBy("f.VENDOR,f.CARRIER,f.TAXCODE,f.SUBCODE,f.SEQNO,f.CREATEDATE");
    else
      this->OrderBy("f.VENDOR,f.CARRIER,TAXCODE,SUBCODE,SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static YQYRFees* mapRowToYQYRFees(Row* row, YQYRFees* feePrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    TaxCode taxCode = row->getString(TAXCODE);
    Indicator subCode = row->getChar(SUBCODE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    YQYRFees* fee;

    fee = new tse::YQYRFees;
    fee->vendor() = vendor;
    fee->carrier() = carrier;
    fee->taxCode() = taxCode;
    fee->subCode() = subCode;
    fee->seqNo() = seqNo;
    fee->createDate() = createDate;

    fee->expireDate() = row->getDate(EXPIREDATE);

    fee->effDate() = row->getDate(EFFDATE);
    fee->discDate() = row->getDate(DISCDATE);
    fee->firstTktDate() = row->getDate(FIRSTTKTDATE);
    fee->lastTktDate() = row->getDate(LASTTKTDATE);
    fee->lastTktDate() = fee->lastTktDate() + Hours(23) + Minutes(59) + Seconds(59);

    fee->returnToOrigin() = row->getChar(RETURNTOORIGIN);
    fee->psgType() = row->getString(PSGTYPE);

    fee->posLocType() = LocType(row->getString(POSLOCTYPE)[0]);
    fee->posLoc() = getValidLoc(row, fee->posLocType(), POSLOC, POSZONETBLITEMNO);

    fee->posLocaleType() = row->getChar(POSLOCALETYPE);
    fee->posAgencyPCC() = row->getString(POSAGENCYPCC);
    fee->posIataTvlAgencyNo() = row->getString(POSIATATVLAGENCYNO);

    fee->potLocType() = LocType(row->getString(POTLOCTYPE)[0]);
    fee->potLoc() = getValidLoc(row, fee->potLocType(), POTLOC, POTZONETBLITEMNO);

    fee->journeyLoc1Ind() = row->getChar(JOURNEYLOC1IND);
    fee->journeyLocType1() = row->getChar(JOURNEYLOCTYPE1);
    fee->journeyLoc1() = getValidLoc(row, fee->journeyLocType1(), JOURNEYLOC1, JOURNEYZONETBLITEMNO1);

    fee->journeyLocType2() = row->getChar(JOURNEYLOCTYPE2);
    fee->journeyLoc2() = getValidLoc(row, fee->journeyLocType2(), JOURNEYLOC2, JOURNEYZONETBLITEMNO2);

    fee->viaLocType() = row->getChar(VIALOCTYPE);
    fee->viaLoc() = getValidLoc(row, fee->viaLocType(), VIALOC, VIAZONETBLITEMNO);

    fee->whollyWithinLocType() = row->getChar(WHOLLYWITHINLOCTYPE);
    fee->whollyWithinLoc() = getValidLoc(row,
                                         fee->whollyWithinLocType(),
                                         WHOLLYWITHINLOC,
                                         WHOLLYWITHINZONETBLITEMNO);

    fee->sectorPortionOfTvlInd() = row->getChar(SECTORPORTIONOFTVLIND);
    fee->directionality() = row->getChar(DIRECTIONALITY);

    fee->sectorPortionLocType1() = row->getChar(SECTORPORTIONLOCTYPE1);
    fee->sectorPortionLoc1() = getValidLoc(row,
                                           fee->sectorPortionLocType1(),
                                           SECTORPORTIONLOC1,
                                           SECTORPORTIONZONETBLITEMNO1);

    fee->sectorPortionLocType2() = row->getChar(SECTORPORTIONLOCTYPE2);
    fee->sectorPortionLoc2() = getValidLoc(row,
                                           fee->sectorPortionLocType2(),
                                           SECTORPORTIONLOC2,
                                           SECTORPORTIONZONETBLITEMNO2);

    fee->sectorPortionViaLocType() = row->getChar(SECTORPORTIONVIALOCTYPE);
    fee->sectorPortionViaLoc() = getValidLoc(row,
                                             fee->sectorPortionViaLocType(),
                                             SECTORPORTIONVIALOC,
                                             SECTORPORTIONVIAZONETBLITEMNO);

    fee->stopConnectInd() = row->getChar(STOPCONNECTIND);
    fee->stopConnectTime() = row->getInt(STOPCONNECTTIME);
    fee->stopConnectUnit() = row->getChar(STOPCONNECTUNIT);
    fee->connectExemptInd() = row->getChar(CONNECTEXEMPTIND);
    fee->intlDomInd() = row->getChar(INTLDOMIND);
    fee->bookingCode1() = row->getString(BOOKINGCODE1);
    fee->bookingCode2() = row->getString(BOOKINGCODE2);
    fee->bookingCode3() = row->getString(BOOKINGCODE3);
    fee->equipCode() = row->getString(EQUIPCODE);
    fee->fareBasis() = row->getString(FAREBASIS);
    fee->hasSlashInFBC() = fee->fareBasis().find('/') != std::string::npos;
    fee->batchNo() = row->getInt(BATCHNO);
    fee->batchCi() = row->getString(BATCHCI);

    fee->noDec() = row->getInt(NODEC);
    fee->feeAmount() = QUERYCLASS::adjustDecimal(row->getInt(FEEAMOUNT), fee->noDec());
    fee->cur() = row->getString(CUR);

    fee->percentNoDec() = row->getInt(PERCENTNODEC);
    fee->percent() = QUERYCLASS::adjustDecimal(row->getLong(PERCENT) / 100, fee->percentNoDec());

    fee->taxIncludedInd() = row->getChar(TAXINCLUDEDIND);
    fee->feeApplInd() = row->getChar(FEEAPPLIND);
    fee->taxCarrierApplTblItemNo() = row->getInt(TAXCARRIERAPPLTBLITEMNO);
    fee->taxCarrierFltTblItemNo() = row->getInt(TAXCARRIERFLTTBLITEMNO);
    fee->taxTextTblItemNo() = row->getInt(TAXTEXTTBLITEMNO);

    return fee;
  } // mapRowToYQYRFees()
protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}

private:
  static const char* getValidLoc(Row* row,
                                 LocTypeCode locType,
                                 ColumnIndexes locColumn,
                                 ColumnIndexes zoneItemColumn)
  {
    static const LocTypeCode locTypeU = 'U';
    if (locType == locTypeU)
    {
      return row->getString(zoneItemColumn);
    }
    else
    {
      return row->getString(locColumn);
    }
  }
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllYQYRFeesSQLStatement : public QueryGetYQYRFeesSQLStatement<QUERYCLASS>
{

protected:
  void adjustBaseSQL() override { this->Where("%cd <= f.EXPIREDATE and f.VALIDITYIND = 'Y'"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetYQYRFeesHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetYQYRFeesHistoricalSQLStatement : public QueryGetYQYRFeesSQLStatement<QUERYCLASS>
{
protected:
  void adjustBaseSQL() override
  {
    this->From("=YQYRFEES f");
    this->Where("f.CARRIER = %1q"
                " and f.VALIDITYIND = 'Y'");
    this->OrderBy("f.VENDOR,TAXCODE,SUBCODE,SEQNO,f.CREATEDATE");
  }
};
}

