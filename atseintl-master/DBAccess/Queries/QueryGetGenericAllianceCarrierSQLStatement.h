// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
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

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGenericAllianceCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetGenericAllianceCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGenericAllianceCarrierSQLStatement() {};
  virtual ~QueryGetGenericAllianceCarrierSQLStatement() {};

  enum ColumnIndexes
  { CARRIER = 0,
    GENERICAIRLINECODE,
    GENERICNAME,
    CREATEDATE,
    LOCKDATE,
    EXPIREDATE,
    LASTMODDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select tc.CARRIER, t.GENERICAIRLINECODE, t.GENERICNAME, t.CREATEDATE, "
                  "       t.LOCKDATE, t.EXPIREDATE, t.LASTMODDATE, t.EFFDATE, t.DISCDATE");

    this->From("=AIRLINEALLIANCECARRIER tc, =AIRLINEALLIANCECODEGROUP t");

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

  static tse::AirlineAllianceCarrierInfo* mapRowToGenericAllianceCarrierInfo(Row* row)
  {
    tse::AirlineAllianceCarrierInfo* airlineAllianceCarrierInfo =
        new tse::AirlineAllianceCarrierInfo;

    airlineAllianceCarrierInfo->carrier() = row->getString(CARRIER);
    airlineAllianceCarrierInfo->genericAllianceCode() = row->getString(GENERICAIRLINECODE);
    airlineAllianceCarrierInfo->genericName() = row->getString(GENERICNAME);
    airlineAllianceCarrierInfo->createDate() = row->getDate(CREATEDATE);
    airlineAllianceCarrierInfo->lockDate() = row->getDate(LOCKDATE);
    airlineAllianceCarrierInfo->expireDate() = row->getDate(EXPIREDATE);
    airlineAllianceCarrierInfo->lastModDate() = row->getDate(LASTMODDATE);
    airlineAllianceCarrierInfo->effDate() = row->getDate(EFFDATE);
    airlineAllianceCarrierInfo->discDate() = row->getDate(DISCDATE);

    return airlineAllianceCarrierInfo;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGenericAllianceCarrierHistoricalSQLStatement
    : public QueryGetGenericAllianceCarrierSQLStatement<QUERYCLASS>
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

