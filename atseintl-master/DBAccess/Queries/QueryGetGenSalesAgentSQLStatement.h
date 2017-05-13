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
#include "DBAccess/Queries/QueryGetGenSalesAgent.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetGenSalesAgentSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGenSalesAgentSQLStatement() {};
  virtual ~QueryGetGenSalesAgentSQLStatement() {};

  enum ColumnIndexes
  {
    // GENRLSLAGRCNTRY:
    NATION = 0,
    GDS,
    SETTLEMENTPLAN,
    VALIDATING_CARRIER,
    NONPARTICIPATING_CARRIER,
    EFF_DATE,
    DISC_DATE,
    CREATE_DATE,
    EXPIRE_DATE,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    const std::string sqlSelect = "select gsa.nation, "
                                  "gsa.gds, "
                                  "gsa.settlementplan, "
                                  "gsa.validating_carrier, "
                                  "gsa.nonparticipating_carrier, "
                                  "gsa.effdate, "
                                  "gsa.discdate, "
                                  "gsa.createdate, "
                                  "gsa.expiredate ";
    const std::string sqlFrom = "v_genrlslagrcntry gsa ";

    const std::string sqlWhere = "gsa.gds = %1q "
                                 "and "
                                 "gsa.nation = %2q "
                                 "and "
                                 "gsa.settlementplan = %3q "
                                 "and "
                                 "gsa.nonparticipating_carrier = %4q ";

    this->Command(sqlSelect);
    this->From(sqlFrom);
    this->Where(sqlWhere);
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::GenSalesAgentInfo* mapRowToGenSalesAgentInfo(Row* row)
  {
    tse::GenSalesAgentInfo* gsaInfo = new tse::GenSalesAgentInfo();
    if (gsaInfo)
    {
      gsaInfo->setCountryCode(row->getString(NATION));
      gsaInfo->setGDSCode(row->getString(GDS));
      gsaInfo->setSettlementPlanCode(row->getString(SETTLEMENTPLAN));
      gsaInfo->setEffDate(row->getDate(EFF_DATE));
      gsaInfo->setDiscDate(row->getDate(DISC_DATE));
      gsaInfo->setCreateDate(row->getDate(CREATE_DATE));
      gsaInfo->setExpireDate(row->getDate(EXPIRE_DATE));
      gsaInfo->setCxrCode(row->getString(VALIDATING_CARRIER));
      gsaInfo->setNonParticipatingCxr(row->getString(NONPARTICIPATING_CARRIER));
    }
    return gsaInfo;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

//-----------------------------------------------------------------------------
// Template used to replace From and Where clauses
//-----------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetGenSalesAgentHistoricalSQLStatement
    : public QueryGetGenSalesAgentSQLStatement<QUERYCLASS>
{
public:
  QueryGetGenSalesAgentHistoricalSQLStatement() {};
  virtual ~QueryGetGenSalesAgentHistoricalSQLStatement() {};

private:
  void adjustBaseSQL() override
  {
    const std::string sqlWhere = "gsa.gds = %1q "
                                 "and "
                                 "gsa.nation = %2q "
                                 "and "
                                 "gsa.settlementplan = %3q "
                                 "and "
                                 "gsa.nonparticipating_carrier = %4q "
                                 "and "
                                 "gsa.createdate <= %6n "
                                 "and "
                                 "gsa.expiredate >= %5n";
    this->Where(sqlWhere);
  }
};

//-----------------------------------------------------------------------------
// Template to adjust the SQL for QueryGetAllGenSalesAgents
//-----------------------------------------------------------------------------
template <class QUERYCLASS>
class QueryGetAllGenSalesAgentSQLStatement
    : public QueryGetGenSalesAgentSQLStatement<QUERYCLASS>
{
public:
  QueryGetAllGenSalesAgentSQLStatement() {};
  virtual ~QueryGetAllGenSalesAgentSQLStatement() {};

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= gsa.expiredate");
    this->OrderBy("GDS,NATION,SETTLEMENTPLAN,NONPARTICIPATING_CARRIER");
  }
};

} // namespace tse

