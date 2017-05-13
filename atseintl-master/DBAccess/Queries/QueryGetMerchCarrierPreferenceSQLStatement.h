//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/MerchCarrierPreferenceInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetMerchCarrierPreferenceSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMerchCarrierPreferenceSQLStatement() {}
  virtual ~QueryGetMerchCarrierPreferenceSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER = 0,
    SVCGROUP,
    CREATEDATE,
    PREFERREDVENDOR,
    ALTPROCESSIND,
    SECTORPORTIONIND,
    CONCURRENCEIND,
    EXPIREDATE,
    EFFDATE,
    DISCDATE
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select CARRIER, SVCGROUP, CREATEDATE, PREFERREDVENDOR, ALTPROCESSIND, SECTORPORTIONIND,"
        "       CONCURRENCEIND, EXPIREDATE, EFFDATE, DISCDATE");

    this->From("=MERCHCARRIERPREFERENCE");

    this->Where("CARRIER= %1q "
                " and SVCGROUP= %2q ");
    this->OrderBy("CARRIER, SVCGROUP, CREATEDATE ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MerchCarrierPreferenceInfo* mapRowToMerchCarrierPreferenceInfo(Row* row)
  {
    MerchCarrierPreferenceInfo* merchCarrierPreference = new MerchCarrierPreferenceInfo;

    merchCarrierPreference->carrier() = row->getString(CARRIER);
    merchCarrierPreference->groupCode() = row->getString(SVCGROUP);
    merchCarrierPreference->createDate() = row->getDate(CREATEDATE);
    merchCarrierPreference->prefVendor() = row->getString(PREFERREDVENDOR);
    merchCarrierPreference->altProcessInd() = row->getChar(ALTPROCESSIND);
    merchCarrierPreference->sectorPortionInd() = row->getChar(SECTORPORTIONIND);
    merchCarrierPreference->concurrenceInd() = row->getChar(CONCURRENCEIND);
    merchCarrierPreference->expireDate() = row->getDate(EXPIREDATE);
    merchCarrierPreference->effDate() = row->getDate(EFFDATE);
    merchCarrierPreference->discDate() = row->getDate(DISCDATE);

    return merchCarrierPreference;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMerchCarrierPreferenceHistoricalSQLStatement
    : public QueryGetMerchCarrierPreferenceSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER= %1q "
                " and SVCGROUP= %2q "
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("CARRIER, SVCGROUP, CREATEDATE");
  }
};
} // tse
