//----------------------------------------------------------------------------
//          File:           QueryGetTrfInhibSQLStatement.h
//          Description:    QueryGetTrfInhibSQLStatement
//          Created:        10/5/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTrfInhib.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTrfInhibSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTrfInhibSQLStatement() {}
  virtual ~QueryGetTrfInhibSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    TARIFFCROSSREFTYPE,
    CARRIER,
    FARETARIFF,
    RULETARIFFCODE,
    RULETARIFF,
    INHIBIT,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,TARIFFCROSSREFTYPE,CARRIER,FARETARIFF,RULETARIFFCODE,"
                  "       RULETARIFF,INHIBIT,CREATEDATE");
    this->From("=TARIFFINHIBITS");
    this->Where(" VENDOR=%1q "
                "    and TARIFFCROSSREFTYPE= %2q "
                "    and CARRIER= %3q "
                "    and FARETARIFF= %4q "
                "    and RULETARIFFCODE= %5q ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, TARIFFCROSSREFTYPE, CARRIER, FARETARIFF, RULETARIFFCODE, RULETARIFF");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TariffInhibits* mapRowToTariffInhibits(Row* row)
  {
    tse::TariffInhibits* ti = new tse::TariffInhibits;

    // Still use fareVendor to key by ... other just to determine inhibit status
    ti->vendor() = row->getString(VENDOR);
    ti->tariffCrossRefType() = row->getChar(TARIFFCROSSREFTYPE);
    ti->carrier() = row->getString(CARRIER);

    ti->fareTariff() = row->getInt(FARETARIFF);
    ti->ruleTariffCode() = row->getString(RULETARIFFCODE);
    ti->inhibit() = row->getChar(INHIBIT);
    ti->createDate() = row->getDate(CREATEDATE);

    return ti;
  } // mapRowToTariffInhibits()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllTrfInhibSQLStatement : public QueryGetTrfInhibSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where(""); }
};
}
