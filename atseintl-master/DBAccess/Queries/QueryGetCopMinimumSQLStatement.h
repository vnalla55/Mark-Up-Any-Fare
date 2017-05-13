//----------------------------------------------------------------------------
//          File:           QueryGetCopMinimumSQLStatement.h
//          Description:    QueryGetCopMinimumSQLStatement
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
#include "DBAccess/Queries/QueryGetCopMinimum.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCopMinBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCopMinBaseSQLStatement() {};
  virtual ~QueryGetCopMinBaseSQLStatement() {};

  enum ColumnIndexes
  {
    COPNATION = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    USERAPPL,
    TRAVELAPPL,
    PUNORMALSPECIALTYPE,
    PUTRIPTYPE,
    FARETYPE,
    PUAPPL,
    PUORIGLOCTYPE,
    PUORIGLOC,
    PUWITHINLOCTYPE,
    PUWITHINLOC,
    CONSTPOINTLOCTYPE,
    CONSTPOINTLOC,
    FARETYPEAPPL,
    USERAPPLTYPE,
    COPCARRIER,
    PARTICIPATIONIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select m.COPNATION,m.VERSIONDATE,m.SEQNO,m.CREATEDATE,EXPIREDATE,EFFDATE,"
                  "  DISCDATE,USERAPPL,TRAVELAPPL,PUNORMALSPECIALTYPE,PUTRIPTYPE,"
                  "  FARETYPE,PUAPPL,PUORIGLOCTYPE,PUORIGLOC,PUWITHINLOCTYPE,PUWITHINLOC,"
                  "  CONSTPOINTLOCTYPE,CONSTPOINTLOC,FARETYPEAPPL,USERAPPLTYPE,"
                  "  c.COPCARRIER,c.PARTICIPATIONIND");

    //		        this->From("=COPMINIMUM m LEFT OUTER JOIN =COPCARRIER c"
    //		                   "                USING (COPNATION,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("COPNATION");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=COPMINIMUM", "m", "LEFT OUTER JOIN", "=COPCARRIER", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("m.COPNATION = %1q "
                " and %cd <= EXPIREDATE");
    this->OrderBy("m.COPNATION,m.VERSIONDATE,m.SEQNO,m.CREATEDATE,c.COPCARRIER");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CopMinimum* mapRowToCopMinBase(Row* row, CopMinimum* cmPrev)
  { // Load up Parent Determinant Fields
    NationCode copNation = row->getString(COPNATION);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    CopMinimum* cm;

    // If Parent hasn't changed, add to Children (tktgCarriers)
    if (cmPrev != nullptr && cmPrev->copNation() == copNation && cmPrev->versionDate() == versionDate &&
        cmPrev->seqNo() == seqNo && cmPrev->createDate() == createDate)
    { // Just add to Prev
      cm = cmPrev;
    }
    else
    { // Time for a new Parent
      cm = new tse::CopMinimum;
      cm->copNation() = copNation;
      cm->versionDate() = versionDate;
      cm->seqNo() = seqNo;
      cm->createDate() = createDate;

      cm->expireDate() = row->getDate(EXPIREDATE);
      cm->effDate() = row->getDate(EFFDATE);
      cm->discDate() = row->getDate(DISCDATE);
      cm->userAppl() = row->getString(USERAPPL);
      cm->travelAppl() = row->getChar(TRAVELAPPL);
      cm->puNormalSpecialType() = row->getChar(PUNORMALSPECIALTYPE);
      cm->puTripType() = row->getChar(PUTRIPTYPE);
      cm->fareType() = row->getString(FARETYPE);
      cm->puAppl() = row->getChar(PUAPPL);

      LocKey* loc = &cm->puOrigLoc();
      loc->locType() = row->getChar(PUORIGLOCTYPE);
      loc->loc() = row->getString(PUORIGLOC);

      loc = &cm->puWithinLoc();
      loc->locType() = row->getChar(PUWITHINLOCTYPE);
      loc->loc() = row->getString(PUWITHINLOC);

      loc = &cm->constPointLoc();
      loc->locType() = row->getChar(CONSTPOINTLOCTYPE);
      loc->loc() = row->getString(CONSTPOINTLOC);

      cm->fareTypeAppl() = row->getChar(FARETYPEAPPL);
      cm->userApplType() = row->getChar(USERAPPLTYPE);
    } // New Parent

    // Add new CopCarrier & return
    if (!row->isNull(PARTICIPATIONIND))
    {
      CopCarrier* cc = new CopCarrier;
      cc->copCarrier() = row->getString(COPCARRIER);
      cc->participationInd() = row->getChar(PARTICIPATIONIND);
      cm->cxrs().push_back(cc);
    }
    return cm;
  } // mapRowToCopMinBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCopMinBaseSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCopMinBaseHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCopMinBaseHistoricalSQLStatement : public QueryGetCopMinBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("m.COPNATION = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("m.COPNATION,m.VERSIONDATE,m.SEQNO,m.CREATEDATE,c.COPCARRIER");
    else
      this->OrderBy("SEQNO,m.CREATEDATE");
  }
}; // class QueryGetCopMinBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCopMinBase
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllCopMinBaseSQLStatement : public QueryGetCopMinBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCopMinBaseHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllCopMinBaseHistoricalSQLStatement
    : public QueryGetCopMinBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    if (DataManager::forceSortOrder())
      this->OrderBy("m.COPNATION,m.VERSIONDATE,m.SEQNO,m.CREATEDATE,c.COPCARRIER");
    else
      this->OrderBy("COPNATION,SEQNO,m.CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetCopTktgCxrExcpts
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCopTktgCxrExcptsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCopTktgCxrExcptsSQLStatement() {};
  virtual ~QueryGetCopTktgCxrExcptsSQLStatement() {};

  enum ColumnIndexes
  {
    TKTGCARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TKTGCARRIER");
    this->From("=COPTKTGCXREXCEPT");
    this->Where("COPNATION = %1q"
                "  and VERSIONDATE = %2n"
                "  and SEQNO = %3n"
                "  and CREATEDATE = %4n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToTktgCarrier(Row* row) { return row->getString(TKTGCARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

} // tse
