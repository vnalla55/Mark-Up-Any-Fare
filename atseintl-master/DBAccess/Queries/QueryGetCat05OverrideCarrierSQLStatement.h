
/*----------------------------------------------------------------------------
          File:           QueryGetCat05OverrideCarrierSQLStatement.h
          Description:    QueryGetCat05OverrideCarrierSQLStatement
          Created:        04/22/2013
          Authors:        Sashi Reddy
     (C) 2013, Sabre Inc.  All rights reserved.  This software/documentation is
     the confidential and proprietary product of Sabre Inc. Any unauthorized
     use, reproduction, or transfer of this software/documentation, in any
     medium, or incorporation of this software/documentation into any system
     or publication, is strictly prohibited
-----------------------------------------------------------------------*/

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCat05OverrideCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <typename QUERYCLASS>
class QueryGetCat05OverrideCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCat05OverrideCarrierSQLStatement() {}
  virtual ~QueryGetCat05OverrideCarrierSQLStatement() {}
  enum ColumnIndexes
  {
    PSEUDOCITY = 0,
    CARRIER,
    NUMBEROFCOLUMNS
  };
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PSEUDOCITY, CARRIER");
    this->From("=CAT05OVERRIDECARRIER");
    this->Where("PSEUDOCITY = %1q");

    // if ( DataManager::forceSortOrder() )
    //      this->OrderBy( "PSEUDOCITY" ) ;

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);
    return *this;
  }

  static void mapRowToCat05OverrideCarrier(Row* row, Cat05OverrideCarrier* cat05Cxr)
  {

    // both pcc and carrier are primarykey so cannot be null
    PseudoCityCode pcc(row->getString(PSEUDOCITY));
    // 2 is to read  two chars from carrier code
    CarrierCode carrier(row->getString(CARRIER), 2);
    cat05Cxr->pseudoCity() = pcc;
    cat05Cxr->carrierList().push_back(carrier);
    return;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCat05OverrideCarrierSQLStatement

} // tse
