//----------------------------------------------------------------------------
//     © 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#ifndef QUERY_GET_PAX_TYPE_CODE_SQL_STATEMENT_H
#define QUERY_GET_PAX_TYPE_CODE_SQL_STATEMENT_H

#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/Queries/QueryGetPaxTypeCode.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetPaxTypeCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPaxTypeCodeSQLStatement() {};

  virtual ~QueryGetPaxTypeCodeSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    APPLTAG,
    PSGRTYPE,
    PSGRMINAGE,
    PSGRMAXAGE,
    PSGRSTATUS,
    LOCTYPE,
    LOC,
    PTCMATCHIND,
    VERSIONNBR
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,LOCKDATE,EXPIREDATE,LASTMODDATE,APPLTAG,"
                  "PSGRTYPE,PSGRMINAGE,PSGRMAXAGE,PSGRSTATUS,LOCTYPE,LOC,PTCMATCHIND,VERSIONNBR");

    this->From("=PASSENGERTYPECODE");

    adjustBaseSQL();

    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const tse::PaxTypeCodeInfo* mapRowToPaxTypeCodeInfo(const Row* row)
  {
    tse::PaxTypeCodeInfo* paxInfo = new tse::PaxTypeCodeInfo;

    paxInfo->vendor() = row->getString(VENDOR);
    paxInfo->itemNo() = row->getInt(ITEMNO);
    paxInfo->seqNo() = row->getLong(SEQNO);
    paxInfo->createDate() = row->getDate(CREATEDATE);
    paxInfo->lockDate() = row->getDate(LOCKDATE);
    paxInfo->expireDate() = row->getDate(EXPIREDATE);
    paxInfo->lastModificationDate() = row->getDate(LASTMODDATE);
    paxInfo->applyTag() = row->getChar(APPLTAG);
    paxInfo->psgrType() = row->getString(PSGRTYPE);
    paxInfo->paxMinAge() = row->getInt(PSGRMINAGE);
    paxInfo->paxMaxAge() = row->getInt(PSGRMAXAGE);
    paxInfo->paxStatus() = row->getChar(PSGRSTATUS);
    paxInfo->loc().locType() = row->getChar(LOCTYPE);
    paxInfo->loc().loc() = row->getString(LOC);
    paxInfo->ptcMatchIndicator() = row->getChar(PTCMATCHIND);
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
}; // class QueryGetPaxTypeCodeSQLStatement

template <class QUERYCLASS>
class QueryGetPaxTypeCodeHistoricalSQLStatement : public QueryGetPaxTypeCodeSQLStatement<QUERYCLASS>
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
}; // class QueryGetPaxTypeCodeHistoricalSQLStatement

} // tse

#endif // QUERY_GET_PAX_TYPE_CODE_SQL_STATEMENT_H
