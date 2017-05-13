//----------------------------------------------------------------------------
//          File:           QueryGetETicketCxrsSQLStatement.h
//          Description:    QueryGetETicketCxrsSQLStatement
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
#include "DBAccess/Queries/QueryGetETicketCxrs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetETicketCxrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetETicketCxrsSQLStatement() {};
  virtual ~QueryGetETicketCxrsSQLStatement() {};

  enum ColumnIndexes
  {
    INTERLINECARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct INTERLINECARRIER");
    this->From("=ETICKETCARRIER");
    this->Where("PSEUDOCITY     = %1q"
                "   and ETICKETCARRIER = %2q");
    //                   "   and PSEUDOIND = 'N'";

    // callback to allow for replacement of SQL clauses by a derived class/template
    if (DataManager::forceSortOrder())
      this->OrderBy("INTERLINECARRIER");
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const CarrierCode mapRowToCarrierCode(Row* row)
  {
    return row->getString(INTERLINECARRIER);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
