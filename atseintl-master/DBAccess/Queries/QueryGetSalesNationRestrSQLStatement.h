//----------------------------------------------------------------------------
//          File:           QueryGetSalesNationRestrSQLStatement.h
//          Description:    QueryGetSalesNationRestrSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetSalesNationRestr.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSalesNationRestrBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSalesNationRestrBaseSQLStatement() {};
  virtual ~QueryGetSalesNationRestrBaseSQLStatement() {};

  enum ColumnIndexes
  {
    NATION = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FIRSTTVLDATE,
    LASTTVLDATE,
    MEMONO,
    NEWSEQNO,
    SPECIALPROCESSNO,
    RESTRICTION,
    VENDOR,
    USERAPPLTYPE,
    USERAPPL,
    EXCEPTGOVERNINGCXR,
    EXCEPTTICKETINGCXR,
    DIRECTIONALITY,
    EXCEPTLOC,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    TRAVELTYPE,
    VIALOCTYPE,
    VIALOC,
    CURRESTRAPPL,
    POSEXCEPTIND,
    POSLOCTYPE,
    POSLOC,
    POIEXCEPTIND,
    POILOCTYPE,
    POILOC,
    CUR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select s.NATION,s.VERSIONDATE,s.SEQNO,s.CREATEDATE,EXPIREDATE,EFFDATE,"
                  "       DISCDATE,FIRSTTVLDATE,LASTTVLDATE,MEMONO,NEWSEQNO,SPECIALPROCESSNO,"
                  "       RESTRICTION,VENDOR,USERAPPLTYPE,USERAPPL,EXCEPTGOVERNINGCXR,"
                  "       EXCEPTTICKETINGCXR,DIRECTIONALITY,EXCEPTLOC,LOC1TYPE,LOC1,LOC2TYPE,"
                  "       LOC2,GLOBALDIR,TRAVELTYPE,VIALOCTYPE,VIALOC,CURRESTRAPPL,POSEXCEPTIND,"
                  "       POSLOCTYPE,POSLOC,POIEXCEPTIND,POILOCTYPE,POILOC,c.CUR");

    //		        this->From("=SALESNATIONRESTR s LEFT OUTER JOIN =SALESNATRESTRCURRSTR c"
    //		                   "                            USING
    //(NATION,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("NATION");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=SALESNATIONRESTR",
                             "s",
                             "LEFT OUTER JOIN",
                             "=SALESNATRESTRCURRSTR",
                             "c",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("s.NATION = %1q and %cd <= EXPIREDATE");
    //"    and EXPIREDATE > 0"

    if (DataManager::forceSortOrder())
      this->OrderBy("s.NATION, s.VERSIONDATE, s.SEQNO, s.CREATEDATE, c.CUR");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SalesNationRestr* mapRowToSalesNationRestrBase(Row* row, SalesNationRestr* snrPrev)
  { // Load up Parent Determinant Fields
    NationCode nation = row->getString(NATION);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    SalesNationRestr* snr;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (snrPrev != nullptr && snrPrev->nation() == nation && snrPrev->versionDate() == versionDate &&
        snrPrev->seqNo() == seqNo && snrPrev->createDate() == createDate)
    { // Just add to Prev
      snr = snrPrev;
    }
    else
    { // Time for a new Parent
      snr = new tse::SalesNationRestr;
      snr->nation() = nation;
      snr->versionDate() = versionDate;
      snr->seqNo() = seqNo;
      snr->createDate() = createDate;

      snr->expireDate() = row->getDate(EXPIREDATE);
      snr->effDate() = row->getDate(EFFDATE);
      snr->discDate() = row->getDate(DISCDATE);
      snr->firstTvlDate() = row->getDate(FIRSTTVLDATE);
      snr->lastTvlDate() = row->getDate(LASTTVLDATE);
      snr->specialProcessNo() = row->getInt(SPECIALPROCESSNO);
      snr->restriction() = row->getChar(RESTRICTION);
      snr->vendor() = row->getString(VENDOR);
      snr->userApplType() = row->getChar(USERAPPLTYPE);
      snr->userAppl() = row->getString(USERAPPL);
      snr->exceptGoverningCxr() = row->getChar(EXCEPTGOVERNINGCXR);
      snr->exceptTicketingCxr() = row->getChar(EXCEPTTICKETINGCXR);

      std::string direct = row->getString(DIRECTIONALITY);
      if (direct == "F")
        snr->directionality() = FROM;
      else if (direct == "W")
        snr->directionality() = WITHIN;
      else if (direct == "O")
        snr->directionality() = ORIGIN;
      else if (direct == "X")
        snr->directionality() = TERMINATE;
      else if (direct.empty() || direct == " " || direct == "B")
        snr->directionality() = BETWEEN;

      snr->exceptLoc() = row->getChar(EXCEPTLOC);

      LocKey* loc = &snr->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &snr->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      strToGlobalDirection(snr->globalDir(), row->getString(GLOBALDIR));
      snr->travelType() = row->getChar(TRAVELTYPE);

      loc = &snr->viaLoc();
      loc->locType() = row->getChar(VIALOCTYPE);
      loc->loc() = row->getString(VIALOC);

      snr->curRestrAppl() = row->getChar(CURRESTRAPPL);
      snr->posExceptInd() = row->getChar(POSEXCEPTIND);

      loc = &snr->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      snr->poiExceptInd() = row->getChar(POIEXCEPTIND);

      loc = &snr->poiLoc();
      loc->locType() = row->getChar(POILOCTYPE);
      loc->loc() = row->getString(POILOC);
    } // New Parent

    // Add new Currency Restr & return
    if (!row->isNull(CUR))
    {
      CurrencyCode cr = row->getString(CUR);
      snr->curRstrs().push_back(cr);
    }
    return snr;
  } // mapRowToSalesNationRestrBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSalesNationRestrBaseHistoricalSQLStatement
    : public QueryGetSalesNationRestrBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("s.NATION = %1q");
    //          "    and EXPIREDATE > 0");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause for Historical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllSalesNationRestrBaseHistoricalSQLStatement
    : public QueryGetSalesNationRestrBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    //        this->Where("EXPIREDATE > 0");
    this->Where("");
    this->OrderBy("s.NATION");
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetSalesNatRestrGovCxrs
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSalesNatRestrGovCxrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSalesNatRestrGovCxrsSQLStatement() {};
  virtual ~QueryGetSalesNatRestrGovCxrsSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER");
    this->From("=SALESNATRESTRGOVCXR");
    this->Where("NATION = %1q"
                "   and VERSIONDATE = %2n"
                "   and SEQNO = %3n"
                "   and CREATEDATE = %4n");

    if (DataManager::forceSortOrder())
      this->OrderBy("NATION, VERSIONDATE, SEQNO, CREATEDATE, CARRIER");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const CarrierCode mapRowToCarrier(Row* row) { return row->getString(CARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSalesNatRestrTktgCxrs
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSalesNatRestrTktgCxrsSQLStatement
    : public QueryGetSalesNatRestrGovCxrsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->From("=SALESNATRESTRTKTGCXR"); }
};

////////////////////////////////////////////////////////////////////////
// QueryGetSalesNatRestrText
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetSalesNatRestrTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSalesNatRestrTextSQLStatement() {};
  virtual ~QueryGetSalesNatRestrTextSQLStatement() {};

  enum ColumnIndexes
  {
    TEXT = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select s.TEXT");
    this->From("=SALESNATRESTRTEXT t, =FREETEXTSEG s");
    this->Where("NATION = %1q"
                "   and VERSIONDATE = %2n"
                "   and t.SEQNO = %3n"
                "   and t.CREATEDATE = %4n"
                "   and t.MESSAGETYPE = s.MESSAGETYPE"
                "   and t.MSGITEMNO = s.ITEMNO");

    if (DataManager::forceSortOrder())
      this->OrderBy("t.NATION, t.VERSIONDATE, t.SEQNO, t.CREATEDATE, t.USERAPPLTYPE, t.USERAPPL, "
                    "t.MSGCATEGORY, t.MSGITEMNO, s.MESSAGETYPE, s.ITEMNO, s.SEQNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToText(Row* row) { return row->getString(TEXT); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

} // tse
