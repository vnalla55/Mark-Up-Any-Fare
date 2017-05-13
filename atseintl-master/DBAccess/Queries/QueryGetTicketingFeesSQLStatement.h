//----------------------------------------------------------------------------
//          File:           QueryGetTicketingFeesSQLStatement.h
//          Description:    QueryGetTicketingFeesSQLStatement
//          Created:        2/26/2009
// Authors:
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTicketingFees.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetTicketingFeesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTicketingFeesSQLStatement() {};
  virtual ~QueryGetTicketingFeesSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    SERVICETYPECODE,
    SERVICESUBTYPECODE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    PUBLICPRIVATEIND,
    EFFDATE,
    DISCDATE,
    TICKETEFFDATE,
    TICKETDISCDATE,
    PSGTYPE,
    SVCFEESACCOUNTCODETBLITEMNO,
    SVCFEESTKTDESIGTBLITEMNO,
    SVCFEESSECURITYTBLITEMNO,
    JOURNEYIND,
    LOC1TYPE,
    LOC1,
    LOC1ZONETBLITEMNO,
    LOC2TYPE,
    LOC2,
    LOC2ZONETBLITEMNO,
    WHOLLYWITHINLOCTYPE,
    WHOLLYWITHINLOC,
    WHOLLYWITHINZONETBLITEMNO,
    VIALOCTYPE,
    VIALOC,
    VIAZONETBLITEMNO,
    FAREINDICATOR,
    PRIMARYFARECARRIER,
    FAREBASIS,
    FOPBINNUMBER,
    REFUNDREISSUEIND,
    COMMISSIONIND,
    INTERLINEIND,
    NOCHARGEIND,
    NODEC,
    FEEAMOUNT,
    CUR,
    PERCENT,
    PERCENTNODEC,
    TAXINCLIND,
    MAXFEEAMOUNT,
    MAXFEECUR,
    MAXFEENODEC,
    COMMERCIALNAME,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR, CARRIER, SERVICETYPECODE, SERVICESUBTYPECODE, SEQNO,  "
        "       CREATEDATE,EXPIREDATE,VALIDITYIND,PUBLICPRIVATEIND, "
        "       EFFDATE, DISCDATE, TICKETEFFDATE, TICKETDISCDATE, "
        "       PSGTYPE,SVCFEESACCOUNTCODETBLITEMNO, SVCFEESTKTDESIGTBLITEMNO, "
        "       SVCFEESSECURITYTBLITEMNO,"
        "       JOURNEYIND, LOC1TYPE,LOC1,LOC1ZONETBLITEMNO,LOC2TYPE,LOC2,LOC2ZONETBLITEMNO,"
        "       WHOLLYWITHINLOCTYPE,WHOLLYWITHINLOC,WHOLLYWITHINZONETBLITEMNO,VIALOCTYPE,VIALOC,"
        "       VIAZONETBLITEMNO,FAREINDICATOR,PRIMARYFARECARRIER,FAREBASIS,FOPBINNUMBER,"
        "       REFUNDREISSUEIND,COMMISSIONIND,INTERLINEIND,NOCHARGEIND,"
        "       NODEC,FEEAMOUNT,CUR,PERCENT, PERCENTNODEC,"
        "       TAXINCLIND,MAXFEEAMOUNT,MAXFEECUR,MAXFEENODEC, COMMERCIALNAME");

    this->From("=TICKETINGFEES");

    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");
    this->OrderBy("SERVICETYPECODE, SERVICESUBTYPECODE, SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::TicketingFeesInfo* mapRowToTicketingFeesInfo(Row* row)
  {
    tse::TicketingFeesInfo* tktF = new tse::TicketingFeesInfo;

    tktF->vendor() = row->getString(VENDOR);
    tktF->carrier() = row->getString(CARRIER);
    tktF->serviceTypeCode() = row->getString(SERVICETYPECODE);
    tktF->serviceSubTypeCode() = row->getString(SERVICESUBTYPECODE);
    tktF->seqNo() = row->getLong(SEQNO);
    tktF->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      tktF->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(PUBLICPRIVATEIND))
      tktF->publicPrivateInd() = row->getChar(PUBLICPRIVATEIND);
    if (!row->isNull(EFFDATE))
      tktF->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      tktF->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(TICKETEFFDATE))
      tktF->ticketEffDate() = row->getDate(TICKETEFFDATE);
    if (!row->isNull(TICKETDISCDATE))
      tktF->ticketDiscDate() = row->getDate(TICKETDISCDATE);
    if (!row->isNull(PSGTYPE))
      tktF->paxType() = row->getString(PSGTYPE);
    if (!row->isNull(SVCFEESACCOUNTCODETBLITEMNO))
      tktF->svcFeesAccCodeTblItemNo() = row->getInt(SVCFEESACCOUNTCODETBLITEMNO);
    if (!row->isNull(SVCFEESTKTDESIGTBLITEMNO))
      tktF->svcFeesTktDsgnTblItemNo() = row->getInt(SVCFEESTKTDESIGTBLITEMNO);
    if (!row->isNull(SVCFEESSECURITYTBLITEMNO))
      tktF->svcFeesSecurityTblItemNo() = row->getInt(SVCFEESSECURITYTBLITEMNO);
    if (!row->isNull(JOURNEYIND))
      tktF->journeyInd() = row->getChar(JOURNEYIND);
    LocKey* loc1 = &tktF->loc1();
    if (!row->isNull(LOC1TYPE))
      loc1->locType() = row->getChar(LOC1TYPE);
    if (!row->isNull(LOC1))
      loc1->loc() = row->getString(LOC1);
    if (!row->isNull(LOC1ZONETBLITEMNO))
      tktF->loc1ZoneItemNo() = row->getString(LOC1ZONETBLITEMNO);
    LocKey* loc2 = &tktF->loc2();
    if (!row->isNull(LOC2TYPE))
      loc2->locType() = row->getChar(LOC2TYPE);
    if (!row->isNull(LOC2))
      loc2->loc() = row->getString(LOC2);
    if (!row->isNull(LOC2ZONETBLITEMNO))
      tktF->loc2ZoneItemNo() = row->getString(LOC2ZONETBLITEMNO);
    LocKey* locW = &tktF->locWhlWithin();
    if (!row->isNull(WHOLLYWITHINLOCTYPE))
      locW->locType() = row->getChar(WHOLLYWITHINLOCTYPE);
    if (!row->isNull(WHOLLYWITHINLOC))
      locW->loc() = row->getString(WHOLLYWITHINLOC);
    if (!row->isNull(WHOLLYWITHINZONETBLITEMNO))
      tktF->locZoneWhlWithinItemNo() = row->getString(WHOLLYWITHINZONETBLITEMNO);
    LocKey* locV = &tktF->locVia();
    if (!row->isNull(VIALOCTYPE))
      locV->locType() = row->getChar(VIALOCTYPE);
    if (!row->isNull(VIALOC))
      locV->loc() = row->getString(VIALOC);
    if (!row->isNull(VIAZONETBLITEMNO))
      tktF->locZoneViaItemNo() = row->getString(VIAZONETBLITEMNO);
    if (!row->isNull(FAREINDICATOR))
      tktF->fareInd() = row->getChar(FAREINDICATOR);
    if (!row->isNull(PRIMARYFARECARRIER))
      tktF->primaryFareCarrier() = row->getString(PRIMARYFARECARRIER);
    if (!row->isNull(FAREBASIS))
      tktF->fareBasis() = row->getString(FAREBASIS);
    if (!row->isNull(FOPBINNUMBER))
      tktF->fopBinNumber() = row->getString(FOPBINNUMBER);
    if (!row->isNull(REFUNDREISSUEIND))
      tktF->refundReissue() = row->getChar(REFUNDREISSUEIND);
    if (!row->isNull(COMMISSIONIND))
      tktF->commission() = row->getChar(COMMISSIONIND);
    if (!row->isNull(INTERLINEIND))
      tktF->interline() = row->getChar(INTERLINEIND);
    if (!row->isNull(NOCHARGEIND))
      tktF->noCharge() = row->getChar(NOCHARGEIND);
    if (!row->isNull(NODEC))
      tktF->noDec() = row->getInt(NODEC);
    if (!row->isNull(FEEAMOUNT))
      tktF->feeAmount() = QUERYCLASS::adjustDecimal(row->getInt(FEEAMOUNT), tktF->noDec());
    if (!row->isNull(CUR))
      tktF->cur() = row->getString(CUR);
    if (!row->isNull(PERCENTNODEC))
      tktF->feePercentNoDec() = row->getInt(PERCENTNODEC);
    if (!row->isNull(PERCENT))
      tktF->feePercent() = QUERYCLASS::adjustDecimal(row->getInt(PERCENT), tktF->feePercentNoDec());
    if (!row->isNull(TAXINCLIND))
      tktF->taxInclude() = row->getChar(TAXINCLIND);
    if (!row->isNull(MAXFEECUR))
      tktF->maxFeeCur() = row->getString(MAXFEECUR);
    if (!row->isNull(MAXFEENODEC))
      tktF->maxFeeNoDec() = row->getInt(MAXFEENODEC);
    if (!row->isNull(MAXFEEAMOUNT))
      tktF->maxFeeAmount() =
          QUERYCLASS::adjustDecimal(row->getInt(MAXFEEAMOUNT), tktF->maxFeeNoDec());
    tktF->commercialName() = row->getString(COMMERCIALNAME);

    return tktF;
  }; // mapRowToTicketingFeesInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetTicketingFeesSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTicketingFeesHistoricalSQLStatement
    : public QueryGetTicketingFeesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and CARRIER= %2q "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("SERVICETYPECODE, SERVICESUBTYPECODE, SEQNO, CREATEDATE");
  };
};
}
