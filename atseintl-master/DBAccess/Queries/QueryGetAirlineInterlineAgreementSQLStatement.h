//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/Queries/QueryGetAirlineInterlineAgreement.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetAirlineInterlineAgreementSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAirlineInterlineAgreementSQLStatement() {}
  virtual ~QueryGetAirlineInterlineAgreementSQLStatement() {}

  enum ColumnIndexes
  {
    NATION = 0,
    GDS,
    VALIDATING_CARRIER,
    PARTICIPATING_CARRIER,
    AGREEMENT_TYPE_CODE,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBER_OF_COLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select aia.nation, "
                                  "aia.gds, "
                                  "aia.validating_carrier, "
                                  "aia.participating_carrier, "
                                  "aia.alnintrlneagreementcd, "
                                  "aia.effdate, "
                                  "aia.discdate, "
                                  "aia.createdate, "
                                  "aia.expiredate";

    const std::string sqlFrom = "v_airlineinterlineagreement aia ";

    const std::string sqlWhere = "aia.nation = %1q "
                                 "and "
                                 "aia.gds = %2q "
                                 "and "
                                 "aia.validating_carrier = %3q ";

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static AirlineInterlineAgreementInfo* mapRowToAirlineInterlineAgreementInfo(Row* row)
  {
    AirlineInterlineAgreementInfo* info = new AirlineInterlineAgreementInfo;

    info->setCountryCode(row->getString(NATION));
    info->setGds(row->getString(GDS));
    info->setValidatingCarrier(row->getString(VALIDATING_CARRIER));
    info->setParticipatingCarrier(row->getString(PARTICIPATING_CARRIER));

    std::string agreement = row->getString(AGREEMENT_TYPE_CODE);
    if (" " == agreement)
      agreement = vcx::AGR_PAPER;
    info->setAgreementTypeCode(agreement);

    info->setEffDate(row->getDate(EFF_DATE));
    info->setDiscDate(row->getDate(DISC_DATE));
    info->setCreateDate(row->getDate(CREATE_DATE));
    info->setExpireDate(row->getDate(EXPIRE_DATE));

    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction 106
  virtual void adjustBaseSQL() { }
};

//-----------------------------------------------------------------------------
// Template used to replace From and Where clauses
//-----------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetAirlineInterlineAgreementHistoricalSQLStatement
    : public QueryGetAirlineInterlineAgreementSQLStatement<QUERYCLASS>
{
public:
  QueryGetAirlineInterlineAgreementHistoricalSQLStatement() {}
  virtual ~QueryGetAirlineInterlineAgreementHistoricalSQLStatement() {}

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere = "aia.nation = %1q "
                                 "and "
                                 "aia.gds = %2q "
                                 "and "
                                 "aia.validating_carrier = %3q "
                                 "and "
                                 "aia.expiredate >= %4n "     // historical start date
                                 "and "
                                 "aia.createdate <= %5n ";    // historical end date
    this->Where(sqlWhere);
  }
};

//-----------------------------------------------------------------------------
// Template to adjust the SQL for QueryGetAllAirlineInterlineAgreements
//-----------------------------------------------------------------------------
template <class QUERYCLASS>
class QueryGetAllAirlineInterlineAgreementSQLStatement
    : public QueryGetAirlineInterlineAgreementSQLStatement<QUERYCLASS>
{
public:
  QueryGetAllAirlineInterlineAgreementSQLStatement() {}
  virtual ~QueryGetAllAirlineInterlineAgreementSQLStatement() {}

private:
  virtual void adjustBaseSQL()
  {
    this->Where("%cd <= aia.expiredate");
    this->OrderBy("NATION,GDS,VALIDATING_CARRIER");
  }
};

}
