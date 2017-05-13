//----------------------------------------------------------------------------
//          File:           QueryGetNegFareSecuritySQLStatement.h
//          Description:    QueryGetNegFareSecuritySQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetNegFareSecurity.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNegFareSecuritySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNegFareSecuritySQLStatement() {};
  virtual ~QueryGetNegFareSecuritySQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    CREATORID,
    CREATORBUSINESSUNIT,
    APPLIND,
    TVLAGENCYIND,
    CARRIERCRS,
    DUTYFUNCTIONCODE,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    LOCALETYPE,
    AGENCYPCC,
    IATATVLAGENCYNO,
    DEPARTMENTID,
    CRSCARRIERDEPARTMENT,
    LINEIATANO,
    ERSPNO,
    UPDATEIND,
    REDISTRIBUTEIND,
    SELLIND,
    TICKETIND,
    CHANGESONLYIND,
    SECONDARYSELLERID,
    LASTMODDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,CREATORID,CREATORBUSINESSUNIT,"
                  "       APPLIND,TVLAGENCYIND,CARRIERCRS,DUTYFUNCTIONCODE,LOC1TYPE,LOC1,LOC2TYPE,"
                  "       LOC2,LOCALETYPE,AGENCYPCC,IATATVLAGENCYNO,DEPARTMENTID,"
                  "       CRSCARRIERDEPARTMENT,LINEIATANO,ERSPNO,UPDATEIND,REDISTRIBUTEIND,"
                  "       SELLIND,TICKETIND,CHANGESONLYIND,SECONDARYSELLERID,LASTMODDATE");
    this->From("=NEGFARESECURITY");

    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and VALIDITYIND = 'Y'"
                "    and (   (CARRIERCRS not like '1%')"
                "         or (CARRIERCRS = '1S')"
                "         or (CARRIERCRS = '1J')"
                "         or (CARRIERCRS = '1F')"
                "         or (CARRIERCRS = '1B')  )"
                "    and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,SEQNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NegFareSecurityInfo* mapRowToNegFareSecurityInfo(Row* row)
  {
    tse::NegFareSecurityInfo* negFareSec = new tse::NegFareSecurityInfo;

    negFareSec->vendor() = row->getString(VENDOR);
    negFareSec->itemNo() = row->getInt(ITEMNO);
    negFareSec->seqNo() = row->getLong(SEQNO);
    negFareSec->createDate() = row->getDate(CREATEDATE);
    negFareSec->expireDate() = row->getDate(EXPIREDATE);
    negFareSec->applInd() = row->getChar(APPLIND);
    negFareSec->tvlAgencyInd() = row->getChar(TVLAGENCYIND);
    negFareSec->carrierCrs() = row->getString(CARRIERCRS);
    negFareSec->dutyFunctionCode() = row->getString(DUTYFUNCTIONCODE);

    LocKey* loc = &negFareSec->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);

    loc = &negFareSec->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);

    negFareSec->localeType() = row->getChar(LOCALETYPE);
    negFareSec->agencyPCC() = row->getString(AGENCYPCC);
    negFareSec->iataTvlAgencyNo() = row->getString(IATATVLAGENCYNO);
    negFareSec->departmentId() = row->getInt(DEPARTMENTID);
    negFareSec->crsCarrierDepartment() = row->getString(CRSCARRIERDEPARTMENT);
    negFareSec->lineIATANo() = row->getString(LINEIATANO);
    negFareSec->erspno() = row->getInt(ERSPNO);
    negFareSec->updateInd() = row->getChar(UPDATEIND);
    negFareSec->redistributeInd() = row->getChar(REDISTRIBUTEIND);
    negFareSec->sellInd() = row->getChar(SELLIND);
    negFareSec->ticketInd() = row->getChar(TICKETIND);
    negFareSec->changesOnlyInd() = row->getChar(CHANGESONLYIND);
    negFareSec->secondarySellerId() = row->getLong(SECONDARYSELLERID);

    return negFareSec;
  } // mapRowToNegFareSecurityInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetNegFareSecuritySQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetNegFareSecurityHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNegFareSecurityHistoricalSQLStatement
    : public QueryGetNegFareSecuritySQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,CREATORID,CREATORBUSINESSUNIT,"
        "       APPLIND,TVLAGENCYIND,CARRIERCRS,DUTYFUNCTIONCODE,LOC1TYPE,LOC1,LOC2TYPE,"
        "       LOC2,LOCALETYPE,AGENCYPCC,IATATVLAGENCYNO,DEPARTMENTID,"
        "       CRSCARRIERDEPARTMENT,LINEIATANO,ERSPNO,UPDATEIND,REDISTRIBUTEIND,"
        "       SELLIND,TICKETIND,CHANGESONLYIND,SECONDARYSELLERID,LASTMODDATE");
    partialStatement.From("=NEGFARESECURITYH");

    partialStatement.Where("VENDOR = %1q "
                           " and ITEMNO = %2n "
                           " and VALIDITYIND = 'Y'"
                           " and (   (CARRIERCRS not like '1%')"
                           "      or (CARRIERCRS = '1S')"
                           "      or (CARRIERCRS = '1J')"
                           "      or (CARRIERCRS = '1F')"
                           "      or (CARRIERCRS = '1B')  )"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE"
                           ")");

    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,CREATORID,CREATORBUSINESSUNIT,"
        "       APPLIND,TVLAGENCYIND,CARRIERCRS,DUTYFUNCTIONCODE,LOC1TYPE,LOC1,LOC2TYPE,"
        "       LOC2,LOCALETYPE,AGENCYPCC,IATATVLAGENCYNO,DEPARTMENTID,"
        "       CRSCARRIERDEPARTMENT,LINEIATANO,ERSPNO,UPDATEIND,REDISTRIBUTEIND,"
        "       SELLIND,TICKETIND,CHANGESONLYIND,SECONDARYSELLERID,LASTMODDATE");
    partialStatement.From("=NEGFARESECURITY");

    partialStatement.Where("VENDOR = %5q "
                           " and ITEMNO = %6n "
                           " and VALIDITYIND = 'Y'"
                           " and (   (CARRIERCRS not like '1%')"
                           "      or (CARRIERCRS = '1S')"
                           "      or (CARRIERCRS = '1J')"
                           "      or (CARRIERCRS = '1F')"
                           "      or (CARRIERCRS = '1B')  )"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE"
                           ")");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetNegFareSecurityHistoricalSQLStatement
} // tse
