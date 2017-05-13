//----------------------------------------------------------------------------
//          File:           QueryGetJointCxrSQLStatement.h
//          Description:    QueryGetJointCxrSQLStatement
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
#include "DBAccess/Queries/QueryGetJointCxr.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetJointCxrSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetJointCxrSQLStatement() {};
  virtual ~QueryGetJointCxrSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    INHIBIT,
    VALIDITYIND,
    CARRIER1,
    CARRIER2,
    CARRIER3,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,INHIBIT,"
                  "       VALIDITYIND,CARRIER1,CARRIER2,CARRIER3");
    this->From("=JOINTCARRIER");
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n "
                " and %cd <= EXPIREDATE "
                " and VALIDITYIND = 'Y'");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::JointCarrier* mapRowToJointCarrier(Row* row)
  {
    tse::JointCarrier* jointCxr = new tse::JointCarrier;

    jointCxr->vendor() = row->getString(VENDOR);
    jointCxr->itemNo() = row->getInt(ITEMNO);
    jointCxr->seqNo() = row->getLong(SEQNO);
    jointCxr->createDate() = row->getDate(CREATEDATE);
    jointCxr->expireDate() = row->getDate(EXPIREDATE);
    jointCxr->inhibit() = row->getChar(INHIBIT);
    jointCxr->validityInd() = row->getChar(VALIDITYIND);
    jointCxr->carrier1() = row->getString(CARRIER1);
    jointCxr->carrier2() = row->getString(CARRIER2);
    jointCxr->carrier3() = row->getString(CARRIER3);

    return jointCxr;
  } // mapRowToJointCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetJointCxrSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetJointCxrHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetJointCxrHistoricalSQLStatement : public QueryGetJointCxrSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetJointCxrHistoricalSQLStatement
}
