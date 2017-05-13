//----------------------------------------------------------------------------
//          File:           QueryGetFareTypeQualifierSQLStatement.h
//          Description:    QueryGetFareTypeQualifierSQLStatement
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
#include "DBAccess/Queries/QueryGetFareTypeQualifier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareTypeQualPsgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareTypeQualPsgSQLStatement() {};
  virtual ~QueryGetFareTypeQualPsgSQLStatement() {};

  enum ColumnIndexes
  {
    PSGTYPE = 0,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select b.PSGTYPE");

    //		        this->From("=FARETYPEQUALIFIER a LEFT OUTER JOIN =FARETYPEQUALPSG b"
    //		                   " USING (USERAPPLTYPE,USERAPPL,FARETYPEQUALIFIER,JOURNEYTYPEDOM,"
    //		                   " JOURNEYTYPEINTL,JOURNEYTYPEEOE,PRICINGUNITDOM,PRICINGUNITINTL,"
    //		                   " CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(9);
    joinFields.push_back("USERAPPLTYPE");
    joinFields.push_back("USERAPPL");
    joinFields.push_back("FARETYPEQUALIFIER");
    joinFields.push_back("JOURNEYTYPEDOM");
    joinFields.push_back("JOURNEYTYPEINTL");
    joinFields.push_back("JOURNEYTYPEEOE");
    joinFields.push_back("PRICINGUNITDOM");
    joinFields.push_back("PRICINGUNITINTL");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=FARETYPEQUALIFIER", "a", "LEFT OUTER JOIN", "=FARETYPEQUALPSG", "b", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("a.USERAPPLTYPE = %1q"
                " and a.USERAPPL = %2q"
                " and a.FARETYPEQUALIFIER = %3q"
                " and a.JOURNEYTYPEDOM = %4q"
                " and a.JOURNEYTYPEINTL = %5q"
                " and a.JOURNEYTYPEEOE = %6q"
                " and a.PRICINGUNITDOM = %7q"
                " and a.PRICINGUNITINTL = %8q"
                " and %cd <= a.EXPIREDATE"
                " and b.PSGTYPE IS NOT NULL");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static void mapRowToFareTypeQualPsg(FareTypeQualifier& ftq, Row* row)
  {
    ftq.addPsgType(row->getString(PSGTYPE));
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};

}; // class QueryGetFareTypeQualPsgSQLStatement

template <class QUERYCLASS>
class QueryGetFareTypeQualifierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareTypeQualifierSQLStatement() {};
  virtual ~QueryGetFareTypeQualifierSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    FARETYPEQUALIFIER,
    JOURNEYTYPEDOM,
    JOURNEYTYPEINTL,
    JOURNEYTYPEEOE,
    PRICINGUNITDOM,
    PRICINGUNITINTL,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    MEMONO,
    CREATORID,
    CREATORBUSINESSUNIT,
    FARETYPE,
    FARETYPEREQIND,
    GROUPTRAILERMSGIND,
    ITTRAILERMSGIND,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.USERAPPLTYPE,a.USERAPPL,a.FARETYPEQUALIFIER,"
                  " a.JOURNEYTYPEDOM,a.JOURNEYTYPEINTL,a.JOURNEYTYPEEOE,"
                  " a.PRICINGUNITDOM,a.PRICINGUNITINTL,"
                  " a.CREATEDATE,a.EXPIREDATE,a.LOCKDATE,a.EFFDATE,"
                  " a.DISCDATE,a.MEMONO,a.CREATORID,a.CREATORBUSINESSUNIT,"
                  " b.FARETYPE,b.FARETYPEREQIND,b.GROUPTRAILERMSGIND,b.ITTRAILERMSGIND");

    //		        this->From(" =FARETYPEQUALIFIER a LEFT OUTER JOIN =FARETYPEQUALMSG b"
    //		                     " USING (USERAPPLTYPE,USERAPPL,FARETYPEQUALIFIER,JOURNEYTYPEDOM,"
    //		                     " JOURNEYTYPEINTL,JOURNEYTYPEEOE,PRICINGUNITDOM,PRICINGUNITINTL,"
    //		                     " CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(9);
    joinFields.push_back("USERAPPLTYPE");
    joinFields.push_back("USERAPPL");
    joinFields.push_back("FARETYPEQUALIFIER");
    joinFields.push_back("JOURNEYTYPEDOM");
    joinFields.push_back("JOURNEYTYPEINTL");
    joinFields.push_back("JOURNEYTYPEEOE");
    joinFields.push_back("PRICINGUNITDOM");
    joinFields.push_back("PRICINGUNITINTL");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=FARETYPEQUALIFIER", "a", "LEFT OUTER JOIN", "=FARETYPEQUALMSG", "b", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("a.USERAPPLTYPE = %1q"
                " and a.USERAPPL = %2q"
                " and a.FARETYPEQUALIFIER = %3q"
                " and %cd <= a.EXPIREDATE"
                " and b.FARETYPE IS NOT NULL");

    this->OrderBy("a.USERAPPLTYPE,a.USERAPPL,a.FARETYPEQUALIFIER,a.JOURNEYTYPEDOM,a."
                  "JOURNEYTYPEINTL,a.JOURNEYTYPEEOE,a.PRICINGUNITDOM,a.PRICINGUNITINTL,a."
                  "CREATEDATE,b.FARETYPE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static void mapRowToFareTypeQualifier(FareTypeQualifier& fareTypeQualifier, Row* row)
  {
    fareTypeQualifier.userApplType() = row->getChar(USERAPPLTYPE);
    fareTypeQualifier.userAppl() = row->getString(USERAPPL);
    fareTypeQualifier.fareTypeQualifier() = row->getString(FARETYPEQUALIFIER);
    fareTypeQualifier.journeyTypeDom() = row->getChar(JOURNEYTYPEDOM);
    fareTypeQualifier.journeyTypeIntl() = row->getChar(JOURNEYTYPEINTL);
    fareTypeQualifier.journeyTypeEoe() = row->getChar(JOURNEYTYPEEOE);
    fareTypeQualifier.pricingUnitDom() = row->getChar(PRICINGUNITDOM);
    fareTypeQualifier.pricingUnitIntl() = row->getChar(PRICINGUNITINTL);
    fareTypeQualifier.createDate() = row->getDate(CREATEDATE);
    fareTypeQualifier.expireDate() = row->getDate(EXPIREDATE);
    fareTypeQualifier.lockDate() = row->getDate(LOCKDATE);
    fareTypeQualifier.effDate() = row->getDate(EFFDATE);
    fareTypeQualifier.discDate() = row->getDate(DISCDATE);
    fareTypeQualifier.memoNo() = row->getInt(MEMONO);
    fareTypeQualifier.creatorId() = row->getString(CREATORID);
    fareTypeQualifier.creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFareTypeQualifierSQLStatement

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllFareTypeQualifierSQLStatement
    : public QueryGetFareTypeQualifierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= a.EXPIREDATE and b.FARETYPE IS NOT NULL"); };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareTypeQualPsgHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareTypeQualPsgHistoricalSQLStatement
    : public QueryGetFareTypeQualPsgSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("a.USERAPPLTYPE = %1q"
                " and a.USERAPPL = %2q"
                " and a.FARETYPEQUALIFIER = %3q"
                " and a.JOURNEYTYPEDOM = %4q"
                " and a.JOURNEYTYPEINTL = %5q"
                " and a.JOURNEYTYPEEOE = %6q"
                " and a.PRICINGUNITDOM = %7q"
                " and a.PRICINGUNITINTL = %8q"
                " and b.PSGTYPE IS NOT NULL");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFareTypeQualifierHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFareTypeQualifierHistoricalSQLStatement
    : public QueryGetFareTypeQualifierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("a.USERAPPLTYPE = %1q"
                " and a.USERAPPL = %2q"
                " and a.FARETYPEQUALIFIER = %3q"
                " and b.FARETYPE IS NOT NULL");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareTypeQualifierHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareTypeQualifierHistoricalSQLStatement
    : public QueryGetFareTypeQualifierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("b.FARETYPE IS NOT NULL");
    if (!DataManager::forceSortOrder())
      this->OrderBy("a.USERAPPLTYPE,a.USERAPPL,a.FARETYPEQUALIFIER");
  }
};
}

