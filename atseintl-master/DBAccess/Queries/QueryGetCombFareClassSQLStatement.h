//----------------------------------------------------------------------------
//          File:           QueryGetCombFareClassSQLStatement.h
//          Description:    QueryGetCombFareClassSQLStatement
//          Created:        10/30/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCombFareClass.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetCombFareClassSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCombFareClassSQLStatement() {};
  virtual ~QueryGetCombFareClassSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    FARETARIFF,
    CARRIER,
    ADDONFARECLASS,
    GEOAPPL,
    OWRT,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FARECLASS,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,FARETARIFF,CARRIER,ADDONFARECLASS,GEOAPPL,OWRT,"
                  " CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,FARECLASS");
    this->From(" =ADDONCOMBFARECLASS");
    this->Where("VENDOR = %1q "
                " and FARETARIFF = %2n "
                " and CARRIER = %3q "
                " and %cd <= EXPIREDATE ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  static std::pair<AddonCombFareClassInfoKey1, AddonCombFareClassInfo*>
         mapRowToAddonCombFareClassInfo(Row* row)
  {
    AddonCombFareClassInfoKey1 key(row->getString(VENDOR),
                                   row->getInt(FARETARIFF),
                                   row->getString(CARRIER));
    tse::AddonCombFareClassInfo* addon = new tse::AddonCombFareClassInfo;
    if (const char* afc = row->getString(ADDONFARECLASS))
      addon->addonFareClass() = afc[0];
    addon->geoAppl() = row->getChar(GEOAPPL);
    addon->owrt() = row->getChar(OWRT);
    addon->createDate() = row->getDate(CREATEDATE);
    addon->expireDate() = row->getDate(EXPIREDATE);
    addon->fareClass() = row->getString(FARECLASS);
    return std::make_pair(key, addon);
  } // QueryGetCombFareClass::mapRowToAddonCombFareClassInfo()

#else

  static AddonCombFareClassInfo* mapRowToAddonCombFareClassInfo(Row* row)
  {
    tse::AddonCombFareClassInfo* addon = new tse::AddonCombFareClassInfo;
    addon->vendor() = row->getString(VENDOR);
    addon->fareTariff() = row->getInt(FARETARIFF);
    addon->carrier() = row->getString(CARRIER);
    addon->addonFareClass() = row->getString(ADDONFARECLASS);
    addon->geoAppl() = row->getChar(GEOAPPL);
    addon->owrt() = row->getChar(OWRT);
    addon->createDate() = row->getDate(CREATEDATE);
    addon->expireDate() = row->getDate(EXPIREDATE);
    addon->effDate() = row->getDate(EFFDATE);
    addon->discDate() = row->getDate(DISCDATE);
    addon->fareClass() = row->getString(FARECLASS);
    return addon;
  } // QueryGetCombFareClass::mapRowToAddonCombFareClassInfo()

#endif

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCombFareClassHist
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCombFareClassHistoricalSQLStatement
    : public QueryGetCombFareClassSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,FARETARIFF,CARRIER,ADDONFARECLASS,GEOAPPL,OWRT,"
                             " CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,FARECLASS");
    partialStatement.From("=ADDONCOMBFARECLASSH");
    partialStatement.Where("VENDOR = %1q "
                           " and FARETARIFF = %2n "
                           " and CARRIER = %3q "
                           " and %4n <= EXPIREDATE"
                           " and (   %5n >=  CREATEDATE"
                           "      or %6n >= EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR,FARETARIFF,CARRIER,ADDONFARECLASS,GEOAPPL,OWRT,"
                             " CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,FARECLASS");
    partialStatement.From("=ADDONCOMBFARECLASS");
    partialStatement.Where("VENDOR = %7q "
                           " and FARETARIFF = %8n "
                           " and CARRIER = %9q "
                           " and %10n <= EXPIREDATE"
                           " and (   %11n >=  CREATEDATE"
                           "      or %12n >= EFFDATE)"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCombFareClass
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllCombFareClassSQLStatement : public QueryGetCombFareClassSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");

    this->OrderBy("VENDOR, FARETARIFF, CARRIER");

  };
};
} // tse
