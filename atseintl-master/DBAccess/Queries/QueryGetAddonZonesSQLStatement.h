//----------------------------------------------------------------------------
//          File:           QueryGetAddonZonesSQLStatement.h
//          Description:    QueryGetAddonZonesSQLStatement
//          Created:        10/25/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAddonZones.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAddonZonesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAddonZonesSQLStatement() {};
  virtual ~QueryGetAddonZonesSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ADDONTARIFF,
    CARRIER,
    ZONENO,
    CREATEDATE,
    EXPIREDATE,
    INCLEXCLIND,
    LOCTYPE,
    LOC,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ADDONTARIFF,CARRIER,ZONENO,CREATEDATE,EXPIREDATE,"
                  "       INCLEXCLIND,LOCTYPE,LOC,EFFDATE,DISCDATE");
    this->From(" =ADDONZONE ");
    this->Where("VENDOR = %1q"
                "   and CARRIER = %2q "
                "   and LOC = %3q "
                "   and LOCTYPE = 'C'"
                "   and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,LOC,ADDONTARIFF,ZONENO,SEQNO,INCLEXCLIND,LOCTYPE,LOC,"
                    "CREATEDATE,EFFDATE,DISCDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::AddonZoneInfo* mapRowToAddonZoneInfo(Row* row)
  {
    tse::AddonZoneInfo* addOn = new tse::AddonZoneInfo;

    addOn->vendor() = row->getString(VENDOR);
    addOn->fareTariff() = row->getInt(ADDONTARIFF);
    addOn->carrier() = row->getString(CARRIER);
    addOn->inclExclInd() = row->getChar(INCLEXCLIND);

    LocKey* loc = &addOn->market();
    loc->locType() = row->getChar(LOCTYPE);
    loc->loc() = row->getString(LOC);
    addOn->zone() = row->getInt(ZONENO);

    addOn->createDate() = row->getDate(CREATEDATE);
    addOn->expireDate() = row->getDate(EXPIREDATE);
    addOn->effDate() = row->getDate(EFFDATE);
    addOn->discDate() = row->getDate(DISCDATE);

    return addOn;
  } // mapRowToAddonZoneInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllAddonZones
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllAddonZonesSQLStatement : public QueryGetAddonZonesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("LOCTYPE = 'C'"
                "    and %cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,LOC,ADDONTARIFF,ZONENO,SEQNO,INCLEXCLIND,LOCTYPE,LOC,"
                    "CREATEDATE,EFFDATE,DISCDATE");
    else
      this->OrderBy("VENDOR, CARRIER, LOC");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonZoneInfo
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAddonZoneInfoSQLStatement : public QueryGetAddonZonesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                "   and CARRIER = %2q "
                "   and ADDONTARIFF = %3n "
                "   and ZONENO = %4n "
                "   and %cd <= EXPIREDATE ");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonZonesHist
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAddonZonesHistoricalSQLStatement : public QueryGetAddonZonesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,ADDONTARIFF,CARRIER,ZONENO,CREATEDATE,EXPIREDATE,"
                             "       INCLEXCLIND,LOCTYPE,LOC,EFFDATE,DISCDATE");
    partialStatement.From("=ADDONZONEH");
    partialStatement.Where("VENDOR = %1q"
                           " and CARRIER = %2q "
                           " and LOC = %3q "
                           " and LOCTYPE = 'C'"
                           " and %4n <= EXPIREDATE"
                           " and (   %5n >= CREATEDATE"
                           "      or %6n >= EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             "("
                             "select VENDOR,ADDONTARIFF,CARRIER,ZONENO,CREATEDATE,EXPIREDATE,"
                             "       INCLEXCLIND,LOCTYPE,LOC,EFFDATE,DISCDATE");
    partialStatement.From("=ADDONZONE");
    partialStatement.Where("VENDOR = %7q"
                           " and CARRIER = %8q "
                           " and LOC = %9q "
                           " and LOCTYPE = 'C'"
                           " and %10n <= EXPIREDATE"
                           " and (   %11n >= CREATEDATE"
                           "      or %12n >= EFFDATE)"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonZoneInfoHist
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAddonZoneInfoHistoricalSQLStatement
    : public QueryGetAddonZonesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,ADDONTARIFF,CARRIER,ZONENO,CREATEDATE,EXPIREDATE,"
                             "       INCLEXCLIND,LOCTYPE,LOC,EFFDATE,DISCDATE");
    partialStatement.From("=ADDONZONEH");
    partialStatement.Where("VENDOR = %1q"
                           " and CARRIER = %2q "
                           " and ADDONTARIFF = %3n "
                           " and ZONENO = %4n "
                           " and %5n <= EXPIREDATE"
                           " and (   %6n >= CREATEDATE"
                           "      or %7n >= EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             "("
                             "select VENDOR,ADDONTARIFF,CARRIER,ZONENO,CREATEDATE,EXPIREDATE,"
                             "       INCLEXCLIND,LOCTYPE,LOC,EFFDATE,DISCDATE");
    partialStatement.From("=ADDONZONE");
    partialStatement.Where("VENDOR = %8q"
                           " and CARRIER = %9q "
                           " and ADDONTARIFF = %10n "
                           " and ZONENO = %11n "
                           " and %12n <= EXPIREDATE"
                           " and (   %13n >= CREATEDATE"
                           "      or %14n >= EFFDATE)"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
} // tse
