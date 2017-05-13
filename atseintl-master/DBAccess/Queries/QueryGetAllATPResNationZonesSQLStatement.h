//----------------------------------------------------------------------------
//          File:           QueryGetAllATPResNationZonesSQLStatement.h
//          Description:    QueryGetAllATPResNationZonesSQLStatement
//          Created:        10/26/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllATPResNationZones.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAllATPResNationZonesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllATPResNationZonesSQLStatement() {};
  virtual ~QueryGetAllATPResNationZonesSQLStatement() {};

  enum ColumnIndexes
  {
    NATION = 0,
    ZONENO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select NATION,ZONENO");
    this->From("=ATPRESZONENATIONXREF");
    this->OrderBy("NATION,ZONENO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static ATPResNationZones* mapRowToATPResNationZones(Row* row, ATPResNationZones* nZonesPrev)
  {
    NationCode nation = row->getString(NATION);

    ATPResNationZones* nZones;

    // If Parent hasn't changed, add to Children (applSegs)
    if (nZonesPrev != nullptr && nZonesPrev->nation() == nation)
    { // Just add to Prev
      nZones = nZonesPrev;
    }
    else
    { // Time for a new Parent
      nZones = new ATPResNationZones;
      nZones->nation() = nation;
    } // New Parent

    // Add new Zone to Nation & return
    if (!row->isNull(ZONENO))
    {
      nZones->zones().push_back(row->getString(ZONENO));
    }

    return nZones;
  } // mapRowToATPResNationZones()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
