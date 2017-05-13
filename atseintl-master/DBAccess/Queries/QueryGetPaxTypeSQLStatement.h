//----------------------------------------------------------------------------
//          File:           QueryGetPaxTypeSQLStatement.h
//          Description:    QueryGetPaxTypeSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetPaxType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPaxTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPaxTypeSQLStatement() {}
  virtual ~QueryGetPaxTypeSQLStatement() {}

  enum ColumnIndexes
  {
    PSGTYPE = 0,
    VENDOR,
    DESCRIPTION,
    CHILDIND,
    ADULTIND,
    PSGGROUPTYPE,
    NUMBERSEATSREQ,
    INFANTIND,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PSGTYPE,VENDOR,DESCRIPTION,CHILDIND,ADULTIND,"
                  "       PSGGROUPTYPE,NUMBERSEATSREQ,INFANTIND,CREATEDATE");
    this->From("=PSGTYPE ");
    this->Where("PSGTYPE = %1q "
                "  and VENDOR = %2q");

    if (DataManager::forceSortOrder())
      this->OrderBy("PSGTYPE,VENDOR");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PaxTypeInfo* mapRowToPaxType(Row* row)
  {
    tse::PaxTypeInfo* paxType = new tse::PaxTypeInfo;

    paxType->paxType() = row->getString(PSGTYPE);
    paxType->vendor() = row->getString(VENDOR);
    paxType->description() = row->getString(DESCRIPTION);
    paxType->childInd() = row->getChar(CHILDIND);
    paxType->adultInd() = row->getChar(ADULTIND);
    paxType->psgGroupType() = row->getString(PSGGROUPTYPE);
    paxType->numberSeatsReq() = row->getInt(NUMBERSEATSREQ);
    paxType->infantInd() = row->getChar(INFANTIND);
    paxType->createDate() = row->getDate(CREATEDATE);

    paxType->initPsgType();

    return paxType;
  } // mapRowToPaxType()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetPaxTypesSQLStatement : public QueryGetPaxTypeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where(""); }
};
}

