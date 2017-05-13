//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/Queries/QueryGetEmdInterlineAgreement.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetEmdInterlineAgreementSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    NATION = 0,
    GDS,
    VALIDATING_CARRIER,
    PARTICIPATING_CARRIER,
    CREATE_DATE,
    EFF_DATE,
    EXPIRE_DATE,
    DISC_DATE,
    NUMBER_OF_COLUMNS
  };

  QueryGetEmdInterlineAgreementSQLStatement()
  {}

  virtual
  ~QueryGetEmdInterlineAgreementSQLStatement()
  {}

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select eia.nation as nation, "
                                  "eia.globaldistributionsystemcd as gds, "
                                  "pav.carrier as validating_carrier, "
                                  "pap.carrier as participating_carrier, "
                                  "eia.createdate as createdate, "
                                  "eia.effdate as effdate, "
                                  "eia.expiredate as expiredate, "
                                  "eia.discdate as discdate ";

    const std::string sqlFrom = "emdinterlineagreement eia, "
                                "passengerairline pav, "
                                "passengerairline pap ";

    const std::string sqlWhere = "eia.validatingairlinesurogtid=pav.passengerairlineid "
                                 "and "
                                 "eia.participatingalnsurogtid=pap.passengerairlineid "
                                 "and "
                                 "eia.nation = %1q "
                                 "and "
                                 "eia.globaldistributionsystemcd = %2q "
                                 "and "
                                 "pav.carrier = %3q ";
    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static
  EmdInterlineAgreementInfo* mapRowToEmdInterlineAgreementInfo(Row* row)
  {
    EmdInterlineAgreementInfo* info = new EmdInterlineAgreementInfo;

    info->setCountryCode(row->getString(NATION));
    info->setGds(row->getString(GDS));
    info->setValidatingCarrier(row->getString(VALIDATING_CARRIER));
    info->setParticipatingCarrier(row->getString(PARTICIPATING_CARRIER));

    info->setCreateDate(row->getDate(CREATE_DATE));
    info->setEffDate(row->getDate(EFF_DATE));
    info->setExpireDate(row->getDate(EXPIRE_DATE));
    info->setDiscDate(row->getDate(DISC_DATE));

    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction 106
  virtual
  void adjustBaseSQL()
  {}
};


//-----------------------------------------------------------------------------
// Template to adjust the SQL for QueryGetAllEmdInterlineAgreements
//-----------------------------------------------------------------------------
template <class QUERYCLASS>
class QueryGetAllEmdInterlineAgreementSQLStatement
    : public QueryGetEmdInterlineAgreementSQLStatement<QUERYCLASS>
{
public:
  QueryGetAllEmdInterlineAgreementSQLStatement() {}
  virtual ~QueryGetAllEmdInterlineAgreementSQLStatement() {}

private:
  virtual void adjustBaseSQL()
  {
    this->Where("%cd <= eia.expiredate");
    this->OrderBy("NATION,GDS,VALIDATING_CARRIER");
  }
};


} // tse

