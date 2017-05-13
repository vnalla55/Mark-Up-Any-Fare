//----------------------------------------------------------------------------
//     2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivation.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetServiceFeesCxrActivationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetServiceFeesCxrActivationSQLStatement() {};
  virtual ~QueryGetServiceFeesCxrActivationSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE");
    this->From(" =SERVICEFEESCXRACTIVATION");
    this->Where(" CARRIER= %1q  "
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CREATEDATE, CARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();
    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::ServiceFeesCxrActivation* mapRowToServiceFeesCxrActivation(Row* row)
  {
    tse::ServiceFeesCxrActivation* svcFeeCxr = new ServiceFeesCxrActivation;

    svcFeeCxr->carrier() = row->getString(CARRIER);
    svcFeeCxr->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      svcFeeCxr->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(EFFDATE))
      svcFeeCxr->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      svcFeeCxr->discDate() = row->getDate(DISCDATE);

    return svcFeeCxr;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetServiceFeesCxrActivationHistoricalSQLStatement
    : public QueryGetServiceFeesCxrActivationSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER= %1q  "
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  };
};
}
