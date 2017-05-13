//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetMerchActivationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMerchActivationSQLStatement() {}
  virtual ~QueryGetMerchActivationSQLStatement() {}

  enum ColumnIndexes
  {
    ACTIVATIONID = 0,
    PRODUCTID,
    CARRIER,
    USERAPPLTYPE,
    USERAPPL,
    PSEUDOCITY,
    GROUPCODE,
    SUBGROUPCODE,
    SUBCODE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DISPLAYONLY,
    INCLUDEIND,
    MEMONO
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ACTIVATIONID, PRODUCTID, CARRIER, USERAPPLTYPE, USERAPPL, PSEUDOCITY,"
                  "       GROUPCODE, SUBGROUPCODE, SUBCODE, CREATEDATE, EXPIREDATE,"
                  "       EFFDATE, DISCDATE, DISPLAYONLY, INCLUDEIND, MEMONO");

    this->From("=MERCHACTIVATION");

    this->Where("PRODUCTID= %1n "
                " and CARRIER= %2q "
                " and PSEUDOCITY= %3q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("ACTIVATIONID");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MerchActivationInfo* mapRowToMerchActivationInfo(Row* row)
  {
    MerchActivationInfo* merchActivation = new MerchActivationInfo;

    merchActivation->activationId() = row->getInt(ACTIVATIONID);

    if (!row->isNull(PRODUCTID))
      merchActivation->productId() = row->getInt(PRODUCTID);
    if (!row->isNull(CARRIER))
      merchActivation->carrier() = row->getString(CARRIER);
    if (!row->isNull(USERAPPLTYPE))
      merchActivation->userApplType() = row->getChar(USERAPPLTYPE);
    if (!row->isNull(USERAPPL))
      merchActivation->userAppl() = row->getString(USERAPPL);
    if (!row->isNull(PSEUDOCITY))
      merchActivation->pseudoCity() = row->getString(PSEUDOCITY);
    if (!row->isNull(GROUPCODE))
      merchActivation->groupCode() = row->getString(GROUPCODE);
    if (!row->isNull(SUBGROUPCODE))
      merchActivation->subgroupCode() = row->getString(SUBGROUPCODE);
    if (!row->isNull(SUBCODE))
      merchActivation->subCode() = row->getString(SUBCODE);
    if (!row->isNull(CREATEDATE))
      merchActivation->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      merchActivation->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(EFFDATE))
      merchActivation->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      merchActivation->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(DISPLAYONLY))
      merchActivation->displayOnly() = row->getChar(DISPLAYONLY);
    if (!row->isNull(INCLUDEIND))
      merchActivation->includeInd() = row->getChar(INCLUDEIND);
    if (!row->isNull(MEMONO))
      merchActivation->memoNo() = row->getInt(MEMONO);

    return merchActivation;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMerchActivationHistoricalSQLStatement
    : public QueryGetMerchActivationSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("PRODUCTID= %1n "
                " and CARRIER= %2q "
                " and PSEUDOCITY= %3q "
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
    this->OrderBy("ACTIVATIONID");
  }
};
} // tse
