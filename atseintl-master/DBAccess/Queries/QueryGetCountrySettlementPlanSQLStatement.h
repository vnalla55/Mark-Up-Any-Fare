//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Queries/QueryGetCountrySettlementPlan.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetCountrySettlementPlanSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCountrySettlementPlanSQLStatement() {}
  virtual ~QueryGetCountrySettlementPlanSQLStatement() {}

  enum ColumnIndexes
  {
    NATION = 0,
    SETTLEMENTPLAN,
    PREFERRED_TICKING_METHOD,
    REQUIRED_TICKING_METHOD,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlFrom = "v_countrysettlementplan csp ";

    const std::string sqlSelect("select csp.nation, "
                                "csp.settlementplan, "
                                "csp.preftktissuingmethdcd, "
                                "csp.rqrtktissuingmethdcd, "
                                "csp.effdate, "
                                "csp.discdate, "
                                "csp.createdate, "
                                "csp.expiredate ");

    const std::string sqlWhere("csp.nation = %1q");

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CountrySettlementPlanInfo* mapRowToCountrySettlementPlanInfo(Row* row)
  {
    CountrySettlementPlanInfo* info = new CountrySettlementPlanInfo;

    info->setCountryCode(row->getString(NATION));
    info->setSettlementPlanTypeCode(row->getString(SETTLEMENTPLAN));
    info->setPreferredTicketingMethod(row->getChar(PREFERRED_TICKING_METHOD));
    info->setRequiredTicketingMethod(row->getChar(REQUIRED_TICKING_METHOD));
    info->setEffDate(row->getDate(EFF_DATE));
    info->setDiscDate(row->getDate(DISC_DATE));
    info->setCreateDate(row->getDate(CREATE_DATE));
    info->setExpireDate(row->getDate(EXPIRE_DATE));

    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction 106 105
  virtual void adjustBaseSQL() { }
};

//-----------------------------------------------------------------------------
// Historical
//-----------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetCountrySettlementPlanHistoricalSQLStatement
  : public QueryGetCountrySettlementPlanSQLStatement<QUERYCLASS>
{
public:
  QueryGetCountrySettlementPlanHistoricalSQLStatement() {}
  virtual ~QueryGetCountrySettlementPlanHistoricalSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere("csp.nation = %1q "
                               "and "
                               "csp.expiredate >= %2n "    // Historical start date
                               "and "
                               "csp.createdate <= %3n ");  // Historical end date
    this->Where(sqlWhere);
  }
};

//-----------------------------------------------------------------------------
// Get all
//-----------------------------------------------------------------------------
template <class QUERYCLASS>
class QueryGetAllCountrySettlementPlanSQLStatement
    : public QueryGetCountrySettlementPlanSQLStatement<QUERYCLASS>
{
public:
  QueryGetAllCountrySettlementPlanSQLStatement() {}
  virtual ~QueryGetAllCountrySettlementPlanSQLStatement() {}
private:
  virtual void adjustBaseSQL()
  {
    this->Where("%cd <= csp.expiredate");
  }
};

}
