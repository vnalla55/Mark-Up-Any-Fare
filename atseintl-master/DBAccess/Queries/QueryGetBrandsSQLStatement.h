//----------------------------------------------------------------------------
//          File:           QueryGetBrandsSQLStatement.h
//          Description:    QueryGetBrandsSQLStatement
//          Created:        10/29/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetBrands.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBrandsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBrandsSQLStatement() {};
  virtual ~QueryGetBrandsSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    CARRIER,
    BRANDID,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NEWSEQNO,
    BRANDNAME,
    BRANDTEXT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,CARRIER,BRANDID,VERSIONDATE,SEQNO,"
                  "       CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,NEWSEQNO,BRANDNAME,"
                  "       BRANDTEXT");
    this->From("=BRAND ");
    this->Where("USERAPPLTYPE = %1q "
                "   and USERAPPL = %2q "
                "   and CARRIER = %3q "
                "   and %cd <= EXPIREDATE ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static Brand* mapRowToBrand(Row* row)
  {
    Brand* info = new Brand;

    info->userApplType() = row->getChar(USERAPPLTYPE);
    info->userAppl() = row->getString(USERAPPL);
    info->carrier() = row->getString(CARRIER);
    info->brandId() = row->getString(BRANDID);
    info->versionDate() = row->getDate(VERSIONDATE);
    info->seqno() = row->getLong(SEQNO);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    info->effDate() = row->getDate(EFFDATE);
    info->discDate() = row->getDate(DISCDATE);
    info->newSeqno() = row->getLong(NEWSEQNO);
    info->brandName() = row->getString(BRANDNAME);
    info->brandText() = row->getString(BRANDTEXT);

    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBrandsHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBrandsHistoricalSQLStatement : public QueryGetBrandsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("USERAPPLTYPE = %1q "
                "   and USERAPPL = %2q "
                "   and CARRIER = %3q ");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBrands
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBrandsSQLStatement : public QueryGetBrandsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("USERAPPLTYPE,USERAPPL,CARRIER,SEQNO");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBrandsHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBrandsHistoricalSQLStatement : public QueryGetBrandsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,CARRIER,SEQNO");
  };
};
} // tse
