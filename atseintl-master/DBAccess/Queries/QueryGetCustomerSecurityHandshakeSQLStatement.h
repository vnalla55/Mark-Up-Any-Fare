#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCustomerSecurityHandshake.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetCustomerSecurityHandshakeSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    SECURITYSOURCEPCC
    , CREATEDATE
    , PRODUCTCD
    , SECURITYTARGETPCC
    , EXPIREDATE
    , EFFDATETIME
    , DISCDATETIME
    , HOMEPCC
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select SECURITYSOURCEPCC, CREATEDATE, PRODUCTCD, SECURITYTARGETPCC,"
            "EXPIREDATE, EFFDATETIME, DISCDATETIME, HOMEPCC");
    this->From("=CUSTOMERSECURITYHANDSHAKE");
    this->Where("SECURITYSOURCEPCC = %1q"
                " and PRODUCTCD = %2q"
                " and %3n <= DISCDATETIME"
                " and %4n <= EXPIREDATE"
                " and EFFDATETIME <= DISCDATETIME");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CustomerSecurityHandshakeInfo* mapRow(Row* row, CustomerSecurityHandshakeInfo*)
  {
    CustomerSecurityHandshakeInfo* info(new CustomerSecurityHandshakeInfo);
    info->securitySourcePCC() = row->getString(SECURITYSOURCEPCC);
    if (!row->isNull(CREATEDATE))
    {
      info->createDate() = row->getDate(CREATEDATE);
    }
    info->productCode() = row->getString(PRODUCTCD);
    info->securityTargetPCC() = row->getString(SECURITYTARGETPCC);
    if (!row->isNull(EXPIREDATE))
    {
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    if (!row->isNull(EFFDATETIME))
    {
      info->effDate() = row->getDate(EFFDATETIME);
    }
    if (!row->isNull(DISCDATETIME))
    {
      info->discDate() = row->getDate(DISCDATETIME);
    }
    info->homePCC() = row->getString(HOMEPCC);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCustomerSecurityHandshakeHistoricalSQLStatement
    : public QueryGetCustomerSecurityHandshakeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("SECURITYSOURCEPCC = %1q"
                " and PRODUCTCD = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllCustomerSecurityHandshakeSQLStatement
    : public QueryGetCustomerSecurityHandshakeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");
  }
};

} // tse

