//----------------------------------------------------------------------------
//          File:           QueryGetBrandedFareAppSQLStatement.h
//          Description:    QueryGetBrandedFareAppSQLStatement
//          Created:        10/29/2007
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
#include "DBAccess/Queries/QueryGetBrandedFareApp.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBrandedFareAppSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBrandedFareAppSQLStatement() {};
  virtual ~QueryGetBrandedFareAppSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    CARRIER,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    TVLEFFDATE,
    TVLDISCDATE,
    EFFDATE,
    DISCDATE,
    NEWSEQNO,
    DIRECTIONALITY,
    MARKETTYPECODE,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    BRANDID,
    BOOKINGCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.USERAPPLTYPE,a.USERAPPL,a.CARRIER,a.VERSIONDATE,a.SEQNO,"
                  "       a.CREATEDATE,a.EXPIREDATE,a.TVLEFFDATE,a.TVLDISCDATE,"
                  "       a.EFFDATE,a.DISCDATE,a.NEWSEQNO,a.DIRECTIONALITY,"
                  "       a.MARKETTYPECODE,a.LOC1TYPE,a.LOC1,a.LOC2TYPE,a.LOC2,"
                  "       b.BRANDID,b.BOOKINGCODE");

    //		        this->From("=BRANDEDFAREAPP a"
    //		                   "        left outer join =BRANDEDFAREAPPSEG b"
    //		                   "          using
    //(USERAPPLTYPE,USERAPPL,CARRIER,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(6);
    joinFields.push_back("USERAPPLTYPE");
    joinFields.push_back("USERAPPL");
    joinFields.push_back("CARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=BRANDEDFAREAPP", "a", "left outer join", "=BRANDEDFAREAPPSEG", "b", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("a.USERAPPLTYPE = %1q"
                "   and a.USERAPPL = %2q"
                "   and a.CARRIER = %3q"
                "   and %cd <= a.EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("a.SEQNO,a.USERAPPLTYPE,a.USERAPPL,a.CARRIER,a.VERSIONDATE,a.CREATEDATE,b."
                    "BRANDID,b.BOOKINGCODE");
    else
      this->OrderBy("a.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static BrandedFareApp* mapRowToBrandedFareApp(Row* row)
  {
    BrandedFareApp* info = new BrandedFareApp;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->carrier() = row->getString(CARRIER);
    info->versionDate() = row->getDate(VERSIONDATE);
    info->seqno() = row->getLong(SEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    info->tvlEffDate() = row->getDate(TVLEFFDATE);
    info->tvlDiscDate() = row->getDate(TVLDISCDATE);
    info->effDate() = row->getDate(EFFDATE);
    info->discDate() = row->getDate(DISCDATE);
    info->newSeqno() = row->getLong(NEWSEQNO);

    std::string dir = row->getString(DIRECTIONALITY);
    if (dir == "F")
      info->directionality() = FROM;
    else if (dir == "W")
      info->directionality() = WITHIN;
    else if (dir == "O")
      info->directionality() = ORIGIN;
    else if (dir == "X")
      info->directionality() = TERMINATE;
    else if (dir.empty() || dir == " " || dir == "B")
      info->directionality() = BETWEEN;

    info->marketTypeCode() = row->getString(MARKETTYPECODE);
    info->loc1().locType() = row->getChar(LOC1TYPE);
    info->loc1().loc() = row->getString(LOC1);
    info->loc2().locType() = row->getChar(LOC2TYPE);
    info->loc2().loc() = row->getString(LOC2);
    info->brandId() = row->getString(BRANDID);
    info->bookingCode() = row->getString(BOOKINGCODE);

    return info;
  } // mapRowToBrandedFareApp()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
