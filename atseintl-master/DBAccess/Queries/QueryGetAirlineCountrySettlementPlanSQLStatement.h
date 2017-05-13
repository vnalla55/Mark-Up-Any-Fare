//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/Queries/QueryGetAirlineCountrySettlementPlan.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetAirlineCountrySettlementPlanSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAirlineCountrySettlementPlanSQLStatement() {}
  virtual ~QueryGetAirlineCountrySettlementPlanSQLStatement() {}

  enum ColumnIndexes
  {
    COUNTRY_CODE = 0,
    GDS,
    AIRLINE,
    PREFERRED_TICKING_METHOD,
    REQUIRED_TICKING_METHOD,
    SETTLEMENT_PLAN_TYPE,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBER_OF_COLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select acsp.nation, "
                                  "acsp.gds, "
                                  "acsp.carrier, "
                                  "acsp.preftktissuingmethdcd, "
                                  "acsp.rqrtktissuingmethdcd, "
                                  "acsp.settlementplan, "
                                  "acsp.effdate, "
                                  "acsp.discdate, "
                                  "acsp.createdate, "
                                  "acsp.expiredate ";

    const std::string sqlFrom = "v_airlinecountrysettlementplan acsp ";

    const std::string sqlWhere =
        "acsp.nation = %1q "
        "and "
        "acsp.gds = %2q "
        "and "
        "acsp.carrier = %3q "
        "and "
        "acsp.settlementplan = %4q ";

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL(); // callback to allow for replacement of SQL clauses by a derived class
    this->ConstructSQL();
    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static AirlineCountrySettlementPlanInfo* mapRowToAirlineCountrySettlementPlan(Row* row)
  {
    AirlineCountrySettlementPlanInfo* acsp = new AirlineCountrySettlementPlanInfo;

    acsp->setCountryCode(row->getString(COUNTRY_CODE));
    acsp->setGds(row->getString(GDS));
    acsp->setAirline(row->getString(AIRLINE));
    acsp->setPreferredTicketingMethod(row->getChar(PREFERRED_TICKING_METHOD));
    acsp->setRequiredTicketingMethod(row->getChar(REQUIRED_TICKING_METHOD));
    acsp->setSettlementPlanType(row->getString(SETTLEMENT_PLAN_TYPE));
    acsp->setEffDate(row->getDate(EFF_DATE));
    acsp->setDiscDate(row->getDate(DISC_DATE));
    acsp->setCreateDate(row->getDate(CREATE_DATE));
    acsp->setExpireDate(row->getDate(EXPIRE_DATE));

    return acsp;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() { }
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From and Where clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAirlineCountrySettlementPlanHistoricalSQLStatement
    : public QueryGetAirlineCountrySettlementPlanSQLStatement<QUERYCLASS>
{
public:
  QueryGetAirlineCountrySettlementPlanHistoricalSQLStatement() {}
  virtual ~QueryGetAirlineCountrySettlementPlanHistoricalSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere =
        "acsp.nation = %1q "
        "and "
        "acsp.gds = %2q "
        "and "
        "acsp.carrier = %3q "
        "and "
        "acsp.settlementplan = %4q "
        "and "
        "acsp.createdate <= %6n"
        "and "
        "acsp.expiredate >= %5n ";

    this->Where(sqlWhere);
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetAirlineCountrySettlementPlanHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllAirlineCountrySettlementPlan
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllAirlineCountrySettlementPlanSQLStatement
    : public QueryGetAirlineCountrySettlementPlanSQLStatement<QUERYCLASS>
{
public:
  QueryGetAllAirlineCountrySettlementPlanSQLStatement() {}
  virtual ~QueryGetAllAirlineCountrySettlementPlanSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= acsp.expiredate");
    this->OrderBy("NATION,GDS,CARRIER,SETTLEMENTPLAN");
  }
}; // QueryGetAllAirlineCountrySettlementPlanSQLStatement

}
