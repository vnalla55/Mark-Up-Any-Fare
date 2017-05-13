//----------------------------------------------------------------------------
//          File:           QueryCheckLocsAndZonesSQLStatement.h
//          Description:    QueryCheckLocsAndZonesSQLStatement
//          Created:        10/26/2007
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

#include "DBAccess/Queries/QueryCheckLocsAndZones.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

struct SITAAddonScoreResult
{
  int zone;
  int addonTarriff;
  char inclExclInd;
  char locType;
  const char* loc;
  DateTime createDate;
  DateTime expireDate;
  DateTime effDate;
  DateTime discDate;
  int score;
};

template <class QUERYCLASS>
class QueryGetSITAAddonScoreSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSITAAddonScoreSQLStatement() {};
  virtual ~QueryGetSITAAddonScoreSQLStatement() {};

  enum ColumnIndexes
  {
    ZONENO = 0,
    ADDONTARIFF,
    INCLEXCLIND,
    LOCTYPE,
    LOC,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    SCORE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -1"
        "            when 'I' then 1"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = 'A' "
                           "     and mkt.area  = zone.loc "
                           "     and mkt.market = %1q"
                           "     and carrier = %2q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE"
                           "     and %cd <= mkt.EXPIREDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union"
        " select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -2"
        "            when 'I' then 2"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = '*' "
                           "     and mkt.subarea  = zone.loc"
                           "     and mkt.market = %3q"
                           "     and carrier = %4q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE"
                           "     and %cd <= mkt.EXPIREDATE");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union"
        " select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -2"
        "            when 'I' then 2"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone");
    partialStatement.Where("loctype = 'Z' "
                           "     and zone.loc = %11q"
                           "     and zone.loc <> ''"
                           "     and carrier = %12q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE");
    adjustBaseSQL(2, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union"
        " select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -3"
        "            when 'I' then 3"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt  ");
    partialStatement.Where("loctype = 'N' "
                           "     and mkt.nation   = zone.loc  "
                           "     and mkt.market = %5q"
                           "     and carrier = %6q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE"
                           "     and %cd <= mkt.EXPIREDATE");
    adjustBaseSQL(3, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union"
        " select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -4"
        "            when 'I' then 4"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = 'S' "
                           "     and concat(mkt.nation, mkt.state)  = zone.loc"
                           "     and mkt.market = %7q"
                           "     and carrier = %8q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE"
                           "     and %cd <= mkt.EXPIREDATE");
    adjustBaseSQL(4, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union"
        " select distinct zone.ZONENO, zone.ADDONTARIFF,"
        "                 zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
        "                 zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
        "          case INCLEXCLIND"
        "            when 'E' then -5"
        "            when 'I' then 5"
        "          end AS SCORE");
    partialStatement.From("=ADDONZONE zone ");
    partialStatement.Where(" loctype = 'C' "
                           "     and zone.loc = %9q"
                           "     and carrier = %10q "
                           "     and vendor = 'SITA'"
                           "     and %cd <= zone.EXPIREDATE");
    partialStatement.OrderBy("1,2,3");
    adjustBaseSQL(5, partialStatement);
    compoundStatement.push_back(partialStatement);

    this->Command(compoundStatement.ConstructSQL());

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static void mapRowToSITAAddonScoreResult(Row* row, struct SITAAddonScoreResult& result)
  {
    result.zone = row->getInt(ZONENO);
    result.addonTarriff = row->getInt(ADDONTARIFF);
    result.inclExclInd = row->getChar(INCLEXCLIND);
    result.locType = row->getChar(LOCTYPE);
    result.loc = row->getString(LOC);
    result.createDate = row->getDate(CREATEDATE);
    result.expireDate = row->getDate(EXPIREDATE);
    result.effDate = row->getDate(EFFDATE);
    result.discDate = row->getDate(DISCDATE);
    result.score = row->getInt(SCORE);
  }

private:
  // Override these functions to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSITAAddonScoreHist
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSITAAddonScoreHistoricalSQLStatement
    : public QueryGetSITAAddonScoreSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    // First the Historical
    partialStatement.Command("(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -1"
                             "   when 'I' then 1"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone, =MARKET mkt");
    partialStatement.Where("loctype = 'A' "
                           " and mkt.area  = zone.loc "
                           " and mkt.market = %1q"
                           " and carrier = %2q "
                           " and vendor = 'SITA'"
                           " and %3n <= zone.EXPIREDATE"
                           " and (   %4n >= zone.CREATEDATE"
                           "      or %5n >= zone.EFFDATE"
                           "     )"
                           " and %6n <= mkt.EXPIREDATE"
                           " and (   %7n >= mkt.CREATEDATE"
                           "      or %8n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -2"
                             "   when 'I' then 2"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone, =MARKET mkt");
    partialStatement.Where("loctype = '*' "
                           " and mkt.subarea  = zone.loc"
                           " and mkt.market = %9q"
                           " and carrier = %10q "
                           " and vendor = 'SITA'"
                           " and %11n <= zone.EXPIREDATE"
                           " and (   %12n >= zone.CREATEDATE"
                           "      or %13n >= zone.EFFDATE"
                           "     )"
                           " and %14n <= mkt.EXPIREDATE"
                           " and (   %15n >= mkt.CREATEDATE"
                           "      or %16n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -2"
                             "   when 'I' then 2"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone");
    partialStatement.Where("loctype = 'Z' "
                           " and zone.loc = %17q"
                           " and zone.loc <> ''"
                           " and carrier = %18q "
                           " and vendor = 'SITA'"
                           " and %19n <= zone.EXPIREDATE"
                           " and (   %20n >= zone.CREATEDATE"
                           "      or %21n >= zone.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(2, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -3"
                             "   when 'I' then 3"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone, =MARKET mkt ");
    partialStatement.Where("loctype = 'N' "
                           " and mkt.nation   = zone.loc  "
                           " and mkt.market = %22q"
                           " and carrier = %23q "
                           " and vendor = 'SITA'"
                           " and %24n <= zone.EXPIREDATE"
                           " and (   %25n >= zone.CREATEDATE"
                           "      or %26n >= zone.EFFDATE"
                           "     )"
                           " and %27n <= mkt.EXPIREDATE"
                           " and (   %28n >= mkt.CREATEDATE"
                           "      or %29n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(3, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -4"
                             "   when 'I' then 4"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone, =MARKET mkt");
    partialStatement.Where("loctype = 'S' "
                           " and concat(mkt.nation, mkt.state)  = zone.loc"
                           " and mkt.market = %30q"
                           " and carrier = %31q "
                           " and vendor = 'SITA'"
                           " and %32n <= zone.EXPIREDATE"
                           " and (   %33n >= zone.CREATEDATE"
                           "      or %34n >= zone.EFFDATE"
                           "     )"
                           " and %35n <= mkt.EXPIREDATE"
                           " and (   %36n >= mkt.CREATEDATE"
                           "      or %37n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(4, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -5"
                             "   when 'I' then 5"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONEH zone ");
    partialStatement.Where(" loctype = 'C' "
                           " and zone.loc = %38q"
                           " and carrier = %39q "
                           " and vendor = 'SITA'"
                           " and %40n <= zone.EXPIREDATE"
                           " and (   %41n >= zone.CREATEDATE"
                           "      or %42n >= zone.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(5, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();

    // Now the Current
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -1"
                             "   when 'I' then 1"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = 'A' "
                           " and mkt.area  = zone.loc "
                           " and mkt.market = %43q"
                           " and carrier = %44q "
                           " and vendor = 'SITA'"
                           " and %45n <= zone.EXPIREDATE"
                           " and (   %46n >= zone.CREATEDATE"
                           "      or %47n >= zone.EFFDATE"
                           "     )"
                           " and %48n <= mkt.EXPIREDATE"
                           " and (   %49n >= mkt.CREATEDATE"
                           "      or %50n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(6, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -2"
                             "   when 'I' then 2"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = '*' "
                           " and mkt.subarea  = zone.loc"
                           " and mkt.market = %51q"
                           " and carrier = %52q "
                           " and vendor = 'SITA'"
                           " and %53n <= zone.EXPIREDATE"
                           " and (   %54n >= zone.CREATEDATE"
                           "      or %55n >= zone.EFFDATE"
                           "     )"
                           " and %56n <= mkt.EXPIREDATE"
                           " and (   %57n >= mkt.CREATEDATE"
                           "      or %58n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(7, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -2"
                             "   when 'I' then 2"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone");
    partialStatement.Where("loctype = 'Z' "
                           " and zone.loc = %59q"
                           " and zone.loc <> ''"
                           " and carrier = %60q "
                           " and vendor = 'SITA'"
                           " and %61n <= zone.EXPIREDATE"
                           " and (   %62n >= zone.CREATEDATE"
                           "      or %63n >= zone.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(8, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -3"
                             "   when 'I' then 3"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt ");
    partialStatement.Where("loctype = 'N' "
                           " and mkt.nation   = zone.loc  "
                           " and mkt.market = %64q"
                           " and carrier = %65q "
                           " and vendor = 'SITA'"
                           " and %66n <= zone.EXPIREDATE"
                           " and (   %67n >= zone.CREATEDATE"
                           "      or %68n >= zone.EFFDATE"
                           "     )"
                           " and %69n <= mkt.EXPIREDATE"
                           " and (   %70n >= mkt.CREATEDATE"
                           "      or %71n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(9, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -4"
                             "   when 'I' then 4"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone, =MARKET mkt");
    partialStatement.Where("loctype = 'S' "
                           " and concat(mkt.nation, mkt.state)  = zone.loc"
                           " and mkt.market = %72q"
                           " and carrier = %73q "
                           " and vendor = 'SITA'"
                           " and %74n <= zone.EXPIREDATE"
                           " and (   %75n >= zone.CREATEDATE"
                           "      or %76n >= zone.EFFDATE"
                           "     )"
                           " and %77n <= mkt.EXPIREDATE"
                           " and (   %78n >= mkt.CREATEDATE"
                           "      or %79n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(10, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union "
                             "(select distinct zone.ZONENO, zone.ADDONTARIFF,"
                             " zone.INCLEXCLIND, zone.LOCTYPE, zone.LOC,"
                             " zone.CREATEDATE, zone.EXPIREDATE, zone.EFFDATE, zone.DISCDATE, "
                             " case INCLEXCLIND"
                             "   when 'E' then -5"
                             "   when 'I' then 5"
                             " end AS SCORE");
    partialStatement.From("=ADDONZONE zone ");
    partialStatement.Where("loctype = 'C' "
                           " and zone.loc = %80q"
                           " and carrier = %81q "
                           " and vendor = 'SITA'"
                           " and %82n <= zone.EXPIREDATE"
                           " and (   %83n >= zone.CREATEDATE"
                           "      or %84n >= zone.EFFDATE"
                           "     )"
                           ")");
    partialStatement.OrderBy("1,2,3");
    adjustBaseSQL(11, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  } // adjustBaseSQL

  //  override this version to replace parts of the compound statement
  void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) override {}
}; // class QueryGetSITAAddonScoreHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// QueryGetZoneLocTypes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetZoneLocTypesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetZoneLocTypesSQLStatement() {};
  virtual ~QueryGetZoneLocTypesSQLStatement() {};

  enum ColumnIndexes
  {
    LOCTYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct LOCTYPE ");
    this->From("=ZONE zone, =ZONESEQ zoneseq ");
    this->Where("zone.zoneno = %1q "
                " and zone.zonetype = %2q "
                " and zoneseq.inclexclind = %3q "
                " and zone.vendor = %4q "
                " and zone.zoneno = zoneseq.zoneno "
                " and zone.zonetype = zoneseq.zonetype "
                " and zone.vendor = zoneseq.VENDOR "
                " and zone.createdate = zoneseq.createdate "
                " and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const char* mapRowToString(Row* row) { return row->getString(LOCTYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetZoneLocTypesHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetZoneLocTypesHistoricalSQLStatement
    : public QueryGetZoneLocTypesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select distinct LOCTYPE ");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq ");
    partialStatement.Where("zone.zoneno = %1q "
                           " and zone.zonetype = %2q "
                           " and zoneseq.inclexclind = %3q "
                           " and zone.vendor = %4q "
                           " and %5n <= zone.EXPIREDATE"
                           " and (   %6n >= zone.CREATEDATE"
                           "      or %7n >= zone.EFFDATE"
                           "     )"
                           " and zone.zoneno = zoneseq.zoneno "
                           " and zone.zonetype = zoneseq.zonetype "
                           " and zone.vendor = zoneseq.VENDOR "
                           " and zone.createdate = zoneseq.createdate"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select distinct LOCTYPE ");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq ");
    partialStatement.Where("zone.zoneno = %8q "
                           " and zone.zonetype = %9q "
                           " and zoneseq.inclexclind = %10q "
                           " and zone.vendor = %11q "
                           " and %12n <= zone.EXPIREDATE"
                           " and (   %13n >= zone.CREATEDATE"
                           "      or %14n >= zone.EFFDATE"
                           "     )"
                           " and zone.zoneno = zoneseq.zoneno "
                           " and zone.zonetype = zoneseq.zonetype "
                           " and zone.vendor = zoneseq.VENDOR "
                           " and zone.createdate = zoneseq.createdate"
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
// QueryGetMktCtyZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktCtyZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktCtyZoneSQLStatement() {};
  virtual ~QueryGetMktCtyZoneSQLStatement() {};

  enum ColumnIndexes
  {
    SCORE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select 5 AS SCORE");
    this->From("=ZONE zone, =ZONESEQ zoneseq");
    this->Where("zone.zoneno = zoneseq.zoneno"
                " and zone.zonetype = zoneseq.zonetype"
                " and zone.zonetype = %1q"
                " and loctype = 'C'"
                " and inclexclind = %2q"
                " and zone.vendor = zoneseq.VENDOR"
                " and zone.createdate = zoneseq.createdate"
                " and zoneseq.loc = %3q"
                " and zoneseq.zoneno = %4n"
                " and zone.zoneno = %5q"
                " and zone.vendor = %6q"
                " and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(SCORE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktCtyZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktCtyZoneHistoricalSQLStatement : public QueryGetMktCtyZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select 5 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %1q"
                           " and loctype = 'C'"
                           " and inclexclind = %2q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and zoneseq.loc = %3q"
                           " and zoneseq.zoneno = %4n"
                           " and zone.zoneno = %5q"
                           " and zone.vendor = %6q"
                           " and %7n <= zone.EXPIREDATE"
                           " and (   %8n >= zone.CREATEDATE"
                           "      or %9n >= zone.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select 5 AS SCORE");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq ");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %10q"
                           " and loctype = 'C'"
                           " and inclexclind = %11q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and zoneseq.loc = %12q"
                           " and zoneseq.zoneno = %13n"
                           " and zone.zoneno = %14q"
                           " and zone.vendor = %15q"
                           " and %16n <= zone.EXPIREDATE"
                           " and (   %17n >= zone.CREATEDATE"
                           "      or %18n >= zone.EFFDATE"
                           "     )"
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
// QueryGetMktStateZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktStateZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktStateZoneSQLStatement() {};
  virtual ~QueryGetMktStateZoneSQLStatement() {};

  enum ColumnIndexes
  {
    SCORE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select 4 AS SCORE");
    this->From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    this->Where("zone.zoneno = zoneseq.zoneno"
                " and zone.zonetype = zoneseq.zonetype"
                " and zone.zonetype = %1q"
                " and loctype = 'S'"
                " and inclexclind = %2q"
                " and zone.vendor = zoneseq.VENDOR"
                " and zone.createdate = zoneseq.createdate"
                " and concat(mkt.nation, mkt.state) = zoneseq.loc"
                " and mkt.market = %3q"
                " and zoneseq.zoneno = %4n"
                " and zone.zoneno = %5q"
                " and zone.vendor = %6q"
                " and %cd <= mkt.EXPIREDATE"
                " and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(SCORE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktStateZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktStateZoneHistoricalSQLStatement
    : public QueryGetMktStateZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select 4 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %1q"
                           " and loctype = 'S'"
                           " and inclexclind = %2q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and concat(mkt.nation, mkt.state) = zoneseq.loc"
                           " and mkt.market = %3q"
                           " and zoneseq.zoneno = %4n"
                           " and zone.zoneno = %5q"
                           " and zone.vendor = %6q"
                           " and %7n <= zone.EXPIREDATE"
                           " and (   %8n >= zone.CREATEDATE"
                           "      or %9n >= zone.EFFDATE"
                           "     )"
                           " and %10n <= mkt.EXPIREDATE"
                           " and (   %11n >= mkt.CREATEDATE"
                           "      or %12n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select 4 AS SCORE");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    partialStatement.Where("(select 4 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %13q"
                           " and loctype = 'S'"
                           " and inclexclind = %14q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and concat(mkt.nation, mkt.state) = zoneseq.loc"
                           " and mkt.market = %15q"
                           " and zoneseq.zoneno = %16n"
                           " and zone.zoneno = %17q"
                           " and zone.vendor = %18q"
                           " and %19n <= zone.EXPIREDATE"
                           " and (   %20n >= zone.CREATEDATE"
                           "      or %21n >= zone.EFFDATE"
                           "     )"
                           " and %22n <= mkt.EXPIREDATE"
                           " and (   %23n >= mkt.CREATEDATE"
                           "      or %24n >= mkt.EFFDATE"
                           "     )"
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
// QueryGetMktNationZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktNationZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktNationZoneSQLStatement() {};
  virtual ~QueryGetMktNationZoneSQLStatement() {};

  enum ColumnIndexes
  {
    SCORE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select 3 AS SCORE");
    this->From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt ");
    this->Where("zone.zoneno = zoneseq.zoneno "
                " and zone.zonetype = zoneseq.zonetype "
                " and zone.zonetype = %1q "
                " and loctype = 'N' "
                " and inclexclind = %2q "
                " and zone.vendor = zoneseq.VENDOR "
                " and zone.createdate = zoneseq.createdate "
                " and mkt.nation = zoneseq.loc "
                " and mkt.market = %3q "
                " and zoneseq.zoneno = %4n "
                " and zone.zoneno = %5q "
                " and zone.vendor = %6q"
                " and %cd <= mkt.EXPIREDATE"
                " and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(SCORE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktNationZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktNationZoneHistoricalSQLStatement
    : public QueryGetMktNationZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select 3 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno "
                           " and zone.zonetype = zoneseq.zonetype "
                           " and zone.zonetype = %1q "
                           " and loctype = 'N' "
                           " and inclexclind = %2q "
                           " and zone.vendor = zoneseq.VENDOR "
                           " and zone.createdate = zoneseq.createdate "
                           " and mkt.nation = zoneseq.loc "
                           " and mkt.market = %3q "
                           " and zoneseq.zoneno = %4n "
                           " and zone.zoneno = %5q "
                           " and zone.vendor = %6q"
                           " and %7n <= zone.EXPIREDATE"
                           " and (   %8n >= zone.CREATEDATE"
                           "      or %9n >= zone.EFFDATE"
                           "     )"
                           " and %10n <= mkt.EXPIREDATE"
                           " and (   %11n >= mkt.CREATEDATE"
                           "      or %12n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select 3 AS SCORE");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno "
                           " and zone.zonetype = zoneseq.zonetype "
                           " and zone.zonetype = %13q "
                           " and loctype = 'N' "
                           " and inclexclind = %14q "
                           " and zone.vendor = zoneseq.VENDOR "
                           " and zone.createdate = zoneseq.createdate "
                           " and mkt.nation = zoneseq.loc "
                           " and mkt.market = %15q "
                           " and zoneseq.zoneno = %16n "
                           " and zone.zoneno = %17q "
                           " and zone.vendor = %18q"
                           " and %19n <= zone.EXPIREDATE"
                           " and (   %20n >= zone.CREATEDATE"
                           "      or %21n >= zone.EFFDATE"
                           "     )"
                           " and %22n <= mkt.EXPIREDATE"
                           " and (   %23n >= mkt.CREATEDATE"
                           "      or %24n >= mkt.EFFDATE"
                           "     )"
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
}; // class QueryGetMktNationZoneHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// QueryGetMktSubAreaZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktSubAreaZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktSubAreaZoneSQLStatement() {};
  virtual ~QueryGetMktSubAreaZoneSQLStatement() {};

  enum ColumnIndexes
  {
    SCORE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select 2 AS SCORE");
    this->From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    this->Where("zone.zoneno = zoneseq.zoneno"
                "   and zone.zonetype = zoneseq.zonetype"
                "   and zone.zonetype = %1q"
                "   and loctype = '*'"
                "   and inclexclind = %2q"
                "   and zone.vendor = zoneseq.VENDOR"
                "   and zone.createdate = zoneseq.createdate"
                "   and mkt.subarea   = zoneseq.loc"
                "   and mkt.market = %3q"
                "   and zoneseq.zoneno = %4n"
                "   and zone.zoneno = %5n"
                "   and zone.vendor = %6q"
                "   and %cd <= mkt.EXPIREDATE"
                "   and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(SCORE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktSubAreaZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktSubAreaZoneHistoricalSQLStatement
    : public QueryGetMktSubAreaZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select 2 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %1q"
                           " and loctype = '*'"
                           " and inclexclind = %2q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and mkt.subarea   = zoneseq.loc"
                           " and mkt.market = %3q"
                           " and zoneseq.zoneno = %4n"
                           " and zone.zoneno = %5n"
                           " and zone.vendor = %6q"
                           " and %7n <= zone.EXPIREDATE"
                           " and (   %8n >= zone.CREATEDATE"
                           "      or %9n >= zone.EFFDATE"
                           "     )"
                           " and %10n <= mkt.EXPIREDATE"
                           " and (   %11n >= mkt.CREATEDATE"
                           "      or %12n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select 2 AS SCORE");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %13q"
                           " and loctype = '*'"
                           " and inclexclind = %14q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and mkt.subarea   = zoneseq.loc"
                           " and mkt.market = %15q"
                           " and zoneseq.zoneno = %16n"
                           " and zone.zoneno = %17n"
                           " and zone.vendor = %18q"
                           " and %19n <= zone.EXPIREDATE"
                           " and (   %20n >= zone.CREATEDATE"
                           "      or %21n >= zone.EFFDATE"
                           "     )"
                           " and %22n <= mkt.EXPIREDATE"
                           " and (   %23n >= mkt.CREATEDATE"
                           "      or %24n >= mkt.EFFDATE"
                           "     )"
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
}; // class QueryGetMktSubAreaZoneHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// QueryGetMktAreaZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktAreaZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktAreaZoneSQLStatement() {};
  virtual ~QueryGetMktAreaZoneSQLStatement() {};

  enum ColumnIndexes
  {
    SCORE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select 1 AS SCORE");
    this->From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    this->Where("zone.zoneno = zoneseq.zoneno"
                "  and zone.zonetype = zoneseq.zonetype"
                "  and zone.zonetype = %1q"
                "  and loctype = 'A'"
                "  and inclexclind = %2q"
                "  and zone.vendor = zoneseq.VENDOR"
                "  and zone.createdate = zoneseq.createdate"
                "  and mkt.area   = zoneseq.loc"
                "  and mkt.market = %3q"
                "  and zoneseq.zoneno = %4n"
                "  and zone.zoneno = %5q"
                "  and zone.vendor = %6q"
                "  and %cd <= mkt.EXPIREDATE"
                "  and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(SCORE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktAreaZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktAreaZoneHistoricalSQLStatement : public QueryGetMktAreaZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select 1 AS SCORE");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %1q"
                           " and loctype = 'A'"
                           " and inclexclind = %2q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and mkt.area   = zoneseq.loc"
                           " and mkt.market = %3q"
                           " and zoneseq.zoneno = %4n"
                           " and zone.zoneno = %5q"
                           " and zone.vendor = %6q"
                           " and %7n <= zone.EXPIREDATE"
                           " and (   %8n >= zone.CREATEDATE"
                           "      or %9n >= zone.EFFDATE"
                           "     )"
                           " and %10n <= mkt.EXPIREDATE"
                           " and (   %11n >= mkt.CREATEDATE"
                           "      or %12n >= mkt.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select 1 AS SCORE");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq, =MARKET mkt");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %13q"
                           " and loctype = 'A'"
                           " and inclexclind = %14q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and mkt.area   = zoneseq.loc"
                           " and mkt.market = %15q"
                           " and zoneseq.zoneno = %16n"
                           " and zone.zoneno = %17q"
                           " and zone.vendor = %18q"
                           " and %19n <= zone.EXPIREDATE"
                           " and (   %20n >= zone.CREATEDATE"
                           "      or %21n >= zone.EFFDATE"
                           "     )"
                           " and %22n <= mkt.EXPIREDATE"
                           " and (   %23n >= mkt.CREATEDATE"
                           "      or %24n >= mkt.EFFDATE"
                           "     )"
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
// QueryGetMktZoneZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktZoneZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktZoneZoneSQLStatement() {};
  virtual ~QueryGetMktZoneZoneSQLStatement() {};

  enum ColumnIndexes
  {
    LOC = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select zoneseq.LOC");
    this->From("=ZONE zone, =ZONESEQ zoneseq");
    this->Where("zone.zoneno = zoneseq.zoneno"
                "   and zone.zonetype = zoneseq.zonetype"
                "   and zone.zonetype = %1q"
                "   and loctype = 'Z'"
                "   and inclexclind = %2q"
                "   and zone.vendor = zoneseq.VENDOR"
                "   and zone.createdate = zoneseq.createdate"
                "   and zoneseq.zoneno = %3n"
                "   and zone.zoneno = %4q"
                "   and zone.vendor = %5q"
                "   and %cd <= zone.EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const int mapRowToInt(Row* row) { return row->getInt(LOC); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMktZoneZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktZoneZoneHistoricalSQLStatement : public QueryGetMktZoneZoneSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select zoneseq.LOC");
    partialStatement.From("=ZONEH zone, =ZONESEQH zoneseq");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %1q"
                           " and loctype = 'Z'"
                           " and inclexclind = %2q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and zoneseq.zoneno = %3n"
                           " and zone.zoneno = %4q"
                           " and zone.vendor = %5q"
                           " and %6n <= zone.EXPIREDATE"
                           " and (   %7n >= zone.CREATEDATE"
                           "      or %8n >= zone.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select zoneseq.LOC");
    partialStatement.From("=ZONE zone, =ZONESEQ zoneseq");
    partialStatement.Where("zone.zoneno = zoneseq.zoneno"
                           " and zone.zonetype = zoneseq.zonetype"
                           " and zone.zonetype = %9q"
                           " and loctype = 'Z'"
                           " and inclexclind = %10q"
                           " and zone.vendor = zoneseq.VENDOR"
                           " and zone.createdate = zoneseq.createdate"
                           " and zoneseq.zoneno = %11n"
                           " and zone.zoneno = %12q"
                           " and zone.vendor = %13q"
                           " and %14n <= zone.EXPIREDATE"
                           " and (   %15n >= zone.CREATEDATE"
                           "      or %16n >= zone.EFFDATE"
                           "     )"
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
} // tse
