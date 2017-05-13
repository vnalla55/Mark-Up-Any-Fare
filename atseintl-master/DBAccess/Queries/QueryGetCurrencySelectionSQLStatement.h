//----------------------------------------------------------------------------
//          File:           QueryGetCurrencySelectionSQLStatement.h
//          Description:    QueryGetCurrencySelectionSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetCurrencySelection.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetOneCurSelBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOneCurSelBaseSQLStatement() {};
  virtual ~QueryGetOneCurSelBaseSQLStatement() {};

  enum ColumnIndexes
  {
    VERSIONDATE = 0,
    NATION,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    JOURNEYRESTR,
    FARECOMPONENTRESTR,
    POSLOCTYPE,
    POSLOC,
    POSEXCEPT,
    POILOCTYPE,
    POILOC,
    POIEXCEPT,
    FARECOMPPRIMECUR,
    EQUIVOVERRIDECUR,
    FAREQUOTEOVERRIDECUR,
    GOVCARRIEREXCEPT,
    RESTRCUREXCEPT,
    PSGTYPEEXCEPT,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    GOVCARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select s.VERSIONDATE,s.NATION,s.SEQNO,s.CREATEDATE,EXPIREDATE,EFFDATE,"
                  "       DISCDATE,JOURNEYRESTR,FARECOMPONENTRESTR,POSLOCTYPE,POSLOC,"
                  "       POSEXCEPT,POILOCTYPE,POILOC,POIEXCEPT,FARECOMPPRIMECUR,"
                  "       EQUIVOVERRIDECUR,FAREQUOTEOVERRIDECUR,GOVCARRIEREXCEPT,RESTRCUREXCEPT,"
                  "       PSGTYPEEXCEPT,VERSIONINHERITEDIND,VERSIONDISPLAYIND,g.GOVCARRIER");

    //		        this->From("=CURRENCYSELECTION s LEFT OUTER JOIN =CURSELECTIONGOVCXR g"
    //		                   "                             USING
    //(VERSIONDATE,NATION,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("NATION");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=CURRENCYSELECTION", "s", "LEFT OUTER JOIN", "=CURSELECTIONGOVCXR", "g", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("s.NATION = %1q"
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("s.NATION,s.VERSIONDATE,s.SEQNO,s.CREATEDATE,g.GOVCARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CurrencySelection* mapRowToCurrencySelection(Row* row, CurrencySelection* csPrev)
  { // Load up Parent Determinant Fields
    DateTime versionDate = row->getDate(VERSIONDATE);
    NationCode nation = row->getString(NATION);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    CurrencySelection* cs;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (csPrev != nullptr && csPrev->versionDate() == versionDate && csPrev->nation() == nation &&
        csPrev->seqNo() == seqNo && csPrev->createDate() == createDate)
    { // Just add to Prev
      cs = csPrev;
    }
    else
    { // Time for a new Parent
      cs = new tse::CurrencySelection;
      cs->versionDate() = versionDate;
      cs->nation() = nation;
      cs->seqNo() = seqNo;
      cs->createDate() = createDate;

      cs->expireDate() = row->getDate(EXPIREDATE);
      cs->effDate() = row->getDate(EFFDATE);
      cs->discDate() = row->getDate(DISCDATE);
      cs->journeyRestr() = row->getChar(JOURNEYRESTR);
      cs->farecomponentRestr() = row->getChar(FARECOMPONENTRESTR);

      LocKey* loc = &cs->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);
      cs->posexcept() = row->getChar(POSEXCEPT);

      loc = &cs->poiLoc();
      loc->locType() = row->getChar(POILOCTYPE);
      loc->loc() = row->getString(POILOC);
      cs->poiExcept() = row->getChar(POIEXCEPT);

      cs->fareCompPrimeCur() = row->getString(FARECOMPPRIMECUR);
      cs->equivOverrideCur() = row->getString(EQUIVOVERRIDECUR);
      cs->fareQuoteOverrideCur() = row->getString(FAREQUOTEOVERRIDECUR);
      cs->govCarrierExcept() = row->getChar(GOVCARRIEREXCEPT);
      cs->restrCurExcept() = row->getChar(RESTRCUREXCEPT);
      cs->psgTypeExcept() = row->getChar(PSGTYPEEXCEPT);
      cs->versioninheritedInd() = row->getChar(VERSIONINHERITEDIND);
      cs->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);
    } // New Parent

    // Add new TktgCarrier & return
    if (!row->isNull(GOVCARRIER))
    {
      cs->govCarriers().push_back(row->getString(GOVCARRIER));
    }
    return cs;
  } // mapRowToCurrencySelection()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetOneCurSelBaseSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetOneCurSelBaseHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOneCurSelBaseHistoricalSQLStatement
    : public QueryGetOneCurSelBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("s.NATION = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("NATION,VERSIONDATE,SEQNO,CREATEDATE");
    else
      this->OrderBy("");
  }
}; // class QueryGetOneCurSelBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCurSelBase
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllCurSelBaseSQLStatement : public QueryGetOneCurSelBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCurSelBaseHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllCurSelBaseHistoricalSQLStatement
    : public QueryGetOneCurSelBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    if (DataManager::forceSortOrder())
      this->OrderBy("s.NATION,s.VERSIONDATE,s.SEQNO,s.CREATEDATE,g.GOVCARRIER");
    else
      this->OrderBy("s.NATION,s.SEQNO,s.CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetCurSelRestrCur
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCurSelRestrCurSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCurSelRestrCurSQLStatement() {};
  virtual ~QueryGetCurSelRestrCurSQLStatement() {};

  enum ColumnIndexes
  {
    RESTRICTEDCUR = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select RESTRICTEDCUR");
    this->From("=CURSELECTIONRESTRCUR");
    this->Where("VERSIONDATE = %1n"
                "   and NATION = %2q"
                "   and SEQNO = %3n"
                "   and CREATEDATE = %4n");
    if (DataManager::forceSortOrder())
      this->OrderBy("RESTRICTEDCUR");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToRestrictedCur(Row* row) { return row->getString(RESTRICTEDCUR); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCurSelRestrCurSQLStatement

////////////////////////////////////////////////////////////////////////
// QueryGetCurSelPsgrType
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCurSelPsgrTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCurSelPsgrTypeSQLStatement() {};
  virtual ~QueryGetCurSelPsgrTypeSQLStatement() {};

  enum ColumnIndexes
  {
    PASSENGERTYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PASSENGERTYPE");
    this->From("=CURSELECTIONPSGTYPE");
    this->Where("VERSIONDATE = %1n"
                "   and NATION = %2q"
                "   and SEQNO = %3n"
                "   and CREATEDATE = %4n");

    if (DataManager::forceSortOrder())
      this->OrderBy("PASSENGERTYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToPassengerType(Row* row) { return row->getString(PASSENGERTYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetCurSelAseanCur
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCurSelAseanCurSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCurSelAseanCurSQLStatement() {};
  virtual ~QueryGetCurSelAseanCurSQLStatement() {};

  enum ColumnIndexes
  {
    CUR = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CUR");
    this->From("=CURSELECTASEANCUR");
    this->Where("VERSIONDATE = %1n"
                "   and NATION = %2q"
                "   and SEQNO = %3n"
                "   and CREATEDATE = %4n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToCurrencySelection(Row* row) { return row->getString(CUR); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetCurSelTextMsg
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCurSelTextMsgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCurSelTextMsgSQLStatement() {};
  virtual ~QueryGetCurSelTextMsgSQLStatement() {};

  enum ColumnIndexes
  {
    TEXTAPPL = 0,
    CUSTOMERID,
    SEQNO,
    TEXT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select cst.TEXTAPPL,cst.CUSTOMERID,fts.SEQNO,fts.TEXT");
    this->From("=CURSELECTIONTEXT cst, =FREETEXT ft, =FREETEXTSEG fts");
    this->Where("cst.VERSIONDATE = %1n"
                "    and cst.NATION = %2q"
                "    and cst.SEQNO = %3n"
                "    and cst.CREATEDATE = %4n"
                "    and cst.MESSAGETYPE = ft.MESSAGETYPE"
                "    and cst.ITEMNO = ft.ITEMNO"
                "    and ft.MESSAGETYPE = fts.MESSAGETYPE"
                "    and ft.ITEMNO = fts.ITEMNO");

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
