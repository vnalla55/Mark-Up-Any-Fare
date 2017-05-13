//----------------------------------------------------------------------------
//          File:           QueryGetMinFareRuleSQLStatement.h
//          Description:    QueryGetMinFareRuleSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetMinFareRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMinFareRuleLevExclBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareRuleLevExclBaseSQLStatement() {};
  virtual ~QueryGetMinFareRuleLevExclBaseSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    TEXTTBLITEMNO,
    GOVERNINGCARRIER,
    RULETARIFF,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    TARIFFCAT,
    ROUTINGTARIFF1,
    ROUTINGTARIFF2,
    RULETARIFFCODE,
    USERAPPLTYPE,
    USERAPPL,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    MPMIND,
    ROUTINGIND,
    ROUTING,
    HIPMINFAREAPPL,
    HIPFARECOMPAPPL,
    HIPSAMEGROUPAPPL,
    CTMMINFAREAPPL,
    CTMFARECOMPAPPL,
    CTMSAMEGROUPAPPL,
    BACKHAULMINFAREAPPL,
    BACKHAULFARECOMPAPPL,
    BACKHAULSAMEGROUPAPPL,
    DMCMINFAREAPPL,
    DMCFARECOMPAPPL,
    DMCSAMEGROUPAPPL,
    COMMINFAREAPPL,
    COMFARECOMPAPPL,
    COMSAMEGROUPAPPL,
    COPMINFAREAPPL,
    COPFARECOMPAPPL,
    COPSAMEGROUPAPPL,
    CPMMINFAREAPPL,
    CPMFARECOMPAPPL,
    CPMSAMEGROUPAPPL,
    OSCFARECOMPAPPL,
    OSCSAMEGROUPAPPL,
    RSCFARECOMPAPPL,
    RSCSAMEGROUPAPPL,
    RULE,
    FOOTNOTE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select r.VENDOR,r.TEXTTBLITEMNO,r.GOVERNINGCARRIER,r.RULETARIFF,"
        " r.VERSIONDATE,r.SEQNO,r.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
        " TARIFFCAT,ROUTINGTARIFF1,ROUTINGTARIFF2,RULETARIFFCODE,USERAPPLTYPE,"
        " USERAPPL,DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,GLOBALDIR,MPMIND,"
        " ROUTINGIND,ROUTING,HIPMINFAREAPPL,HIPFARECOMPAPPL,HIPSAMEGROUPAPPL,CTMMINFAREAPPL,"
        " CTMFARECOMPAPPL,CTMSAMEGROUPAPPL,BACKHAULMINFAREAPPL,BACKHAULFARECOMPAPPL,"
        " BACKHAULSAMEGROUPAPPL,DMCMINFAREAPPL,DMCFARECOMPAPPL,DMCSAMEGROUPAPPL,COMMINFAREAPPL,"
        " COMFARECOMPAPPL,COMSAMEGROUPAPPL,COPMINFAREAPPL,COPFARECOMPAPPL,COPSAMEGROUPAPPL,"
        "CPMMINFAREAPPL,"
        " CPMFARECOMPAPPL,CPMSAMEGROUPAPPL,OSCFARECOMPAPPL,OSCSAMEGROUPAPPL,RSCFARECOMPAPPL,"
        "RSCSAMEGROUPAPPL,rr.RULE, rr.FOOTNOTE");

    //		        this->From("=EXCLUDEBYRULE r LEFT OUTER JOIN =EXCLBYRULERULE rr"
    //		                   "   USING (VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,RULETARIFF,"
    //		                   "          VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("VENDOR");
    joinFields.push_back("TEXTTBLITEMNO");
    joinFields.push_back("GOVERNINGCARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=EXCLUDEBYRULE", "r", "LEFT OUTER JOIN", "=EXCLBYRULERULE", "rr", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("r.VENDOR = %1q"
                " and r.TEXTTBLITEMNO in (0, %2n)"
                " and r.GOVERNINGCARRIER in ('', %3q)"
                " and r.RULETARIFF in (-1, %4n)"
                " and %cd <= EXPIREDATE");
    this->OrderBy("VENDOR,TEXTTBLITEMNO desc,GOVERNINGCARRIER desc,RULETARIFF desc, SEQNO, RULE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MinFareRuleLevelExcl*
  mapRowToMinFareRuleLevelExclBase(Row* row, MinFareRuleLevelExcl* rlePrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    int textTblItemNo = row->getInt(TEXTTBLITEMNO);
    CarrierCode governingCarrier = row->getString(GOVERNINGCARRIER);
    TariffNumber ruleTariff = row->getInt(RULETARIFF);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    MinFareRuleLevelExcl* rle;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (rlePrev != nullptr && rlePrev->vendor() == vendor && rlePrev->textTblItemNo() == textTblItemNo &&
        rlePrev->governingCarrier() == governingCarrier && rlePrev->ruleTariff() == ruleTariff &&
        rlePrev->versionDate() == versionDate && rlePrev->seqNo() == seqNo &&
        rlePrev->createDate() == createDate)
    { // Just add to Prev
      rle = rlePrev;
    }
    else
    { // Time for a new Parent
      rle = new tse::MinFareRuleLevelExcl;
      rle->vendor() = vendor;
      rle->textTblItemNo() = textTblItemNo;
      rle->governingCarrier() = governingCarrier;
      rle->ruleTariff() = ruleTariff;
      rle->versionDate() = versionDate;
      rle->seqNo() = seqNo;
      rle->createDate() = createDate;
      rle->expireDate() = row->getDate(EXPIREDATE);
      rle->effDate() = row->getDate(EFFDATE);
      rle->discDate() = row->getDate(DISCDATE);
      rle->tariffCat() = row->getInt(TARIFFCAT);
      rle->ruleTariffCode() = row->getString(RULETARIFFCODE);
      rle->routingTariff1() = row->getInt(ROUTINGTARIFF1);
      rle->routingTariff2() = row->getInt(ROUTINGTARIFF2);
      rle->userApplType() = row->getChar(USERAPPLTYPE);
      rle->userAppl() = row->getString(USERAPPL);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(rle->globalDir(), gd);

      std::string dir = row->getString(DIRECTIONALITY);
      if (dir == "F")
        rle->directionality() = FROM;
      else if (dir == "W")
        rle->directionality() = WITHIN;
      else if (dir == "O")
        rle->directionality() = ORIGIN;
      else if (dir == "X")
        rle->directionality() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        rle->directionality() = BETWEEN;

      LocKey* loc = &rle->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &rle->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      rle->mpmInd() = row->getChar(MPMIND);
      rle->routingInd() = row->getChar(ROUTINGIND);
      rle->routing() = row->getString(ROUTING);
      rle->hipMinFareAppl() = row->getChar(HIPMINFAREAPPL);
      rle->hipFareCompAppl() = row->getChar(HIPFARECOMPAPPL);
      rle->hipSameGroupAppl() = row->getChar(HIPSAMEGROUPAPPL);
      rle->ctmMinFareAppl() = row->getChar(CTMMINFAREAPPL);
      rle->ctmFareCompAppl() = row->getChar(CTMFARECOMPAPPL);
      rle->ctmSameGroupAppl() = row->getChar(CTMSAMEGROUPAPPL);
      rle->backhaulMinFareAppl() = row->getChar(BACKHAULMINFAREAPPL);
      rle->backhaulFareCompAppl() = row->getChar(BACKHAULFARECOMPAPPL);
      rle->backhaulSameGroupAppl() = row->getChar(BACKHAULSAMEGROUPAPPL);
      rle->dmcMinFareAppl() = row->getChar(DMCMINFAREAPPL);
      rle->dmcFareCompAppl() = row->getChar(DMCFARECOMPAPPL);
      rle->dmcSameGroupAppl() = row->getChar(DMCSAMEGROUPAPPL);
      rle->comMinFareAppl() = row->getChar(COMMINFAREAPPL);
      rle->comFareCompAppl() = row->getChar(COMFARECOMPAPPL);
      rle->comSameGroupAppl() = row->getChar(COMSAMEGROUPAPPL);
      rle->copMinFareAppl() = row->getChar(COPMINFAREAPPL);
      rle->copFareCompAppl() = row->getChar(COPFARECOMPAPPL);
      rle->copSameGroupAppl() = row->getChar(COPSAMEGROUPAPPL);
      rle->cpmMinFareAppl() = row->getChar(CPMMINFAREAPPL);
      rle->cpmFareCompAppl() = row->getChar(CPMFARECOMPAPPL);
      rle->cpmSameGroupAppl() = row->getChar(CPMSAMEGROUPAPPL);
      rle->oscFareCompAppl() = row->getChar(OSCFARECOMPAPPL);
      rle->oscSameGroupAppl() = row->getChar(OSCSAMEGROUPAPPL);
      rle->rscFareCompAppl() = row->getChar(RSCFARECOMPAPPL);
      rle->rscSameGroupAppl() = row->getChar(RSCSAMEGROUPAPPL);
    }

    // Add new Rule & return
    if (!row->isNull(RULE))
    {
      RuleNumber newRule = row->getString(RULE);
      Footnote newFootnote = (!row->isNull(FOOTNOTE)) ? row->getString(FOOTNOTE) : "";
      rle->ruleFootnotes().push_back(std::make_pair(newRule, newFootnote));
    }

    return rle;
  } // mapRowToMinFareRuleLevelExclBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareRuleLevExclBaseHistorical
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRuleLevExclBaseHistoricalSQLStatement
    : public QueryGetMinFareRuleLevExclBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("VENDOR");
    joinFields.push_back("TEXTTBLITEMNO");
    joinFields.push_back("GOVERNINGCARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");

    partialStatement.Command(
        "("
        "select r.VENDOR,r.TEXTTBLITEMNO,r.GOVERNINGCARRIER,r.RULETARIFF,"
        " r.VERSIONDATE,r.SEQNO,r.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
        " TARIFFCAT,ROUTINGTARIFF1,ROUTINGTARIFF2,RULETARIFFCODE,USERAPPLTYPE,"
        " USERAPPL,DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,GLOBALDIR,MPMIND,"
        " ROUTINGIND,ROUTING,HIPMINFAREAPPL,HIPFARECOMPAPPL,HIPSAMEGROUPAPPL,CTMMINFAREAPPL,"
        " CTMFARECOMPAPPL,CTMSAMEGROUPAPPL,BACKHAULMINFAREAPPL,BACKHAULFARECOMPAPPL,"
        "BACKHAULSAMEGROUPAPPL,"
        " DMCMINFAREAPPL,DMCFARECOMPAPPL,DMCSAMEGROUPAPPL,COMMINFAREAPPL,COMFARECOMPAPPL,"
        "COMSAMEGROUPAPPL,"
        " COPMINFAREAPPL,COPFARECOMPAPPL,COPSAMEGROUPAPPL,CPMMINFAREAPPL,CPMFARECOMPAPPL,"
        "CPMSAMEGROUPAPPL,"
        " OSCFARECOMPAPPL,OSCSAMEGROUPAPPL,RSCFARECOMPAPPL,RSCSAMEGROUPAPPL,rr.RULE, rr.FOOTNOTE");
    // partialStatement.From(     "=EXCLUDEBYRULEH r LEFT OUTER JOIN =EXCLBYRULERULEH rr"
    //                           "   USING (VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,RULETARIFF,"
    //                           "          VERSIONDATE,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=EXCLUDEBYRULEH",
                             "r",
                             "LEFT OUTER JOIN",
                             "=EXCLBYRULERULEH",
                             "rr",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r.VENDOR = %1q"
                           " and r.TEXTTBLITEMNO in (0, %2n)"
                           " and r.GOVERNINGCARRIER in ('', %3q)"
                           " and r.RULETARIFF in (-1, %4n)"
                           " and %5n <= r.EXPIREDATE"
                           " and (   %6n >=  r.CREATEDATE"
                           "      or %7n >= r.EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    from.clear();
    partialStatement.Command(
        " union all"
        " ("
        "select r.VENDOR,r.TEXTTBLITEMNO,r.GOVERNINGCARRIER,r.RULETARIFF,"
        " r.VERSIONDATE,r.SEQNO,r.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
        " TARIFFCAT,ROUTINGTARIFF1,ROUTINGTARIFF2,RULETARIFFCODE,USERAPPLTYPE,"
        " USERAPPL,DIRECTIONALITY,LOC1TYPE,LOC1,LOC2TYPE,LOC2,GLOBALDIR,MPMIND,"
        " ROUTINGIND,ROUTING,HIPMINFAREAPPL,HIPFARECOMPAPPL,HIPSAMEGROUPAPPL,CTMMINFAREAPPL,"
        " CTMFARECOMPAPPL,CTMSAMEGROUPAPPL,BACKHAULMINFAREAPPL,BACKHAULFARECOMPAPPL,"
        "BACKHAULSAMEGROUPAPPL,"
        " DMCMINFAREAPPL,DMCFARECOMPAPPL,DMCSAMEGROUPAPPL,COMMINFAREAPPL,COMFARECOMPAPPL,"
        "COMSAMEGROUPAPPL,"
        " COPMINFAREAPPL,COPFARECOMPAPPL,COPSAMEGROUPAPPL,CPMMINFAREAPPL,CPMFARECOMPAPPL,"
        "CPMSAMEGROUPAPPL,"
        " OSCFARECOMPAPPL,OSCSAMEGROUPAPPL,RSCFARECOMPAPPL,RSCSAMEGROUPAPPL,rr.RULE, rr.FOOTNOTE");
    // partialStatement.From(     "=EXCLUDEBYRULE r LEFT OUTER JOIN =EXCLBYRULERULE rr"
    //                           "   USING (VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,RULETARIFF,"
    //                           "          VERSIONDATE,SEQNO,CREATEDATE)");
    this->generateJoinString(partialStatement,
                             "=EXCLUDEBYRULE",
                             "r",
                             "LEFT OUTER JOIN",
                             "=EXCLBYRULERULE",
                             "rr",
                             joinFields,
                             from);
    partialStatement.From(from);
    partialStatement.Where("r.VENDOR = %8q"
                           " and r.TEXTTBLITEMNO in (0, %9n)"
                           " and r.GOVERNINGCARRIER in ('', %10q)"
                           " and r.RULETARIFF in (-1, %11n)"
                           " and %12n <= r.EXPIREDATE"
                           " and (   %13n >=  r.CREATEDATE"
                           "      or %14n >= r.EFFDATE)"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("VENDOR,TEXTTBLITEMNO desc,GOVERNINGCARRIER desc,RULETARIFF desc, VERSIONDATE, "
                  "SEQNO, CREATEDATE, RULE");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetMinFareRuleLevExclBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareRuleFareClasses
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRuleFareClassesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareRuleFareClassesSQLStatement() {};
  virtual ~QueryGetMinFareRuleFareClassesSQLStatement() {};

  enum ColumnIndexes
  {
    FARECLASS = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARECLASS");
    this->From("=EXCLBYRULEFARECLASS");
    this->Where("VENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and RULETARIFF = %4n"
                " and VERSIONDATE = %5n"
                " and SEQNO = %6n"
                " and CREATEDATE = %7n");

    if (DataManager::forceSortOrder())
      this->OrderBy("FARECLASS");
    //         else
    //            this->OrderBy("FARECLASS");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareClassCode mapRowToFareClass(Row* row) { return row->getString(FARECLASS); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareRuleFareClassesSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareRuleFareClassesHistorical
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRuleFareClassesHistoricalSQLStatement
    : public QueryGetMinFareRuleFareClassesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select FARECLASS");
    partialStatement.From("=EXCLBYRULEFARECLASSH");
    partialStatement.Where("VENDOR = %1q"
                           " and TEXTTBLITEMNO = %2n"
                           " and GOVERNINGCARRIER = %3q"
                           " and RULETARIFF = %4n"
                           " and VERSIONDATE = %5n"
                           " and SEQNO = %6n"
                           " and CREATEDATE = %7n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select FARECLASS");
    partialStatement.From("=EXCLBYRULEFARECLASS");
    partialStatement.Where("VENDOR = %8q"
                           " and TEXTTBLITEMNO = %9n"
                           " and GOVERNINGCARRIER = %10q"
                           " and RULETARIFF = %11n"
                           " and VERSIONDATE = %12n"
                           " and SEQNO = %13n"
                           " and CREATEDATE = %14n"
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
// SQL for QueryGetMinFareRuleFareTypes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRuleFareTypesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareRuleFareTypesSQLStatement() {};
  virtual ~QueryGetMinFareRuleFareTypesSQLStatement() {};

  enum ColumnIndexes
  {
    FARETYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARETYPE");
    this->From("=EXCLBYRULEFARETYPE");
    this->Where("VENDOR = %1q"
                "   and TEXTTBLITEMNO = %2n"
                "   and GOVERNINGCARRIER = %3q"
                "   and RULETARIFF = %4n"
                "   and VERSIONDATE = %5n"
                "   and SEQNO = %6n"
                "   and CREATEDATE = %7n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareTypeAbbrev mapRowToFareType(Row* row) { return row->getString(FARETYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRuleFareTypesHistoricalSQLStatement
    : public QueryGetMinFareRuleFareTypesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select FARETYPE");
    partialStatement.From("=EXCLBYRULEFARETYPEH");
    partialStatement.Where("VENDOR = %1q"
                           " and TEXTTBLITEMNO = %2n"
                           " and GOVERNINGCARRIER = %3q"
                           " and RULETARIFF = %4n"
                           " and VERSIONDATE = %5n"
                           " and SEQNO = %6n"
                           " and CREATEDATE = %7n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select FARETYPE");
    partialStatement.From("=EXCLBYRULEFARETYPE");
    partialStatement.Where("VENDOR = %8q"
                           " and TEXTTBLITEMNO = %9n"
                           " and GOVERNINGCARRIER = %10q"
                           " and RULETARIFF = %11n"
                           " and VERSIONDATE = %12n"
                           " and SEQNO = %13n"
                           " and CREATEDATE = %14n"
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
// SQL for QueryGetMinFareRuleSameFareGroupChild
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMinFareRuleSameFareGroupChildSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareRuleSameFareGroupChildSQLStatement() {};
  virtual ~QueryGetMinFareRuleSameFareGroupChildSQLStatement() {};

  enum ColumnIndexes
  {
    SETNO = 0,
    FAREGROUPRULETARIFF,
    RULE,
    FARETYPE,
    FARECLASS,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SETNO, FAREGROUPRULETARIFF,RULE,FARETYPE,FARECLASS");
    this->From("=EXCLBYRULESAMEGRPFAREQUAL");
    this->Where("VENDOR = %1q"
                "   and TEXTTBLITEMNO = %2n"
                "   and GOVERNINGCARRIER = %3q"
                "   and RULETARIFF = %4n"
                "   and VERSIONDATE = %5n"
                "   and SEQNO = %6n"
                "   and CREATEDATE = %7n");
    this->OrderBy("SETNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SetNumber mapRowToSetNumber(Row* row) { return row->getInt(SETNO); }

  static TariffNumber mapRowToSameFareGroupTariff(Row* row)
  {
    return row->getInt(FAREGROUPRULETARIFF);
  }

  static RuleNumber mapRowToSameFareGroupRules(Row* row) { return row->getString(RULE); }

  static FareTypeAbbrev mapRowToSameFareGroupFareTypes(Row* row)
  {
    return row->getString(FARETYPE);
  }

  static FareClassCode mapRowToSameFareGroupFareClasses(Row* row)
  {
    return row->getString(FARECLASS);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace From clause
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMinFareRuleSameFareGroupChildHistoricalSQLStatement
    : public QueryGetMinFareRuleSameFareGroupChildSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select SETNO, FAREGROUPRULETARIFF,RULE,FARETYPE,FARECLASS");
    partialStatement.From("=EXCLBYRULESAMEGRPFAREQUALH");
    partialStatement.Where("VENDOR = %1q"
                           " and TEXTTBLITEMNO = %2n"
                           " and GOVERNINGCARRIER = %3q"
                           " and RULETARIFF = %4n"
                           " and VERSIONDATE = %5n"
                           " and SEQNO = %6n"
                           " and CREATEDATE = %7n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();

    partialStatement.Command(" union all"
                             " ("
                             "select SETNO, FAREGROUPRULETARIFF,RULE,FARETYPE,FARECLASS");
    partialStatement.From("=EXCLBYRULESAMEGRPFAREQUAL");
    partialStatement.Where("VENDOR = %8q"
                           " and TEXTTBLITEMNO = %9n"
                           " and GOVERNINGCARRIER = %10q"
                           " and RULETARIFF = %11n"
                           " and VERSIONDATE = %12n"
                           " and SEQNO = %13n"
                           " and CREATEDATE = %14n"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("SETNO");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

} // tse
