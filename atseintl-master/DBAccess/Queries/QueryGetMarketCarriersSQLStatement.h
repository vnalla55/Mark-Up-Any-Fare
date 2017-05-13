//----------------------------------------------------------------------------
//          File:           QueryGetMarketCarriersSQLStatement.h
//          Description:    QueryGetMarketCarriersSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetMarketCarriers.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMarketCarriersSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMarketCarriersSQLStatement() {};
  virtual ~QueryGetMarketCarriersSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER");
    this->From("=MULTICARRIERMKT");
    this->Where("MARKET1 = %1q"
                " and MARKET2 = %2q");
    // callback to allow for replacement of SQL clauses by a derived class/template
    if (DataManager::forceSortOrder())
      this->OrderBy("Carrier");

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MarketCarrier* mapRowToMarketCarrier(Row* row)
  {
    tse::MarketCarrier* mc = new tse::MarketCarrier;
    mc->_carrier = row->getString(CARRIER);
    mc->_inhibit = 'N';

    return mc;
  } // mapRowToMarketCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMarketCarriersSQLStatement

} // tse
