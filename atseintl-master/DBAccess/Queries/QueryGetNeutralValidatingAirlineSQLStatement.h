//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/Queries/QueryGetNeutralValidatingAirline.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetNeutralValidatingAirlineSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNeutralValidatingAirlineSQLStatement() {}
  virtual ~QueryGetNeutralValidatingAirlineSQLStatement() {}

  enum ColumnIndexes
  {
    COUNTRY_CODE = 0,
    GDS,
    AIRLINE,
    SETTLEMENT_PLAN_TYPE,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBER_OF_COLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select nva.nation, "
                                  "nva.gds, "
                                  "nva.carrier, "
                                  "nva.settlementplan, "
                                  "nva.effdate, "
                                  "nva.discdate, "
                                  "nva.createdate, "
                                  "nva.expiredate ";

    const std::string sqlFrom = "v_neutralvalidatingairline nva ";

    const std::string sqlWhere =
        "nva.nation = %1q "
        "and "
        "nva.gds = %2q "
        "and "
        "nva.settlementplan = %3q ";

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static NeutralValidatingAirlineInfo* mapRowToNeutralValidatingAirlineInfo(Row* row)
  {
    NeutralValidatingAirlineInfo* nva = new NeutralValidatingAirlineInfo;

    nva->setCountryCode(row->getString(COUNTRY_CODE));
    nva->setGds(row->getString(GDS));
    nva->setAirline(row->getString(AIRLINE));
    nva->setSettlementPlanType(row->getString(SETTLEMENT_PLAN_TYPE));
    nva->setEffDate(row->getDate(EFF_DATE));
    nva->setDiscDate(row->getDate(DISC_DATE));
    nva->setCreateDate(row->getDate(CREATE_DATE));
    nva->setExpireDate(row->getDate(EXPIRE_DATE));

    return nva;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() { }
};

//-----------------------------------------------------------------------------
// Template used to replace From and Where clauses
//-----------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetNeutralValidatingAirlineHistoricalSQLStatement
    : public QueryGetNeutralValidatingAirlineSQLStatement<QUERYCLASS>
{
public:
  QueryGetNeutralValidatingAirlineHistoricalSQLStatement() {}
  virtual ~QueryGetNeutralValidatingAirlineHistoricalSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere =
        "nva.nation = %1q "
        "and "
        "nva.gds = %2q "
        "and "
        "nva.settlementplan = %3q "
        "and "
        "nva.expiredate >= %4n "       // 4 = start date
        "and "
        "nva.createdate <= %5n ";      // 5 = end date
    this->Where(sqlWhere);
  }
};

}
