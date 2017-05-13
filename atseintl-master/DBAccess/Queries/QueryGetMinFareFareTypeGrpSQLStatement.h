//----------------------------------------------------------------------------
//          File:           QueryGetMinFareFareTypeGrpSQLStatement.h
//          Description:    QueryGetMinFareFareTypeGrpSQLStatement
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
#include "DBAccess/Queries/QueryGetMinFareFareTypeGrp.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMinFareFareTypeGrpSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareFareTypeGrpSQLStatement() {};
  virtual ~QueryGetMinFareFareTypeGrpSQLStatement() {};

  enum ColumnIndexes
  {
    SPECIALPROCESSNAME = 0,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    FARETYPE,
    GROUPSETNO,
    SETNO,
    GROUPTYPE,
    ORDERNO,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select g.SPECIALPROCESSNAME,g.CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,"
                  "       s.FARETYPE,GROUPSETNO,SETNO,GROUPTYPE,ORDERNO");

    //		        this->From("=MINFAREFARETYPEGRP g LEFT OUTER JOIN =MINFAREFARETYPEGRPSEG s"
    //		                   "                              USING
    //(SPECIALPROCESSNAME,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(2);
    joinFields.push_back("SPECIALPROCESSNAME");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=MINFAREFARETYPEGRP",
                             "g",
                             "LEFT OUTER JOIN",
                             "=MINFAREFARETYPEGRPSEG",
                             "s",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("%cd <= EXPIREDATE"
                "    and g.SPECIALPROCESSNAME = %1q");
    this->OrderBy("g.CREATEDATE,GROUPSETNO,SETNO,ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MinFareFareTypeGrp* mapRowToMinFareFareTypeGrp(Row* row, MinFareFareTypeGrp* ftgPrev)
  {
    std::string specialProcessName = row->getString(SPECIALPROCESSNAME);
    DateTime createDate = row->getDate(CREATEDATE);

    MinFareFareTypeGrp* ftg;

    // If Parent hasn't changed, add to Child (segs)
    if (ftgPrev != nullptr && ftgPrev->specialProcessName() == specialProcessName &&
        ftgPrev->createDate() == createDate)
    { // Add to Prev
      ftg = ftgPrev;
    }
    else
    { // Time for a new Parent
      ftg = new tse::MinFareFareTypeGrp;
      ftg->specialProcessName() = specialProcessName;
      ftg->createDate() = createDate;
      ftg->effDate() = row->getDate(EFFDATE);
      ftg->expireDate() = row->getDate(EXPIREDATE);
      ftg->discDate() = row->getDate(DISCDATE);
    } // else (New Parent)

    // Check for Child
    if (!row->isNull(FARETYPE))
    {
      MinFareFareTypeGrpSeg* newSeg = new MinFareFareTypeGrpSeg;
      newSeg->fareType() = row->getString(FARETYPE);
      newSeg->setNo() = row->getInt(SETNO);
      newSeg->orderNo() = row->getInt(ORDERNO);
      newSeg->grpSetNo() = row->getInt(GROUPSETNO);
      newSeg->grpType() = row->getInt(GROUPTYPE);
      ftg->segs().push_back(newSeg);
    }
    return ftg;
  } // mapRowToMinFareFareTypeGrp()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where an OrderBy clauses
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetMinFareFareTypeGrpHistoricalSQLStatement
    : public QueryGetMinFareFareTypeGrpSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("g.SPECIALPROCESSNAME = %1q");
    this->OrderBy("g.CREATEDATE,GROUPSETNO,SETNO,ORDERNO");
  };
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where an OrderBy clauses
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMinFareFareTypeGrpSQLStatement
    : public QueryGetMinFareFareTypeGrpSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("SPECIALPROCESSNAME,g.CREATEDATE,GROUPSETNO,SETNO,ORDERNO");
  };
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where an OrderBy clauses
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllMinFareFareTypeGrpHistoricalSQLStatement
    : public QueryGetMinFareFareTypeGrpSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("SPECIALPROCESSNAME,CREATEDATE,GROUPSETNO,SETNO,ORDERNO,FARETYPE");
  };
};

} // tse
