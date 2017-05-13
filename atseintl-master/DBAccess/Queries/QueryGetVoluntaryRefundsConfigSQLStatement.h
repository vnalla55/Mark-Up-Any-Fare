//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsConfig.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetVoluntaryRefundsConfigSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    RULEAPPLICATIONDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command("select CARRIER,CREATEDATE,RULEAPPLICATIONDATE,"
                  " EFFDATE,DISCDATE,EXPIREDATE ");
    this->From(" =VOLUNTARYREFUNDSCONFIG");
    this->Where("CARRIER = %1q");
    this->OrderBy("EXPIREDATE desc");

    adjustBaseSQL();

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static VoluntaryRefundsConfig* map(Row* row)
  {
    VoluntaryRefundsConfig* vrc = new VoluntaryRefundsConfig;

    vrc->carrier() = row->getString(CARRIER);
    vrc->effDate() = row->getDate(EFFDATE);
    vrc->createDate() = row->getDate(CREATEDATE);
    vrc->expireDate() = row->getDate(EXPIREDATE);
    vrc->discDate() = row->getDate(DISCDATE);
    vrc->applDate() = row->getDate(RULEAPPLICATIONDATE);

    return vrc;
  }

  virtual ~QueryGetVoluntaryRefundsConfigSQLStatement() {}

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetAllVoluntaryRefundsConfigSQLStatement
    : public QueryGetVoluntaryRefundsConfigSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("CARRIER,EXPIREDATE desc");
  }
};
}

