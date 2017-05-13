//----------------------------------------------------------------------------
//     © 2013, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#ifndef QUERY_GET_SERVICE_BAGGAGE_SQL_STATEMENT_H
#define QUERY_GET_SERVICE_BAGGAGE_SQL_STATEMENT_H

#include "DBAccess/Queries/QueryGetServiceBaggage.h"
#include "DBAccess/Row.h"
#include "DBAccess/ServiceBaggageInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetServiceBaggageSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetServiceBaggageSQLStatement() {};

  virtual ~QueryGetServiceBaggageSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    SVCTYPE,
    FEEOWNERCXR,
    TAXCODE,
    TAXTYPESUBCODE,
    ATTRSUBGROUP,
    ATTRGROUP,
    APPLTAG,
    VERSIONNBR
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,LOCKDATE,EXPIREDATE,LASTMODDATE,SVCTYPE,"
                  "FEEOWNERCXR,TAXCODE,TAXTYPESUBCODE,ATTRSUBGROUP,ATTRGROUP,APPLTAG,VERSIONNBR");

    this->From("=SERVICEBAGGAGE");

    adjustBaseSQL();
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const tse::ServiceBaggageInfo* mapRowToServiceBaggageInfo(const Row* row)
  {
    tse::ServiceBaggageInfo* serviceInfo = new tse::ServiceBaggageInfo;

    serviceInfo->vendor() = row->getString(VENDOR);
    serviceInfo->itemNo() = row->getInt(ITEMNO);
    serviceInfo->seqNo() = row->getLong(SEQNO);
    serviceInfo->createDate() = row->getDate(CREATEDATE);
    serviceInfo->lockDate() = row->getDate(LOCKDATE);
    serviceInfo->expireDate() = row->getDate(EXPIREDATE);
    serviceInfo->lastModificationDate() = row->getDate(LASTMODDATE);
    serviceInfo->svcType() = row->getChar(SVCTYPE);
    serviceInfo->feeOwnerCxr() = row->getString(FEEOWNERCXR);
    serviceInfo->taxCode() = row->getString(TAXCODE);
    serviceInfo->taxTypeSubCode() = row->getString(TAXTYPESUBCODE);
    serviceInfo->attrSubGroup() = row->getString(ATTRSUBGROUP);
    serviceInfo->attrGroup() = row->getString(ATTRGROUP);
    serviceInfo->applyTag() = row->getChar(APPLTAG);
    serviceInfo->versionNbr() = row->getInt(VERSIONNBR);
    return serviceInfo;
  }

private:
  virtual void adjustBaseSQL()
  {
    this->Where("VENDOR = %1q "
                "and ITEMNO = %2n "
                "and %cd <= EXPIREDATE "
                "and VALIDITYIND = 'Y'");
  }
}; // class QueryGetServiceBaggageSQLStatement

template <class QUERYCLASS>
class QueryGetServiceBaggageHistoricalSQLStatement
    : public QueryGetServiceBaggageSQLStatement<QUERYCLASS>
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
}; // class QueryGetServiceBaggageHistoricalSQLStatement

} // tse

#endif // QUERY_GET_SERVICE_BAGGAGE_SQL_STATEMENT_H
