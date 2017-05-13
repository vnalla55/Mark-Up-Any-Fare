//----------------------------------------------------------------------------
//          File:           QueryGetAddonFaresSQLStatement.h
//          Description:    QueryGetAddonFaresSQLStatement
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
#include "DBAccess/Queries/QueryGetAddonFares.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAddonFaresSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAddonFaresSQLStatement() {};
  virtual ~QueryGetAddonFaresSQLStatement() {};

  enum ColumnIndexes
  {
    GATEWAYMARKET = 0,
    INTERIORMARKET,
    CARRIER,
    FARECLASS,
    ADDONTARIFF,
    VENDOR,
    CUR,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NODEC,
    FAREAMT,
    HASHKEY,
    FOOTNOTE1,
    FOOTNOTE2,
    OWRT,
    ROUTING,
    DIRECTIONALITY,
    ARBZONE,
    INHIBIT,
    BASEFAREROUTING,
    THRUFAREROUTING,
    BASEMPMIND,
    THROUGHMPMIND,
    RULETARIFF,
    THROUGHRULE,
    RULEEXCLUDEIND,
    GATEWAYZONE,
    INTERIORZONE,
    CLASSFAREBASISIND,
    GLOBALCLASSFLAG,
    ROUTECODE,
    TARIFFFAMILY,
    FAREQUALIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                  " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                  " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                  " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                  " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                  " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                  " TARIFFFAMILY,FAREQUALIND");
    this->From("=ADDONFARE");
    this->Where("INTERIORMARKET = %1q "
                " and CARRIER = %2q"
                " and %cd <= DISCDATE"
                " and %cd <= EXPIREDATE"
                " and EFFDATE <= DISCDATE"
                " and VALIDITYIND = 'Y' ");

    if (DataManager::forceSortOrder())
      this->OrderBy("GATEWAYMARKET, INTERIORMARKET, CARRIER, FARECLASS, ADDONTARIFF, VENDOR, CUR, "
                    "LINKNO, SEQNO, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static AddonFareInfo* mapRowToAddonFareInfo(Row* row)
  {
    tse::AddonFareInfo* af;
    VendorCode vendor = row->getString(VENDOR);
    if (vendor.equalToConst("SITA"))
      af = new tse::SITAAddonFareInfo;
    else
      af = new tse::AddonFareInfo;

    af->gatewayMarket() = row->getString(GATEWAYMARKET);
    af->interiorMarket() = row->getString(INTERIORMARKET);
    af->carrier() = row->getString(CARRIER);
    af->fareClass() = row->getString(FARECLASS);
    af->addonTariff() = row->getInt(ADDONTARIFF);
    af->vendor() = vendor;
    af->cur() = row->getString(CUR);
    af->createDate() = row->getDate(CREATEDATE);
    af->expireDate() = row->getDate(EXPIREDATE);
    af->effDate() = row->getDate(EFFDATE);
    af->discDate() = row->getDate(DISCDATE);
    af->noDec() = row->getInt(NODEC);
    af->fareAmt() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT), af->noDec());
    long long int hashkey = row->getLongLong(HASHKEY);
    char buf[64];
    memset(buf, 0, 64);
    buf[63] = 0;
    snprintf(buf, 63, "%lld", hashkey);
    af->hashKey() = buf;
    af->footNote1() = row->getString(FOOTNOTE1);
    af->footNote2() = row->getString(FOOTNOTE2);
    af->owrt() = row->getChar(OWRT);
    af->routing() = row->getString(ROUTING);
    af->directionality() = row->getChar(DIRECTIONALITY);
    af->arbZone() = row->getInt(ARBZONE);
    af->inhibit() = row->getChar(INHIBIT);

    if (vendor != "SITA")
      return af;

    ////////////////////////////// Must be SITA! //////////////////////////////
    SITAAddonFareInfo* saf = (SITAAddonFareInfo*)af;

    saf->baseFareRouting() = row->getString(BASEFAREROUTING);
    saf->thruFareRouting() = row->getString(THRUFAREROUTING);
    saf->baseMPMInd() = row->getChar(BASEMPMIND);
    saf->throughMPMInd() = row->getChar(THROUGHMPMIND);
    saf->ruleTariff() = row->getInt(RULETARIFF);
    saf->throughRule() = row->getString(THROUGHRULE);
    saf->ruleExcludeInd() = row->getChar(RULEEXCLUDEIND);
    saf->gatewayZone() = row->getInt(GATEWAYZONE);
    saf->interiorZone() = row->getInt(INTERIORZONE);
    saf->classFareBasisInd() = row->getChar(CLASSFAREBASISIND);
    saf->globalClassFlag() = row->getChar(GLOBALCLASSFLAG);
    saf->routeCode() = row->getString(ROUTECODE);
    saf->tariffFamily() = row->getChar(TARIFFFAMILY);
    saf->fareQualInd() = row->getChar(FAREQUALIND);

    return saf;
  } // mapRowToAddonFareInfo

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonFaresHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAddonFaresHistoricalSQLStatement : public QueryGetAddonFaresSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                             " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                             " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                             " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                             " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                             " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                             " TARIFFFAMILY,FAREQUALIND,LINKNO,SEQNO");
    partialStatement.From("=ADDONFAREH");
    partialStatement.Where("INTERIORMARKET = %1q "
                           " and CARRIER = %2q"
                           " and %3n <= EXPIREDATE"
                           " and (   %4n >=  CREATEDATE"
                           "      or %5n >= EFFDATE)"
                           " and EFFDATE <= DISCDATE"
                           " and VALIDITYIND = 'Y' "
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                             " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                             " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                             " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                             " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                             " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                             " TARIFFFAMILY,FAREQUALIND,LINKNO,SEQNO");
    partialStatement.From("=ADDONFARE");
    partialStatement.Where("INTERIORMARKET = %6q "
                           " and CARRIER = %7q"
                           " and %8n <= EXPIREDATE"
                           " and (   %9n >=  CREATEDATE"
                           "      or %10n >= EFFDATE)"
                           " and EFFDATE <= DISCDATE"
                           " and VALIDITYIND = 'Y' "
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");

    // if ( DataManager::forceSortOrder() )
    //    this->OrderBy("GATEWAYMARKET, INTERIORMARKET, CARRIER, FARECLASS, ADDONTARIFF, VENDOR,
    // CUR, LINKNO, SEQNO, CREATEDATE");
    // else
    //    this->OrderBy("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetAddonFaresGW
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAddonFaresGWSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAddonFaresGWSQLStatement() {};
  virtual ~QueryGetAddonFaresGWSQLStatement() {};

  enum ColumnIndexes
  {
    GATEWAYMARKET = 0,
    INTERIORMARKET,
    CARRIER,
    FARECLASS,
    ADDONTARIFF,
    VENDOR,
    CUR,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NODEC,
    FAREAMT,
    HASHKEY,
    FOOTNOTE1,
    FOOTNOTE2,
    OWRT,
    ROUTING,
    DIRECTIONALITY,
    ARBZONE,
    INHIBIT,
    BASEFAREROUTING,
    THRUFAREROUTING,
    BASEMPMIND,
    THROUGHMPMIND,
    RULETARIFF,
    THROUGHRULE,
    RULEEXCLUDEIND,
    GATEWAYZONE,
    INTERIORZONE,
    CLASSFAREBASISIND,
    GLOBALCLASSFLAG,
    ROUTECODE,
    TARIFFFAMILY,
    FAREQUALIND,
    LASTMODDATE,
    GLOBALDIR,
    LINKNO,
    SEQNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                  " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                  " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                  " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                  " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                  " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                  " TARIFFFAMILY,FAREQUALIND, LASTMODDATE, GLOBALDIR, LINKNO, SEQNO");
    this->From("=ADDONFARE");
    this->Where("GATEWAYMARKET = %1q "
                " and INTERIORMARKET = %2q "
                " and CARRIER = %3q"
                " and %cd <= DISCDATE"
                " and %cd <= EXPIREDATE"
                " and EFFDATE <= DISCDATE"
                " and VALIDITYIND = 'Y' ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static AddonFareInfo* mapRowToAddonFareInfoGW(Row* row)
  {
    tse::AddonFareInfo* af;
    VendorCode vendor = row->getString(VENDOR);
    if (vendor.equalToConst("SITA"))
      af = new tse::SITAAddonFareInfo;
    else
      af = new tse::AddonFareInfo;

    af->gatewayMarket() = row->getString(GATEWAYMARKET);
    af->interiorMarket() = row->getString(INTERIORMARKET);
    af->carrier() = row->getString(CARRIER);
    af->fareClass() = row->getString(FARECLASS);
    af->addonTariff() = row->getInt(ADDONTARIFF);
    af->vendor() = vendor;
    af->cur() = row->getString(CUR);
    af->createDate() = row->getDate(CREATEDATE);
    af->expireDate() = row->getDate(EXPIREDATE);
    af->effDate() = row->getDate(EFFDATE);
    af->discDate() = row->getDate(DISCDATE);
    af->noDec() = row->getInt(NODEC);
    af->fareAmt() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT), af->noDec());
    long long int hashkey = row->getLongLong(HASHKEY);
    char buf[64];
    memset(buf, 0, 64);
    buf[63] = 0;
    snprintf(buf, 63, "%lld", hashkey);
    af->hashKey() = buf;
    af->footNote1() = row->getString(FOOTNOTE1);
    af->footNote2() = row->getString(FOOTNOTE2);
    af->owrt() = row->getChar(OWRT);
    af->routing() = row->getString(ROUTING);
    af->directionality() = row->getChar(DIRECTIONALITY);
    af->arbZone() = row->getInt(ARBZONE);
    af->inhibit() = row->getChar(INHIBIT);

    af->lastModDate() = row->getDate(LASTMODDATE);
    af->classFareBasisInd() = row->getChar(CLASSFAREBASISIND);
    strToGlobalDirection(af->globalDir(), row->getString(GLOBALDIR));
    af->routeCode() = row->getString(ROUTECODE);
    af->linkNo() = row->getInt(LINKNO);
    af->seqNo() = row->getLong(SEQNO);

    if (vendor != "SITA")
      return af;

    ////////////////////////////// Must be SITA! //////////////////////////////
    SITAAddonFareInfo* saf = (SITAAddonFareInfo*)af;

    saf->baseFareRouting() = row->getString(BASEFAREROUTING);
    saf->thruFareRouting() = row->getString(THRUFAREROUTING);
    saf->baseMPMInd() = row->getChar(BASEMPMIND);
    saf->throughMPMInd() = row->getChar(THROUGHMPMIND);
    saf->ruleTariff() = row->getInt(RULETARIFF);
    saf->throughRule() = row->getString(THROUGHRULE);
    saf->ruleExcludeInd() = row->getChar(RULEEXCLUDEIND);
    saf->gatewayZone() = row->getInt(GATEWAYZONE);
    saf->interiorZone() = row->getInt(INTERIORZONE);
    saf->classFareBasisInd() = row->getChar(CLASSFAREBASISIND);
    saf->globalClassFlag() = row->getChar(GLOBALCLASSFLAG);
    saf->routeCode() = row->getString(ROUTECODE);
    saf->tariffFamily() = row->getChar(TARIFFFAMILY);
    saf->fareQualInd() = row->getChar(FAREQUALIND);

    return saf;
  } // mapRowToAddonFareInfoGW

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonFaresGWHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAddonFaresGWHistoricalSQLStatement
    : public QueryGetAddonFaresGWSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                             " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                             " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                             " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                             " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                             " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                             " TARIFFFAMILY,FAREQUALIND,LASTMODDATE,GLOBALDIR,LINKNO,SEQNO");
    partialStatement.From("=ADDONFAREH");
    partialStatement.Where("GATEWAYMARKET = %1q "
                           " and INTERIORMARKET = %2q "
                           " and CARRIER = %3q"
                           " and %4n <= EXPIREDATE"
                           " and (   %5n >=  CREATEDATE"
                           "      or %6n >= EFFDATE)"
                           " and EFFDATE <= DISCDATE"
                           " and VALIDITYIND = 'Y' "
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select GATEWAYMARKET,INTERIORMARKET,CARRIER,FARECLASS,ADDONTARIFF,"
                             " VENDOR,CUR,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NODEC,"
                             " FAREAMT,HASHKEY,FOOTNOTE1,FOOTNOTE2,OWRT,ROUTING,DIRECTIONALITY,"
                             " ARBZONE,INHIBIT,BASEFAREROUTING,THRUFAREROUTING,BASEMPMIND,"
                             " THROUGHMPMIND,RULETARIFF,THROUGHRULE,RULEEXCLUDEIND,GATEWAYZONE,"
                             " INTERIORZONE,CLASSFAREBASISIND,GLOBALCLASSFLAG,ROUTECODE,"
                             " TARIFFFAMILY,FAREQUALIND,LASTMODDATE,GLOBALDIR,LINKNO,SEQNO");
    partialStatement.From("=ADDONFARE");
    partialStatement.Where("GATEWAYMARKET = %7q "
                           " and INTERIORMARKET = %8q "
                           " and CARRIER = %9q"
                           " and %10n <= EXPIREDATE"
                           " and (   %11n >=  CREATEDATE"
                           "      or %12n >= EFFDATE)"
                           " and EFFDATE <= DISCDATE"
                           " and VALIDITYIND = 'Y' "
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
// QueryGetSitaAddonQualCodes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonQualCodesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSitaAddonQualCodesSQLStatement() {};
  virtual ~QueryGetSitaAddonQualCodesSQLStatement() {};

  enum ColumnIndexes
  {
    FAREQUALCODE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FAREQUALCODE");
    this->From("=SITAADDONFAREQUALCD");
    this->Where("GATEWAYMARKET = %1q "
                "   and INTERIORMARKET = %2q"
                "   and HASHKEY = %3n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static char mapRowToChar(Row* row) { return row->getChar(FAREQUALCODE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetSitaAddonQualCodesHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonQualCodesHistoricalSQLStatement
    : public QueryGetSitaAddonQualCodesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select FAREQUALCODE");
    partialStatement.From("=SITAADDONFAREQUALCDH");
    partialStatement.Where("GATEWAYMARKET = %1q "
                           "   and INTERIORMARKET = %2q"
                           "   and HASHKEY = %3n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select FAREQUALCODE");
    partialStatement.From("=SITAADDONFAREQUALCD");
    partialStatement.Where("GATEWAYMARKET = %4q "
                           "   and INTERIORMARKET = %5q"
                           "   and HASHKEY = %6n"
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
// QueryGetSitaAddonDBEClasses
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonDBEClassesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSitaAddonDBEClassesSQLStatement() {};
  virtual ~QueryGetSitaAddonDBEClassesSQLStatement() {};

  enum ColumnIndexes
  {
    DBECLASS = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select DBECLASS");
    this->From("=SITAADDONDBECLASS");
    this->Where("GATEWAYMARKET = %1q "
                "   and INTERIORMARKET = %2q"
                "   and HASHKEY = %3n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const char* mapRowToString(Row* row) { return row->getString(DBECLASS); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetSitaAddonDBEClassesHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonDBEClassesHistoricalSQLStatement
    : public QueryGetSitaAddonDBEClassesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select DBECLASS");
    partialStatement.From("=SITAADDONDBECLASSH");
    partialStatement.Where("GATEWAYMARKET = %1q "
                           "   and INTERIORMARKET = %2q"
                           "   and HASHKEY = %3n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select DBECLASS");
    partialStatement.From("=SITAADDONDBECLASS");
    partialStatement.Where("GATEWAYMARKET = %4q "
                           "   and INTERIORMARKET = %5q"
                           "   and HASHKEY = %6n"
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
// QueryGetSitaAddonRules
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonRulesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSitaAddonRulesSQLStatement() {};
  virtual ~QueryGetSitaAddonRulesSQLStatement() {};

  enum ColumnIndexes
  {
    RULE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select RULE");
    this->From("=SITAADDONRULE");
    this->Where("GATEWAYMARKET = %1q "
                "   and INTERIORMARKET = %2q"
                "   and HASHKEY = %3n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static const char* mapRowToString(Row* row) { return row->getString(RULE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetSitaAddonRulesHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSitaAddonRulesHistoricalSQLStatement
    : public QueryGetSitaAddonRulesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select RULE");
    partialStatement.From("=SITAADDONRULEH");
    partialStatement.Where("GATEWAYMARKET = %1q "
                           "   and INTERIORMARKET = %2q"
                           "   and HASHKEY = %3n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select RULE");
    partialStatement.From("=SITAADDONRULE");
    partialStatement.Where("GATEWAYMARKET = %4q "
                           "   and INTERIORMARKET = %5q"
                           "   and HASHKEY = %6n"
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
