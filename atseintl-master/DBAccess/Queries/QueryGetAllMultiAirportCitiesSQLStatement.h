//----------------------------------------------------------------------------
//          File:           QueryGetAllMultiAirportCitiesSQLStatement.h
//          Description:    QueryGetAllMultiAirportCitiesSQLStatement
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
#include "DBAccess/Queries/QueryGetAllMultiAirportCities.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMultiAirportCitySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMultiAirportCitySQLStatement() {};
  virtual ~QueryGetMultiAirportCitySQLStatement() {};

  enum ColumnIndexes
  {
    AIRPORTCODE = 0,
    CITY,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select AIRPORTCODE,CITY");
    this->From("=MULTIAIRPORTCITY");
    this->Where("AIRPORTCODE = %1q");

    if (DataManager::forceSortOrder())
      this->OrderBy("CITY,AIRPORTCODE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MultiAirportCity* mapRowToMultiAirportCity(Row* row)
  {
    tse::MultiAirportCity* multiAirportCity = new tse::MultiAirportCity;

    multiAirportCity->airportCode() = row->getString(AIRPORTCODE);
    multiAirportCity->city() = row->getString(CITY);

    return multiAirportCity;
  } // mapRowToMultiAirportCity()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get QueryGetAllMultiAirportCitiesSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMultiAirportCitiesSQLStatement
    : public QueryGetMultiAirportCitySQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where(""); }
};

////////////////////////////////////////////////////////////////////////
// QueryGetAllMultiAirportsByCity
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMultiAirportsByCitySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllMultiAirportsByCitySQLStatement() {};
  virtual ~QueryGetAllMultiAirportsByCitySQLStatement() {};

  enum ColumnIndexes
  {
    CITY = 0,
    AIRPORTCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CITY,AIRPORTCODE");
    this->From("=MULTIAIRPORTCITY");
    this->OrderBy("CITY,AIRPORTCODE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MultiAirportsByCity* mapRowToMultiAirportsByCity(Row* row)
  {
    tse::MultiAirportsByCity* multiAirportsByCity = new tse::MultiAirportsByCity;

    multiAirportsByCity->city() = row->getString(CITY);
    multiAirportsByCity->airportCode() = row->getString(AIRPORTCODE);

    return multiAirportsByCity;
  } // mapRowToMultiAirportsByCity()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
