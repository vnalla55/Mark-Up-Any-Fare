//----------------------------------------------------------------------------
//          File:           QueryGetNoPNRFareTypeGroupSQLStatement.h
//          Description:    QueryGetNoPNRFareTypeGroupSQLStatement
//          Created:        1/11/2008
//          Authors:        Karolina Golebiewska
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
#include "DBAccess/Queries/QueryGetNoPNRFareTypeGroup.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNoPNRFareTypeGroupSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNoPNRFareTypeGroupSQLStatement() {};
  virtual ~QueryGetNoPNRFareTypeGroupSQLStatement() {};

  enum ColumnIndexes
  {
    FARETYPEGROUP = 0,
    FARETYPEDESIG,
    FARETYPE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    CABIN,
    FARETYPEAPPL,
    RESTRIND,
    FARETYPEDISPLAY,
    DESCRIPTION,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select c.FARETYPEGROUP, cs.FARETYPEDESIG,"
        "       cs.FARETYPE, cs.SEQNO, cs.CREATEDATE,"
        "       cs.EXPIREDATE, cs.EFFDATE, cs.DISCDATE,"
        "       cs.CABIN, cs.FARETYPEAPPL, cs.RESTRIND,"
        "       cs.FARETYPEDISPLAY, cs.DESCRIPTION, cs.VERSIONINHERITEDIND, cs.VERSIONDISPLAYIND");

    //		        this->From("=NOPNRFARETYPEGROUPSEG c LEFT OUTER JOIN =FARETYPEMATRIX cs "
    //		                   "                        USING (FARETYPEDESIG)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(1);
    joinFields.push_back("FARETYPEDESIG");
    this->generateJoinString("=NOPNRFARETYPEGROUPSEG",
                             "c",
                             "LEFT OUTER JOIN",
                             "=FARETYPEMATRIX",
                             "cs",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("c.FARETYPEGROUP = %1n AND %cd <= cs.EXPIREDATE");
    // this->OrderBy("cs.FARETYPE");

    if (DataManager::forceSortOrder())
      this->OrderBy("c.FARETYPEGROUP, cs.FARETYPE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NoPNRFareTypeGroup*
  mapRowToNoPNRFareTypeGroup(Row* row, NoPNRFareTypeGroup* npftgPrev)
  {
    Indicator cabin = row->getChar(CABIN);

    int fareTypeGroup = row->getInt(FARETYPEGROUP);
    // int fareTypeDesig    = row->getInt(FARETYPEDESIG);

    NoPNRFareTypeGroup* npftg;

    // If Parent hasn't changed, add to Child (segs)
    if (npftgPrev != nullptr && npftgPrev->fareTypeGroup() == fareTypeGroup)
    //    && npftgPrev->fareTypeDesig() == fareTypeDesig)
    {
      npftg = npftgPrev;
    }
    else
    { // Time for a new Parent
      npftg = new tse::NoPNRFareTypeGroup;
      npftg->fareTypeGroup() = fareTypeGroup;
      // npftg->fareTypeDesig() = fareTypeDesig;
    }

    // Load up new Seg
    if (!row->isNull(FARETYPE))
    {
      FareTypeMatrix* newSeg = new FareTypeMatrix;

      newSeg->fareType() = row->getString(FARETYPE);
      newSeg->seqNo() = row->getLong(SEQNO);
      newSeg->createDate() = row->getDate(CREATEDATE);
      newSeg->expireDate() = row->getDate(EXPIREDATE);
      newSeg->effDate() = row->getDate(EFFDATE);
      newSeg->discDate() = row->getDate(DISCDATE);
      newSeg->cabin().setClass(cabin);
      int ftd = row->getInt(FARETYPEDESIG);
      newSeg->fareTypeDesig().setFareTypeDesignator(ftd);
      newSeg->fareTypeAppl() = row->getChar(FARETYPEAPPL);
      newSeg->restrInd() = row->getChar(RESTRIND);
      newSeg->fareTypeDisplay() = row->getChar(FARETYPEDISPLAY);
      newSeg->description() = row->getString(DESCRIPTION);
      newSeg->versioninheritedInd() = row->getChar(VERSIONINHERITEDIND);
      newSeg->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);

      npftg->segs().push_back(newSeg);
    }

    return npftg;
  } // mapRowToNoPNROptions()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllNoPNRFareTypeGroup
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllNoPNRFareTypeGroupSQLStatement
    : public QueryGetNoPNRFareTypeGroupSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1=1");
    this->OrderBy(
        "c.FARETYPEGROUP, c.FARETYPEDESIG, cs.VERSIONDATE, cs.FARETYPE, cs.SEQNO, cs.CREATEDATE ");
  }
};
} // tse
