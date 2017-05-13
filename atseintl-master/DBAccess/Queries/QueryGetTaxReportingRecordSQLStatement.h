//----------------------------------------------------------------------------
//     � 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/Queries/QueryGetTaxReportingRecord.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/TaxReportingRecordInfo.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetTaxReportingRecordSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxReportingRecordSQLStatement() {};

  virtual ~QueryGetTaxReportingRecordSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    NATION,
    TAXCARRIER,
    TAXCODE,
    TAXTYPE,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    TAXCHARGE,
    ACCOUNTABLEDOCTAXTAG,
    REPORTINGTEXTTBLNO,
    INTERLINEABLETAXTAG,
    REFUNDABLETAXTAG,
    COMMISSIONABLETAXTAG,
    VATIND,
    TAXNAME,
    TAXTEXTITEMNO,
    TAXAPPLICABLETOTBLNO,
    TAXRATETEXTTBLNO,
    TAXEXEMPTIONTEXTTBLNO,
    TAXCOLLECTNREMITTBLNO,
    TAXINGAUTHORITYTEXTTBLNO,
    TAXCOMMENTSTEXTTBLNO,
    TAXSPLINSTRUCTIONSTBLNO,
    VERSIONNBR
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR,NATION,TAXCARRIER,TAXCODE,TAXTYPE,CREATEDATE,EFFDATE,DISCDATE,LOCKDATE,"
        "EXPIREDATE,LASTMODDATE,TAXCHARGE,ACCOUNTABLEDOCTAXTAG,REPORTINGTEXTTBLNO,"
        "INTERLINEABLETAXTAG,REFUNDABLETAXTAG,COMMISSIONABLETAXTAG,VATIND,TAXNAME,"
        "TAXTEXTITEMNO,TAXAPPLICABLETOTBLNO,TAXRATETEXTTBLNO,TAXEXEMPTIONTEXTTBLNO,"
        "TAXCOLLECTNREMITTBLNO,TAXINGAUTHORITYTEXTTBLNO,TAXCOMMENTSTEXTTBLNO,"
        "TAXSPLINSTRUCTIONSTBLNO,VERSIONNBR");

    this->From("=TAXREPORTINGRECORD");

    adjustBaseSQL();
    this->OrderBy("VENDOR,NATION,TAXCODE,TAXTYPE,TAXCARRIER,CREATEDATE");

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const tse::TaxReportingRecordInfo* mapRowToTaxReportingRecordInfo(const Row* row)
  {
    tse::TaxReportingRecordInfo* serviceInfo = new tse::TaxReportingRecordInfo;
    serviceInfo->vendor() = row->getString(VENDOR);
    serviceInfo->nationCode() = row->getString(NATION);
    serviceInfo->taxCarrier() = row->getString(TAXCARRIER);
    serviceInfo->taxCode() = row->getString(TAXCODE);
    serviceInfo->taxType() = row->getString(TAXTYPE);
    serviceInfo->createDate() = row->getDate(CREATEDATE);
    serviceInfo->effDate() = row->getDate(EFFDATE);
    serviceInfo->discDate() = row->getDate(DISCDATE);
    serviceInfo->lockDate() = row->getDate(LOCKDATE);
    serviceInfo->expireDate() = row->getDate(EXPIREDATE);
    serviceInfo->lastModificationDate() = row->getDate(LASTMODDATE);
    serviceInfo->taxCharge() = row->getChar(TAXCHARGE);
    serviceInfo->accountableDocTaxTag() = row->getChar(ACCOUNTABLEDOCTAXTAG);
    serviceInfo->reportingTextTblNo() = row->getInt(REPORTINGTEXTTBLNO);
    serviceInfo->interlineAbleTaxTag() = row->getChar(INTERLINEABLETAXTAG);
    serviceInfo->refundableTaxTag() = row->getChar(REFUNDABLETAXTAG);
    serviceInfo->commisionableTaxTag() = row->getChar(COMMISSIONABLETAXTAG);
    serviceInfo->vatInd() = row->getChar(VATIND);
    serviceInfo->taxName() = row->getString(TAXNAME);
    serviceInfo->taxTextItemNo() = row->getInt(TAXTEXTITEMNO);
    serviceInfo->taxApplicableToTblNo() = row->getInt(TAXAPPLICABLETOTBLNO);
    serviceInfo->taxRateTextTblNo() = row->getInt(TAXRATETEXTTBLNO);
    serviceInfo->taxExemptionTextTblNo() = row->getInt(TAXEXEMPTIONTEXTTBLNO);
    serviceInfo->taxCollectNrEmmitTblNo() = row->getInt(TAXCOLLECTNREMITTBLNO);
    serviceInfo->taxingAuthorityTextTblNo() = row->getInt(TAXINGAUTHORITYTEXTTBLNO);
    serviceInfo->taxCommentsTextTblNo() = row->getInt(TAXCOMMENTSTEXTTBLNO);
    serviceInfo->taxSplInstructionsTblNo() = row->getInt(TAXSPLINSTRUCTIONSTBLNO);
    serviceInfo->versionNbr() = row->getInt(VERSIONNBR);
    return serviceInfo;
  }

private:
  virtual void adjustBaseSQL()
  {
    this->Where("VENDOR = %1q "
                "and NATION = %2n "
                "and TAXCARRIER = %3n "
                "and TAXCODE = %4n "
                "and TAXTYPE = %5n "
                "and %cd <= EXPIREDATE "
                "and VALIDITYIND = 'Y'");
  }
}; // class QueryGetTaxReportingRecordSQLStatement

template <class QUERYCLASS>
class QueryGetTaxReportingRecordHistoricalSQLStatement
    : public QueryGetTaxReportingRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q "
                "and NATION = %2n "
                "and TAXCARRIER = %3n "
                "and TAXCODE = %4n "
                "and TAXTYPE = %5n "
                "and VALIDITYIND = 'Y' "
                "and %6n <= EXPIREDATE "
                "and %7n >= CREATEDATE");
  }
}; // class QueryGetTaxReportingRecordHistoricalSQLStatement

template <class QUERYCLASS>
class QueryGetTaxReportingRecordByCodeSQLStatementBase
  : public QueryGetTaxReportingRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("TAXCODE = %1n "
                "and VALIDITYIND = 'Y' ");
  }
};

template <class QUERYCLASS>
class QueryGetTaxReportingRecordByCodeSQLStatement
  : public QueryGetTaxReportingRecordByCodeSQLStatementBase<QUERYCLASS>
{
}; // class QueryGetTaxReportingRecordByCodeSQLStatement

template <class QUERYCLASS>
class QueryGetTaxReportingRecordByCodeHistoricalSQLStatement
  : public QueryGetTaxReportingRecordByCodeSQLStatementBase<QUERYCLASS>
{
}; // class QueryGetTaxReportingRecordByCodeHistoricalSQLStatement

} // tse

