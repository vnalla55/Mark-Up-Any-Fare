//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/OptionalServicesActivationInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetOptionalServicesActivationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOptionalServicesActivationSQLStatement() {}
  virtual ~QueryGetOptionalServicesActivationSQLStatement() {}

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    SVCGROUP,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    APPLICATION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE, USERAPPL, SVCGROUP, "
                  "       CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE, APPLICATION");

    this->From("=OPTIONALSERVICESACTIVATION");

    this->Where("USERAPPLTYPE= %1q "
                " and USERAPPL= %2q "
                " and APPLICATION= %3q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("USERAPPL, SVCGROUP, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static OptionalServicesActivationInfo* mapRowToOptServicesActivationInfo(Row* row)
  {
    OptionalServicesActivationInfo* optSrvActivation = new OptionalServicesActivationInfo;

    optSrvActivation->userApplType() = row->getChar(USERAPPLTYPE);
    optSrvActivation->userAppl() = row->getString(USERAPPL);
    optSrvActivation->groupCode() = row->getString(SVCGROUP);
    optSrvActivation->createDate() = row->getDate(CREATEDATE);
    optSrvActivation->expireDate() = row->getDate(EXPIREDATE);
    optSrvActivation->effDate() = row->getDate(EFFDATE);
    optSrvActivation->discDate() = row->getDate(DISCDATE);
    optSrvActivation->application() = row->getString(APPLICATION);

    return optSrvActivation;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOptionalServicesActivationHistoricalSQLStatement
    : public QueryGetOptionalServicesActivationSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("USERAPPLTYPE= %1q "
                " and USERAPPL= %2q "
                " and APPLICATION= %3q "
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
    this->OrderBy("USERAPPL, SVCGROUP, CREATEDATE");
  }
};
} // tse
