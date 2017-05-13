//----------------------------------------------------------------------------
//          File:           QueryGetAllBrandedCarriersSQLStatement.h
//          Description:    QueryGetAllBrandedCarriersSQLStatement
//          Created:        02/28/2008
//          Authors:        Mauricio Dantas
//
//          Updates:
//
//     (C) 2008, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllBrandedCarriers.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetAllBrandedCarriersSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllBrandedCarriersSQLStatement() {};
  virtual ~QueryGetAllBrandedCarriersSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    BFVERSIONCODE,
    CARRIERSTATUS,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, BFVERSIONCODE");
    this->From("=BRANDEDCARRIER ");

    this->Where("CARRIERSTATUS='A'");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BrandedCarrier* mapRowToBrandedCarrier(Row* row)
  {
    tse::BrandedCarrier* pBranded = new tse::BrandedCarrier();
    pBranded->carrier() = row->getString(CARRIER);
    pBranded->bfvCode() = row->getString(BFVERSIONCODE);
    return pBranded;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
