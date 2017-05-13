// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAirlineAllianceContinent.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetAirlineAllianceContinentSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAirlineAllianceContinentSQLStatement() {};
  virtual ~QueryGetAirlineAllianceContinentSQLStatement() {};

  enum ColumnIndexes
  {
    // AIRLINEALLIANCECONTINENT:
    CONTINENTCODE,
    LOCTYPE,
    LOC,

    // AIRLINEALLIANCECODEGROUP:
    GENERICAIRLINECODE,
    GENERICNAME,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select tc.CONTINENTCODE, tc.LOCTYPE, tc.LOC, t.GENERICAIRLINECODE, "
                  "t.GENERICNAME, t.CREATEDATE, "
                  "       t.LOCKDATE, t.EXPIREDATE, t.LASTMODDATE, t.EFFDATE, t.DISCDATE");

    this->From("=AIRLINEALLIANCECONTINENT tc, =AIRLINEALLIANCECODEGROUP t");

    this->Where("tc.GENERICAIRLINECODE = %1q"
                " and t.VENDOR = tc.VENDOR"
                " and t.GENERICAIRLINECODE = tc.GENERICAIRLINECODE"
                " and t.CREATEDATE = tc.CREATEDATE"
                " and %cd <= t.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::AirlineAllianceContinentInfo* mapRowToAirlineAllianceContinentInfo(Row* row)
  {
    tse::AirlineAllianceContinentInfo* airlineAllianceContinentInfo =
        new tse::AirlineAllianceContinentInfo;

    airlineAllianceContinentInfo->continent() =
        getContinentByAllianceContinentCode(row->getInt(CONTINENTCODE));
    airlineAllianceContinentInfo->locType() = row->getChar(LOCTYPE);
    airlineAllianceContinentInfo->locCode() = row->getString(LOC);
    airlineAllianceContinentInfo->genericAllianceCode() = row->getString(GENERICAIRLINECODE);
    airlineAllianceContinentInfo->createDate() = row->getDate(CREATEDATE);
    airlineAllianceContinentInfo->lockDate() = row->getDate(LOCKDATE);
    airlineAllianceContinentInfo->expireDate() = row->getDate(EXPIREDATE);
    airlineAllianceContinentInfo->lastModDate() = row->getDate(LASTMODDATE);
    airlineAllianceContinentInfo->effDate() = row->getDate(EFFDATE);
    airlineAllianceContinentInfo->discDate() = row->getDate(DISCDATE);

    return airlineAllianceContinentInfo;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAirlineAllianceContinentHistoricalSQLStatement
    : public QueryGetAirlineAllianceContinentSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("tc.GENERICAIRLINECODE = %1q"
                " and t.VENDOR = tc.VENDOR"
                " and t.GENERICAIRLINECODE = tc.GENERICAIRLINECODE"
                " and t.CREATEDATE = tc.CREATEDATE"
                " and %2n <= t.EXPIREDATE"
                " and %3n >= t.CREATEDATE");
  };
};
}

