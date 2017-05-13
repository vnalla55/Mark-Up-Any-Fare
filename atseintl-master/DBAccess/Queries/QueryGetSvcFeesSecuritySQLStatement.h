//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesSequritySQLStatement.h
//          Description:    QueryGetSvcFeesSequritySQLStatement
//          Created:        3/11/2009
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
#include "DBAccess/Queries/QueryGetSvcFeesSecurity.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesSecuritySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesSecuritySQLStatement() {};
  virtual ~QueryGetSvcFeesSecuritySQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    TRAVELAGENCYIND,
    CXRGDSCODE,
    DUTYFUNCTIONCODE,
    LOCTYPE,
    LOC,
    CODETYPE,
    CODE,
    VIEWBOOKTKTIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO,  "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, TRAVELAGENCYIND, "
                  "       CXRGDSCODE, DUTYFUNCTIONCODE, LOCTYPE, LOC,  "
                  "       CODETYPE, CODE , VIEWBOOKTKTIND");

    this->From("=SVCFEESSECURITY");

    this->Where("VENDOR = %1q  "
                " and ITEMNO= %2n "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::SvcFeesSecurityInfo* mapRowToSvcFeesSecurityInfo(Row* row)
  {
    tse::SvcFeesSecurityInfo* sec = new tse::SvcFeesSecurityInfo;

    sec->vendor() = row->getString(VENDOR);
    sec->itemNo() = row->getInt(ITEMNO);
    sec->seqNo() = row->getLong(SEQNO);
    sec->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      sec->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(TRAVELAGENCYIND))
      sec->travelAgencyInd() = row->getChar(TRAVELAGENCYIND);
    if (!row->isNull(CXRGDSCODE))
      sec->carrierGdsCode() = row->getString(CXRGDSCODE);
    if (!row->isNull(DUTYFUNCTIONCODE))
      sec->dutyFunctionCode() = row->getString(DUTYFUNCTIONCODE);
    LocKey* loc1 = &sec->loc();
    if (!row->isNull(LOCTYPE))
      loc1->locType() = row->getChar(LOCTYPE);
    if (!row->isNull(LOC))
      loc1->loc() = row->getString(LOC);
    LocKey* code = &sec->code();
    if (!row->isNull(CODETYPE))
      code->locType() = row->getChar(CODETYPE);
    if (!row->isNull(CODE))
      code->loc() = row->getString(CODE);
    if (!row->isNull(VIEWBOOKTKTIND))
      sec->viewBookTktInd() = row->getChar(VIEWBOOKTKTIND);

    return sec;
  }; // mapRowToSvcFeesSecurityInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesSecuritySQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesSecurityHistoricalSQLStatement
    : public QueryGetSvcFeesSecuritySQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and ITEMNO= %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  };
};
}
