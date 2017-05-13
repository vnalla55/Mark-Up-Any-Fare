//----------------------------------------------------------------------------
//          File:           QueryGetAllGenericTaxCodeSQLStatement.h
//          Description:    QueryGetAllGenericTaxCodeSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllGenericTaxCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAllGenericTaxCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllGenericTaxCodeSQLStatement() {};
  virtual ~QueryGetAllGenericTaxCodeSQLStatement() {};

  enum ColumnIndexes
  {
    TAXCODE = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    VENDOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,VENDOR");
    this->From("=GENERICTAXCODES ");
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("TAXCODE,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::GenericTaxCode* mapRowToGenericTaxCode(Row* row)
  {
    tse::GenericTaxCode* taxCode = new tse::GenericTaxCode;

    taxCode->taxCode() = row->getString(TAXCODE);
    taxCode->createDate() = row->getDate(CREATEDATE);
    taxCode->expireDate() = row->getDate(EXPIREDATE);
    taxCode->effDate() = row->getDate(EFFDATE);
    taxCode->discDate() = row->getDate(DISCDATE);
    taxCode->vendor() = row->getString(VENDOR);

    return taxCode;
  } // mapRowToGenericTaxCode()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetGenericTaxCodeHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllGenericTaxCodeHistoricalSQLStatement
    : public QueryGetAllGenericTaxCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%1n <= EXPIREDATE "
                "and (%2n >= CREATEDATE"
                " or %3n >= EFFDATE)");
  }
};

} // tse
