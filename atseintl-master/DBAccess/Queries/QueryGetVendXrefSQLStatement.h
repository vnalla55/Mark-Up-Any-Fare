//----------------------------------------------------------------------------
//          File:           QueryGetVendXrefSQLStatement.h
//          Description:    QueryGetVendXrefSQLStatement
//          Created:        10/7/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVendXref.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetVendXrefSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetVendXrefSQLStatement() {}
  virtual ~QueryGetVendXrefSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    VENDORTYPE,
    TARIFFCROSSREFVENDOR,
    RULECATEGORYVENDOR,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,VENDORTYPE,TARIFFCROSSREFVENDOR,RULECATEGORYVENDOR,CREATEDATE");
    this->From("=VENDORCROSSREF");
    this->Where("VENDOR = %1q");
    this->OrderBy("VENDOR");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::VendorCrossRef* mapRowToVendorCrossRef(Row* row)
  {
    tse::VendorCrossRef* vxr = new tse::VendorCrossRef;

    vxr->vendor() = row->getString(VENDOR);
    vxr->vendorType() = row->getChar(VENDORTYPE);
    vxr->tariffCrossRefVendor() = row->getString(TARIFFCROSSREFVENDOR);
    vxr->ruleCategoryVendor() = row->getString(RULECATEGORYVENDOR);
    vxr->createDate() = row->getDate(CREATEDATE);

    return vxr;
  } // mapRowToVendorCrossRef()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllVendXrefSQLStatement : public QueryGetVendXrefSQLStatement<QUERYCLASS>
{

private:
  virtual void adjustBaseSQL() { this->Where(""); }
};
}

