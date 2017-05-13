//----------------------------------------------------------------------------
//  � 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//  the confidential and proprietary product of Sabre Inc. Any unauthorized
//  use, reproduction, or transfer of this software/documentation, in any
//  medium, or incorporation of this software/documentation into any system
//  or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/Queries/QueryGetTaxCodeText.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/TaxCodeTextInfo.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetTaxCodeTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCodeTextSQLStatement() {};

  virtual ~QueryGetTaxCodeTextSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    TAXCODETEXT,
    VERSIONNBR
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,LOCKDATE,EXPIREDATE,LASTMODDATE,"
                  "TAXCODETEXT,VERSIONNBR");

    this->From("=TAXCODETEXT");

    adjustBaseSQL();
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");
    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const tse::TaxCodeTextInfo* mapRowToTaxCodeTextInfo(const Row* row)
  {
    tse::TaxCodeTextInfo* paxInfo = new tse::TaxCodeTextInfo;

    paxInfo->vendor() = row->getString(VENDOR);
    paxInfo->itemNo() = row->getInt(ITEMNO);
    paxInfo->seqNo() = row->getLong(SEQNO);
    paxInfo->createDate() = row->getDate(CREATEDATE);
    paxInfo->lockDate() = row->getDate(LOCKDATE);
    paxInfo->expireDate() = row->getDate(EXPIREDATE);
    paxInfo->lastModificationDate() = row->getDate(LASTMODDATE);
    paxInfo->taxCodeText() = row->getString(TAXCODETEXT);
    paxInfo->versionNbr() = row->getInt(VERSIONNBR);
    return paxInfo;
  }

private:
  virtual void adjustBaseSQL()
  {
    this->Where("VENDOR = %1q "
                "and ITEMNO = %2n "
                "and %cd <= EXPIREDATE "
                "and VALIDITYIND = 'Y'");
  }
}; // class QueryGetTaxCodeTextSQLStatement

template <class QUERYCLASS>
class QueryGetTaxCodeTextHistoricalSQLStatement : public QueryGetTaxCodeTextSQLStatement<QUERYCLASS>
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
}; // class QueryGetTaxCodeTextHistoricalSQLStatement

} // tse

