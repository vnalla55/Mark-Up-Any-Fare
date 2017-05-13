//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetOptionalServicesConcurSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOptionalServicesConcurSQLStatement() {}
  virtual ~QueryGetOptionalServicesConcurSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    SERVICETYPECODE,
    SERVICESUBTYPECODE,
    SVCGROUP,
    SVCSUBGROUP,
    ASSESSEDCARRIER,
    MKGOPERFAREOWNER,
    CONCUR
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, CARRIER, SEQNO, CREATEDATE, EXPIREDATE, EFFDATE, "
                  "       DISCDATE, SERVICETYPECODE, SERVICESUBTYPECODE, SVCGROUP, "
                  "       SVCSUBGROUP, ASSESSEDCARRIER, MKGOPERFAREOWNER, CONCUR");

    this->From("=OPTIONALSVCSCONCUR");

    this->Where("VENDOR = %1q"
                " and CARRIER= %2q"
                " and SERVICETYPECODE= %3q"
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

  static OptionalServicesConcur* mapRowToOptionalServicesConcur(Row* row)
  {
    OptionalServicesConcur* concur = new OptionalServicesConcur;

    concur->vendor() = row->getString(VENDOR);
    concur->carrier() = row->getString(CARRIER);
    concur->seqNo() = row->getLong(SEQNO);
    concur->createDate() = row->getDate(CREATEDATE);

    if (!row->isNull(EXPIREDATE))
      concur->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(EFFDATE))
      concur->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      concur->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(SERVICETYPECODE))
      concur->serviceTypeCode() = row->getString(SERVICETYPECODE);
    if (!row->isNull(SERVICESUBTYPECODE))
      concur->serviceSubTypeCode() = row->getString(SERVICESUBTYPECODE);
    if (!row->isNull(SVCGROUP))
      concur->serviceGroup() = row->getString(SVCGROUP);
    if (!row->isNull(SVCSUBGROUP))
      concur->serviceSubGroup() = row->getString(SVCSUBGROUP);
    if (!row->isNull(ASSESSEDCARRIER))
      concur->accessedCarrier() = row->getString(ASSESSEDCARRIER);
    if (!row->isNull(MKGOPERFAREOWNER))
      concur->mkgOperFareOwner() = row->getChar(MKGOPERFAREOWNER);
    if (!row->isNull(CONCUR))
      concur->concur() = row->getChar(CONCUR);

    return concur;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOptionalServicesConcurHistoricalSQLStatement
    : public QueryGetOptionalServicesConcurSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and CARRIER= %2q"
                " and SERVICETYPECODE= %3q"
                " and VALIDITYIND = 'Y'"
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
    this->OrderBy("SEQNO, CREATEDATE");
  }
};
} // tse
