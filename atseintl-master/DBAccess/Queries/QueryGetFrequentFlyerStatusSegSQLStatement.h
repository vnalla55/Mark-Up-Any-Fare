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
class QueryGetFrequentFlyerStatusSegSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFrequentFlyerStatusSegSQLStatement() {}
  virtual ~QueryGetFrequentFlyerStatusSegSQLStatement() {}

  enum ColumnIndexes
  { CARRIER,
    FFYSTATUSLEVEL,
    PARTNERAIRLINE,
    PARTNERAIRLINEFFYLEVEL,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    NUMBEROFCOLUMNS };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select seg.CARRIER, seg.FFYSTATUSLEVEL, seg.PARTNERAIRLINE, seg.PARTNERAIRLINEFFYLEVEL,"
        "status.CREATEDATE, status.EFFDATE, status.EXPIREDATE, status.DISCDATE");

    std::string from;
    std::vector<std::string> joinFields = {"CARRIER", "FFYSTATUSLEVEL", "CREATEDATE"};
    this->generateJoinString(
        "=FREQFLYERSTATUS", "status", "INNER JOIN", "=FREQFLYERSTATUSSEG", "seg", joinFields, from);

    this->From(from);

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const uint8_t minLevelValue = 1;
  static const uint8_t maxLevelValue = 9;

  static FreqFlyerTierStatusRow* mapRowToStatus(Row* row)
  {
    std::unique_ptr<FreqFlyerTierStatusRow> status(new FreqFlyerTierStatusRow());

    status->_carrier = row->getString(CARRIER);
    status->_level = row->getInt(FFYSTATUSLEVEL);

    TSE_ASSERT(status->_level >= minLevelValue && status->_level <= maxLevelValue);

    status->_partnerCarrier = row->getString(PARTNERAIRLINE);
    status->_partnerLevel = row->getInt(PARTNERAIRLINEFFYLEVEL);

    status->_createDate = row->getDate(CREATEDATE);
    status->_effectiveDate = row->getDate(EFFDATE);
    status->_expireDate = row->getDate(EXPIREDATE);
    status->_discDate = row->getDate(DISCDATE);

    return status.release();
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL()
  {
    this->Where("seg.CARRIER = %1q "
                " and %cd <= status.EXPIREDATE");
  }
};
template <class QUERYCLASS>
class QueryGetFrequentFlyerStatusSegHistoricalSQLStatement
    : public QueryGetFrequentFlyerStatusSegSQLStatement<QUERYCLASS>
{
  virtual void adjustBaseSQL() override
  {
    this->Where("seg.CARRIER = %1q "
                " and %2n <= status.EXPIREDATE"
                " and %3n >= status.CREATEDATE");
  }
};
}
