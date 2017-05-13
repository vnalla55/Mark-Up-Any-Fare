//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/PassengerAirlineInfo.h"
#include "DBAccess/Queries/QueryGetPassengerAirline.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPassengerAirlineSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPassengerAirlineSQLStatement() {}
  virtual ~QueryGetPassengerAirlineSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER_CODE = 0,
    CARRIER_NAME,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBER_OF_COLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select pa.carrier, "
                                  "pa.carriername, "
                                  "pa.effdate, "
                                  "pa.discdate, "
                                  "pa.createdate, "
                                  "pa.expiredate ";

    const std::string sqlFrom = "passengerairline pa ";

    const std::string sqlWhere =
        "pa.carrier = %1q ";

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static PassengerAirlineInfo* mapRowToPassengerAirlineInfo(Row* row)
  {
    PassengerAirlineInfo* info = new PassengerAirlineInfo;

    info->setAirlineCode(row->getString(CARRIER_CODE));
    info->setAirlineName(row->getString(CARRIER_NAME));
    info->setEffDate(row->getDate(EFF_DATE));
    info->setDiscDate(row->getDate(DISC_DATE));
    info->setCreateDate(row->getDate(CREATE_DATE));
    info->setExpireDate(row->getDate(EXPIRE_DATE));

    return info;
  }
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() { }
};

//-----------------------------------------------------------------------------
// Template used to replace From and Where clauses
//-----------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetPassengerAirlineHistoricalSQLStatement
    : public QueryGetPassengerAirlineSQLStatement<QUERYCLASS>
{
public:
  QueryGetPassengerAirlineHistoricalSQLStatement() {}
  virtual ~QueryGetPassengerAirlineHistoricalSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere =
        "pa.carrier = %1q "
        "and "
        "pa.expiredate >= %2n "      // Historical start date
        "and "
        "pa.createdate <= %3n ";      // Historical end date
    this->Where(sqlWhere);
  }
};

}
