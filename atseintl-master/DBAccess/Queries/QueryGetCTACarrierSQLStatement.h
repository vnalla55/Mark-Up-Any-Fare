//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCTACarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCTACarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCTACarrierSQLStatement() {};
  virtual ~QueryGetCTACarrierSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, CREATEDATE, EXPIREDATE");
    this->From("=CTACARRIER");
    this->Where("CARRIER = %1q "
                "and %cd <= EXPIREDATE");
    this->OrderBy("CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CTACarrier* mapRowToCTACarrier(Row* row)
  {
    CTACarrier* cf = new CTACarrier;
    cf->carrier() = row->getString(CARRIER);
    cf->createDate() = row->getDate(CREATEDATE);
    cf->expireDate() = row->getDate(EXPIREDATE);

    return cf;
  } // mapRowToCTACarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCTACarrierSQLStatement

template <class QUERYCLASS>
class QueryGetCTACarrierHistoricalSQLStatement
    : public QueryGetCTACarrierSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("CARRIER = %1q"); }
}; // class QueryGetCTACarrierHistoricalSQLStatement

} // tse

