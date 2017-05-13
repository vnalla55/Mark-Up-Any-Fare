//----------------------------------------------------------------------------
//          File:           QueryGetVoluntaryChangesInfoSQLStatement.h
//          Description:    QueryGetVoluntaryChangesInfoSQLStatement
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
#include "DBAccess/Queries/QueryGetVoluntaryChangesInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetVoluntaryChangesInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetVoluntaryChangesInfoSQLStatement() {}
  virtual ~QueryGetVoluntaryChangesInfoSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    WAIVERTBLITEMNO,
    REISSUETBLITEMNO,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    ADVRESLASTTOD,
    PSGOCCURRENCEFIRST,
    PSGOCCURRENCELAST,
    INHIBIT,
    UNAVAILTAG,
    PSGTYPE,
    TKTVALIDITYIND,
    DEPARTUREIND,
    PRICEABLEUNITIND,
    FARECOMPONENTIND,
    ADVRESFROM,
    ADVRESTO,
    ADVRESPERIOD,
    ADVRESUNIT,
    TKTTIMELIMIT,
    CHANGEIND,
    PENALTYAMT1,
    PENALTYAMT2,
    NODEC1,
    NODEC2,
    PERCENT,
    PERCENTNODEC,
    CUR1,
    CUR2,
    HIGHLOWIND,
    MINIMUMAMT,
    MINNODEC,
    MINCUR,
    JOURNEYIND,
    FEEAPPL,
    TYPEOFSVCIND,
    TKTTRANSACTIONIND,
    DISCOUNTTAG1,
    DISCOUNTTAG2,
    DISCOUNTTAG3,
    DISCOUNTTAG4,
    RESIDUALIND,
    FORMOFREFUND,
    ENDORSEMENT,
    SAMEAIRPORT,
    DOMESTICINTLCOMB,
    CARRIERAPPLTBLITEMNO,
    RESPENHIERARCHY,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,WAIVERTBLITEMNO,REISSUETBLITEMNO,TEXTTBLITEMNO,"
        " OVERRIDEDATETBLITEMNO,ADVRESLASTTOD,PSGOCCURRENCEFIRST,PSGOCCURRENCELAST,INHIBIT,"
        " UNAVAILTAG,PSGTYPE,TKTVALIDITYIND,DEPARTUREIND,PRICEABLEUNITIND,FARECOMPONENTIND,"
        " ADVRESFROM,ADVRESTO,ADVRESPERIOD,ADVRESUNIT,TKTTIMELIMIT,CHANGEIND,PENALTYAMT1,"
        " PENALTYAMT2,NODEC1,NODEC2,PERCENT,PERCENTNODEC,cur1,cur2,HIGHLOWIND,MINIMUMAMT,"
        " MINNODEC,MINCUR,JOURNEYIND,FEEAPPL,TYPEOFSVCIND,TKTTRANSACTIONIND,DISCOUNTTAG1,"
        " DISCOUNTTAG2,DISCOUNTTAG3,DISCOUNTTAG4,RESIDUALIND,FORMOFREFUND,ENDORSEMENT,"
        " SAMEAIRPORT,DOMESTICINTLCOMB,CARRIERAPPLTBLITEMNO,RESPENHIERARCHY");
    this->From(" =VOLUNTARYCHANGES");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE, HASHCODE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::VoluntaryChangesInfo* mapRowToVoluntaryChangesInfo(Row* row)
  { // Load up Parent Determinant Fields
    VoluntaryChangesInfo* vci = new tse::VoluntaryChangesInfo;
    vci->vendor() = row->getString(VENDOR);
    vci->itemNo() = row->getInt(ITEMNO);
    vci->createDate() = row->getDate(CREATEDATE);
    vci->expireDate() = row->getDate(EXPIREDATE);
    vci->waiverTblItemNo() = row->getInt(WAIVERTBLITEMNO);
    vci->reissueTblItemNo() = row->getInt(REISSUETBLITEMNO);
    vci->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    vci->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    vci->advResLastTOD() = row->getInt(ADVRESLASTTOD);
    vci->psgOccurrenceFirst() = row->getInt(PSGOCCURRENCEFIRST);
    vci->psgOccurrenceLast() = row->getInt(PSGOCCURRENCELAST);
    vci->inhibit() = row->getChar(INHIBIT);
    vci->unavailTag() = row->getChar(UNAVAILTAG);
    vci->psgType() = row->getString(PSGTYPE);
    vci->tktValidityInd() = row->getChar(TKTVALIDITYIND);
    vci->departureInd() = row->getChar(DEPARTUREIND);
    vci->priceableUnitInd() = row->getChar(PRICEABLEUNITIND);
    vci->fareComponentInd() = row->getChar(FARECOMPONENTIND);
    vci->advResFrom() = row->getChar(ADVRESFROM);
    vci->advResTo() = row->getChar(ADVRESTO);
    vci->advResPeriod() = row->getString(ADVRESPERIOD);
    vci->advResUnit() = row->getString(ADVRESUNIT);
    vci->tktTimeLimit() = row->getChar(TKTTIMELIMIT);
    vci->changeInd() = row->getChar(CHANGEIND);
    vci->noDec1() = row->getInt(NODEC1);
    vci->noDec2() = row->getInt(NODEC2);
    vci->penaltyAmt1() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT1), vci->noDec1());
    vci->penaltyAmt2() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT2), vci->noDec2());
    vci->percent() = QUERYCLASS::adjustDecimal(row->getInt(PERCENT), row->getInt(PERCENTNODEC));
    vci->cur1() = row->getString(CUR1);
    vci->cur2() = row->getString(CUR2);
    vci->highLowInd() = row->getChar(HIGHLOWIND);
    vci->minNoDec() = row->getInt(MINNODEC);
    vci->minAmt() = QUERYCLASS::adjustDecimal(row->getInt(MINIMUMAMT), vci->minNoDec());
    vci->minCur() = row->getString(MINCUR);
    vci->journeyInd() = row->getChar(JOURNEYIND);
    vci->feeAppl() = row->getChar(FEEAPPL);
    vci->typeOfSvcInd() = row->getChar(TYPEOFSVCIND);
    vci->tktTransactionInd() = row->getChar(TKTTRANSACTIONIND);
    vci->discountTag1() = row->getChar(DISCOUNTTAG1);
    vci->discountTag2() = row->getChar(DISCOUNTTAG2);
    vci->discountTag3() = row->getChar(DISCOUNTTAG3);
    vci->discountTag4() = row->getChar(DISCOUNTTAG4);
    vci->residualInd() = row->getChar(RESIDUALIND);
    vci->formOfRefund() = row->getChar(FORMOFREFUND);
    vci->endorsement() = row->getChar(ENDORSEMENT);
    vci->sameAirport() = row->getChar(SAMEAIRPORT);
    vci->domesticIntlComb() = row->getChar(DOMESTICINTLCOMB);
    vci->carrierApplTblItemNo() = row->getInt(CARRIERAPPLTBLITEMNO);

    if (!row->isNull(RESPENHIERARCHY))
      vci->residualHierarchy() = row->getChar(RESPENHIERARCHY);

    return vci;
  } // mapRowToVoluntaryChangesInfo()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetVoluntaryChangesInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetVoluntaryChangesInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetVoluntaryChangesInfoHistoricalSQLStatement
    : public QueryGetVoluntaryChangesInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetVoluntaryChangesInfoHistoricalSQLStatement
}
