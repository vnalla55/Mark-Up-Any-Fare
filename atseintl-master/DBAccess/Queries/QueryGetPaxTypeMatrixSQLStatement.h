//----------------------------------------------------------------------------
//          File:           QueryGetPaxTypeMatrixSQLStatement.h
//          Description:    QueryGetPaxTypeMatrixSQLStatement
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
#include "DBAccess/Queries/QueryGetPaxTypeMatrix.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPaxTypeMatrixSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPaxTypeMatrixSQLStatement() {}
  virtual ~QueryGetPaxTypeMatrixSQLStatement() {}

  enum ColumnIndexes
  {
    SABREPSGTYPE = 0,
    ATPPSGTYPE,
    CARRIER,
    ALTERNATEPSGTYPE,
    BULKIND,
    CREATEDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SABREPSGTYPE,ATPPSGTYPE,CARRIER,ALTERNATEPSGTYPE,BULKIND,CREATEDATE");
    this->From("=PSGTYPEMATRIX ");
    this->Where("SABREPSGTYPE = %1q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("SABREPSGTYPE,ATPPSGTYPE,CARRIER,SABREPSGTYPE,LOCKDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PaxTypeMatrix* mapRowToPaxTypeMatrix(Row* row)
  {
    tse::PaxTypeMatrix* paxTypeMatrix = new tse::PaxTypeMatrix;

    paxTypeMatrix->sabrePaxType() = row->getString(SABREPSGTYPE);
    paxTypeMatrix->atpPaxType() = row->getString(ATPPSGTYPE);
    paxTypeMatrix->carrier() = row->getString(CARRIER);
    paxTypeMatrix->alternatePaxType() = row->getString(ALTERNATEPSGTYPE);
    paxTypeMatrix->bulkInd() = row->getChar(BULKIND);
    paxTypeMatrix->createDate() = row->getDate(CREATEDATE);

    return paxTypeMatrix;
  } // mapRowToPaxTypeMatrix()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetPaxTypeMatrixsSQLStatement : public QueryGetPaxTypeMatrixSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where(""); }
};
}

