//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/FreqFlyerStatusSeg.h"

namespace tse
{
template <class QUERYCLASS>
class QueryGetFrequentFlyerStatusSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFrequentFlyerStatusSQLStatement() {}
  virtual ~QueryGetFrequentFlyerStatusSQLStatement() {}

  enum ColumnIndexes
  { CARRIER,
    FFYSTATUSLEVEL,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    STATUSLEVEL,
    MAXPASSENGERSSAMEPNR,
    MAXPASSENGERSDIFFPNR,
    NUMBEROFCOLUMNS };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select CARRIER, FFYSTATUSLEVEL, CREATEDATE, EFFDATE, EXPIREDATE, DISCDATE, "
        "STATUSLEVEL, MAXPASSENGERSSAMEPNR, MAXPASSENGERSDIFFPNR");

    this->From("=FREQFLYERSTATUS");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FreqFlyerStatus* mapRowToStatus(Row* row)
  {
    std::unique_ptr<FreqFlyerStatus> status(new FreqFlyerStatus());

    status->_carrier = row->getString(CARRIER);
    status->_level = row->getInt(FFYSTATUSLEVEL);

    status->_createDate = row->getDate(CREATEDATE);
    status->_effectiveDate = row->getDate(EFFDATE);
    status->_expireDate = row->getDate(EXPIREDATE);
    status->_discDate = row->getDate(DISCDATE);

    status->_statusLevel = row->getString(STATUSLEVEL);
    status->_maxPassengersSamePNR = row->getInt(MAXPASSENGERSSAMEPNR);
    status->_maxPassengersDiffPNR = row->getInt(MAXPASSENGERSDIFFPNR);

    return status.release();
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL()
  {
    this->Where("VALIDITYIND = 'Y' "
                "and CARRIER = %1q "
                "and %cd <= EXPIREDATE");
  }
};
template <class QUERYCLASS>
class QueryGetFrequentFlyerStatusHistoricalSQLStatement
    : public QueryGetFrequentFlyerStatusSQLStatement<QUERYCLASS>
{
  virtual void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y' "
                "and CARRIER = %1q "
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }
};
}
