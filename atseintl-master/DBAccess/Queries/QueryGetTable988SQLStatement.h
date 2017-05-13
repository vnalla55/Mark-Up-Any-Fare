//----------------------------------------------------------------------------
//          File:           QueryGetTable988SQLStatement.h
//          Description:    QueryGetTable988SQLStatement
//          Created:        11/02/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTable988.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTable988SQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTable988SQLStatement() {};
  virtual ~QueryGetTable988SQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    LASTMODDATE,
    TVLGEOTBLITEMNOFROM,
    TVLGEOTBLITEMNOTO,
    SAMEPOINTTBLITEMNO,
    MINGEOTBLITEMNOFROM,
    MINGEOTBLITEMNOTO,
    FARECXRAPPLTBLITEMNO,
    RISRESTCXRTBLITEMNO,
    SURCHARGEAMT1,
    SURCHARGEAMT2,
    PROCESSINGIND,
    RULETARIFFNO,
    REISSUETOD,
    DEPARTURE,
    MINSTAYTOD,
    SURCHARGENODEC1,
    SURCHARGENODEC2,
    CREATORID,
    CREATORBUSINESSUNIT,
    INHIBIT,
    VALIDITYIND,
    STOPOVERCONNECTIND,
    PORTIONIND,
    TERMINALPOINTIND,
    FIRSTBREAKIND,
    COUPONIND,
    EXTENDIND,
    JOURNEYIND,
    UNUSEDFLIGHTIND,
    RULEIND,
    RULENO,
    FARETYPEIND,
    FARECLASS,
    FARETYPE,
    SAMEIND,
    FAREAMTIND,
    NORMALSPECIALIND,
    OWRT,
    DATEIND,
    FROMADVRESIND,
    TOADVRESIND,
    REVALIDATIONIND,
    PROVISION1,
    PROVISION2,
    PROVISION3,
    PROVISION4,
    PROVISION5,
    PROVISION6,
    PROVISION7,
    PROVISION8,
    PROVISION9,
    PROVISION10,
    PROVISION11,
    PROVISION12,
    PROVISION13,
    PROVISION14,
    PROVISION15,
    PROVISION17,
    PROVISION18,
    PROVISION50,
    BKGCDREVALIND,
    TICKETRESVIND,
    REISSUEPERIOD,
    REISSUEUNIT,
    OPTIONIND,
    DEPARTUREUNIT,
    DEPARTUREIND,
    MINSTAYPERIOD,
    MINSTAYUNIT,
    AUTOTKTIND,
    ELECTRONICTKTIND,
    CARRIERRESTIND,
    AGENCYLOCREST,
    IATAAGENCYNO,
    SURCHARGETYPE,
    SURCHARGEAPPL,
    SURCHARGECUR1,
    SURCHARGECUR2,
    STOPIND,
    ORIGSCHEDFLTPERIOD,
    ORIGSCHEDFLTUNIT,
    OUTBOUNDIND,
    FLIGHTNOIND,
    EXCLUDEPRIVATE,
    HISTORICALTKTTVLIND,
    REISSUETOLOWER,
    TICKETEQUALORHIGHER,
    FARETYPETBLITEMNO,
    SEASONALITYDOWTBLITEMNO,
    EXPNDKEEP
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, LASTMODDATE,"
                  "       TVLGEOTBLITEMNOFROM, TVLGEOTBLITEMNOTO, SAMEPOINTTBLITEMNO,"
                  "       MINGEOTBLITEMNOFROM, MINGEOTBLITEMNOTO, FARECXRAPPLTBLITEMNO,"
                  "       RISRESTCXRTBLITEMNO, SURCHARGEAMT1, SURCHARGEAMT2, PROCESSINGIND,"
                  "       RULETARIFFNO, REISSUETOD, DEPARTURE, MINSTAYTOD, SURCHARGENODEC1,"
                  "       SURCHARGENODEC2, CREATORID, CREATORBUSINESSUNIT, INHIBIT,"
                  "       VALIDITYIND, STOPOVERCONNECTIND, PORTIONIND, TERMINALPOINTIND,"
                  "       FIRSTBREAKIND, COUPONIND, EXTENDIND, JOURNEYIND, UNUSEDFLIGHTIND,"
                  "       RULEIND, RULENO, FARETYPEIND, FARECLASS, FARETYPE, SAMEIND,"
                  "       FAREAMTIND, NORMALSPECIALIND, OWRT, DATEIND, FROMADVRESIND,"
                  "       TOADVRESIND, REVALIDATIONIND, PROVISION1, PROVISION2, PROVISION3,"
                  "       PROVISION4, PROVISION5, PROVISION6, PROVISION7, PROVISION8,"
                  "       PROVISION9, PROVISION10, PROVISION11, PROVISION12, PROVISION13,"
                  "       PROVISION14, PROVISION15, PROVISION17, PROVISION18, PROVISION50,"
                  "       BKGCDREVALIND, TICKETRESVIND, REISSUEPERIOD, REISSUEUNIT,"
                  "       OPTIONIND, DEPARTUREUNIT, DEPARTUREIND, MINSTAYPERIOD, MINSTAYUNIT,"
                  "       AUTOTKTIND, ELECTRONICTKTIND, CARRIERRESTIND, AGENCYLOCREST,"
                  "       IATAAGENCYNO, SURCHARGETYPE, SURCHARGEAPPL, SURCHARGECUR1,"
                  "       SURCHARGECUR2, STOPIND, ORIGSCHEDFLTPERIOD, ORIGSCHEDFLTUNIT,"
                  "       OUTBOUNDIND, FLIGHTNOIND, EXCLUDEPRIVATE, HISTORICALTKTTVLIND,"
                  "       REISSUETOLOWER, TICKETEQUALORHIGHER, FARETYPETBLITEMNO,"
                  "       SEASONALITYDOWTBLITEMNO, EXPNDKEEP");

    this->From("=REISSUE ");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::ReissueSequence* mapRowToReissueSequence(Row* row)
  {
    tse::ReissueSequence* table988 = new tse::ReissueSequence;

    table988->vendor() = row->getString(VENDOR);
    table988->itemNo() = row->getInt(ITEMNO);
    table988->seqNo() = row->getLong(SEQNO);
    table988->createDate() = row->getDate(CREATEDATE);
    table988->expireDate() = row->getDate(EXPIREDATE);
    table988->inhibit() = row->getChar(INHIBIT);
    table988->validityInd() = row->getChar(VALIDITYIND);
    table988->tvlGeoTblItemNoFrom() = row->getInt(TVLGEOTBLITEMNOFROM);
    table988->tvlGeoTblItemNoTo() = row->getInt(TVLGEOTBLITEMNOTO);
    table988->samePointTblItemNo() = row->getInt(SAMEPOINTTBLITEMNO);
    table988->minGeoTblItemNoFrom() = row->getInt(MINGEOTBLITEMNOFROM);
    table988->minGeoTblItemNoTo() = row->getInt(MINGEOTBLITEMNOTO);
    table988->fareCxrApplTblItemNo() = row->getInt(FARECXRAPPLTBLITEMNO);
    table988->risRestCxrTblItemNo() = row->getInt(RISRESTCXRTBLITEMNO);
    table988->surchargeNoDec1() = row->getInt(SURCHARGENODEC1);
    table988->surchargeNoDec2() = row->getInt(SURCHARGENODEC2);
    table988->surchargeAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT1), table988->surchargeNoDec1());
    table988->surchargeAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(SURCHARGEAMT2), table988->surchargeNoDec2());
    table988->processingInd() = row->getInt(PROCESSINGIND);
    table988->ruleTariffNo() = row->getInt(RULETARIFFNO);
    table988->reissueTOD() = row->getInt(REISSUETOD);
    table988->departure() = row->getInt(DEPARTURE);
    table988->minStayTOD() = row->getInt(MINSTAYTOD);
    table988->creatorId() = row->getString(CREATORID);
    table988->creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);
    table988->stopoverConnectInd() = row->getChar(STOPOVERCONNECTIND);
    table988->portionInd() = row->getChar(PORTIONIND);
    table988->terminalPointInd() = row->getChar(TERMINALPOINTIND);
    table988->firstBreakInd() = row->getChar(FIRSTBREAKIND);
    table988->couponInd() = row->getChar(COUPONIND);
    table988->extendInd() = row->getChar(EXTENDIND);
    table988->journeyInd() = row->getChar(JOURNEYIND);
    table988->unusedFlightInd() = row->getChar(UNUSEDFLIGHTIND);
    table988->ruleInd() = row->getChar(RULEIND);
    table988->ruleNo() = row->getString(RULENO);
    table988->fareTypeInd() = row->getChar(FARETYPEIND);
    table988->fareClass() = row->getString(FARECLASS);
    table988->fareType() = row->getString(FARETYPE);
    table988->sameInd() = row->getChar(SAMEIND);
    table988->fareAmtInd() = row->getChar(FAREAMTIND);
    table988->normalspecialInd() = row->getChar(NORMALSPECIALIND);
    table988->owrt() = row->getChar(OWRT);
    table988->dateInd() = row->getChar(DATEIND);
    table988->fromAdvResInd() = row->getChar(FROMADVRESIND);
    table988->toAdvResInd() = row->getChar(TOADVRESIND);
    table988->revalidationInd() = row->getChar(REVALIDATIONIND);
    table988->provision1() = row->getChar(PROVISION1);
    table988->provision2() = row->getChar(PROVISION2);
    table988->provision3() = row->getChar(PROVISION3);
    table988->provision4() = row->getChar(PROVISION4);
    table988->provision5() = row->getChar(PROVISION5);
    table988->provision6() = row->getChar(PROVISION6);
    table988->provision7() = row->getChar(PROVISION7);
    table988->provision8() = row->getChar(PROVISION8);
    table988->provision9() = row->getChar(PROVISION9);
    table988->provision10() = row->getChar(PROVISION10);
    table988->provision11() = row->getChar(PROVISION11);
    table988->provision12() = row->getChar(PROVISION12);
    table988->provision13() = row->getChar(PROVISION13);
    table988->provision14() = row->getChar(PROVISION14);
    table988->provision15() = row->getChar(PROVISION15);
    table988->provision17() = row->getChar(PROVISION17);
    table988->provision18() = row->getChar(PROVISION18);
    table988->provision50() = row->getChar(PROVISION50);
    table988->bkgCdRevalInd() = row->getChar(BKGCDREVALIND);
    table988->ticketResvInd() = row->getChar(TICKETRESVIND);
    table988->reissuePeriod() = row->getString(REISSUEPERIOD);
    table988->reissueUnit() = row->getString(REISSUEUNIT);
    table988->optionInd() = row->getChar(OPTIONIND);
    table988->departureUnit() = row->getChar(DEPARTUREUNIT);
    table988->departureInd() = row->getChar(DEPARTUREIND);
    table988->minStayPeriod() = row->getString(MINSTAYPERIOD);
    table988->minStayUnit() = row->getChar(MINSTAYUNIT);
    table988->autoTktInd() = row->getChar(AUTOTKTIND);
    table988->electronicTktInd() = row->getChar(ELECTRONICTKTIND);
    table988->carrierRestInd() = row->getChar(CARRIERRESTIND);
    table988->agencyLocRest() = row->getChar(AGENCYLOCREST);
    table988->iataAgencyNo() = row->getString(IATAAGENCYNO);
    table988->surchargeType() = row->getChar(SURCHARGETYPE);
    table988->surchargeAppl() = row->getChar(SURCHARGEAPPL);
    table988->surchargeCur1() = row->getString(SURCHARGECUR1);
    table988->surchargeCur2() = row->getString(SURCHARGECUR2);
    table988->stopInd() = row->getChar(STOPIND);
    table988->origSchedFltPeriod() = row->getString(ORIGSCHEDFLTPERIOD);
    table988->origSchedFltUnit() = row->getChar(ORIGSCHEDFLTUNIT);
    table988->outboundInd() = row->getChar(OUTBOUNDIND);
    table988->flightNoInd() = row->getChar(FLIGHTNOIND);
    table988->excludePrivate() = row->getChar(EXCLUDEPRIVATE);
    table988->historicalTktTvlInd() = row->getChar(HISTORICALTKTTVLIND);
    table988->reissueToLower() = row->getChar(REISSUETOLOWER);
    table988->ticketEqualOrHigher() = row->getChar(TICKETEQUALORHIGHER);
    if (!row->isNull(FARETYPETBLITEMNO))
      table988->fareTypeTblItemNo() = row->getInt(FARETYPETBLITEMNO);
    if (!row->isNull(SEASONALITYDOWTBLITEMNO))
      table988->seasonalityDOWTblItemNo() = row->getInt(SEASONALITYDOWTBLITEMNO);
    table988->expndKeep() = row->getChar(EXPNDKEEP);

    return table988;
  } // mapRowToReissueSequence()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTable988Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTable988HistoricalSQLStatement : public QueryGetTable988SQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetTable988HistoricalSQLStatement
} // tse
