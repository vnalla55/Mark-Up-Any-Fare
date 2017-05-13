//----------------------------------------------------------------------------
//          File:           QueryCheckNationAndStateLocsSQLStatement.h
//          Description:    QueryCheckNationAndStateLocsSQLStatement
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

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryCheckNationAndStateLocs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInSubArea
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInSubAreaSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationInSubAreaSQLStatement() {};
  virtual ~QueryCheckNationInSubAreaSQLStatement() {};

  enum ColumnIndexes
  {
    SUBAREA = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SUBAREA");
    this->From("=NATION");
    this->Where("NATION  = %1q "
                " and SUBAREA = %2q"
                " and %cd <= EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
}; // class QueryCheckNationInSubAreaSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInSubAreaHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInSubAreaHistoricalSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationInSubAreaHistoricalSQLStatement() {};
  virtual ~QueryCheckNationInSubAreaHistoricalSQLStatement() {};

  enum ColumnIndexes
  {
    CREATEDATE = 0,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CREATEDATE,EXPIREDATE");
    this->From("=NATION");
    this->Where("NATION = %1q "
                " and SUBAREA = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NationStateHistIsCurrChk* mapRowToNationStateHistIsCurrChk(Row* row)
  {
    tse::NationStateHistIsCurrChk* cc = new tse::NationStateHistIsCurrChk;
    cc->inclExclInd() = 'I'; // On these, any row returned is good
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    return cc;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  // for other simple Historical classes
  virtual void adjustBaseSQL() {}
}; // class QueryCheckNationInSubAreaHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInArea
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInAreaSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationInAreaSQLStatement() {};
  virtual ~QueryCheckNationInAreaSQLStatement() {};

  enum ColumnIndexes
  {
    AREA = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select AREA");
    this->From("=NATION");
    this->Where("NATION = %1q "
                " and AREA   = %2q"
                " and %cd <= EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
}; // class QueryCheckNationInAreaSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInAreaHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInAreaHistoricalSQLStatement
    : public QueryCheckNationInSubAreaHistoricalSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("NATION = %1q "
                " and AREA = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryCheckNationInAreaHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationInZoneSQLStatement() {};
  virtual ~QueryCheckNationInZoneSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select zs.INCLEXCLIND");
    this->From("=ZONE z, =ZONESEQ zs");
    this->Where("z.VENDOR     = %1q"
                " and z.ZONENO     = %2q"
                " and z.ZONETYPE   = %3q"
                " and zs.LOCTYPE   = 'N'"
                " and zs.LOC       = %4q"
                " and z.VENDOR     = zs.VENDOR"
                " and z.ZONENO     = zs.ZONENO"
                " and z.ZONETYPE   = zs.ZONETYPE"
                " and z.CREATEDATE = zs.CREATEDATE"
                " and %cd <= z.EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToString(Row* row) { return row->getString(INCLEXCLIND); }
}; // class QueryCheckNationInZoneSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationInZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationInZoneHistoricalSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationInZoneHistoricalSQLStatement() {};
  virtual ~QueryCheckNationInZoneHistoricalSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONEH z, =ZONESEQH zs");
    partialStatement.Where("z.VENDOR          = %1q"
                           " and z.ZONENO     = %2q"
                           " and z.ZONETYPE   = %3q"
                           " and zs.LOCTYPE   = 'N'"
                           " and zs.LOC       = %4q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %5n <= z.EXPIREDATE"
                           " and (   %6n >= z.CREATEDATE"
                           "      or %7n >= z.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONE z, =ZONESEQ zs");
    partialStatement.Where("z.VENDOR          = %8q"
                           " and z.ZONENO     = %9q"
                           " and z.ZONETYPE   = %10q"
                           " and zs.LOCTYPE   = 'N'"
                           " and zs.LOC       = %11q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %12n <= z.EXPIREDATE"
                           " and (   %13n >= z.CREATEDATE"
                           "      or %14n >= z.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    this->Command(compoundStatement.ConstructSQL());

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NationStateHistIsCurrChk* mapRowToNationStateHistIsCurrChk(Row* row)
  {
    tse::NationStateHistIsCurrChk* cc = new tse::NationStateHistIsCurrChk;
    cc->inclExclInd() = row->getChar(INCLEXCLIND);
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    return cc;
  }

private:
  void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryCheckNationInZoneHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationSubAreaInZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationSubAreaInZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationSubAreaInZoneSQLStatement() {};
  virtual ~QueryCheckNationSubAreaInZoneSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select zs.INCLEXCLIND");
    this->From("=ZONE z, =ZONESEQ zs, =NATION n");
    this->Where("z.VENDOR          = %1q"
                " and z.ZONENO     = %2q"
                " and z.ZONETYPE   = %3q"
                " and zs.LOCTYPE   = '*'"
                " and zs.LOC       = n.SUBAREA"
                " and n.NATION     = %4q"
                " and z.VENDOR     = zs.VENDOR"
                " and z.ZONENO     = zs.ZONENO"
                " and z.ZONETYPE   = zs.ZONETYPE"
                " and z.CREATEDATE = zs.CREATEDATE"
                " and %cd <= z.EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const char* mapRowToString(Row* row) { return row->getString(INCLEXCLIND); }
}; // class QueryCheckNationSubAreaInZoneSQLStatement

///////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationSubAreaInZoneHistorical
///////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationSubAreaInZoneHistoricalSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationSubAreaInZoneHistoricalSQLStatement() {};
  virtual ~QueryCheckNationSubAreaInZoneHistoricalSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONEH z, =ZONESEQH zs, =NATION n");
    partialStatement.Where("z.VENDOR          = %1q"
                           " and z.ZONENO     = %2q"
                           " and z.ZONETYPE   = %3q"
                           " and zs.LOCTYPE   = '*'"
                           " and zs.LOC       = n.SUBAREA"
                           " and n.NATION     = %4q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %5n <= z.EXPIREDATE"
                           " and (   %6n >= z.CREATEDATE"
                           "      or %7n >= z.EFFDATE"
                           "     )"
                           " and %8n <= n.EXPIREDATE"
                           " and %9n >= n.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONE z, =ZONESEQ zs, =NATION n");
    partialStatement.Where("z.VENDOR          = %10q"
                           " and z.ZONENO     = %11q"
                           " and z.ZONETYPE   = %12q"
                           " and zs.LOCTYPE   = '*'"
                           " and zs.LOC       = n.SUBAREA"
                           " and n.NATION     = %13q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %14n <= z.EXPIREDATE"
                           " and (   %15n >= z.CREATEDATE"
                           "      or %16n >= z.EFFDATE"
                           "     )"
                           " and %17n <= n.EXPIREDATE"
                           " and %18n >= n.CREATEDATE"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    this->Command(compoundStatement.ConstructSQL());

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NationStateHistIsCurrChk* mapRowToNationStateHistIsCurrChk(Row* row)
  {
    tse::NationStateHistIsCurrChk* cc = new tse::NationStateHistIsCurrChk;
    cc->inclExclInd() = row->getChar(INCLEXCLIND);
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    return cc;
  }

private:
  void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationAreaInZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationAreaInZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationAreaInZoneSQLStatement() {};
  virtual ~QueryCheckNationAreaInZoneSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select zs.INCLEXCLIND");
    this->From("=ZONE z, =ZONESEQ zs, =NATION n");
    this->Where("z.VENDOR          = %1q"
                " and z.ZONENO     = %2q"
                " and z.ZONETYPE   = %3q"
                " and zs.LOCTYPE   = 'A'"
                " and zs.LOC       = n.AREA"
                " and n.NATION     = %4q"
                " and z.VENDOR     = zs.VENDOR"
                " and z.ZONENO     = zs.ZONENO"
                " and z.ZONETYPE   = zs.ZONETYPE"
                " and z.CREATEDATE = zs.CREATEDATE"
                " and %cd <= z.EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToString(Row* row) { return row->getString(INCLEXCLIND); }
}; // class QueryCheckNationAreaInZoneSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckNationAreaInZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckNationAreaInZoneHistoricalSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckNationAreaInZoneHistoricalSQLStatement() {};
  virtual ~QueryCheckNationAreaInZoneHistoricalSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONEH z, =ZONESEQH zs, =NATION n");
    partialStatement.Where("z.VENDOR          = %1q"
                           " and z.ZONENO     = %2q"
                           " and z.ZONETYPE   = %3q"
                           " and zs.LOCTYPE   = 'A'"
                           " and zs.LOC       = n.AREA"
                           " and n.NATION     = %4q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %5n <= z.EXPIREDATE"
                           " and (   %6n >= z.CREATEDATE"
                           "      or %7n >= z.EFFDATE"
                           "     )"
                           " and %8n <= n.EXPIREDATE"
                           " and %9n >= n.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONE z, =ZONESEQ zs, =NATION n");
    partialStatement.Where("z.VENDOR          = %10q"
                           " and z.ZONENO     = %11q"
                           " and z.ZONETYPE   = %12q"
                           " and zs.LOCTYPE   = 'A'"
                           " and zs.LOC       = n.AREA"
                           " and n.NATION     = %13q"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %14n <= z.EXPIREDATE"
                           " and (   %15n >= z.CREATEDATE"
                           "      or %16n >= z.EFFDATE"
                           "     )"
                           " and %17n <= n.EXPIREDATE"
                           " and %18n >= n.CREATEDATE"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    this->Command(compoundStatement.ConstructSQL());

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NationStateHistIsCurrChk* mapRowToNationStateHistIsCurrChk(Row* row)
  {
    tse::NationStateHistIsCurrChk* cc = new tse::NationStateHistIsCurrChk;
    cc->inclExclInd() = row->getChar(INCLEXCLIND);
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    return cc;
  }

private:
  void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInSubArea
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInSubAreaSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckStateInSubAreaSQLStatement() {};
  virtual ~QueryCheckStateInSubAreaSQLStatement() {};

  enum ColumnIndexes
  {
    SUBAREA = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SUBAREA");
    this->From("=STATE");
    this->Where("NATION = %1q "
                " and STATE = %2q "
                " and SUBAREA = %3q"
                " and %cd <= EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
}; // class QueryCheckStateInSubAreaSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInSubAreaHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInSubAreaHistoricalSQLStatement
    : public QueryCheckNationInSubAreaHistoricalSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->From("=STATE");
    this->Where("NATION = %1q "
                " and STATE = %2q "
                " and SUBAREA = %3q"
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
  }
}; // class QueryCheckStateInSubAreaHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInArea
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInAreaSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckStateInAreaSQLStatement() {};
  virtual ~QueryCheckStateInAreaSQLStatement() {};

  enum ColumnIndexes
  {
    AREA = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select AREA");
    this->From("=STATE");
    this->Where("NATION = %1q "
                " and STATE = %2q "
                " and AREA = %3q"
                " and %cd <= EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
}; // class QueryCheckStateInAreaSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInAreaHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInAreaHistoricalSQLStatement
    : public QueryCheckNationInSubAreaHistoricalSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->From("=STATE");
    this->Where("NATION = %1q "
                " and STATE = %2q "
                " and AREA = %3q"
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
  }
}; // class QueryCheckStateInAreaHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInZone
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInZoneSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckStateInZoneSQLStatement() {};
  virtual ~QueryCheckStateInZoneSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select zs.INCLEXCLIND");
    this->From("=ZONE z, =ZONESEQ zs");
    this->Where("z.VENDOR     = %1q"
                " and z.ZONENO     = %2q"
                " and z.ZONETYPE   = %3q"
                " and zs.LOCTYPE   = 'S'"
                " and zs.LOC       = CONCAT(%4q, %5q)"
                " and z.VENDOR     = zs.VENDOR"
                " and z.ZONENO     = zs.ZONENO"
                " and z.ZONETYPE   = zs.ZONETYPE"
                " and z.CREATEDATE = zs.CREATEDATE"
                " and %cd <= z.EXPIREDATE");

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToString(Row* row) { return row->getString(INCLEXCLIND); }
}; // class QueryCheckStateInZoneSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryCheckStateInZoneHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCheckStateInZoneHistoricalSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCheckStateInZoneHistoricalSQLStatement() {};
  virtual ~QueryCheckStateInZoneHistoricalSQLStatement() {};

  enum ColumnIndexes
  {
    INCLEXCLIND = 0,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONEH z, =ZONESEQH zs");
    partialStatement.Where("z.VENDOR          = %1q"
                           " and z.ZONENO     = %2q"
                           " and z.ZONETYPE   = %3q"
                           " and zs.LOCTYPE   = 'S'"
                           " and zs.LOC       = CONCAT(%4q, %5q)"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %6n <= z.EXPIREDATE"
                           " and (   %7n >= z.CREATEDATE"
                           "      or %8n >= z.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all "
                             "(select zs.INCLEXCLIND,z.CREATEDATE,z.EXPIREDATE");
    partialStatement.From("=ZONE z, =ZONESEQ zs");
    partialStatement.Where("z.VENDOR          = %9q"
                           " and z.ZONENO     = %10q"
                           " and z.ZONETYPE   = %11q"
                           " and zs.LOCTYPE   = 'S'"
                           " and zs.LOC       = CONCAT(%12q, %13q)"
                           " and z.VENDOR     = zs.VENDOR"
                           " and z.ZONENO     = zs.ZONENO"
                           " and z.ZONETYPE   = zs.ZONETYPE"
                           " and z.CREATEDATE = zs.CREATEDATE"
                           " and %14n <= z.EXPIREDATE"
                           " and (   %15n >= z.CREATEDATE"
                           "      or %16n >= z.EFFDATE"
                           "     )"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    this->Command(compoundStatement.ConstructSQL());

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NationStateHistIsCurrChk* mapRowToNationStateHistIsCurrChk(Row* row)
  {
    tse::NationStateHistIsCurrChk* cc = new tse::NationStateHistIsCurrChk;
    cc->inclExclInd() = row->getChar(INCLEXCLIND);
    cc->createDate() = row->getDate(CREATEDATE);
    cc->expireDate() = row->getDate(EXPIREDATE);
    return cc;
  }

private:
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
} // tse
