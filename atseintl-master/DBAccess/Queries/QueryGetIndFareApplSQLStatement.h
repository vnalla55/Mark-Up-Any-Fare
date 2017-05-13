//----------------------------------------------------------------------------
//          File:           QueryGetIndFareApplSQLStatement.h
//          Description:    QueryGetIndFareApplSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetIndFareAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetIndFareApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetIndFareApplSQLStatement() {};
  virtual ~QueryGetIndFareApplSQLStatement() {};

  enum ColumnIndexes
  {
    SELECTIONTYPE = 0,
    CARRIER,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FARETARIFF,
    VENDOR,
    USERAPPL,
    YYFAREAPPL,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    RULE,
    ROUTING,
    FOOTNOTE,
    CUR,
    FARECLASS,
    FARETYPE,
    OWRT,
    USERAPPLTYPE,
    EXCEPTCARRIER,
    ITEMNO,
    XFARETARIFF,
    XVENDOR,
    XUSERAPPL,
    XYYFAREAPPL,
    XDIRECTIONALITY,
    XLOC1TYPE,
    XLOC1,
    XLOC2TYPE,
    XLOC2,
    XGLOBALDIR,
    XRULE,
    XROUTING,
    XFOOTNOTE,
    XCUR,
    XFARECLASS,
    XFARETYPE,
    XOWRT,
    XUSERAPPLTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ap.SELECTIONTYPE,ap.CARRIER,ap.SEQNO,ap.CREATEDATE,EXPIREDATE,"
                  " EFFDATE,DISCDATE,ap.FARETARIFF,ap.VENDOR,ap.USERAPPL,ap.YYFAREAPPL,"
                  " ap.DIRECTIONALITY,ap.LOC1TYPE,ap.LOC1,ap.LOC2TYPE,ap.LOC2,ap.GLOBALDIR,"
                  " ap.RULE,ap.ROUTING,ap.FOOTNOTE,ap.CUR,ap.FARECLASS,ap.FARETYPE,ap.OWRT,"
                  " ap.USERAPPLTYPE,EXCEPTCARRIER,ITEMNO,ex.FARETARIFF XFARETARIFF,"
                  " ex.VENDOR XVENDOR,ex.USERAPPL XUSERAPPL,ex.YYFAREAPPL XYYFAREAPPL,"
                  " ex.DIRECTIONALITY XDIRECTIONALITY,ex.LOC1TYPE XLOC1TYPE,ex.LOC1 XLOC1,"
                  " ex.LOC2TYPE XLOC2TYPE,ex.LOC2 XLOC2,ex.GLOBALDIR XGLOBALDIR,ex.RULE XRULE,"
                  " ex.ROUTING XROUTING,ex.FOOTNOTE XFOOTNOTE,ex.CUR XCUR,ex.FARECLASS XFARECLASS,"
                  " ex.FARETYPE XFARETYPE,ex.OWRT XOWRT,ex.USERAPPLTYPE XUSERAPPLTYPE");

    //		        this->From("=INDUSTRYFAREAPPL ap left outer join =INDFAREYYEXCEPT ex"
    //		                   " using (SELECTIONTYPE,CARRIER,VERSIONDATE,"
    //		                          " SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(5);
    joinFields.push_back("SELECTIONTYPE");
    joinFields.push_back("CARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=INDUSTRYFAREAPPL", "ap", "left outer join", "=INDFAREYYEXCEPT", "ex", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("ap.SELECTIONTYPE = %1q"
                " and ap.CARRIER = %2q "
                " and %cd <= ap.EXPIREDATE");
    this->OrderBy("ap.SELECTIONTYPE,ap.CARRIER,ap.YYFAREAPPL,ap.SEQNO,ap.CREATEDATE,ex."
                  "EXCEPTCARRIER,ex.ITEMNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::IndustryFareAppl*
  mapRowToIndustryFareAppl(Row* row, tse::IndustryFareAppl* prevIndFare, DateTime& prevDate)
  {
    tse::IndustryFareAppl* indFare;

    Indicator selectionType = row->getChar(SELECTIONTYPE);
    CarrierCode carrier = row->getString(CARRIER);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    if (prevIndFare != nullptr && prevIndFare->selectionType() == selectionType &&
        prevIndFare->carrier() == carrier && prevIndFare->seqNo() == seqNo &&
        prevDate == createDate)
    {
      indFare = prevIndFare;
    }
    else
    {
      indFare = new tse::IndustryFareAppl;

      indFare->selectionType() = selectionType;
      indFare->carrier() = carrier;
      indFare->seqNo() = seqNo;
      prevDate = createDate;

      indFare->createDate() = row->getDate(CREATEDATE);
      indFare->expireDate() = row->getDate(EXPIREDATE);
      indFare->effDate() = row->getDate(EFFDATE);
      indFare->discDate() = row->getDate(DISCDATE);

      indFare->fareTariff() = row->getInt(FARETARIFF);
      indFare->vendor() = row->getString(VENDOR);
      indFare->userAppl() = row->getString(USERAPPL);
      indFare->yyFareAppl() = row->getChar(YYFAREAPPL);
      indFare->directionality() = row->getChar(DIRECTIONALITY);
      indFare->loc1().locType() = row->getChar(LOC1TYPE);
      indFare->loc1().loc() = row->getString(LOC1);
      indFare->loc2().locType() = row->getChar(LOC2TYPE);
      indFare->loc2().loc() = row->getString(LOC2);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(indFare->globalDir(), gd);

      indFare->rule() = row->getString(RULE);
      indFare->routing() = row->getString(ROUTING);
      indFare->footNote() = row->getString(FOOTNOTE);
      indFare->cur() = row->getString(CUR);
      indFare->fareClass() = row->getString(FARECLASS);
      indFare->fareType() = row->getString(FARETYPE);
      indFare->owrt() = row->getChar(OWRT);
      indFare->userApplType() = row->getChar(USERAPPLTYPE);
    }

    if (!row->isNull(EXCEPTCARRIER) && row->getString(EXCEPTCARRIER) != std::string(""))
    {
      tse::IndustryFareAppl::ExceptAppl exAppl;

      exAppl.exceptCarrier() = row->getString(EXCEPTCARRIER);
      exAppl.itemNo() = row->getInt(ITEMNO);
      exAppl.fareTariff() = row->getInt(XFARETARIFF);
      exAppl.vendor() = row->getString(XVENDOR);
      exAppl.userAppl() = row->getString(XUSERAPPL);
      exAppl.yyFareAppl() = row->getChar(XYYFAREAPPL);
      exAppl.directionality() = row->getChar(XDIRECTIONALITY);
      exAppl.loc1().locType() = row->getChar(XLOC1TYPE);
      exAppl.loc1().loc() = row->getString(XLOC1);
      exAppl.loc2().locType() = row->getChar(XLOC2TYPE);
      exAppl.loc2().loc() = row->getString(XLOC2);

      std::string gd = row->getString(XGLOBALDIR);
      strToGlobalDirection(exAppl.globalDir(), gd);

      exAppl.rule() = row->getString(XRULE);
      exAppl.routing() = row->getString(XROUTING);
      exAppl.footNote() = row->getString(XFOOTNOTE);
      exAppl.cur() = row->getString(XCUR);
      exAppl.fareClass() = row->getString(XFARECLASS);
      exAppl.fareType() = row->getString(XFARETYPE);
      exAppl.owrt() = row->getChar(XOWRT);
      exAppl.userApplType() = row->getChar(XUSERAPPLTYPE);

      indFare->except().push_back(exAppl);
    }

    return indFare;
  } // mapRowToIndustryFareAppl()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetIndFareApplSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetIndFareApplHistoricalSQLStatement : public QueryGetIndFareApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("ap.SELECTIONTYPE = %1q"
                " and (ap.CARRIER = %2q or ap.CARRIER = '')"
                " and %3n <= ap.EXPIREDATE"
                " and %4n >= ap.CREATEDATE");
  }; // adjustBaseSQL()
}; // class QueryGetIndFareApplHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndFareApplSQLStatement : public QueryGetIndFareApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }; // adjustBaseSQL()
}; // class QueryGetAllIndFareApplSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllIndFareApplHistoricalSQLStatement
    : public QueryGetIndFareApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("1 = 1"); }; // adjustBaseSQL()
}; // class QueryGetAllIndFareApplHistoricalSQLStatement
}
